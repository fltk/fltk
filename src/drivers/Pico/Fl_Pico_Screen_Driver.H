//
// "$Id$"
//
// Definition of Pico Screen interface
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
 \file Fl_Pico_Screen_Driver.H
 \brief Definition of Pico Screen interface.
 */

#ifndef FL_PICO_SCREEN_DRIVER_H
#define FL_PICO_SCREEN_DRIVER_H

#include "../../Fl_Screen_Driver.H"


class FL_EXPORT Fl_Pico_Screen_Driver : public Fl_Screen_Driver
{
public:
  Fl_Pico_Screen_Driver();
  virtual ~Fl_Pico_Screen_Driver();
  virtual void init();
  virtual int x();
  virtual int y();
  virtual int w();
  virtual int h();
  virtual void screen_xywh(int &X, int &Y, int &W, int &H, int n);
  virtual void screen_dpi(float &h, float &v, int n=0);
  virtual void screen_work_area(int &X, int &Y, int &W, int &H, int n);
  // --- audible output
  virtual void beep(int type);
  // --- global events
  virtual void flush();
  virtual int ready();
  virtual void grab(Fl_Window* win);
  // --- global colors
  virtual void get_system_colors();
  // --- global timers
  virtual void add_timeout(double time, Fl_Timeout_Handler cb, void *argp);
  virtual void repeat_timeout(double time, Fl_Timeout_Handler cb, void *argp);
  virtual int has_timeout(Fl_Timeout_Handler cb, void *argp);
  virtual void remove_timeout(Fl_Timeout_Handler cb, void *argp);
};


#endif // FL_PICO_SCREEN_DRIVER_H

//
// End of "$Id$".
//
