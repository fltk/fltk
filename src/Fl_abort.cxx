//
// "$Id$"
//
// Warning/error message code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2011 by Bill Spitzak and others.
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

// This method is in its own source file so that stdlib and stdio
// do not need to be included in Fl.cxx:
// You can also override this by redefining all of these.

#include <FL/Fl.H>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "flstring.h"

#ifdef WIN32
#  include <windows.h>

static void warning(const char *, ...) {
  // Show nothing for warnings under WIN32...
}

static void error(const char *format, ...) {
  va_list args;
  char buf[1024];
  va_start(args, format);
  vsnprintf(buf, 1024, format, args);
  va_end(args);
  MessageBox(0,buf,"Error",MB_ICONEXCLAMATION|MB_SYSTEMMODAL);
}

static void fatal(const char *format, ...) {
  va_list args;
  char buf[1024];
  va_start(args, format);
  vsnprintf(buf, 1024, format, args);
  va_end(args);
  MessageBox(0,buf,"Error",MB_ICONSTOP|MB_SYSTEMMODAL);
  ::exit(1);
}

#else

static void warning(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputc('\n', stderr);
  fflush(stderr);
}

static void error(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputc('\n', stderr);
  fflush(stderr);
}

static void fatal(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputc('\n', stderr);
  fflush(stderr);
  ::exit(1);
}

#endif

void (*Fl::warning)(const char* format, ...) = ::warning;
void (*Fl::error)(const char* format, ...) = ::error;
void (*Fl::fatal)(const char* format, ...) = ::fatal;

//
// End of "$Id$".
//
