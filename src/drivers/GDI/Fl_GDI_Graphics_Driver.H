//
// "$Id$"
//
// Definition of classes  Fl_Graphics_Driver, Fl_Surface_Device, Fl_Display_Device
// for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2017 by Bill Spitzak and others.
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
 \file Fl_GDI_Graphics_Driver.H
 \brief Definition of MSWindows GDI graphics driver.
 */

#ifndef FL_GDI_GRAPHICS_DRIVER_H
#define FL_GDI_GRAPHICS_DRIVER_H

#include <FL/Fl_Graphics_Driver.H>
#include <windows.h>


/**
 \brief The MSWindows-specific graphics class.
 *
 This class is implemented only on the MSWindows platform.
 */
class FL_EXPORT Fl_GDI_Graphics_Driver : public Fl_Scalable_Graphics_Driver {
private:
  BOOL alpha_blend_(int x, int y, int w, int h, HDC src_gc, int srcx, int srcy, int srcw, int srch);
  int depth; // to support translation
  POINT *origins; // to support translation
protected:
  HDC gc_;
  int numcount;
  int counts[20];
  uchar **mask_bitmap_;
  uchar **mask_bitmap() {return mask_bitmap_;}
  void mask_bitmap(uchar **value) { mask_bitmap_ = value; }
  int p_size;
  POINT *p;
public:
  Fl_GDI_Graphics_Driver() {mask_bitmap_ = NULL; gc_ = NULL; p_size = 0; p = NULL; depth = -1; origins = NULL;}
  virtual ~Fl_GDI_Graphics_Driver() { if (p) free(p); delete[] origins;}
  virtual int has_feature(driver_feature mask) { return mask & NATIVE; }
  char can_do_alpha_blending();
  virtual void gc(void *ctxt) { gc_ = (HDC)ctxt; global_gc(); }
  virtual void *gc() {return gc_;}

