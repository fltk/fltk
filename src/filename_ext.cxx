//
// "$Id$"
//
// Filename extension routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
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

#include <FL/Fl_System_Driver.H>
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

const char *Fl_System_Driver::filename_ext(const char *buf) {
  const char *q = 0;
  const char *p = buf;
  for (p=buf; *p; p++) {
    if (*p == '/') q = 0;
    else if (*p == '.') q = p;
  }
  return q ? q : p;
}

//
// End of "$Id$".
//
