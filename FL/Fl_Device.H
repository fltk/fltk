//
// "$Id$"
//
// Definition of classes Fl_Device, Fl_Display, Fl_Quartz_Display, Fl_GDI_Display,
// Fl_Xlib_Display and Fl_Device_Plugin for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

#ifndef Fl_Device_H
#define Fl_Device_H

#include <FL/x.H>
#include <FL/Fl_Plugin.H>
#ifdef WIN32
#include <Commdlg.h>
#elif defined(__APPLE__)
#else
#include <stdio.h>
#endif

class Fl_Widget;
class Fl_Image;
class Fl_RGB_Image;
class Fl_Pixmap;
class Fl_Bitmap;
class Fl_Display;
class Fl_Virtual_Printer;
extern Fl_Display *fl_display_device;
typedef void (*Fl_Draw_Image_Cb)(void* ,int,int,int,uchar*);

/**
 @brief A pure virtual class subclassed to send graphics output to display, local files, or printers.
 */
class Fl_Device {
protected:
  int type_;
  uchar bg_r_, bg_g_, bg_b_; // color for background and/or mixing if particular device does not support masking and/or alpha
  friend void fl_rect(int x, int y, int w, int h);
  friend void fl_rectf(int x, int y, int w, int h);
  friend void fl_line_style(int style, int width, char* dashes);
  friend void fl_xyline(int x, int y, int x1);
  friend void fl_xyline(int x, int y, int x1, int y2);
  friend void fl_xyline(int x, int y, int x1, int y2, int x3);
  friend void fl_yxline(int x, int y, int y1);
  friend void fl_yxline(int x, int y, int y1, int x2);
  friend void fl_yxline(int x, int y, int y1, int x2, int y3);
  friend void fl_line(int x, int y, int x1, int y1);
  friend void fl_line(int x, int y, int x1, int y1, int x2, int y2);
  friend void fl_draw(const char *str, int n, int x, int y);
  friend void fl_draw(int angle, const char *str, int n, int x, int y);
  friend void fl_font(Fl_Font face, Fl_Fontsize size);
  friend void fl_color(Fl_Color c);
  friend void fl_color(uchar r, uchar g, uchar b);
  friend void fl_point(int x, int y);
  friend void fl_loop(int x0, int y0, int x1, int y1, int x2, int y2);
  friend void fl_loop(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3);
  friend void fl_polygon(int x0, int y0, int x1, int y1, int x2, int y2);
  friend void fl_polygon(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3);
  friend void fl_concat();
  friend void fl_reconcat();
  friend void fl_begin_points();
  friend void fl_begin_line();
  friend void fl_begin_loop();
  friend void fl_begin_polygon();
  friend void fl_vertex(double x, double y);
  friend void fl_curve(double x, double y, double x1, double y1, double x2, double y2, double x3, double y3);
  friend void fl_circle(double x, double y, double r);
  friend void fl_arc(double x, double y, double r, double start, double a);
  friend void fl_arc(int x, int y, int w, int h, double a1, double a2);
  friend void fl_pie(int x, int y, int w, int h, double a1, double a2);
  friend void fl_end_points();
  friend void fl_end_line();
  friend void fl_end_loop();
  friend void fl_end_polygon();
  friend void fl_transformed_vertex(double x, double y);
  friend void fl_push_clip(int x, int y, int w, int h);
  friend int fl_clip_box(int x, int y, int w, int h, int &X, int &Y, int &W, int &H);
  friend int fl_not_clipped(int x, int y, int w, int h);
  friend void fl_push_no_clip();
  friend void fl_pop_clip();
  friend void fl_begin_complex_polygon();
  friend void fl_gap();
  friend void fl_end_complex_polygon();
  friend void fl_draw_image(const uchar*, int,int,int,int, int delta, int ldelta);
  friend void fl_draw_image_mono(const uchar*, int,int,int,int, int delta, int ld);
  friend void fl_draw_image(Fl_Draw_Image_Cb, void*, int,int,int,int, int delta);
  friend void fl_draw_image_mono(Fl_Draw_Image_Cb, void*, int,int,int,int, int delta);
  
