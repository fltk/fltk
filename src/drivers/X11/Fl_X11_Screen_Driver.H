//
// "$Id$"
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
 \file Fl_X11_Screen_Driver.H
 \brief Definition of X11 Screen interface
 */

#ifndef FL_X11_SCREEN_DRIVER_H
#define FL_X11_SCREEN_DRIVER_H

#include <FL/Fl_Screen_Driver.H>


class Fl_Window;


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
  int poll_or_select();
  int poll_or_select_with_delay(double time_to_wait);

public:
  static int ewmh_supported();
  static void copy_image(const unsigned char* data, int W, int H, int destination);
  // --- display management
  virtual void display(const char *disp);
  virtual int visual(int flags);
  // --- screen configuration
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
  // --- global events
  virtual void flush();
  virtual double wait(double time_to_wait);
  virtual int ready();
  virtual void grab(Fl_Window* win);
  // --- global colors
  virtual int parse_color(const char* p, uchar& r, uchar& g, uchar& b);
  virtual void get_system_colors();
  virtual const char *get_system_scheme();
  // --- global timers
  virtual void add_timeout(double time, Fl_Timeout_Handler cb, void *argp);
  virtual void repeat_timeout(double time, Fl_Timeout_Handler cb, void *argp);
  virtual int has_timeout(Fl_Timeout_Handler cb, void *argp);
  virtual void remove_timeout(Fl_Timeout_Handler cb, void *argp);
  virtual int dnd(int unused);
  virtual int compose(int &del);
  virtual void compose_reset();
  virtual int text_display_can_leak();
  virtual Fl_RGB_Image *read_win_rectangle(uchar *p, int X, int Y, int w, int h, int alpha);
  virtual void get_mouse(int &x, int &y);
  virtual void enable_im();
  virtual void disable_im();
  virtual void open_display();
  virtual void close_display();
  virtual Fl_Font set_fonts(const char *name);
  virtual int get_font_sizes(Fl_Font fnum, int*& sizep);
  virtual const char* get_font_name(Fl_Font fnum, int* ap);
};


#endif // FL_X11_SCREEN_DRIVER_H

//
// End of "$Id$".
//
