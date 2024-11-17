//
// Main event handling code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2024 by Bill Spitzak and others.
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

/** \file src/Fl.cxx
  Implementation of the member functions of class Fl.
*/

#include <FL/Fl.H>
#include <FL/platform.H>
#include "Fl_Screen_Driver.H"
#include "Fl_Window_Driver.H"
#include "Fl_System_Driver.H"
#include "Fl_Timeout.h"
#include <FL/Fl_Window.H>
#include <FL/Fl_Tooltip.H>
#include <FL/fl_draw.H>

#include <ctype.h>
#include <stdlib.h>
#include "flstring.h"

#if defined(DEBUG) || defined(DEBUG_WATCH)
#  include <stdio.h>
#endif // DEBUG || DEBUG_WATCH

//
// Globals...
//

Fl_Widget *fl_selection_requestor;

#ifndef FL_DOXYGEN
Fl_Widget       *Fl::belowmouse_,
                *Fl::pushed_,
                *Fl::focus_,
                *Fl::selection_owner_;
int             Fl::damage_,
                Fl::e_number,
                Fl::e_x,
                Fl::e_y,
                Fl::e_x_root,
                Fl::e_y_root,
                Fl::e_dx,
                Fl::e_dy,
                Fl::e_state,
                Fl::e_clicks,
                Fl::e_is_click,
                Fl::e_keysym,
                Fl::e_original_keysym,
                Fl::scrollbar_size_ = 16,
                Fl::menu_linespacing_ = 4;      // 4: was a local macro in Fl_Menu.cxx called "LEADING"

char            *Fl::e_text = (char *)"";
int             Fl::e_length;
const char      *Fl::e_clipboard_type = "";
void            *Fl::e_clipboard_data = NULL;

Fl_Event_Dispatch Fl::e_dispatch = 0;
Fl_Callback_Reason Fl::callback_reason_ = FL_REASON_UNKNOWN;

unsigned char   Fl::options_[] = { 0, 0 };
unsigned char   Fl::options_read_ = 0;

int             Fl::selection_to_clipboard_ = 0;

Fl_Window       *fl_xfocus = NULL; // which window X thinks has focus
Fl_Window       *fl_xmousewin;     // which window X thinks has FL_ENTER
Fl_Window       *Fl::grab_;        // most recent Fl::grab()
Fl_Window       *Fl::modal_;       // topmost modal() window

#endif // FL_DOXYGEN

char const * const Fl::clipboard_plain_text = "text/plain";
char const * const Fl::clipboard_image = "image";


//
// Drivers
//

/** Returns a pointer to the unique Fl_Screen_Driver object of the platform */
Fl_Screen_Driver *Fl::screen_driver()
{
  static  Fl_Screen_Driver* screen_driver_ = Fl_Screen_Driver::newScreenDriver();
  return screen_driver_;
}

/** Returns a pointer to the unique Fl_System_Driver object of the platform */
Fl_System_Driver *Fl::system_driver()
{
  if (!Fl_Screen_Driver::system_driver) {
    Fl_Screen_Driver::system_driver = Fl_System_Driver::newSystemDriver();
  }
  return Fl_Screen_Driver::system_driver;
}

//
// 'Fl::version()' - Return the API version number...
//

/**
  Returns the compiled-in value of the FL_VERSION constant. This
  is useful for checking the version of a shared library.

  \deprecated   Use int Fl::api_version() instead.
*/
double Fl::version() {
  return FL_VERSION;
}

/**
  Returns the compiled-in value of the FL_API_VERSION constant.
  This is useful for checking the version of a shared library.
*/
int Fl::api_version() {
  return FL_API_VERSION;
}

/**
  Returns the compiled-in value of the FL_ABI_VERSION constant.
  This is useful for checking the version of a shared library.
*/
int Fl::abi_version() {
  return FL_ABI_VERSION;
}

/**
  Gets the default scrollbar size used by
  Fl_Browser_,
  Fl_Help_View,
  Fl_Scroll, and
  Fl_Text_Display widgets.
  \returns The default size for widget scrollbars, in pixels.
*/
int Fl::scrollbar_size() {
  return scrollbar_size_;
}

/**
  Sets the default scrollbar size that is used by the
  Fl_Browser_,
  Fl_Help_View,
  Fl_Scroll, and
  Fl_Text_Display widgets.
  \param[in] W The new default size for widget scrollbars, in pixels.
*/
void Fl::scrollbar_size(int W) {
  scrollbar_size_ = W;
}

/**
  Gets the default line spacing used by menus.
  \returns The default line spacing, in pixels.
  \since 1.4.0
*/
int Fl::menu_linespacing() {
  return menu_linespacing_;
}

/**
  Sets the default line spacing used by menus.
  Default is 4.
  \param[in] H The new default line spacing between menu items, in pixels.
  \since 1.4.0
*/
void Fl::menu_linespacing(int H) {
  menu_linespacing_ = H;
}


/** Returns whether or not the mouse event is inside the given rectangle.

    Returns non-zero if the current Fl::event_x() and Fl::event_y()
    put it inside the given arbitrary bounding box.

    You should always call this rather than doing your own comparison
    so you are consistent about edge effects.

    To find out, whether the event is inside a child widget of the
    current window, you can use Fl::event_inside(const Fl_Widget *).

    \param[in] xx,yy,ww,hh      bounding box
    \return                     non-zero, if mouse event is inside
*/
int Fl::event_inside(int xx,int yy,int ww,int hh) /*const*/ {
  int mx = e_x - xx;
  int my = e_y - yy;
  return (mx >= 0 && mx < ww && my >= 0 && my < hh);
}

/** Returns whether or not the mouse event is inside a given child widget.

    Returns non-zero if the current Fl::event_x() and Fl::event_y()
    put it inside the given child widget's bounding box.

    This method can only be used to check whether the mouse event is
    inside a \b child widget of the window that handles the event, and
    there must not be an intermediate subwindow (i.e. the widget must
    not be inside a subwindow of the current window). However, it is
    valid if the widget is inside a nested Fl_Group.

    You must not use it with the window itself as the \p o argument
    in a window's handle() method.

    \note The mentioned restrictions are necessary, because this method
    does not transform coordinates of child widgets, and thus the given
    widget \p o must be within the \e same window that is handling the
    current event. Otherwise the results are undefined.

    You should always call this rather than doing your own comparison
    so you are consistent about edge effects.

    \see Fl::event_inside(int, int, int, int)

    \param[in] o        child widget to be tested
    \return             non-zero, if mouse event is inside the widget
*/
int Fl::event_inside(const Fl_Widget *o) /*const*/ {
  int mx = e_x - o->x();
  int my = e_y - o->y();
  return (mx >= 0 && mx < o->w() && my >= 0 && my < o->h());
}

//
// Cross-platform timer support
//
// User (doxygen) documentation is in this file but the implementation
// of all functions is in class Fl_Timeout in Fl_Timeout.cxx.

/**
  Adds a one-shot timeout callback.

  The callback function \p cb will be called by Fl::wait() at \p time seconds
  after this function is called.
  The callback function must have the signature \ref Fl_Timeout_Handler.
  The optional \p data argument is passed to the callback (default: NULL).

  The timer is removed from the timer queue before the callback function is
  called. It is safe to reschedule the timeout inside the callback function.

  You can have multiple timeout callbacks, even the same timeout callback
  with different timeout values and/or different \p data values. They are
  all considered different timer objects.

  To remove a timeout while it is active (pending) use Fl::remove_timeout().

  If you need more accurate, repeated timeouts, use Fl::repeat_timeout() to
  reschedule the subsequent timeouts. Please see Fl::repeat_timeout() for
  an example.

  Since version 1.4, a timeout can be started from a child thread under the
  condition that the call to Fl::add_timeout is wrapped in Fl::lock() and Fl::unlock().

  \param[in]  time    delta time in seconds until the timer expires
  \param[in]  cb      callback function
  \param[in]  data    optional user data (default: \p NULL)

  \see Fl_Timeout_Handler
  \see Fl::repeat_timeout(double time, Fl_Timeout_Handler cb, void *data)
  \see Fl::remove_timeout(Fl_Timeout_Handler cb, void *data)
  \see Fl::has_timeout(Fl_Timeout_Handler cb, void *data)

*/
void Fl::add_timeout(double time, Fl_Timeout_Handler cb, void *data) {
  Fl_Timeout::add_timeout(time, cb, data);
}

/**
  Repeats a timeout callback from the expiration of the previous timeout,
  allowing for more accurate timing.

  You should call this method only inside a timeout callback of the same or
  a logically related timer from whose expiration time the new timeout shall
  be scheduled. Otherwise the timing accuracy can't be improved and the
  exact behavior is undefined.

  If you call this outside a timeout callback the behavior is the same as
  Fl::add_timeout().

  Example: The following code will print "TICK" each second on stdout with
  a fair degree of accuracy:

  \code
    #include <FL/Fl.H>
    #include <FL/Fl_Window.H>
    #include <stdio.h>

    void callback(void *) {
      printf("TICK\n");
      Fl::repeat_timeout(1.0, callback); // retrigger timeout
    }

    int main() {
      Fl_Window win(100, 100);
      win.show();
      Fl::add_timeout(1.0, callback); // set up first timeout
      return Fl::run();
    }
  \endcode

  \param[in]  time    delta time in seconds until the timer expires
  \param[in]  cb      callback function
  \param[in]  data    optional user data (default: \p NULL)
*/
void Fl::repeat_timeout(double time, Fl_Timeout_Handler cb, void *data) {
  Fl_Timeout::repeat_timeout(time, cb, data);
}

/**
  Returns true if the timeout exists and has not been called yet.

  Both arguments \p cb and \p data must match with at least one timer
  in the queue of active timers to return true (1).

  \note It is a known inconsistency that Fl::has_timeout() does not use
    the \p data argument as a wildcard (match all) if it is zero (NULL)
    which Fl::remove_timeout() does.
    This is so for backwards compatibility with FLTK 1.3.x.
    Therefore using 0 (zero, NULL) as the timeout \p data value is discouraged
    unless you're sure that you don't need to use
    <kbd>Fl::has_timeout(callback, (void *)0);</kbd> or
    <kbd>Fl::remove_timeout(callback, (void *)0);</kbd>.

  \param[in]  cb    Timer callback
  \param[in]  data  User data

  \returns      whether the timer was found in the queue
  \retval   0   not found
  \retval   1   found
*/
int Fl::has_timeout(Fl_Timeout_Handler cb, void *data) {
  return Fl_Timeout::has_timeout(cb, data);
}

