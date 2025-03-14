//
// Filename handling code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2025 by Bill Spitzak and others.
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

/** \file fluid/filename.cxx

  \brief File names and URI utility functions for FLUID only.

  This file defines all fl_filename* functions using std::string and also
  includes the main header file <FL/filename.H>.

  \note This file contains some filename functions using std::string which
        which are used in FLTK 1.4.x but will be removed in the next minor
        or major release after 1.4.x (i.e. 1.5 or maybe 4.0).

  \note This entire file should become obsolete in 1.5 or higher, whatever
        the next release after 1.4.x will be. We'll use std::string instead!
*/

#include "tools/filename.h"

#include <FL/filename.H>
#include <FL/Fl.H>
//#include <FL/fl_string_functions.h>
//#include "../src/flstring.h"
//
//#include <stdlib.h>
//#include <string>

/**
 Return a shortened filename for limited display width.

 Replace the start uf a path with "~" if it matches the home directory.
 If the remaining filename has more than the give number of characters, it will
 be shortened by replacing parts of the path with an ellipsis ("...").

 The shortened name can no longer be used to open a file. This is purely to
 make as much information visible while fitting into a give space.

 \param[in] filename absolute path and name, UTF-8 aware
 \param[in[ max_chars maximum number of characters in result, including ellipsis
 \return shortened file path and name
 */
std::string fl_filename_shortened(const std::string &filename, int max_chars) {
  // Insert this as the ellipsis
  static const char *ell = "...";
  static const int ell_bytes = 3;
  // Replace the start of a path with "~" if it matches the home directory
  static std::string tilde = "~/";
  static std::string home;
  static int home_chars = -1;
  if (home_chars==-1) {
    home = fl_filename_expand_str(tilde);
    home_chars = fl_utf_nb_char((const uchar*)home.c_str(), (int)home.size());
  }
  std::string homed_filename;
#if defined(_WIN32) || defined(__APPLE__)
  bool starts_with_home = fl_utf_strncasecmp(home.c_str(), filename.c_str(), home_chars)==0;
#else
  bool starts_with_home = ::strncmp(home.c_str(), filename.c_str(), home.size())==0;
#endif
  if (starts_with_home) {
    homed_filename = tilde + filename.substr(home.size());
  } else {
    homed_filename = filename;
  }
  // C style pointer will stay valid until filename is modified.
  const unsigned char *u8str = reinterpret_cast<const unsigned char *>(homed_filename.c_str());
  // Count the number of UTF-8 characters in the name.
  int num_chars = fl_utf_nb_char(u8str, (int)homed_filename.size());
  if (num_chars+ell_bytes-1 > max_chars) {
    // Create a new string by replacing characters in the middle.
    int remove_chars = num_chars - max_chars + ell_bytes;
    int left_chars = (max_chars - ell_bytes)/2;
//    int right_chars = max_chars - left_chars - 3;
//    int right_start_char = num_chars - right_chars;
    // Convert character counts into byte counts.
    int left_bytes = fl_utf8strlen(homed_filename.c_str(), left_chars);
    int right_start_byte = fl_utf8strlen(homed_filename.c_str()+left_bytes, remove_chars) + left_bytes;
    return homed_filename.substr(0, left_bytes) + ell + homed_filename.substr(right_start_byte);
  } else {
    // Nothing to change.
    return homed_filename;
  }
}

/**
 Make sure that a path name ends with a forward slash.
 \param[in] str directory or path name
 \return a new string, ending with a '/'
 */
std::string fld::end_with_slash(const std::string &str) {
  char last = str[str.size()-1];
  if (last !='/' && last != '\\')
    return str + "/";
  else
    return str;
}

/**
 Replace Windows '\\' directory separator with UNix '/' separators.
 \param[in] fn a file path in Unix or Windows format
 \return a copy of the file path in Unix format.
 */
std::string fld::fix_separators(const std::string &fn) {
  std::string ret = fn;
  for (size_t i=0; i<ret.size(); ++i) {
    if (ret[i] == '\\') {
      ret[i] = '/';
    }
  }
  return ret;
}
