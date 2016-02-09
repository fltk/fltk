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

/** \file Fl_Graphics_Driver.H
 \brief declaration of classe Fl_Graphics_Driver.
*/

#ifndef FL_GRAPHICS_DRIVER_H
#define FL_GRAPHICS_DRIVER_H

#include <FL/x.H>
#include <FL/Fl_Device.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Bitmap.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_RGB_Image.H>
#include <stdlib.h>

class Fl_Graphics_Driver;
class Fl_Font_Descriptor;
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

// typedef what the x,y fields in a point are:
#ifdef WIN32
typedef int COORD_T;
#  define XPOINT XPoint
#elif defined(__APPLE__)
typedef float COORD_T;
typedef struct { float x; float y; } QPoint;
#  define XPOINT QPoint
extern float fl_quartz_line_width_;
#elif defined(FL_PORTING)
# pragma message "FL_PORTING: define types for COORD_T and XPOINT"
typedef int COORD_T; // default if not ported
typedef struct { int x; int y; } QPoint;
#  define XPOINT QPoint
#else
typedef short COORD_T;
#  define XPOINT XPoint
#endif


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
class FL_EXPORT Fl_Graphics_Driver : public Fl_Device {
  friend class Fl_Pixmap;
  friend class Fl_Bitmap;
  friend class Fl_RGB_Image;
public:
  // The following functions create the various graphics drivers that are required
  // for core operations. They must be implemented as members of Fl_Graphics_Driver,
  // but located in the device driver module that is linked to the core library
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

  int fl_clip_state_number;
protected:
  static const matrix m0;
  Fl_Font font_; // current font
  Fl_Fontsize size_; // current font size
  Fl_Color color_; // current color
  int sptr;
  static const int matrix_stack_size = FL_MATRIX_STACK_SIZE;
  matrix stack[FL_MATRIX_STACK_SIZE];
  matrix m;
  int n, p_size, gap_;
  XPOINT *p;
  int what;
  int rstackptr;
  static const int region_stack_max = FL_REGION_STACK_SIZE - 1;
  Fl_Region rstack[FL_REGION_STACK_SIZE];
  Fl_Font_Descriptor *font_descriptor_;
#ifndef FL_DOXYGEN
  enum {LINE, LOOP, POLYGON, POINT_};
  inline int vertex_no() { return n; }
  inline XPOINT *vertices() {return p;}
  inline int vertex_kind() {return what;}
#endif
  matrix *fl_matrix; /**< Points to the current coordinate transformation matrix */

