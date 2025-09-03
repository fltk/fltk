//
// Timeout support functions for the Fast Light Tool Kit (FLTK).
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

#include <config.h>

#include "Fl_Timeout.h"
#include "Fl_System_Driver.H"

#include <stdio.h>
#include <math.h> // for trunc()

#if !HAVE_TRUNC
static inline double trunc(double x) { return x >= 0 ? floor(x) : ceil(x); }
#endif // !HAVE_TRUNC

/**
  \file Fl_Timeout.cxx
*/

// static class variables

Fl_Timeout *Fl_Timeout::free_timeout = 0;
Fl_Timeout *Fl_Timeout::first_timeout = 0;
Fl_Timeout *Fl_Timeout::current_timeout = 0;

#if FL_TIMEOUT_DEBUG
static int num_timers = 0;    // DEBUG
#endif

/**
 Set a time stamp at this point in time with optional signed offset in seconds.

 The time stamp is an opaque type and does not represent the time of day or
 some time and date in the calendar. It is used with Fl::seconds_between() and
 Fl::seconds_since() to measure elapsed time.

 \code
 Fl_Timestamp start = Fl::now();
 // do something
 double s = Fl::seconds_since(start);
 printf("That operation took %g seconds\n", s);
 \endcode

 Depending on the system the resolution may be milliseconds or microseconds.
 Under certain conditions (particularly on Windows) the value in member `sec`
 may wrap around and does not represent a real time (maybe runtime of the system).
 Function seconds_since() below uses this to subtract two timestamps which is
 always a correct delta time with milliseconds or microseconds resolution.

 \param offset optional signed offset in seconds added to the current time
 \return this moment in time offset by \p offset as an opaque time stamp

 \todo Fl::system_driver()->gettime() was implemented for the Forms library and
 has a limited resolution (on Windows: milliseconds). On POSIX platforms it uses
 gettimeofday() with microsecond resolution.
 A new function could use a better resolution on Windows with its multimedia
 timers which requires a new dependency: winmm.lib (dll). This could be a future
 improvement, maybe set as a build option or generally (requires Win95 or 98?).

 \see Fl::seconds_since(Fl_Timestamp& then)
 \see Fl::seconds_between(Fl_Timestamp& back, Fl_Timestamp& further_back)
 \see Fl::ticks_since(Fl_Timestamp& then)
 \see Fl::ticks_between(Fl_Timestamp& back, Fl_Timestamp& further_back)
 */
Fl_Timestamp Fl::now(double offset) {
  Fl_Timestamp ts;
  time_t sec;
  int usec;
  Fl::system_driver()->gettime(&sec, &usec);
  ts.sec = sec;
  ts.usec = usec;
  if (offset) {
    sec = (time_t)trunc(offset);
    usec = int((offset - sec) * 1000000);
    ts.sec += sec;
    if (usec + ts.usec >= 1000000) {
      ts.sec++;
      ts.usec += (usec - 1000000);
    } else if (usec + ts.usec < 0) {
      ts.sec--;
      ts.usec += (usec + 1000000);
    } else
      ts.usec += usec;
  }
  return ts; // C++ will copy the result into the lvalue for us
}


/**
 Return the time in seconds between now and a previously taken time stamp.

 \param[in] then a previously taken time stamp
 \return elapsed seconds and fractions of a second

 \see Fl::seconds_between(Fl_Timestamp& back, Fl_Timestamp& further_back)
    \see Fl::now()  \see Fl::distant_past()
 */
double Fl::seconds_since(Fl_Timestamp& then) {
  Fl_Timestamp ts_now = Fl::now();
  return Fl::seconds_between(ts_now, then);
}

/**
 Return the time in seconds between two time stamps.

 \param[in] back a previously taken time stamp
 \param[in] further_back an even earlier time stamp
 \return elapsed seconds and fractions of a second

 \see Fl::seconds_since(Fl_Timestamp& then) \see  Fl::now()
 */
double Fl::seconds_between(Fl_Timestamp& back, Fl_Timestamp& further_back) {
  return double((back.sec - further_back.sec) + (back.usec - further_back.usec) / 1000000.);
}

