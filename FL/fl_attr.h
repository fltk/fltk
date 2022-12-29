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

/**
  This macro makes it safe to use the C++11 keyword \c override with
  older compilers.
*/
#define FL_OVERRIDE override

#else

#ifdef __cplusplus

// Visual Studio defines __cplusplus = '199711L' which is not helpful.
// We assume that Visual Studio 2015 (1900) and later support the
// 'override' keyword. For VS version number encoding see:
// https://learn.microsoft.com/en-us/cpp/preprocessor/predefined-macros

#if defined(_MSC_VER) && (_MSC_VER >= 1900)

#define FL_OVERRIDE override

#else // not Visual Studio or an older version

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
#else
// replace non-existing `override` with no-op
#define FL_OVERRIDE
#endif

#if (__cplusplus >= 199711L)
// put here definitions applying to C++98 and above
#endif

#endif /* not Visual Studio */

#else
/* C, not C++ */

#endif /* __cplusplus */

#endif /* FL_DOXYGEN */


#endif /* !_FL_fl_attr_h_ */
