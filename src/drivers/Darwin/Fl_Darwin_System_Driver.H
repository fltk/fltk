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

#include <FL/Fl_System_Driver.H>

/*
 Move everything here that manages the system interface.

 There is exactly one system driver.

 - filename and pathname management
 - directory and file access
 - system time and system timer
 - multithreading
 */

class Fl_Darwin_System_Driver : public Fl_System_Driver
{
public:
  virtual int single_arg(const char *arg);
  virtual int arg_and_value(const char *name, const char *value);
  virtual int compose(int &del);
  virtual int dnd(int use_selection);
};

#endif // FL_DARWIN_SYSTEM_DRIVER_H

//
// End of "$Id: quartz.H 11017 2016-01-20 21:40:12Z matt $".
//
