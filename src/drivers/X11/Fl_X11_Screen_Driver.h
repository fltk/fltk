//
// "$Id: quartz.H 11017 2016-01-20 21:40:12Z matt $"
//
// Definition of X11 Screen interface
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
 \file Fl_X11_Screen_Driver.h
 \brief Definition of X11 Screen interface
 */

#ifndef FL_X11_SCREEN_DRIVER_H
#define FL_X11_SCREEN_DRIVER_H

#include <FL/Fl_Screen_Driver.H>


class FL_EXPORT Fl_X11_Screen_Driver : public Fl_Screen_Driver 
{
protected:
  typedef struct {
    short x_org;
    short y_org;
    short width;
    short height;
  } FLScreenInfo;
  FLScreenInfo screens[MAX_SCREENS];
  float dpi[MAX_SCREENS][2];

public:
  void init_workarea();
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
};


#endif // FL_X11_SCREEN_DRIVER_H

//
// End of "$Id: quartz.H 11017 2016-01-20 21:40:12Z matt $".
//
