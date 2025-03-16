/*
 * Filename handling header file for the Fast Light Tool Kit (FLTK).
 *
 * Copyright 1998-2025 by Bill Spitzak and others.
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

/** \file fluid/filename.h
  \brief Handling file names operations that are not in the core library.
*/

#ifndef FLUID_TOOLS_FILENAME_H
#define FLUID_TOOLS_FILENAME_H

#include <string>

std::string fl_filename_shortened(const std::string &filename, int maxchars);

namespace fld {

extern std::string end_with_slash(const std::string &fn);
extern std::string fix_separators(const std::string &fn);

} // namespace fld

#endif // FLUID_TOOLS_FILENAME_H
