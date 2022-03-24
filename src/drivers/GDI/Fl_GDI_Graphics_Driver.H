//
// Definition of classes  Fl_Graphics_Driver, Fl_Surface_Device, Fl_Display_Device
// for the Fast Light Tool Kit (FLTK).
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
class FL_EXPORT Fl_GDI_Graphics_Driver : public Fl_Scalable_Graphics_Driver {
private:
  BOOL alpha_blend_(int x, int y, int w, int h, HDC src_gc, int srcx, int srcy, int srcw, int srch);
  int depth; // to support translation
  POINT *origins; // to support translation
  void set_current_();
  void draw_fixed(Fl_Pixmap *pxm, int XP, int YP, int WP, int HP, int cx, int cy);
  void draw_fixed(Fl_Bitmap *pxm, int XP, int YP, int WP, int HP, int cx, int cy);
  virtual void make_unused_color_(unsigned char &r, unsigned char &g, unsigned char &b, int color_count, void **data);
protected:
  void draw_fixed(Fl_RGB_Image *rgb, int XP, int YP, int WP, int HP, int cx, int cy);
  void cache(Fl_RGB_Image *rgb);
  HDC gc_;
  int numcount;
  int counts[20];
  uchar *mask_bitmap_;
  uchar **mask_bitmap() {return &mask_bitmap_;}
  POINT *long_point;
public:
  Fl_GDI_Graphics_Driver();
  virtual ~Fl_GDI_Graphics_Driver();
  virtual int has_feature(driver_feature mask) { return mask & NATIVE; }
  char can_do_alpha_blending();
  virtual void gc(void *ctxt) { gc_ = (HDC)ctxt; global_gc(); }
  virtual void *gc() {return gc_;}

  // --- bitmap stuff
  HBITMAP create_bitmask(int w, int h, const uchar *array); // NOT virtual
  virtual void delete_bitmask(fl_uintptr_t bm);
  HBITMAP create_alphamask(int w, int h, int d, int ld, const uchar *array);
  virtual void draw_unscaled(const char* str, int n, int x, int y);
  virtual void draw_unscaled(int angle, const char *str, int n, int x, int y);
  virtual void rtl_draw_unscaled(const char* str, int n, int x, int y);
  virtual void font_unscaled(Fl_Font face, Fl_Fontsize size);
  virtual void draw_rgb(Fl_RGB_Image *img, int XP, int YP, int WP, int HP, int cx, int cy);
  virtual void draw_image_unscaled(const uchar* buf, int X,int Y,int W,int H, int D=3, int L=0);
  virtual void draw_image_unscaled(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=3);
  virtual void draw_image_mono_unscaled(const uchar* buf, int X,int Y,int W,int H, int D=1, int L=0);
  virtual void draw_image_mono_unscaled(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=1);
  void cache(Fl_Pixmap *img);
  virtual void uncache_pixmap(fl_uintptr_t p);
  void cache(Fl_Bitmap *img);
  void uncache(Fl_RGB_Image *img, fl_uintptr_t &id_, fl_uintptr_t &mask_);
  virtual double width_unscaled(const char *str, int n);
  virtual double width_unscaled(unsigned int c);
  void text_extents_unscaled(const char*, int n, int& dx, int& dy, int& w, int& h);
  int height_unscaled();
  int descent_unscaled();
  Fl_Fontsize size_unscaled();
#if ! defined(FL_DOXYGEN)
  void copy_offscreen_with_alpha(int x,int y,int w,int h,HBITMAP bitmap,int srcx,int srcy);
#endif
  virtual void copy_offscreen(int x, int y, int w, int h, Fl_Offscreen pixmap, int srcx, int srcy);
  void add_rectangle_to_region(Fl_Region r, int x, int y, int w, int h);
  Fl_Region XRectangleRegion(int x, int y, int w, int h);
  void XDestroyRegion(Fl_Region r);
  void translate_all(int x, int y);
  void untranslate_all(void);
  static HRGN scale_region(HRGN r, float f, Fl_GDI_Graphics_Driver *dr);
  virtual void scale(float f);
  float scale() {return Fl_Graphics_Driver::scale();}
protected:
  void transformed_vertex0(float x, float y);
  void fixloop();
  virtual void point(int x, int y);
  void focus_rect(int x, int y, int w, int h);
  virtual void rectf_unscaled(int x, int y, int w, int h);
#if USE_COLORMAP
  virtual void colored_rectf(int x, int y, int w, int h, uchar r, uchar g, uchar b);
#endif
  virtual void line_unscaled(int x, int y, int x1, int y1);
  virtual void line_unscaled(int x, int y, int x1, int y1, int x2, int y2);
  virtual void xyline_unscaled(int x, int y, int x1);
  virtual void yxline_unscaled(int x, int y, int y1);
  virtual void loop_unscaled(int x0, int y0, int x1, int y1, int x2, int y2);
  virtual void loop_unscaled(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3);
  virtual void polygon_unscaled(int x0, int y0, int x1, int y1, int x2, int y2);
  virtual void polygon_unscaled(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3);
  // --- clipping
  void push_clip(int x, int y, int w, int h);
  int clip_box(int x, int y, int w, int h, int &X, int &Y, int &W, int &H);
  int not_clipped(int x, int y, int w, int h);
  void restore_clip();
  virtual Fl_Region scale_clip(float f);
  // --- implementation is in src/fl_vertex.cxx which includes src/cfg_gfx/xxx_rect.cxx
  void begin_complex_polygon();
  void end_points();
  void end_line();
  void end_loop();
  void end_polygon();
  void end_complex_polygon();
  void gap();
  virtual void ellipse_unscaled(double xt, double yt, double rx, double ry);
  virtual void arc_unscaled(int x, int y, int w, int h, double a1, double a2);
  virtual void pie_unscaled(int x, int y, int w, int h, double a1, double a2);
  virtual void line_style_unscaled(int style, int width, char* dashes);
  void color(Fl_Color c);
  Fl_Color color() { return color_; }
  void color(uchar r, uchar g, uchar b);
  void set_color(Fl_Color i, unsigned int c);
  void free_color(Fl_Color i, int overlay);
  void set_spot(int font, int size, int X, int Y, int W, int H, Fl_Window *win);
  virtual Fl_Font set_fonts(const char *name);
  virtual int get_font_sizes(Fl_Font fnum, int*& sizep);
  virtual const char* get_font_name(Fl_Font fnum, int* ap);
  virtual const char *font_name(int num);
  virtual void font_name(int num, const char *name);
  void global_gc();
  virtual void overlay_rect(int x, int y, int w , int h);
  virtual void cache_size(Fl_Image *img, int &width, int &height);
  virtual void* change_pen_width(int width);
  virtual void reset_pen_width(void *data);
};


