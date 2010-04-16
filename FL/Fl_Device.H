//
// "$Id$"
//
// Definition of classes Fl_Device, Fl_Display_Device, Fl_Graphics_Device
// for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010 by Bill Spitzak and others.
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
/** \file Fl_Device.H 
 \brief declaration of classes Fl_Device, Fl_Display_Device.
*/

#ifndef Fl_Device_H
#define Fl_Device_H

#include <FL/x.H>
#include <FL/Fl_Plugin.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Bitmap.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_RGB_Image.H>
#ifdef WIN32
#include <commdlg.h>
#elif defined(__APPLE__)
#else
#include <stdio.h>
#endif

class Fl_Device;
class Fl_Display_Device;
/** \brief Points to the device that currently receives all graphics requests */
extern Fl_Device *fl_device;
/** \brief Points to the platform's display device */
extern Fl_Display_Device *fl_display_device;

/**
 signature of image generation callback function.
 \param[in]  data  user data passed to function
 \param[in]  x,y,w position and width of scan line in image
 \param[out] buf   buffer for generated image data. You must copy \p w
 pixels from scanline \p y, starting at pixel \p x
 to this buffer.
 */
typedef void (*Fl_Draw_Image_Cb)(void* data,int x,int y,int w,uchar* buf);

/**
 \brief A pure virtual class subclassed to send the output of drawing functions to display, printers, or local files.
 *
 The protected virtual methods of this class are those that a device should implement to
 support all of FLTK drawing functions.
 <br> The preferred FLTK API for drawing operations is the function collection of the 
 \ref fl_drawings and \ref fl_attributes modules. 
 <br> Alternatively, methods of the Fl_Device class can be called
 using the global variable Fl_Device * \ref fl_device that points at all time to the single device 
 (an instance of an Fl_Device subclass) that's currently receiving graphics requests:
 \code fl_device->rect(x, y, w, h); \endcode
 <br>Each protected method of the Fl_Device class has the same effect as the
 function of the \ref fl_drawings and \ref fl_attributes modules which bears the same name
 prefixed with fl_ and has the same parameter list.
  */
class Fl_Device {
protected:
  /** \brief The device type */
  const char *type_; 
  /** \brief red color for background and/or mixing if device does not support masking or alpha */
  uchar bg_r_; 
  /** \brief green color for background and/or mixing if device does not support masking or alpha */
  uchar bg_g_; 
  /** \brief blue color for background and/or mixing if device does not support masking or alpha */
  uchar bg_b_; 
  friend class Fl_Pixmap;
  friend class Fl_Bitmap;
  friend class Fl_RGB_Image;
  friend class Fl_PS_Device;
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
  friend void fl_begin_points();
  friend void fl_begin_line();
  friend void fl_begin_loop();
  friend void fl_begin_polygon();
  friend void fl_vertex(double x, double y);
  friend void fl_curve(double X0, double Y0, double X1, double Y1, double X2, double Y2, double X3, double Y3);
  friend void fl_circle(double x, double y, double r);
  friend void fl_arc(double x, double y, double r, double start, double end);
  friend void fl_arc(int x, int y, int w, int h, double a1, double a2);
  friend void fl_pie(int x, int y, int w, int h, double a1, double a2);
  friend void fl_end_points();
  friend void fl_end_line();
  friend void fl_end_loop();
  friend void fl_end_polygon();
  friend void fl_transformed_vertex(double xf, double yf);
  friend void fl_push_clip(int x, int y, int w, int h);
  friend int fl_clip_box(int x, int y, int w, int h, int &X, int &Y, int &W, int &H);
  friend int fl_not_clipped(int x, int y, int w, int h);
  friend void fl_push_no_clip();
  friend void fl_pop_clip();
  friend void fl_begin_complex_polygon();
  friend void fl_gap();
  friend void fl_end_complex_polygon();
  friend void fl_draw_image(const uchar* buf, int X,int Y,int W,int H, int D, int L);
  friend void fl_draw_image_mono(const uchar* buf, int X,int Y,int W,int H, int D, int L);
  friend void fl_draw_image(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D);
  friend FL_EXPORT void fl_draw_image_mono(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D);
  
