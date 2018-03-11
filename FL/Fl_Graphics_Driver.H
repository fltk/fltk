//
// "$Id$"
//
// Definition of classes Fl_Graphics_Driver, Fl_Surface_Device, Fl_Display_Device
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

/** \file Fl_Graphics_Driver.H
 \brief declaration of class Fl_Graphics_Driver.
*/

#ifndef FL_GRAPHICS_DRIVER_H
#define FL_GRAPHICS_DRIVER_H

#include <FL/Fl_Device.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Bitmap.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_RGB_Image.H>
#include <stdlib.h>

class Fl_Graphics_Driver;
class Fl_Shared_Image;
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

struct Fl_Fontdesc;

#define FL_REGION_STACK_SIZE 10
#define FL_MATRIX_STACK_SIZE 32
/**
 An abstract class subclassed for each graphics driver FLTK uses.
 Typically, FLTK applications do not use directly objects from this class. Rather, they perform
 drawing operations (e.g., fl_rectf()) that operate on the current drawing surface (see Fl_Surface_Device).
 Drawing operations are functionally presented in \ref drawing and as function lists
 in the \ref fl_drawings and \ref fl_attributes modules.
 
 \p <tt>Fl_Surface_Device::surface()->driver()</tt>
 gives at any time the graphics driver used by all drawing operations. 
 For compatibility with older FLTK versions, the \ref fl_graphics_driver global variable gives the same result.
 Its value changes when
 drawing operations are directed to another drawing surface by Fl_Surface_Device::push_current() /
 Fl_Surface_Device::pop_current() / Fl_Surface_Device::set_current().

 \p The Fl_Graphics_Driver class is of interest if one wants to perform new kinds of drawing operations.
 An example would be to draw to a PDF file. This would involve creating a new Fl_Graphics_Driver derived
 class. This new class should implement all virtual methods of the Fl_Graphics_Driver class
 to support all FLTK drawing functions.
 
 \p The Fl_Graphics_Driver class is essential for developers of the FLTK library.
 Each platform supported by FLTK requires to create a derived class of Fl_Graphics_Driver that
 implements all its virtual member functions according to the platform.
 */