/**
  Remove one or more matching timeout callbacks from the timer queue.

  This method removes \b all matching timeouts, not just the first one.

  If the \p data argument is \p NULL (the default!) only the callback
  \p cb must match, i.e. all timer entries with this callback are removed.

  It is harmless to remove a timeout callback that no longer exists.

  If you want to remove only the next matching timeout you can use
  Fl::remove_next_timeout(Fl_Timeout_Handler cb, void *data, void **data_return)
  (available since FLTK 1.4.0).

  \param[in]  cb    Timer callback to be removed (must match)
  \param[in]  data  Wildcard if NULL (default), must match otherwise

  \see Fl::remove_next_timeout(Fl_Timeout_Handler cb, void *data, void **data_return)
*/
void Fl::remove_timeout(Fl_Timeout_Handler cb, void *data) {
  Fl_Timeout::remove_timeout(cb, data);
}


/**
  Remove the next matching timeout callback and return its \p data pointer.

  This method removes only the next matching timeout and returns in
  \p data_return (if non-NULL) the \p data member given when the timeout
  was scheduled.

  This method is useful if you remove a timeout before it is scheduled
  and you need to get and use its data value, for instance to free() or
  delete the data associated with the timeout.

  This method returns non-zero if a matching timeout was found and zero
  if no timeout matched the request.

  If the return value is \c N \> 1 then there are N - 1 more matching
  timeouts pending.

  If you need to remove all timeouts with a particular callback \p cb
  you must repeat this call until it returns 1 (all timeouts removed)
  or zero (no matching timeout), whichever occurs first.


  \param[in]    cb    Timer callback to be removed (must match)
  \param[in]    data  Wildcard if NULL, must match otherwise
  \param[inout] data_return  Pointer to (void *) to receive the data value

  \return       non-zero if a timer was found and removed
  \retval   0   no matching timer was found
  \retval   1   the last matching timeout was found and removed
  \retval  N>1  a matching timeout was removed and there are \n
                (N - 1) matching timeouts pending

  \see Fl::remove_timeout(Fl_Timeout_Handler cb, void *data)

  \since 1.4.0
*/
int Fl::remove_next_timeout(Fl_Timeout_Handler cb, void *data, void **data_return) {
  return Fl_Timeout::remove_next_timeout(cb, data, data_return);
}


////////////////////////////////////////////////////////////////
// Checks are just stored in a list. They are called in the reverse
// order that they were added (this may change in the future).
// This is a bit messy because I want to allow checks to be added,
// removed, and have wait() called from inside them. To do this
// next_check points at the next unprocessed one for the outermost
// call to Fl::wait().

struct Check {
  void (*cb)(void*);
  void* arg;
  Check* next;
};
static Check *first_check, *next_check, *free_check;

/**
  FLTK will call this callback just before it flushes the display and
  waits for events.  This is different than an idle callback because it
  is only called once, then FLTK calls the system and tells it not to
  return until an event happens.

  This can be used by code that wants to monitor the
  application's state, such as to keep a display up to date. The
  advantage of using a check callback is that it is called only when no
  events are pending. If events are coming in quickly, whole blocks of
  them will be processed before this is called once. This can save
  significant time and avoid the application falling behind the events.

  Sample code:

  \code
  bool state_changed; // anything that changes the display turns this on

  void callback(void*) {
   if (!state_changed) return;
   state_changed = false;
   do_expensive_calculation();
   widget->redraw();
  }

  main() {
   Fl::add_check(callback);
   return Fl::run();
  }
  \endcode
*/
void Fl::add_check(Fl_Timeout_Handler cb, void *argp) {
  Check* t = free_check;
  if (t) free_check = t->next;
  else t = new Check;
  t->cb = cb;
  t->arg = argp;
  t->next = first_check;
  if (next_check == first_check) next_check = t;
  first_check = t;
}

/**
  Removes a check callback. It is harmless to remove a check
  callback that no longer exists.
*/
void Fl::remove_check(Fl_Timeout_Handler cb, void *argp) {
  for (Check** p = &first_check; *p;) {
    Check* t = *p;
    if (t->cb == cb && t->arg == argp) {
      if (next_check == t) next_check = t->next;
      *p = t->next;
      t->next = free_check;
      free_check = t;
    } else {
      p = &(t->next);
    }
  }
}

/**
  Returns 1 if the check exists and has not been called yet, 0 otherwise.
*/
int Fl::has_check(Fl_Timeout_Handler cb, void *argp) {
  for (Check** p = &first_check; *p;) {
    Check* t = *p;
    if (t->cb == cb && t->arg == argp) {
      return 1;
    } else {
      p = &(t->next);
    }
  }
  return 0;
}

void Fl::run_checks()
{
  // checks are a bit messy so that add/remove and wait may be called
  // from inside them without causing an infinite loop:
  if (next_check == first_check) {
    while (next_check) {
      Check* checkp = next_check;
      next_check = checkp->next;
      (checkp->cb)(checkp->arg);
    }
    next_check = first_check;
  }
}


////////////////////////////////////////////////////////////////
// Clipboard notifications

struct Clipboard_Notify {
  Fl_Clipboard_Notify_Handler handler;
  void *data;
  struct Clipboard_Notify *next;
};

static struct Clipboard_Notify *clip_notify_list = NULL;

void Fl::add_clipboard_notify(Fl_Clipboard_Notify_Handler h, void *data) {
  struct Clipboard_Notify *node;

  remove_clipboard_notify(h);

  node = new Clipboard_Notify;

  node->handler = h;
  node->data = data;
  node->next = clip_notify_list;

  clip_notify_list = node;

  Fl::screen_driver()->clipboard_notify_change();
}

void Fl::remove_clipboard_notify(Fl_Clipboard_Notify_Handler h) {
  struct Clipboard_Notify *node, **prev;

  node = clip_notify_list;
  prev = &clip_notify_list;
  while (node != NULL) {
    if (node->handler == h) {
      *prev = node->next;
      delete node;

      Fl::screen_driver()->clipboard_notify_change();

      return;
    }

    prev = &node->next;
    node = node->next;
  }
}

bool fl_clipboard_notify_empty(void) {
  return clip_notify_list == NULL;
}

void fl_trigger_clipboard_notify(int source) {
  struct Clipboard_Notify *node, *next;

  node = clip_notify_list;
  while (node != NULL) {
    next = node->next;
    node->handler(source, node->data);
    node = next;
  }
}

////////////////////////////////////////////////////////////////
// idle/wait/run/check/ready:

void (*Fl::idle)(); // see Fl::add_idle.cxx for the add/remove functions

/*
  Private, undocumented method to run idle callbacks.

  FLTK guarantees that idle callbacks will never be called recursively,
  i.e. while an idle callback is being executed.

  This method should (must) be the only way to run idle callbacks to
  ensure that the `in_idle' flag is respected.

  Idle callbacks are executed whenever Fl::wait() is called and no events
  are waiting to be serviced.

  If Fl::idle is set (non-NULL) this points at a function that executes
  the first idle callback and appends it to the end of the list of idle
  callbacks. For details see static function call_idle() in Fl_add_idle.cxx.

  If it is NULL then no idle callbacks are active and Fl::run_idle() returns
  immediately.

  Note: idle callbacks can be queued in nested FLTK event loops like
  ```
    while (win->shown())
      Fl::wait();
  ```
  if an event (timeout or button click etc.) handler calls Fl::add_idle()
  or even in Fl::flush() if a draw() method calls Fl::add_idle().
*/
void Fl::run_idle() {
  static char in_idle;
  if (Fl::idle && !in_idle) {
    in_idle = 1;
    Fl::idle();
    in_idle = 0;
  }
}

/**
 Waits a maximum of \p time_to_wait seconds or until "something happens".

 See Fl::wait() for the description of operations performed when
 "something happens".
 \return Always 1 on Windows. Otherwise, it is positive
 if an event or fd happens before the time elapsed.
 It is zero if nothing happens.  It is negative if an error
 occurs (this will happen on X11 if a signal happens).
*/
double Fl::wait(double time_to_wait) {
  return system_driver()->wait(time_to_wait);
}

#define FOREVER 1e20

/**
  Calls Fl::wait()repeatedly as long as any windows are displayed.
  When all the windows are closed it returns zero
  (supposedly it would return non-zero on any errors, but FLTK calls
  exit directly for these).  A normal program will end main()
  with return Fl::run();.

  \note Fl::run() and Fl::wait() (but not Fl::wait(double)) both
  return when all FLTK windows are closed. Therefore, a MacOS FLTK
  application possessing Fl_Sys_Menu_Bar items able to create new windows
  and expected to keep running without any open window cannot use
  these two functions. One solution is to run the event loop as follows:
  \code    while (!Fl::program_should_quit()) Fl::wait(1e20); \endcode
*/
int Fl::run() {
  while (Fl_X::first) wait(FOREVER);
  return 0;
}

/**
  Waits until "something happens" and then returns.  Call this
  repeatedly to "run" your program.  You can also check what happened
  each time after this returns, which is quite useful for managing
  program state.

  What this really does is call all idle callbacks, all elapsed
  timeouts, call Fl::flush() to get the screen to update, and
  then wait some time (zero if there are idle callbacks, the shortest of
  all pending timeouts, or infinity), for any events from the user or
  any Fl::add_fd() callbacks.  It then handles the events and
  calls the callbacks and then returns.

  \return non-zero if there are any
  visible windows - this may change in future versions of FLTK.
*/
int Fl::wait() {
  if (!Fl_X::first) return 0;
  wait(FOREVER);
  return Fl_X::first != 0; // return true if there is a window
}

/**
  Same as Fl::wait(0).  Calling this during a big calculation
  will keep the screen up to date and the interface responsive:

  \code
  while (!calculation_done()) {
    calculate();
    Fl::check();
    if (user_hit_abort_button()) break;
  }
  \endcode

  This returns non-zero if any windows are displayed, and 0 if no
  windows are displayed (this is likely to change in future versions of
  FLTK).
*/
int Fl::check() {
  wait(0.0);
  return Fl_X::first != 0; // return true if there is a window
}

