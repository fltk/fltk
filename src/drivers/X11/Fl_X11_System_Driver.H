//
// Definition of Posix system driver
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

#ifndef FL_X11_SYSTEM_DRIVER_H
#define FL_X11_SYSTEM_DRIVER_H

#include <config.h>
#include "../Unix/Fl_Unix_System_Driver.H"

class Fl_X11_System_Driver : public Fl_Unix_System_Driver {
public:
  Fl_X11_System_Driver() : Fl_Unix_System_Driver() {
    // X11 system driver does not use a key table
    key_table = NULL;
    key_table_size = 0;
  }
  virtual void display_arg(const char *arg);
  virtual int XParseGeometry(const char*, int*, int*, unsigned int*, unsigned int*);
  // these 2 are in Fl_get_key.cxx
  virtual int event_key(int k);
  virtual int get_key(int k);
  virtual int need_menu_handle_part1_extra() {return 1;}
  virtual const char *shortcut_add_key_name(unsigned key, char *p, char *buf, const char **);
  virtual void own_colormap();
  // 2 additional virtual members
  virtual int poll_or_select();
  virtual int poll_or_select_with_delay(double time_to_wait);
};

#endif /* FL_X11_SYSTEM_DRIVER_H */