class FL_EXPORT Fl_Graphics_Driver {
  friend class Fl_Surface_Device;
  friend class Fl_Display_Device;
  friend class Fl_Screen_Driver;
  friend class Fl_Window_Driver;
  friend class Fl_Pixmap;
  friend class Fl_Bitmap;
  friend class Fl_RGB_Image;
  friend class Fl_Shared_Image;
  friend void fl_draw_image(const uchar* buf, int X,int Y,int W,int H, int D, int L);
  friend void fl_draw_image_mono(const uchar* buf, int X,int Y,int W,int H, int D, int L);
  friend void fl_draw_image_mono(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D);
  friend void fl_draw_image(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D);
  friend void fl_copy_offscreen(int x, int y, int w, int h, Fl_Offscreen pixmap, int srcx, int srcy);
  friend FL_EXPORT int fl_draw_pixmap(const char*const* cdata, int x, int y, Fl_Color bg);
  friend FL_EXPORT void gl_start();
  friend FL_EXPORT void gl_finish();
  friend FL_EXPORT Fl_Bitmask fl_create_bitmask(int w, int h, const uchar *array);
  friend FL_EXPORT void fl_delete_bitmask(Fl_Bitmask);
private:
  // some platforms may need to reimplement this
  virtual void set_current_();
protected:
  float scale_; // scale between user and graphical coordinates: graphical = user * scale_
  /** Sets the current value of the scaling factor */
  virtual void scale(float f) { scale_ = f; }
public:
  // The following functions create the various graphics drivers that are required
  // for core operations. They must be implemented as members of Fl_Graphics_Driver,
  // but located in the device driver module that is linked to the core library
  /** Instantiate the graphics driver adequate to draw to the platform's display driver.
   Each platform implements this method its own way.
   */
  static Fl_Graphics_Driver *newMainGraphicsDriver();
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
  virtual void global_gc();
  /** Support function for Fl_Pixmap drawing */
  virtual fl_uintptr_t cache(Fl_Pixmap *img, int w, int h, const char *const*array) { return 0; }
  /** Support function for Fl_Bitmap drawing */
  virtual fl_uintptr_t cache(Fl_Bitmap *img, int w, int h, const uchar *array) { return 0; }
  /** Support function for Fl_RGB_Image drawing */
  virtual void uncache(Fl_RGB_Image *img, fl_uintptr_t &id_, fl_uintptr_t &mask_) { }
  // --- implementation is in src/drivers/xxx/Fl_xxx_Graphics_Driver_image.cxx
  /** see fl_draw_image(const uchar* buf, int X,int Y,int W,int H, int D, int L) */
  virtual void draw_image(const uchar* buf, int X,int Y,int W,int H, int D=3, int L=0) {}
  /** see fl_draw_image_mono(const uchar* buf, int X,int Y,int W,int H, int D, int L) */
  virtual void draw_image_mono(const uchar* buf, int X,int Y,int W,int H, int D=1, int L=0) {}
  /** see fl_draw_image(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D) */
  virtual void draw_image(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=3) {}
  /** see fl_draw_image_mono(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D) */
  virtual void draw_image_mono(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=1) {}
  /** \brief Draws an Fl_RGB_Image object using this graphics driver.
   *
   Specifies a bounding box for the image, with the origin (upper left-hand corner) of
   the image offset by the cx and cy arguments.
   */
  virtual void draw(Fl_RGB_Image * rgb,int XP, int YP, int WP, int HP, int cx, int cy) {}
  /** \brief Draws an Fl_Pixmap object using this graphics driver.
   *
   Specifies a bounding box for the image, with the origin (upper left-hand corner) of
   the image offset by the cx and cy arguments.
   */
  virtual void draw(Fl_Pixmap * pxm,int XP, int YP, int WP, int HP, int cx, int cy) {}
  /** \brief Draws an Fl_Bitmap object using this graphics driver.
   *
   Specifies a bounding box for the image, with the origin (upper left-hand corner) of
   the image offset by the cx and cy arguments.
   */
  virtual void draw(Fl_Bitmap *bm, int XP, int YP, int WP, int HP, int cx, int cy) {}
  virtual void draw(Fl_Shared_Image *shared, int X, int Y);
  virtual void copy_offscreen(int x, int y, int w, int h, Fl_Offscreen pixmap, int srcx, int srcy);