/**
  This is similar to Fl::check() except this does \e not
  call Fl::flush() or any callbacks, which is useful if your
  program is in a state where such callbacks are illegal.  This returns
  true if Fl::check() would do anything (it will continue to
  return true until you call Fl::check() or Fl::wait()).

  \code
  while (!calculation_done()) {
    calculate();
    if (Fl::ready()) {
      do_expensive_cleanup();
      Fl::check();
      if (user_hit_abort_button()) break;
    }
  }
  \endcode
*/
int Fl::ready()
{
  return system_driver()->ready();
}

/** Hide all visible windows to make FLTK leave Fl::run().

  Fl:run() will run as long as there are visible windows.
  Call Fl::hide_all_windows() to hide (close) all currently shown
  (visible) windows, effectively terminating the Fl::run() loop.
  \see Fl::run()
  \since 1.4.0
*/
void Fl::hide_all_windows() {
  while (Fl::first_window()) {
    Fl::first_window()->hide();
  }
}

int Fl::program_should_quit_ = 0;

////////////////////////////////////////////////////////////////
// Window list management:

#ifndef FL_DOXYGEN
Fl_X* Fl_X::first;
#endif

/** Returns the Fl_Window that corresponds to the given window reference,
  or \c NULL if not found.
  \deprecated Kept in the X11, Windows, and macOS platforms for compatibility
    with FLTK versions before 1.4.
    Please use fl_x11_find(Window), fl_wl_find(struct wld_window*),
    fl_win32_find(HWND) or fl_mac_find(FLWindow*) with FLTK 1.4.0 and above.
*/
Fl_Window* fl_find(Window xid) {
  return Fl_Window_Driver::find((fl_uintptr_t)xid);
}

/**
  Returns the first top-level window in the list of shown() windows.  If
  a modal() window is shown this is the top-most modal window, otherwise
  it is the most recent window to get an event.
*/
Fl_Window* Fl::first_window() {
  Fl_X* i = Fl_X::first;
  return i ? i->w : 0;
}

/**
  Returns the next top-level window in the list of shown() windows.
  You can use this call to iterate through all the windows that are shown().
  \param[in] window must be shown and not NULL
*/
Fl_Window* Fl::next_window(const Fl_Window* window) {
  Fl_X* i = window ? Fl_X::flx(window) : 0;
  if (!i) {
    Fl::error("Fl::next_window() failed: window (%p) not shown.", window);
    return 0;
  }
  i = i->next;
  return i ? i->w : 0;
}

/**
 Sets the window that is returned by first_window().
 The window is removed from wherever it is in the
 list and inserted at the top.  This is not done if Fl::modal()
 is on or if the window is not shown(). Because the first window
 is used to set the "parent" of modal windows, this is often
 useful.
 */
void Fl::first_window(Fl_Window* window) {
  if (!window || !window->shown()) return;
  Fl_Window_Driver::find( Fl_X::flx(window)->xid );
}

/**
  Redraws all widgets.
*/
void Fl::redraw() {
  for (Fl_X* i = Fl_X::first; i; i = i->next) i->w->redraw();
}

/**
  Causes all the windows that need it to be redrawn and graphics forced
  out through the pipes.

  This is what wait() does before looking for events.

  Note: in multi-threaded applications you should only call Fl::flush()
  from the main thread. If a child thread needs to trigger a redraw event,
  it should instead call Fl::awake() to get the main thread to process the
  event queue.
*/
void Fl::flush() {
  if (damage()) {
    damage_ = 0;
    for (Fl_X* i = Fl_X::first; i; i = i->next) {
      Fl_Window* wi = i->w;
      if (Fl_Window_Driver::driver(wi)->wait_for_expose_value) {damage_ = 1; continue;}
      if (!wi->visible_r()) continue;
      if (wi->damage()) {
        Fl_Window_Driver::driver(wi)->flush();
        wi->clear_damage();
      }
      // destroy damage regions for windows that don't use them:
      if (i->region) {
        fl_graphics_driver->XDestroyRegion(i->region);
        i->region = 0;
      }
    }
  }
  screen_driver()->flush();
}


////////////////////////////////////////////////////////////////
// Event handlers:


struct handler_link {
  int (*handle)(int);
  handler_link *next;
};


static handler_link *handlers = 0;


/**
  Install a function to parse unrecognized events.  If FLTK cannot
  figure out what to do with an event, it calls each of these functions
  (most recent first) until one of them returns non-zero.  If none of
  them returns non-zero then the event is ignored.  Events that cause
  this to be called are:

  - \ref FL_SHORTCUT events that are not recognized by any widget.
    This lets you provide global shortcut keys.
  - \ref FL_SCREEN_CONFIGURATION_CHANGED events.
    Under X11, this event requires the libXrandr.so shared library to be
    loadable at run-time and the X server to implement the RandR extension.
  - \ref FL_ZOOM_EVENT events.
  - System events that FLTK does not recognize.  See fl_xevent.
  - \e Some other events when the widget FLTK selected returns
    zero from its handle() method.  Exactly which ones may change
    in future versions, however.

 \see Fl::remove_handler(Fl_Event_Handler)
 \see Fl::event_dispatch(Fl_Event_Dispatch d)
 \see Fl::handle(int, Fl_Window*)
*/
void Fl::add_handler(Fl_Event_Handler ha) {
  handler_link *l = new handler_link;
  l->handle = ha;
  l->next = handlers;
  handlers = l;
}


/** Returns the last function installed by a call to Fl::add_handler().

  \since 1.4.0
*/
Fl_Event_Handler Fl::last_handler() {
  return handlers ? handlers->handle : NULL;
}


/** Install a function to parse unrecognized events with less priority than another function.
  Install function \p ha to handle unrecognized events
  giving it the priority just lower than that of function \p before
  which was previously installed.
  \see Fl::add_handler(Fl_Event_Handler)
  \see Fl::last_handler()
  \since 1.4.0
*/
void Fl::add_handler(Fl_Event_Handler ha, Fl_Event_Handler before) {
  if (!before) return Fl::add_handler(ha);
  handler_link *l = handlers;
  while (l) {
    if (l->handle == before) {
      handler_link *p = l->next, *q = new handler_link;
      q->handle = ha;
      q->next = p;
      l->next = q;
      return;
    }
    l = l->next;
  }
}

/**
 Removes a previously added event handler.
 \see Fl::handle(int, Fl_Window*)
*/
void Fl::remove_handler(Fl_Event_Handler ha) {
  handler_link *l, *p;

  // Search for the handler in the list...
  for (l = handlers, p = 0; l && l->handle != ha; p = l, l = l->next) {
    /* empty */
  }

  if (l) {
    // Found it, so remove it from the list...
    if (p) p->next = l->next;
    else handlers = l->next;

    // And free the record...
    delete l;
  }
}

int (*fl_local_grab)(int); // used by fl_dnd.cxx

static int send_handlers(int e) {
  for (const handler_link *hl = handlers; hl; hl = hl->next)
    if (hl->handle(e)) return 1;
  return 0;
}


////////////////////////////////////////////////////////////////
// System event handlers:


struct system_handler_link {
  Fl_System_Handler handle;
  void *data;
  system_handler_link *next;
};


static system_handler_link *sys_handlers = 0;


/**
 \brief Install a function to intercept system events.

 FLTK calls each of these functions as soon as a new system event is
 received. The processing will stop at the first function to return
 non-zero. If all functions return zero then the event is passed on
 for normal handling by FLTK.

 Each function will be called with a pointer to the system event as
 the first argument and \p data as the second argument. The system
 event pointer will always be void *, but will point to different
 objects depending on the platform:
   - X11: XEvent
   - Windows: MSG
   - OS X: NSEvent
   - Wayland: NULL (FLTK runs the event handler(s) just before calling \e wl_display_dispatch())

 \param ha The event handler function to register
 \param data User data to include on each call

 \see Fl::remove_system_handler(Fl_System_Handler)
*/
void Fl::add_system_handler(Fl_System_Handler ha, void *data) {
  system_handler_link *l = new system_handler_link;
  l->handle = ha;
  l->data = data;
  l->next = sys_handlers;
  sys_handlers = l;
}


/**
 Removes a previously added system event handler.

 \param ha The event handler function to remove

 \see Fl::add_system_handler(Fl_System_Handler)
*/
void Fl::remove_system_handler(Fl_System_Handler ha) {
  system_handler_link *l, *p;

  // Search for the handler in the list...
  for (l = sys_handlers, p = 0; l && l->handle != ha; p = l, l = l->next) {
    /* empty */
  }

  if (l) {
    // Found it, so remove it from the list...
    if (p) p->next = l->next;
    else sys_handlers = l->next;

    // And free the record...
    delete l;
  }
}

int fl_send_system_handlers(void *e) {
  for (const system_handler_link *hl = sys_handlers; hl; hl = hl->next) {
    if (hl->handle(e, hl->data))
      return 1;
  }
  return 0;
}


////////////////////////////////////////////////////////////////

Fl_Widget* fl_oldfocus; // kludge for Fl_Group...

/**
    Sets the widget that will receive FL_KEYBOARD events.

    Use this function inside the \c handle(int) member function of a widget of yours
    to give focus to the widget, for example when it receives the FL_FOCUS or the FL_PUSH event.
    Otherwise, use Fl_Widget::take_focus() to give focus to a widget;

    If you change Fl::focus(), the previous widget and all
    parents (that don't contain the new widget) are sent FL_UNFOCUS
    events.  Changing the focus does \e not send FL_FOCUS to
    this or any widget, because sending FL_FOCUS is supposed to
    \e test if the widget wants the focus (by it returning non-zero from
    handle()).

    Since FLTK 1.4.0 widgets can set the NEEDS_KEYBOARD flag to indicate that
    a keyboard is essential for the widget to function. Touchscreen devices
    will be sent a request to show an on-screen keyboard if no hardware keyboard
    is connected.

    \see Fl_Widget::take_focus()
    \see Fl_Widget::needs_keyboard() const
    \see Fl_Widget::needs_keyboard(bool)
*/
void Fl::focus(Fl_Widget *o)
{
  if (grab()) return; // don't do anything while grab is on

  // request an on-screen keyboard on touch screen devices if needed
  Fl_Widget *prevFocus = Fl::focus();
  char hideKeyboard = (prevFocus && prevFocus->needs_keyboard());
  char showKeyboard = (o && o->needs_keyboard());
  if (hideKeyboard && !showKeyboard)
    Fl::screen_driver()->release_keyboard();
  if (showKeyboard && !hideKeyboard)
    Fl::screen_driver()->request_keyboard();

  if (o && !o->visible_focus()) return;
  Fl_Widget *p = focus_;
  if (o != p) {
    Fl::compose_reset();
    focus_ = o;
    // make sure that fl_xfocus is set to the top level window
    // of this widget, or fl_fix_focus will clear our focus again
    if (o) {
      Fl_Window *win = 0, *w1 = o->as_window();
      if (!w1) w1 = o->window();
      while (w1) { win=w1; w1=win->window(); }
      if (win) {
        if (fl_xfocus != win) {
          Fl_Window_Driver::driver(win)->take_focus();
          fl_xfocus = win;
        }
      }
    }
    // take focus from the old focused window
    fl_oldfocus = 0;
    int old_event = e_number;
    e_number = FL_UNFOCUS;
    for (; p; p = p->parent()) {
      p->handle(FL_UNFOCUS);
      fl_oldfocus = p;
    }
    e_number = old_event;
  }
}

