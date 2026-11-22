//
// Header for timeout support functions for the Fast Light Tool Kit (FLTK).
//
// Author: Albrecht Schlosser
// Copyright 2021-2024 by Bill Spitzak and others.
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

#ifndef _src_Fl_Timeout_h_
#define _src_Fl_Timeout_h_

#include <FL/Fl.H>
#include <FL/Fl_Callback_Interface.H>

#define FL_TIMEOUT_DEBUG 0        // 1 = include debugging features, 0 = no

/** \file
  Fl_Timeout handling.

  This file contains implementations of:

  - Fl::add_timeout()
  - Fl::repeat_timeout()
  - Fl::has_timeout()
  - Fl::remove_timeout()
  - Fl::remove_next_timeout()

  and related methods of class Fl_Timeout.
*/

/**
  The internal class Fl_Timeout handles all timeout related functions.

  All code is platform independent except retrieving a timestamp which
  requires calling a system driver function and potentially results in
  different timer resolutions (from milliseconds to microseconds).

  Related user documentation:

  - \ref Fl_Timeout_Handler
  - Fl::add_timeout(double time, Fl_Timeout_Handler cb, void *data)
  - Fl::repeat_timeout(double time, Fl_Timeout_Handler cb, void *data)
  - Fl::has_timeout(Fl_Timeout_Handler cb, void *data)
  - Fl::remove_timeout(Fl_Timeout_Handler cb, void *data)
  - Fl::remove_next_timeout(Fl_Timeout_Handler cb, void *data, void **data_return)

*/
class Fl_Timeout : public Fl_Simple_Callback_Interface<Fl_Timeout> {

protected:

  Fl_Timeout *next;             // ** Link to next timeout
  double time;                  // delay until timeout
  int skip;                     // skip "new" (inserted) timers (issue #450)

  // constructor
  Fl_Timeout() {
    next = 0;
    time = 0;
    skip = 0;
  }

  // destructor
  ~Fl_Timeout() = default;

  /**
    Get an Fl_Timeout instance for further handling.

    The timer object will be initialized with the input parameters
    as given by Fl::add_timeout() or Fl::repeat_timeout().

    Fl_Timeout objects are maintained in three queues:
    - active timer queue
    - list (stack, i.e. LIFO) of currently executing timer callbacks
    - free timer entries.

    When the FLTK program is launched all queues are empty. Whenever
    a new timer object is required the get() method is called and a timer
    object is either found in the queue of free timer entries or a new
    timer object is created (operator new).

    Active timer entries are inserted into the "active timer queue" until
    they expire and their callback is called.

    Before the callback is called the timer entry is inserted into the list
    of current timers, i.e. it becomes the Fl_Timeout::current() timeout.
    This can be used in Fl::repeat_timeout() to find out if and how long the
    current timeout has been delayed.

    When a timer is no longer used it is popped from the \p current list
    and inserted into the "free timer" list so it can be reused later.

    Timer queue entries are never returned to the system, there's no garbage
    collection. The total number of timer objects is determined by the
    largest number of concurrently active timers.

    \param[in]  time  requested delta time
    \param[in]  cb    timer callback
    \param[in]  data  userdata for timer callback

    \return  Fl_Timeout*  Timer entry

    \see Fl::add_timeout(), Fl::repeat_timeout()
  */
  template<typename... Args>
  static Fl_Timeout *get(double time, Args&&... args)
  {
    Fl_Timeout *t = get();
    t->delay(time);
    t->callback(std::forward<Args>(args)...); // unpack the arguments
    return t;
  }

  static Fl_Timeout *get();

  // insert this timer into the active timer queue, sorted by expiration time
  void insert();

  // remove this timer from the active timer queue and
  // add it to the "current" timer stack
  void make_current();

  // remove this timer from the current timer stack and
  // add it to the list of free timers
  void release();

  /** Get the timer's delay in seconds. */
  double delay() {
    return time;
  }

