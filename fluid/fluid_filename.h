/*
 * Filename header file for the Fast Light Tool Kit (FLTK).
 *
 * Copyright 1998-2023 by Bill Spitzak and others.
 *
 * This library is free software. Distribution and use rights are outlined in
 * the file "COPYING" which should have been included with this file.  If this
 * file is missing or damaged, see the license at:
 *
 *     https://www.fltk.org/COPYING.php
 *
 * Please see the following page on how to report bugs and issues:
 *
 *     https://www.fltk.org/bugs.php
 */

/** \file fluid/fluid_filename.h

  \brief File names and URI utility functions for FLUID only.

  This file declares all fl_filename* functions using Fl_String and also
  includes the main header file <FL/filename.H>.

  \note This file contains some filename functions using Fl_String which
        which are used in FLTK 1.4.x but will be removed in the next minor
        or major release after 1.4.x (i.e. 1.5 or maybe 4.0).

  \note This entire file should become obsolete in 1.5 or higher, whatever
        the next release after 1.4.x will be. We'll use std::string instead!
*/

#ifndef FLUID_FILENAME_H
#  define FLUID_FILENAME_H

#include <FL/Fl_Export.H>
#include <FL/platform_types.h>

#include <FL/filename.H>

#  if defined(__cplusplus)

class Fl_String;

Fl_String fl_filename_name(const Fl_String &filename);
Fl_String fl_filename_path(const Fl_String &filename);
Fl_String fl_filename_ext(const Fl_String &filename);
Fl_String fl_filename_setext(const Fl_String &filename, const Fl_String &new_extension);
Fl_String fl_filename_expand(const Fl_String &from);
Fl_String fl_filename_absolute(const Fl_String &from);
Fl_String fl_filename_absolute(const Fl_String &from, const Fl_String &base);
Fl_String fl_filename_relative(const Fl_String &from);
Fl_String fl_filename_relative(const Fl_String &from, const Fl_String &base);
Fl_String fl_getcwd();

#  endif

/** @} */

#endif /* FLUID_FILENAME_H */
