//
// Definition of X11 Screen interface
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

/**
 \file Fl_X11_Screen_Driver.H
 \brief Definition of X11 Screen interface
 */

#ifndef FL_X11_SCREEN_DRIVER_H
#define FL_X11_SCREEN_DRIVER_H

#include <config.h>
#include "../Unix/Fl_Unix_Screen_Driver.H"
#include <X11/Xlib.h>


class Fl_Window;


class Fl_X11_Screen_Driver : public Fl_Unix_Screen_Driver
{
  friend class Fl_Screen_Driver;
protected:
  typedef struct {
    short x_org;
    short y_org;
    short width;
    short height;
#if USE_XFT
    float scale;
#endif
  } FLScreenInfo;
  FLScreenInfo screens[MAX_SCREENS];
  float dpi[MAX_SCREENS][2];
  int get_mouse_unscaled(int &xx, int &yy);

public:
#if USE_XFT // scaling does not work without Xft
  float current_xft_dpi; // current value of the Xft.dpi X resource
  APP_SCALING_CAPABILITY rescalable() FL_OVERRIDE { return PER_SCREEN_APP_SCALING; }
  float scale(int n) FL_OVERRIDE {return screens[n].scale;}
  void scale(int n, float f) FL_OVERRIDE { screens[n].scale = f;}
  void desktop_scale_factor() FL_OVERRIDE;
  int screen_num_unscaled(int x, int y);
#endif

  Fl_X11_Screen_Driver();
  static int ewmh_supported();
  static void copy_image(const unsigned char* data, int W, int H, int destination);
  // --- display management
  void display(const char *disp) FL_OVERRIDE;
  int XParseGeometry(const char*, int*, int*, unsigned int*, unsigned int*) FL_OVERRIDE;
  int poll_or_select_with_delay(double time_to_wait) FL_OVERRIDE;
  int poll_or_select() FL_OVERRIDE;
  void own_colormap() FL_OVERRIDE;
  const char *shortcut_add_key_name(unsigned key, char *p, char *buf, const char **) FL_OVERRIDE;
  int need_menu_handle_part1_extra() FL_OVERRIDE {return 1;}
  int need_menu_handle_part2() FL_OVERRIDE {return 1;}
  // these 2 are in Fl_get_key.cxx
  int event_key(int) FL_OVERRIDE;
  int get_key(int) FL_OVERRIDE;
  int visual(int flags) FL_OVERRIDE;
  // --- screen configuration
  void init_workarea();
  void init() FL_OVERRIDE;
  int x() FL_OVERRIDE;
  int y() FL_OVERRIDE;
  int w() FL_OVERRIDE;
  int h() FL_OVERRIDE;
  void screen_xywh(int &X, int &Y, int &W, int &H, int n) FL_OVERRIDE;
  void screen_dpi(float &h, float &v, int n=0) FL_OVERRIDE;
  void screen_work_area(int &X, int &Y, int &W, int &H, int n) FL_OVERRIDE;
  // --- audible output
  void beep(int type) FL_OVERRIDE;
  // --- global events
  void flush() FL_OVERRIDE;
  void grab(Fl_Window* win) FL_OVERRIDE;
  // --- global colors
  int parse_color(const char* p, uchar& r, uchar& g, uchar& b) FL_OVERRIDE;
  void get_system_colors() FL_OVERRIDE;
  const char *get_system_scheme() FL_OVERRIDE;
  int dnd(int unused) FL_OVERRIDE;
  int compose(int &del) FL_OVERRIDE;
  void compose_reset() FL_OVERRIDE;
  int text_display_can_leak() const FL_OVERRIDE;
  Fl_RGB_Image *read_win_rectangle(int X, int Y, int w, int h, Fl_Window *win, bool may_capture_subwins, bool *did_capture_subwins) FL_OVERRIDE;
  int get_mouse(int &x, int &y) FL_OVERRIDE;

  void open_display_platform() FL_OVERRIDE;
  void close_display() FL_OVERRIDE;
  // --- compute dimensions of an Fl_Offscreen
  void offscreen_size(Fl_Offscreen o, int &width, int &height) FL_OVERRIDE;
  void default_icons(const Fl_RGB_Image *icons[], int count) FL_OVERRIDE;
  // this one is in Fl_x.cxx
  void copy(const char *stuff, int len, int clipboard, const char *type) FL_OVERRIDE;
  // this one is in Fl_x.cxx
  void paste(Fl_Widget &receiver, int clipboard, const char *type) FL_OVERRIDE;
  // this one is in Fl_x.cxx
  int clipboard_contains(const char *type) FL_OVERRIDE;
  // this one is in Fl_x.cxx
  void clipboard_notify_change() FL_OVERRIDE;
  // for support of input methods
  static char fl_is_over_the_spot;
  static XRectangle fl_spot;
  static int fl_spotf;
  static int fl_spots;
  static XIM xim_im;
  static XIC xim_ic;
  static Window xim_win;
  static void new_ic();
  static void xim_activate(Window xid);
  static void xim_deactivate(void);
  static void init_xim();
  void enable_im() FL_OVERRIDE;
  void disable_im() FL_OVERRIDE;
  void set_spot(int font, int size, int X, int Y, int W, int H, Fl_Window *win) FL_OVERRIDE;
  void reset_spot() FL_OVERRIDE;
  void set_status(int X, int Y, int W, int H) FL_OVERRIDE;
};


#endif // FL_X11_SCREEN_DRIVER_H
