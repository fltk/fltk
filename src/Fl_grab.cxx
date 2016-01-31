//
// "$Id$"
//
// Grab/release code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2016 by Bill Spitzak and others.
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

#include <config.h>
#include <FL/Fl.H>
#include <FL/x.H>

////////////////////////////////////////////////////////////////
// "Grab" is done while menu systems are up.  This has several effects:
// Events are all sent to the "grab window", which does not even
// have to be displayed (and in the case of Fl_Menu.cxx it isn't).
// The system is also told to "grab" events and send them to this app.
// This also modifies how Fl_Window::show() works, on X it turns on
// override_redirect, it does similar things on WIN32.

extern void fl_fix_focus(); // in Fl.cxx

#ifdef WIN32
// We have to keep track of whether we have captured the mouse, since
// MSWindows shows little respect for this... Grep for fl_capture to
// see where and how this is used.
extern HWND fl_capture;
#endif

#ifdef __APPLE__
extern void *fl_capture;
#endif

void Fl::grab(Fl_Window* win) {
#ifdef USE_X11
    Fl_Window *fullscreen_win = NULL;
    for (Fl_Window *W = Fl::first_window(); W; W = Fl::next_window(W)) {
      if (W->fullscreen_active()) {
        fullscreen_win = W;
        break;
      }
    }
#endif
  if (win) {
    if (!grab_) {
#ifdef WIN32
      SetActiveWindow(fl_capture = fl_xid(first_window()));
      SetCapture(fl_capture);
#elif defined(__APPLE__)
      fl_capture = Fl_X::i(first_window())->xid;
      Fl_X::i(first_window())->set_key_window();
#else
      Window xid = fullscreen_win ? fl_xid(fullscreen_win) : fl_xid(first_window());
      XGrabPointer(fl_display,
		   xid,
		   1,
		   ButtonPressMask|ButtonReleaseMask|
		   ButtonMotionMask|PointerMotionMask,
		   GrabModeAsync,
		   GrabModeAsync, 
		   None,
		   0,
		   fl_event_time);
      XGrabKeyboard(fl_display,
		    xid,
		    1,
		    GrabModeAsync,
		    GrabModeAsync, 
		    fl_event_time);
#endif
    }
    grab_ = win;
  } else {
    if (grab_) {
#ifdef WIN32
      fl_capture = 0;
      ReleaseCapture();
#elif defined(__APPLE__)
      fl_capture = 0;
#else
      // We must keep the grab in the non-EWMH fullscreen case
      if (!fullscreen_win || Fl_X::ewmh_supported()) {
      XUngrabKeyboard(fl_display, fl_event_time);
      }
      XUngrabPointer(fl_display, fl_event_time);
      // this flush is done in case the picked menu item goes into
      // an infinite loop, so we don't leave the X server locked up:
      XFlush(fl_display);
#endif
      grab_ = 0;
      fl_fix_focus();
    }
  }
}

//
// End of "$Id$".
//
