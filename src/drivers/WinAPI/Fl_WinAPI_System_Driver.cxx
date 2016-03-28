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


#include "../../config_lib.h"
#include "Fl_WinAPI_System_Driver.H"
#include <stdio.h>
#include <windows.h>

#if !defined(FL_DOXYGEN)
const char* fl_local_alt   = "Alt";
const char* fl_local_ctrl  = "Ctrl";
const char* fl_local_meta  = "Meta";
const char* fl_local_shift = "Shift";
#endif

Fl_System_Driver *Fl_System_Driver::driver() {
  static Fl_System_Driver *d = new Fl_WinAPI_System_Driver();
  return d;
}

void Fl_WinAPI_System_Driver::warning(const char *format, va_list args) {
  // Show nothing for warnings under WIN32...
}

void Fl_WinAPI_System_Driver::error(const char *format, va_list args) {
  char buf[1024];
  vsnprintf(buf, 1024, format, args);
  MessageBox(0,buf,"Error",MB_ICONEXCLAMATION|MB_SYSTEMMODAL);
}

void Fl_WinAPI_System_Driver::fatal(const char *format, va_list args) {
  char buf[1024];
  vsnprintf(buf, 1024, format, args);
  MessageBox(0,buf,"Error",MB_ICONSTOP|MB_SYSTEMMODAL);
  ::exit(1);
}


//
// End of "$Id$".
//