/**
 Return the time in ticks (60Hz) between now and a previously taken time stamp.

 Ticks are a convenient way to time animations 'per frame'. Even though
 modern computers use all kinds of screen refresh rates, 60Hz is a very good
 base for animation that is typically shown in user interface graphics.

 \param[in] then a previously taken time stamp
 \return elapsed ticks in 60th of a second

 \see Fl::ticks_between(Fl_Timestamp& back,  Fl_Timestamp& further_back) \see  Fl::now()
 */
long Fl::ticks_since(Fl_Timestamp& then) {
  Fl_Timestamp ts_now = Fl::now();
  return Fl::ticks_between(ts_now, then);
}

/**
 Return the time in ticks (60Hz) between two time stamps.

 \param[in] back a previously taken time stamp
 \param[in] further_back an even earlier time stamp
 \return elapsed ticks in 60th of a second

 \see Fl::ticks_since(Fl_Timestamp& then) \see  Fl::now()
 */
long Fl::ticks_between(Fl_Timestamp& back, Fl_Timestamp& further_back) {
  return long((back.sec-further_back.sec)*60 + (back.usec-further_back.usec)/16666);
}


// Returns 0 and initializes the "previous" timestamp when called for the first time.

/*
  Return the elapsed time since the last call in seconds.

  The first call initializes the internal "previous" timestamp and returns 0.
  This must only be called from Fl_Timeout::elapse_timeouts().

  Todo: remove static variable in this function: previous time should be
  maintained in the caller.

  Return:  double  Elapsed time since the last call
*/
static double elapsed_time() {
  static int first = 1;                 // initialization
  static Fl_Timestamp prev;             // previous timestamp
  Fl_Timestamp now = Fl::now();         // current timestamp
  double elapsed = 0.0;
  if (first) {
    first = 0;
  } else {
    elapsed = Fl::seconds_between(now, prev);
  }
  prev = now;
  return elapsed;
}

/**
  Insert this timer entry into the active timer queue.

  The timer is inserted at the required position so the timer queue
  is always ordered by due time.
*/
void Fl_Timeout::insert() {
  Fl_Timeout **p = (Fl_Timeout **)&first_timeout;
  while (*p && (*p)->time <= time) {
    p = (Fl_Timeout **)&((*p)->next);
  }
  next = *p;
  *p = this;
}

