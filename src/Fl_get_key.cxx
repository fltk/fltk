//
// "$Id$"
//
// Keyboard state routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2018 by Bill Spitzak and others.
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

#include "config_lib.h"
#if defined(FL_CFG_SYS_POSIX) && !defined(FL_DOXYGEN)

// Return the current state of a key.  This is the X version.  I identify
// keys (mostly) by the X keysym.  So this turns the keysym into a keycode
// and looks it up in the X key bit vector, which Fl_x.cxx keeps track of.

#include <FL/Fl.H>
#include "drivers/X11/Fl_X11_System_Driver.H"
#include <FL/platform.H> // for fl_display

extern char fl_key_vector[32]; // in Fl_x.cxx

int Fl_X11_System_Driver::event_key(int k) {
  if (k > FL_Button && k <= FL_Button+8)
    return Fl::event_state(8<<(k-FL_Button));
  int i;
#  ifdef __sgi
  // get some missing PC keyboard keys:
  if (k == FL_Meta_L) i = 147;
  else if (k == FL_Meta_R) i = 148;
  else if (k == FL_Menu) i = 149;
  else
#  endif
    i = XKeysymToKeycode(fl_display, k);
  if (i==0) return 0;
  return fl_key_vector[i/8] & (1 << (i%8));
}

int Fl_X11_System_Driver::get_key(int k) {
  fl_open_display();
  XQueryKeymap(fl_display, fl_key_vector);
  return event_key(k);
}

#endif // FL_CFG_SYS_POSIX

//
// End of "$Id$".
//
