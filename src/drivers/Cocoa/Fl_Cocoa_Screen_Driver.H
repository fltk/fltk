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
  void init() FL_OVERRIDE;
  int x() FL_OVERRIDE;
  int y() FL_OVERRIDE;
  int w() FL_OVERRIDE;
  int h() FL_OVERRIDE;
  void screen_xywh(int &X, int &Y, int &W, int &H, int n) FL_OVERRIDE;
  void screen_dpi(float &h, float &v, int n=0) FL_OVERRIDE;
  // implemented in Fl_cocoa.mm because uses Objective-c
  void screen_work_area(int &X, int &Y, int &W, int &H, int n) FL_OVERRIDE;
  // --- audible output
  void beep(int type) FL_OVERRIDE;
  // --- global events
  void grab(Fl_Window* win) FL_OVERRIDE;
  // --- global colors
  void get_system_colors() FL_OVERRIDE;
  int has_marked_text() const FL_OVERRIDE;
  static void reset_marked_text();
  static void insertion_point_location(int x, int y, int height);
  static int insertion_point_location(int *px, int *py, int *pheight);
  int dnd(int use_selection) FL_OVERRIDE;
  int compose(int &del) FL_OVERRIDE;
  int input_widget_handle_key(int key, unsigned mods, unsigned shift, Fl_Input *input) FL_OVERRIDE;
  int get_mouse(int &x, int &y) FL_OVERRIDE;
  void enable_im() FL_OVERRIDE;
  void disable_im() FL_OVERRIDE;
  void open_display_platform() FL_OVERRIDE;
  // --- compute dimensions of an Fl_Offscreen
  void offscreen_size(Fl_Offscreen o, int &width, int &height) FL_OVERRIDE;

  APP_SCALING_CAPABILITY rescalable() FL_OVERRIDE { return SYSTEMWIDE_APP_SCALING; }
  float scale(int /*nscreen*/) FL_OVERRIDE {return scale_;}
  void scale(int /*nscreen*/, float f) FL_OVERRIDE { scale_ = f;}
  Fl_RGB_Image *read_win_rectangle(int X, int Y, int w, int h, Fl_Window *win, bool may_capture_subwins, bool *did_capture_subwins) FL_OVERRIDE;
  void default_icons(const Fl_RGB_Image *icons[], int count) FL_OVERRIDE;
  void copy(const char *stuff, int len, int clipboard, const char *type) FL_OVERRIDE;
  void paste(Fl_Widget &receiver, int clipboard, const char *type) FL_OVERRIDE;
  int clipboard_contains(const char *type) FL_OVERRIDE;
  void set_spot(int font, int size, int X, int Y, int W, int H, Fl_Window *win) FL_OVERRIDE;
  void reset_spot() FL_OVERRIDE;
  int need_menu_handle_part2() FL_OVERRIDE {return 1;}
  // these 2 are in Fl_get_key_mac.cxx
  int event_key(int) FL_OVERRIDE;
  int get_key(int) FL_OVERRIDE;
private:
  float scale_;
};


#endif // FL_COCOA_SCREEN_DRIVER_H
