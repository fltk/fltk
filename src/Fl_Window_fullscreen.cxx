//
// Fullscreen window support for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2015 by Bill Spitzak and others.
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

#include <FL/Fl_Window.H>
#include "Fl_Window_Driver.H"

void Fl_Window::border(int b) {
  if (b) {
    if (border()) return;
    clear_flag(NOBORDER);
  } else {
    if (!border()) return;
    set_flag(NOBORDER);
  }
  pWindowDriver->use_border();
}

/* Note: The previous implementation toggled border(). With this new
   implementation this is not necessary. Additionally, if we do that,
   the application may lose focus when switching out of fullscreen
   mode with some window managers. Besides, the API does not say that
   the FLTK border state should be toggled; it only says that the
   borders should not be *visible*.
*/
void Fl_Window::fullscreen() {
  if (!is_resizable()) return;
  if (!maximize_active()) {
    no_fullscreen_x = x();
    no_fullscreen_y = y();
    no_fullscreen_w = w();
    no_fullscreen_h = h();
  }
  if (shown() && !(flags() & Fl_Widget::FULLSCREEN)) {
    pWindowDriver->fullscreen_on();
  } else {
    set_flag(FULLSCREEN);
  }
}

void Fl_Window::fullscreen_off(int X,int Y,int W,int H) {
  if (shown() && (flags() & Fl_Widget::FULLSCREEN)) {
    pWindowDriver->fullscreen_off(X, Y, W, H);
  } else {
    clear_flag(FULLSCREEN);
  }
  if (!maximize_active())
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
    pWindowDriver->fullscreen_on();
}
