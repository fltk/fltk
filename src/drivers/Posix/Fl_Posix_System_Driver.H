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
  int mkdir(const char* f, int mode) override {return ::mkdir(f, mode);}
  int open(const char* f, int oflags, int pmode) override {
    return pmode == -1 ?  ::open(f, oflags) : ::open(f, oflags, pmode);
  }
  char *getenv(const char *v) override { return ::getenv(v); }
  int putenv(const char *var) override {return ::putenv(strdup(var));}
  int system(const char* cmd) override {return ::system(cmd);}
  int execvp(const char *file, char *const *argv) override {return ::execvp(file, argv);}
  int chmod(const char* f, int mode) override {return ::chmod(f, mode);}
  int access(const char* f, int mode) override { return ::access(f, mode);}
  int flstat(const char* f, struct stat *b) override { return ::stat(f, b);}
  char *getcwd(char* b, int l) override {return ::getcwd(b, l);}
  int chdir(const char* path) override {return ::chdir(path);}
  int unlink(const char* f) override {return ::unlink(f);}
  int rmdir(const char* f) override {return ::rmdir(f);}
  int rename(const char* f, const char *n) override {return ::rename(f, n);}
  const char *getpwnam(const char *login) override;
#if HAVE_DLFCN_H
  void *load(const char *filename) override;
#if HAVE_DLSYM
  static void *ptr_gtk;
  static bool probe_for_GTK(int major, int minor, void **ptr_gtk);
#endif
#endif
  static void *dlopen_or_dlsym(const char *lib_name, const char *func_name = NULL);
  // these 4 are implemented in Fl_lock.cxx
  void awake(void*) override;
  int lock() override;
  void unlock() override;
  void* thread_message() override;
  int file_type(const char *filename) override;
  const char *home_directory_name() override { return ::getenv("HOME"); }
  int dot_file_hidden() override {return 1;}
  void gettime(time_t *sec, int *usec) override;
  char* strdup(const char *s) override {return ::strdup(s);}
  int close_fd(int fd) override;
#if defined(HAVE_PTHREAD)
  void lock_ring() override;
  void unlock_ring() override;
#endif
};

#endif // FL_POSIX_SYSTEM_DRIVER_H
