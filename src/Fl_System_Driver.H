//
// A base class for platform specific system calls
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
 \cond DriverDev
 \addtogroup DriverDeveloper
 \{
 */

/** \file Fl_System_Driver.H
 \brief declaration of class Fl_System_Driver.
*/

/* Class hierarchy
 + Fl_System_Driver
 | + Fl_Posix_System_Driver
 | | + Fl_Unix_System_Driver
 | | + Fl_Darwin_System_Driver
 | + Fl_WinAPI_System_Driver
 */

#ifndef FL_SYSTEM_DRIVER_H
#define FL_SYSTEM_DRIVER_H

#include <FL/Fl.H>
#include <FL/Fl_Export.H>
#include <FL/filename.H>
#include <FL/Fl_Preferences.H>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

class Fl_File_Icon;
class Fl_File_Browser;
class Fl_Pixmap;
class Fl_Widget;
class Fl_Sys_Menu_Bar_Driver;

/**
 \brief A base class for platform-specific system operations.
 This class is only for internal use by the FLTK library.
 Each supported platform implements several of the virtual methods of this class.
  */
class Fl_System_Driver {
  friend class Fl;
protected:
  // implement once for each platform
  static Fl_System_Driver *newSystemDriver();
  Fl_System_Driver();
  static bool awake_ring_empty();
public:
  virtual ~Fl_System_Driver();
  static int command_key;
  static int control_key;

  // implement if the system adds unwanted program argument(s)
  virtual int single_arg(const char *) { return 0; }
  // implement if the system adds unwanted program argument pair(s)
  virtual int arg_and_value(const char * /*name*/, const char * /*value*/) { return 0; }
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
  virtual char *getenv(const char*) {return NULL;}
  virtual int putenv(const char *) {return -1;}
  virtual int open(const char* /*f*/, int /*oflags*/, int /*pmode*/) {return -1;}

  // implement these to support cross-platform string operations
  virtual char *strdup(const char *) {return NULL;}

  // Note: the default implementation ignores the 'binary' argument.
  // Some platforms (notably Windows) may use this argument.
  virtual int open_ext(const char* f, int /*binary*/, int oflags, int pmode) {
    return this->open(f, oflags, pmode);
  }
  virtual FILE *fopen(const char* f, const char *mode);
  virtual int system(const char*) {return -1;}
  virtual int execvp(const char * /*file*/, char *const * /*argv*/) {return -1;}
  virtual int chmod(const char* /*f*/, int /*mode*/) {return -1;}
  virtual int access(const char* /*f*/, int /*mode*/) { return -1;}
  virtual int flstat(const char* /*f*/, struct stat *) { return -1;}
  virtual char *getcwd(char* /*b*/, int /*l*/) {return NULL;}
  virtual int chdir(const char*) {return -1;}
  virtual int unlink(const char*) {return -1;}
  virtual int mkdir(const char* /*f*/, int /*mode*/) {return -1;}
  virtual int rmdir(const char*) {return -1;}
  virtual int rename(const char* /*f*/, const char * /*n*/) {return -1;}

  // Windows commandline argument conversion to UTF-8.
  // Default implementation: no-op, overridden only on Windows
  virtual int args_to_utf8(int argc, char ** &argv) { return argc; }

  // the default implementation of these utf8... functions should be enough
  virtual unsigned utf8towc(const char* src, unsigned srclen, wchar_t* dst, unsigned dstlen);
  virtual unsigned utf8fromwc(char* dst, unsigned dstlen, const wchar_t* src, unsigned srclen);
  virtual int utf8locale() {return 1;}
  virtual unsigned utf8to_mb(const char* src, unsigned srclen, char* dst, unsigned dstlen);
  virtual unsigned utf8from_mb(char* dst, unsigned dstlen, const char* src, unsigned srclen);
  // implement to shield fprintf() from locale changes in decimal point
  virtual int clocale_vprintf(FILE *output, const char *format, va_list args);
  virtual int clocale_vsnprintf(char *output, size_t output_size, const char *format, va_list args);
  virtual int clocale_vsscanf(const char *input, const char *format, va_list args);
  // implement scandir-like function
  virtual int filename_list(const char * /*d*/, dirent ***,
                            int (* /*sort*/)(struct dirent **, struct dirent **),
                            char *errmsg=NULL, int errmsg_sz=0) {
    (void)errmsg; (void)errmsg_sz;
    return -1;
  }
  // the default implementation of filename_expand() may be enough
  virtual int filename_expand(char *to, int tolen, const char *from);
  // to implement
  virtual const char *getpwnam(const char *) {return NULL;}
  // the default implementation of filename_relative() is in src/filename_absolute.cxx and may be enough
  virtual int filename_relative(char *to, int tolen, const char *from, const char *base);
  // the default implementation of filename_absolute() is in src/filename_absolute.cxx and may be enough
  virtual int filename_absolute(char *to, int tolen, const char *from, const char *base);
  // the default implementation of filename_isdir() is in src/filename_isdir.cxx and may be enough
  virtual int filename_isdir(const char* n);
  // the default implementation of filename_isdir_quick() is in src/filename_isdir.cxx and may be enough
  virtual int filename_isdir_quick(const char* n);
  // the default implementation of filename_ext() is in src/filename_ext.cxx and may be enough
  virtual const char *filename_ext(const char *buf);
  // implement to support fl_filename_name()
  virtual const char *filename_name(const char *buf) {return buf;}
  // implement to support fl_open_uri()
  virtual int open_uri(const char * /*uri*/, char * /*msg*/, int /*msglen*/) {return 0;}
  // the default implementation of use_tooltip_timeout_condition() may be enough
  virtual int use_tooltip_timeout_condition() {return 0;}
  // the default implementation of use_recent_tooltip_fix() may be enough
  virtual int use_recent_tooltip_fix() {return 0;}
  // the default implementation of need_test_shortcut_extra() may be enough
  virtual int need_test_shortcut_extra() {return 0;}
  // implement to support Fl_File_Browser::load()
  virtual int file_browser_load_filesystem(Fl_File_Browser *, char * /*filename*/, int /*lname*/, Fl_File_Icon *) {return 0;}
  // the default implementation of file_browser_load_directory() should be enough
  virtual int file_browser_load_directory(const char *directory, char *filename, size_t name_size,
                                          dirent ***pfiles, Fl_File_Sort_F *sort,
                                          char *errmsg=NULL, int errmsg_sz=0);
  // implement to support Fl_Preferences
  virtual void newUUID(char *uuidBuffer) { uuidBuffer[0] = 0; }
  // implement to support Fl_Preferences
  virtual char *preference_rootnode(Fl_Preferences *, Fl_Preferences::Root,
                                    const char * /*vendor*/,
                                    const char * /*application*/) {return NULL;}
  // the default implementation of preferences_need_protection_check() may be enough
  virtual int preferences_need_protection_check() {return 0;}
  // implement to support Fl_Plugin_Manager::load()
  virtual void *load(const char *) {return NULL;}
  // the default implementation is most probably enough
  virtual void png_extra_rgba_processing(unsigned char * /*array*/, int /*w*/, int /*h*/) {}
  // the default implementation is most probably enough
  virtual const char *next_dir_sep(const char *start) { return strchr(start, '/');}
  // implement to support threading
  virtual void awake(void*) {}
  virtual int lock() {return 1;}
  virtual void unlock() {}
  virtual void* thread_message() {return NULL;}
  // implement to support Fl_File_Icon
  virtual int file_type(const char *filename);
  // implement to return the user's home directory name
  virtual const char *home_directory_name() { return ""; }
  // the default implementation is most probably enough
  virtual const char *filesystems_label() { return "File Systems"; }
  // return TRUE means \ same as / in file names
  virtual int backslash_as_slash() {return 0;}
  // return TRUE means : indicates a drive letter in file names
  virtual int colon_is_drive() {return 0;}
  // return TRUE means that files whose name begins with dot are hidden
  virtual int dot_file_hidden() {return 0;}
  // return TRUE when file names are case insensitive
  virtual int case_insensitive_filenames() {return 0;}
  // the implementations of local_to_latin1() and latin1_to_local() are in fl_encoding_latin1.cxx
  virtual const char *local_to_latin1(const char *t, int n);
  virtual const char *latin1_to_local(const char *t, int n);
  // the implementations of local_to_mac_roman() and mac_roman_to_local() are in fl_encoding_mac_roman.cxx
  virtual const char *local_to_mac_roman(const char *t, int n);
  virtual const char *mac_roman_to_local(const char *t, int n);
  // draw default tree view expando button
  virtual void tree_draw_expando_button(int x, int y, bool state, bool active);
  // the default implementation of tree_connector_style() is in Fl_Tree_Prefs.cxx and can be enough
  virtual int tree_connector_style();
  virtual void add_fd(int fd, int when, Fl_FD_Handler cb, void* = 0);
  virtual void add_fd(int fd, Fl_FD_Handler cb, void* = 0);
  virtual void remove_fd(int, int when);
  virtual void remove_fd(int);
  // the default implementation of open_callback() may be enough
  virtual void open_callback(void (*)(const char *));
  // The default implementation may be enough.
  virtual void gettime(time_t *sec, int *usec);
  // The default implementation of the next 4 functions may be enough.
  virtual const char *shift_name() { return "Shift"; }
  virtual const char *meta_name() { return "Meta"; }
  virtual const char *alt_name() { return "Alt"; }
  virtual const char *control_name() { return "Ctrl"; }
  virtual Fl_Sys_Menu_Bar_Driver *sys_menu_bar_driver() { return NULL; }
  virtual void lock_ring() {}
  virtual void unlock_ring() {}
  virtual double wait(double);                             // must FL_OVERRIDE
  virtual int ready() { return 0; }                        // must FL_OVERRIDE
  virtual int close_fd(int) {return -1;} // to close a file descriptor
};

#endif // FL_SYSTEM_DRIVER_H

/**
 \}
 \endcond
 */
