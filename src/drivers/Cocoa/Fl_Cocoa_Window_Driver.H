//
// "$Id$"
//
// Definition of Apple Cocoa window driver
// for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2018 by Bill Spitzak and others.
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

#include "../../Fl_Window_Driver.H"
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
#else
class CALayer;
class NSCursor;
class NSImage;
class FLWindow;
class NSOpenGLContext;
class NSOpenGLPixelFormat;
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
struct Fl_Window_Driver::shape_data_type {
  Fl_Image* shape_; ///<  shape image
  typedef struct CGImage* CGImageRef;
  CGImageRef mask;
};

/**
 \}
 \endcond
 */


class FL_EXPORT Fl_Cocoa_Window_Driver : public Fl_Window_Driver
{
private:
  void shape_bitmap_(Fl_Image* b);
  void shape_alpha_(Fl_Image* img, int offset);
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
  CGRect* subRect() { return subRect_; } // getter
  void subRect(CGRect *r) { subRect_ = r; } // setter
  static void destroy(FLWindow*);
  CGImageRef CGImage_from_window_rect(int x, int y, int w, int h, bool capture_subwins = true);

  // --- window data
  virtual int decorated_w();
  virtual int decorated_h();
  virtual const Fl_Image* shape();

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
  virtual void use_border();
  virtual void size_range();
  virtual void iconize();
  virtual void decoration_sizes(int *top, int *left,  int *right, int *bottom);
  // --- window cursor stuff
  virtual int set_cursor(Fl_Cursor);
  virtual int set_cursor(const Fl_RGB_Image*, int, int);

  virtual void shape(const Fl_Image* img);
  // next 4 are in Fl_cocoa.mm because they use Objective-c
  virtual void capture_titlebar_and_borders(Fl_RGB_Image*& top, Fl_RGB_Image*& left, Fl_RGB_Image*& bottom, Fl_RGB_Image*& right);
  virtual void wait_for_expose();
  CALayer *get_titlebar_layer();
  static void draw_layer_to_context(CALayer *layer, CGContextRef gc, int w, int h);
  virtual int scroll(int src_x, int src_y, int src_w, int src_h, int dest_x, int dest_y, void (*draw_area)(void*, int,int,int,int), void* data);
  
  // these functions are OpenGL-related and use objective-c
  // they are put here to avoid libfltk_gl dependency in Fl_cocoa.mm
  static NSOpenGLContext* create_GLcontext_for_window(NSOpenGLPixelFormat *pixelformat, NSOpenGLContext *shared_ctx, Fl_Window *window);
  static NSOpenGLPixelFormat *mode_to_NSOpenGLPixelFormat(int mode, const int*); // uses Objective-c
  static void GLcontext_update(NSOpenGLContext*); // uses Objective-c
  static void GLcontext_release(NSOpenGLContext*); // uses Objective-c
  static void flush_context(NSOpenGLContext*); // uses Objective-c
  static void GLcontext_makecurrent(NSOpenGLContext*); // uses Objective-c
  static void GL_cleardrawable(void); // uses Objective-c
  static void gl_start(NSOpenGLContext*); // uses Objective-c
  
  //icons
  virtual void icons(const Fl_RGB_Image *icons[], int count);
  NSImage *icon_image;
};

#endif // FL_COCOA_WINDOW_DRIVER_H

//
// End of "$Id$".
//
