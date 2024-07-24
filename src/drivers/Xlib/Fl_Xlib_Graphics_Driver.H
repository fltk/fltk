//
// Definition of class Fl_Xlib_Graphics_Driver for the Fast Light Tool Kit (FLTK).
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
 \file Fl_Xlib_Graphics_Driver.H
 \brief Definition of X11 Xlib graphics driver.
 */

#ifndef FL_XLIB_GRAPHICS_DRIVER_H
#define FL_XLIB_GRAPHICS_DRIVER_H

#include <config.h>
#include <FL/Fl_Graphics_Driver.H>
#include <FL/platform.H>

#if HAVE_X11_XREGION_H
#   include <X11/Xregion.h>
#else // if the X11/Xregion.h header is not available, we assume this is the layout of an X11 Region:
typedef struct {
  short x1, x2, y1, y2;
} BOX;
struct _XRegion {
  long size;
  long numRects;
  BOX *rects;
  BOX extents;
};
#endif // HAVE_X11_XREGION_H

#if USE_PANGO
#include <pango/pango.h>
#endif

#define FL_XLIB_GRAPHICS_TRANSLATION_STACK_SIZE (20)

/**
 \brief The Xlib-specific graphics class.

 This class is implemented only on the Xlib platform.
 */
class Fl_Xlib_Graphics_Driver : public Fl_Scalable_Graphics_Driver {
private:
  int offset_x_, offset_y_; // translation between user and graphical coordinates: graphical = user + offset
  unsigned depth_; // depth of translation stack
  int stack_x_[FL_XLIB_GRAPHICS_TRANSLATION_STACK_SIZE]; // translation stack allowing cumulative translations
  int stack_y_[FL_XLIB_GRAPHICS_TRANSLATION_STACK_SIZE];
  void set_current_() FL_OVERRIDE;
  int clip_max_; // +/- x/y coordinate limit (16-bit coordinate space)
  void draw_fixed(Fl_Pixmap *pxm, int XP, int YP, int WP, int HP, int cx, int cy) FL_OVERRIDE;
  void draw_fixed(Fl_Bitmap *pxm, int XP, int YP, int WP, int HP, int cx, int cy) FL_OVERRIDE;
  void draw_fixed(Fl_RGB_Image *rgb, int XP, int YP, int WP, int HP, int cx, int cy) FL_OVERRIDE;
#if USE_PANGO
  double do_width_unscaled_(const char *str, int n);
#endif
protected:
  void draw_image_unscaled(const uchar* buf, int X,int Y,int W,int H, int D=3, int L=0) FL_OVERRIDE;
  void draw_image_unscaled(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=3) FL_OVERRIDE;
  void draw_image_mono_unscaled(const uchar* buf, int X,int Y,int W,int H, int D=1, int L=0) FL_OVERRIDE;
  void draw_image_mono_unscaled(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=1) FL_OVERRIDE;
#if HAVE_XRENDER
  void draw_rgb(Fl_RGB_Image *rgb, int XP, int YP, int WP, int HP, int cx, int cy) FL_OVERRIDE;
  int scale_and_render_pixmap(Fl_Offscreen pixmap, int depth, double scale_x, double scale_y, int XP, int YP, int WP, int HP);
#endif
  int height_unscaled() FL_OVERRIDE;
  int descent_unscaled() FL_OVERRIDE;
  Fl_Region scale_clip(float f) FL_OVERRIDE;
#if USE_XFT
  void drawUCS4(const void *str, int n, int x, int y);
#endif
#if USE_PANGO
  static PangoContext *pctxt_;
  static PangoFontMap *pfmap_;
  static PangoLayout *playout_;
public:
  PangoFontDescription *pango_font_description() FL_OVERRIDE { return pfd_array[font()]; }
private:
  static PangoFontDescription **pfd_array; // one array element for each Fl_Font
  static int pfd_array_length;
  void do_draw(int from_right, const char *str, int n, int x, int y);
  static PangoContext *context();
  static void init_built_in_fonts();
#endif
  static GC gc_;
  uchar *mask_bitmap_;
  uchar **mask_bitmap() FL_OVERRIDE {return &mask_bitmap_;}
  XPoint *short_point;
#if USE_XFT
  static Window draw_window;
  static struct _XftDraw* draw_;
#endif
  void cache(Fl_RGB_Image *img) FL_OVERRIDE;
public:
  Fl_Xlib_Graphics_Driver(void);
  ~Fl_Xlib_Graphics_Driver() FL_OVERRIDE;
  void translate_all(int dx, int dy);
  void untranslate_all();
  void scale(float f) FL_OVERRIDE;
  float scale() {return Fl_Graphics_Driver::scale();}
  int has_feature(driver_feature mask) FL_OVERRIDE { return mask & NATIVE; }
  void *gc() FL_OVERRIDE { return gc_; }
  void gc(void *value) FL_OVERRIDE;
  char can_do_alpha_blending() FL_OVERRIDE;
#if USE_XFT
  static void destroy_xft_draw(Window id);
#endif
  static int fl_overlay;

