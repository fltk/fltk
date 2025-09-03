//
// Definition of Apple Darwin system driver
// for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2021 by Bill Spitzak and others.
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
  int single_arg(const char *arg) FL_OVERRIDE;
  int arg_and_value(const char *name, const char *value) FL_OVERRIDE;
  int clocale_vprintf(FILE *output, const char *format, va_list args) FL_OVERRIDE;
  int clocale_vsnprintf(char *output, size_t output_size, const char *format, va_list args) FL_OVERRIDE;
  int clocale_vsscanf(const char *input, const char *format, va_list args) FL_OVERRIDE;
  static void *get_carbon_function(const char *name);
  static int calc_mac_os_version(); // computes the fl_mac_os_version global variable
  static unsigned short *compute_macKeyLookUp();

  int filename_list(const char *d, dirent ***list,
                            int (*sort)(struct dirent **, struct dirent **),
                            char *errmsg=NULL, int errmsg_sz=0) FL_OVERRIDE;
  int open_uri(const char *uri, char *msg, int msglen) FL_OVERRIDE;
  int need_test_shortcut_extra() FL_OVERRIDE {return 1;}
  int file_browser_load_filesystem(Fl_File_Browser *browser, char *filename, int lname, Fl_File_Icon *icon) FL_OVERRIDE;
  void newUUID(char *uuidBuffer) FL_OVERRIDE;
  char *preference_rootnode(Fl_Preferences *prefs, Fl_Preferences::Root root, const char *vendor,
                                   const char *application) FL_OVERRIDE;
  const char *local_to_latin1(const char *t, int n) FL_OVERRIDE;
  const char *latin1_to_local(const char *t, int n) FL_OVERRIDE;
  const char *local_to_mac_roman(const char *t, int n) FL_OVERRIDE;
  const char *mac_roman_to_local(const char *t, int n) FL_OVERRIDE;
  void tree_draw_expando_button(int x, int y, bool state, bool active) FL_OVERRIDE;
  int tree_connector_style() FL_OVERRIDE;
  const char *filename_name(const char *buf) FL_OVERRIDE;
  void add_fd(int fd, int when, Fl_FD_Handler cb, void* = 0) FL_OVERRIDE;
  void add_fd(int fd, Fl_FD_Handler cb, void* = 0) FL_OVERRIDE;
  void remove_fd(int, int when) FL_OVERRIDE;
  void remove_fd(int) FL_OVERRIDE;
  void open_callback(void (*)(const char *)) FL_OVERRIDE;
  const char *shift_name() FL_OVERRIDE;
  const char *meta_name() FL_OVERRIDE;
  const char *alt_name() FL_OVERRIDE;
  const char *control_name() FL_OVERRIDE;
  Fl_Sys_Menu_Bar_Driver *sys_menu_bar_driver() FL_OVERRIDE;
  double wait(double time_to_wait) FL_OVERRIDE;
  int ready() FL_OVERRIDE;
};

#endif // FL_DARWIN_SYSTEM_DRIVER_H
