//
// Definition of Apple Cocoa Screen interface
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

class Fl_Cocoa_Screen_Driver : public Fl_Screen_Driver
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
  virtual void grab(Fl_Window* win);
  // --- global colors
  virtual void get_system_colors();
  virtual int has_marked_text() const;
  static void reset_marked_text();
  static void insertion_point_location(int x, int y, int height);
  static int insertion_point_location(int *px, int *py, int *pheight);
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
  virtual float scale(int /*nscreen*/) {return scale_;}
  virtual void scale(int /*nscreen*/, float f) { scale_ = f;}
  virtual Fl_RGB_Image *read_win_rectangle(int X, int Y, int w, int h, Fl_Window *win, bool may_capture_subwins, bool *did_capture_subwins);
  virtual void default_icons(const Fl_RGB_Image *icons[], int count);
  virtual void copy(const char *stuff, int len, int clipboard, const char *type);
  virtual void paste(Fl_Widget &receiver, int clipboard, const char *type);
  virtual int clipboard_contains(const char *type);
  virtual void set_spot(int font, int size, int X, int Y, int W, int H, Fl_Window *win);
  virtual void reset_spot();
private:
  float scale_;
};


#endif // FL_COCOA_SCREEN_DRIVER_H
