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

#define FL_WIN32_TARGET_VERSION 0x0602
#include "../FL/win32_target.h"

#include <windows.h>

int main() {
    return POINTER_FLAG_SECONDBUTTON; /* required symbol */
}
