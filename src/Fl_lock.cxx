//
// Multi-threading support code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2025 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//

#include <config.h>
#include <FL/Fl.H>
#include "Fl_System_Driver.H"

#include <stdlib.h>

/*
   From Bill:

   I would prefer that FLTK contain the minimal amount of extra
   stuff for doing threads.  There are other portable thread
   wrapper libraries out there and FLTK should not be providing
   another.  This file is an attempt to make minimal additions
   and make them self-contained in this source file.

   From Mike:

   Starting with 1.1.8, we now have a callback so that you can
   process awake() messages as they come in.

   The API:

   Fl::lock() - recursive lock.  You must call this before the
   first call to Fl::wait()/run() to initialize the thread
   system. The lock is locked all the time except when
   Fl::wait() is waiting for events.

   Fl::unlock() - release the recursive lock.

   Fl::awake(void*) - Causes Fl::wait() to return (with the lock
   locked) even if there are no events ready.

   Fl::awake(void (*cb)(void *), void*) - Call a function
   in the main thread from within another thread of execution.

   Fl::thread_message() - returns an argument sent to an
   Fl::awake() call, or returns NULL if none.  WARNING: the
   current implementation only has a one-entry queue and only
   returns the most recent value!

   From Matt:

   25 years later, we have blazing fast CPUs with 24 cores and more.
   Fl::awake(void*) is no longer useful as "the last value" could have
   been overwritten by another thread before the main thread gets to it.
   Also, the ring buffer may potentially fill up much faster than expected,
   so I introduced Fl::awake_once(void (*cb)(void *), void*) which removes
   duplicate entries in the queue.
*/

#ifndef FL_DOXYGEN

static constexpr int AWAKE_RING_SIZE = 1024;
Fl_Awake_Handler *Fl_System_Driver::awake_ring_ = nullptr;
void **Fl_System_Driver::awake_data_ = nullptr;
int Fl_System_Driver::awake_ring_size_ = 0;
int Fl_System_Driver::awake_ring_head_ = 0;
int Fl_System_Driver::awake_ring_tail_ = 0;
bool Fl_System_Driverawake_pending_ = false;

#endif

/**
 \cond DriverDev
 \addtogroup DriverDeveloper
 \{
 */

/**
 \brief Adds an awake handler for use in awake().

 \internal Adds an awake handler for use in awake().

 \param[in] func The function to call when the main thread is awake.
 \param[in] data The user data to pass to the function.
 \param[in] once If true, the handler will be added only once, removing any
                 existing handler with the same function pointer and data pointer.
 \return 0 on success, -1 if the ring buffer is full.
 */
int Fl_System_Driver::push_awake_handler(Fl_Awake_Handler func, void *data, bool once)
{
  int ret = 0;
  Fl::system_driver()->lock_ring();

  // Allocate the ring buffers if we have not done so yet.
  if (!awake_ring_) {
    awake_ring_size_ = AWAKE_RING_SIZE;
    awake_ring_ = (Fl_Awake_Handler*)malloc(awake_ring_size_*sizeof(Fl_Awake_Handler));
    awake_data_ = (void**)malloc(awake_ring_size_*sizeof(void*));
    // explicitly initialize the head and tail indices
    awake_ring_head_= awake_ring_tail_ = 0;
  }

  // If we want to add the handler only once, go through the list of existing
  // handlers and remove any handler with the same function pointer
  // and data pointer.
  if (once) {
    int src = awake_ring_tail_;
    int dst = awake_ring_tail_;
    while (src != awake_ring_head_) {
      if ((awake_ring_[src] != func) || (awake_data_[src] != data)) {
        if (src != dst) {
          awake_ring_[dst] = awake_ring_[src];
          awake_data_[dst] = awake_data_[src];
        }
        dst++;
        if (dst >= awake_ring_size_) dst = 0; // wrap around
      }
      src++;
      if (src >= awake_ring_size_) src = 0; // wrap around
    }
    awake_ring_head_ = dst;
  }

  // The next head index we will want (not the current index):
  // We use this to check if the ring-buffer is full or not
  // (and to update awake_ring_head_ if we do use the current index.)
  int next_head = awake_ring_head_ + 1;
  if (next_head >= awake_ring_size_) {
    next_head = 0;
  }
  // check that the ring buffer is not full
  if (next_head == awake_ring_tail_) {
    // ring is full. Return -1 as an error indicator.
    ret = -1;
  } else {
    awake_ring_[awake_ring_head_] = func;
    awake_data_[awake_ring_head_] = data;
    awake_ring_head_ = next_head;
  }

  Fl::system_driver()->unlock_ring();
  return ret;
}

/**
 \brief Gets the last stored awake handler for use in awake().
 \internal Used in the main event loop when an Awake message is received.
 */
int Fl_System_Driver::pop_awake_handler(Fl_Awake_Handler &func, void *&data)
{
  int ret = 0;
  Fl::system_driver()->lock_ring();
  if ((!awake_ring_) || (awake_ring_head_ == awake_ring_tail_)) {
    ret = -1;
  } else {
    func = awake_ring_[awake_ring_tail_];
    data = awake_data_[awake_ring_tail_];
    ++awake_ring_tail_;
    if (awake_ring_tail_ >= awake_ring_size_) {
      awake_ring_tail_ = 0;
    }
  }
  Fl::system_driver()->unlock_ring();
  return ret;
}

/**
 \brief Checks if the awake ring buffer is empty.
 \internal Used in the main event loop when an Awake message is received.
 */
bool Fl_System_Driver::awake_ring_empty() {
  Fl::system_driver()->lock_ring();
  bool retval = (awake_ring_head_ == awake_ring_tail_);
  Fl::system_driver()->unlock_ring();
  return retval;
}

