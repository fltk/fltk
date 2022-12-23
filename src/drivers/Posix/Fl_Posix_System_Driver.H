//
// Definition of POSIX system driver
// for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2022 by Bill Spitzak and others.
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

/**
 \file Fl_Posix_System_Driver.H
 \brief Definition of Posix system driver.
 */

#ifndef FL_POSIX_SYSTEM_DRIVER_H
#define FL_POSIX_SYSTEM_DRIVER_H

#include <config.h>
#include "../../Fl_System_Driver.H"
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
 - string management
 */

class Fl_Posix_System_Driver : public Fl_System_Driver
{
protected:
  int run_program(const char *program, char **argv, char *msg, int msglen);
public:
  int mkdir(const char* f, int mode) FL_OVERRIDE {return ::mkdir(f, mode);}
  int open(const char* f, int oflags, int pmode) FL_OVERRIDE {
    return pmode == -1 ?  ::open(f, oflags) : ::open(f, oflags, pmode);
  }
  char *getenv(const char *v) FL_OVERRIDE { return ::getenv(v); }
  int putenv(const char *var) FL_OVERRIDE {return ::putenv(strdup(var));}
  int system(const char* cmd) FL_OVERRIDE {return ::system(cmd);}
  int execvp(const char *file, char *const *argv) FL_OVERRIDE {return ::execvp(file, argv);}
  int chmod(const char* f, int mode) FL_OVERRIDE {return ::chmod(f, mode);}
  int access(const char* f, int mode) FL_OVERRIDE { return ::access(f, mode);}
  int flstat(const char* f, struct stat *b) FL_OVERRIDE { return ::stat(f, b);}
  char *getcwd(char* b, int l) FL_OVERRIDE {return ::getcwd(b, l);}
  int chdir(const char* path) FL_OVERRIDE {return ::chdir(path);}
  int unlink(const char* f) FL_OVERRIDE {return ::unlink(f);}
  int rmdir(const char* f) FL_OVERRIDE {return ::rmdir(f);}
  int rename(const char* f, const char *n) FL_OVERRIDE {return ::rename(f, n);}
  const char *getpwnam(const char *login) FL_OVERRIDE;
#if HAVE_DLFCN_H
  void *load(const char *filename) FL_OVERRIDE;
#if HAVE_DLSYM
  static void *ptr_gtk;
  static bool probe_for_GTK(int major, int minor, void **ptr_gtk);
#endif
#endif
  static void *dlopen_or_dlsym(const char *lib_name, const char *func_name = NULL);
  // these 4 are implemented in Fl_lock.cxx
  void awake(void*) FL_OVERRIDE;
  int lock() FL_OVERRIDE;
  void unlock() FL_OVERRIDE;
  void* thread_message() FL_OVERRIDE;
  int file_type(const char *filename) FL_OVERRIDE;
  const char *home_directory_name() FL_OVERRIDE { return ::getenv("HOME"); }
  int dot_file_hidden() FL_OVERRIDE {return 1;}
  void gettime(time_t *sec, int *usec) FL_OVERRIDE;
  char* strdup(const char *s) FL_OVERRIDE {return ::strdup(s);}
  int close_fd(int fd) FL_OVERRIDE;
#if defined(HAVE_PTHREAD)
  void lock_ring() FL_OVERRIDE;
  void unlock_ring() FL_OVERRIDE;
#endif
};

#endif // FL_POSIX_SYSTEM_DRIVER_H
