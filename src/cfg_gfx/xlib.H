//
// "$Id$"
//
// Definition of classes Fl_Device, Fl_Graphics_Driver, Fl_Surface_Device, Fl_Display_Device
// for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2014 by Bill Spitzak and others.
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
 \file xlib.H
 \brief Definition of X11 Xlib graphics driver.
 */

#ifndef FL_CFG_GFX_XLIB_H
#define FL_CFG_GFX_XLIB_H

#include <FL/Fl_Device.H>

/**
 \brief The Xlib-specific graphics class.
 *
 This class is implemented only on the Xlib platform.
 */
class FL_EXPORT Fl_Xlib_Graphics_Driver : public Fl_Graphics_Driver {
public:
  static const char *class_id;
  const char *class_name() {return class_id;};
  void color(Fl_Color c);
  void color(uchar r, uchar g, uchar b);
  void draw(const char* str, int n, int x, int y);
  void draw(int angle, const char *str, int n, int x, int y);
  void rtl_draw(const char* str, int n, int x, int y);
  void font(Fl_Font face, Fl_Fontsize size);
  void draw(Fl_Pixmap *pxm, int XP, int YP, int WP, int HP, int cx, int cy);
  void draw(Fl_Bitmap *pxm, int XP, int YP, int WP, int HP, int cx, int cy);
  void draw(Fl_RGB_Image *img, int XP, int YP, int WP, int HP, int cx, int cy);
  void draw_image(const uchar* buf, int X,int Y,int W,int H, int D=3, int L=0);
  void draw_image(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=3);
  void draw_image_mono(const uchar* buf, int X,int Y,int W,int H, int D=1, int L=0);
  void draw_image_mono(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=1);
  double width(const char *str, int n);
  double width(unsigned int c);
  void text_extents(const char*, int n, int& dx, int& dy, int& w, int& h);
  int height();
  int descent();
  void copy_offscreen(int x, int y, int w, int h, Fl_Offscreen pixmap, int srcx, int srcy);
#if ! defined(FL_DOXYGEN)
  void copy_offscreen_with_alpha(int x, int y, int w, int h, Fl_Offscreen pixmap, int srcx, int srcy);
#endif
protected:
  // --- implementation is in src/fl_rect.cxx which includes src/cfg_gfx/xlib_rect.cxx
  void point(int x, int y);
  void rect(int x, int y, int w, int h);
  void rectf(int x, int y, int w, int h);
  void line(int x, int y, int x1, int y1);
  void line(int x, int y, int x1, int y1, int x2, int y2);
  void xyline(int x, int y, int x1);
  void xyline(int x, int y, int x1, int y2);
  void xyline(int x, int y, int x1, int y2, int x3);
  void yxline(int x, int y, int y1);
  void yxline(int x, int y, int y1, int x2);
  void yxline(int x, int y, int y1, int x2, int y3);
  void loop(int x0, int y0, int x1, int y1, int x2, int y2);
  void loop(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3);
  void polygon(int x0, int y0, int x1, int y1, int x2, int y2);
  void polygon(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3);
  // --- clipping
  void push_clip(int x, int y, int w, int h);
  int clip_box(int x, int y, int w, int h, int &X, int &Y, int &W, int &H);
  int not_clipped(int x, int y, int w, int h);
  void push_no_clip();
  void pop_clip();
  void restore_clip();
};

#endif

