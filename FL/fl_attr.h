/*
 * Function attribute declarations for the Fast Light Tool Kit (FLTK).
 *
 * Copyright 1998-2021 by Bill Spitzak and others.
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

#ifndef _FL_fl_attr_h_
#define _FL_fl_attr_h_

/*
  The GNUC-specific attribute appearing below in prototypes with a variable
  list of arguments helps detection of mismatches between format string and
  argument list at compilation time.

  Examples: see fl_ask.H
*/

#ifdef __GNUC__
#  define __fl_attr(x) __attribute__ (x)
#else
#  define __fl_attr(x)
#endif

#endif /* !_FL_fl_attr_h_ */
