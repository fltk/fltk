//
// Definition of X11 window driver
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
 \file Fl_X11_Window_Driver.H
 \brief Definition of X11 window driver.
 */

#ifndef FL_X11_WINDOW_DRIVER_H
#define FL_X11_WINDOW_DRIVER_H

#include "../../Fl_Window_Driver.H"
#include <config.h>
#include <FL/platform.H> // for Cursor

#if FLTK_USE_CAIRO
typedef struct _cairo cairo_t;
#endif // FLTK_USE_CAIRO

class Fl_Bitmap;

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


class Fl_X11_Window_Driver : public Fl_Window_Driver
{
  friend class Fl_X;
  friend int fl_handle(const XEvent&);

private:
  struct icon_data {
    const void *legacy_icon;
    Fl_RGB_Image **icons;
    int count;
  } *icon_;
  struct shape_data_type {
    int lw_; ///<  width of shape image
    int lh_; ///<  height of shape image
    Fl_Image* shape_; ///<  shape image
    Fl_Bitmap *effective_bitmap_; ///<  auxiliary bitmap image
  } *shape_data_;
#if  USE_XFT
  // --- support for screen-specific scaling factors
  struct type_for_resize_window_between_screens {
    int screen;
    bool busy;
  };
  static type_for_resize_window_between_screens data_for_resize_window_between_screens_;
#endif // USE_XFT
#if FLTK_USE_CAIRO
  cairo_t *cairo_;
#endif // FLTK_USE_CAIRO
  void decorated_win_size(int &w, int &h);
  void combine_mask();
  void shape_bitmap_(Fl_Image* b);
  void shape_alpha_(Fl_Image* img, int offset) override;
  void flush_double(int erase_overlay);
  void sendxjunk();
  void activate_window();

public:
  Fl_X11_Window_Driver(Fl_Window*);
  ~Fl_X11_Window_Driver() override;
  static inline Fl_X11_Window_Driver* driver(const Fl_Window *w) {return (Fl_X11_Window_Driver*)Fl_Window_Driver::driver(w);}
#if  USE_XFT
  static void resize_after_screen_change(void *data);
#endif // USE_XFT

  // --- window data
  int decorated_w() override;
  int decorated_h() override;
  const Fl_Image* shape() override;

  // --- window management
  Fl_X *makeWindow() override;
  void take_focus() override;
  void flush_double() override;
  void flush_overlay() override;
  void draw_begin() override;
  void make_current() override;
  void show() override;
  void resize(int X,int Y,int W,int H) override;
  void label(const char *name, const char *mininame) override;
  void destroy_double_buffer() override;
  void hide() override;
  void map() override;
  void unmap() override;
  void fullscreen_on() override;
  void fullscreen_off(int X, int Y, int W, int H) override;
  void use_border() override;
  void size_range() override;
  void iconize() override;
  void decoration_sizes(int *top, int *left,  int *right, int *bottom) override;
  void show_with_args_begin() override;
  void show_with_args_end(int argc, char **argv) override;
  // --- window cursor stuff
  int set_cursor(Fl_Cursor) override;
  int set_cursor(const Fl_RGB_Image*, int, int) override;

  void shape(const Fl_Image* img) override;
  void icons(const Fl_RGB_Image *icons[], int count) override;
  const void *icon() const override;
  void icon(const void * ic) override;
  void free_icons() override;
  void set_icons(); // driver-internal support function
  void capture_titlebar_and_borders(Fl_RGB_Image*& top, Fl_RGB_Image*& left, Fl_RGB_Image*& bottom, Fl_RGB_Image*& right) override;
  int scroll(int src_x, int src_y, int src_w, int src_h, int dest_x, int dest_y, void (*draw_area)(void*, int,int,int,int), void* data) override;
  fl_uintptr_t os_id() override;
};


#endif // FL_X11_WINDOW_DRIVER_H