static char dnd_flag = 0; // make 'belowmouse' send DND_LEAVE instead of LEAVE

/**
    Sets the widget that is below the mouse.  This is for
    highlighting buttons.  It is not used to send FL_PUSH or
    FL_MOVE directly, for several obscure reasons, but those events
    typically go to this widget.  This is also the first widget tried for
    FL_SHORTCUT events.

    If you change the belowmouse widget, the previous one and all
    parents (that don't contain the new widget) are sent FL_LEAVE
    events.  Changing this does \e not send FL_ENTER to this
    or any widget, because sending FL_ENTER is supposed to \e test
    if the widget wants the mouse (by it returning non-zero from
    handle()).
*/
void Fl::belowmouse(Fl_Widget *o) {
  if (grab()) return; // don't do anything while grab is on
  Fl_Widget *p = belowmouse_;
  if (o != p) {
    belowmouse_ = o;
    int old_event = e_number;
    e_number = dnd_flag ? FL_DND_LEAVE : FL_LEAVE;
    for (; p && !p->contains(o); p = p->parent()) {
      p->handle(e_number);
    }
    e_number = old_event;
  }
}

/**
    Sets the widget that is being pushed. FL_DRAG or
    FL_RELEASE (and any more FL_PUSH) events will be sent to
    this widget.

    If you change the pushed widget, the previous one and all parents
    (that don't contain the new widget) are sent FL_RELEASE
    events.  Changing this does \e not send FL_PUSH to this
    or any widget, because sending FL_PUSH is supposed to \e test
    if the widget wants the mouse (by it returning non-zero from
    handle()).
*/
 void Fl::pushed(Fl_Widget *o) {
  pushed_ = o;
}

static void nothing(Fl_Widget *) {}
void (*Fl_Tooltip::enter)(Fl_Widget *) = nothing;
void (*Fl_Tooltip::exit)(Fl_Widget *) = nothing;

// Update modal(), focus() and other state according to system state,
// and send FL_ENTER, FL_LEAVE, FL_FOCUS, and/or FL_UNFOCUS events.
// This is the only function that produces these events in response
// to system activity.
// This is called whenever a window is added or hidden, and whenever
// X says the focus or mouse window have changed.

void fl_fix_focus() {
#ifdef DEBUG
  puts("fl_fix_focus();");
#endif // DEBUG

  if (Fl::grab()) return; // don't do anything while grab is on.

  // set focus based on Fl::modal() and fl_xfocus
  Fl_Widget* w = fl_xfocus;
  if (w) {
    int saved = Fl::e_keysym;
    if (Fl::e_keysym < (FL_Button + FL_LEFT_MOUSE) ||
        Fl::e_keysym > (FL_Button + FL_RIGHT_MOUSE))
      Fl::e_keysym = 0; // make sure widgets don't think a keystroke moved focus
    while (w->parent()) w = w->parent();
    if (Fl::modal()) w = Fl::modal();
    if (!w->contains(Fl::focus()))
      if (!w->take_focus()) Fl::focus(w);
    Fl::e_keysym = saved;
  } else
    Fl::focus(0);

// MRS: Originally we checked the button state, but a user reported that it
//      broke click-to-focus in FLWM?!?
//  if (!(Fl::event_state() & 0x7f00000 /*FL_BUTTONS*/)) {
  if (!Fl::pushed()) {
    // set belowmouse based on Fl::modal() and fl_xmousewin:
    w = fl_xmousewin;
    if (w) {
      if (Fl::modal()) w = Fl::modal();
      if (!w->contains(Fl::belowmouse())) {
        int old_event = Fl::e_number;
        w->handle(Fl::e_number = FL_ENTER);
        Fl::e_number = old_event;
        if (!w->contains(Fl::belowmouse())) Fl::belowmouse(w);
      } else {
        // send a FL_MOVE event so the enter/leave state is up to date
        Fl::e_x = Fl::e_x_root - fl_xmousewin->x();
        Fl::e_y = Fl::e_y_root - fl_xmousewin->y();
        int old_event = Fl::e_number;
        w->handle(Fl::e_number = FL_MOVE);
        Fl::e_number = old_event;
      }
    } else {
      Fl::belowmouse(0);
      Fl_Tooltip::enter(0);
    }
  }
}


// This function is called by ~Fl_Widget() and by Fl_Widget::deactivate()
// and by Fl_Widget::hide().  It indicates that the widget does not want
// to receive any more events, and also removes all global variables that
// point at the widget.
// I changed this from the 1.0.1 behavior, the older version could send
// FL_LEAVE or FL_UNFOCUS events to the widget.  This appears to not be
// desirable behavior and caused flwm to crash.

void fl_throw_focus(Fl_Widget *o) {
#ifdef DEBUG
  printf("fl_throw_focus(o=%p)\n", o);
#endif // DEBUG

  if (o->contains(Fl::pushed())) Fl::pushed_ = 0;
  if (o->contains(fl_selection_requestor)) fl_selection_requestor = 0;
  if (o->contains(Fl::belowmouse())) Fl::belowmouse_ = 0;
  if (o->contains(Fl::focus())) Fl::focus_ = 0;
  if (o == fl_xfocus) fl_xfocus = 0;
  if (o == Fl_Tooltip::current()) Fl_Tooltip::current(0);
  if (o == fl_xmousewin) fl_xmousewin = 0;
  Fl_Tooltip::exit(o);
  fl_fix_focus();
}

////////////////////////////////////////////////////////////////

// Find the first active_r() widget, starting at the widget wi and
// walking up the widget hierarchy to the top level window.
//
// In other words: find_active() returns an active group that contains
// the inactive widget and all inactive parent groups.
//
// This is used to send FL_SHORTCUT events to the Fl::belowmouse() widget
// in case the target widget itself is !active_r(). In this case the event
// is sent to the first active_r() parent.
//
// This prevents sending events to inactive widgets that might get the
// input focus otherwise. The search is fast and light and avoids calling
// !active_r() multiple times.
// See STR #3216.
//
// Returns: first active_r() widget "above" the widget wi or NULL if
// no widget is active. May return the top level window.

static Fl_Widget *find_active(Fl_Widget *wi) {
  Fl_Widget *found = 0;
  for (; wi; wi = wi->parent()) {
    if (wi->active()) {
      if (!found) found = wi;
    }
    else found = 0;
  }
  return found;
}

////////////////////////////////////////////////////////////////

// Call to->handle(), but first replace the mouse x/y with the correct
// values to account for nested windows. 'window' is the outermost
// window the event was posted to by the system:
static int send_event(int event, Fl_Widget* to, Fl_Window* window) {
  int dx, dy;
  int old_event = Fl::e_number;
  if (window) {
    dx = window->x();
    dy = window->y();
  } else {
    dx = dy = 0;
  }
  for (const Fl_Widget* w = to; w; w = w->parent()) {
    if (w->type() >= FL_WINDOW) {
      dx -= w->x();
      dy -= w->y();
    }
  }
  int save_x = Fl::e_x; Fl::e_x += dx;
  int save_y = Fl::e_y; Fl::e_y += dy;
  int ret = to->handle(Fl::e_number = event);
  Fl::e_number = old_event;
  Fl::e_y = save_y;
  Fl::e_x = save_x;
  return ret;
}

/**
  Give the reason for calling a callback.

  \return the reason for the current callback
  \see Fl_Widget::when(), Fl_Widget::do_callback(), Fl_Widget::callback()
  \since 1.4.0
 */
Fl_Callback_Reason Fl::callback_reason() {
  return callback_reason_;
}

/**
 \brief Set a new event dispatch function.

 The event dispatch function is called after native events are converted to
 FLTK events, but before they are handled by FLTK. If the dispatch function
 Fl_Event_Dispatch \p d is set, it is up to the dispatch function to call
 Fl::handle_(int, Fl_Window*) or to ignore the event.

 The dispatch function itself must return 0 if it ignored the event,
 or non-zero if it used the event. If you call Fl::handle_(), then
 this will return the correct value.

 The event dispatch can be used to handle exceptions in FLTK events and
 callbacks before they reach the native event handler:

 \code
 int myHandler(int e, Fl_Window *w) {
   try {
     return Fl::handle_(e, w);
   } catch () {
     ...
   }
 }

 main() {
   Fl::event_dispatch(myHandler);
   ...
   Fl::run();
 }
 \endcode

 \param d new dispatch function, or NULL
 \see Fl::add_handler(Fl_Event_Handler)
 \see Fl::handle(int, Fl_Window*)
 \see Fl::handle_(int, Fl_Window*)
 */
void Fl::event_dispatch(Fl_Event_Dispatch d)
{
  e_dispatch = d;
}


/**
 \brief Return the current event dispatch function.
 */
Fl_Event_Dispatch Fl::event_dispatch()
{
  return e_dispatch;
}


/**
 \brief Handle events from the window system.

 This is called from the native event dispatch after native events have been
 converted to FLTK notation. This function calls Fl::handle_(int, Fl_Window*)
 unless the user sets a dispatch function. If a user dispatch function is set,
 the user must make sure that Fl::handle_() is called, or the event will be
 ignored.

 \param e the event type (Fl::event_number() is not yet set)
 \param window the window that caused this event
 \return 0 if the event was not handled

 \see Fl::add_handler(Fl_Event_Handler)
 \see Fl::event_dispatch(Fl_Event_Dispatch)
 */
