/*
 * Internal UTF-8 header file for the Fast Light Tool Kit (FLTK).
 *
 * Copyright 1998-2016 by Bill Spitzak and others.
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

/*
  ----------------
  Note to editors:
  ----------------

  This file may only contain common, platform-independent function
  declarations used internally in FLTK. It may be #included everywhere
  in source files in the library, but not in public header files.
*/

#ifndef _SRC__FL_UTF8_H
#define _SRC__FL_UTF8_H

#  ifdef __cplusplus
extern "C" {
#  endif

unsigned short
XUtf8IsNonSpacing(
        unsigned int ucs);

unsigned short
XUtf8IsRightToLeft(
        unsigned int ucs);


int
XUtf8Tolower(
        int ucs);

int
XUtf8Toupper(
        int ucs);


#  ifdef __cplusplus
}
#  endif

#endif /* _SRC__FL_UTF8_H */
