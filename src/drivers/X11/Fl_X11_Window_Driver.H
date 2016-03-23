//
// "$Id: quartz.H 11017 2016-01-20 21:40:12Z matt $"
//
// Definition of X11 window driver
// for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2016 by Bill Spitzak and others.
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

private:
  struct icon_data {
    const void *legacy_icon;
    Fl_RGB_Image **icons;
    int count;
  } *icon_;
  void decorated_win_size(int &w, int &h);
  void combine_mask();
  void shape_bitmap_(Fl_Image* b);
  void shape_alpha_(Fl_Image* img, int offset);
  void flush_double(int erase_overlay);
#ifdef USE_XDBE
  void flush_double_dbe(int erase_overlay);
#endif
  
public:
  Fl_X11_Window_Driver(Fl_Window*);
  virtual ~Fl_X11_Window_Driver();

  // --- window data
  virtual int decorated_w();
  virtual int decorated_h();

  // --- window management
  virtual void take_focus();
  virtual void flush_double();
  virtual void flush_overlay();
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

  virtual void shape(const Fl_Image* img);
  virtual void icons(const Fl_RGB_Image *icons[], int count);
  virtual const void *icon() const;
  virtual void icon(const void * ic);
  virtual void free_icons();
  virtual void capture_titlebar_and_borders(Fl_Shared_Image*& top, Fl_Shared_Image*& left, Fl_Shared_Image*& bottom, Fl_Shared_Image*& right);
  virtual void wait_for_expose();
};


#endif // FL_X11_WINDOW_DRIVER_H

//
// End of "$Id: quartz.H 11017 2016-01-20 21:40:12Z matt $".
//
