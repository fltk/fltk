//
// "$Id$"
//
// Definition of MSWindows Win32/64 Screen interface
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
 \file Fl_WinAPI_Screen_Driver.H
 \brief Definition of MSWindows Win32/64 Screen interface.
 */

#ifndef FL_WINAPI_SCREEN_DRIVER_H
#define FL_WINAPI_SCREEN_DRIVER_H

#include <FL/Fl_Screen_Driver.H>
#include <windows.h>

class Fl_Window;


class FL_EXPORT Fl_WinAPI_Screen_Driver : public Fl_Screen_Driver 
{
protected:
  RECT screens[MAX_SCREENS];
  RECT work_area[MAX_SCREENS];
  float dpi[MAX_SCREENS][2];

  static BOOL CALLBACK screen_cb(HMONITOR mon, HDC, LPRECT r, LPARAM);
  BOOL screen_cb(HMONITOR mon, HDC, LPRECT r);
  int screen_num_unscaled(int x, int y);
  int get_mouse_unscaled(int &mx, int &my);

public:
  Fl_WinAPI_Screen_Driver() : Fl_Screen_Driver() { scale_ = 1; }
  // --- display management
  virtual int visual(int flags);
  // --- screen configuration
  virtual void init();
  virtual int x();
  virtual int y();
  virtual int w();
  virtual int h();
  virtual void screen_xywh(int &X, int &Y, int &W, int &H, int n);
  virtual void screen_dpi(float &h, float &v, int n=0);
  float DWM_scaling_factor(int screen_num);
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
  virtual Fl_RGB_Image *read_win_rectangle(uchar *p, int X, int Y, int w, int h, int alpha);
  Fl_RGB_Image *read_win_rectangle_unscaled(uchar *p, int X, int Y, int w, int h, int alpha);
  virtual int get_mouse(int &x, int &y);
  virtual void enable_im();
  virtual void disable_im();
  virtual void open_display_platform();
  virtual void offscreen_size(Fl_Offscreen off, int &width, int &height);
#if defined(FLTK_HIDPI_SUPPORT)  
  virtual APP_SCALING_CAPABILITY rescalable() {
    return SYSTEMWIDE_APP_SCALING;
  }
#endif
  virtual float scale(int n) {return scale_;}
  virtual void scale(int n, float f) { scale_ = f;}
  virtual float desktop_scale_factor();
private:
  float scale_;
};


#endif // FL_WINAPI_SCREEN_DRIVER_H

//
// End of "$Id$".
//
