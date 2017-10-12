//
// "$Id$"
//
// Definition of MSWindows system driver
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
  static unsigned win_pixmap_bg_color; // the RGB() of the pixmap background color
  virtual void warning(const char *format, va_list args);
  virtual void error(const char *format, va_list args);
  virtual void fatal(const char *format, va_list args);
  virtual char *utf2mbcs(const char *s);
  virtual char *getenv(const char* v);
  virtual int putenv(char* v) {return _putenv(v);}
  virtual int open(const char* f, int oflags, int pmode);
  virtual int open_ext(const char* f, int translation, int oflags, int pmode);
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
  virtual unsigned utf8towc(const char* src, unsigned srclen, wchar_t* dst, unsigned dstlen);
  virtual unsigned utf8fromwc(char* dst, unsigned dstlen, const wchar_t* src, unsigned srclen);
  virtual int utf8locale();
  virtual unsigned utf8to_mb(const char* src, unsigned srclen, char* dst, unsigned dstlen);
  virtual unsigned utf8from_mb(char* dst, unsigned dstlen, const char* src, unsigned srclen);
  virtual int clocale_printf(FILE *output, const char *format, va_list args);
  // these 2 are in Fl_get_key_win32.cxx
  virtual int event_key(int k);
  virtual int get_key(int k);
  virtual int filename_list(const char *d, dirent ***list, int (*sort)(struct dirent **, struct dirent **) );
  virtual int filename_expand(char *to,int tolen, const char *from);
  virtual int filename_relative(char *to, int tolen, const char *from, const char *base);
  virtual int filename_absolute(char *to, int tolen, const char *from);
  virtual int filename_isdir(const char* n);
  virtual int filename_isdir_quick(const char* n);
  virtual const char *filename_ext(const char *buf);
  virtual int open_uri(const char *uri, char *msg, int msglen);
  virtual int use_recent_tooltip_fix() {return 1;}
  virtual int file_browser_load_filesystem(Fl_File_Browser *browser, char *filename, int lname, Fl_File_Icon *icon);
  virtual int file_browser_load_directory(const char *directory, char *filename, size_t name_size, dirent ***pfiles, Fl_File_Sort_F *sort);
  virtual void newUUID(char *uuidBuffer);
  virtual char *preference_rootnode(Fl_Preferences *prefs, Fl_Preferences::Root root, const char *vendor,
                                    const char *application);
  virtual void *dlopen(const char *filename);
  virtual void png_extra_rgba_processing(unsigned char *array, int w, int h);
  virtual const char *next_dir_sep(const char *start);
  // these 3 are implemented in Fl_lock.cxx
  virtual void awake(void*);
  virtual int lock();
  virtual void unlock();
  // this one is implemented in Fl_win32.cxx
  virtual void* thread_message();
  virtual int file_type(const char *filename);
  virtual int pixmap_extra_transparent_processing() {return 1;}
  // this one is implemented in fl_draw_pixmap.cxx
  virtual void make_unused_color(unsigned char &r, unsigned char &g, unsigned char &b);
  virtual const char *home_directory_name();
  virtual const char *filesystems_label() { return "My Computer"; }
  virtual int backslash_as_slash() {return 1;}
  virtual int colon_is_drive() {return 1;}
  virtual int case_insensitive_filenames() {return 1;}
  // this one is implemented in Fl_win32.cxx
  virtual const char *filename_name(const char *buf);
  // this one is implemented in Fl_win32.cxx
  virtual void copy(const char *stuff, int len, int clipboard, const char *type);
  // this one is implemented in Fl_win32.cxx
  virtual void paste(Fl_Widget &receiver, int clipboard, const char *type);
  // this one is implemented in Fl_win32.cxx
  virtual int clipboard_contains(const char *type);
  // this one is implemented in Fl_win32.cxx
  virtual void clipboard_notify_change();
  virtual void add_fd(int fd, int when, Fl_FD_Handler cb, void* = 0);
  virtual void add_fd(int fd, Fl_FD_Handler cb, void* = 0);
  virtual void remove_fd(int, int when);
  virtual void remove_fd(int);
  virtual void gettime(time_t *sec, int *usec);
};

#endif // FL_WINAPI_SYSTEM_DRIVER_H

//
// End of "$Id$".
//