  /** Support function for image drawing */
  virtual Fl_Bitmask create_bitmask(int w, int h, const uchar *array) {return 0; }
  /** Support function for image drawing */
  virtual void delete_bitmask(Fl_Bitmask bm) {}
  /** For internal library use only */
  static void change_image_size(Fl_Image *img, int W, int H) {
    img->w(W);
    img->h(H);
  }
  // Support function for image drawing
  virtual void uncache_pixmap(fl_uintptr_t p);
  // accessor functions to protected image members
  int start_image(Fl_Image *img, int XP, int YP, int WP, int HP, int &cx, int &cy,
                int &X, int &Y, int &W, int &H);
  /** Accessor to a private member variable of Fl_RGB_Image */
  static fl_uintptr_t* id(Fl_RGB_Image *rgb) {return &(rgb->id_);}
  /** Accessor to a private member variable of Fl_Pixmap */
  static fl_uintptr_t* id(Fl_Pixmap *pm) {return &(pm->id_);}
  /** Accessor to a private member variable of Fl_Bitmap */
  static fl_uintptr_t* id(Fl_Bitmap *bm) {return &(bm->id_);}
  /** Accessor to a private member variable of Fl_RGB_Image */
  static fl_uintptr_t* mask(Fl_RGB_Image *rgb) {return &(rgb->mask_);}
  /** Accessor to a private member variable of Fl_Pixmap */
  static fl_uintptr_t* mask(Fl_Pixmap *pm) {return &(pm->mask_);}
  /** Accessor to a private member variable of Fl_Pixmap */
  static float* cache_scale(Fl_Pixmap *pm) {return &(pm->cache_scale_);}
  /** Accessor to a private member variable of Fl_Bitmap */
  static float* cache_scale(Fl_Bitmap *bm) {return &(bm->cache_scale_);}
  /** Accessor to a private member variable of Fl_RGB_Image */
  static float* cache_scale(Fl_RGB_Image *rgb) {return &(rgb->cache_scale_);}
  /** Accessor to a private member variable of Fl_Pixmap */
  static Fl_Color* pixmap_bg_color(Fl_Pixmap *pm) {return &(pm->pixmap_bg_color);}
  /** For internal library use only */
  static void draw_empty(Fl_Image* img, int X, int Y) {img->draw_empty(X, Y);}
  /** Accessor to a private member function of Fl_Bitmap */
  static int prepare(Fl_Bitmap *bm, int XP, int YP, int WP, int HP, int &cx, int &cy,
                   int &X, int &Y, int &W, int &H) {
    return bm->prepare(XP,YP,WP,HP,cx,cy,X,Y,W,H);
  }
  /** Accessor to a private member function of Fl_Pixmap */
  static int prepare(Fl_Pixmap *pm, int XP, int YP, int WP, int HP, int &cx, int &cy,
                     int &X, int &Y, int &W, int &H) {
    return pm->prepare(XP,YP,WP,HP,cx,cy,X,Y,W,H);
  }

public:
  Fl_Graphics_Driver();
  virtual ~Fl_Graphics_Driver() {} ///< Destructor
  static Fl_Graphics_Driver &default_driver();
  /** Current scale factor between FLTK and drawing units: drawing = FLTK * scale() */
  float scale() { return scale_; }
  /** Return whether the graphics driver can do alpha blending */
  virtual char can_do_alpha_blending() { return 0; }
  // --- implementation is in src/fl_rect.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_rect.cxx
  /** see fl_point() */
  virtual void point(int x, int y) {}
  /** see fl_rect() */
  virtual void rect(int x, int y, int w, int h) {}
  virtual void focus_rect(int x, int y, int w, int h);
  /** see fl_rectf() */
  virtual void rectf(int x, int y, int w, int h) {}
  /** see fl_line(int, int, int, int) */
  virtual void line(int x, int y, int x1, int y1) {}
  /** see fl_line(int, int, int, int, int, int) */
  virtual void line(int x, int y, int x1, int y1, int x2, int y2) {}
  /** see fl_xyline(int, int, int) */
  virtual void xyline(int x, int y, int x1) {}
  /** see fl_xyline(int, int, int, int) */
  virtual void xyline(int x, int y, int x1, int y2) {}
  /** see fl_xyline(int, int, int, int, int) */
  virtual void xyline(int x, int y, int x1, int y2, int x3) {}
  /** see fl_yxline(int, int, int) */
  virtual void yxline(int x, int y, int y1) {}
  /** see fl_yxline(int, int, int, int) */
  virtual void yxline(int x, int y, int y1, int x2) {}
  /** see fl_yxline(int, int, int, int, int) */
  virtual void yxline(int x, int y, int y1, int x2, int y3) {}
  /** see fl_loop(int, int, int, int, int, int) */
  virtual void loop(int x0, int y0, int x1, int y1, int x2, int y2) {}
  /** see fl_loop(int, int, int, int, int, int, int, int) */
  virtual void loop(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) {}
  /** see fl_polygon(int, int, int, int, int, int) */
  virtual void polygon(int x0, int y0, int x1, int y1, int x2, int y2) {}
  /** see fl_polygon(int, int, int, int, int, int, int, int) */
  virtual void polygon(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) {}
  // --- clipping
  /** see fl_push_clip() */
  virtual void push_clip(int x, int y, int w, int h) {}
  /** see fl_clip_box() */
  virtual int clip_box(int x, int y, int w, int h, int &X, int &Y, int &W, int &H) {return 0;}
  /** see fl_not_clipped() */
  virtual int not_clipped(int x, int y, int w, int h) {return 1;}
  /** see fl_push_no_clip() */
  virtual void push_no_clip() {}
  /** see fl_pop_clip() */
  virtual void pop_clip() {}
  virtual Fl_Region clip_region();              // has default implementation
  virtual void clip_region(Fl_Region r);        // has default implementation
  virtual void restore_clip();
  // --- implementation is in src/fl_vertex.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_vertex.cxx
  virtual void push_matrix();
  virtual void pop_matrix();
  virtual void mult_matrix(double a, double b, double c, double d, double x, double y);
  virtual void rotate(double d);
  virtual void translate(double x,double y);
  virtual void begin_points();
  virtual void begin_line();
  virtual void begin_loop();
  virtual void begin_polygon();
  /** see fl_begin_complex_polygon() */
  virtual void begin_complex_polygon() {}
  virtual double transform_x(double x, double y);
  virtual double transform_y(double x, double y);
  virtual double transform_dx(double x, double y);
  virtual double transform_dy(double x, double y);
  /** see fl_transformed_vertex() */
  virtual void transformed_vertex(double xf, double yf) {}
  /** see fl_vertex() */
  virtual void vertex(double x, double y) {}
  /** see fl_end_points() */
  virtual void end_points() {}
  /** see fl_end_line() */
  virtual void end_line() {}
  /** see fl_end_loop() */
  virtual void end_loop() {}
  /** see fl_end_polygon() */
  virtual void end_polygon() {}
  /** see fl_end_complex_polygon() */
  virtual void end_complex_polygon() {}
  /** see fl_gap() */
  virtual void gap() {}
  /** see fl_circle() */
  virtual void circle(double x, double y, double r) {}
  // --- implementation is in src/fl_arc.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_arc.cxx if needed
  virtual void arc(double x, double y, double r, double start, double end);
  // --- implementation is in src/fl_arci.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_arci.cxx
  /** see fl_arc(int x, int y, int w, int h, double a1, double a2) */
  virtual void arc(int x, int y, int w, int h, double a1, double a2) {}
  /** see fl_pie() */
  virtual void pie(int x, int y, int w, int h, double a1, double a2) {}
  // --- implementation is in src/fl_curve.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_curve.cxx if needed
  virtual void curve(double X0, double Y0, double X1, double Y1, double X2, double Y2, double X3, double Y3);
  // --- implementation is in src/fl_line_style.cxx which includes src/cfg_gfx/xxx_line_style.cxx
  /** see fl_line_style() */
  virtual void line_style(int style, int width=0, char* dashes=0) {}
  // --- implementation is in src/fl_color.cxx which includes src/cfg_gfx/xxx_color.cxx
  /** see fl_color(Fl_Color) */
  virtual void color(Fl_Color c) { color_ = c; }
  virtual void set_color(Fl_Color i, unsigned int c);
  virtual void free_color(Fl_Color i, int overlay);
  /** see fl_color(void) */
  virtual Fl_Color color() { return color_; }
  /** see fl_color(uchar, uchar, uchar) */
  virtual void color(uchar r, uchar g, uchar b) {}
  /** see fl_draw(const char *str, int n, int x, int y) */
  virtual void draw(const char *str, int n, int x, int y) {}
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
  inline Fl_Font_Descriptor *font_descriptor() { return font_descriptor_;}
  /** Set the current Fl_Font_Descriptor */
  virtual void font_descriptor(Fl_Font_Descriptor *d) { font_descriptor_ = d;}
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
  /** Support for Fl::get_font_name() */
  virtual const char* get_font_name(Fl_Font fnum, int* ap) {return NULL;}
  /** Support for Fl::get_font_sizes() */
  virtual int get_font_sizes(Fl_Font fnum, int*& sizep) {return 0;}
  /** Support for Fl::set_fonts() */
  virtual Fl_Font set_fonts(const char *name) {return 0;}
  /** Some platforms may need to implement this to support fonts */
  virtual Fl_Fontdesc* calc_fl_fonts(void) {return NULL;}
  /** Support for Fl::set_font() */
  virtual unsigned font_desc_size();
  /** Support for Fl::get_font() */
  virtual const char *font_name(int num) {return NULL;}
  /** Support for Fl::set_font() */
  virtual void font_name(int num, const char *name) {}
  /** Support function for Fl_Shared_Image drawing */
  virtual int draw_scaled(Fl_Image *img, int X, int Y, int W, int H);
  /** Support function for fl_overlay_rect() and scaled GUI.
   Defaut implementation may be enough */
  virtual bool overlay_rect_unscaled();
  /** Support function for fl_overlay_rect() and scaled GUI.
   Defaut implementation may be enough */
  virtual void overlay_rect(int x, int y, int w , int h) { loop(x, y, x+w-1, y, x+w-1, y+h-1, x, y+h-1); }
};

