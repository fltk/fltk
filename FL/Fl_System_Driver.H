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
#include <FL/platform_types.h>
#include <FL/filename.H>
#include <FL/Fl_Preferences.H>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

class Fl_File_Icon;
class Fl_File_Browser;

/**
 \brief A base class for platform-specific system operations.
  */
class FL_EXPORT Fl_System_Driver {
  friend class Fl;
protected:
  // implement once for each platform
  static Fl_System_Driver *newSystemDriver();
  Fl_System_Driver();
  struct Keyname {unsigned int key; const char* name;};
  static Keyname table[];
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
  virtual int putenv(char* v) {return ::putenv(v);}
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
  // implement functions telling whether a key is pressed
  virtual int event_key(int k) {return 0;}
  virtual int get_key(int k) {return 0;}
  // implement scandir-like function
  virtual int filename_list(const char *d, dirent ***list, int (*sort)(struct dirent **, struct dirent **) ) {return -1;}
  // the default implementation of filename_expand() may be enough
  virtual int filename_expand(char *to, int tolen, const char *from);
  // to implement
  virtual const char *getpwnam(const char *login) {return NULL;}
  // the default implementation of filename_relative() is in src/filename_absolute.cxx and may be enough
  virtual int filename_relative(char *to, int tolen, const char *from, const char *base);
  // the default implementation of filename_absolute() is in src/filename_absolute.cxx and may be enough
  virtual int filename_absolute(char *to, int tolen, const char *from);
  // the default implementation of filename_isdir() is in src/filename_isdir.cxx and may be enough
  virtual int filename_isdir(const char* n);
  // the default implementation of filename_isdir_quick() is in src/filename_isdir.cxx and may be enough
  virtual int filename_isdir_quick(const char* n);
  // the default implementation of filename_ext() is in src/filename_ext.cxx and may be enough
  virtual const char *filename_ext(const char *buf);
  // whether a platform uses additional code in Fl_Menu::handle(int e)
  virtual int need_menu_handle_part2() {return 0;}
  // whether a platform uses additional code in Fl_Menu::handle_part1(int e)
  virtual int need_menu_handle_part1_extra() {return 0;}
  // implement to support fl_open_uri()
  virtual int open_uri(const char *uri, char *msg, int msglen) {return 0;}
  // the default implementation of use_tooltip_timeout_condition() may be enough
  virtual int use_tooltip_timeout_condition() {return 0;}
  // the default implementation of use_recent_tooltip_fix() may be enough
  virtual int use_recent_tooltip_fix() {return 0;}
  // the default implementation of shortcut_add_key_name() is in src/fl_shortcut.cxx
  virtual const char *shortcut_add_key_name(unsigned key, char *p, char *buf, const char **);
  // the default implementation of need_test_shortcut_extra() may be enough
  virtual int need_test_shortcut_extra() {return 0;}
  // implement to support Fl_File_Browser::load()
  virtual int file_browser_load_filesystem(Fl_File_Browser *browser, char *filename, Fl_File_Icon *icon) {return 0;}
  // the default implementation of file_browser_load_directory() should be enough
  virtual int file_browser_load_directory(const char *directory, char *filename, size_t name_size, dirent ***pfiles, Fl_File_Sort_F *sort);
  // implement to support Fl_Preferences
  virtual void newUUID(char *uuidBuffer) { uuidBuffer[0] = 0; }
  // implement to support Fl_Preferences
  virtual char *preference_rootnode(Fl_Preferences *prefs, Fl_Preferences::Root root, const char *vendor,
                                    const char *application) {return NULL;}
  // the default implementation of preferences_need_protection_check() may be enough
  virtual int preferences_need_protection_check() {return 0;}
  // implement to support Fl_Plugin_Manager::load()
  virtual void *dlopen(const char *filename) {return NULL;}
  // the default implementation is most probably enough
  virtual void png_extra_rgba_processing(unsigned char *array, int w, int h) {}
  // the default implementation is most probably enough
  virtual const char *next_dir_sep(const char *start) { return strchr(start, '/');}
  // implement to support threading
  virtual void awake(void*) {}
  virtual int lock() {return 1;}
  virtual void unlock() {}
  virtual void* thread_message() {return NULL;}
};

#endif // FL_SYSTEM_DRIVER_H

//
// End of "$Id$".
//
