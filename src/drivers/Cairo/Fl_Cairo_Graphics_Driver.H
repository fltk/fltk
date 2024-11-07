//
// Support for Cairo graphics for the Fast Light Tool Kit (FLTK).
//
// Copyright 2021-2022 by Bill Spitzak and others.
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

/* \file
    Declaration of class Fl_Cairo_Graphics_Driver.
*/

#ifndef FL_CAIRO_GRAPHICS_DRIVER_H
# define FL_CAIRO_GRAPHICS_DRIVER_H

#include <FL/Fl_Graphics_Driver.H>
#include <cairo/cairo.h>

typedef struct _PangoLayout  PangoLayout;
typedef struct _PangoContext PangoContext;
typedef struct _PangoFontDescription PangoFontDescription;


class Fl_Cairo_Font_Descriptor : public Fl_Font_Descriptor {
public:
  Fl_Cairo_Font_Descriptor(const char* fontname, Fl_Fontsize size, PangoContext *context);
  FL_EXPORT ~Fl_Cairo_Font_Descriptor() FL_OVERRIDE;
  PangoFontDescription *fontref;
  int **width; // array of arrays of character widths
  int line_height;
};


class FL_EXPORT Fl_Cairo_Graphics_Driver : public Fl_Graphics_Driver {
private:
  bool *needs_commit_tag_; // NULL or points to whether cairo surface was drawn to
  cairo_t *dummy_cairo_; // used to measure text width before showing a window
  int linestyle_;
  int do_width_unscaled_(const char* str, int n);
protected:
  cairo_t *cairo_;
  PangoContext *pango_context_;
  PangoLayout *pango_layout_;
  static Fl_Font font_count_;
public:
  Fl_Cairo_Graphics_Driver();
  ~Fl_Cairo_Graphics_Driver() FL_OVERRIDE;

  class Clip {
  public:
    int x, y, w, h;
    Clip *prev;
  };
  Clip * clip_;

  int gap_;
  cairo_t *cr() { return cairo_; }
  PangoLayout *pango_layout() {return pango_layout_;}
  void set_cairo(cairo_t *c, float f = 0);
  static cairo_pattern_t *calc_cairo_mask(const Fl_RGB_Image *rgb);
  static const char *clean_utf8(const char* str, int &n);

  void check_status(void);

  unsigned char cr_,cg_,cb_;
  char  linedash_[256];//should be enough
  void concat();  // transform ror scalable dradings...
  void reconcat(); //invert
  void recover(); //recovers the state after grestore (such as line styles...)
  void reset();

  float scale_x;
  float scale_y;
  int wld_scale;
  float angle;
  int left_margin;
  int top_margin;
  static const cairo_format_t cairo_format;

  void surface_needs_commit() {
    if (needs_commit_tag_) *needs_commit_tag_ = true;
  }
  void needs_commit_tag(bool *tag) { needs_commit_tag_ = tag; }

  // implementation of drawing methods
  void color(Fl_Color c) FL_OVERRIDE;
  void color(uchar r, uchar g, uchar b) FL_OVERRIDE;
  Fl_Color color() FL_OVERRIDE;

  void push_clip(int x, int y, int w, int h) FL_OVERRIDE;
  void push_no_clip() FL_OVERRIDE;
  void pop_clip() FL_OVERRIDE;

  void line_style(int style, int width=0, char* dashes=0) FL_OVERRIDE;

  void rect(int x, int y, int w, int h) FL_OVERRIDE;
  void rectf(int x, int y, int w, int h) FL_OVERRIDE;

  void xyline(int x, int y, int x1) FL_OVERRIDE;
  void xyline(int x, int y, int x1, int y2) FL_OVERRIDE;
  void xyline(int x, int y, int x1, int y2, int x3) FL_OVERRIDE;

  void yxline(int x, int y, int y1) FL_OVERRIDE;
  void yxline(int x, int y, int y1, int x2) FL_OVERRIDE;
  void yxline(int x, int y, int y1, int x2, int y3) FL_OVERRIDE;

  void line(int x1, int y1, int x2, int y2) FL_OVERRIDE;
  void line(int x1, int y1, int x2, int y2, int x3, int y3) FL_OVERRIDE;

  void loop(int x0, int y0, int x1, int y1, int x2, int y2) FL_OVERRIDE;
  void loop(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) FL_OVERRIDE;
  void polygon(int x0, int y0, int x1, int y1, int x2, int y2) FL_OVERRIDE;
  void polygon(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) FL_OVERRIDE;
  void point(int x, int y) FL_OVERRIDE;
  void overlay_rect(int x, int y, int w , int h) FL_OVERRIDE;
  int clip_box(int x, int y, int w, int h, int &X, int &Y, int &W, int &H) FL_OVERRIDE;
  void restore_clip() FL_OVERRIDE;
  int not_clipped(int x, int y, int w, int h) FL_OVERRIDE;

