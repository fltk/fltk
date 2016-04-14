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

#include "../Posix/Fl_Posix_System_Driver.H"
#include <stdlib.h>
#include <unistd.h>

/*
 Move everything here that manages the system interface.

 There is exactly one system driver.

 - filename and pathname management
 - directory and file access
 - system time and system timer
 - multithreading
 */

class Fl_Darwin_System_Driver : public Fl_Posix_System_Driver
{
public:
  Fl_Darwin_System_Driver();
  virtual int single_arg(const char *arg);
  virtual int arg_and_value(const char *name, const char *value);
  virtual int clocale_printf(FILE *output, const char *format, va_list args);
  static void *get_carbon_function(const char *name);
  static int calc_mac_os_version(); // computes the fl_mac_os_version global variable
  // these 2 are in Fl_get_key_mac.cxx
  virtual int event_key(int k);
  virtual int get_key(int k);
  virtual int filename_list(const char *d, dirent ***list, int (*sort)(struct dirent **, struct dirent **) );
  virtual int open_uri(const char *uri, char *msg, int msglen);
  virtual int need_test_shortcut_extra() {return 1;}
  virtual int file_browser_load_filesystem(Fl_File_Browser *browser, char *filename, Fl_File_Icon *icon);
  virtual void newUUID(char *uuidBuffer);
  virtual char *preference_rootnode(Fl_Preferences *prefs, Fl_Preferences::Root root, const char *vendor,
                                   const char *application);
  virtual const char *local_to_latin1(const char *t, int n);
  virtual const char *latin1_to_local(const char *t, int n);
  virtual const char *local_to_mac_roman(const char *t, int n);
  virtual const char *mac_roman_to_local(const char *t, int n);
  virtual Fl_Pixmap *tree_openpixmap();
  static const char * const tree_open_xpm_darwin[]; // used by tree_openpixmap()
  virtual Fl_Pixmap *tree_closepixmap();
  static const char * const tree_close_xpm_darwin[]; // used by tree_closepixmap()
  virtual int tree_connector_style();
};

#endif // FL_DARWIN_SYSTEM_DRIVER_H

//
// End of "$Id: quartz.H 11017 2016-01-20 21:40:12Z matt $".
//
