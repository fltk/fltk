//
// Implementation of the nanosvg library for the Fast Light Tool Kit (FLTK).
//
// Copyright 2017-2024 by Bill Spitzak and others.
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

// This code includes the header-only nanosvg library and builds a
// separate object file comprised of the nanosvg library and nothing
// else. Moved here from Fl_SVG_Image.cxx for better code separation.

#include <config.h>

#if defined(FLTK_USE_SVG) || defined(FL_DOXYGEN)

// GitHub Issue #937: "Support for HP-UX" (version 11.11, Dec. 2000)
//
// C90 does not provide roundf() but nanosvg.h uses it although
// nanosvgrast.h has a replacement function: nsvg__roundf()
//
// Solution: use nsvg_roundf() instead.
// Advantage: we don't need to check system macros.
//
// Note: using nsvg__roundf() in nanosvg.h should be applied upstream.
//       Once this is available we can remove this comment block and
//       the following 3-line "fix":

#include <math.h>                       // must be before #define below !
static float nsvg__roundf(float x);     // prototype (see nanosvgrast.h)
#define roundf nsvg__roundf             // redefinition (#937)

// End of GitHub Issue #937. Remove this entire block when upstream is patched.

#if !defined(HAVE_LONG_LONG)
static double strtoll(const char *str, char **endptr, int base) {
  return (double)strtol(str, endptr, base);
}
#endif

#ifdef _MSC_VER
#pragma warning (disable: 4244)         // Switch off conversion warnings
#endif

#define NANOSVG_ALL_COLOR_KEYWORDS      // include full list of color keywords
#define NANOSVG_IMPLEMENTATION          // use nanosvg.h implementation
#define NANOSVGRAST_IMPLEMENTATION      // use nanosvgrast.h implementation

#include "../nanosvg/nanosvg.h"
#include "../nanosvg/nanosvgrast.h"

#endif // FLTK_USE_SVG
