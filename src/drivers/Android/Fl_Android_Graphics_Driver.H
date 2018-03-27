//
// "$Id$"
//
// Definition of classes Fl_Graphics_Driver, Fl_Surface_Device, Fl_Display_Device
// for the Fast Light Tool Kit (FLTK).
//
// Copyright 2018 by Bill Spitzak and others.
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
 \file Fl_Android_Graphics_Driver.H
 \brief Definition of Android graphics driver.
 */

#ifndef FL_ANDROID_GRAPHICS_DRIVER_H
#define FL_ANDROID_GRAPHICS_DRIVER_H

#include <FL/Fl_Graphics_Driver.H>
#include "Fl_Android_Graphics_Clipping.H"
#include "Fl_Android_Graphics_Font.H"
#include <limits.h>


class Fl_Android_Window_Driver;
class Fl_Android_Bytemap;

/**
  \brief The Windows-specific graphics driver class.

  This class is implemented only on the Windows platform.
*/
class FL_EXPORT Fl_Android_Graphics_Driver : public Fl_Graphics_Driver
{
  // easy access to the super class
  typedef Fl_Graphics_Driver super;

protected:
  class Vertex; // see below

  // --- this is a copy of Fl_Graphics_Driver ----------------------------------
  // - use this to find unimplementet calls in the derived driver
  // - excluded by #if/#endif means that we have not implemneted this yet
  // - methods marked with // super: use the implemnetation of the super class
  // - virtual ... override functions are implemented for Android
#if 0
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
  virtual fl_uintptr_t cache(Fl_Pixmap *img) { return 0; }
#endif
  /** Support function for Fl_Bitmap drawing */
  virtual fl_uintptr_t cache(Fl_Bitmap *img) override;
#if 0
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
#endif
  /** \brief Draws an Fl_Bitmap object using this graphics driver.
   *
   Specifies a bounding box for the image, with the origin (upper left-hand corner) of
   the image offset by the cx and cy arguments.
   */
  virtual void draw(Fl_Bitmap *bm, int XP, int YP, int WP, int HP, int cx, int cy) override;
#if 0
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
#endif
public:
  /** Constructor, C++11 initialises member variables in-line */
  Fl_Android_Graphics_Driver();
  /** destructor */
  virtual ~Fl_Android_Graphics_Driver() override;
  /** Return whether the graphics driver can do alpha blending */
  virtual char can_do_alpha_blending() override { return 0; }
  // --- implementation is in src/fl_rect.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_rect.cxx
  /** see fl_point() */
  virtual void point(int x, int y) override;
  /** see fl_rect() */
  virtual void rect(int x, int y, int w, int h) override;
  // super: virtual void focus_rect(int x, int y, int w, int h);
  /** see fl_rectf() */
  virtual void rectf(int x, int y, int w, int h) override;
  /** see fl_line(int, int, int, int) */
  virtual void line(int x, int y, int x1, int y1) override;
  /** see fl_line(int, int, int, int, int, int) */
  virtual void line(int x, int y, int x1, int y1, int x2, int y2) override;
  /** see fl_xyline(int, int, int) */
  virtual void xyline(int x, int y, int x1) override;
  /** see fl_xyline(int, int, int, int) */
  virtual void xyline(int x, int y, int x1, int y2) override;
  /** see fl_xyline(int, int, int, int, int) */
  virtual void xyline(int x, int y, int x1, int y2, int x3) override;
  /** see fl_yxline(int, int, int) */
  virtual void yxline(int x, int y, int y1) override;
  /** see fl_yxline(int, int, int, int) */
  virtual void yxline(int x, int y, int y1, int x2) override;
  /** see fl_yxline(int, int, int, int, int) */
  virtual void yxline(int x, int y, int y1, int x2, int y3) override;
  /** see fl_loop(int, int, int, int, int, int) */
  virtual void loop(int x0, int y0, int x1, int y1, int x2, int y2) override;
  /** see fl_loop(int, int, int, int, int, int, int, int) */
  virtual void loop(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) override;
  /** see fl_polygon(int, int, int, int, int, int) */
  virtual void polygon(int x0, int y0, int x1, int y1, int x2, int y2) override;
  /** see fl_polygon(int, int, int, int, int, int, int, int) */
  virtual void polygon(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) override;
  // --- clipping
  /** see fl_push_clip() */
  virtual void push_clip(int x, int y, int w, int h) override;
  /** see fl_clip_box() */
  virtual int clip_box(int x, int y, int w, int h, int &X, int &Y, int &W, int &H) override;
  /** see fl_not_clipped() */
  virtual int not_clipped(int x, int y, int w, int h) override;
  /** see fl_push_no_clip() */
  virtual void push_no_clip() override;
  /** see fl_pop_clip() */
  virtual void pop_clip() override;
  virtual Fl_Region clip_region() override;
  virtual void clip_region(Fl_Region r) override;
  virtual void restore_clip() override;
  // --- implementation is in src/fl_vertex.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_vertex.cxx
  // super: virtual void push_matrix();
  // super: virtual void pop_matrix();
  // super: virtual void mult_matrix(double a, double b, double c, double d, double x, double y);
  // super: virtual void rotate(double d);
  // super: virtual void translate(double x,double y);
  virtual void begin_points() override;
  virtual void begin_line() override;
  virtual void begin_loop() override;
  virtual void begin_polygon() override;
  virtual void begin_complex_polygon() override;
  // super: virtual double transform_x(double x, double y);
  // super: virtual double transform_y(double x, double y);
  // super: virtual double transform_dx(double x, double y);
  // super: virtual double transform_dy(double x, double y);
  /** see fl_transformed_vertex() */
  virtual void transformed_vertex(double xf, double yf) override;
  /** see fl_vertex() */
  virtual void vertex(double x, double y) override;
  /** see fl_end_points() */
  virtual void end_points() override;
  /** see fl_end_line() */
  virtual void end_line() override;
  /** see fl_end_loop() */
  virtual void end_loop() override;
  /** see fl_end_polygon() */
  virtual void end_polygon() override;
  /** see fl_end_complex_polygon() */
  virtual void end_complex_polygon() override;
  /** see fl_gap() */
  virtual void gap() override;
  /** see fl_circle() */
  virtual void circle(double x, double y, double r) override;
#if 0
  // TODO: arc()
  // --- implementation is in src/fl_arc.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_arc.cxx if needed
  virtual void arc(double x, double y, double r, double start, double end);
#endif
  // --- implementation is in src/fl_arci.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_arci.cxx
  /** see fl_arc(int x, int y, int w, int h, double a1, double a2) */
  virtual void arc(int x, int y, int w, int h, double a1, double a2) override;
  /** see fl_pie() */
  virtual void pie(int x, int y, int w, int h, double a1, double a2) override;
#if 0
  // TODO: curve()
  // --- implementation is in src/fl_curve.cxx which includes src/drivers/xxx/Fl_xxx_Graphics_Driver_curve.cxx if needed
  virtual void curve(double X0, double Y0, double X1, double Y1, double X2, double Y2, double X3, double Y3);
#endif
  // --- implementation is in src/fl_line_style.cxx which includes src/cfg_gfx/xxx_line_style.cxx
  // TODO: line_style()
  /** see fl_line_style() */
  virtual void line_style(int style, int width=0, char* dashes=0) override;
  // --- implementation is in src/fl_color.cxx which includes src/cfg_gfx/xxx_color.cxx
  /** see fl_color(Fl_Color) */
  // super: virtual void color(Fl_Color c);
#if 0
  virtual void set_color(Fl_Color i, unsigned int c);
  virtual void free_color(Fl_Color i, int overlay);
  /** see fl_color(void) */
  virtual Fl_Color color() { return color_; }
  /** see fl_color(uchar, uchar, uchar) */
  virtual void color(uchar r, uchar g, uchar b) {}
#endif
  /** see fl_draw(const char *str, int n, int x, int y) */
  virtual void draw(const char *str, int n, int x, int y) override;
#if 0
  /** Draw the first \p n bytes of the string \p str starting at position \p x , \p y */
  virtual void draw(const char *str, int n, float x, float y) { draw(str, n, (int)(x+0.5), (int)(y+0.5));}
  /** see fl_draw(int angle, const char *str, int n, int x, int y) */
  virtual void draw(int angle, const char *str, int n, int x, int y) { draw(str, n, x, y); }
  /** see fl_rtl_draw(const char *str, int n, int x, int y) */
  virtual void rtl_draw(const char *str, int n, int x, int y) { draw(str, n, x, y); }
  /** Returns non-zero if the graphics driver possesses the \p feature */
  virtual int has_feature(driver_feature feature) { return 0; }
#endif
  /** see fl_font(Fl_Font, Fl_Fontsize) */
  virtual void font(Fl_Font face, Fl_Fontsize fsize) override;
  /** see fl_font(void) */
  // super: virtual Fl_Font font() {return font_; }
  /** Return the current font size */
  virtual Fl_Fontsize size() override;
  /** Compute the width of the first \p n bytes of the string \p str if drawn with current font */
  virtual double width(const char *str, int n) override;
  /** Compute the width of Unicode character \p c if drawn with current font */
  virtual double width(unsigned int c) override;
  virtual void text_extents(const char*, int n, int& dx, int& dy, int& w, int& h) override;
  /** Return the current line height */
  virtual int height() override;
  /** Return the current line descent */
  virtual int descent() override;
  /** Return the current Fl_Font_Descriptor */
  // super: inline Fl_Font_Descriptor *font_descriptor() { return font_descriptor_;}
  /** Set the current Fl_Font_Descriptor */
  // super: virtual void font_descriptor(Fl_Font_Descriptor *d) { font_descriptor_ = d;}
#if 0
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
#endif
  /** Support for Fl::get_font_name() */
  virtual const char* get_font_name(Fl_Font fnum, int* ap) override;
  /** Support for Fl::get_font_sizes() */
  virtual int get_font_sizes(Fl_Font fnum, int*& sizep) override;
  /** Support for Fl::set_fonts() */
  virtual Fl_Font set_fonts(const char *name) override;
#if 0
  /** Some platforms may need to implement this to support fonts */
  virtual Fl_Fontdesc* calc_fl_fonts(void) {return NULL;}
  /** Support for Fl::set_font() */
  virtual unsigned font_desc_size();
#endif
  /** Support for Fl::get_font() */
  virtual const char *font_name(int num) override;
  /** Support for Fl::set_font() */
  virtual void font_name(int num, const char *name) override;
#if 0
  // Draws an Fl_Image scaled to width W & height H
  virtual int draw_scaled(Fl_Image *img, int X, int Y, int W, int H);
  /** Support function for fl_overlay_rect() and scaled GUI.
   Defaut implementation may be enough */
  virtual bool overlay_rect_unscaled();
  /** Support function for fl_overlay_rect() and scaled GUI.
   Defaut implementation may be enough */
  virtual void overlay_rect(int x, int y, int w , int h) { loop(x, y, x+w-1, y, x+w-1, y+h-1, x, y+h-1); }
#endif
  // --- end of original Fl_Graphics_Driver header -----------------------------

