//
// Definition of Windows window driver
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
 \file Fl_WinAPI_Window_Driver.H
 \brief Definition of Windows window driver.
 */

#ifndef FL_WINAPI_WINDOW_DRIVER_H
#define FL_WINAPI_WINDOW_DRIVER_H

#include "../../Fl_Window_Driver.H"
#include <windows.h>

/*
 Move everything here that manages the native window interface.

 There is one window driver for each Fl_Window. Window drivers manage window
 actions such as resizing, events, decoration, fullscreen modes, etc. . All
 drawing and rendering is managed by the Surface device and the associated
 graphics driver.

 - window specific event handling
 - window types and styles, depth, etc.
 - decorations

 ? where do we handle the interface between OpenGL/DirectX and Cocoa/Windows/Glx?
 */



class Fl_WinAPI_Window_Driver : public Fl_Window_Driver
{
  struct icon_data {
    const void *legacy_icon;
    Fl_RGB_Image **icons;
    int count;
    HICON big_icon;
    HICON small_icon;
  };
  struct shape_data_type {
    int lw_; ///<  width of shape image
    int lh_; ///<  height of shape image
    Fl_Image* shape_; ///<  shape image
    Fl_Bitmap *effective_bitmap_; ///<  auxiliary bitmap image
  } *shape_data_;
private:
  void shape_bitmap_(Fl_Image* b);
  void shape_alpha_(Fl_Image* img, int offset) FL_OVERRIDE;
public:
  Fl_WinAPI_Window_Driver(Fl_Window*);
  ~Fl_WinAPI_Window_Driver();
  static inline Fl_WinAPI_Window_Driver* driver(const Fl_Window *w) {return (Fl_WinAPI_Window_Driver*)Fl_Window_Driver::driver(w);}
  HDC private_dc; // used for OpenGL
  RECT border_width_title_bar_height(int &bx, int &by, int &bt);

  struct icon_data *icon_;
  HCURSOR cursor;
  int custom_cursor;
  struct type_for_resize_window_between_screens {
    int screen;
    bool busy;
  };
  static type_for_resize_window_between_screens data_for_resize_window_between_screens_;
  void set_minmax(LPMINMAXINFO minmax);
  int fake_X_wm(int &X, int &Y, int &bt, int &bx, int &by, DWORD style = 0, DWORD styleEx = 0);
  void make_fullscreen(int X, int Y, int W, int H);
  // --- window data
  int decorated_w() FL_OVERRIDE;
  int decorated_h() FL_OVERRIDE;
  const Fl_Image* shape() FL_OVERRIDE;

  // --- window management
  void makeWindow() FL_OVERRIDE;
  void size_range() FL_OVERRIDE {
    // currently nothing to do
  }
  void flush_double() FL_OVERRIDE;
  void flush_overlay() FL_OVERRIDE;
  void draw_begin() FL_OVERRIDE;
  void make_current() FL_OVERRIDE;
  void show() FL_OVERRIDE;
  void label(const char *name,const char *iname) FL_OVERRIDE;
  void resize(int X,int Y,int W,int H) FL_OVERRIDE;
  void hide() FL_OVERRIDE;
  void map() FL_OVERRIDE;
  void unmap() FL_OVERRIDE;
  void fullscreen_on() FL_OVERRIDE;
  void fullscreen_off(int X, int Y, int W, int H) FL_OVERRIDE;
  void maximize() FL_OVERRIDE;
  void un_maximize() FL_OVERRIDE;
  bool maximize_needs_hide() FL_OVERRIDE { return true; }
  void iconize() FL_OVERRIDE;
  void decoration_sizes(int *top, int *left,  int *right, int *bottom) FL_OVERRIDE;
  // --- window cursor stuff
  int set_cursor(Fl_Cursor) FL_OVERRIDE;
  int set_cursor(const Fl_RGB_Image*, int, int) FL_OVERRIDE;

  void shape(const Fl_Image* img) FL_OVERRIDE;
  void icons(const Fl_RGB_Image *icons[], int count) FL_OVERRIDE;
  const void *icon() const FL_OVERRIDE;
  void icon(const void * ic) FL_OVERRIDE;
  void free_icons() FL_OVERRIDE;
  void set_icons(); // driver-internal support function
  // this one is implemented in Fl_win32.cxx
  void capture_titlebar_and_borders(Fl_RGB_Image*& top, Fl_RGB_Image*& left, Fl_RGB_Image*& bottom, Fl_RGB_Image*& right) FL_OVERRIDE;
  int scroll(int src_x, int src_y, int src_w, int src_h, int dest_x, int dest_y,
                     void (*draw_area)(void*, int,int,int,int), void* data) FL_OVERRIDE;
  static void resize_after_screen_change(void *data);
};


#endif // FL_WINAPI_WINDOW_DRIVER_H
