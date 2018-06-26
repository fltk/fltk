//
// "$Id$"
//
// Definition of Pico system driver
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

#ifndef FL_PICO_SYSTEM_DRIVER_H
#define FL_PICO_SYSTEM_DRIVER_H

#include "../../Fl_System_Driver.H"

class Fl_Pico_System_Driver : public Fl_System_Driver {
public:
  Fl_Pico_System_Driver() : Fl_System_Driver() {}
  virtual void add_fd(int fd, int when, Fl_FD_Handler cb, void* = 0);
  virtual void add_fd(int fd, Fl_FD_Handler cb, void* = 0);
  virtual void remove_fd(int, int when);
  virtual void remove_fd(int);
};

#endif /* FL_PICO_SYSTEM_DRIVER_H */

//
// End of "$Id$".
//
