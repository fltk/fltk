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
#include <FL/Fl.H>
#include <FL/x.H>
#include <string.h>
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
#include <xlocale.h>
#else
#include <locale.h>
#endif
#include <stdio.h>

int Fl_X::next_marked_length = 0;

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

Fl_Darwin_System_Driver::Fl_Darwin_System_Driver() {
  if (fl_mac_os_version == 0) fl_mac_os_version = Fl_X::calc_mac_os_version();
}

int Fl_Darwin_System_Driver::single_arg(const char *arg) {
  // The Finder application in MacOS X passes the "-psn_N_NNNNN" option to all apps.
  return (strncmp(arg, "psn_", 4) == 0);
}

int Fl_Darwin_System_Driver::arg_and_value(const char *name, const char *value) {
  // Xcode in MacOS X may pass "-NSDocumentRevisionsDebugMode YES"
  return strcmp(name, "NSDocumentRevisionsDebugMode") == 0;
}

static int insertion_point_x = 0;
static int insertion_point_y = 0;
static int insertion_point_height = 0;
static bool insertion_point_location_is_valid = false;

int Fl_Darwin_System_Driver::has_marked_text() {
  return true;
}

void Fl_Darwin_System_Driver::reset_marked_text() {
  Fl::compose_state = 0;
  Fl_X::next_marked_length = 0;
  insertion_point_location_is_valid = false;
}

int Fl_X::insertion_point_location(int *px, int *py, int *pheight)
// return true if the current coordinates of the insertion point are available
{
  if ( ! insertion_point_location_is_valid ) return false;
  *px = insertion_point_x;
  *py = insertion_point_y;
  *pheight = insertion_point_height;
  return true;
}

void Fl_Darwin_System_Driver::insertion_point_location(int x, int y, int height) {
  insertion_point_location_is_valid = true;
  insertion_point_x = x;
  insertion_point_y = y;
  insertion_point_height = height;
}

int Fl_Darwin_System_Driver::compose(int &del) {
  int condition;
  int has_text_key = Fl::compose_state || Fl::e_keysym <= '~' || Fl::e_keysym == FL_Iso_Key ||
  (Fl::e_keysym >= FL_KP && Fl::e_keysym <= FL_KP_Last && Fl::e_keysym != FL_KP_Enter);
  condition = Fl::e_state&(FL_META | FL_CTRL) ||
  (Fl::e_keysym >= FL_Shift_L && Fl::e_keysym <= FL_Alt_R) || // called from flagsChanged
  !has_text_key ;
  if (condition) { del = 0; return 0;} // this stuff is to be treated as a function key
  del = Fl::compose_state;
  Fl::compose_state = Fl_X::next_marked_length;
  return 1;
}

int Fl_Darwin_System_Driver::clocale_printf(FILE *output, const char *format, va_list args) {
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
  if (fl_mac_os_version >= 100400) {
    static locale_t postscript_locale = newlocale(LC_NUMERIC_MASK, "C", (locale_t)0);
    return vfprintf_l(output, postscript_locale, format, args);
  }
#endif
  char *saved_locale = setlocale(LC_NUMERIC, NULL);
  setlocale(LC_NUMERIC, "C");
  int retval = vfprintf(output, format, args);
  setlocale(LC_NUMERIC, saved_locale);
  return retval;
}

//
// End of "$Id$".
//
