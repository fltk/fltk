//
// Definition of classes  Fl_Graphics_Driver, Fl_Surface_Device, Fl_Display_Device
// for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2023 by Bill Spitzak and others.
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
 \file Fl_GDI_Graphics_Driver.H
 \brief Definition of Windows GDI graphics driver.
 */

#ifndef FL_GDI_GRAPHICS_DRIVER_H
#define FL_GDI_GRAPHICS_DRIVER_H

#include <FL/Fl_Graphics_Driver.H>
#include <windows.h>
#include <stdlib.h>
#include <config.h>

#if USE_GDIPLUS
#  if defined(_MSC_VER)
#    include <objidl.h>
#  else
#    include <wtypes.h> // for PROPID needed with gcc 4.9.0 but not with 4.9.3
#  endif
#  include <gdiplus.h>
#endif

/**
  \brief The Windows-specific graphics driver class.

  This class is implemented only on the Windows platform.
*/
class Fl_GDI_Graphics_Driver : public Fl_Scalable_Graphics_Driver {
private:
  BOOL alpha_blend_(int x, int y, int w, int h, HDC src_gc, int srcx, int srcy, int srcw, int srch);
  int depth; // to support translation
  POINT *origins; // to support translation
  void set_current_() FL_OVERRIDE;
  void draw_fixed(Fl_Pixmap *pxm, int XP, int YP, int WP, int HP, int cx, int cy) FL_OVERRIDE;
  void draw_fixed(Fl_Bitmap *pxm, int XP, int YP, int WP, int HP, int cx, int cy) FL_OVERRIDE;
  void make_unused_color_(unsigned char &r, unsigned char &g, unsigned char &b, int color_count, void **data) FL_OVERRIDE;
protected:
  void draw_fixed(Fl_RGB_Image *rgb, int XP, int YP, int WP, int HP, int cx, int cy) FL_OVERRIDE;
  void cache(Fl_RGB_Image *rgb) FL_OVERRIDE;
  HDC gc_;
  int numcount;
  int counts[20];
  uchar *mask_bitmap_;
  uchar **mask_bitmap() FL_OVERRIDE {return &mask_bitmap_;}
  POINT *long_point;
  bool is_solid_;
  int style_;
public:
  Fl_GDI_Graphics_Driver();
  ~Fl_GDI_Graphics_Driver() FL_OVERRIDE;
  int has_feature(driver_feature mask) FL_OVERRIDE { return mask & NATIVE; }
  char can_do_alpha_blending() FL_OVERRIDE;
  void gc(void *ctxt) FL_OVERRIDE { gc_ = (HDC)ctxt; global_gc(); }
  void *gc() FL_OVERRIDE {return gc_;}

