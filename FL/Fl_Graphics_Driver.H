//
// "$Id$"
//
// Definition of classes  Fl_Graphics_Driver, Fl_Surface_Device, Fl_Display_Device
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

/** \file Fl_Graphics_Driver.H
 \brief declaration of classe Fl_Graphics_Driver.
*/

#ifndef FL_GRAPHICS_DRIVER_H
#define FL_GRAPHICS_DRIVER_H

#include <FL/Fl_System_Driver.H>
#include <FL/Fl_Device.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Bitmap.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_RGB_Image.H>
#include <stdlib.h>

class Fl_Graphics_Driver;
/** a platform-specific class implementing a system font */
class Fl_Font_Descriptor
#ifdef FL_DOXYGEN
{}
#endif
;
/** \brief Points to the driver that currently receives all graphics requests */
FL_EXPORT extern Fl_Graphics_Driver *fl_graphics_driver;

/**
 signature of image generation callback function.
 \param[in]  data  user data passed to function
 \param[in]  x,y,w position and width of scan line in image
 \param[out] buf   buffer for generated image data. You must copy \p w
 pixels from scanline \p y, starting at pixel \p x
 to this buffer.
 */
typedef void (*Fl_Draw_Image_Cb)(void* data,int x,int y,int w,uchar* buf);


#define FL_REGION_STACK_SIZE 10
#define FL_MATRIX_STACK_SIZE 32
/**
 \brief A virtual class subclassed for each graphics driver FLTK uses.
 Typically, FLTK applications do not use directly objects from this class. Rather, they perform
 drawing operations (e.g., fl_rectf()) that operate on the current drawing surface (see Fl_Surface_Device).
 Drawing operations are functionally presented in \ref drawing and as function lists
 in the \ref fl_drawings and \ref fl_attributes modules. The \ref fl_graphics_driver global variable
 gives at any time the graphics driver used by all drawing operations. Its value changes when
 drawing operations are directed to another drawing surface by Fl_Surface_Device::set_current().

 \p The Fl_Graphics_Driver class is of interest if one wants to perform new kinds of drawing operations.
 An example would be to draw to a PDF file. This would involve creating a new Fl_Graphics_Driver derived
 class. This new class should implement all virtual methods of the Fl_Graphics_Driver class
 to support all FLTK drawing functions.
  */
