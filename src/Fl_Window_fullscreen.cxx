//
// "$Id$"
//
// Fullscreen window support for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2015 by Bill Spitzak and others.
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

// Turning the border on/off by changing the motif_wm_hints property
// works on Irix 4DWM.  Does not appear to work for any other window
// manager.  Fullscreen still works on some window managers (fvwm is one)
// because they allow the border to be placed off-screen.

// Unfortunately most X window managers ignore changes to the border
// and refuse to position the border off-screen, so attempting to make
// the window full screen will lose the size of the border off the
// bottom and right.

#include <FL/Fl.H>
#include <FL/x.H>

#include <config.h>

#if FLTK_ABI_VERSION < 10301
int Fl_Window::no_fullscreen_x = 0;
int Fl_Window::no_fullscreen_y = 0;
int Fl_Window::no_fullscreen_w = 0;
int Fl_Window::no_fullscreen_h = 0;
#endif

#if FLTK_ABI_VERSION < 10303
int Fl_Window::fullscreen_screen_top = -1;
int Fl_Window::fullscreen_screen_bottom = -1;
int Fl_Window::fullscreen_screen_left = -1;
int Fl_Window::fullscreen_screen_right = -1;
#endif

void Fl_Window::border(int b) {
  if (b) {
    if (border()) return;
    clear_flag(NOBORDER);
  } else {
    if (!border()) return;
    set_flag(NOBORDER);
  }
#if defined(USE_X11)
  if (shown()) Fl_X::i(this)->sendxjunk();
#elif defined(WIN32)
  // not yet implemented, but it's possible
  // for full fullscreen we have to make the window topmost as well
#elif defined(__APPLE_QUARTZ__)
  // warning: not implemented in Quartz/Carbon
#else
# error unsupported platform
#endif
}

/* Note: The previous implementation toggled border(). With this new
   implementation this is not necessary. Additionally, if we do that,
   the application may lose focus when switching out of fullscreen
   mode with some window managers. Besides, the API does not say that
   the FLTK border state should be toggled; it only says that the
   borders should not be *visible*. 
*/
void Fl_Window::fullscreen() {
  no_fullscreen_x = x();
  no_fullscreen_y = y();
  no_fullscreen_w = w();
  no_fullscreen_h = h();
  if (shown() && !(flags() & Fl_Widget::FULLSCREEN)) {
    fullscreen_x();
  } else {
    set_flag(FULLSCREEN);
  }
}

void Fl_Window::fullscreen_off(int X,int Y,int W,int H) {
  if (shown() && (flags() & Fl_Widget::FULLSCREEN)) {
    fullscreen_off_x(X, Y, W, H);
  } else {
    clear_flag(FULLSCREEN);
  }
  no_fullscreen_x = no_fullscreen_y = no_fullscreen_w = no_fullscreen_h = 0;
}

void Fl_Window::fullscreen_off() {
  if (!no_fullscreen_x && !no_fullscreen_y) {
    // Window was initially created fullscreen - default to current monitor
    no_fullscreen_x = x();
    no_fullscreen_y = y();
  }
  fullscreen_off(no_fullscreen_x, no_fullscreen_y, no_fullscreen_w, no_fullscreen_h);
}

void Fl_Window::fullscreen_screens(int top, int bottom, int left, int right) {
  if ((top < 0) || (bottom < 0) || (left < 0) || (right < 0)) {
    fullscreen_screen_top = -1;
    fullscreen_screen_bottom = -1;
    fullscreen_screen_left = -1;
    fullscreen_screen_right = -1;
  } else {
    fullscreen_screen_top = top;
    fullscreen_screen_bottom = bottom;
    fullscreen_screen_left = left;
    fullscreen_screen_right = right;
  }

  if (shown() && fullscreen_active())
    fullscreen_x();
}


//
// End of "$Id$".
//
