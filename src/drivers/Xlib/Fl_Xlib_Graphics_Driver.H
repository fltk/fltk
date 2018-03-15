//
// "$Id$"
//
// Definition of class Fl_Xlib_Graphics_Driver for the Fast Light Tool Kit (FLTK).
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
 *
 This class is implemented only on the Xlib platform.
 */
class FL_EXPORT Fl_Xlib_Graphics_Driver : public Fl_Scalable_Graphics_Driver {
private:
  int offset_x_, offset_y_; // translation between user and graphical coordinates: graphical = user + offset
  unsigned depth_; // depth of translation stack
  int stack_x_[FL_XLIB_GRAPHICS_TRANSLATION_STACK_SIZE]; // translation stack allowing cumulative translations
  int stack_y_[FL_XLIB_GRAPHICS_TRANSLATION_STACK_SIZE];
  int line_delta_;
  virtual void set_current_();
  int clip_max_; // +/- x/y coordinate limit (16-bit coordinate space)
protected:
  virtual void draw_unscaled(Fl_Pixmap *pxm, float s, int XP, int YP, int WP, int HP, int cx, int cy);
  virtual void draw_unscaled(Fl_Bitmap *pxm, float s, int XP, int YP, int WP, int HP, int cx, int cy);
  virtual void draw_unscaled(Fl_RGB_Image *img, float s, int XP, int YP, int WP, int HP, int cx, int cy);
  virtual void draw_image_unscaled(const uchar* buf, int X,int Y,int W,int H, int D=3, int L=0);
  virtual void draw_image_unscaled(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=3);
  virtual void draw_image_mono_unscaled(const uchar* buf, int X,int Y,int W,int H, int D=1, int L=0);
  virtual void draw_image_mono_unscaled(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=1);
#if HAVE_XRENDER
  virtual int draw_scaled(Fl_Image *img, int XP, int YP, int WP, int HP);
  int scale_and_render_pixmap(Fl_Offscreen pixmap, int depth, double scale_x, double scale_y, int srcx, int srcy, int XP, int YP, int WP, int HP);
#endif
  virtual int height_unscaled();
  virtual int descent_unscaled();
  virtual Region scale_clip(float f);
#if USE_XFT
  void drawUCS4(const void *str, int n, int x, int y);
#endif
#if USE_PANGO
  friend class Fl_X11_Screen_Driver;
  static PangoContext *pctxt_;
  static PangoFontMap *pfmap_;
  static PangoLayout *playout_;
  PangoFontDescription *pfd_;
  void do_draw(int from_right, const char *str, int n, int x, int y);
  static PangoContext *context();
  static void init_built_in_fonts();
#endif
  static GC gc_;
  uchar **mask_bitmap_;
  uchar **mask_bitmap() {return mask_bitmap_;}
  void mask_bitmap(uchar **value) { mask_bitmap_ = value; }
  int p_size;
  typedef struct {short x, y;} XPOINT;
  XPOINT *p;
#if USE_XFT
  static Window draw_window;
  static struct _XftDraw* draw_;
#endif
public:
  Fl_Xlib_Graphics_Driver(void);
  virtual ~Fl_Xlib_Graphics_Driver();
  void translate_all(int dx, int dy);
  void untranslate_all();
  virtual void scale(float f);
  virtual int has_feature(driver_feature mask) { return mask & NATIVE; }
  virtual void *gc() { return gc_; }
  virtual void gc(void *value);
  char can_do_alpha_blending();
#if USE_XFT
  static void destroy_xft_draw(Window id);
#endif