  // === all code below in this class has been to the reorganisation FL_PORTING process
public:
  static const char *class_id;
  virtual const char *class_name() {return class_id;};
  Fl_Graphics_Driver();
  virtual ~Fl_Graphics_Driver() { if (p) free(p); }
public:
  // --- implementation is in src/fl_rect.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_rect.cxx
  virtual void point(int x, int y) = 0;
  virtual void rect(int x, int y, int w, int h) = 0;
  virtual void focus_rect(int x, int y, int w, int h);
  virtual void rectf(int x, int y, int w, int h) = 0;
  virtual void line(int x, int y, int x1, int y1) = 0;
  virtual void line(int x, int y, int x1, int y1, int x2, int y2) = 0;
  virtual void xyline(int x, int y, int x1) = 0;
  virtual void xyline(int x, int y, int x1, int y2) = 0;
  virtual void xyline(int x, int y, int x1, int y2, int x3) = 0;
  virtual void yxline(int x, int y, int y1) = 0;
  virtual void yxline(int x, int y, int y1, int x2) = 0;
  virtual void yxline(int x, int y, int y1, int x2, int y3) = 0;
  virtual void loop(int x0, int y0, int x1, int y1, int x2, int y2) = 0;
  virtual void loop(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) = 0;
  virtual void polygon(int x0, int y0, int x1, int y1, int x2, int y2) = 0;
  virtual void polygon(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) = 0;
  // --- clipping
  virtual void push_clip(int x, int y, int w, int h) = 0;
  virtual int clip_box(int x, int y, int w, int h, int &X, int &Y, int &W, int &H) = 0;
  virtual int not_clipped(int x, int y, int w, int h) = 0;
  virtual void push_no_clip() = 0;
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
  virtual void begin_complex_polygon() = 0;
  virtual double transform_x(double x, double y);
  virtual double transform_y(double x, double y);
  virtual double transform_dx(double x, double y);
  virtual double transform_dy(double x, double y);
  virtual void transformed_vertex(double xf, double yf) = 0;
  virtual void vertex(double x, double y) = 0;
  virtual void end_points() = 0;
  virtual void end_line() = 0;
  virtual void end_loop() = 0;
  virtual void end_polygon() = 0;
  virtual void end_complex_polygon() = 0;
  virtual void gap() = 0;
  virtual void circle(double x, double y, double r) = 0;
  // --- implementation is in src/fl_arc.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_arc.cxx if needed
  virtual void arc(double x, double y, double r, double start, double end);
  // --- implementation is in src/fl_arci.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_arci.cxx
  virtual void arc(int x, int y, int w, int h, double a1, double a2) = 0;
  virtual void pie(int x, int y, int w, int h, double a1, double a2) = 0;
  // --- implementation is in src/fl_curve.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_curve.cxx if needed
  virtual void curve(double X0, double Y0, double X1, double Y1, double X2, double Y2, double X3, double Y3);
  // --- implementation is in src/fl_line_style.cxx which includes src/cfg_gfx/xxx_line_style.cxx
  virtual void line_style(int style, int width=0, char* dashes=0) = 0;
  // --- implementation is in src/fl_color.cxx which includes src/cfg_gfx/xxx_color.cxx
  virtual void color(Fl_Color c) { color_ = c; }
  virtual Fl_Color color() { return color_; }
  virtual void color(uchar r, uchar g, uchar b) = 0;
  // --- implementation is in src/fl_font.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_font.cxx
  virtual void draw(const char *str, int n, int x, int y) = 0;
  virtual void draw(const char *str, int n, float x, float y) { draw(str, n, (int)(x+0.5), (int)(y+0.5));}
  virtual void draw(int angle, const char *str, int n, int x, int y) { draw(str, n, x, y); }
  virtual void rtl_draw(const char *str, int n, int x, int y) { draw(str, n, x, y); }
  /** Returns non-zero if the graphics driver possesses the \p feature */
  virtual int has_feature(driver_feature feature) { return 0; }
  virtual void font(Fl_Font face, Fl_Fontsize fsize) {font_ = face; size_ = fsize;}
  virtual Fl_Font font() {return font_; }
  virtual Fl_Fontsize size() {return size_; }
  virtual double width(const char *str, int n) { return 0; }
  virtual double width(unsigned int c) { char ch = (char)c; return width(&ch, 1); }
  virtual void text_extents(const char*, int n, int& dx, int& dy, int& w, int& h);
  virtual int height() { return size(); }
  virtual int descent() { return 0; }
  virtual Fl_Font_Descriptor *font_descriptor() { return font_descriptor_;}
  virtual void font_descriptor(Fl_Font_Descriptor *d) { font_descriptor_ = d;}
  // --- implementation is in src/fl_image.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_font.cxx
  virtual Fl_Bitmask create_bitmask(int w, int h, const uchar *array) = 0;
  virtual fl_uintptr_t cache(Fl_Pixmap *img, int w, int h, const char *const*array) { return 0; }
  virtual fl_uintptr_t cache(Fl_Bitmap *img, int w, int h, const uchar *array) { return 0; }
  virtual void uncache(Fl_Bitmap *img, fl_uintptr_t &id_) { }
  virtual void uncache(Fl_RGB_Image *img, fl_uintptr_t &id_, fl_uintptr_t &mask_) { }
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

protected:
  // --- implementation is in src/fl_vertex.cxx which includes src/cfg_gfx/xxx_rect.cxx
  virtual void transformed_vertex0(COORD_T x, COORD_T y);
  virtual void fixloop();
};

#endif // FL_GRAPHICS_DRIVER_H

//
// End of "$Id$".
//