  // --- start of Android additions --------------------------------------------
  // start drawing with this driver into the given window
  // TODO: how is this different to Fl_Graphics_Driver::set_current_() above
  void make_current(Fl_Window*);

protected:
  static uint16_t make565(Fl_Color crgba);
  void rectf_unclipped(int x, int y, int w, int h);
  void xyline_unclipped(int x, int y, int x1);
  void yxline_unclipped(int x, int y, int y1);
  void end_polygon(int begin, int end);
  void ellipse(double xt, double yt, double rx, double ry);
  void render_bytemap(int x, int y, Fl_Android_Bytemap *bm, Fl_Rect_Region &r);
  int render_letter(int xx, int yy, uint32_t c, Fl_Rect_Region &r);

  // pointer into the screen buffer at the top left corner of the current window
  uint16_t *pBits = nullptr;

  // advance to next line in screen buffer
  int32_t pStride = 0;

  // TODO: current line style, temp kludge to make focus rect work.
  int pLineStyle = 0;

  // Clipping region of the current window in window coordinates (see: pStride and pBits)
  Fl_Rect_Region pWindowRegion;

  // clipping region of the window minus overlapping other windows
  Fl_Complex_Region pDesktopWindowRegion;

  // Final clipping region for all graphics calls to this class.
  Fl_Complex_Region pClippingRegion;