/**
  The graphics driver used when printing on Windows.

  This class is implemented only on the Windows platform.
  It is extremely similar to Fl_GDI_Graphics_Driver.
*/
class FL_EXPORT Fl_GDI_Printer_Graphics_Driver : public Fl_GDI_Graphics_Driver {
private:
  typedef BOOL (WINAPI* transparent_f_type) (HDC,int,int,int,int,HDC,int,int,int,int,UINT);
  transparent_f_type TransparentBlt();
public:
  virtual int has_feature(driver_feature mask) { return mask & (NATIVE | PRINTER); }
  void draw_pixmap(Fl_Pixmap *pxm, int XP, int YP, int WP, int HP, int cx, int cy);
  void draw_bitmap(Fl_Bitmap *bm, int XP, int YP, int WP, int HP, int cx, int cy);
  void draw_rgb(Fl_RGB_Image *rgb, int XP, int YP, int WP, int HP, int cx, int cy);
  void copy_offscreen(int x, int y, int w, int h, Fl_Offscreen bitmap, int srcx, int srcy);
};

#if USE_GDIPLUS

class Fl_GDIplus_Graphics_Driver : public Fl_GDI_Graphics_Driver {
  friend class Fl_Graphics_Driver;
private:
  Gdiplus::Color gdiplus_color_;
  Gdiplus::Pen *pen_;
  Gdiplus::SolidBrush *brush_;
  static ULONG_PTR gdiplusToken;
public:
  Fl_GDIplus_Graphics_Driver();
  virtual ~Fl_GDIplus_Graphics_Driver();
  bool active;
  static void shutdown(void);
virtual void color(Fl_Color c);
virtual Fl_Color color() { return color_; }
virtual void color(uchar r, uchar g, uchar b);
  virtual void line(int x, int y, int x1, int y1);
  virtual void line(int x, int y, int x1, int y1, int x2, int y2);
  void loop(int x0, int y0, int x1, int y1, int x2, int y2);
  void loop(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3);
  void polygon(int x0, int y0, int x1, int y1, int x2, int y2);
  void polygon(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3);
  virtual void line_style(int style, int width, char* dashes);
  virtual void arc_unscaled(int x, int y, int w, int h, double a1, double a2);
  virtual void pie_unscaled(int x, int y, int w, int h, double a1, double a2);
  virtual void transformed_vertex(double xf, double yf);
  virtual void vertex(double x,double y);
  virtual void end_points();
  virtual void end_line();
  virtual void end_loop();
  virtual void end_polygon();
  virtual void end_complex_polygon();
  virtual void circle(double x, double y, double r);
  virtual void antialias(int state);
  virtual int antialias();
};

#endif // USE_GDIPLUS

#endif // FL_GDI_GRAPHICS_DRIVER_H