int Fl::handle(int e, Fl_Window* window)
{
  if (e_dispatch) {
    return e_dispatch(e, window);
  } else {
    return handle_(e, window);
  }
}


/**
 \brief Handle events from the window system.

 This function is called from the native event dispatch, unless the user sets
 another dispatch function. In that case, the user dispatch function must
 decide when to call Fl::handle_(int, Fl_Window*)

 Callbacks can set \p FL_REASON_CLOSED and \p FL_REASON_CANCELLED.

 \param e the event type (Fl::event_number() is not yet set)
 \param window the window that caused this event
 \return 0 if the event was not handled

 \see Fl::event_dispatch(Fl_Event_Dispatch)
 */
int Fl::handle_(int e, Fl_Window* window)
{
  e_number = e;
  if (fl_local_grab) return fl_local_grab(e);

  Fl_Widget* wi = window;

  switch (e) {

  case FL_CLOSE:
    if ( grab() || (modal() && window != modal()) ) return 0;
    wi->do_callback(FL_REASON_CLOSED);
    return 1;

  case FL_SHOW:
    wi->Fl_Widget::show(); // this calls Fl_Widget::show(), not Fl_Window::show()
    return 1;

  case FL_HIDE:
    wi->Fl_Widget::hide(); // this calls Fl_Widget::hide(), not Fl_Window::hide()
    return 1;

  case FL_PUSH:
#ifdef DEBUG
    printf("Fl::handle(e=%d, window=%p);\n", e, window);
#endif // DEBUG

    if (grab()) wi = grab();
    else if (modal() && wi != modal()) return 0;
    pushed_ = wi;
    Fl_Tooltip::current(wi);
    if (send_event(e, wi, window)) return 1;
    // raise windows that are clicked on:
    window->show();
    return 1;

  case FL_DND_ENTER:
  case FL_DND_DRAG:
    dnd_flag = 1;
    break;

  case FL_DND_LEAVE:
    dnd_flag = 1;
    belowmouse(0);
    dnd_flag = 0;
    return 1;

  case FL_DND_RELEASE:
    wi = belowmouse();
    break;

  case FL_MOVE:
  case FL_DRAG:
    fl_xmousewin = window; // this should already be set, but just in case.
    if (pushed()) {
      wi = pushed();
      if (grab()) wi = grab();
      e_number = e = FL_DRAG;
      break;
    }
    if (modal() && wi != modal()) wi = 0;
    if (grab()) wi = grab();
    { int ret;
      Fl_Widget* pbm = belowmouse();
      ret = (wi && send_event(e, wi, window));
      if (pbm != belowmouse()) {
#ifdef DEBUG
        printf("Fl::handle(e=%d, window=%p) -- Fl_Tooltip::enter(%p);\n", e, window, belowmouse());
#endif // DEBUG
        Fl_Tooltip::enter(belowmouse());
      }
      return ret;
    }

  case FL_RELEASE: {

    // Mouse drag release mode - Jul 14, 2023, Albrecht-S (WIP):
    //   0 = old: *first* mouse button release ("up") turns drag mode off
    //   1 = new: *last* mouse button release ("up") turns drag mode off
    // The latter enables dragging with two or more pressed mouse buttons
    // and to continue dragging (i.e. sending FL_DRAG) until the *last*
    // mouse button is released.
    // See fltk.general, thread started on Jul 12, 2023
    // "Is handling simultaneous Left-click and Right-click drags supported?"

    static const int drag_release = 1; // should be 1 => new behavior since Jul 2023

    // Implementation notes:
    // (1) Mode 1 (new): only if *all* mouse buttons have been released, the
    //     Fl::pushed_ widget is reset to 0 (NULL) so subsequent system "move"
    //     events are no longer sent as FL_DRAG events.
    // (2) Mode 0 (old): Fl::pushed_ was reset on the *first* mouse button release.
    // (3) The constant 'drag_release' should be removed once the new mode has been
    //     confirmed to work correctly and no side effects have been observed.
    //     Hint: remove condition "!drag_release || " twice (below).

    // printf("FL_RELEASE: window=%p, pushed() = %p, grab() = %p, modal() = %p, drag_release = %d, buttons = 0x%x\n",
    //        window, pushed(), grab(), modal(), drag_release, Fl::event_buttons()>>24);

    if (grab()) {
      wi = grab();
      if (!drag_release || !Fl::event_buttons())
        pushed_ = 0; // must be zero before callback is done!
    } else if (pushed()) {
      wi = pushed();
      if (!drag_release || !Fl::event_buttons())
        pushed_ = 0; // must be zero before callback is done!
    } else if (modal() && wi != modal())
      return 0;
    int r = send_event(e, wi, window);
    fl_fix_focus();
    return r;
  }

  case FL_UNFOCUS:
    window = 0;
      // FALLTHROUGH
  case FL_FOCUS:
    fl_xfocus = window;
    fl_fix_focus();
    return 1;

  case FL_KEYUP:
    // Send the key-up to the current focus widget. This is not
    // always the same widget that received the corresponding
    // FL_KEYBOARD event because focus may have changed.
    // Sending the KEYUP to the right KEYDOWN is possible, but
    // would require that we track the KEYDOWN for every possible
    // key stroke (users may hold down multiple keys!) and then
    // make sure that the widget still exists before sending
    // a KEYUP there. I believe that the current solution is
    // "close enough".
    for (wi = grab() ? grab() : focus(); wi; wi = wi->parent()) {
      if (send_event(FL_KEYUP, wi, window))
        return 1;
    }
    return 0;

  case FL_KEYBOARD:
#ifdef DEBUG
    printf("Fl::handle(e=%d, window=%p);\n", e, window);
#endif // DEBUG

    Fl_Tooltip::enter((Fl_Widget*)0);

    fl_xfocus = window; // this should not happen!  But maybe it does:

    // Try it as keystroke, sending it to focus and all parents:
    for (wi = grab() ? grab() : focus(); wi; wi = wi->parent()) {
      if (send_event(FL_KEYBOARD, wi, window)) return 1;
    }

    // recursive call to try shortcut:
    if (handle(FL_SHORTCUT, window)) return 1;

    // and then try a shortcut with the case of the text swapped, by
    // changing the text and falling through to FL_SHORTCUT case:
    {
      unsigned char* c = (unsigned char*)event_text(); // cast away const
      if (!isalpha(*c)) return 0;
      *c = isupper(*c) ? tolower(*c) : toupper(*c);
    }
    e_number = e = FL_SHORTCUT;

  case FL_SHORTCUT:
    if (grab()) {wi = grab(); break;} // send it to grab window

    // Try it as shortcut, sending to mouse widget and all parents:
    wi = find_active(belowmouse()); // STR #3216
    if (!wi) {
      wi = modal();
      if (!wi) wi = window;
    } else if (wi->window() != first_window()) {
      if (send_event(FL_SHORTCUT, first_window(), first_window())) return 1;
    }

    for (; wi; wi = wi->parent()) {
      if (send_event(FL_SHORTCUT, wi, wi->window())) return 1;
    }

    // try using add_handle() functions:
    if (send_handlers(FL_SHORTCUT)) return 1;

    // make Escape key close windows:
    if (event_key()==FL_Escape) {
      wi = modal(); if (!wi) wi = window;
      wi->do_callback(FL_REASON_CANCELLED);
      return 1;
    }

    return 0;

  case FL_ENTER:
#ifdef DEBUG
    printf("Fl::handle(e=%d, window=%p);\n", e, window);
#endif // DEBUG

    fl_xmousewin = window;
    fl_fix_focus();
    Fl_Tooltip::enter(belowmouse());
    return 1;

  case FL_LEAVE:
#ifdef DEBUG
    printf("Fl::handle(e=%d, window=%p);\n", e, window);
#endif // DEBUG

    if (!pushed_) {
      belowmouse(0);
      Fl_Tooltip::enter(0);
    }
    if (window == fl_xmousewin) {fl_xmousewin = 0; fl_fix_focus();}
    return 1;

  case FL_MOUSEWHEEL:
    fl_xfocus = window; // this should not happen!  But maybe it does:

    // Try sending it to the "grab" first
    if (grab() && grab()!=modal() && grab()!=window) {
      if (send_event(FL_MOUSEWHEEL, grab(), window)) return 1;
    }
    // Now try sending it to the "modal" window
    if (modal()) {
      send_event(FL_MOUSEWHEEL, modal(), window);
      return 1;
    }
    // Finally try sending it to the window, the event occurred in
    if (send_event(FL_MOUSEWHEEL, window, window->top_window())) return 1;
  default:
    break;
  }
  if (wi && send_event(e, wi, window)) {
    dnd_flag = 0;
    return 1;
  }
  dnd_flag = 0;
  return send_handlers(e);
}


////////////////////////////////////////////////////////////////
// Back compatibility cut & paste functions for fltk 1.1 only:

/** Back-compatibility only: The single-argument call can be used to
    move the selection to another widget or to set the owner to
    NULL, without changing the actual text of the
    selection. FL_SELECTIONCLEAR is sent to the previous
    selection owner, if any.

    <i>Copying the buffer every time the selection is changed is
    obviously wasteful, especially for large selections.  An interface will
    probably be added in a future version to allow the selection to be made
    by a callback function.  The current interface will be emulated on top
    of this.</i>
*/
void Fl::selection_owner(Fl_Widget *owner) {selection_owner_ = owner;}

/**
  Changes the current selection.  The block of text is
  copied to an internal buffer by FLTK (be careful if doing this in
  response to an FL_PASTE as this \e may be the same buffer
  returned by event_text()).  The selection_owner()
  widget is set to the passed owner.
*/
void Fl::selection(Fl_Widget &owner, const char* text, int len) {
  selection_owner_ = &owner;
  Fl::copy(text, len, 0);
}

/** Backward compatibility only.
  This calls Fl::paste(receiver, 0);
  \see Fl::paste(Fl_Widget &receiver, int clipboard, const char* type)
*/
void Fl::paste(Fl_Widget &receiver) {
  Fl::screen_driver()->paste(receiver, 0, Fl::clipboard_plain_text);
}

void Fl::paste(Fl_Widget &receiver, int clipboard, const char *type)
{
  Fl::screen_driver()->paste(receiver, clipboard, type);
}

