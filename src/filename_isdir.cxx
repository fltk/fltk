//
// "$Id$"
//
// Directory detection routines for the Fast Light Tool Kit (FLTK).
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

// Used by fl_file_chooser

#include "flstring.h"
#include <FL/Fl_System_Driver.H>
#include <FL/filename.H>
#include <FL/Fl.H>

/*
 * filename_isdir_quick() is a private function that checks for a
 * trailing slash and assumes that the passed name is a directory if
 * it finds one.  This function is used by Fl_File_Browser and
 * Fl_File_Chooser to avoid extra stat() calls, but is not supported
 * outside of FLTK...
 */
int Fl_System_Driver::filename_isdir_quick(const char* n) {
  // Do a quick optimization for filenames with a trailing slash...
  if (*n && n[strlen(n) - 1] == '/') return 1;
  return filename_isdir(n);
}

/**
   Determines if a file exists and is a directory from its filename.
   \code
   #include <FL/filename.H>
   [..]
   fl_filename_isdir("/etc");		// returns non-zero
   fl_filename_isdir("/etc/hosts");	// returns 0
   \endcode
   \param[in] n the filename to parse
   \return non zero if file exists and is a directory, zero otherwise
*/
int fl_filename_isdir(const char* n) {
  return Fl::system_driver()->filename_isdir(n);
}

int Fl_System_Driver::filename_isdir(const char* n) {
  struct stat	s;
  char		fn[FL_PATH_MAX];
  int		length;
  length = (int) strlen(n);
  // Matt: Just in case, we strip the slash for other operating
  // systems as well, avoid bugs by sloppy implementations
  // of "stat".
  if (length > 1 && n[length - 1] == '/') {
    length --;
    memcpy(fn, n, length);
    fn[length] = '\0';
    n = fn;
  }
  return !stat(n, &s) && (s.st_mode & S_IFMT) == S_IFDIR;
}

//
// End of "$Id$".
//
