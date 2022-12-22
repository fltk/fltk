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
  int single_arg(const char *arg) override;
  int arg_and_value(const char *name, const char *value) override;
  int clocale_vprintf(FILE *output, const char *format, va_list args) override;
  int clocale_vsnprintf(char *output, size_t output_size, const char *format, va_list args) override;
  int clocale_vsscanf(const char *input, const char *format, va_list args) override;
  static void *get_carbon_function(const char *name);
  static int calc_mac_os_version(); // computes the fl_mac_os_version global variable
  static unsigned short *compute_macKeyLookUp();

  int filename_list(const char *d, dirent ***list,
                            int (*sort)(struct dirent **, struct dirent **),
                            char *errmsg=NULL, int errmsg_sz=0) override;
  int open_uri(const char *uri, char *msg, int msglen) override;
  int need_test_shortcut_extra() override {return 1;}
  int file_browser_load_filesystem(Fl_File_Browser *browser, char *filename, int lname, Fl_File_Icon *icon) override;
  void newUUID(char *uuidBuffer) override;
  char *preference_rootnode(Fl_Preferences *prefs, Fl_Preferences::Root root, const char *vendor,
                                   const char *application) override;
  const char *local_to_latin1(const char *t, int n) override;
  const char *latin1_to_local(const char *t, int n) override;
  const char *local_to_mac_roman(const char *t, int n) override;
  const char *mac_roman_to_local(const char *t, int n) override;
  Fl_Pixmap *tree_openpixmap() override;
  static const char * const tree_open_xpm_darwin[]; // used by tree_openpixmap()
  Fl_Pixmap *tree_closepixmap() override;
  static const char * const tree_close_xpm_darwin[]; // used by tree_closepixmap()
  int tree_connector_style() override;
  const char *filename_name(const char *buf) override;
  void add_fd(int fd, int when, Fl_FD_Handler cb, void* = 0) override;
  void add_fd(int fd, Fl_FD_Handler cb, void* = 0) override;
  void remove_fd(int, int when) override;
  void remove_fd(int) override;
  void open_callback(void (*)(const char *)) override;
  const char *shift_name() override;
  const char *meta_name() override;
  const char *alt_name() override;
  const char *control_name() override;
  Fl_Sys_Menu_Bar_Driver *sys_menu_bar_driver() override;
  double wait(double time_to_wait) override;
  int ready() override;
};

#endif // FL_DARWIN_SYSTEM_DRIVER_H