  void begin_line() FL_OVERRIDE;
  void begin_loop() FL_OVERRIDE;
  void begin_polygon() FL_OVERRIDE;
  void vertex(double x, double y) FL_OVERRIDE;
  void curve(double x, double y, double x1, double y1, double x2, double y2, double x3, double y3) FL_OVERRIDE;
  void circle(double x, double y, double r) FL_OVERRIDE;
  void arc(double x, double y, double r, double start, double a) FL_OVERRIDE;
  void arc(int x, int y, int w, int h, double a1, double a2) FL_OVERRIDE;
  void pie(int x, int y, int w, int h, double a1, double a2) FL_OVERRIDE;
  void end_points() FL_OVERRIDE;
  void end_line() FL_OVERRIDE;
  void end_loop() FL_OVERRIDE;
  void end_polygon() FL_OVERRIDE;
  void begin_complex_polygon() FL_OVERRIDE { begin_polygon(); }
  void gap() FL_OVERRIDE { gap_ = 1; }
  void end_complex_polygon() FL_OVERRIDE { end_polygon(); }
  void transformed_vertex(double x, double y) FL_OVERRIDE;

  void draw_image_mono(const uchar* d, int x,int y,int w,int h, int delta=1, int ld=0) FL_OVERRIDE;
  void draw_image(Fl_Draw_Image_Cb call, void* data, int x,int y, int w, int h, int delta=3) FL_OVERRIDE;
  void draw_image_mono(Fl_Draw_Image_Cb call, void* data, int x,int y, int w, int h, int delta=1) FL_OVERRIDE;

  void ps_origin(int x, int y);
  void ps_translate(int, int);
  void ps_untranslate();

  void draw_cached_pattern_(Fl_Image *img, cairo_pattern_t *pat, int X, int Y, int W, int H, int cx, int cy, int cache_w, int cache_h);
  void draw_image(const uchar *data, int ix, int iy, int iw, int ih, int D, int LD) FL_OVERRIDE;
  void draw_rgb(Fl_RGB_Image *rgb,int XP, int YP, int WP, int HP, int cx, int cy) FL_OVERRIDE;
  void cache(Fl_RGB_Image *rgb) FL_OVERRIDE;
  void uncache(Fl_RGB_Image *img, fl_uintptr_t &id_, fl_uintptr_t &mask_) FL_OVERRIDE;
  void draw_fixed(Fl_Bitmap *bm,int XP, int YP, int WP, int HP, int cx, int cy) FL_OVERRIDE;
  static cairo_pattern_t *bitmap_to_pattern(Fl_Bitmap *bm, bool complement,
                                            cairo_surface_t **p_surface);
  void cache(Fl_Bitmap *img) FL_OVERRIDE;
  void delete_bitmask(fl_uintptr_t bm) FL_OVERRIDE;
  void cache(Fl_Pixmap *pxm) FL_OVERRIDE;
  void draw_fixed(Fl_Pixmap *rgb,int XP, int YP, int WP, int HP, int cx, int cy) FL_OVERRIDE;
  void uncache_pixmap(fl_uintptr_t p) FL_OVERRIDE;

  void font(Fl_Font fnum, Fl_Fontsize s) FL_OVERRIDE;
  Fl_Font font() FL_OVERRIDE { return Fl_Graphics_Driver::font(); }
  void draw(const char* s, int nBytes, int x, int y) FL_OVERRIDE { draw(s, nBytes, float(x), float(y)); }
  void draw(const char* s, int nBytes, float x, float y) FL_OVERRIDE;
  void draw(int angle, const char *str, int n, int x, int y) FL_OVERRIDE;
  void rtl_draw(const char* str, int n, int x, int y) FL_OVERRIDE;
  int height() FL_OVERRIDE;
  int descent() FL_OVERRIDE;
  double width(const char *str, int n) FL_OVERRIDE;
  double width(unsigned c) FL_OVERRIDE;
  void text_extents(const char* txt, int n, int& dx, int& dy, int& w, int& h) FL_OVERRIDE;
  PangoFontDescription* pango_font_description() FL_OVERRIDE {
    return ((Fl_Cairo_Font_Descriptor*)font_descriptor())->fontref;
  }
  static void init_built_in_fonts();
  Fl_Font set_fonts(const char* pattern_name) FL_OVERRIDE;
  const char *font_name(int num) FL_OVERRIDE;
  void font_name(int num, const char *name) FL_OVERRIDE;
  const char* get_font_name(Fl_Font fnum, int* ap) FL_OVERRIDE;
  int get_font_sizes(Fl_Font fnum, int*& sizep) FL_OVERRIDE;
  Fl_Region XRectangleRegion(int x, int y, int w, int h) FL_OVERRIDE;
  void XDestroyRegion(Fl_Region r) FL_OVERRIDE;
  void add_rectangle_to_region(Fl_Region r, int X, int Y, int W, int H) FL_OVERRIDE;
  char can_do_alpha_blending() FL_OVERRIDE;
  float override_scale() FL_OVERRIDE;
  void restore_scale(float) FL_OVERRIDE;
  void antialias(int state) FL_OVERRIDE;
  int antialias() FL_OVERRIDE;
  void focus_rect(int x, int y, int w, int h) FL_OVERRIDE;
};

#endif // FL_CAIRO_GRAPHICS_DRIVER_H
