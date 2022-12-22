//
// Definition of Apple Cocoa window driver
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
 \file Fl_Cocoa_Window_Driver.H
 \brief Definition of Apple Cocoa window driver.
 */

#ifndef FL_COCOA_WINDOW_DRIVER_H
#define FL_COCOA_WINDOW_DRIVER_H

#include "../../Fl_Window_Driver.H"
#include <FL/Fl_Plugin.H>
#include <ApplicationServices/ApplicationServices.h>

class Fl_Image;
class Fl_Window;
#ifdef __OBJC__
@class CALayer;
@class NSCursor;
@class NSImage;
@class FLWindow;
@class NSOpenGLContext;
@class NSOpenGLPixelFormat;
@class NSView;
#else
class CALayer;
class NSCursor;
class NSImage;
class FLWindow;
class NSOpenGLContext;
class NSOpenGLPixelFormat;
class NSView;
#endif // __OBJC__

/**
 \cond DriverDev
 \addtogroup DriverDeveloper
 \{
 */

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

/**
 \}
 \endcond
 */


class Fl_Cocoa_Window_Driver : public Fl_Window_Driver
{
private:
  struct shape_data_type {
    Fl_Image* shape_; ///<  shape image
    CGImageRef mask;
  } *shape_data_;
  void shape_bitmap_(Fl_Image* b);
  void shape_alpha_(Fl_Image* img, int offset) override;
  CGRect* subRect_;           // makes sure subwindow remains inside its parent window
  // stores 3 binary flags: whether window is mapped to retina display; whether resolution just changed;
  // whether window's view received the [FLView view_did_resize] message
  unsigned window_flags_;
public:
  Fl_Cocoa_Window_Driver(Fl_Window*);
  ~Fl_Cocoa_Window_Driver();
  static inline Fl_Cocoa_Window_Driver* driver(const Fl_Window *w) {return (Fl_Cocoa_Window_Driver*)Fl_Window_Driver::driver(w);}
  CGContextRef gc;                 // graphics context
  NSCursor *cursor;
  static void q_release_context(Fl_Cocoa_Window_Driver *x = 0); // free all resources associated with gc
  static void clip_to_rounded_corners(CGContextRef gc, int w, int h);
  void set_key_window();
  bool mapped_to_retina();      // is window mapped to retina display?
  void mapped_to_retina(bool);  // sets whether window is mapped to retina display
  bool changed_resolution();      // did window just moved to display with another resolution?
  void changed_resolution(bool);// sets whether window just moved to display with another resolution
  bool view_resized();   // did window's view receive [FLView view_did_resize] message?
  void view_resized(bool b); // sets whether window's view received [FLView view_did_resize] message
  bool through_resize();   // did Fl_Window::resize() run already
  void through_resize(bool b); // set whether Fl_Window::resize() run already
  CGRect* subRect() { return subRect_; } // getter
  void subRect(CGRect *r) { subRect_ = r; } // setter
  static void destroy(FLWindow*);
  CGImageRef CGImage_from_window_rect(int x, int y, int w, int h, bool capture_subwins = true);

  // --- window data
  int decorated_w() override;
  int decorated_h() override;
  const Fl_Image* shape() override;

  // --- window management
  Fl_X *makeWindow() override;
  void take_focus() override;
  void flush() override;
  void flush_overlay() override;
  void draw_begin() override;
  void draw_end() override;
  void make_current() override;
  void label(const char *name, const char *mininame) override;
  void destroy_double_buffer() override;
  void show() override;
  void resize(int X,int Y,int W,int H) override;
  void hide() override;
  void map() override;
  void unmap() override;
  void fullscreen_on() override;
  void fullscreen_off(int X, int Y, int W, int H) override;
  void use_border() override;
  void size_range() override;
  void iconize() override;
  void decoration_sizes(int *top, int *left,  int *right, int *bottom) override;
  // --- window cursor stuff
  int set_cursor(Fl_Cursor) override;
  int set_cursor(const Fl_RGB_Image*, int, int) override;

  void shape(const Fl_Image* img) override;
  // next 4 are in Fl_cocoa.mm because they use Objective-c
  void capture_titlebar_and_borders(Fl_RGB_Image*& top, Fl_RGB_Image*& left, Fl_RGB_Image*& bottom, Fl_RGB_Image*& right) override;
  void wait_for_expose() override;
  void draw_titlebar_to_context(CGContextRef gc, int w, int h);
  int scroll(int src_x, int src_y, int src_w, int src_h, int dest_x, int dest_y, void (*draw_area)(void*, int,int,int,int), void* data) override;

  //icons
  void icons(const Fl_RGB_Image *icons[], int count) override;
  NSImage *icon_image;

  fl_uintptr_t os_id() override;
};

class Fl_Cocoa_Plugin : public Fl_Plugin {
public:
  Fl_Cocoa_Plugin(const char *pluginName)  : Fl_Plugin(klass(), pluginName) { }
  virtual const char *klass() { return "fltk:cocoa"; }
  virtual const char *name() = 0;
  virtual void resize(Fl_Gl_Window *glw, int x, int y, int w, int h) = 0;
};

#endif // FL_COCOA_WINDOW_DRIVER_H