  /** \brief see fl_rect(int x, int y, int w, int h). */
  virtual void rect(int x, int y, int w, int h);
  /** \brief see fl_rectf(int x, int y, int w, int h). */
  virtual void rectf(int x, int y, int w, int h);
  /** \brief see fl_line_style(int style, int width, char* dashes). */
  virtual void line_style(int style, int width=0, char* dashes=0);
  /** \brief see fl_xyline(int x, int y, int x1). */
  virtual void xyline(int x, int y, int x1);
  /** \brief see fl_xyline(int x, int y, int x1, int y2). */
  virtual void xyline(int x, int y, int x1, int y2);
  /** \brief see fl_xyline(int x, int y, int x1, int y2, int x3). */
  virtual void xyline(int x, int y, int x1, int y2, int x3);
  /** \brief see fl_yxline(int x, int y, int y1). */
  virtual void yxline(int x, int y, int y1);
  /** \brief see fl_yxline(int x, int y, int y1, int x2). */
  virtual void yxline(int x, int y, int y1, int x2);
  /** \brief see fl_yxline(int x, int y, int y1, int x2, int y3). */
  virtual void yxline(int x, int y, int y1, int x2, int y3);
  /** \brief see fl_line(int x, int y, int x1, int y1). */
  virtual void line(int x, int y, int x1, int y1);
  /** \brief see fl_line(int x, int y, int x1, int y1, int x2, int y2). */
  virtual void line(int x, int y, int x1, int y1, int x2, int y2);
  /** \brief see fl_draw(const char *str, int n, int x, int y). */
  virtual void draw(const char *str, int n, int x, int y);
  /** \brief see fl_draw(int angle, const char *str, int n, int x, int y). */
  virtual void draw(int angle, const char *str, int n, int x, int y);
  /** \brief see fl_font(Fl_Font face, Fl_Fontsize size). */
  virtual void font(Fl_Font face, Fl_Fontsize size);
  /** \brief see fl_color(Fl_Color c). */
  virtual void color(Fl_Color c);
  /** \brief see fl_color(uchar r, uchar g, uchar b). */
  virtual void color(uchar r, uchar g, uchar b);
  /** \brief see fl_point(int x, int y). */
  virtual void point(int x, int y);
  /** \brief see fl_loop(int x0, int y0, int x1, int y1, int x2, int y2). */
  virtual void loop(int x0, int y0, int x1, int y1, int x2, int y2);
  /** \brief see fl_loop(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3). */
  virtual void loop(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3);
  /** \brief see fl_polygon(int x0, int y0, int x1, int y1, int x2, int y2). */
  virtual void polygon(int x0, int y0, int x1, int y1, int x2, int y2);
  /** \brief see fl_polygon(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3). */
  virtual void polygon(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3);
  /** \brief see fl_begin_points(). */
  virtual void begin_points();
  /** \brief see fl_begin_line(). */
  virtual void begin_line();
  /** \brief see fl_begin_loop(). */
  virtual void begin_loop();
  /** \brief see fl_begin_polygon(). */
  virtual void begin_polygon();
  /** \brief see fl_vertex(double x, double y). */
  virtual void vertex(double x, double y);
  /** \brief see fl_curve(double X0, double Y0, double X1, double Y1, double X2, double Y2, double X3, double Y3). */
  virtual void curve(double X0, double Y0, double X1, double Y1, double X2, double Y2, double X3, double Y3);
  /** \brief see fl_circle(double x, double y, double r). */
  virtual void circle(double x, double y, double r);
  /** \brief see fl_arc(double x, double y, double r, double start, double end). */
  virtual void arc(double x, double y, double r, double start, double end);
  /** \brief see fl_arc(int x, int y, int w, int h, double a1, double a2). */
  virtual void arc(int x, int y, int w, int h, double a1, double a2);
  /** \brief see fl_pie(int x, int y, int w, int h, double a1, double a2). */
  virtual void pie(int x, int y, int w, int h, double a1, double a2);
  /** \brief see fl_end_points(). */
  virtual void end_points();
  /** \brief see fl_end_line(). */
  virtual void end_line();
  /** \brief see fl_end_loop(). */
  virtual void end_loop();
  /** \brief see fl_end_polygon(). */
  virtual void end_polygon();
  /** \brief see fl_begin_complex_polygon(). */
  virtual void begin_complex_polygon();
  /** \brief see fl_gap(). */
  virtual void gap();
  /** \brief see fl_end_complex_polygon(). */
  virtual void end_complex_polygon();
  /** \brief see fl_transformed_vertex(double xf, double yf). */
  virtual void transformed_vertex(double xf, double yf);
  /** \brief see fl_push_clip(int x, int y, int w, int h). */
  virtual void push_clip(int x, int y, int w, int h);
  /** \brief see fl_clip_box(int x, int y, int w, int h, int &X, int &Y, int &W, int &H). */
  virtual int clip_box(int x, int y, int w, int h, int &X, int &Y, int &W, int &H);
  /** \brief see fl_not_clipped(int x, int y, int w, int h). */
  virtual int not_clipped(int x, int y, int w, int h);
  /** \brief see fl_push_no_clip(). */
  virtual void push_no_clip();
  /** \brief see fl_pop_clip(). */
  virtual void pop_clip();
  // Images
  /** \brief see fl_draw_image(const uchar* buf, int X,int Y,int W,int H, int D, int L). */
  virtual   void draw_image(const uchar* buf, int X,int Y,int W,int H, int D=3, int L=0);
  /** \brief see fl_draw_image_mono(const uchar* buf, int X,int Y,int W,int H, int D, int L). */
  virtual   void draw_image_mono(const uchar* buf, int X,int Y,int W,int H, int D=1, int L=0);
  /** \brief see fl_draw_image(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D). */
  virtual   void draw_image(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=3);
  /** \brief see fl_draw_image_mono(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D). */
  virtual   void draw_image_mono(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=1);
  // Image classes
  /** \brief Draws an Fl_RGB_Image object to the device. 
   *
   Specifies a bounding box for the image, with the origin (upper left-hand corner) of 
   the image offset by the cx and cy arguments.
   */
  virtual   void draw(Fl_RGB_Image * rgb,int XP, int YP, int WP, int HP, int cx, int cy) {};
  /** \brief Draws an Fl_Pixmap object to the device. 
   *
   Specifies a bounding box for the image, with the origin (upper left-hand corner) of 
   the image offset by the cx and cy arguments.
   */
  virtual   void draw(Fl_Pixmap * pxm,int XP, int YP, int WP, int HP, int cx, int cy) {};
  /** \brief Draws an Fl_Bitmap object to the device. 
   *
   Specifies a bounding box for the image, with the origin (upper left-hand corner) of 
   the image offset by the cx and cy arguments.
   */
  virtual void draw(Fl_Bitmap *bm, int XP, int YP, int WP, int HP, int cx, int cy) {};
    
public:
  /** A name that identifies each subclass of Fl_Device */
  static const char *device_type;
  /** 
   @brief An RTTI emulation of device classes. 
   *
   The type of an instance of an Fl_Device subclass can be checked with code such as:
   \code
   if ( fl_device->type() == Fl_Printer::device_type ) { ... }
   \endcode
   */
  inline const char *type() {return type_;};
  virtual Fl_Device *set_current(void);

