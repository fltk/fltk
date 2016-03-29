//
// "$Id: quartz.H 11017 2016-01-20 21:40:12Z matt $"
//
// Definition of MSWindows system driver
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

/**
 \file Fl_WinAPI_System_Driver.H
 \brief Definition of MSWindows system driver.
 */

#ifndef FL_WINAPI_SYSTEM_DRIVER_H
#define FL_WINAPI_SYSTEM_DRIVER_H

#include <FL/Fl_System_Driver.H>
#include <stdarg.h>

/*
 Move everything here that manages the system interface.

 There is exactly one system driver.

 - filename and pathname management
 - directory and file access
 - system time and system timer
 - multithreading
 */

class Fl_WinAPI_System_Driver : public Fl_System_Driver
{
public:
  virtual void warning(const char *format, va_list args);
  virtual void error(const char *format, va_list args);
  virtual void fatal(const char *format, va_list args);
  virtual int compose(int &del);
  virtual int dnd(int unused);
  virtual char *utf2mbcs(const char *s);
  virtual char *getenv(const char* v);
  virtual int open(const char* f, int oflags, int pmode);
  virtual FILE *fopen(const char* f, const char *mode);
  virtual int system(const char* cmd);
  virtual int execvp(const char *file, char *const *argv);
  virtual int chmod(const char* f, int mode);
  virtual int access(const char* f, int mode);
  virtual int stat(const char* f, struct stat *b);
  virtual char *getcwd(char* b, int l);
  virtual int unlink(const char* f);
  virtual int mkdir(const char* f, int mode);
  virtual int rmdir(const char* f);
  virtual int rename(const char* f, const char *n);

};

#endif // FL_WINAPI_SYSTEM_DRIVER_H

//
// End of "$Id: quartz.H 11017 2016-01-20 21:40:12Z matt $".
//
