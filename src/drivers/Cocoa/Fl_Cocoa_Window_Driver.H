//
// "$Id$"
//
// Definition of Apple Cocoa window driver
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
 \file Fl_Cocoa_Window_Driver.H
 \brief Definition of Apple Cocoa window driver.
 */

#ifndef FL_COCOA_WINDOW_DRIVER_H
#define FL_COCOA_WINDOW_DRIVER_H

#include <FL/Fl_Window_Driver.H>
#include <ApplicationServices/ApplicationServices.h>

class Fl_Image;
class Fl_Window;
#ifdef __OBJC__
@class CALayer;
@class NSCursor;
@class FLWindow;
#else
class CALayer;
class NSCursor;
class FLWindow;
#endif // __OBJC__

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
  Fl_Image* shape_; ///<  shape image
  typedef struct CGImage* CGImageRef;
  CGImageRef mask;
};


class FL_EXPORT Fl_Cocoa_Window_Driver : public Fl_Window_Driver
{
private:
  void shape_bitmap_(Fl_Image* b);
  void shape_alpha_(Fl_Image* img, int offset);
  CGRect* subRect_;           // makes sure subwindow remains inside its parent window
  // stores 3 binary flags: whether window is mapped to retina display; whether resolution just changed;
  // whether window is OpenGL and is currently being resized.
  unsigned window_flags_;
public:
  Fl_Cocoa_Window_Driver(Fl_Window*);
  ~Fl_Cocoa_Window_Driver();
  static inline Fl_Cocoa_Window_Driver* driver(Fl_Window *w) {return (Fl_Cocoa_Window_Driver*)w->driver();}
  CGContextRef gc;                 // graphics context
  NSCursor *cursor;
  static void q_release_context(Fl_Cocoa_Window_Driver *x = 0); // free all resources associated with gc
  static void clip_to_rounded_corners(CGContextRef gc, int w, int h);
  void set_key_window();
  bool mapped_to_retina();      // is window mapped to retina display?
  void mapped_to_retina(bool);  // sets whether window is mapped to retina display
  bool changed_resolution();      // did window just moved to display with another resolution?
  void changed_resolution(bool);// sets whether window just moved to display with another resolution
  bool in_windowDidResize();      // is window performing windowDidResize?
  void in_windowDidResize(bool);  // sets whether window is performing windowDidResize
  CGRect* subRect() { return subRect_; } // getter
  void subRect(CGRect *r) { subRect_ = r; } // setter
  static void destroy(FLWindow*);
  unsigned char *bitmap_from_window_rect(int x, int y, int w, int h, int *bytesPerPixel);
  CGImageRef CGImage_from_window_rect(int x, int y, int w, int h);

  // --- window data
  virtual int decorated_w();
  virtual int decorated_h();

  // --- window management
  virtual Fl_X *makeWindow();
  virtual void take_focus();
  virtual void flush();
  virtual void flush_overlay();
  virtual void draw_begin();
  virtual void draw_end();
  virtual void make_current();
  virtual void label(const char *name, const char *mininame);
  virtual void destroy_double_buffer();
  virtual void show();
  virtual void resize(int X,int Y,int W,int H);
  virtual void hide();
  virtual void map();
  virtual void unmap();
  virtual void fullscreen_on();
  virtual void fullscreen_off(int X, int Y, int W, int H);
  virtual void size_range();
  virtual void iconize();
  virtual void decoration_sizes(int *top, int *left,  int *right, int *bottom);
  // --- window cursor stuff
  virtual int set_cursor(Fl_Cursor);
  virtual int set_cursor(const Fl_RGB_Image*, int, int);

  virtual void shape(const Fl_Image* img);
  // that one is implemented in Fl_Cocoa.mm because it uses Objective-c
  virtual void capture_titlebar_and_borders(Fl_Shared_Image*& top, Fl_Shared_Image*& left, Fl_Shared_Image*& bottom, Fl_Shared_Image*& right);
  //this one is in Fl_cocoa.mm because it uses Objective-c
  virtual void wait_for_expose();
  static void draw_layer_to_context(CALayer *layer, CGContextRef gc, int w, int h);
  virtual int scroll(int src_x, int src_y, int src_w, int src_h, int dest_x, int dest_y, void (*draw_area)(void*, int,int,int,int), void* data);
};



#endif // FL_COCOA_WINDOW_DRIVER_H

//
// End of "$Id$".
//
