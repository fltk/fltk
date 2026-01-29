/*
  Test Pen/Tablet support availability (Windows).

  Copyright 2026 by Bill Spitzak and others.

  This library is free software. Distribution and use rights are outlined in
  the file "COPYING" which should have been included with this file.  If this
  file is missing or damaged, see the license at:

      https://www.fltk.org/COPYING.php

  Please see the following page on how to report bugs and issues:

      https://www.fltk.org/bugs.php
*/

/*
  CMake test function: test if this can be compiled.
  If compilation fails, then Pen/Tablet support can't be built and is disabled.
*/

/* We require Windows 8 or later features for Pen/Tablet support */

# if !defined(WINVER) || (WINVER < 0x0602)
#  ifdef WINVER
#   undef WINVER
#  endif
#  define WINVER 0x0602
# endif
# if !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x0602)
#  ifdef _WIN32_WINNT
#   undef _WIN32_WINNT
#  endif
#  define _WIN32_WINNT 0x0602
# endif

#include <windows.h>

int main() {
    return POINTER_CHANGE_FIRSTBUTTON_DOWN; /* required symbol */
}
