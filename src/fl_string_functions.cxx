/*
 * Platform agnostic string portability functions for the Fast Light Tool Kit (FLTK).
 *
 * Copyright 2020 by Bill Spitzak and others.
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

#include <FL/fl_string_functions.h>
#include <string.h>  // strdup/_strdup
#include "Fl_System_Driver.H"

/**
  Cross platform interface to POSIX function strdup().

  The fl_strdup() function returns a pointer to a new string which is
  a duplicate of the string 's'. Memory for the new string is obtained
  with malloc(3), and can be freed with free(3).

  Implementation:
    - POSIX: strdup()
    - WinAPI: _strdup()
 */
char *fl_strdup(const char *s) {
  return Fl::system_driver()->strdup(s);
}

/*
 * 'fl_strlcpy()' - Safely copy two strings.
 */
size_t                          /* O - Length of string */
fl_strlcpy(char       *dst,     /* O - Destination string */
           const char *src,     /* I - Source string */
           size_t      size) {  /* I - Size of destination string buffer */
  size_t        srclen;         /* Length of source string */


  /*
   * Figure out how much room is needed...
   */

  size --;

  srclen = strlen(src);

  /*
   * Copy the appropriate amount...
   */

  if (srclen > size) srclen = size;

  memcpy(dst, src, srclen);
  dst[srclen] = '\0';

  return (srclen);
}