  // --- bitmap stuff
  Fl_Bitmask create_bitmask(int w, int h, const uchar *array);
  void delete_bitmask(Fl_Bitmask bm);
  virtual void draw_unscaled(const char* str, int n, int x, int y);
  virtual void draw_unscaled(int angle, const char *str, int n, int x, int y);
  virtual void rtl_draw_unscaled(const char* str, int n, int x, int y);
  virtual void font_unscaled(Fl_Font face, Fl_Fontsize size);
  fl_uintptr_t cache(Fl_Pixmap *img, int w, int h, const char *const*array);
  virtual void uncache_pixmap(fl_uintptr_t p);
  fl_uintptr_t cache(Fl_Bitmap *img, int w, int h, const uchar *array);
  void uncache(Fl_RGB_Image *img, fl_uintptr_t &id_, fl_uintptr_t &mask_);
  virtual double width_unscaled(const char *str, int n);
  virtual double width_unscaled(unsigned int c);
  virtual void text_extents_unscaled(const char*, int n, int& dx, int& dy, int& w, int& h);
  virtual Fl_Fontsize size_unscaled();
  virtual void copy_offscreen(int x, int y, int w, int h, Fl_Offscreen pixmap, int srcx, int srcy);
#if ! defined(FL_DOXYGEN)
  void copy_offscreen_with_alpha(int x, int y, int w, int h, Fl_Offscreen pixmap, int srcx, int srcy);
#endif
  void add_rectangle_to_region(Fl_Region r, int x, int y, int w, int h);
  Fl_Region XRectangleRegion(int x, int y, int w, int h);
  void XDestroyRegion(Fl_Region r);
protected:
  virtual void transformed_vertex0(float x, float y);
  void fixloop();
  // --- implementation is in src/fl_rect.cxx which includes src/cfg_gfx/xlib_rect.cxx
  virtual void point_unscaled(float x, float y);
  virtual void rect_unscaled(float x, float y, float w, float h);
  virtual void rectf_unscaled(float x, float y, float w, float h);
  virtual void line_unscaled(float x, float y, float x1, float y1);
  virtual void line_unscaled(float x, float y, float x1, float y1, float x2, float y2);
  virtual void xyline_unscaled(float x, float y, float x1);
  virtual void yxline_unscaled(float x, float y, float y1);
  virtual void loop_unscaled(float x0, float y0, float x1, float y1, float x2, float y2);
  virtual void loop_unscaled(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3);
  virtual void polygon_unscaled(float x0, float y0, float x1, float y1, float x2, float y2);
  virtual void polygon_unscaled(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3);

  // --- clipped drawing (after scaling, with real screen coordinates)
  // --- currently specific to Fl_Xlib_Graphics_Driver (16-bit coordinate clipping)

  // clipping limits
  virtual int clip_max() { return clip_max_; }
  virtual int clip_min() { return -clip_max_; }

  // clip one coordinate (x or y)
  virtual int clip_xy(int x);

  // clip one line
  virtual int clip_line(int &x1, int &y1, int &x2, int &y2);

  // clip one rectangle
  virtual int clip_rect(int &x, int &y, int &w, int &h);

  // draw a line after clipping (if visible)
  virtual void draw_clipped_line(int x1, int y1, int x2, int y2);

  // --- clipping
  void push_clip(int x, int y, int w, int h);
  int clip_box(int x, int y, int w, int h, int &X, int &Y, int &W, int &H);
  int not_clipped(int x, int y, int w, int h);
  void push_no_clip();
  void pop_clip();
  void restore_clip();
  void begin_complex_polygon();
  void end_points();
  void end_line();
  void end_loop();
  void end_polygon();
  void end_complex_polygon();
  void gap();
  virtual void ellipse_unscaled(double xt, double yt, double rx, double ry);
  // --- implementation is in src/fl_arci.cxx which includes src/cfg_gfx/xxx_arci.cxx
  virtual void arc_unscaled(float x, float y, float w, float h, double a1, double a2);
  virtual void pie_unscaled(float x, float y, float w, float h, double a1, double a2);
  virtual void line_style_unscaled(int style, float width, char* dashes);
  void color(Fl_Color c);
  void set_color(Fl_Color i, unsigned int c);
  void free_color(Fl_Color i, int overlay);
  Fl_Color color() { return color_; }
  void color(uchar r, uchar g, uchar b);
  virtual float scale_font_for_PostScript(Fl_Font_Descriptor *desc, int s);
  virtual float scale_bitmap_for_PostScript();
  virtual void set_spot(int font, int size, int X, int Y, int W, int H, Fl_Window *win);
  virtual void reset_spot();
  virtual const char* get_font_name(Fl_Font fnum, int* ap);
  virtual int get_font_sizes(Fl_Font fnum, int*& sizep);
#if !USE_XFT
  virtual unsigned font_desc_size();
#endif
  virtual const char *font_name(int num);
  virtual void font_name(int num, const char *name);
  virtual Fl_Font set_fonts(const char* xstarname);
};

#endif // FL_XLIB_GRAPHICS_DRIVER_H

//
// End of "$Id$".
//