  virtual ~Fl_Device() {};
  static Fl_Device *current();
  
  /**
   @brief    Returns the platform's display device.
   */
  static Fl_Display_Device *display_device() { return fl_display_device; };
  
};

#if defined(__APPLE__) 
// The Mac OS X-specific graphics class.
class Fl_Graphics_Device : public Fl_Device {
protected:
  Fl_Graphics_Device() { type_ = device_type; };
public:
  static const char *device_type;
  void draw(Fl_Pixmap *pxm, int XP, int YP, int WP, int HP, int cx, int cy);
  void draw(Fl_Bitmap *pxm, int XP, int YP, int WP, int HP, int cx, int cy);
  void draw(Fl_RGB_Image *img, int XP, int YP, int WP, int HP, int cx, int cy);
};
typedef Fl_Graphics_Device Fl_Quartz_Device;

#elif defined(WIN32) 
// The MSWindows-specific graphics class.
class Fl_Graphics_Device : public Fl_Device {
protected:
  Fl_Graphics_Device() { type_ = device_type; };
public:
  static const char *device_type;
  void draw(Fl_Pixmap *pxm, int XP, int YP, int WP, int HP, int cx, int cy);
  void draw(Fl_Bitmap *pxm, int XP, int YP, int WP, int HP, int cx, int cy);
  void draw(Fl_RGB_Image *img, int XP, int YP, int WP, int HP, int cx, int cy);
};
typedef  Fl_Graphics_Device Fl_GDI_Device;

#else
/**
 @brief A class representing OS-specific graphics system.
 *
 This class is also named Fl_Quartz_Device on Mac OS X, Fl_GDI_Device on MS-Win, Fl_Xlib_Device on X11.
 A graphics system can be shared by various devices (e.g., display, printer, clipboard).
 */
class Fl_Graphics_Device : public Fl_Device {
protected:
  Fl_Graphics_Device() { type_ = device_type; };
public:
  static const char *device_type;
  void draw(Fl_Pixmap *pxm, int XP, int YP, int WP, int HP, int cx, int cy);
  void draw(Fl_Bitmap *pxm, int XP, int YP, int WP, int HP, int cx, int cy);
  void draw(Fl_RGB_Image *img, int XP, int YP, int WP, int HP, int cx, int cy);
};
typedef  Fl_Graphics_Device Fl_Xlib_Device;
#endif

/**
 @brief The platform's display.
*/
class Fl_Display_Device : public Fl_Graphics_Device {
public:
  static const char *device_type;
  Fl_Display_Device() { type_ = device_type; };
};


#endif // Fl_Device_H

//
// End of "$Id$".
//
