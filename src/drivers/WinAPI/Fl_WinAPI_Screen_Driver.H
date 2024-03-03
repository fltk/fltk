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
  enum APP_SCALING_CAPABILITY scaling_capability;
  void update_scaling_capability();
  Fl_WinAPI_Screen_Driver();
  // --- display management
  int visual(int flags) FL_OVERRIDE;
  // --- screen configuration
  void init() FL_OVERRIDE;
  int x() FL_OVERRIDE;
  int y() FL_OVERRIDE;
  int w() FL_OVERRIDE;
  int h() FL_OVERRIDE;
  void screen_xywh(int &X, int &Y, int &W, int &H, int n) FL_OVERRIDE;
  void screen_xywh_unscaled(int &X, int &Y, int &W, int &H, int n);
  void screen_dpi(float &h, float &v, int n=0) FL_OVERRIDE;
  int screen_num_unscaled(int x, int y);
  void screen_work_area(int &X, int &Y, int &W, int &H, int n) FL_OVERRIDE;
  // --- audible output
  void beep(int type) FL_OVERRIDE;
  // --- global events
  void flush() FL_OVERRIDE;
  void grab(Fl_Window* win) FL_OVERRIDE;
  // --- global colors
  void get_system_colors() FL_OVERRIDE;
  int dnd(int unused) FL_OVERRIDE;
  int compose(int &del) FL_OVERRIDE;
  Fl_RGB_Image *read_win_rectangle(int X, int Y, int w, int h, Fl_Window *win, bool may_capture_subwins, bool *did_capture_subwins) FL_OVERRIDE;
  Fl_RGB_Image *read_win_rectangle_unscaled(int X, int Y, int w, int h, Fl_Window *win);
  int get_mouse(int &x, int &y) FL_OVERRIDE;
  void enable_im() FL_OVERRIDE;
  void disable_im() FL_OVERRIDE;
  void open_display_platform() FL_OVERRIDE;
  void offscreen_size(Fl_Offscreen off, int &width, int &height) FL_OVERRIDE;
  APP_SCALING_CAPABILITY rescalable() FL_OVERRIDE {
    return scaling_capability;
  }
  float scale(int n) FL_OVERRIDE {
    return scale_of_screen[n];
  }
  void scale(int n, float f) FL_OVERRIDE {
    scale_of_screen[n] = f;
  }
  void desktop_scale_factor() FL_OVERRIDE;
  void default_icons(const Fl_RGB_Image *icons[], int count) FL_OVERRIDE;
  // this one is implemented in Fl_win32.cxx
  void copy(const char *stuff, int len, int clipboard, const char *type) FL_OVERRIDE;
  // this one is implemented in Fl_win32.cxx
  void paste(Fl_Widget &receiver, int clipboard, const char *type) FL_OVERRIDE;
  // this one is implemented in Fl_win32.cxx
  int clipboard_contains(const char *type) FL_OVERRIDE;
  // this one is implemented in Fl_win32.cxx
  void clipboard_notify_change() FL_OVERRIDE;
  // this one is implemented in Fl_win32.cxx
  void set_spot(int font, int size, int X, int Y, int W, int H, Fl_Window *win) FL_OVERRIDE;
  // these two are implemented in Fl_get_key_win32.cxx
  int event_key(int) FL_OVERRIDE;
  int get_key(int) FL_OVERRIDE;
  float base_scale(int numscreen) FL_OVERRIDE;
};


#endif // FL_WINAPI_SCREEN_DRIVER_H
