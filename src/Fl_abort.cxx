// This method is in it's own source file so that stdlib and stdio
// do not need to be included in Fl.C:
// You can also override this by redefining all of these.

#include <FL/Fl.H>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef WIN32

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
//abort(); // this produces a core dump, probably not desirable?
  ::exit(1);
}

#else

// windows version is currently lame as vsprintf() is missing?

#include <windows.h>

static void warning(const char *format, ...) {
  MessageBox(0,format,"Fltk warning",MB_ICONEXCLAMATION|MB_OK);
}

static void error(const char *format, ...) {
  MessageBox(0,format,"Fltk error",MB_ICONSTOP|MB_SYSTEMMODAL);
  ::exit(1);
}

#endif

void (*Fl::warning)(const char* format, ...) = ::warning;
void (*Fl::error)(const char* format, ...) = ::error;
void (*Fl::fatal)(const char* format, ...) = ::error;
