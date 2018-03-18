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
class FL_EXPORT Fl_Android_Graphics_Driver : public Fl_Scalable_Graphics_Driver
{
public:
  Fl_Android_Graphics_Driver();
  virtual ~Fl_Android_Graphics_Driver() override;

  void make_current(Fl_Window*);

  // --- text and font stuff
  virtual void draw_unscaled(const char* str, int n, int x, int y) override;
  virtual double width_unscaled(const char *str, int n) override;
  virtual double width_unscaled(unsigned int c) override;
  virtual Fl_Fontsize size_unscaled() override;
  virtual void text_extents_unscaled(const char *str, int n, int &dx, int &dy, int &w, int &h) override;
  virtual int height_unscaled() override;
  virtual int descent_unscaled() override;
  virtual void font_unscaled(Fl_Font face, Fl_Fontsize size) override;
  virtual const char *get_font_name(Fl_Font fnum, int* ap) override;
  virtual const char *font_name(int num) override;
  virtual int get_font_sizes(Fl_Font fnum, int*& sizep) override;
  virtual Fl_Font set_fonts(const char *name) override;
  virtual void font_name(int num, const char *name) override;

  // --- line drawinf stuff
  virtual void rectf_unscaled(float x, float y, float w, float h) override;
  void rectf_unclipped(float x, float y, float w, float h);
  virtual void point_unscaled(float x, float y) override;
  void rect_unscaled(float x, float y, float w, float h);
  virtual void xyline_unscaled(float x, float y, float x1) override;
  void xyline_unclipped(float x, float y, float x1);
  virtual void yxline_unscaled(float x, float y, float y1) override;
  void yxline_unclipped(float x, float y, float y1);

  // --- clipping
  virtual void push_clip(int x, int y, int w, int h) override;
  virtual int clip_box(int x, int y, int w, int h, int &X, int &Y, int &W, int &H) override;
  virtual int not_clipped(int x, int y, int w, int h) override;
  virtual void push_no_clip() override;
  virtual void pop_clip() override;
  virtual void restore_clip() override;
  virtual void clip_region(Fl_Region r) override;
  virtual Fl_Region clip_region() override;

  virtual void line_style_unscaled(int style, float width, char* dashes) override;

  // --- matrix based drawing
//  virtual void line_unscaled(float x, float y, float x1, float y1) override;


#if 0
  virtual int has_feature(driver_feature mask) { return mask & NATIVE; }
  char can_do_alpha_blending();
  // --- bitmap stuff
  Fl_Bitmask create_bitmask(int w, int h, const uchar *array);
  void delete_bitmask(Fl_Bitmask bm);
  virtual void draw_unscaled(int angle, const char *str, int n, int x, int y);
  virtual void rtl_draw_unscaled(const char* str, int n, int x, int y);
  void draw_unscaled(Fl_Pixmap *pxm, float s, int XP, int YP, int WP, int HP, int cx, int cy);
  void draw_unscaled(Fl_Bitmap *pxm, float s, int XP, int YP, int WP, int HP, int cx, int cy);
  void draw_unscaled(Fl_RGB_Image *img, float s, int XP, int YP, int WP, int HP, int cx, int cy);
  int draw_scaled(Fl_Image *img, int XP, int YP, int WP, int HP);
  virtual void draw_image_unscaled(const uchar* buf, int X,int Y,int W,int H, int D=3, int L=0);
  virtual void draw_image_unscaled(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=3);
  virtual void draw_image_mono_unscaled(const uchar* buf, int X,int Y,int W,int H, int D=1, int L=0);
  virtual void draw_image_mono_unscaled(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=1);
  fl_uintptr_t cache(Fl_Pixmap *img, int w, int h, const char *const*array);
  virtual void uncache_pixmap(fl_uintptr_t p);
  fl_uintptr_t cache(Fl_Bitmap *img, int w, int h, const uchar *array);
  void uncache(Fl_RGB_Image *img, fl_uintptr_t &id_, fl_uintptr_t &mask_);
  void copy_offscreen_with_alpha(int x,int y,int w,int h,HBITMAP bitmap,int srcx,int srcy);
  virtual void copy_offscreen(int x, int y, int w, int h, Fl_Offscreen pixmap, int srcx, int srcy);
  void add_rectangle_to_region(Fl_Region r, int x, int y, int w, int h);
  Fl_Region XRectangleRegion(int x, int y, int w, int h);
  void XDestroyRegion(Fl_Region r);
  void translate_all(int x, int y);
  void untranslate_all(void);
  static HRGN scale_region(HRGN r, float f, Fl_GDI_Graphics_Driver *dr);
  virtual void scale(float f);
protected:
  void transformed_vertex0(float x, float y);
  void fixloop();
  // --- implementation is in src/fl_rect.cxx which includes src/cfg_gfx/gdi_rect.cxx
  virtual void line_unscaled(float x, float y, float x1, float y1);
  virtual void line_unscaled(float x, float y, float x1, float y1, float x2, float y2);
  virtual void loop_unscaled(float x0, float y0, float x1, float y1, float x2, float y2);
  virtual void loop_unscaled(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3);
  virtual void polygon_unscaled(float x0, float y0, float x1, float y1, float x2, float y2);
  virtual void polygon_unscaled(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3);
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
  void color(uchar r, uchar g, uchar b);
  Fl_Color color() { return color_; }
  void set_color(Fl_Color i, unsigned int c);
  void free_color(Fl_Color i, int overlay);
  void set_spot(int font, int size, int X, int Y, int W, int H, Fl_Window *win);
  void reset_spot();
  void global_gc();
  virtual void overlay_rect(int x, int y, int w , int h);

#endif

protected:
  static uint16_t make565(Fl_Color crgba);

  void render_bytemap(int x, int y, Fl_Android_Bytemap *bm, Fl_Rect_Region &r);
  int render_letter(int xx, int yy, uint32_t c, Fl_Rect_Region &r);

  int32_t pStride;
  uint16_t *pBits;

  int pLineStyle = 0;

  // Clipping region of the current window in window coordinates (see: pStride and pBits)
  Fl_Rect_Region pWindowRegion;

  // clipping region of the window minus overlapping other windows
  Fl_Complex_Region pDesktopWindowRegion;

  // Final clipping region for all graphics calls to this class.
  Fl_Complex_Region pClippingRegion;
};


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


#endif // FL_ANDROID_GRAPHICS_DRIVER_H

//
// End of "$Id$".
//
