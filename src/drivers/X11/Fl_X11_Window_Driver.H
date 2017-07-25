//
// "$Id$"
//
// Definition of X11 window driver
// for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2017 by Bill Spitzak and others.
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
 \file Fl_X11_Window_Driver.H
 \brief Definition of X11 window driver.
 */

#ifndef FL_X11_WINDOW_DRIVER_H
#define FL_X11_WINDOW_DRIVER_H

#include <FL/Fl_Window_Driver.H>
#include <config.h> // for USE_XDBE
#include <FL/x.H> // for Cursor
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
 
 ? where do we handle the interface between OpenGL/DirectX and Cocoa/WIN32/Glx?
 */



struct Fl_Window_Driver::shape_data_type {
  int lw_; ///<  width of shape image
  int lh_; ///<  height of shape image
  Fl_Image* shape_; ///<  shape image
  Fl_Bitmap *todelete_; ///<  auxiliary bitmap image
};

class FL_EXPORT Fl_X11_Window_Driver : public Fl_Window_Driver
{
  friend class Fl_X;
  friend class Fl_X11_Screen_Driver;
  friend int fl_handle(const XEvent&);

private:
  struct icon_data {
    const void *legacy_icon;
    Fl_RGB_Image **icons;
    int count;
  } *icon_;
  Cursor current_cursor_;
#if  USE_XFT
  // --- support for screen-specific scaling factors
  struct type_for_resize_window_between_screens {
    int screen;
    bool busy;
  };
  static type_for_resize_window_between_screens data_for_resize_window_between_screens_;
  int screen_num_;
  void screen_num(int n) { screen_num_ = n; }
#endif // USE_XFT
  void decorated_win_size(int &w, int &h);
  void combine_mask();
  void shape_bitmap_(Fl_Image* b);
  void shape_alpha_(Fl_Image* img, int offset);
  void flush_double(int erase_overlay);
  void sendxjunk();
  void activate_window();
#ifdef USE_XDBE
  char backbuffer_bad;
  void flush_double_dbe(int erase_overlay);
#endif
  
public:
  Fl_X11_Window_Driver(Fl_Window*);
  virtual ~Fl_X11_Window_Driver();
  static inline Fl_X11_Window_Driver* driver(Fl_Window *w) {return (Fl_X11_Window_Driver*)w->driver();}
#if  USE_XFT
  virtual int screen_num();
  static void resize_after_screen_change(void *data);
#endif // USE_XFT
  
  // --- window data
  virtual int decorated_w();
  virtual int decorated_h();

  // --- window management
  virtual Fl_X *makeWindow();
  virtual void take_focus();
  virtual void flush_double();
  virtual void flush_overlay();
  virtual void flush_menu();
  virtual void erase_menu();
  virtual void draw_begin();
  virtual void make_current();
  virtual void show();
  virtual void show_menu();
  virtual void resize(int X,int Y,int W,int H);
  virtual void label(const char *name, const char *mininame);
  virtual void destroy_double_buffer();
  virtual void hide();
  virtual void map();
  virtual void unmap();
  virtual void fullscreen_on();
  virtual void fullscreen_off(int X, int Y, int W, int H);
  virtual void use_border();
  virtual void size_range();
  virtual void iconize();
  virtual void decoration_sizes(int *top, int *left,  int *right, int *bottom);
  virtual void show_with_args_begin();
  virtual void show_with_args_end(int argc, char **argv);
  // --- window cursor stuff
  virtual int set_cursor(Fl_Cursor);
  virtual int set_cursor(const Fl_RGB_Image*, int, int);
  virtual fl_uintptr_t current_cursor() {return current_cursor_;}

  virtual void shape(const Fl_Image* img);
  virtual void icons(const Fl_RGB_Image *icons[], int count);
  virtual const void *icon() const;
  virtual void icon(const void * ic);
  virtual void free_icons();
  void set_icons(); // driver-internal support function
  virtual void capture_titlebar_and_borders(Fl_Shared_Image*& top, Fl_Shared_Image*& left, Fl_Shared_Image*& bottom, Fl_Shared_Image*& right);
  virtual int can_do_overlay();
  virtual void redraw_overlay();
  virtual int scroll(int src_x, int src_y, int src_w, int src_h, int dest_x, int dest_y, void (*draw_area)(void*, int,int,int,int), void* data);
};


#endif // FL_X11_WINDOW_DRIVER_H

//
// End of "$Id$".
//
