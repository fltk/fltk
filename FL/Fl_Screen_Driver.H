//
// "$Id$"
//
// All screen related calls in a driver style class.
//
// Copyright 1998-2016 by Bill Spitzak and others.
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

#ifndef FL_SCREEN_DRIVER_H
#define FL_SCREEN_DRIVER_H

#include <FL/Fl_Device.H>
#include <FL/x.H>

// TODO: add text composition?
// TODO: add Fl::display
// TODO: add copy/paste, drag/drop?
// TODO: get key/get mouse?
// TODO: system colors/colormaps
// TODO: system menu?
// TODO: native filechooser
// TODO: native message boxes
// TODO: read screen to image
// TODO: application shortcuts

class Fl_Window;


class FL_EXPORT Fl_Screen_Driver {

protected:
  Fl_Screen_Driver();

  static const int MAX_SCREENS = 16;

  int num_screens;

public:
  static char bg_set;
  static char bg2_set;
  static char fg_set;

public:
  static Fl_Screen_Driver *newScreenDriver();
  // --- display management
  virtual void display(const char *disp);
  virtual int visual(int flags);
  // --- screen configuration
  virtual void init() = 0;
  virtual int x() = 0;
  virtual int y() = 0;
  virtual int w() = 0;
  virtual int h() = 0;
  virtual int screen_count();
  virtual void screen_xywh(int &X, int &Y, int &W, int &H);
  virtual void screen_xywh(int &X, int &Y, int &W, int &H, int mx, int my);
  virtual void screen_xywh(int &X, int &Y, int &W, int &H, int n) = 0;
  virtual void screen_xywh(int &X, int &Y, int &W, int &H, int mx, int my, int mw, int mh);
  virtual int screen_num(int x, int y);
  virtual int screen_num(int x, int y, int w, int h);
  virtual void screen_dpi(float &h, float &v, int n=0) = 0;
  virtual void screen_work_area(int &X, int &Y, int &W, int &H, int mx, int my);
  virtual void screen_work_area(int &X, int &Y, int &W, int &H, int n) = 0;
  virtual void screen_work_area(int &X, int &Y, int &W, int &H);
  // --- audible output
  virtual void beep(int type) = 0;
  // --- global events
  virtual void flush() = 0;
  virtual double wait(double time_to_wait) = 0;
  virtual int ready() = 0;
  virtual void grab(Fl_Window* win) = 0;
  // --- global colors
  virtual int parse_color(const char* p, uchar& r, uchar& g, uchar& b) = 0;
  virtual void get_system_colors() = 0;
  virtual const char *get_system_scheme();
};


#endif // !FL_SCREEN_DRIVER_H

//
// End of "$Id$".
//
