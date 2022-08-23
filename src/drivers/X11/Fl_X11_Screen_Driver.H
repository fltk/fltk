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
#include "../../Fl_Screen_Driver.H"
#include <X11/Xlib.h>


class Fl_Window;


class Fl_X11_Screen_Driver : public Fl_Screen_Driver
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
  virtual APP_SCALING_CAPABILITY rescalable() { return PER_SCREEN_APP_SCALING; }
  virtual float scale(int n) {return screens[n].scale;}
  virtual void scale(int n, float f) { screens[n].scale = f;}
  virtual void desktop_scale_factor();
  int screen_num_unscaled(int x, int y);
#endif

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
  virtual void grab(Fl_Window* win);
  // --- global colors
  virtual int parse_color(const char* p, uchar& r, uchar& g, uchar& b);
  virtual void get_system_colors();
  virtual const char *get_system_scheme();
  virtual int dnd(int unused);
  virtual int compose(int &del);
  virtual void compose_reset();
  virtual int text_display_can_leak() const;
  virtual Fl_RGB_Image *read_win_rectangle(int X, int Y, int w, int h, Fl_Window *win, bool may_capture_subwins, bool *did_capture_subwins);
  virtual int get_mouse(int &x, int &y);

  virtual void open_display_platform();
  virtual void close_display();
  // --- compute dimensions of an Fl_Offscreen
  virtual void offscreen_size(Fl_Offscreen o, int &width, int &height);
  virtual void default_icons(const Fl_RGB_Image *icons[], int count);
  // this one is in Fl_x.cxx
  virtual void copy(const char *stuff, int len, int clipboard, const char *type);
  // this one is in Fl_x.cxx
  virtual void paste(Fl_Widget &receiver, int clipboard, const char *type);
  // this one is in Fl_x.cxx
  virtual int clipboard_contains(const char *type);
  // this one is in Fl_x.cxx
  virtual void clipboard_notify_change();
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
  virtual void enable_im();
  virtual void disable_im();
  virtual void set_spot(int font, int size, int X, int Y, int W, int H, Fl_Window *win);
  virtual void reset_spot();
  virtual void set_status(int X, int Y, int W, int H);
};


#endif // FL_X11_SCREEN_DRIVER_H