////////////////////////////////////////////////////////////////

void Fl_Widget::redraw() {
  damage(FL_DAMAGE_ALL);
}

void Fl_Widget::redraw_label() {
  if (window()) {
    if (box() == FL_NO_BOX) {
      // Widgets with the FL_NO_BOX boxtype need a parent to
      // redraw, since it is responsible for redrawing the
      // background...
      int X = x() > 0 ? x() - 1 : 0;
      int Y = y() > 0 ? y() - 1 : 0;
      window()->damage(FL_DAMAGE_ALL, X, Y, w() + 2, h() + 2);
    }

    if (align() && !(align() & FL_ALIGN_INSIDE) && window()->shown()) {
      // If the label is not inside the widget, compute the location of
      // the label and redraw the window within that bounding box...
      int W = 0, H = 0;
      label_.measure(W, H);
      W += 5; // Add a little to the size of the label to cover overflow
      H += 5;

      // FIXME:
      // This assumes that measure() returns the correct outline, which it does
      // not in all possible cases of alignment combined with image and symbols.
      switch (align() & 0x0f) {
        case FL_ALIGN_TOP_LEFT:
          window()->damage(FL_DAMAGE_EXPOSE, x(), y()-H, W, H); break;
        case FL_ALIGN_TOP:
          window()->damage(FL_DAMAGE_EXPOSE, x()+(w()-W)/2, y()-H, W, H); break;
        case FL_ALIGN_TOP_RIGHT:
          window()->damage(FL_DAMAGE_EXPOSE, x()+w()-W, y()-H, W, H); break;
        case FL_ALIGN_LEFT_TOP:
          window()->damage(FL_DAMAGE_EXPOSE, x()-W, y(), W, H); break;
        case FL_ALIGN_RIGHT_TOP:
          window()->damage(FL_DAMAGE_EXPOSE, x()+w(), y(), W, H); break;
        case FL_ALIGN_LEFT:
          window()->damage(FL_DAMAGE_EXPOSE, x()-W, y()+(h()-H)/2, W, H); break;
        case FL_ALIGN_RIGHT:
          window()->damage(FL_DAMAGE_EXPOSE, x()+w(), y()+(h()-H)/2, W, H); break;
        case FL_ALIGN_LEFT_BOTTOM:
          window()->damage(FL_DAMAGE_EXPOSE, x()-W, y()+h()-H, W, H); break;
        case FL_ALIGN_RIGHT_BOTTOM:
          window()->damage(FL_DAMAGE_EXPOSE, x()+w(), y()+h()-H, W, H); break;
        case FL_ALIGN_BOTTOM_LEFT:
          window()->damage(FL_DAMAGE_EXPOSE, x(), y()+h(), W, H); break;
        case FL_ALIGN_BOTTOM:
          window()->damage(FL_DAMAGE_EXPOSE, x()+(w()-W)/2, y()+h(), W, H); break;
        case FL_ALIGN_BOTTOM_RIGHT:
          window()->damage(FL_DAMAGE_EXPOSE, x()+w()-W, y()+h(), W, H); break;
        default:
          window()->damage(FL_DAMAGE_ALL); break;
      }
    } else {
      // The label is inside the widget, so just redraw the widget itself...
      damage(FL_DAMAGE_ALL);
    }
  }
}

void Fl_Widget::damage(uchar fl) {
  if (type() < FL_WINDOW) {
    // damage only the rectangle covered by a child widget:
    damage(fl, x(), y(), w(), h());
  } else {
    // damage entire window by deleting the region:
    Fl_X* i = Fl_X::flx((Fl_Window*)this);
    if (!i) return; // window not mapped, so ignore it
    if (i->region) {
      fl_graphics_driver->XDestroyRegion(i->region);
      i->region = 0;
    }
    damage_ |= fl;
    Fl::damage(FL_DAMAGE_CHILD);
  }
}

void Fl_Widget::damage(uchar fl, int X, int Y, int W, int H) {
  Fl_Widget* wi = this;
  // mark all parent widgets between this and window with FL_DAMAGE_CHILD:
  while (wi->type() < FL_WINDOW) {
    wi->damage_ |= fl;
    wi = wi->parent();
    if (!wi) return;
    fl = FL_DAMAGE_CHILD;
  }
  Fl_X* i = Fl_X::flx((Fl_Window*)wi);
  if (!i) return; // window not mapped, so ignore it

  // clip the damage to the window and quit if none:
  if (X < 0) {W += X; X = 0;}
  if (Y < 0) {H += Y; Y = 0;}
  if (W > wi->w()-X) W = wi->w()-X;
  if (H > wi->h()-Y) H = wi->h()-Y;
  if (W <= 0 || H <= 0) return;

  if (!X && !Y && W==wi->w() && H==wi->h()) {
    // if damage covers entire window delete region:
    wi->damage(fl);
    return;
  }

  if (wi->damage()) {
    // if we already have damage we must merge with existing region:
    if (i->region) {
      fl_graphics_driver->add_rectangle_to_region(i->region, X, Y, W, H);
    }
    wi->damage_ |= fl;
  } else {
    // create a new region:
    if (i->region) fl_graphics_driver->XDestroyRegion(i->region);
    i->region = fl_graphics_driver->XRectangleRegion(X,Y,W,H);
    wi->damage_ = fl;
  }
  Fl::damage(FL_DAMAGE_CHILD);
}


//
// The following methods allow callbacks to schedule the deletion of
// widgets at "safe" times.
//


static int        num_dwidgets = 0, alloc_dwidgets = 0;
static Fl_Widget  **dwidgets = 0;


/**
  Schedules a widget for deletion at the next call to the event loop.
  Use this method to delete a widget inside a callback function.

  To avoid early deletion of widgets, this function should be called
  toward the end of a callback and only after any call to the event
  loop (Fl::wait(), Fl::flush(), Fl::check(), fl_ask(), etc.).

  When deleting groups or windows, you must only delete the group or
  window widget and not the individual child widgets.

  \since FLTK 1.3.4 the widget will be hidden immediately, but the actual
  destruction will be delayed until the event loop is finished. Up to
  FLTK 1.3.3 windows wouldn't be hidden before the event loop was done,
  hence you had to hide() a window in your window close callback if
  you called Fl::delete_widget() to destroy (and hide) the window.

  \since FLTK 1.3.0 it is not necessary to remove widgets from their parent
  groups or windows before calling this, because it will be done in the
  widget's destructor, but it is not a failure to do this nevertheless.

  \note In FLTK 1.1 you \b must remove widgets from their parent group
  (or window) before deleting them.

  \see Fl_Widget::~Fl_Widget()
*/
void Fl::delete_widget(Fl_Widget *wi) {
  if (!wi) return;

  // if the widget is shown(), hide() it (FLTK 1.3.4)
  if (wi->visible_r()) wi->hide();
  Fl_Window *win = wi->as_window();
  if (win && win->shown()) win->hide(); // case of iconified window

  // don't add the same widget twice to the widget delete list
  for (int i = 0; i < num_dwidgets; i++) {
    if (dwidgets[i]==wi) return;
  }

  if (num_dwidgets >= alloc_dwidgets) {
    Fl_Widget **temp;

    temp = new Fl_Widget *[alloc_dwidgets + 10];
    if (alloc_dwidgets) {
      memcpy(temp, dwidgets, alloc_dwidgets * sizeof(Fl_Widget *));
      delete[] dwidgets;
    }

    dwidgets = temp;
    alloc_dwidgets += 10;
  }

  dwidgets[num_dwidgets] = wi;
  num_dwidgets ++;
}


/**
    Deletes widgets previously scheduled for deletion.

    This is for internal use only. You should never call this directly.

    Fl::do_widget_deletion() is called from the FLTK event loop or whenever
    you call Fl::wait(). The previously scheduled widgets are deleted in the
    same order they were scheduled by calling Fl::delete_widget().

    \see Fl::delete_widget(Fl_Widget *wi)
*/
void Fl::do_widget_deletion() {
  if (!num_dwidgets) return;

  for (int i = 0; i < num_dwidgets; i ++)
    delete dwidgets[i];

  num_dwidgets = 0;
}


static Fl_Widget ***widget_watch = 0;
static int num_widget_watch = 0;
static int max_widget_watch = 0;


/**
  Adds a widget pointer to the widget watch list.

  \note Internal use only, please use class Fl_Widget_Tracker instead.

  This can be used, if it is possible that a widget might be deleted during
  a callback or similar function. The widget pointer must be added to the
  watch list before calling the callback. After the callback the widget
  pointer can be queried, if it is NULL. \e If it is NULL, then the widget has been
  deleted during the callback and must not be accessed anymore. If the widget
  pointer is \e not NULL, then the widget has not been deleted and can be accessed
  safely.

  After accessing the widget, the widget pointer must be released from the
  watch list by calling Fl::release_widget_pointer().

  Example for a button that is clicked (from its handle() method):
  \code
    Fl_Widget *wp = this;           // save 'this' in a pointer variable
    Fl::watch_widget_pointer(wp);   // add the pointer to the watch list
    set_changed();                  // set the changed flag
    do_callback();                  // call the callback
    if (!wp) {                      // the widget has been deleted

      // DO NOT ACCESS THE DELETED WIDGET !

    } else {                        // the widget still exists
      clear_changed();              // reset the changed flag
    }

    Fl::release_widget_pointer(wp); // remove the pointer from the watch list
   \endcode

   This works, because all widgets call Fl::clear_widget_pointer() in their
   destructors.

   \see Fl::release_widget_pointer()
   \see Fl::clear_widget_pointer()

   An easier and more convenient method to control widget deletion during
   callbacks is to use the class Fl_Widget_Tracker with a local (automatic)
   variable.

   \see class Fl_Widget_Tracker
*/
void Fl::watch_widget_pointer(Fl_Widget *&w)
{
  Fl_Widget **wp = &w;
  int i;
  for (i=0; i<num_widget_watch; ++i) {
    if (widget_watch[i]==wp) return;
  }
  if (num_widget_watch==max_widget_watch) {
    max_widget_watch += 8;
    widget_watch = (Fl_Widget***)realloc(widget_watch, sizeof(Fl_Widget**)*max_widget_watch);
  }
  widget_watch[num_widget_watch++] = wp;
#ifdef DEBUG_WATCH
  printf ("\nwatch_widget_pointer:   (%d/%d) %8p => %8p\n",
    num_widget_watch,num_widget_watch,wp,*wp);
  fflush(stdout);
#endif // DEBUG_WATCH
}


