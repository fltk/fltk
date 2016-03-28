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

//
// End of "$Id$".
//
