//
// "$Id: quartz.H 11017 2016-01-20 21:40:12Z matt $"
//
// Definition of Posix system driver
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

#ifndef FL_X11_SYSTEM_DRIVER_H
#define FL_X11_SYSTEM_DRIVER_H

#include "../Posix/Fl_Posix_System_Driver.H"

class Fl_X11_System_Driver : public Fl_Posix_System_Driver {
public:
  Fl_X11_System_Driver() : Fl_Posix_System_Driver() {}
  virtual void display_arg(const char *arg);
  virtual int XParseGeometry(const char*, int*, int*, unsigned int*, unsigned int*);
  virtual int clocale_printf(FILE *output, const char *format, va_list args);
  // these 2 are in Fl_get_key.cxx
  virtual int event_key(int k);
  virtual int get_key(int k);
  virtual int filename_list(const char *d, dirent ***list, int (*sort)(struct dirent **, struct dirent **) );
  virtual int need_menu_handle_part1_extra() {return 1;}
  virtual int open_uri(const char *uri, char *msg, int msglen);
  virtual int use_tooltip_timeout_condition() {return 1;}
  // this one is in fl_shortcut.cxx
  virtual const char *shortcut_add_key_name(unsigned key, char *p, char *buf, const char **);
  virtual int file_browser_load_filesystem(Fl_File_Browser *browser, char *filename, Fl_File_Icon *icon);
  virtual void newUUID(char *uuidBuffer);
  virtual char *preference_rootnode(Fl_Preferences *prefs, Fl_Preferences::Root root, const char *vendor,
                                    const char *application);
  virtual int preferences_need_protection_check() {return 1;} 
};

#endif /* FL_X11_SYSTEM_DRIVER_H */

//
// End of "$Id: quartz.H 11017 2016-01-20 21:40:12Z matt $".
//
