//
// "$Id: quartz.H 11017 2016-01-20 21:40:12Z matt $"
//
// Definition of Apple Darwin system driver
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
 \file Fl_Darwin_System_Driver.H
 \brief Definition of Apple Darwin system driver.
 */

#ifndef FL_DARWIN_SYSTEM_DRIVER_H
#define FL_DARWIN_SYSTEM_DRIVER_H

#include <FL/Fl_System_Driver.H>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

/*
 Move everything here that manages the system interface.

 There is exactly one system driver.

 - filename and pathname management
 - directory and file access
 - system time and system timer
 - multithreading
 */

class Fl_Darwin_System_Driver : public Fl_System_Driver
{
public:
  Fl_Darwin_System_Driver();
  virtual int single_arg(const char *arg);
  virtual int arg_and_value(const char *name, const char *value);
  virtual int compose(int &del);
  virtual int dnd(int use_selection);
  virtual int mkdir(const char* f, int mode) {return ::mkdir(f, mode);}
  virtual int open(const char* f, int oflags, int pmode) {
    return pmode == -1 ?  ::open(f, oflags) : ::open(f, oflags, pmode);
  }
  virtual FILE *fopen(const char* f, const char *mode) {return ::fopen(f, mode);}
  virtual int system(const char* cmd) {return ::system(cmd);}
  virtual int execvp(const char *file, char *const *argv) {return ::execvp(file, argv);}
  virtual int chmod(const char* f, int mode) {return ::chmod(f, mode);}
  virtual int access(const char* f, int mode) { return ::access(f, mode);}
  virtual int stat(const char* f, struct stat *b) { return ::stat(f, b);}
  virtual char *getcwd(char* b, int l) {return ::getcwd(b, l);}
  virtual int unlink(const char* f) {return ::unlink(f);}
  virtual int rmdir(const char* f) {return ::rmdir(f);}
  virtual int rename(const char* f, const char *n) {return ::rename(f, n);}
  virtual int clocale_printf(FILE *output, const char *format, va_list args);
};

#endif // FL_DARWIN_SYSTEM_DRIVER_H

//
// End of "$Id: quartz.H 11017 2016-01-20 21:40:12Z matt $".
//