#ifndef FL_DOXYGEN

/* This class is not part of FLTK's public API.
 Platforms usually define a derived class called Fl_XXX_Font_Descriptor
 containing extra platform-specific data/functions.
 This is a class for an actual system font, with junk to
 help choose it and info on character sizes.  Each Fl_Fontdesc has a
 linked list of these.  These are created the first time each system
 font/size combination is used.
 */
class Fl_Font_Descriptor {
public:
  /** linked list for this Fl_Fontdesc */
  Fl_Font_Descriptor *next;
  Fl_Fontsize size; /**< font size */
  Fl_Font_Descriptor(const char* fontname, Fl_Fontsize size);
  FL_EXPORT ~Fl_Font_Descriptor() {}
  short ascent, descent, q_width;
  unsigned int listbase;// base of display list, 0 = none
};

// This struct is not part of FLTK's public API.
struct Fl_Fontdesc {
  const char *name;
  char fontname[128];  // "Pretty" font name
  Fl_Font_Descriptor *first;  // linked list of sizes of this style
};

/* Abstract class Fl_Scalable_Graphics_Driver is platform-independent.
 It supports the scaling of all graphics coordinates by a
 float factor helpful to support HiDPI displays.
 This class does :
 - compute scaled coordinates
 - scale the cached offscreen of image objects
 - scale the pixel arrays used when performing direct image draws
 - call the member functions of a platform-specific,
 Fl_Scalable_Graphics_Driver-derived class that do the drawings with adequately 
 scaled coordinates. The member functions are named with the _unscaled suffix.
 - scale and unscale the clipping region.
 
 This class is presently used by the X11 and Windows platforms to support HiDPI displays.
 In the future, it may also be used by other platforms.
 */
