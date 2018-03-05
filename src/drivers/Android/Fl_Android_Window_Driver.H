//
// "$Id$"
//
// Definition of Android window driver
// for the Fast Light Tool Kit (FLTK).
//
// Copyright 2018 by Bill Spitzak and others.
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
 \file Fl_Android_Window_Driver.H
 \brief Definition of Android window driver.
 */

#ifndef FL_ANDROID_WINDOW_DRIVER_H
#define FL_ANDROID_WINDOW_DRIVER_H

#include <FL/Fl_Window_Driver.H>
//#include <windows.h>

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


struct Fl_Window_Driver::shape_data_type {
  int lw_; ///<  width of shape image
  int lh_; ///<  height of shape image
  Fl_Image* shape_; ///<  shape image
  Fl_Bitmap *todelete_; ///<  auxiliary bitmap image
};

class FL_EXPORT Fl_Android_Window_Driver : public Fl_Window_Driver
{
  static Fl_Window *sCurrent;
#if 0

  friend class Fl_Window;
  struct icon_data {
    const void *legacy_icon;
    Fl_RGB_Image **icons;
    int count;
    HICON big_icon;
    HICON small_icon;
  };
  int screen_num_;
private:
  void shape_bitmap_(Fl_Image* b);
  void shape_alpha_(Fl_Image* img, int offset);
#endif

public:
  Fl_Android_Window_Driver(Fl_Window *w) : Fl_Window_Driver(w) {}
  ~Fl_Android_Window_Driver() {}
  static inline Fl_Android_Window_Driver* driver(Fl_Window *w) {
    return (Fl_Android_Window_Driver*)w->driver();
  }

#if 0
  HDC private_dc; // used for OpenGL
  RECT border_width_title_bar_height(int &bx, int &by, int &bt);
  virtual void screen_num(int n);
  virtual int screen_num();

  struct icon_data *icon_;
  HCURSOR cursor;
  int custom_cursor;
  struct type_for_resize_window_between_screens {
    int screen;
    bool busy;
  };
  static type_for_resize_window_between_screens data_for_resize_window_between_screens_;
  void set_minmax(LPMINMAXINFO minmax);
  int fake_X_wm(int &X, int &Y, int &bt,int &bx,int &by);
  void make_fullscreen(int X, int Y, int W, int H);
  // --- window data
  virtual int decorated_w();
  virtual int decorated_h();

  // --- window management
  virtual Fl_X *makeWindow();
  virtual void flush_double();
  virtual void flush_overlay();
  virtual void draw_begin();
#endif
  virtual void make_current();
  virtual void show();
#if 0
  virtual void label(const char *name,const char *iname);
  virtual void resize(int X,int Y,int W,int H);
  virtual void hide();
  virtual void map();
  virtual void unmap();
  virtual void fullscreen_on();
  virtual void fullscreen_off(int X, int Y, int W, int H);
  virtual void iconize();
  virtual void decoration_sizes(int *top, int *left,  int *right, int *bottom);
  // --- window cursor stuff
  virtual int set_cursor(Fl_Cursor);
  virtual int set_cursor(const Fl_RGB_Image*, int, int);

  virtual void shape(const Fl_Image* img);
  virtual void icons(const Fl_RGB_Image *icons[], int count);
  virtual const void *icon() const;
  virtual void icon(const void * ic);
  virtual void free_icons();
  void set_icons(); // driver-internal support function
  // this one is implemented in Fl_win32.cxx
  virtual void capture_titlebar_and_borders(Fl_Shared_Image*& top, Fl_Shared_Image*& left, Fl_Shared_Image*& bottom, Fl_Shared_Image*& right);
  virtual int scroll(int src_x, int src_y, int src_w, int src_h, int dest_x, int dest_y,
                     void (*draw_area)(void*, int,int,int,int), void* data);
  static void resize_after_screen_change(void *data);

#endif

// Leuwer: 0171 473 1850

};


#endif // FL_ANDROID_WINDOW_DRIVER_H

//
// End of "$Id$".
//
