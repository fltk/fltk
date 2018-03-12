//
// "$Id$"
//
// Template header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 2016-2018 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

// Do not directly include this file, instead use <FL/android.H>.

// These types and variables give access to internal, platform-specific data through the public API.
// They require to include platform.H (in contrast to types defined in platform_types.h)

#if !defined(FL_PLATFORM_H)
#  error "Never use <FL/android.H> directly; include <FL/platform.H> instead."
#endif // !FL_PLATFORM_H

typedef void *Window; // used by fl_find(), fl_xid() and class Fl_X

/* Reference to the current device context
 For back-compatibility only. The preferred procedure to get this reference is
 Fl_Surface_Device::surface()->driver()->gc().
 */
extern void *fl_gc;

/**
 * These events are specific to the Android OS driver system. They can
 * be read by adding a callback via Fl::add_system_handler() and reading Fl::event()
 * Don't change the order of these enums! See Fl_Android_Application.H .
 */
enum Fl_Android_Platform_Event
{
  FL_ANDROID_EVENTS = 0x00010000,

  /**
   * Command from main thread: the AInputQueue has changed.  Upon processing
   * this command, android_app->inputQueue will be updated to the new queue
   * (or NULL).
   */
  FL_ANDROID_EVENT_INPUT_CHANGED,

  /**
   * Command from main thread: a new ANativeWindow is ready for use.  Upon
   * receiving this command, android_app->window will contain the new window
   * surface.
   */
  FL_ANDROID_EVENT_INIT_WINDOW,

  /**
   * Command from main thread: the existing ANativeWindow needs to be
   * terminated.  Upon receiving this command, android_app->window still
   * contains the existing window; after calling android_app_exec_cmd
   * it will be set to NULL.
   */
  FL_ANDROID_EVENT_TERM_WINDOW,

  /**
   * Command from main thread: the current ANativeWindow has been resized.
   * Please redraw with its new size.
   */
  FL_ANDROID_EVENT_WINDOW_RESIZED,

  /**
   * Command from main thread: the system needs that the current ANativeWindow
   * be redrawn.  You should redraw the window before handing this to
   * android_app_exec_cmd() in order to avoid transient drawing glitches.
   */
  FL_ANDROID_EVENT_WINDOW_REDRAW_NEEDED,

  /**
   * Command from main thread: the content area of the window has changed,
   * such as from the soft input window being shown or hidden.  You can
   * find the new content rect in android_app::contentRect.
   */
  FL_ANDROID_EVENT_CONTENT_RECT_CHANGED,

  /**
   * Command from main thread: the app's activity window has gained
   * input focus.
   */
  FL_ANDROID_EVENT_GAINED_FOCUS,

  /**
   * Command from main thread: the app's activity window has lost
   * input focus.
   */
  FL_ANDROID_EVENT_LOST_FOCUS,

  /**
   * Command from main thread: the current device configuration has changed.
   */
  FL_ANDROID_EVENT_CONFIG_CHANGED,

  /**
   * Command from main thread: the system is running low on memory.
   * Try to reduce your memory use.
   */
  FL_ANDROID_EVENT_LOW_MEMORY,

  /**
   * Command from main thread: the app's activity has been started.
   */
  FL_ANDROID_EVENT_START,

  /**
   * Command from main thread: the app's activity has been resumed.
   */
  FL_ANDROID_EVENT_RESUME,

  /**
   * Command from main thread: the app should generate a new saved state
   * for itself, to restore from later if needed.  If you have saved state,
   * allocate it with malloc and place it in android_app.savedState with
   * the size in android_app.savedStateSize.  The will be freed for you
   * later.
   */
  FL_ANDROID_EVENT_SAVE_STATE,

  /**
   * Command from main thread: the app's activity has been paused.
   */
  FL_ANDROID_EVENT_PAUSE,

  /**
   * Command from main thread: the app's activity has been stopped.
   */
  FL_ANDROID_EVENT_STOP,

  /**
   * Command from main thread: the app's activity is being destroyed,
   * and waiting for the app thread to clean up and exit before proceeding.
   */
  FL_ANDROID_EVENT_DESTROY
};


//
// End of "$Id$".
//
