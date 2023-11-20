//
// Windows system driver for the Fast Light Tool Kit (FLTK).
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
 \file Fl_WinAPI_System_Driver.H
 \brief Definition of Windows system driver.
 */

#ifndef FL_WINAPI_SYSTEM_DRIVER_H
#define FL_WINAPI_SYSTEM_DRIVER_H

#include "../../Fl_System_Driver.H"
#include <stdarg.h>
#include <string.h>  // strdup

/*
 Move everything here that manages the system interface.

 There is exactly one system driver.

 - filename and pathname management
 - directory and file access
 - system time and system timer
 - multithreading
 - string management
 */

class Fl_WinAPI_System_Driver : public Fl_System_Driver
{
public:
  void warning(const char *format, va_list args) FL_OVERRIDE;
  void error(const char *format, va_list args) FL_OVERRIDE;
  void fatal(const char *format, va_list args) FL_OVERRIDE;
  char *utf2mbcs(const char *s) FL_OVERRIDE;
  char *getenv(const char *var) FL_OVERRIDE;
  int putenv(const char *var) FL_OVERRIDE;
  int open(const char *fnam, int oflags, int pmode) FL_OVERRIDE;
  int open_ext(const char *fnam, int binary, int oflags, int pmode) FL_OVERRIDE;
  FILE *fopen(const char *fnam, const char *mode) FL_OVERRIDE;
  int system(const char *cmd) FL_OVERRIDE;
  int execvp(const char *file, char *const *argv) FL_OVERRIDE;
  int chmod(const char *fnam, int mode) FL_OVERRIDE;
  int access(const char *fnam, int mode) FL_OVERRIDE;
  int flstat(const char *fnam, struct stat *b) FL_OVERRIDE;
  char *getcwd(char *b, int l) FL_OVERRIDE;
  int chdir(const char *path) FL_OVERRIDE;
  int unlink(const char *fnam) FL_OVERRIDE;
  int mkdir(const char *fnam, int mode) FL_OVERRIDE;
  int rmdir(const char *fnam) FL_OVERRIDE;
  int rename(const char *fnam, const char *newnam) FL_OVERRIDE;
  // Windows commandline argument conversion to UTF-8
  int args_to_utf8(int argc, char ** &argv) FL_OVERRIDE;
  // Windows specific UTF-8 conversions
  unsigned utf8towc(const char *src, unsigned srclen, wchar_t* dst, unsigned dstlen) FL_OVERRIDE;
  unsigned utf8fromwc(char *dst, unsigned dstlen, const wchar_t* src, unsigned srclen) FL_OVERRIDE;
  int utf8locale() FL_OVERRIDE;
  unsigned utf8to_mb(const char *src, unsigned srclen, char *dst, unsigned dstlen) FL_OVERRIDE;
  unsigned utf8from_mb(char *dst, unsigned dstlen, const char *src, unsigned srclen) FL_OVERRIDE;

  int clocale_vprintf(FILE *output, const char *format, va_list args) FL_OVERRIDE;
  int clocale_vsnprintf(char *output, size_t output_size, const char *format, va_list args) FL_OVERRIDE;
  int clocale_vsscanf(const char *input, const char *format, va_list args) FL_OVERRIDE;
  int filename_list(const char *d, dirent ***list,
                            int (*sort)(struct dirent **, struct dirent **),
                            char *errmsg=NULL, int errmsg_sz=0) FL_OVERRIDE;
  int filename_expand(char *to,int tolen, const char *from) FL_OVERRIDE;
  int filename_relative(char *to, int tolen, const char *from, const char *base) FL_OVERRIDE;
  int filename_absolute(char *to, int tolen, const char *from, const char *base) FL_OVERRIDE;
  int filename_isdir(const char *n) FL_OVERRIDE;
  int filename_isdir_quick(const char *n) FL_OVERRIDE;
  const char *filename_ext(const char *buf) FL_OVERRIDE;
  int open_uri(const char *uri, char *msg, int msglen) FL_OVERRIDE;
  int use_recent_tooltip_fix() FL_OVERRIDE {return 1;}
  int file_browser_load_filesystem(Fl_File_Browser *browser, char *filename, int lname, Fl_File_Icon *icon) FL_OVERRIDE;
  int file_browser_load_directory(const char *directory, char *filename, size_t name_size,
                                          dirent ***pfiles, Fl_File_Sort_F *sort,
                                          char *errmsg=NULL, int errmsg_sz=0) FL_OVERRIDE;
  void newUUID(char *uuidBuffer) FL_OVERRIDE;
  char *preference_rootnode(Fl_Preferences *prefs, Fl_Preferences::Root root, const char *vendor,
                                    const char *application) FL_OVERRIDE;
  void *load(const char *filename) FL_OVERRIDE;
  void png_extra_rgba_processing(unsigned char *array, int w, int h) FL_OVERRIDE;
  const char *next_dir_sep(const char *start) FL_OVERRIDE;
  // these 3 are implemented in Fl_lock.cxx
  void awake(void*) FL_OVERRIDE;
  int lock() FL_OVERRIDE;
  void unlock() FL_OVERRIDE;
  // this one is implemented in Fl_win32.cxx
  void* thread_message() FL_OVERRIDE;
  int file_type(const char *filename) FL_OVERRIDE;
  const char *home_directory_name() FL_OVERRIDE;
  const char *filesystems_label() FL_OVERRIDE { return "My Computer"; }
  int backslash_as_slash() FL_OVERRIDE {return 1;}
  int colon_is_drive() FL_OVERRIDE {return 1;}
  int case_insensitive_filenames() FL_OVERRIDE {return 1;}
  // this one is implemented in Fl_win32.cxx
  const char *filename_name(const char *buf) FL_OVERRIDE;
  void add_fd(int fd, int when, Fl_FD_Handler cb, void* = 0) FL_OVERRIDE;
  void add_fd(int fd, Fl_FD_Handler cb, void* = 0) FL_OVERRIDE;
  void remove_fd(int, int when) FL_OVERRIDE;
  void remove_fd(int) FL_OVERRIDE;
  void gettime(time_t *sec, int *usec) FL_OVERRIDE;
  char* strdup(const char *s) FL_OVERRIDE { return ::_strdup(s); }
  void lock_ring() FL_OVERRIDE;
  void unlock_ring() FL_OVERRIDE;
  double wait(double time_to_wait) FL_OVERRIDE;
  int ready() FL_OVERRIDE;
  int close_fd(int fd) FL_OVERRIDE;
};

#endif // FL_WINAPI_SYSTEM_DRIVER_H