  // --- bitmap stuff
  static unsigned long create_bitmask(int w, int h, const uchar *array); // NOT virtual
  void delete_bitmask(fl_uintptr_t bm) FL_OVERRIDE;
  void draw_unscaled(const char* str, int n, int x, int y) FL_OVERRIDE;
  void draw_unscaled(int angle, const char *str, int n, int x, int y) FL_OVERRIDE;
  void rtl_draw_unscaled(const char* str, int n, int x, int y) FL_OVERRIDE;
  void font_unscaled(Fl_Font face, Fl_Fontsize size) FL_OVERRIDE;
  void cache(Fl_Pixmap *img) FL_OVERRIDE;
  void uncache_pixmap(fl_uintptr_t p) FL_OVERRIDE;
  void cache(Fl_Bitmap *img) FL_OVERRIDE;
  void uncache(Fl_RGB_Image *img, fl_uintptr_t &id_, fl_uintptr_t &mask_) FL_OVERRIDE;
  double width_unscaled(const char *str, int n) FL_OVERRIDE;
  double width_unscaled(unsigned int c) FL_OVERRIDE;
  void text_extents_unscaled(const char*, int n, int& dx, int& dy, int& w, int& h) FL_OVERRIDE;
  Fl_Fontsize size_unscaled() FL_OVERRIDE;
  void copy_offscreen(int x, int y, int w, int h, Fl_Offscreen pixmap, int srcx, int srcy) FL_OVERRIDE;
  void add_rectangle_to_region(Fl_Region r, int x, int y, int w, int h) FL_OVERRIDE;
  Fl_Region XRectangleRegion(int x, int y, int w, int h) FL_OVERRIDE;
  void XDestroyRegion(Fl_Region r) FL_OVERRIDE;
protected:
  void transformed_vertex0(float x, float y) FL_OVERRIDE;
  void fixloop() FL_OVERRIDE;
  void focus_rect(int x, int y, int w, int h) FL_OVERRIDE;
  void rect_unscaled(int x, int y, int w, int h) FL_OVERRIDE;
  void rectf_unscaled(int x, int y, int w, int h) FL_OVERRIDE;
  void colored_rectf(int x, int y, int w, int h, uchar r, uchar g, uchar b) FL_OVERRIDE;
  void line_unscaled(int x, int y, int x1, int y1) FL_OVERRIDE;
  void line_unscaled(int x, int y, int x1, int y1, int x2, int y2) FL_OVERRIDE;
  void xyline_unscaled(int x, int y, int x1) FL_OVERRIDE;
  void *change_pen_width(int lwidth) FL_OVERRIDE;
  void reset_pen_width(void *data) FL_OVERRIDE;
  void yxline_unscaled(int x, int y, int y1) FL_OVERRIDE;
  void loop_unscaled(int x0, int y0, int x1, int y1, int x2, int y2) FL_OVERRIDE;
  void loop_unscaled(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) FL_OVERRIDE;
  void polygon_unscaled(int x0, int y0, int x1, int y1, int x2, int y2) FL_OVERRIDE;
  void polygon_unscaled(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) FL_OVERRIDE;

  // Scaling and clipping stuff for internal usage.
  // Currently specific to Fl_Xlib_Graphics_Driver (16-bit coordinate clipping)
  // These methods must not be virtual so they can be inlined.
  // All these methods work with "real" drawing coordinates (after scaling).

  // Clipping limits
  int clip_max() { return clip_max_; }
  int clip_min() { return -clip_max_; }

  /*
    clip_xy() returns a single coordinate value clipped to the 16-bit
    coordinate space.

    This can be used to draw horizontal and vertical lines that can be
    handled by X. Each single coordinate value can be clipped individually
    and the result can be used directly, e.g. in fl_xyline() and fl_yxline().
  */
  int clip_xy(int x) {
    return (x < clip_min() ? clip_min() : (x > clip_max() ? clip_max() : x ));
  }

  // clip an arbitrary line
  int clip_line(int &x1, int &y1, int &x2, int &y2);

  // clip a rectangle
  int clip_rect(int &x, int &y, int &w, int &h);

  // draw a line after clipping (if visible)
  void draw_clipped_line(int x1, int y1, int x2, int y2);

  // --- clipping
  void push_clip(int x, int y, int w, int h) FL_OVERRIDE;
  int clip_box(int x, int y, int w, int h, int &X, int &Y, int &W, int &H) FL_OVERRIDE;
  int not_clipped(int x, int y, int w, int h) FL_OVERRIDE;
  void restore_clip() FL_OVERRIDE;
  void end_points() FL_OVERRIDE;
  void end_line() FL_OVERRIDE;
  void end_loop() FL_OVERRIDE;
  void end_polygon() FL_OVERRIDE;
  void end_complex_polygon() FL_OVERRIDE;
  bool can_fill_non_convex_polygon() FL_OVERRIDE;
  void gap() FL_OVERRIDE;
  void ellipse_unscaled(double xt, double yt, double rx, double ry) FL_OVERRIDE;
  void arc_unscaled(int x, int y, int w, int h, double a1, double a2) FL_OVERRIDE;
  void pie_unscaled(int x, int y, int w, int h, double a1, double a2) FL_OVERRIDE;
  void line_style_unscaled(int style, int width, char* dashes) FL_OVERRIDE;
  void color(Fl_Color c) FL_OVERRIDE;
  void set_color(Fl_Color i, unsigned int c) FL_OVERRIDE;
  void free_color(Fl_Color i, int overlay) FL_OVERRIDE;
  Fl_Color color() FL_OVERRIDE { return color_; }
  void color(uchar r, uchar g, uchar b) FL_OVERRIDE;
  float scale_font_for_PostScript(Fl_Font_Descriptor *desc, int s) FL_OVERRIDE;
  const char* get_font_name(Fl_Font fnum, int* ap) FL_OVERRIDE;
  int get_font_sizes(Fl_Font fnum, int*& sizep) FL_OVERRIDE;
#if !USE_XFT
  unsigned font_desc_size() FL_OVERRIDE;
  float scale_bitmap_for_PostScript() FL_OVERRIDE;
#endif
  const char *font_name(int num) FL_OVERRIDE;
  void font_name(int num, const char *name) FL_OVERRIDE;
  Fl_Font set_fonts(const char* xstarname) FL_OVERRIDE;
};

#endif // FL_XLIB_GRAPHICS_DRIVER_H