  // --- bitmap stuff
  Fl_Bitmask create_bitmask(int w, int h, const uchar *array);
  void delete_bitmask(Fl_Bitmask bm);
  virtual void draw_unscaled(const char* str, int n, int x, int y);
  virtual void draw_unscaled(int angle, const char *str, int n, int x, int y);
  virtual void rtl_draw_unscaled(const char* str, int n, int x, int y);
  virtual void font_unscaled(Fl_Font face, Fl_Fontsize size);
  void draw_unscaled(Fl_Pixmap *pxm, float s, int XP, int YP, int WP, int HP, int cx, int cy);
  void draw_unscaled(Fl_Bitmap *pxm, float s, int XP, int YP, int WP, int HP, int cx, int cy);
  void draw_unscaled(Fl_RGB_Image *img, float s, int XP, int YP, int WP, int HP, int cx, int cy);
  int draw_scaled(Fl_Image *img, int XP, int YP, int WP, int HP);
  virtual void draw_image_unscaled(const uchar* buf, int X,int Y,int W,int H, int D=3, int L=0);
  virtual void draw_image_unscaled(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=3);
  virtual void draw_image_mono_unscaled(const uchar* buf, int X,int Y,int W,int H, int D=1, int L=0);
  virtual void draw_image_mono_unscaled(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=1);
  fl_uintptr_t cache(Fl_Pixmap *img, int w, int h, const char *const*array);
  fl_uintptr_t cache(Fl_Bitmap *img, int w, int h, const uchar *array);
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
  virtual void copy_offscreen_unscaled(float x, float y, float w, float h, Fl_Offscreen pixmap, float srcx, float srcy);
  void add_rectangle_to_region(Fl_Region r, int x, int y, int w, int h);
  Fl_Region XRectangleRegion(int x, int y, int w, int h);
  void XDestroyRegion(Fl_Region r);
  void translate_all(int x, int y);
  void untranslate_all(void);
  static HRGN scale_region(HRGN r, float f, Fl_GDI_Graphics_Driver *dr);
  virtual void scale(float f);
  virtual float scale();
protected:
  void transformed_vertex0(float x, float y);
  void fixloop();
  // --- implementation is in src/fl_rect.cxx which includes src/cfg_gfx/gdi_rect.cxx
  virtual void point_unscaled(float x, float y);
  void rect_unscaled(float x, float y, float w, float h);
  void focus_rect(int x, int y, int w, int h);
  void rectf_unscaled(float x, float y, float w, float h);
  virtual void line_unscaled(float x, float y, float x1, float y1);
  virtual void line_unscaled(float x, float y, float x1, float y1, float x2, float y2);
  virtual void xyline_unscaled(float x, float y, float x1);
  virtual void xyline_unscaled(float x, float y, float x1, float y2);
  virtual void xyline_unscaled(float x, float y, float x1, float y2, float x3);
  virtual void yxline_unscaled(float x, float y, float y1);
  virtual void yxline_unscaled(float x, float y, float y1, float x2);
  virtual void yxline_unscaled(float x, float y, float y1, float x2, float y3);
  virtual void loop_unscaled(float x0, float y0, float x1, float y1, float x2, float y2);
  virtual void loop_unscaled(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3);
  virtual void polygon_unscaled(float x0, float y0, float x1, float y1, float x2, float y2);
  virtual void polygon_unscaled(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3);
  // --- clipping
  void push_clip(int x, int y, int w, int h);
  int clip_box(int x, int y, int w, int h, int &X, int &Y, int &W, int &H);
  int not_clipped(int x, int y, int w, int h);
  void push_no_clip();
  void pop_clip();
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
  // --- implementation is in src/fl_arc.cxx which includes src/cfg_gfx/xxx_arc.cxx if needed
  // using void Fl_Graphics_Driver::arc(double x, double y, double r, double start, double end);
  // --- implementation is in src/fl_arci.cxx which includes src/cfg_gfx/xxx_arci.cxx
  virtual void arc_unscaled(float x, float y, float w, float h, double a1, double a2);
  virtual void pie_unscaled(float x, float y, float w, float h, double a1, double a2);
  // --- implementation is in src/fl_line_style.cxx which includes src/cfg_gfx/xxx_line_style.cxx
  virtual void line_style_unscaled(int style, float width, char* dashes);
  // --- implementation is in src/fl_color.cxx which includes src/cfg_gfx/xxx_color.cxx
  void color(Fl_Color c);
  Fl_Color color() { return color_; }
  void color(uchar r, uchar g, uchar b);
  void set_color(Fl_Color i, unsigned int c);
  void free_color(Fl_Color i, int overlay);
  void set_spot(int font, int size, int X, int Y, int W, int H, Fl_Window *win);
  void reset_spot();
  virtual Fl_Font set_fonts(const char *name);
  virtual int get_font_sizes(Fl_Font fnum, int*& sizep);
  virtual const char* get_font_name(Fl_Font fnum, int* ap);
  virtual const char *font_name(int num);
  virtual void font_name(int num, const char *name);
  virtual unsigned font_desc_size();
  void global_gc();
};


/**
 The graphics driver used when printing on MSWindows.
 *
 This class is implemented only on the MSWindows platform. It 's extremely similar to Fl_GDI_Graphics_Driver.
 */
class FL_EXPORT Fl_GDI_Printer_Graphics_Driver : public Fl_GDI_Graphics_Driver {
private:
  typedef BOOL (WINAPI* transparent_f_type) (HDC,int,int,int,int,HDC,int,int,int,int,UINT);
  transparent_f_type TransparentBlt();
public:
  virtual int has_feature(driver_feature mask) { return mask & (NATIVE | PRINTER); }
  void draw_unscaled(Fl_Pixmap *pxm, float s, int XP, int YP, int WP, int HP, int cx, int cy);
  void draw_unscaled(Fl_Bitmap *bm, float s, int XP, int YP, int WP, int HP, int cx, int cy);
  int draw_scaled(Fl_Image *img, int XP, int YP, int WP, int HP);
};


#endif // FL_GDI_GRAPHICS_DRIVER_H

//
// End of "$Id$".
//
