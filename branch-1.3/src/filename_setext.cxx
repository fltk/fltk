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

// Replace .ext with new extension

#include <FL/filename.H>
#include "flstring.h"

/**
   Replaces the extension in \p buf of max.<br>
   size \p buflen with the extension in \p ext.<br>
   If there's no '.' in \p buf, \p ext is appended.<br>
   If \p ext is NULL, behaves as if it were an empty string ("").

   \b Example
   \code
   #include <FL/filename.H>
   [..]
   char buf[FL_PATH_MAX] = "/path/myfile.cxx";
   fl_filename_setext(buf, sizeof(buf), ".txt");      // buf[] becomes "/path/myfile.txt"
   \endcode

   \return buf itself for calling convenience.
*/
char *fl_filename_setext(char *buf, int buflen, const char *ext) {
  char *q = (char *)fl_filename_ext(buf);
  if (ext) {
    strlcpy(q,ext,buflen - (q - buf));
  } else *q = 0;
  return(buf);
}


//
// End of "$Id$".
//