  virtual void rect(int x, int y, int w, int h);
  virtual void rectf(int x, int y, int w, int h);
  virtual void line_style(int style, int width=0, char* dashes=0);
  virtual void xyline(int x, int y, int x1);
  virtual void xyline(int x, int y, int x1, int y2);
  virtual void xyline(int x, int y, int x1, int y2, int x3);
  virtual void yxline(int x, int y, int y1);
  virtual void yxline(int x, int y, int y1, int x2);
  virtual void yxline(int x, int y, int y1, int x2, int y3);
  virtual void line(int x, int y, int x1, int y1);
  virtual void line(int x, int y, int x1, int y1, int x2, int y2);
  virtual void draw(const char *str, int n, int x, int y);
  virtual void draw(int angle, const char *str, int n, int x, int y);
  virtual void font(Fl_Font face, Fl_Fontsize size);
  virtual void color(Fl_Color c);
  virtual void color(uchar r, uchar g, uchar b);
  virtual void point(int x, int y);
  virtual void loop(int x0, int y0, int x1, int y1, int x2, int y2);
  virtual void loop(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3);
  virtual void polygon(int x0, int y0, int x1, int y1, int x2, int y2);
  virtual void polygon(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3);
  virtual void begin_points();
  virtual void begin_line();
  virtual void begin_loop();
  virtual void begin_polygon();
  virtual void vertex(double x, double y);
  virtual void curve(double x, double y, double x1, double y1, double x2, double y2, double x3, double y3);
  virtual void circle(double x, double y, double r);
  virtual void arc(double x, double y, double r, double start, double a);
  virtual void arc(int x, int y, int w, int h, double a1, double a2);
  virtual void pie(int x, int y, int w, int h, double a1, double a2);
  virtual void end_points();
  virtual void end_line();
  virtual void end_loop();
  virtual void end_polygon();
  virtual void begin_complex_polygon();
  virtual void gap();
  virtual void end_complex_polygon();
  virtual void transformed_vertex(double x, double y);
  virtual void push_clip(int x, int y, int w, int h);
  virtual int clip_box(int x, int y, int w, int h, int &X, int &Y, int &W, int &H);
  virtual int not_clipped(int x, int y, int w, int h);
  virtual void push_no_clip();
  virtual void pop_clip();
  // Images
  virtual   void draw_image(const uchar*, int,int,int,int, int delta=3, int ldelta=0);
  virtual   void draw_image_mono(const uchar*, int,int,int,int, int delta=1, int ld=0);
  virtual   void draw_image(Fl_Draw_Image_Cb, void*, int,int,int,int, int delta=3);
  virtual   void draw_image_mono(Fl_Draw_Image_Cb, void*, int,int,int,int, int delta=1);
  // Image classes
  virtual   void draw(Fl_Pixmap * pxm,int XP, int YP, int WP, int HP, int cx, int cy);
  virtual   void draw(Fl_RGB_Image * rgb,int XP, int YP, int WP, int HP, int cx, int cy);
  virtual   void draw(Fl_Bitmap * bmp,int XP, int YP, int WP, int HP, int cx, int cy);
  
public:
  enum device_types { 
    xlib_display = 0, quartz_display, gdi_display, 
    gdi_printer = 256, quartz_printer, postscript_device 
  };
  /** 
   @brief An RTTI emulation of device classes. It returns values < 256 if it is a screen device 
   */
  int type() {return type_;};
  /**
   @brief Sets this device (display, local file, printer) as the target of future graphics calls.
   *
   @return  The current target device of graphics calls.
   */
  virtual Fl_Device *set_current();
  
  virtual ~Fl_Device() {};

  /**
   @brief    Returns the current target device of graphics calls.
   */
  static Fl_Device *current();
  
  /**
   @brief    Returns the platform's display device.
   */
  static Fl_Display *display_device() { return fl_display_device; };
  
};
extern Fl_Device *fl_device;

/**
 @brief A virtual class subclassed for OS-specific display graphics.
 */
class Fl_Display : public Fl_Device {
  friend class Fl_PSfile_Device; //RK: temporary hack for font sizes
};

#if defined(__APPLE__) || defined(FL_DOXYGEN)
/**
 @brief The Mac OS X-specific display graphics class.
 */
class Fl_Quartz_Display : public Fl_Display {
public:
  Fl_Quartz_Display() { type_ = quartz_display; };
};
#endif
#if defined(WIN32) || defined(FL_DOXYGEN)
/**
 @brief The MSWindows-specific display graphics class.
 */
class Fl_GDI_Display : public Fl_Display {
public:
  Fl_GDI_Display() { type_ = gdi_display; };
};
#endif
#if !( defined(__APPLE__) || defined(WIN32)) || defined(FL_DOXYGEN)
/**
 @brief The X11-specific display graphics class.
 */
class Fl_Xlib_Display : public Fl_Display {
public:
  Fl_Xlib_Display() { type_ = xlib_display; };
};
#endif

/**
 This plugin socket allows the integration of new device drivers for special
 window or screen types. It is currently used to provide an automated printing
 service for OpenGL windows, if linked with fltk_gl.
 */
class Fl_Device_Plugin : public Fl_Plugin {
public:
  Fl_Device_Plugin(const char *name)
  : Fl_Plugin(klass(), name) { }
  virtual const char *klass() { return "fltk:device"; }
  virtual const char *name() = 0;
  virtual int print(Fl_Virtual_Printer*, Fl_Widget*, int x, int y) { return 0; }
};

#endif // Fl_Device_H

//
// End of "$Id$".
//
