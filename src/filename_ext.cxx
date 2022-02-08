//
// Filename extension routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
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

#include "Fl_System_Driver.H"
#include <FL/Fl.H>

/** Gets the extension of a filename.
   \code
   #include <FL/filename.H>
   [..]
   const char *out;
   out = fl_filename_ext("/some/path/foo.txt");        // result: ".txt"
   out = fl_filename_ext("/some/path/foo");            // result: NULL
   \endcode
   \param[in] buf the filename to be parsed
   \return a pointer to the extension (including '.') if any or NULL otherwise
 */
const char *fl_filename_ext(const char *buf) {
  return Fl::system_driver()->filename_ext(buf);
}


/**
 \cond DriverDev
 \addtogroup DriverDeveloper
 \{
 */

/**
 Finds a filename extension.

 The default implementation assumes that the last `.` character separates
 the extension from the basename of a file.

 \see fl_filename_ext(const char*)
 */
const char *Fl_System_Driver::filename_ext(const char *buf) {
  const char *q = 0;
  const char *p = buf;
  for (p=buf; *p; p++) {
    if (*p == '/') q = 0;
    else if (*p == '.') q = p;
  }
  return q ? q : p;
}

/**
 \}
 \endcond
 */