/**
 A drawing surface that's susceptible to receive graphical output.
 Any FLTK application has at any time a current drawing surface to which all drawing requests are directed.
 The current surface is given by Fl_Surface_Device::surface().
 When main() begins running, the current drawing surface has been set to the computer's display, 
 an instance of the Fl_Display_Device class.

 A drawing surface other than the computer's display, is typically used as follows:
 <ol><li> Create \c surface, an object from a particular Fl_Surface_Device derived class (e.g., Fl_Copy_Surface, Fl_Printer).
 <li> Memorize what is the current drawing surface with <tt> Fl_Surface_Device *old_current = Fl_Surface_Device::surface();</tt>
 <li> Call \c surface->set_current(); to redirect all graphics requests to \c surface which becomes the new
 current drawing surface (not necessary with class Fl_Printer because it is done by Fl_Printer::start_job()).
 <li> At this point any of the \ref fl_drawings (e.g., fl_rect()) or the \ref fl_attributes or \ref drawing_images functions
 (e.g., fl_draw_image(), Fl_Image::draw()) operates on the new current drawing surface.
 Certain drawing surfaces allow additional ways to draw to them (e.g., Fl_Printer::print_widget(), Fl_Image_Surface::draw()).
 <li> After all drawing requests have been performed, redirect graphics requests back to their previous destination
 with \c old_current->set_current();.
 <li> Delete \c surface.
 </ol>
 */
class FL_EXPORT Fl_Surface_Device : public Fl_Device {
  /** \brief The graphics driver in use by this surface. */
  Fl_Graphics_Driver *_driver;
  static Fl_Surface_Device *_surface; // the surface that currently receives graphics output
  static Fl_Surface_Device *default_surface(); // create surface is none exists yet
protected:
  /** \brief Constructor that sets the graphics driver to use for the created surface. */
  Fl_Surface_Device(Fl_Graphics_Driver *graphics_driver) {_driver = graphics_driver; };
public:
  static const char *class_id;
  const char *class_name() {return class_id;};
  virtual void set_current(void);
  /** \brief Sets the graphics driver of this drawing surface. */
  inline void driver(Fl_Graphics_Driver *graphics_driver) {_driver = graphics_driver;};
  /** \brief Returns the graphics driver of this drawing surface. */
  inline Fl_Graphics_Driver *driver() {return _driver; };
  /** The current drawing surface.
   In other words, the Fl_Surface_Device object that currently receives all graphics output */
  static inline Fl_Surface_Device *surface() {
    return _surface ? _surface : default_surface();
  };
  /** \brief The destructor. */
  virtual ~Fl_Surface_Device() {}
};

/**
 A display to which the computer can draw.
 When the program begins running, an Fl_Display_Device instance has been created and made the current drawing surface.
 There is no need to create any other object of this class.
 */
class FL_EXPORT Fl_Display_Device : public Fl_Surface_Device {
  friend class Fl_Quartz_Graphics_Driver;
  static Fl_Display_Device *_display; // the platform display device
#ifdef __APPLE__
  friend class Fl_X;
  friend class Fl_Graphics_Driver;
  static bool high_res_window_; //< true when drawing to a window of a retina display (Mac OS X only)
  static bool high_resolution() {return high_res_window_;}
#elif defined(WIN32)
#elif defined(FL_PORTING)
# pragma message "FL_PORTING: implement functions for extra high res drawing if your platform supports it"
#else
#endif
public:
  static const char *class_id;
  const char *class_name() {return class_id;};
  Fl_Display_Device(Fl_Graphics_Driver *graphics_driver);
  static Fl_Display_Device *display_device();
};

/**
 This plugin socket allows the integration of new device drivers for special
 window or screen types. It is currently used to provide an automated printing
 service and screen capture for OpenGL windows, if linked with fltk_gl.
 */
class FL_EXPORT Fl_Device_Plugin : public Fl_Plugin {
public:
  /** \brief The constructor */
  Fl_Device_Plugin(const char *pluginName)
  : Fl_Plugin(klass(), pluginName) { }
  /** \brief Returns the class name */
  virtual const char *klass() { return "fltk:device"; }
  /** \brief Returns the plugin name */
  virtual const char *name() = 0;
  /** \brief Prints a widget 
   \param w the widget
   \param x,y offsets where to print relatively to coordinates origin
   \param height height of the current drawing area
   */
  virtual int print(Fl_Widget* w, int x, int y, int height) = 0;
  /** captures a rectangle of a widget as an image
   \return The captured pixels as an RGB image
   */
  virtual Fl_RGB_Image* rectangle_capture(Fl_Widget *widget, int x, int y, int w, int h) = 0;
};

#endif // FL_CFG_GFX_XLIB_H

//
// End of "$Id$".
//