class FL_EXPORT Fl_Scalable_Graphics_Driver : public Fl_Graphics_Driver {
public:
  Fl_Scalable_Graphics_Driver();
protected:
  int line_width_;
  void cache_size(Fl_Image *img, int &width, int &height);
  virtual Fl_Region scale_clip(float f) { return 0; }
  void unscale_clip(Fl_Region r);
  virtual void draw(Fl_Pixmap *pxm, int XP, int YP, int WP, int HP, int cx, int cy);
  virtual void draw_unscaled(Fl_Pixmap *pxm, float s, int XP, int YP, int WP, int HP, int cx, int cy) {}
  virtual void draw(Fl_Bitmap *bm, int XP, int YP, int WP, int HP, int cx, int cy);
  virtual void draw_unscaled(Fl_Bitmap *bm, float s, int XP, int YP, int WP, int HP, int cx, int cy) {}
  virtual void draw(Fl_RGB_Image *img, int XP, int YP, int WP, int HP, int cx, int cy);
  virtual void draw_unscaled(Fl_RGB_Image *img, float s, int XP, int YP, int WP, int HP, int cx, int cy) {}
  virtual void draw(Fl_Shared_Image *shared, int X, int Y);
  virtual void point(int x, int y);
  virtual void point_unscaled(float x, float y) {}
  virtual void rect(int x, int y, int w, int h);
  virtual void rect_unscaled(float x, float y, float w, float h) {}
  virtual void rectf(int x, int y, int w, int h);
  virtual void rectf_unscaled(float x, float y, float w, float h) {}
  virtual void line(int x, int y, int x1, int y1);
  virtual void line_unscaled(float x, float y, float x1, float y1) {}
  virtual void line(int x, int y, int x1, int y1, int x2, int y2);
  virtual void line_unscaled(float x, float y, float x1, float y1, float x2, float y2) {}
  virtual void xyline(int x, int y, int x1);
  virtual void xyline(int x, int y, int x1, int y2);
  virtual void xyline(int x, int y, int x1, int y2, int x3);
  virtual void xyline_unscaled(float x, float y, float x1) {}
  virtual void yxline(int x, int y, int y1);
  virtual void yxline(int x, int y, int y1, int x2);
  virtual void yxline(int x, int y, int y1, int x2, int y3);
  virtual void yxline_unscaled(float x, float y, float y1) {}
  virtual void loop(int x0, int y0, int x1, int y1, int x2, int y2);
  virtual void loop_unscaled(float x0, float y0, float x1, float y1, float x2, float y2) {}
  virtual void loop(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3);
  virtual void loop_unscaled(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3) {}
  virtual void polygon(int x0, int y0, int x1, int y1, int x2, int y2);
  virtual void polygon_unscaled(float x0, float y0, float x1, float y1, float x2, float y2) {}
  virtual void polygon(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3);
  virtual void polygon_unscaled(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3) {}
  virtual void circle(double x, double y, double r);
  virtual void ellipse_unscaled(double xt, double yt, double rx, double ry) {}
  virtual void font(Fl_Font face, Fl_Fontsize size);
  virtual void font_unscaled(Fl_Font face, Fl_Fontsize size) {}
  virtual double width(const char *str, int n);
  virtual double width(unsigned int c);
  virtual double width_unscaled(const char *str, int n) { return 0.0; }
  virtual double width_unscaled(unsigned int c) { return 0.0; }
  virtual Fl_Fontsize size();
  virtual Fl_Fontsize size_unscaled() { return 0; }
  virtual void text_extents(const char *str, int n, int &dx, int &dy, int &w, int &h);
  virtual void text_extents_unscaled(const char *str, int n, int &dx, int &dy, int &w, int &h) {}
  virtual int height();
  virtual int descent();
  virtual int height_unscaled() { return 0; }
  virtual int descent_unscaled() { return 0; }
  virtual void draw(const char *str, int n, int x, int y);
  virtual void draw_unscaled(const char *str, int n, int x, int y) {}
  virtual void draw(int angle, const char *str, int n, int x, int y);
  virtual void draw_unscaled(int angle, const char *str, int n, int x, int y) {}
  virtual void rtl_draw(const char* str, int n, int x, int y);
  virtual void rtl_draw_unscaled(const char* str, int n, int x, int y) {}
  virtual void arc(int x, int y, int w, int h, double a1, double a2);
  virtual void arc_unscaled(float x, float y, float w, float h, double a1, double a2) {}
  virtual void pie(int x, int y, int w, int h, double a1, double a2);
  virtual void pie_unscaled(float x, float y, float w, float h, double a1, double a2) {}
  virtual void line_style(int style, int width=0, char* dashes=0);
  virtual void line_style_unscaled(int style, float width, char* dashes) {}
  void draw_image_rescale(void *buf, Fl_Draw_Image_Cb cb, int X, int Y, int W, int H, int D, int L, bool mono, float s);
  virtual void draw_image_unscaled(const uchar* buf, int X,int Y,int W,int H, int D=3, int L=0) {}
  virtual void draw_image_unscaled(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=3) {}
  void draw_image(const uchar* buf, int X,int Y,int W,int H, int D=3, int L=0);
  void draw_image(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=3);
  virtual void draw_image_mono_unscaled(const uchar* buf, int x, int y, int w, int h, int d, int l) {}
  void draw_image_mono(const uchar* buf, int X,int Y,int W,int H, int D=1, int L=0);
  virtual void draw_image_mono_unscaled(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=1) {}
  void draw_image_mono(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=1);

  void transformed_vertex(double xf, double yf);
  virtual void transformed_vertex0(float x, float y) {}
  void vertex(double x, double y);
};
#endif // FL_DOXYGEN

#endif // FL_GRAPHICS_DRIVER_H

//
// End of "$Id$".
//
