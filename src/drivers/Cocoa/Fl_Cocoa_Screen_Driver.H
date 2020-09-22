//
// Definition of Apple Cocoa Screen interface
// for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2018 by Bill Spitzak and others.
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
 \file Fl_Cocoa_Screen_Driver.H
 \brief Definition of Apple Cocoa Screen interface.
 */

#ifndef FL_COCOA_SCREEN_DRIVER_H
#define FL_COCOA_SCREEN_DRIVER_H

#include "../../Fl_Screen_Driver.H"

/*
 Move everything here that manages the native screen interface.

 There is exactly one screen driver in the system.

 - screen configuration and sizes
 - multiple screens
 - native dialog boxes
*/


class Fl_Window;
class Fl_Input;
class Fl_RGB_Image;
#ifdef __OBJC__
@class NSImage;
#else
class NSImage;
#endif

class FL_EXPORT Fl_Cocoa_Screen_Driver : public Fl_Screen_Driver
{
protected:
  struct XRectangle {int x, y, width, height;};
  XRectangle screens[MAX_SCREENS];
  float dpi_h[MAX_SCREENS];
  float dpi_v[MAX_SCREENS];
  static int insertion_point_x;
  static int insertion_point_y;
  static int insertion_point_height;
  static bool insertion_point_location_is_valid;
public:
  NSImage *default_icon;
  Fl_Cocoa_Screen_Driver();
  static int next_marked_length; // next length of marked text after current marked text will have been replaced
  static void breakMacEventLoop();
  // --- display management
  // --- screen configuration
  virtual void init();
  virtual int x();
  virtual int y();
  virtual int w();
  virtual int h();
  virtual void screen_xywh(int &X, int &Y, int &W, int &H, int n);
  virtual void screen_dpi(float &h, float &v, int n=0);
  // implemented in Fl_cocoa.mm because uses Objective-c
  virtual void screen_work_area(int &X, int &Y, int &W, int &H, int n);
  // --- audible output
  virtual void beep(int type);
  // --- global events
  virtual double wait(double time_to_wait);
  virtual int ready();
  virtual void grab(Fl_Window* win);
  // --- global colors
  virtual void get_system_colors();
  // --- global timers
  virtual void add_timeout(double time, Fl_Timeout_Handler cb, void *argp);
  virtual void repeat_timeout(double time, Fl_Timeout_Handler cb, void *argp);
  virtual int has_timeout(Fl_Timeout_Handler cb, void *argp);
  virtual void remove_timeout(Fl_Timeout_Handler cb, void *argp);
  virtual int has_marked_text();
  virtual void reset_marked_text();
  virtual void insertion_point_location(int x, int y, int height);
  int insertion_point_location(int *px, int *py, int *pheight);
  virtual int dnd(int use_selection);
  virtual int compose(int &del);
  virtual int input_widget_handle_key(int key, unsigned mods, unsigned shift, Fl_Input *input);
  virtual int get_mouse(int &x, int &y);
  virtual void enable_im();
  virtual void disable_im();
  virtual void open_display_platform();
  // --- compute dimensions of an Fl_Offscreen
  virtual void offscreen_size(Fl_Offscreen o, int &width, int &height);

  virtual APP_SCALING_CAPABILITY rescalable() { return SYSTEMWIDE_APP_SCALING; }
  virtual float scale(int n) {return scale_;}
  virtual void scale(int n, float f) { scale_ = f;}
  virtual Fl_RGB_Image *read_win_rectangle(int X, int Y, int w, int h, Fl_Window *win, bool may_capture_subwins, bool *did_capture_subwins);
  virtual void default_icons(const Fl_RGB_Image *icons[], int count);
private:
  float scale_;
};


#endif // FL_COCOA_SCREEN_DRIVER_H
