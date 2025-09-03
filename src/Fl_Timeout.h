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
class Fl_Timeout {

protected:

  Fl_Timeout *next;             // ** Link to next timeout
  Fl_Timeout_Handler callback;  // the user's callback
  void *data;                   // the user's callback data
  double time;                  // delay until timeout
  int skip;                     // skip "new" (inserted) timers (issue #450)

  // constructor
  Fl_Timeout() {
    next = 0;
    callback = 0;
    data = 0;
    time = 0;
    skip = 0;
  }

  // destructor
  ~Fl_Timeout() {}

  // get a new timer entry from the pool or allocate a new one
  static Fl_Timeout *get(double time, Fl_Timeout_Handler cb, void *data);

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

  static void add_timeout(double time, Fl_Timeout_Handler cb, void *data);
  static void repeat_timeout(double time, Fl_Timeout_Handler cb, void *data);
  static void remove_timeout(Fl_Timeout_Handler cb, void *data);
  static int remove_next_timeout(Fl_Timeout_Handler cb, void *data = NULL, void **data_return = NULL);

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
