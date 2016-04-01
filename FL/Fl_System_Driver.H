//
// "$Id$"
//
// A base class for platform specific system calls
// for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2016 by Bill Spitzak and others.
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

/** \file Fl_System_Driver.H
 \brief declaration of class Fl_System_Driver.
*/

#ifndef FL_SYSTEM_DRIVER_H
#define FL_SYSTEM_DRIVER_H

#include <FL/Fl_Export.H>
#include <stdio.h>
#include <stdarg.h>

// platform-dependent types are declared here
#ifdef __APPLE__
typedef struct CGContext* Fl_Offscreen;
typedef struct CGImage* Fl_Bitmask;
typedef struct flCocoaRegion* Fl_Region;
typedef int FL_SOCKET;

#elif defined(WIN32)
typedef struct HBITMAP__ *HBITMAP;
typedef HBITMAP Fl_Offscreen;
typedef HBITMAP Fl_Bitmask;
typedef struct HRGN__ *Fl_Region;
# if defined(_WIN64)
typedef  unsigned __int64 FL_SOCKET;
# else
typedef  int FL_SOCKET;
# endif

#elif defined(FL_PORTING)
# pragma message "FL_PORTING: define OS-dependent types"
typedef void* Fl_Offscreen;
typedef void* Fl_Bitmask;
typedef void *Fl_Region;
typedef int FL_SOCKET;

#else
typedef unsigned long Fl_Offscreen;
typedef unsigned long Fl_Bitmask;
typedef struct _XRegion *Fl_Region;
typedef int FL_SOCKET;

#endif // __APPLE__

/**
 \brief A base class for platform-specific system operations.
  */
class FL_EXPORT Fl_System_Driver {
  friend class Fl;
protected:
  // implement once for each platform
  static Fl_System_Driver *newSystemDriver();
  Fl_System_Driver();
public:
  virtual ~Fl_System_Driver();
  // These flags are useful after calling XParseGeometry(). They indicate which of its arguments
  // contain meaningful data upon return.
  static const int flNoValue;
  static const int flWidthValue;
  static const int flHeightValue;
  static const int flXValue;
  static const int flYValue;
  static const int flXNegative;
  static const int flYNegative;
  
  // implement if the system adds unwanted program argument(s)
  virtual int single_arg(const char *arg) { return 0; }
  // implement if the system adds unwanted program argument pair(s)
  virtual int arg_and_value(const char *name, const char *value) { return 0; }
  // implement to process the -display argument
  virtual void display_arg(const char *arg) { }
  // default implementation should be enough
  virtual int XParseGeometry(const char* string, int* x, int* y, unsigned int* width, unsigned int* height);
  static void warning(const char* format, ...);
  // implement to set the default effect of Fl::warning()
  virtual void warning(const char* format, va_list args);
  static void error(const char* format, ...);
  // implement to set the default effect of Fl::error()
  virtual void error(const char* format, va_list args);
  static void fatal(const char* format, ...);
  // implement to set the default effect of Fl::error()
  virtual void fatal(const char* format, va_list args);
  
  // implement these to support cross-platform file operations
  virtual char *utf2mbcs(const char *s) {return (char*)s;}
  virtual char *getenv(const char* v);
  virtual int open(const char* f, int oflags, int pmode) {return -1;}
  virtual FILE *fopen(const char* f, const char *mode) {return NULL;}
  virtual int system(const char* cmd) {return -1;}
  virtual int execvp(const char *file, char *const *argv) {return -1;}
  virtual int chmod(const char* f, int mode) {return -1;}
  virtual int access(const char* f, int mode) { return -1;}
  virtual int stat(const char* f, struct stat *b) { return -1;}
  virtual char *getcwd(char* b, int l) {return NULL;}
  virtual int unlink(const char* f) {return -1;}
  virtual int mkdir(const char* f, int mode) {return -1;}
  virtual int rmdir(const char* f) {return -1;}
  virtual int rename(const char* f, const char *n) {return -1;}
  
  // the default implementation of these utf8... functions should be enough 
  virtual unsigned utf8towc(const char* src, unsigned srclen, wchar_t* dst, unsigned dstlen);
  virtual unsigned utf8fromwc(char* dst, unsigned dstlen, const wchar_t* src, unsigned srclen);
  virtual int utf8locale();
  virtual unsigned utf8to_mb(const char* src, unsigned srclen, char* dst, unsigned dstlen);
  virtual unsigned utf8from_mb(char* dst, unsigned dstlen, const char* src, unsigned srclen);
  // implement to shield fprintf() from locale changes in decimal point
  virtual int clocale_printf(FILE *output, const char *format, va_list args);
};

#endif // FL_SYSTEM_DRIVER_H

//
// End of "$Id$".
//
