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
#include "Fl_Darwin_System_Driver.H"
#include <string.h>

//const char* fl_local_alt   = "\xe2\x8c\xa5\\"; // U+2325 (option key)
const char* fl_local_alt   = "⌥\\"; // U+2325 (option key)
//const char* fl_local_ctrl  = "\xe2\x8c\x83\\"; // U+2303 (up arrowhead)
const char* fl_local_ctrl  = "⌃\\"; // U+2303 (up arrowhead)
//const char* fl_local_meta  = "\xe2\x8c\x98\\"; // U+2318 (place of interest sign)
const char* fl_local_meta  = "⌘\\"; // U+2318 (place of interest sign)
//const char* fl_local_shift = "\xe2\x87\xa7\\"; // U+21E7 (upwards white arrow)
const char* fl_local_shift = "⇧\\"; // U+21E7 (upwards white arrow)

Fl_System_Driver *Fl_System_Driver::driver() {
  static Fl_System_Driver *d = new Fl_Darwin_System_Driver();
  return d;
}

int Fl_Darwin_System_Driver::single_arg(const char *arg) {
  // The Finder application in MacOS X passes the "-psn_N_NNNNN" option to all apps.
  return (strncmp(arg, "psn_", 4) == 0);
}

int Fl_Darwin_System_Driver::arg_and_value(const char *name, const char *value) {
  // Xcode in MacOS X may pass "-NSDocumentRevisionsDebugMode YES"
  return strcmp(name, "NSDocumentRevisionsDebugMode") == 0;
}


//
// End of "$Id$".
//