class FL_EXPORT Fl_Graphics_Driver {
  friend class Fl_Surface_Device;
  friend class Fl_Display_Device;
  friend class Fl_Pixmap;
  friend class Fl_Bitmap;
  friend class Fl_RGB_Image;
  friend int fl_draw_pixmap(const char*const* cdata, int x, int y, Fl_Color bg);
  friend void gl_start();
public:
  // The following functions create the various graphics drivers that are required
  // for core operations. They must be implemented as members of Fl_Graphics_Driver,
  // but located in the device driver module that is linked to the core library
  /** Instantiate the graphics driver adequate to draw to the platform's display driver.
   Each platform implements this method its own way.
   */
  static Fl_Graphics_Driver *newMainGraphicsDriver();
  //static Fl_Graphics_Driver *newOpenGLGraphicsDriver();
  //static Fl_Graphics_Driver *newPrinterGraphicsDriver();
  //static Fl_Graphics_Driver *new...;
public:
  /** A 2D coordinate transformation matrix */
  struct matrix {double a, b, c, d, x, y;};
  /** Features that a derived class may possess.  */
  typedef enum {
    NATIVE = 1, /**< native graphics driver for the platform */
    PRINTER = 2 /**< graphics driver for a printer drawing surface */
  } driver_feature;

protected:
  int fl_clip_state_number; ///< For internal use by FLTK
  static const matrix m0; ///< For internal use by FLTK
  Fl_Font font_; ///< current font
  Fl_Fontsize size_; ///< current font size
  Fl_Color color_; ///< current color
  int sptr;///< For internal use by FLTK
  static const int matrix_stack_size = FL_MATRIX_STACK_SIZE; ///< For internal use by FLTK
  matrix stack[FL_MATRIX_STACK_SIZE]; ///< For internal use by FLTK
  matrix m; ///< current transformation matrix
  int n; ///< For internal use by FLTK
  int gap_; ///< For internal use by FLTK
  int what; ///< For internal use by FLTK
  int rstackptr; ///< For internal use by FLTK
  static const int region_stack_max = FL_REGION_STACK_SIZE - 1; ///< For internal use by FLTK
  Fl_Region rstack[FL_REGION_STACK_SIZE]; ///< For internal use by FLTK
  Fl_Font_Descriptor *font_descriptor_; ///< For internal use by FLTK
#ifndef FL_DOXYGEN
  enum {LINE, LOOP, POLYGON, POINT_};
  inline int vertex_no() { return n; }
  inline int vertex_kind() {return what;}
#endif
  matrix *fl_matrix; /**< Points to the current coordinate transformation matrix */
  /** Returns whether the graphics driver is currently drawing to a high resolution surface */
  virtual bool high_resolution() { return false; }
  virtual void global_gc();
  virtual fl_uintptr_t cache(Fl_Pixmap *img, int w, int h, const char *const*array) { return 0; }
  virtual fl_uintptr_t cache(Fl_Bitmap *img, int w, int h, const uchar *array) { return 0; }
  virtual void uncache(Fl_RGB_Image *img, fl_uintptr_t &id_, fl_uintptr_t &mask_) { }

public:
  Fl_Graphics_Driver();
  virtual ~Fl_Graphics_Driver() {} ///< Destructor
  static Fl_Graphics_Driver &default_driver();
  /** Return whether the graphics driver can do alpha blending */
  virtual char can_do_alpha_blending() { return 0; }
  // --- implementation is in src/fl_rect.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_rect.cxx
  /** see fl_point() */
  virtual void point(int x, int y) = 0;
  /** see fl_rect() */
  virtual void rect(int x, int y, int w, int h) = 0;
  virtual void focus_rect(int x, int y, int w, int h);
  /** see fl_rectf() */
  virtual void rectf(int x, int y, int w, int h) = 0;
  /** see fl_line(int, int, int, int) */
  virtual void line(int x, int y, int x1, int y1) = 0;
  /** see fl_line(int, int, int, int, int, int) */
  virtual void line(int x, int y, int x1, int y1, int x2, int y2) = 0;
  /** see fl_xyline(int, int, int) */
  virtual void xyline(int x, int y, int x1) = 0;
  /** see fl_xyline(int, int, int, int) */
  virtual void xyline(int x, int y, int x1, int y2) = 0;
  /** see fl_xyline(int, int, int, int, int) */
  virtual void xyline(int x, int y, int x1, int y2, int x3) = 0;
  /** see fl_yxline(int, int, int) */
  virtual void yxline(int x, int y, int y1) = 0;
  /** see fl_yxline(int, int, int, int) */
  virtual void yxline(int x, int y, int y1, int x2) = 0;
  /** see fl_yxline(int, int, int, int, int) */
  virtual void yxline(int x, int y, int y1, int x2, int y3) = 0;
  /** see fl_loop(int, int, int, int, int, int) */
  virtual void loop(int x0, int y0, int x1, int y1, int x2, int y2) = 0;
  /** see fl_loop(int, int, int, int, int, int, int, int) */
  virtual void loop(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) = 0;
  /** see fl_polygon(int, int, int, int, int, int) */
  virtual void polygon(int x0, int y0, int x1, int y1, int x2, int y2) = 0;
  /** see fl_polygon(int, int, int, int, int, int, int, int) */
  virtual void polygon(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) = 0;
  // --- clipping
  /** see fl_push_clip() */
  virtual void push_clip(int x, int y, int w, int h) = 0;
  /** see fl_clip_box() */
  virtual int clip_box(int x, int y, int w, int h, int &X, int &Y, int &W, int &H) = 0;
  /** see fl_not_clipped() */
  virtual int not_clipped(int x, int y, int w, int h) = 0;
  /** see fl_push_no_clip() */
  virtual void push_no_clip() = 0;
  /** see fl_pop_clip() */
  virtual void pop_clip() = 0;
  virtual Fl_Region clip_region();              // has default implementation
  virtual void clip_region(Fl_Region r);        // has default implementation
  virtual void restore_clip();
  // --- implementation is in src/fl_vertex.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_vertex.cxx
  virtual void push_matrix();
  virtual void pop_matrix();
  virtual void mult_matrix(double a, double b, double c, double d, double x, double y);
  virtual void rotate(double d);
  virtual void scale(double x, double y);
  virtual void scale(double x);
  virtual void translate(double x,double y);
  virtual void begin_points();
  virtual void begin_line();
  virtual void begin_loop();
  virtual void begin_polygon();
  /** see fl_begin_complex_polygon() */
  virtual void begin_complex_polygon() = 0;
  virtual double transform_x(double x, double y);
  virtual double transform_y(double x, double y);
  virtual double transform_dx(double x, double y);
  virtual double transform_dy(double x, double y);
  /** see fl_transformed_vertex() */
  virtual void transformed_vertex(double xf, double yf) = 0;
  /** see fl_vertex() */
  virtual void vertex(double x, double y) = 0;
  /** see fl_end_points() */
  virtual void end_points() = 0;
  /** see fl_end_line() */
  virtual void end_line() = 0;
  /** see fl_end_loop() */
  virtual void end_loop() = 0;
  /** see fl_end_polygon() */
  virtual void end_polygon() = 0;
  /** see fl_end_complex_polygon() */
  virtual void end_complex_polygon() = 0;
  /** see fl_gap() */
  virtual void gap() = 0;
  /** see fl_circle() */
  virtual void circle(double x, double y, double r) = 0;
  // --- implementation is in src/fl_arc.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_arc.cxx if needed
  virtual void arc(double x, double y, double r, double start, double end);
  // --- implementation is in src/fl_arci.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_arci.cxx
  /** see fl_arc(int x, int y, int w, int h, double a1, double a2) */
  virtual void arc(int x, int y, int w, int h, double a1, double a2) = 0;
  /** see fl_pie() */
  virtual void pie(int x, int y, int w, int h, double a1, double a2) = 0;
  // --- implementation is in src/fl_curve.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_curve.cxx if needed
  virtual void curve(double X0, double Y0, double X1, double Y1, double X2, double Y2, double X3, double Y3);
  // --- implementation is in src/fl_line_style.cxx which includes src/cfg_gfx/xxx_line_style.cxx
  /** see fl_line_style() */
  virtual void line_style(int style, int width=0, char* dashes=0) = 0;
  // --- implementation is in src/fl_color.cxx which includes src/cfg_gfx/xxx_color.cxx
  /** see fl_color(Fl_Color) */
  virtual void color(Fl_Color c) { color_ = c; }
  virtual void set_color(Fl_Color i, unsigned int c);
  virtual void free_color(Fl_Color i, int overlay);
  /** see fl_color(void) */
  virtual Fl_Color color() { return color_; }
  /** see fl_color(uchar, uchar, uchar) */
  virtual void color(uchar r, uchar g, uchar b) = 0;
  // --- implementation is in src/fl_font.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_font.cxx
  /** see fl_draw(const char *str, int n, int x, int y) */
  virtual void draw(const char *str, int n, int x, int y) = 0;
  /** Draw the first \p n bytes of the string \p str starting at position \p x , \p y */
  virtual void draw(const char *str, int n, float x, float y) { draw(str, n, (int)(x+0.5), (int)(y+0.5));}
  /** see fl_draw(int angle, const char *str, int n, int x, int y) */
  virtual void draw(int angle, const char *str, int n, int x, int y) { draw(str, n, x, y); }
  /** see fl_rtl_draw(const char *str, int n, int x, int y) */
  virtual void rtl_draw(const char *str, int n, int x, int y) { draw(str, n, x, y); }
  /** Returns non-zero if the graphics driver possesses the \p feature */
  virtual int has_feature(driver_feature feature) { return 0; }
  /** see fl_font(Fl_Font, Fl_Fontsize) */
  virtual void font(Fl_Font face, Fl_Fontsize fsize) {font_ = face; size_ = fsize;}
  /** see fl_font(void) */
  virtual Fl_Font font() {return font_; }
  /** Return the current font size */
  virtual Fl_Fontsize size() {return size_; }
  /** Compute the width of the first \p n bytes of the string \p str if drawn with current font */
  virtual double width(const char *str, int n) { return 0; }
  /** Compute the width of Unicode character \p c if drawn with current font */
  virtual double width(unsigned int c) { char ch = (char)c; return width(&ch, 1); }
  virtual void text_extents(const char*, int n, int& dx, int& dy, int& w, int& h);
  /** Return the current line height */
  virtual int height() { return size(); }
  /** Return the current line descent */
  virtual int descent() { return 0; }
  /** Return the current Fl_Font_Descriptor */
  virtual Fl_Font_Descriptor *font_descriptor() { return font_descriptor_;}
  /** Set the current Fl_Font_Descriptor */
  virtual void font_descriptor(Fl_Font_Descriptor *d) { font_descriptor_ = d;}
  // --- implementation is in src/fl_image.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_font.cxx
  virtual Fl_Bitmask create_bitmask(int w, int h, const uchar *array) = 0;
  virtual void delete_bitmask(Fl_Bitmask bm) = 0;
  virtual void draw_image(const uchar* buf, int X,int Y,int W,int H, int D=3, int L=0) {}
  virtual void draw_image_mono(const uchar* buf, int X,int Y,int W,int H, int D=1, int L=0) {}
  virtual void draw_image(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=3) {}
  virtual void draw_image_mono(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=1) {}
  /** \brief Draws an Fl_RGB_Image object to the device.
   *
   Specifies a bounding box for the image, with the origin (upper left-hand corner) of
   the image offset by the cx and cy arguments.
   */
  virtual void draw(Fl_RGB_Image * rgb,int XP, int YP, int WP, int HP, int cx, int cy) {}
  /** \brief Draws an Fl_Pixmap object to the device.
   *
   Specifies a bounding box for the image, with the origin (upper left-hand corner) of
   the image offset by the cx and cy arguments.
   */
  virtual void draw(Fl_Pixmap * pxm,int XP, int YP, int WP, int HP, int cx, int cy) {}
  /** \brief Draws an Fl_Bitmap object to the device.
   *
   Specifies a bounding box for the image, with the origin (upper left-hand corner) of
   the image offset by the cx and cy arguments.
   */
  virtual void draw(Fl_Bitmap *bm, int XP, int YP, int WP, int HP, int cx, int cy) {}
  virtual int draw_scaled(Fl_Image *img, int X, int Y, int W, int H);
  virtual void copy_offscreen(int x, int y, int w, int h, Fl_Offscreen pixmap, int srcx, int srcy);

  /** Sets the value of the driver-specific graphics context. */
  virtual void gc(void*) {}
  /** Returns the driver-specific graphics context, of NULL if there's none. */
  virtual void *gc(void) {return NULL;}
  /** Support for pixmap drawing */
  virtual uchar **mask_bitmap() { return 0; }
  /** Support for pixmap drawing */
  virtual void mask_bitmap(uchar **) {}
  // default implementation may be enough
  /** Support for PostScript drawing */
  virtual float scale_font_for_PostScript(Fl_Font_Descriptor *desc, int s) { return float(s); }
  // default implementation may be enough
  /** Support for PostScript drawing */
  virtual float scale_bitmap_for_PostScript() { return 2; }
  virtual void set_spot(int font, int size, int X, int Y, int W, int H, Fl_Window *win);
  virtual void reset_spot();
  // each platform implements these 3 functions its own way
  virtual void add_rectangle_to_region(Fl_Region r, int x, int y, int w, int h);
  virtual Fl_Region XRectangleRegion(int x, int y, int w, int h);
  virtual void XDestroyRegion(Fl_Region r);
};

#endif // FL_GRAPHICS_DRIVER_H

//
// End of "$Id$".
//
