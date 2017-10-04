//
// "$Id$"
//
// Definition of Posix system driver
// for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2017 by Bill Spitzak and others.
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
 \file Fl_Posix_System_Driver.H
 \brief Definition of Posix system driver.
 */

#ifndef FL_POSIX_SYSTEM_DRIVER_H
#define FL_POSIX_SYSTEM_DRIVER_H

#include <FL/Fl_System_Driver.H>
#include <stdlib.h>
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

class Fl_Posix_System_Driver : public Fl_System_Driver
{
protected:
  int run_program(const char *program, char **argv, char *msg, int msglen);
public:
  Fl_Posix_System_Driver() {}
  virtual int mkdir(const char* f, int mode) {return ::mkdir(f, mode);}
  virtual int open(const char* f, int oflags, int pmode) {
    return pmode == -1 ?  ::open(f, oflags) : ::open(f, oflags, pmode);
  }
  virtual char *getenv(const char *v) { return ::getenv(v); }  
  virtual int putenv(char* v) {return ::putenv(v);}
  virtual int system(const char* cmd) {return ::system(cmd);}
  virtual int execvp(const char *file, char *const *argv) {return ::execvp(file, argv);}
  virtual int chmod(const char* f, int mode) {return ::chmod(f, mode);}
  virtual int access(const char* f, int mode) { return ::access(f, mode);}
  virtual int stat(const char* f, struct stat *b) { return ::stat(f, b);}
  virtual char *getcwd(char* b, int l) {return ::getcwd(b, l);}
  virtual int unlink(const char* f) {return ::unlink(f);}
  virtual int rmdir(const char* f) {return ::rmdir(f);}
  virtual int rename(const char* f, const char *n) {return ::rename(f, n);}
  virtual const char *getpwnam(const char *login);
  virtual int need_menu_handle_part2() {return 1;}
  virtual void *dlopen(const char *filename);
  // these 4 are implemented in Fl_lock.cxx
  virtual void awake(void*);
  virtual int lock();
  virtual void unlock();
  virtual void* thread_message();
  virtual int file_type(const char *filename);
  virtual const char *home_directory_name() { return ::getenv("HOME"); }
  virtual int dot_file_hidden() {return 1;}
  virtual void gettime(time_t *sec, int *usec);
  virtual void* gzopen(const char *fname, const char *mode);
};

#endif // FL_POSIX_SYSTEM_DRIVER_H

//
// End of "$Id$".
//
