//
// Filename expansion routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2023 by Bill Spitzak and others.
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

/** \file fluid/fluid_filename.cxx

  \brief File names and URI utility functions for FLUID only.

  This file defines all fl_filename* functions using Fl_String and also
  includes the main header file <FL/filename.H>.

  \note This file contains some filename functions using Fl_String which
        which are used in FLTK 1.4.x but will be removed in the next minor
        or major release after 1.4.x (i.e. 1.5 or maybe 4.0).

  \note This entire file should become obsolete in 1.5 or higher, whatever
        the next release after 1.4.x will be. We'll use std::string instead!
*/

#include <stdlib.h>

#include <FL/filename.H>
#include <FL/Fl.H>
#include <FL/fl_string_functions.h>

#include "../src/Fl_String.H"       // NOTE: FLTK 1.4.x only !
#include "../src/flstring.h"


/**
 Return a new string that contains the name part of the filename.
 \param[in] filename file path and name
 \return the name part of a filename
 \see fl_filename_name(const char *filename)
 */
Fl_String fl_filename_name(const Fl_String &filename) {
  return Fl_String(fl_filename_name(filename.c_str()));
}

/**
 Return a new string that contains the path part of the filename.
 \param[in] filename file path and name
 \return the path part of a filename without the name
 \see fl_filename_name(const char *filename)
 */
Fl_String fl_filename_path(const Fl_String &filename) {
  const char *base = filename.c_str();
  const char *name = fl_filename_name(base);
  if (name) {
    return Fl_String(base, (int)(name-base));
  } else {
    return Fl_String();
  }
}

/**
 Return a new string that contains the filename extension.
 \param[in] filename file path and name
 \return the filename extension including the prepending '.', or an empty
    string if the filename has no extension
 \see fl_filename_ext(const char *buf)
 */
Fl_String fl_filename_ext(const Fl_String &filename) {
  return Fl_String(fl_filename_ext(filename.c_str()));
}

/**
 Return a copy of the old filename with the new extension.
 \param[in] filename file path and name
 \param[in] new_extension new filename extension, starts with a '.'
 \return the new filename
 \see fl_filename_setext(char *to, int tolen, const char *ext)
 */
Fl_String fl_filename_setext(const Fl_String &filename, const Fl_String &new_extension) {
  char buffer[FL_PATH_MAX];
  fl_strlcpy(buffer, filename.c_str(), FL_PATH_MAX);
  fl_filename_setext(buffer, FL_PATH_MAX, new_extension.c_str());
  return Fl_String(buffer);
}

/**
 Expands a filename containing shell variables and tilde (~).
 \param[in] from file path and name
 \return the new, expanded filename
 \see fl_filename_expand(char *to, int tolen, const char *from)
*/
Fl_String fl_filename_expand(const Fl_String &from) {
  char buffer[FL_PATH_MAX];
  fl_filename_expand(buffer, FL_PATH_MAX, from.c_str());
  return Fl_String(buffer);
}

/**
 Makes a filename absolute from a filename relative to the current working directory.
 \param[in] from relative filename
 \return the new, absolute filename
 \see fl_filename_absolute(char *to, int tolen, const char *from)
 */
Fl_String fl_filename_absolute(const Fl_String &from) {
  char buffer[FL_PATH_MAX];
  fl_filename_absolute(buffer, FL_PATH_MAX, from.c_str());
  return Fl_String(buffer);
}

/**
 Append the relative filename `from` to the absolute filename `base` to form
 the new absolute path.
 \param[in] from relative filename
 \param[in] base `from` is relative to this absolute file path
 \return the new, absolute filename
 \see fl_filename_absolute(char *to, int tolen, const char *from, const char *base)
 */
Fl_String fl_filename_absolute(const Fl_String &from, const Fl_String &base) {
  char buffer[FL_PATH_MAX];
  fl_filename_absolute(buffer, FL_PATH_MAX, from.c_str(), base.c_str());
  return Fl_String(buffer);
}

/**
 Makes a filename relative to the current working directory.
 \param[in] from file path and name
 \return the new, relative filename
 \see fl_filename_relative(char *to, int tolen, const char *from)
 */
Fl_String fl_filename_relative(const Fl_String &from) {
  char buffer[FL_PATH_MAX];
  fl_filename_relative(buffer, FL_PATH_MAX, from.c_str());
  return Fl_String(buffer);
}

/**
 Makes a filename relative to any directory.
 \param[in] from file path and name
 \param[in] base relative to this absolute path
 \return the new, relative filename
 \see fl_filename_relative(char *to, int tolen, const char *from, const char *base)
 */
Fl_String fl_filename_relative(const Fl_String &from, const Fl_String &base) {
  char buffer[FL_PATH_MAX];
  fl_filename_relative(buffer, FL_PATH_MAX, from.c_str(), base.c_str());
  return Fl_String(buffer);
}

/** Cross-platform function to get the current working directory
 as a UTF-8 encoded value in an Fl_String.
 \return the CWD encoded as UTF-8
 */
Fl_String fl_getcwd() {
  char buffer[FL_PATH_MAX];
  fl_getcwd(buffer, FL_PATH_MAX);
  return Fl_String(buffer);
}

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
Fl_String fl_filename_shortened(const Fl_String &filename, int max_chars) {
  // Insert this as the ellipsis
  static const char *ell = "...";
  static const int ell_bytes = 3;
  // Replace the start of a path with "~" if it matches the home directory
  static Fl_String tilde = "~/";
  static Fl_String home;
  static int home_chars = -1;
  if (home_chars==-1) {
    home = fl_filename_expand(tilde);
    home_chars = fl_utf_nb_char((const uchar*)home.c_str(), home.size());
  }
  Fl_String homed_filename;
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
  int num_chars = fl_utf_nb_char(u8str, homed_filename.size());
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
