//
// "$Id: Fl_Pico_Graphics_Driver.H 11017 2016-01-20 21:40:12Z matt $"
//
// Definition of the Pico minimal graphics driver
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
 \file Fl_Pico_Graphics_Driver.H
 \brief Definition of Pico minimal graphics driver.
 */

#ifndef FL_PICO_GRAPHICS_DRIVER_H
#define FL_PICO_GRAPHICS_DRIVER_H

#include <FL/Fl_Graphics_Driver.H>


/**
 \brief The Pico minimal graphics class.
 *
 This class is implemented as a base class for minimal core drivers.
 */
class Fl_Pico_Graphics_Driver : public Fl_Graphics_Driver {
//  friend class Fl_Surface_Device;
//  friend class Fl_Pixmap;
//  friend class Fl_Bitmap;
//  friend class Fl_RGB_Image;
//  friend int fl_draw_pixmap(const char*const* cdata, int x, int y, Fl_Color bg);
//public:
//  // The following functions create the various graphics drivers that are required
//  // for core operations. They must be implemented as members of Fl_Graphics_Driver,
//  // but located in the device driver module that is linked to the core library
//  static Fl_Graphics_Driver *newMainGraphicsDriver();
//  //static Fl_Graphics_Driver *newOpenGLGraphicsDriver();
//  //static Fl_Graphics_Driver *newPrinterGraphicsDriver();
//  //static Fl_Graphics_Driver *new...;
//public:
//  /** A 2D coordinate transformation matrix */
//  struct matrix {double a, b, c, d, x, y;};
//  /** Features that a derived class may possess.  */
//  typedef enum {
//    NATIVE = 1, /**< native graphics driver for the platform */
//    PRINTER = 2 /**< graphics driver for a printer drawing surface */
//  } driver_feature;
//
//  int fl_clip_state_number;
//protected:
//  static const matrix m0;
//  Fl_Font font_; // current font
//  Fl_Fontsize size_; // current font size
//  Fl_Color color_; // current color
//  int sptr;
//  static const int matrix_stack_size = FL_MATRIX_STACK_SIZE;
//  matrix stack[FL_MATRIX_STACK_SIZE];
//  matrix m;
//  int n, p_size, gap_;
//  XPOINT *p;
//  int what;
//  int rstackptr;
//  static const int region_stack_max = FL_REGION_STACK_SIZE - 1;
//  Fl_Region rstack[FL_REGION_STACK_SIZE];
//  Fl_Font_Descriptor *font_descriptor_;
//#ifndef FL_DOXYGEN
//  enum {LINE, LOOP, POLYGON, POINT_};
//  inline int vertex_no() { return n; }
//  inline XPOINT *vertices() {return p;}
//  inline int vertex_kind() {return what;}
//#endif
//  matrix *fl_matrix; /**< Points to the current coordinate transformation matrix */
//
//  // === all code below in this class has been to the reorganisation FL_PORTING process
//public:
//  Fl_Graphics_Driver();
//  virtual ~Fl_Graphics_Driver() { if (p) free(p); }
//  virtual char can_do_alpha_blending() { return 0; }
//  // --- implementation is in src/fl_rect.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_rect.cxx
  virtual void point(int x, int y);
  virtual void rect(int x, int y, int w, int h);
//  virtual void focus_rect(int x, int y, int w, int h);
  virtual void rectf(int x, int y, int w, int h);
  virtual void line(int x, int y, int x1, int y1);
  virtual void line(int x, int y, int x1, int y1, int x2, int y2);
  virtual void xyline(int x, int y, int x1);
  virtual void xyline(int x, int y, int x1, int y2);
  virtual void xyline(int x, int y, int x1, int y2, int x3) ;
  virtual void yxline(int x, int y, int y1) ;
  virtual void yxline(int x, int y, int y1, int x2) ;
  virtual void yxline(int x, int y, int y1, int x2, int y3) ;
  virtual void loop(int x0, int y0, int x1, int y1, int x2, int y2) ;
  virtual void loop(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) ;
  virtual void polygon(int x0, int y0, int x1, int y1, int x2, int y2) ;
  virtual void polygon(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) ;
//  // --- clipping
  virtual void push_clip(int x, int y, int w, int h) ;
  virtual int clip_box(int x, int y, int w, int h, int &X, int &Y, int &W, int &H) ;
  virtual int not_clipped(int x, int y, int w, int h) ;
  virtual void push_no_clip() ;
  virtual void pop_clip() ;
//  virtual Fl_Region clip_region();              // has default implementation
//  virtual void clip_region(Fl_Region r);        // has default implementation
//  virtual void restore_clip();
//  // --- implementation is in src/fl_vertex.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_vertex.cxx
//  virtual void push_matrix();
//  virtual void pop_matrix();
//  virtual void mult_matrix(double a, double b, double c, double d, double x, double y);
//  virtual void rotate(double d);
//  virtual void scale(double x, double y);
//  virtual void scale(double x);
//  virtual void translate(double x,double y);
//  virtual void begin_points();
//  virtual void begin_line();
//  virtual void begin_loop();
//  virtual void begin_polygon();
  virtual void begin_complex_polygon() ;
//  virtual double transform_x(double x, double y);
//  virtual double transform_y(double x, double y);
//  virtual double transform_dx(double x, double y);
//  virtual double transform_dy(double x, double y);
  virtual void transformed_vertex(double xf, double yf) ;
  virtual void vertex(double x, double y) ;
  virtual void end_points() ;
  virtual void end_line() ;
  virtual void end_loop() ;
  virtual void end_polygon() ;
  virtual void end_complex_polygon() ;
  virtual void gap() ;
  virtual void circle(double x, double y, double r) ;
//  // --- implementation is in src/fl_arc.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_arc.cxx if needed
//  virtual void arc(double x, double y, double r, double start, double end);
//  // --- implementation is in src/fl_arci.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_arci.cxx
  virtual void arc(int x, int y, int w, int h, double a1, double a2) ;
  virtual void pie(int x, int y, int w, int h, double a1, double a2) ;
//  // --- implementation is in src/fl_curve.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_curve.cxx if needed
//  virtual void curve(double X0, double Y0, double X1, double Y1, double X2, double Y2, double X3, double Y3);
//  // --- implementation is in src/fl_line_style.cxx which includes src/cfg_gfx/xxx_line_style.cxx
  virtual void line_style(int style, int width=0, char* dashes=0) ;
//  // --- implementation is in src/fl_color.cxx which includes src/cfg_gfx/xxx_color.cxx
//  virtual void color(Fl_Color c) { color_ = c; }
//  virtual Fl_Color color() { return color_; }
  virtual void color(uchar r, uchar g, uchar b) ;
//  // --- implementation is in src/fl_font.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_font.cxx
  virtual void draw(const char *str, int n, int x, int y) ;
//  virtual void draw(const char *str, int n, float x, float y) { draw(str, n, (int)(x+0.5), (int)(y+0.5));}
//  virtual void draw(int angle, const char *str, int n, int x, int y) { draw(str, n, x, y); }
//  virtual void rtl_draw(const char *str, int n, int x, int y) { draw(str, n, x, y); }
//  /** Returns non-zero if the graphics driver possesses the \p feature */
//  virtual int has_feature(driver_feature feature) { return 0; }
//  virtual void font(Fl_Font face, Fl_Fontsize fsize) {font_ = face; size_ = fsize;}
//  virtual Fl_Font font() {return font_; }
//  virtual Fl_Fontsize size() {return size_; }
  virtual double width(const char *str, int n);
//  virtual double width(unsigned int c) { char ch = (char)c; return width(&ch, 1); }
  virtual int height();
  virtual int descent();
//  virtual Fl_Font_Descriptor *font_descriptor() { return font_descriptor_;}
//  virtual void font_descriptor(Fl_Font_Descriptor *d) { font_descriptor_ = d;}
//  // --- implementation is in src/fl_image.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_font.cxx
  virtual Fl_Bitmask create_bitmask(int w, int h, const uchar *array) ;
//  virtual fl_uintptr_t cache(Fl_Pixmap *img, int w, int h, const char *const*array) { return 0; }
//  virtual fl_uintptr_t cache(Fl_Bitmap *img, int w, int h, const uchar *array) { return 0; }
//  virtual void uncache(Fl_RGB_Image *img, fl_uintptr_t &id_, fl_uintptr_t &mask_) { }
  virtual void delete_bitmask(Fl_Bitmask bm) ;
//  virtual void draw_image(const uchar* buf, int X,int Y,int W,int H, int D=3, int L=0) {}
//  virtual void draw_image_mono(const uchar* buf, int X,int Y,int W,int H, int D=1, int L=0) {}
//  virtual void draw_image(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=3) {}
//  virtual void draw_image_mono(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=1) {}
//  /** \brief Draws an Fl_RGB_Image object to the device.
//   *
//   Specifies a bounding box for the image, with the origin (upper left-hand corner) of
//   the image offset by the cx and cy arguments.
//   */
//  virtual void draw(Fl_RGB_Image * rgb,int XP, int YP, int WP, int HP, int cx, int cy) {}
//  /** \brief Draws an Fl_Pixmap object to the device.
//   *
//   Specifies a bounding box for the image, with the origin (upper left-hand corner) of
//   the image offset by the cx and cy arguments.
//   */
//  virtual void draw(Fl_Pixmap * pxm,int XP, int YP, int WP, int HP, int cx, int cy) {}
//  /** \brief Draws an Fl_Bitmap object to the device.
//   *
//   Specifies a bounding box for the image, with the origin (upper left-hand corner) of
//   the image offset by the cx and cy arguments.
//   */
//  virtual void draw(Fl_Bitmap *bm, int XP, int YP, int WP, int HP, int cx, int cy) {}
//  virtual int draw_scaled(Fl_Image *img, int X, int Y, int W, int H);
//  virtual void copy_offscreen(int x, int y, int w, int h, Fl_Offscreen pixmap, int srcx, int srcy);
//
//  /** Sets the value of the driver-specific graphics context. */
//  virtual void gc(void*) {}
//  /** Returns the driver-specific graphics context, of NULL if there's none. */
//  virtual void *gc(void) {return NULL;}
//  /** Support for pixmap drawing */
//  virtual uchar **mask_bitmap() { return 0; }
//  /** Support for pixmap drawing */
//  virtual void mask_bitmap(uchar **) {}
//protected:
//  // --- implementation is in src/fl_vertex.cxx which includes src/cfg_gfx/xxx_rect.cxx
//  virtual void transformed_vertex0(COORD_T x, COORD_T y);
//  virtual void fixloop();
//  void global_gc(void);
};

#endif // FL_PICO_GRAPHICS_DRIVER_H

//
// End of "$Id: Fl_Pico_Graphics_Driver.H 11017 2016-01-20 21:40:12Z matt $".
//
