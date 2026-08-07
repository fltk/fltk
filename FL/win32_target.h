/*
 *  Define min. Windows version macros for the Fast Light Tool Kit (FLTK).
 *
 * Copyright 1998-2026 by Bill Spitzak and others.
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

/**
  Define the minimum Windows target version for compilation.

  Usage:
  \code
    #define FL_WIN32_TARGET_VERSION 0x0602  // Windows 8
    #include <FL/win32_target.h>
    #include <windows.h>
  \endcode

  This file should be included at the very beginning of the source file.
  It should \b not be included in header files to avoid unwanted side
  effects: including a header file that wants a higher target version
  would silently raise the target version for the current source file.
  Although this would usually be benign, it shouldn't be done.

  One notable exception is that you may have to #include fl_config.h or
  config.h before this file to determine the required target version,
  but this should be rare.

  For specific version numbers please refer to Microsoft documentation at
  https://learn.microsoft.com/en-us/cpp/porting/modifying-winver-and-win32-winnt

  This is an excerpt as of July 2026:
  \code
    //
    // _WIN32_WINNT version constants
    //
    #define _WIN32_WINNT_NT4          0x0400 // Windows NT 4.0
    #define _WIN32_WINNT_WIN2K        0x0500 // Windows 2000
    #define _WIN32_WINNT_WINXP        0x0501 // Windows XP
    #define _WIN32_WINNT_WS03         0x0502 // Windows Server 2003
    #define _WIN32_WINNT_WIN6         0x0600 // Windows Vista
    #define _WIN32_WINNT_VISTA        0x0600 // Windows Vista
    #define _WIN32_WINNT_WS08         0x0600 // Windows Server 2008
    #define _WIN32_WINNT_LONGHORN     0x0600 // Windows Vista
    #define _WIN32_WINNT_WIN7         0x0601 // Windows 7
    #define _WIN32_WINNT_WIN8         0x0602 // Windows 8
    #define _WIN32_WINNT_WINBLUE      0x0603 // Windows 8.1
    #define _WIN32_WINNT_WIN10        0x0A00 // Windows 10
  \endcode
*/

#ifndef FL_WIN32_TARGET_VERSION
#  error "Please define FL_WIN32_TARGET_VERSION before including FL/win32_target.h"
#endif

/* _WIN32_WINNT */
#ifndef _WIN32_WINNT
#  define _WIN32_WINNT FL_WIN32_TARGET_VERSION
#elif _WIN32_WINNT < FL_WIN32_TARGET_VERSION
#  undef _WIN32_WINNT
#  define _WIN32_WINNT FL_WIN32_TARGET_VERSION
#endif

/* WINVER */
#ifndef WINVER
#  define WINVER FL_WIN32_TARGET_VERSION
#elif WINVER < FL_WIN32_TARGET_VERSION
#  undef WINVER
#  define WINVER FL_WIN32_TARGET_VERSION
#endif

// Note: we can't undefine the macro because this would break the build.
// #undef FL_WIN32_TARGET_VERSION
