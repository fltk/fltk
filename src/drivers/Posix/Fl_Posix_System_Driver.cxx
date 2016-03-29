//
// "$Id$"
//
// Definition of Apple Darwin system driver.
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


#include "Fl_Posix_System_Driver.H"
#include <FL/Fl.H>
#include <X11/Xlib.h>

extern XIC fl_xim_ic; // in Fl_x.cxx


// Pointers you can use to change FLTK to a foreign language.
// Note: Similar pointers are defined in FL/fl_ask.H and src/fl_ask.cxx
const char* fl_local_alt   = "Alt";
const char* fl_local_ctrl  = "Ctrl";
const char* fl_local_meta  = "Meta";
const char* fl_local_shift = "Shift";

Fl_System_Driver *Fl_System_Driver::driver() {
  static Fl_System_Driver *d = new Fl_Posix_System_Driver();
  return d;
}

void Fl_Posix_System_Driver::display_arg(const char *arg) {
  Fl::display(arg);
}

int Fl_Posix_System_Driver::XParseGeometry(const char* string, int* x, int* y,
                                           unsigned int* width, unsigned int* height) {
  return ::XParseGeometry(string, x, y, width, height);
}

int Fl_Posix_System_Driver::compose(int& del) {
  int condition;
  unsigned char ascii = (unsigned char)Fl::e_text[0];
  condition = (Fl::e_state & (FL_ALT | FL_META | FL_CTRL)) && !(ascii & 128) ;
  if (condition) { del = 0; return 0;} // this stuff is to be treated as a function key
  del = Fl::compose_state;
  Fl::compose_state = 0;
  // Only insert non-control characters:
  if ( (!Fl::compose_state) && ! (ascii & ~31 && ascii!=127)) { return 0; }
  return 1;
}

void Fl_Posix_System_Driver::compose_reset()
{
  Fl::compose_state = 0;
  if (fl_xim_ic) XmbResetIC(fl_xim_ic);
}

//
// End of "$Id$".
//
