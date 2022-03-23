//
// Definition of Wayland system driver
// for the Fast Light Tool Kit (FLTK).
//
// Copyright 2021-2022 by Bill Spitzak and others.
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

#ifndef FL_NIX_SYSTEM_DRIVER_H
#define FL_NIX_SYSTEM_DRIVER_H

#include "../Posix/Fl_Posix_System_Driver.H"
class Fl_RGB_Image;

class FL_EXPORT Fl_Unix_System_Driver : public Fl_Posix_System_Driver {
public:
  virtual int clocale_vsnprintf(char *output, size_t output_size, const char *format, va_list args);
  virtual int clocale_vsscanf(const char *input, const char *format, va_list args);
  virtual int clocale_vprintf(FILE *output, const char *format, va_list args);
  virtual int filename_list(const char *d, dirent ***list,
                            int (*sort)(struct dirent **, struct dirent **),
                            char *errmsg=NULL, int errmsg_sz=0);
  virtual int open_uri(const char *uri, char *msg, int msglen);
  virtual int use_tooltip_timeout_condition() {return 1;}
  virtual int file_browser_load_filesystem(Fl_File_Browser *browser, char *filename, int lname, Fl_File_Icon *icon);
  virtual void newUUID(char *uuidBuffer);
  virtual char *preference_rootnode(Fl_Preferences *prefs, Fl_Preferences::Root root, const char *vendor,
                                    const char *application);
  virtual int preferences_need_protection_check() {return 1;}
  virtual int utf8locale();
  virtual const char *filename_name(const char *buf);
  virtual void add_fd(int fd, int when, Fl_FD_Handler cb, void* = 0);
  virtual void add_fd(int fd, Fl_FD_Handler cb, void* = 0);
  virtual void remove_fd(int, int when);
  virtual void remove_fd(int);
  double wait(double time_to_wait);
  int ready();
  // 3 additional virtual members
  virtual int poll_or_select_with_delay(double time_to_wait);
  virtual int poll_or_select();
  virtual void *control_maximize_button(void *data);
  static unsigned char *create_bmp(const unsigned char *data, int W, int H, int *return_size);
  static Fl_RGB_Image *own_bmp_to_RGB(char *bmp);
  static void read_int(uchar *c, int& i);
};

#endif /* FL_NIX_SYSTEM_DRIVER_H */
