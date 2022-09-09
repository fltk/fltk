//
// Definition of Wayland system driver
// for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2021 by Bill Spitzak and others.
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

#include "Fl_Wayland_System_Driver.H"
#include <FL/Fl.H>
#include "Fl_Wayland_Window_Driver.H"
#include "Fl_Wayland_Screen_Driver.H"
#include <FL/platform.H>
#include "../../../libdecor/src/libdecor.h"
#include <stdlib.h>


bool Fl_Wayland_System_Driver::too_late_to_disable = false;


int Fl_Wayland_System_Driver::event_key(int k) {
  if (k > FL_Button && k <= FL_Button+8)
    return Fl::event_state(8<<(k-FL_Button));
  int sym = Fl::event_key();
  if (sym >= 'a' && sym <= 'z' ) sym -= 32;
  if (k >= 'a' && k <= 'z' )  k -= 32;
  return (Fl::event() == FL_KEYDOWN || Fl::event() == FL_SHORTCUT) && sym == k;
}


int Fl_Wayland_System_Driver::get_key(int k) {
  return event_key(k);
}


void *Fl_Wayland_System_Driver::control_maximize_button(void *data) {
  // The code below aims at removing the calling window's fullscreen button
  // while dialog runs. Unfortunately, it doesn't work with some X11 window managers
  // (e.g., KDE, xfce) because the button goes away but doesn't come back,
  // so we move this code to a virtual member function.
  // Noticeably, this code works OK under Wayland.
  struct win_dims {
    Fl_Widget_Tracker *tracker;
    int minw, minh, maxw, maxh;
    struct win_dims *next;
  };

  if (!data) { // this call turns each decorated window's maximize button off
    struct win_dims *first_dim = NULL;
    // consider all bordered, top-level FLTK windows
    Fl_Window *win = Fl::first_window();
    while (win) {
      if (!win->parent() && win->border() &&
          !( ((struct wld_window*)Fl_X::i(win)->xid)->state & LIBDECOR_WINDOW_STATE_MAXIMIZED) ) {
        win_dims *dim = new win_dims;
        dim->tracker = new Fl_Widget_Tracker(win);
        Fl_Window_Driver *dr = Fl_Window_Driver::driver(win);
        dim->minw = dr->minw();
        dim->minh = dr->minh();
        dim->maxw = dr->maxw();
        dim->maxh = dr->maxh();
        //make win un-resizable
        win->size_range(win->w(), win->h(), win->w(), win->h());
        dim->next = first_dim;
        first_dim = dim;
      }
      win = Fl::next_window(win);
    }
    return first_dim;
  } else { // this call returns each decorated window's maximize button to its previous state
    win_dims *first_dim = (win_dims *)data;
    while (first_dim) {
      win_dims *dim = first_dim;
      //give back win its resizing parameters
      if (dim->tracker->exists()) {
        Fl_Window *win = (Fl_Window*)dim->tracker->widget();
        win->size_range(dim->minw, dim->minh, dim->maxw, dim->maxh);
      }
      first_dim = dim->next;
      delete dim->tracker;
      delete dim;
    }
    return NULL;
  }
}


void Fl_Wayland_System_Driver::disable_wayland() {
#if FLTK_USE_X11
  if (too_late_to_disable) {
    fprintf(stderr, "Error: fl_disable_wayland() cannot be called "
            "after the Wayland display was opened\n"
            "or a Wayland window was created or the Wayland screen was accessed\n");
    exit(1);
  }
  Fl_Wayland_Screen_Driver::undo_wayland_backend_if_needed("x11");
#endif
}