  // --- bitmap stuff
  static HBITMAP create_bitmask(int w, int h, const uchar *array); // NOT virtual
  static HBITMAP calc_HBITMAP_mask(Fl_RGB_Image *mask);
  void delete_bitmask(fl_uintptr_t bm) FL_OVERRIDE;
  HBITMAP create_alphamask(int w, int h, int d, int ld, const uchar *array);
  void draw_unscaled(const char* str, int n, int x, int y) FL_OVERRIDE;
  void draw_unscaled(int angle, const char *str, int n, int x, int y) FL_OVERRIDE;
  void rtl_draw_unscaled(const char* str, int n, int x, int y) FL_OVERRIDE;
  void font_unscaled(Fl_Font face, Fl_Fontsize size) FL_OVERRIDE;
  void draw_rgb(Fl_RGB_Image *img, int XP, int YP, int WP, int HP, int cx, int cy) FL_OVERRIDE;
  void draw_image_unscaled(const uchar* buf, int X,int Y,int W,int H, int D=3, int L=0) FL_OVERRIDE;
  void draw_image_unscaled(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=3) FL_OVERRIDE;
  void draw_image_mono_unscaled(const uchar* buf, int X,int Y,int W,int H, int D=1, int L=0) FL_OVERRIDE;
  void draw_image_mono_unscaled(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=1) FL_OVERRIDE;
  void cache(Fl_Pixmap *img) FL_OVERRIDE;
  void uncache_pixmap(fl_uintptr_t p) FL_OVERRIDE;
  void cache(Fl_Bitmap *img) FL_OVERRIDE;
  void uncache(Fl_RGB_Image *img, fl_uintptr_t &id_, fl_uintptr_t &mask_) FL_OVERRIDE;
  double width_unscaled(const char *str, int n) FL_OVERRIDE;
  double width_unscaled(unsigned int c) FL_OVERRIDE;
  void text_extents_unscaled(const char*, int n, int& dx, int& dy, int& w, int& h) FL_OVERRIDE;
  int height_unscaled() FL_OVERRIDE;
  int descent_unscaled() FL_OVERRIDE;
  Fl_Fontsize size_unscaled() FL_OVERRIDE;
#if ! defined(FL_DOXYGEN)
  void copy_offscreen_with_alpha(int x,int y,int w,int h,HBITMAP bitmap,int srcx,int srcy);
#endif
  void copy_offscreen(int x, int y, int w, int h, Fl_Offscreen pixmap, int srcx, int srcy) FL_OVERRIDE;
  void add_rectangle_to_region(Fl_Region r, int x, int y, int w, int h) FL_OVERRIDE;
  Fl_Region XRectangleRegion(int x, int y, int w, int h) FL_OVERRIDE;
  void XDestroyRegion(Fl_Region r) FL_OVERRIDE;
  void translate_all(int x, int y);
  void untranslate_all(void);
  static HRGN scale_region(HRGN r, float f, Fl_GDI_Graphics_Driver *dr);
  void scale(float f) FL_OVERRIDE;
  float scale() {return Fl_Graphics_Driver::scale();}
protected:
  void transformed_vertex0(float x, float y) FL_OVERRIDE;
  void fixloop() FL_OVERRIDE;
  void point(int x, int y) FL_OVERRIDE;
  void focus_rect(int x, int y, int w, int h) FL_OVERRIDE;
  void rect_unscaled(int x, int y, int w, int h) FL_OVERRIDE;
  void rectf_unscaled(int x, int y, int w, int h) FL_OVERRIDE;
#if USE_COLORMAP
  void colored_rectf(int x, int y, int w, int h, uchar r, uchar g, uchar b) FL_OVERRIDE;
#endif
  void line_unscaled(int x, int y, int x1, int y1) FL_OVERRIDE;
  void line_unscaled(int x, int y, int x1, int y1, int x2, int y2) FL_OVERRIDE;
  void xyline_unscaled(int x, int y, int x1) FL_OVERRIDE;
  void yxline_unscaled(int x, int y, int y1) FL_OVERRIDE;
  void loop_unscaled(int x0, int y0, int x1, int y1, int x2, int y2) FL_OVERRIDE;
  void loop_unscaled(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) FL_OVERRIDE;
  void polygon_unscaled(int x0, int y0, int x1, int y1, int x2, int y2) FL_OVERRIDE;
  void polygon_unscaled(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) FL_OVERRIDE;
  // --- clipping
  void push_clip(int x, int y, int w, int h) FL_OVERRIDE;
  int clip_box(int x, int y, int w, int h, int &X, int &Y, int &W, int &H) FL_OVERRIDE;
  int not_clipped(int x, int y, int w, int h) FL_OVERRIDE;
  void restore_clip() FL_OVERRIDE;
  Fl_Region scale_clip(float f) FL_OVERRIDE;
  // --- implementation is in src/fl_vertex.cxx which includes src/cfg_gfx/xxx_rect.cxx
  void begin_complex_polygon() FL_OVERRIDE;
  void end_points() FL_OVERRIDE;
  void end_line() FL_OVERRIDE;
  void end_loop() FL_OVERRIDE;
  void end_polygon() FL_OVERRIDE;
  void end_complex_polygon() FL_OVERRIDE;
  void gap() FL_OVERRIDE;
  void ellipse_unscaled(double xt, double yt, double rx, double ry) FL_OVERRIDE;
  void arc_unscaled(int x, int y, int w, int h, double a1, double a2) FL_OVERRIDE;
  void pie_unscaled(int x, int y, int w, int h, double a1, double a2) FL_OVERRIDE;
  void line_style_unscaled(int style, int width, char* dashes) FL_OVERRIDE;
  void color(Fl_Color c) FL_OVERRIDE;
  Fl_Color color() FL_OVERRIDE { return color_; }
  void color(uchar r, uchar g, uchar b) FL_OVERRIDE;
  void set_color(Fl_Color i, unsigned int c) FL_OVERRIDE;
  void free_color(Fl_Color i, int overlay) FL_OVERRIDE;
  Fl_Font set_fonts(const char *name) FL_OVERRIDE;
  int get_font_sizes(Fl_Font fnum, int*& sizep) FL_OVERRIDE;
  const char* get_font_name(Fl_Font fnum, int* ap) FL_OVERRIDE;
  const char *font_name(int num) FL_OVERRIDE;
  void font_name(int num, const char *name) FL_OVERRIDE;
  void global_gc() FL_OVERRIDE;
  void overlay_rect(int x, int y, int w , int h) FL_OVERRIDE;
  void cache_size(Fl_Image *img, int &width, int &height) FL_OVERRIDE;
  void* change_pen_width(int width) FL_OVERRIDE;
  void reset_pen_width(void *data) FL_OVERRIDE;
};


