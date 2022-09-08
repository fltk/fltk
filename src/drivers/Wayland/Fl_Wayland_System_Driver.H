//
// Definition of Wayland system driver
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

#ifndef FL_WAYLAND_SYSTEM_DRIVER_H
#define FL_WAYLAND_SYSTEM_DRIVER_H

#include "../Unix/Fl_Unix_System_Driver.H"

class Fl_Wayland_System_Driver : public Fl_Unix_System_Driver {
public:
  virtual int need_menu_handle_part2() {return 0;}
  int event_key(int k);
  int get_key(int k);
  virtual void *control_maximize_button(void *data);
  virtual void disable_wayland();
  static bool too_late_to_disable;
};

#endif /* FL_WAYLAND_SYSTEM_DRIVER_H */