/**
  Releases a widget pointer from the watch list.

  This is used to remove a widget pointer that has been added to the watch list
  with Fl::watch_widget_pointer(), when it is not needed anymore.

  \note Internal use only, please use class Fl_Widget_Tracker instead.

  \see Fl::watch_widget_pointer()
*/
void Fl::release_widget_pointer(Fl_Widget *&w)
{
  Fl_Widget **wp = &w;
  int i,j=0;
  for (i=0; i<num_widget_watch; ++i) {
    if (widget_watch[i]!=wp) {
      if (j<i) widget_watch[j] = widget_watch[i]; // fill gap
      j++;
    }
#ifdef DEBUG_WATCH
    else { // found widget pointer
      printf("release_widget_pointer: (%d/%d) %8p => %8p\n",
             i+1, num_widget_watch, wp, *wp);
    }
#endif //DEBUG_WATCH
  }
  num_widget_watch = j;
#ifdef DEBUG_WATCH
  printf ("                        num_widget_watch = %d\n\n",num_widget_watch);
  fflush(stdout);
#endif // DEBUG_WATCH
  return;
}


/**
  Clears a widget pointer \e in the watch list.

  This is called when a widget is destroyed (by its destructor). You should never
  call this directly.

  \note Internal use only !

  This method searches the widget watch list for pointers to the widget and
  clears each pointer that points to it. Widget pointers can be added to the
  widget watch list by calling Fl::watch_widget_pointer() or by using the
  helper class Fl_Widget_Tracker (recommended).

  \see Fl::watch_widget_pointer()
  \see class Fl_Widget_Tracker
*/
void Fl::clear_widget_pointer(Fl_Widget const *w)
{
  if (w==0L) return;
  int i;
  for (i=0; i<num_widget_watch; ++i) {
    if (widget_watch[i] && *widget_watch[i]==w) {
      *widget_watch[i] = 0L;
    }
  }
}


/**
 \brief FLTK library options management.

 Options provide a way for the user to modify the behavior of an FLTK
 application. For example, clearing the `OPTION_SHOW_TOOLTIPS` will disable
 tooltips for all FLTK applications.

 Options are set by the user or the administrator on user or machine level.
 In 1.3, FLUID has an Options dialog for that. In 1.4, there is an app named
 `fltk-options` that can be used from the command line or as a GUI tool.
 The machine level setting is read first, and the user setting can override
 the machine setting.

 This function is used throughout FLTK to quickly query the user's wishes.
 There are options for using a native file chooser instead of the FLTK one
 wherever possible, disabling tooltips, disabling visible focus, disabling
 FLTK file chooser preview, etc. .

 See `Fl::Fl_Option` for a list of available options.

 Example:
 \code
     if ( Fl::option(Fl::OPTION_ARROW_FOCUS) )
         { ..on..  }
     else
         { ..off..  }
 \endcode

 \note Since FLTK 1.4.0 options can be managed with the \c fltk-options program.
   In FLTK 1.3.x options can be set in FLUID.

 \param opt which option
 \return true or false
 \see enum Fl::Fl_Option
 \see Fl::option(Fl_Option, bool)
 \see fltk-options application in command line and GUI mode

 \since FLTK 1.3.0
 */
bool Fl::option(Fl_Option opt)
{
  if (!options_read_) {
    int tmp;
    { // first, read the system wide preferences
      Fl_Preferences prefs(Fl_Preferences::CORE_SYSTEM, "fltk.org", "fltk");
      Fl_Preferences opt_prefs(prefs, "options");
      opt_prefs.get("ArrowFocus", tmp, 0);                      // default: off
      options_[OPTION_ARROW_FOCUS] = tmp;
      //opt_prefs.get("NativeFilechooser", tmp, 1);             // default: on
      //options_[OPTION_NATIVE_FILECHOOSER] = tmp;
      //opt_prefs.get("FilechooserPreview", tmp, 1);            // default: on
      //options_[OPTION_FILECHOOSER_PREVIEW] = tmp;
      opt_prefs.get("VisibleFocus", tmp, 1);                    // default: on
      options_[OPTION_VISIBLE_FOCUS] = tmp;
      opt_prefs.get("DNDText", tmp, 1);                         // default: on
      options_[OPTION_DND_TEXT] = tmp;
      opt_prefs.get("ShowTooltips", tmp, 1);                    // default: on
      options_[OPTION_SHOW_TOOLTIPS] = tmp;
      opt_prefs.get("FNFCUsesGTK", tmp, 1);                     // default: on
      options_[OPTION_FNFC_USES_GTK] = tmp;
      opt_prefs.get("PrintUsesGTK", tmp, 1);                     // default: on
      options_[OPTION_PRINTER_USES_GTK] = tmp;

      opt_prefs.get("ShowZoomFactor", tmp, 1);                  // default: on
      options_[OPTION_SHOW_SCALING] = tmp;
      opt_prefs.get("UseZenity", tmp, 0);                       // default: off
      options_[OPTION_FNFC_USES_ZENITY] = tmp;
      opt_prefs.get("UseKdialog", tmp, 0);                      // default: off
      options_[OPTION_FNFC_USES_KDIALOG] = tmp;
      opt_prefs.get("SimpleZoomShortcut", tmp, 0);              // default: off
      options_[OPTION_SIMPLE_ZOOM_SHORTCUT] = tmp;
    }
    { // next, check the user preferences
      // override system options only, if the option is set ( >= 0 )
      Fl_Preferences prefs(Fl_Preferences::CORE_USER, "fltk.org", "fltk");
      Fl_Preferences opt_prefs(prefs, "options");
      opt_prefs.get("ArrowFocus", tmp, -1);
      if (tmp >= 0) options_[OPTION_ARROW_FOCUS] = tmp;
      //opt_prefs.get("NativeFilechooser", tmp, -1);
      //if (tmp >= 0) options_[OPTION_NATIVE_FILECHOOSER] = tmp;
      //opt_prefs.get("FilechooserPreview", tmp, -1);
      //if (tmp >= 0) options_[OPTION_FILECHOOSER_PREVIEW] = tmp;
      opt_prefs.get("VisibleFocus", tmp, -1);
      if (tmp >= 0) options_[OPTION_VISIBLE_FOCUS] = tmp;
      opt_prefs.get("DNDText", tmp, -1);
      if (tmp >= 0) options_[OPTION_DND_TEXT] = tmp;
      opt_prefs.get("ShowTooltips", tmp, -1);
      if (tmp >= 0) options_[OPTION_SHOW_TOOLTIPS] = tmp;
      opt_prefs.get("FNFCUsesGTK", tmp, -1);
      if (tmp >= 0) options_[OPTION_FNFC_USES_GTK] = tmp;
      opt_prefs.get("PrintUsesGTK", tmp, -1);
      if (tmp >= 0) options_[OPTION_PRINTER_USES_GTK] = tmp;

      opt_prefs.get("ShowZoomFactor", tmp, -1);
      if (tmp >= 0) options_[OPTION_SHOW_SCALING] = tmp;
      opt_prefs.get("UseZenity", tmp, -1);
      if (tmp >= 0) options_[OPTION_FNFC_USES_ZENITY] = tmp;
      opt_prefs.get("UseKdialog", tmp, -1);
      if (tmp >= 0) options_[OPTION_FNFC_USES_KDIALOG] = tmp;
      opt_prefs.get("SimpleZoomShortcut", tmp, -1);
      if (tmp >= 0) options_[OPTION_SIMPLE_ZOOM_SHORTCUT] = tmp;
    }
    { // now, if the developer has registered this app, we could ask for per-application preferences
    }
    options_read_ = 1;
  }
  if (opt<0 || opt>=OPTION_LAST)
    return false;
  return (bool)(options_[opt]!=0);
}

/**
  Override an option while the application is running.

  Apps can override the machine settings and the user settings by calling
  `Fl::option(option, bool)`. The override takes effect immediately for this
  option for all widgets in the app for the life time of the app.

  The override is not saved anywhere, and relaunching the app will restore the
  old settings.

  Example:
  \code
    Fl::option(Fl::OPTION_ARROW_FOCUS, true);     // on
    Fl::option(Fl::OPTION_ARROW_FOCUS, false);    // off
  \endcode

  \param opt which option
  \param val set to true or false

 \see enum Fl::Fl_Option
 \see bool Fl::option(Fl_Option)
*/
void Fl::option(Fl_Option opt, bool val)
{
  if (opt<0 || opt>=OPTION_LAST)
    return;
  if (!options_read_) {
    // make sure that the options_ array is filled in
    option(opt);
  }
  options_[opt] = val;
}


// Helper class Fl_Widget_Tracker

/**
  The constructor adds a widget to the watch list.
*/
Fl_Widget_Tracker::Fl_Widget_Tracker(Fl_Widget *wi)
{
  wp_ = wi;
  Fl::watch_widget_pointer(wp_); // add pointer to watch list
}

/**
  The destructor removes a widget from the watch list.
*/
Fl_Widget_Tracker::~Fl_Widget_Tracker()
{
  Fl::release_widget_pointer(wp_); // remove pointer from watch list
}

int Fl::use_high_res_GL_ = 0;

int Fl::draw_GL_text_with_textures_ = 1;

int Fl::dnd()
{
  return Fl::screen_driver()->dnd();
}

int Fl::event_key(int k) {
  return screen_driver()->event_key(k);
}

int Fl::get_key(int k) {
  return screen_driver()->get_key(k);
}

void Fl::get_mouse(int &x, int &y) {
  Fl::screen_driver()->get_mouse(x, y);
}

const char * fl_filename_name(const char *name) {
  return Fl::system_driver()->filename_name(name);
}

void Fl::copy(const char *stuff, int len, int clipboard, const char *type) {
  Fl::screen_driver()->copy(stuff, len, clipboard, type);
}

int Fl::clipboard_contains(const char *type)
{
  return Fl::screen_driver()->clipboard_contains(type);
}