/**
  Returns true if the timeout exists and has not been called yet.

  \param[in]  cb    Timer callback (must match)
  \param[in]  data  Callback user data (must match)

  \returns      whether the timer was found in the queue
  \retval   0   not found
  \retval   1   found

  Implements:

      int Fl::has_timeout(Fl_Timeout_Handler cb, void *data)

  \see Fl::has_timeout(Fl_Timeout_Handler cb, void *data)
*/
int Fl_Timeout::has_timeout(Fl_Timeout_Handler cb, void *data) {
  for (Fl_Timeout *t = first_timeout; t; t = t->next) {
    if (t->callback == cb && t->data == data)
      return 1;
  }
  return 0;
}

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
void Fl_Timeout::add_timeout(double time, Fl_Timeout_Handler cb, void *data) {
  elapse_timeouts();
  Fl_Timeout *t = get(time, cb, data);
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

void Fl_Timeout::repeat_timeout(double time, Fl_Timeout_Handler cb, void *data) {
  elapse_timeouts();
  Fl_Timeout *t = (Fl_Timeout *)get(time, cb, data);
  Fl_Timeout *cur = current_timeout;
  if (cur) {
    t->time += cur->time;   // was: missed_timeout_by (always <= 0.0)
    if (t->time < 0.0)
      t->time = 0.001;      // at least 1 ms
  }
  t->insert();
}

/**
  Remove a timeout callback.

  This method removes all matching timeouts, not just the first one.
  This may change in the future.

  \param[in]  cb    Timer callback to be removed (must match)
  \param[in]  data  Wildcard if NULL, must match otherwise

  Implements:

      void Fl::remove_timeout(Fl_Timeout_Handler cb, void *data)

  \see Fl::remove_timeout(Fl_Timeout_Handler cb, void *data)
*/
void Fl_Timeout::remove_timeout(Fl_Timeout_Handler cb, void *data) {
  for (Fl_Timeout** p = &first_timeout; *p;) {
    Fl_Timeout* t = *p;
    if (t->callback == cb && (t->data == data || !data)) {
      *p = t->next;
      t->next = free_timeout;
      free_timeout = t;
    } else {
      p = &(t->next);
    }
  }
}

/**
  Remove the next matching timeout callback and return its \p data pointer.

  Implements:

      int Fl::remove_next_timeout(Fl_Timeout_Handler cb, void *data, void **data_return)

  \param[in]    cb    Timer callback to be removed (must match)
  \param[in]    data  Wildcard if NULL, must match otherwise
  \param[inout] data_return  pointer to void * to receive the data value

  \return       non-zero if a timer was found and removed
  \retval   0   no matching timer was found
  \retval   1   the last matching timeout was found and removed
  \retval  N>1  a matching timeout was removed and there are\n
                (N - 1) matching timeouts pending

  For details
  \see Fl::remove_next_timeout(Fl_Timeout_Handler cb, void *data, void **data_return)
*/
int Fl_Timeout::remove_next_timeout(Fl_Timeout_Handler cb, void *data, void **data_return) {
  int ret = 0;
  for (Fl_Timeout** p = &first_timeout; *p;) { // scan all timeouts
    Fl_Timeout* t = *p;
    if (t->callback == cb && (t->data == data || !data)) { // timeout matches
      ret++;
      if (ret == 1) { // first timeout: remove
        if (data_return)
          *data_return = t->data;
        *p = t->next;
        t->next = free_timeout;
        free_timeout = t;
        continue;
      }
      p = &(t->next);
    } // timeout matches
    else { // no match
      p = &(t->next);
    }
  } // scan all timeouts
  return ret;
}

/**
  Remove the timeout from the active timer queue and push it onto
  the stack of currently running callbacks.

  This becomes the current() timeout which can be used in
  Fl::repeat_timeout().

  \see Fl_Timeout::current()
*/
void Fl_Timeout::make_current() {
  // printf("[%4d] Fl_Timeout::make_current(%p)\n", __LINE__, this);
  // remove the timer entry from the active timer queue
  for (Fl_Timeout** p = &first_timeout; *p;) {
    Fl_Timeout* t = *p;
    if (t == this) {
      *p = t->next;
      // push it to the current timer stack
      t->next = current_timeout;
      current_timeout = t;
      break;
    } else {
      p = &(t->next);
    }
  }
}

/**
  Remove the top-most timeout from the stack of currently running
  timeout callbacks and insert it into the list of free timers.

  Typical code in the library would look like:
  \code
    // The timeout \p Fl_Timeout *t has exired, run its callback
    t->make_current();
    (t->callback)(t->data);
    t->release();
  \endcode
*/
void Fl_Timeout::release() {
  Fl_Timeout *t = current_timeout;
  if (t) {

    // The first timer in the "current" list *should* be 'this' but we
    // check it to be sure. Issue an error message which should never appear.
    // If it would happen we'd remove the wrong timer from the current timer
    // list. This is not good but it doesn't really do harm.

    if (t != this) {
      Fl::error("*** Fl_Timeout::release() *** timer t (%p) != this (%p)\n", t, this);
    }

    // remove the timer from the list
    current_timeout = t->next;
  }
  // put the timer into the list of free timers
  next = free_timeout;
  free_timeout = this;
}

/**
  Returns the first (top-most) timeout from the current timeout stack.

  This returns a pointer to the timeout but does not remove it from the
  list of current timeouts. This should be the timeout that is currently
  executing its callback.

  \return   Fl_Timeout*   The current timeout whose callback is running.
  \retval   NULL          if no callback is currently running.
*/
Fl_Timeout *Fl_Timeout::current() {
  return current_timeout;
}

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

Fl_Timeout *Fl_Timeout::get(double time, Fl_Timeout_Handler cb, void *data) {

  Fl_Timeout *t = (Fl_Timeout *)free_timeout;
  if (t) {
    free_timeout = t->next;
    t->next = 0;
  } else {
    t = new Fl_Timeout;
#if FL_TIMEOUT_DEBUG
    num_timers++;                 // DEBUG: count allocated timers
#endif
  }

  t->next = 0;
  t->skip = 1;          // see do_timeouts() (issue #450)
  t->delay(time);
  t->callback = cb;
  t->data = data;
  return t;
}

/**
  Elapse all timers w/o calling their callbacks.

  All timer values are adjusted by the delta time since the last call.
  This method does \b NOT call timer callbacks if timers are expired.

  This must be called before new timers are added to the timer queue to make
  sure that the next timer decrement does not count down too much time.

  \see Fl_Timeout::do_timeouts()
*/
void Fl_Timeout::elapse_timeouts() {
  double elapsed = elapsed_time();
  // printf("elapse_timeouts: elapsed = %9.6f\n", double(elapsed)/1000000.);

  if (elapsed > 0.0) {

    // active timers

    for (Fl_Timeout* t = first_timeout; t; t = t->next) {
      t->time -= elapsed;
    }

    // "current" timers, i.e. timers being serviced

    for (Fl_Timeout* t = current_timeout; t; t = t->next) {
      t->time -= elapsed;
    }
  }
}

/**
  Elapse timers and call their callbacks if any timers are expired.
*/
void Fl_Timeout::do_timeouts() {

  // Reset "skip" flag for existing timers (issue #450).
  // For timers inserted in timer callbacks 'skip' will be true (1)

  Fl_Timeout *t = first_timeout;
  while (t) {
    t->skip = 0;
    t = t->next;
  }

  if (first_timeout) {
    Fl_Timeout::elapse_timeouts();
    while ((t = first_timeout)) {
      if (t->time > 0) break;

      // skip timers inserted during timeout handling (issue #450)
      while (t && t->skip)
        t = t->next;
      if (!t || t->time > 0) break;

      // make this timeout the "current" timeout
      t->make_current();
      // now it is safe for the callback to do add_timeout:
      t->callback(t->data);
      // release the timer entry
      t->release();

      // Elapse timers (again) because the callback may have used a
      // significant amount of time. This is optional though.

      Fl_Timeout::elapse_timeouts();
    }
  }
}

/**
  Returns the delay in seconds until the next timer expires,
  limited by \p ttw.

  This function calculates the time to wait for the FLTK event queue
  processing, depending on the given value \p ttw.

  If at least one timer is active and its timeout value is smaller than
  \p ttw then this value is returned. Fl::wait() will wait no longer than
  until the next timer expires.

  If no timer is active this returns the input value \p ttw unchanged.

  If at least one timer is expired this returns 0.0 so the event processing
  does not wait.

  \param[in]  ttw   time to wait from Fl::wait() etc. (upper limit)

  \return  delay until next timeout or 0.0 (see description)
*/
double Fl_Timeout::time_to_wait(double ttw) {
  Fl_Timeout *t = first_timeout;
  if (!t) return ttw;
  double tdelay = t->delay();
if (tdelay < 0.0)
    return 0.0;
  if (tdelay < ttw)
    return tdelay;
  return ttw;
}


// Write some statistics to stdout for debugging

#if FL_TIMEOUT_DEBUG

void Fl_Timeout::debug(int level) {

  printf("\nFl_Timeout::debug: number of allocated timers = %d\n", num_timers);

  int active = 0;
  Fl_Timeout *t = first_timeout;
  while (t) {
    active++;
    t = t->next;
  }

  int current = 0;
  t = current_timeout;
  while (t) {
    current++;
    t = t->next;
  }

  int free = 0;
  t = free_timeout;
  while (t) {
    free++;
    t = t->next;
  }

  printf("Fl_Timeout::debug: active: %d, current: %d, free: %d\n\n", active, current, free);

  t = first_timeout;
  int n = 0;
  while (t) {
    printf("Active timer %3d: time = %10.6f sec\n", n+1, t->delay());
    t = t->next;
    n++;
  }
} // Fl_Timeout::debug(int)

#endif // FL_TIMEOUT_DEBUG