/**
 \}
 \endcond
 */

/**
 \brief Notifies the main GUI thread from a worker thread.

 In FLTK, worker threads can update the UI, but all UI changes must be wrapped
 between Fl::lock() and Fl::unlock(). After calling Fl::unlock(), the worker
 thread should call Fl::awake() to signal the main thread that
 updates are pending.

 \note Worker threads must not create, show, or hide windows.

 \see \ref advanced_multithreading
 \see Fl::awake(Fl_Awake_Handler, void*)
 \see Fl::awake_once(Fl_Awake_Handler, void*)
 */
void Fl::awake() {
  Fl::system_driver()->awake(nullptr);
}

/**
 \brief Awake the main GUI thread and leave a message pointer.

 \deprecated Use Fl::awake() or Fl::awake(Fl_Awake_Handler, void*) instead.

 This method is deprecated. The API can not ensure that Fl::thread_message()
 returns the messages sent by Fl::awake(void *v) complete and in the correct
 order.

 Use Fl::awake() instead if you do not need to send a specific message.
 Use Fl::awake(Fl_Awake_Handler, void*) or Fl::awake_once(Fl_Awake_Handler, void*)
 if you need to send a message to the main thread and ensure that all messages
 are processed in the order they were sent.

 \see \ref advanced_multithreading
 \see Fl::awake()
 \see Fl::awake(Fl_Awake_Handler, void*)
 \see Fl::awake_once(Fl_Awake_Handler, void*)
*/
void Fl::awake(void *v) {
  Fl::system_driver()->awake(v);
}

/**
 \brief Schedules a callback to be executed by the main thread, then wakes up the main thread.

 This function lets a worker thread request that a specific callback function
 be run by the main thread, passing optional user data. The callback will be
 executed during the main thread's next event handling cycle.

 The queue holding the list of handlers is limited to 1024 entries.
 If the queue is full, the function will return -1 and the callback will not be
 scheduled. However the main thread will still be woken up to process any
 other pending events.

 \note If user_data points to dynamically allocated memory, it is the
 responsibility of the caller to ensure that the memory is valid until the
 callback is executed. The callback will be executed during the main thread's
 next event handling cycle, but depending on the sytems load, this may take
 several seconds.

 \return 0 if the callback was successfully scheduled
 \return -1 if the queue is full.

 \see Fl::awake()
 \see Fl::awake_once(Fl_Awake_Handler, void*)
 \see \ref advanced_multithreading
*/
int Fl::awake(Fl_Awake_Handler handler, void *user_data) {
  int ret = Fl_System_Driver::push_awake_handler(handler, user_data, false);
  Fl::awake();
  return ret;
}

/**
 \brief Schedules a callback to be executed once by the main thread, then wakes up the main thread.

 This function lets a worker thread request that a specific callback function
 be run by the main thread, passing optional user data. If a callback with the
 same user_data is already scheduled, the previous entry will be removed and
 the new entry will be appended to the list.

 \return 0 if the callback was successfully scheduled
 \return -1 if the queue is full.

 \see Fl::awake()
 \see Fl::awake(Fl_Awake_Handler, void*)
 \see \ref advanced_multithreading
*/
int Fl::awake_once(Fl_Awake_Handler handler, void *user_data) {
  // TODO: remove any previous entry with the same handler and user_data
  int ret = Fl_System_Driver::push_awake_handler(handler, user_data, true);
  Fl::awake();
  return ret;
}

/**
 \brief Returns the last message sent by a child thread.

 \deprecated Use Fl::awake(Fl_Awake_Handler, void*) or
 Fl::awake_once(Fl_Awake_Handler, void*) instead.

 The thread_message() method returns the last message
 that was sent from a child by the Fl::awake(void*) method.

 This method is deprecated. The API can not ensure that Fl::thread_message()
 returns the messages sent by Fl::awake(void *v) complete and in the correct
 order.

 \see \ref advanced_multithreading
 \see Fl::awake()
 \see Fl::awake(Fl_Awake_Handler, void*)
 \see Fl::awake_once(Fl_Awake_Handler, void*)
*/
void* Fl::thread_message() {
  return Fl::system_driver()->thread_message();
}


/**
 \brief Acquire the global UI lock for FLTK.

  The lock() method blocks the current thread until it
  can safely access FLTK widgets and data. Child threads should
  call this method prior to updating any widgets or accessing
  data. The main thread must call Fl::lock() once before any windows are shown
  to initialize the threading support in FLTK. The initial Fl::lock() call
  will return non-zero if threading is not available on the platform.

  Child threads enclose calls to FLTK functions between Fl::lock() and
  Fl::unlock() accessing FLTK. When a child thread has finshed accessing FLTK
  and wants the main thread to update the UI, it should call Fl::awake().

  Child threads can never create, show, or hide windows.

  When the wait() method is waiting
  for input or timeouts, child threads are given access to FLTK.
  Similarly, when the main thread needs to do processing, it will
  wait until all child threads have called unlock() before processing
  additional data.

  \return 0 if threading is available on the platform; non-zero
  otherwise.

  \see \ref advanced_multithreading
  \see Fl::lock()
  \see Fl::awake()
*/
int Fl::lock() {
  return Fl::system_driver()->lock();
}

/**
 \brief Release the global UI lock set by Fl::lock().

  The unlock() method releases the lock that was set using the lock() method.
  Child threads should call this method as soon as they are finished
  accessing FLTK.

  \see \ref advanced_multithreading
  \see Fl::lock()
*/
void Fl::unlock() {
  Fl::system_driver()->unlock();
}