  /** Set the timer's delay in seconds. */
  void delay(double t) {
    time = t;
  }

public:
  // Returns whether the given timeout is active.
  static int has_timeout(Fl_Timeout_Handler cb, void *data);

  // Add or remove timeouts

  /**
  Adds a one-shot timeout callback.

  The callback function \p cb will be called by Fl::wait() at \p time seconds
  after this function is called.

  \param[in]  time    delta time in seconds until the timer expires
  \param[in]  cb      callback function
  \param[in]  data    optional user data (default: \p NULL)

  Implements:

      void Fl::add_timeout(double time, Fl_Timeout_Handler cb, void *data)

  \see Fl::add_timeout(double time, Fl_Timeout_Handler cb, void *data)
  */
  template<typename... Args>
  static void add_timeout(double time, Args&&... args) {
    elapse_timeouts();
    Fl_Timeout *t = get(time, std::forward<Args>(args)...);
    t->insert();
  }

  /**
    Repeats a timeout callback from the expiration of the previous timeout,
    allowing for more accurate timing.

    \param[in]  time    delta time in seconds until the timer expires
    \param[in]  cb      callback function
    \param[in]  data    optional user data (default: \p NULL)

    Implements:

        void Fl::repeat_timeout(double time, Fl_Timeout_Handler cb, void *data)

    \see Fl::repeat_timeout(double time, Fl_Timeout_Handler cb, void *data)
  */
  template<typename... Args>
  static void repeat_timeout(double time, Args&&... args) {
    elapse_timeouts();
    Fl_Timeout *t = (Fl_Timeout *)get(time, std::forward<Args>(args)...);
    Fl_Timeout *cur = current_timeout;
    if (cur) {
      t->time += cur->time;   // was: missed_timeout_by (always <= 0.0)
      if (t->time < 0.0)
        t->time = 0.001;      // at least 1 ms
    }
    t->insert();
  }

  static void remove_timeout(Fl_Timeout_Handler cb, void *data);
  static int remove_next_timeout(Fl_Timeout_Handler cb, void *data = NULL, void **data_return = NULL);
  static std::vector<Fl::TimeoutData> timeout_list();

  // Elapse timeouts, i.e. calculate new delay time of all timers.
  // This does not call the timer callbacks.
  static void elapse_timeouts();

  // Elapse timeouts and call timer callbacks.
  static void do_timeouts();

  // Return the delay in seconds until the next timer expires.
  static double time_to_wait(double ttw);

#if FL_TIMEOUT_DEBUG
  // Write some statistics to stdout
  static void debug(int level = 1);
#endif

protected:

  static Fl_Timeout *current();

  /**
    List of active timeouts.

    These timeouts can be triggered when due, which calls their callbacks.
    The lifetime of a timeout:
    - active, in this queue
    - callback running, in queue \p current_timeout
    - done, in list of free timeouts, ready to be reused.
  */
  static Fl_Timeout *first_timeout;

  /**
    List of free timeouts after use.
    Timeouts can be reused many times.
  */
  static Fl_Timeout *free_timeout;

  /**
    The list of current timeouts is used to store the timeout whose callback
    is called while the callback is executed. This is used like a stack, the
    current timeout is pushed to the front of the list and once the callback
    is finished, that timeout is removed and entered into the free list.

    Background: Fl::repeat_timeout() needs to know which timeout triggered it
    and the exact schedule time and/or the delay of that timeout, i.e. how
    long the scheduled time was missed before the callback was called.
    A static, global variable is not sufficient since the user code can call
    other functions, e.g. dialogs, that run a nested event loop which can
    run another timeout callback. Hence this list of "current" timeouts is
    used like a stack (last in, first out).

    \see Fl_Timeout::push()                 Member function (method)
  */
  static Fl_Timeout *current_timeout;   // list of "current" timeouts

}; // class Fl_Timeout

#endif // _src_Fl_Timeout_h_
