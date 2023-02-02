//
// Multi-threading support code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2021 by Bill Spitzak and others.
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
*/

#ifndef FL_DOXYGEN
Fl_Awake_Handler *Fl::awake_ring_;
void **Fl::awake_data_;
int Fl::awake_ring_size_;
int Fl::awake_ring_head_;
int Fl::awake_ring_tail_;
#endif

static const int AWAKE_RING_SIZE = 1024;

/** Adds an awake handler for use in awake(). */
int Fl::add_awake_handler_(Fl_Awake_Handler func, void *data)
{
  int ret = 0;
  Fl::system_driver()->lock_ring();
  if (!awake_ring_) {
    awake_ring_size_ = AWAKE_RING_SIZE;
    awake_ring_ = (Fl_Awake_Handler*)malloc(awake_ring_size_*sizeof(Fl_Awake_Handler));
    awake_data_ = (void**)malloc(awake_ring_size_*sizeof(void*));
    // explicitly initialize the head and tail indices
    awake_ring_head_= awake_ring_tail_ = 0;
  }
  // The next head index we will want (not the current index):
  // We use this to check if the ring-buffer is full or not
  // (and to update awake_ring_head_ if we do use the current index.)
  int next_head = awake_ring_head_ + 1;
  if (next_head >= awake_ring_size_) {
    next_head = 0;
  }
  // check that the ring buffer is not full, and that it exists
  if ((!awake_ring_) || (next_head == awake_ring_tail_)) {
    // ring is non-existent or full. Return -1 as an error indicator.
    ret = -1;
  } else {
    awake_ring_[awake_ring_head_] = func;
    awake_data_[awake_ring_head_] = data;
    awake_ring_head_ = next_head;
  }
  Fl::system_driver()->unlock_ring();
  return ret;
}

/** Gets the last stored awake handler for use in awake(). */
int Fl::get_awake_handler_(Fl_Awake_Handler &func, void *&data)
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
 Let the main thread know an update is pending and have it call a specific function.
 Registers a function that will be
 called by the main thread during the next message handling cycle.
 Returns 0 if the callback function was registered,
 and -1 if registration failed. Over a thousand awake callbacks can be
 registered simultaneously.

 \see Fl::awake(void* message=0)
*/
int Fl::awake(Fl_Awake_Handler func, void *data) {
  int ret = add_awake_handler_(func, data);
  Fl::awake();
  return ret;
}

/** \fn int Fl::lock()
    The lock() method blocks the current thread until it
    can safely access FLTK widgets and data. Child threads should
    call this method prior to updating any widgets or accessing
    data. The main thread must call lock() to initialize
    the threading support in FLTK. lock() will return non-zero
    if threading is not available on the platform.

    Child threads must call unlock() when they are done
    accessing FLTK.

    When the wait() method is waiting
    for input or timeouts, child threads are given access to FLTK.
    Similarly, when the main thread needs to do processing, it will
    wait until all child threads have called unlock() before processing
    additional data.

    \return 0 if threading is available on the platform; non-zero
    otherwise.

    See also: \ref advanced_multithreading
*/
/** \fn void Fl::unlock()
    The unlock() method releases the lock that was set
    using the lock() method. Child
    threads should call this method as soon as they are finished
    accessing FLTK.

    See also: \ref advanced_multithreading
*/
/** \fn void Fl::awake(void* msg)
    Sends a message pointer to the main thread,
    causing any pending Fl::wait() call to
    terminate so that the main thread can retrieve the message and any pending
    redraws can be processed.

    Multiple calls to Fl::awake() will queue multiple pointers
    for the main thread to process, up to a system-defined (typically several
    thousand) depth. The default message handler saves the last message which
    can be accessed using the
    Fl::thread_message() function.

    In the context of a threaded application, a call to Fl::awake() with no
    argument will trigger event loop handling in the main thread. Since
    it is not possible to call Fl::flush() from a subsidiary thread,
    Fl::awake() is the best (and only, really) substitute.

    It's \e not necessary to wrap calls to any form of Fl::awake() by Fl::lock() and Fl::unlock().
    Nevertheless, the early, single call to Fl::lock() used to initialize threading support is necessary.

    Function Fl::awake() in all its forms is typically called by worker threads, but it can be used safely
    by the main thread too, as a means to break the event loop.

    \see \ref advanced_multithreading
*/

void Fl::awake(void *v) {
  Fl::system_driver()->awake(v);
}

void* Fl::thread_message() {
  return Fl::system_driver()->thread_message();
}

int Fl::lock() {
  return Fl::system_driver()->lock();
}

void Fl::unlock() {
  Fl::system_driver()->unlock();
}

#ifndef FL_DOXYGEN

bool Fl_System_Driver::awake_ring_empty() {
  Fl::system_driver()->lock_ring();
  bool retval = (Fl::awake_ring_head_ == Fl::awake_ring_tail_);
  Fl::system_driver()->unlock_ring();
  return retval;
}

#endif // FL_DOXYGEN
