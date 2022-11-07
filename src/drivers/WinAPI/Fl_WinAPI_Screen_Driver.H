//
// Windows screen interface for the Fast Light Tool Kit (FLTK).
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

/**
 \file Fl_WinAPI_Screen_Driver.H
 \brief Definition of Windows screen interface.
 */

#ifndef FL_WINAPI_SCREEN_DRIVER_H
#define FL_WINAPI_SCREEN_DRIVER_H

#include "../../Fl_Screen_Driver.H"
#include <windows.h>

class Fl_Window;


class Fl_WinAPI_Screen_Driver : public Fl_Screen_Driver
{
protected:
  RECT screens[MAX_SCREENS];
  RECT work_area[MAX_SCREENS];
  float scale_of_screen[MAX_SCREENS];

  static BOOL CALLBACK screen_cb(HMONITOR mon, HDC, LPRECT r, LPARAM);
  BOOL screen_cb(HMONITOR mon, HDC, LPRECT r);
  int get_mouse_unscaled(int &mx, int &my);

public:
  float dpi[MAX_SCREENS][2];
  Fl_WinAPI_Screen_Driver() : Fl_Screen_Driver() {
    for (int i = 0; i < MAX_SCREENS; i++) scale_of_screen[i] = 1;
  }
  // --- display management
  virtual int visual(int flags);
  // --- screen configuration
  virtual void init();
  virtual int x();
  virtual int y();
  virtual int w();
  virtual int h();
  virtual void screen_xywh(int &X, int &Y, int &W, int &H, int n);
  void screen_xywh_unscaled(int &X, int &Y, int &W, int &H, int n);
  virtual void screen_dpi(float &h, float &v, int n=0);
  int screen_num_unscaled(int x, int y);
  virtual void screen_work_area(int &X, int &Y, int &W, int &H, int n);
  // --- audible output
  virtual void beep(int type);
  // --- global events
  virtual void flush();
  virtual void grab(Fl_Window* win);
  // --- global colors
  virtual void get_system_colors();
  virtual int dnd(int unused);
  virtual int compose(int &del);
  virtual Fl_RGB_Image *read_win_rectangle(int X, int Y, int w, int h, Fl_Window *win, bool may_capture_subwins, bool *did_capture_subwins);
  Fl_RGB_Image *read_win_rectangle_unscaled(int X, int Y, int w, int h, Fl_Window *win);
  virtual int get_mouse(int &x, int &y);
  virtual void enable_im();
  virtual void disable_im();
  virtual void open_display_platform();
  virtual void offscreen_size(Fl_Offscreen off, int &width, int &height);
  virtual APP_SCALING_CAPABILITY rescalable() {
    return PER_SCREEN_APP_SCALING;
  }
  virtual float scale(int n) {
    return scale_of_screen[n];
  }
  virtual void scale(int n, float f) {
    scale_of_screen[n] = f;
  }
  virtual void desktop_scale_factor();
  virtual void default_icons(const Fl_RGB_Image *icons[], int count);
  // this one is implemented in Fl_win32.cxx
  virtual void copy(const char *stuff, int len, int clipboard, const char *type);
  // this one is implemented in Fl_win32.cxx
  virtual void paste(Fl_Widget &receiver, int clipboard, const char *type);
  // this one is implemented in Fl_win32.cxx
  virtual int clipboard_contains(const char *type);
  // this one is implemented in Fl_win32.cxx
  virtual void clipboard_notify_change();
  // this one is implemented in Fl_win32.cxx
  void set_spot(int font, int size, int X, int Y, int W, int H, Fl_Window *win);
  // these two are implemented in Fl_get_key_win32.cxx
  virtual int event_key(int);
  virtual int get_key(int);
};


#endif // FL_WINAPI_SCREEN_DRIVER_H