  // store vertices for begin_.../end_... drawing
  class Vertex {
  public:
    void set(float x, float y, bool gap = false) { pX = x; pY = y; pIsGap = gap; }
    float pX, pY;
    bool pIsGap;
  };

  void begin_vertices();
  void add_vertex(float x, float y, bool gap=false);
  int pnVertex = 0, pNVertex = 0, pVertexGapStart = 0;
  Vertex *pVertex = nullptr;
};


#if 0
/**
  The graphics driver used when printing on Android.
*/
class FL_EXPORT Fl_Android_Printer_Graphics_Driver : public Fl_Android_Graphics_Driver {

#if 0

private:
  typedef BOOL (WINAPI* transparent_f_type) (HDC,int,int,int,int,HDC,int,int,int,int,UINT);
  transparent_f_type TransparentBlt();
public:
  virtual int has_feature(driver_feature mask) { return mask & (NATIVE | PRINTER); }
  void draw_unscaled(Fl_Pixmap *pxm, float s, int XP, int YP, int WP, int HP, int cx, int cy);
  void draw_unscaled(Fl_Bitmap *bm, float s, int XP, int YP, int WP, int HP, int cx, int cy);
  int draw_scaled(Fl_Image *img, int XP, int YP, int WP, int HP);

#endif

};
#endif


#endif // FL_ANDROID_GRAPHICS_DRIVER_H

//
// End of "$Id$".
//