/**
  The graphics driver used when printing on Windows.

  This class is implemented only on the Windows platform.
  It is extremely similar to Fl_GDI_Graphics_Driver.
*/
class Fl_GDI_Printer_Graphics_Driver : public Fl_GDI_Graphics_Driver {
private:
  typedef BOOL (WINAPI* transparent_f_type) (HDC,int,int,int,int,HDC,int,int,int,int,UINT);
  transparent_f_type TransparentBlt();
public:
  int has_feature(driver_feature mask) FL_OVERRIDE { return mask & (NATIVE | PRINTER); }
  void draw_pixmap(Fl_Pixmap *pxm, int XP, int YP, int WP, int HP, int cx, int cy) FL_OVERRIDE;
  void draw_bitmap(Fl_Bitmap *bm, int XP, int YP, int WP, int HP, int cx, int cy) FL_OVERRIDE;
  void draw_rgb(Fl_RGB_Image *rgb, int XP, int YP, int WP, int HP, int cx, int cy) FL_OVERRIDE;
  void copy_offscreen(int x, int y, int w, int h, Fl_Offscreen bitmap, int srcx, int srcy) FL_OVERRIDE;
};

#if USE_GDIPLUS

class Fl_GDIplus_Graphics_Driver : public Fl_GDI_Graphics_Driver {
  friend class Fl_Graphics_Driver;
private:
  Gdiplus::Color gdiplus_color_;
  Gdiplus::Pen *pen_;
  Gdiplus::SolidBrush *brush_;
  // The code below ensures that a connection to GDIplus is only made once, and that the
  // matching connection shutdown is also done exactly once.
  enum {
        STATE_CLOSED = 0,           // no connection, token is invalid
        STATE_STARTUP,              // attempt to start up, avoid recursions for whatever reason
        STATE_OPEN,                 // connection was successful and the token is valid
        STATE_SHUTDOWN              // shutting down the gdi connection, avoid possible recursion
  };
  static int gdiplus_state_;        // reflect the state of the GDIplus driver connection
  static ULONG_PTR gdiplus_token_;  // the token that GDIplus gives to us
public:
  Fl_GDIplus_Graphics_Driver();
  virtual ~Fl_GDIplus_Graphics_Driver();
  bool active;
  static void shutdown(void);
  void color(Fl_Color c) FL_OVERRIDE;
  Fl_Color color() FL_OVERRIDE { return color_; }
  void color(uchar r, uchar g, uchar b) FL_OVERRIDE;
  void line(int x, int y, int x1, int y1) FL_OVERRIDE;
  void line(int x, int y, int x1, int y1, int x2, int y2) FL_OVERRIDE;
  void loop(int x0, int y0, int x1, int y1, int x2, int y2) FL_OVERRIDE;
  void loop(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) FL_OVERRIDE;
  void polygon(int x0, int y0, int x1, int y1, int x2, int y2) FL_OVERRIDE;
  void polygon(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) FL_OVERRIDE;
  void line_style(int style, int width, char* dashes) FL_OVERRIDE;
  void arc_unscaled(int x, int y, int w, int h, double a1, double a2) FL_OVERRIDE;
  void pie_unscaled(int x, int y, int w, int h, double a1, double a2) FL_OVERRIDE;
  void draw_circle(int x, int y, int d, Fl_Color c) FL_OVERRIDE;
  void transformed_vertex(double xf, double yf) FL_OVERRIDE;
  void vertex(double x,double y) FL_OVERRIDE;
  void end_points() FL_OVERRIDE;
  void end_line() FL_OVERRIDE;
  void end_loop() FL_OVERRIDE;
  void end_polygon() FL_OVERRIDE;
  void end_complex_polygon() FL_OVERRIDE;
  void circle(double x, double y, double r) FL_OVERRIDE;
  void antialias(int state) FL_OVERRIDE;
  int antialias() FL_OVERRIDE;
};

#endif // USE_GDIPLUS

#endif // FL_GDI_GRAPHICS_DRIVER_H
