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


#ifdef FL_DOXYGEN

/// Make FL_OVERRIDE a replacement for \c override for all compiler versions
#define FL_OVERRIDE override

#else

#ifndef __cplusplus
// C, not C++

#undef FL_OVERRIDE

#else

#if (__cplusplus >= 202002L)
// put here definitions applying to C++20 and above
#endif

#if (__cplusplus >= 201703L)
// put here definitions applying to C++17 and above
#endif

#if (__cplusplus >= 201402L)
// put here definitions applying to C++14 and above
#endif

#if (__cplusplus >= 201103L)
// put here definitions applying to C++11 and above
#define FL_OVERRIDE override
#endif

#if (__cplusplus >= 199711L)
// put here definitions applying to C++98 and above
#endif

// C++ before C++98

#ifndef FL_OVERRIDE
#define FL_OVERRIDE
#endif

#endif // __cplusplus

#endif // FL_DOXYGEN


#endif /* !_FL_fl_attr_h_ */