/**
 Adds file descriptor fd to listen to.

 When the fd becomes ready for reading Fl::wait() will call the
 callback and then return. The callback is passed the fd and the
 arbitrary void* argument.

 This version takes a when bitfield, with the bits
 FL_READ, FL_WRITE, and FL_EXCEPT defined,
 to indicate when the callback should be done.

 There can only be one callback of each type for a file descriptor.
 Fl::remove_fd() gets rid of <I>all</I> the callbacks for a given
 file descriptor.

 Under UNIX/Linux/macOS <I>any</I> file descriptor can be monitored (files,
 devices, pipes, sockets, etc.). Due to limitations in Microsoft Windows,
 Windows applications can only monitor sockets.

 Under macOS, Fl::add_fd() opens the display if that's not been done before.
 */
void Fl::add_fd(int fd, int when, Fl_FD_Handler cb, void *d)
{
  Fl::system_driver()->add_fd(fd, when, cb, d);
}

/** Adds file descriptor fd to listen to.
 See Fl::add_fd(int fd, int when, Fl_FD_Handler cb, void* = 0)
 for details */
void Fl::add_fd(int fd, Fl_FD_Handler cb, void *d)
{
  Fl::system_driver()->add_fd(fd, cb, d);
}

void Fl::remove_fd(int fd, int when)
{
  Fl::system_driver()->remove_fd(fd, when);
}

void Fl::remove_fd(int fd)
{
  Fl::system_driver()->remove_fd(fd);
}

/**
 Enables the system input methods facilities. This is the default.
 \see disable_im()
 */
void Fl::enable_im()
{
  Fl::screen_driver()->enable_im();
}

/**
 Disables the system input methods facilities.
 \see enable_im()
 */
void Fl::disable_im()
{
  Fl::screen_driver()->disable_im();
}

/**
 Opens the display.
 Automatically called by the library when the first window is show()'n.
 Does nothing if the display is already open.
 \note Requires \#include <FL/platform.H>
 */
void fl_open_display()
{
  Fl::screen_driver()->open_display();
}

/** Closes the connection to the windowing system when that's possible.
You do \e not need to call this to exit, and in fact it is faster to not do so. It may be
useful to call this if you want your program to continue without
a GUI. You cannot open the display again, and cannot call any FLTK functions.
 \note Requires \#include <FL/platform.H>
*/
void fl_close_display()
{
  Fl::screen_driver()->close_display();
}

#ifdef FL_DOXYGEN
/** Prevent the FLTK library from using its Wayland backend and force it to use its X11 backend.

  Put this declaration somewhere in your source code outside the body of any function:
  \code
    FL_EXPORT bool fl_disable_wayland = true;
  \endcode
  This declaration makes sure that source code developed for FLTK 1.3, including
  X11-specific code, will build and run with FLTK 1.4 and its Wayland platform
  with a single line source code level change.
  This declaration has no effect on non-Wayland platforms.
  Don't add this declaration if you want the Wayland backend to be used when
  it's available.

  \note Please see also chapter 2.1 of README.Wayland.txt for further information
    on how to build your application to ensure that this declaration is effective.

  \since 1.4.0
*/
FL_EXPORT bool fl_disable_wayland = true;
#endif // FL_DOXYGEN

FL_EXPORT Window fl_xid_(const Fl_Window *w) {
  Fl_X *temp = Fl_X::flx(w);
  return temp ? (Window)temp->xid : 0;
}
/** \addtogroup group_macosx
 \{ */

/** Register a function called for each file dropped onto an application icon.

 This function is effective only on the Mac OS X platform.
 \c cb will be called with a single Unix-style file name and path.
 If multiple files were dropped, \c cb will be called multiple times.

 This function should be called before \c fl_open_display() is called,
 either directly or indirectly (this happens at the first \c show() of a window),
 to be effective for files dropped on the application icon at launch time.
 It can also be called at any point to change the function used to open dropped files.
 A call with a NULL argument, after a previous call, makes the app ignore files dropped later.
 */
void fl_open_callback(void (*cb)(const char *))
{
  Fl::system_driver()->open_callback(cb);
}
/** @} */

Fl_Font Fl::set_fonts(const char* xstarname) {
  return Fl_Graphics_Driver::default_driver().set_fonts(xstarname);
}

const char* Fl::get_font_name(Fl_Font fnum, int* ap) {
  return Fl_Graphics_Driver::default_driver().get_font_name(fnum, ap);
}

int Fl::get_font_sizes(Fl_Font fnum, int*& sizep) {
  return Fl_Graphics_Driver::default_driver().get_font_sizes(fnum, sizep);
}

/** Gets the GUI scaling factor of screen number \p n.

  The valid range of \p n is 0 .. Fl::screen_count() - 1.

  The return value is \c 1.0 if screen scaling is not supported or
  \p n is outside the valid range.

  \return Current screen scaling factor (default: \c 1.0)

  \see Fl::screen_count()
  \see Fl::screen_scaling_supported()

  \since 1.4.0
*/
float Fl::screen_scale(int n) {
  if (!Fl::screen_scaling_supported() || n < 0 || n >= Fl::screen_count())
    return 1.;
  return Fl::screen_driver()->scale(n);
}

/** Sets the GUI scaling factor of screen number \p n.

  The valid range of \p n is 0 .. Fl::screen_count() - 1.

  This method does nothing if \p n is out of range or screen scaling
  is not supported by this platform.

  Otherwise it also sets the scaling factor of all windows mapped to
  screen number \p n or all screens, depending on the type of screen
  scaling support on the platform.

  \param[in] n        screen number
  \param[in] factor   scaling factor of screen \p n

  \see Fl::screen_scaling_supported()

  \since 1.4.0
*/
void Fl::screen_scale(int n, float factor) {
  Fl_Screen_Driver::APP_SCALING_CAPABILITY capability = Fl::screen_driver()->rescalable();
  if (!capability || n < 0 || n >= Fl::screen_count()) return;
  if (capability == Fl_Screen_Driver::SYSTEMWIDE_APP_SCALING) {
    for (int s = 0; s < Fl::screen_count(); s++) {
      Fl::screen_driver()->rescale_all_windows_from_screen(s, factor, factor);
    }
  } else
    Fl::screen_driver()->rescale_all_windows_from_screen(n, factor, factor);
}

/**
  Returns whether scaling factors are supported by this platform.

  \retval 0 scaling factors are not supported by this platform
  \retval 1 a single scaling factor is shared by all screens
  \retval 2 each screen can have its own scaling factor

  \see Fl::screen_scale(int)

  \since 1.4.0
*/
int Fl::screen_scaling_supported() {
  return Fl::screen_driver()->rescalable();
}

/** Controls the possibility to scale all windows by ctrl/+/-/0/ or cmd/+/-/0/.

  This function \b should be called before fl_open_display() runs.
  If it is not called, the default is to handle these keys for
  window scaling.

  \note This function can currently only be used to switch the internal
    handler \b off, i.e. \p value must be 0 (zero) - all other values
    result in undefined behavior and are reserved for future extension.

  \param value 0 to stop recognition of ctrl/+/-/0/ (or cmd/+/-/0/ under macOS)
    keys as window scaling.

  \since 1.4.0
*/
void Fl::keyboard_screen_scaling(int value) {
  Fl_Screen_Driver::keyboard_screen_scaling = value;
}

/** Run a command line on the computer */
int Fl::system(const char *command) {
  return Fl::system_driver()->system(command);
}

// Pointers you can use to change FLTK to another language.
// Note: Similar pointers are defined in FL/fl_ask.H and src/fl_ask.cxx
FL_EXPORT const char* fl_local_shift = Fl::system_driver()->shift_name();
FL_EXPORT const char* fl_local_meta  = Fl::system_driver()->meta_name();
FL_EXPORT const char* fl_local_alt   = Fl::system_driver()->alt_name();
FL_EXPORT const char* fl_local_ctrl  = Fl::system_driver()->control_name();

/**
  Convert Windows commandline arguments to UTF-8.

  \note This function does nothing on other (non-Windows) platforms, hence
    you may call it on all platforms or only on Windows by using platform
    specific code like <tt>'\#ifdef _WIN32'</tt> etc. - it's your choice.
    Calling it on other platforms returns quickly w/o wasting much CPU time.

  This function <i>must be called <b>on Windows platforms</b></i> in \c main()
  before the array \c argv is used if your program uses any commandline
  argument strings (these should be UTF-8 encoded).
  This applies also to standard FLTK commandline arguments like
  "-name" (class name) and "-title" (window title in the title bar).

  Unfortunately Windows \b neither provides commandline arguments in UTF-8
  encoding \b nor as Windows "Wide Character" strings in the standard
  \c main() and/or the Windows specific \c WinMain() function.

  On Windows platforms (no matter which build system) this function calls
  a Windows specific function to retrieve commandline arguments as Windows
  "Wide Character" strings, converts these strings to an internally allocated
  buffer (or multiple buffers) and returns the result in \c argv.
  For implementation details please refer to the source code; however these
  details may be changed in the future.

  Note that \c argv is provided by reference so it can be overwritten.

  In the recommended simple form the function overwrites the variable
  \c argv and allocates a new array of strings pointed to by \c argv.
  You may use this form on all platforms and it is as simple as adding
  one line to old programs to make them work with international (UTF-8)
  commandline arguments.

  \code
    int main(int argc, char **argv) {
      Fl::args_to_utf8(argc, argv);   // add this line
      // ... use argc and argv, e.g. for commandline parsing
      window->show(argc, argv);
      return Fl::run();
    }
  \endcode

  For an example see 'examples/howto-parse-args.cxx' in the FLTK sources.

  If you want to retain the original \c argc and \c argv variables the
  following slightly longer and more complicated code works as well on
  all platforms.

  \code
    int main(int argc, char **argv) {
      char **argvn = argv;            // must copy argv to work on all platforms
      int argcn = Fl::args_to_utf8(argc, argvn);
      // ... use argcn and argvn, e.g. for commandline parsing
      window->show(argcn, argvn);
      return Fl::run();
    }
  \endcode

  \param[in]    argc    used only on non-Windows platforms
  \param[out]   argv    modified only on Windows platforms
  \returns  argument count (always the same as argc)

  \since 1.4.0

  \internal This function must not open the display, otherwise
    commandline processing (e.g. by fluid) would open the display.
    OTOH calling it when the display is opened wouldn't work either
    for the same reasons ('fluid -c' doesn't open the display).
*/
int Fl::args_to_utf8(int argc, char ** &argv) {
  return Fl::system_driver()->args_to_utf8(argc, argv);
}
