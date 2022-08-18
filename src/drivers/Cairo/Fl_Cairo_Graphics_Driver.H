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
  FL_EXPORT ~Fl_Cairo_Font_Descriptor();
  PangoFontDescription *fontref;
  int **width; // array of arrays of character widths
  int line_height;
};


class FL_EXPORT Fl_Cairo_Graphics_Driver : public Fl_Graphics_Driver {
private:
  bool *needs_commit_tag_; // NULL or points to whether cairo surface was drawn to
  cairo_t *dummy_cairo_; // used to measure text width before showing a window
  int linestyle_;
  int width_unscaled_(unsigned int c);
protected:
  cairo_t *cairo_;
  PangoContext *pango_context_;
  PangoLayout *pango_layout_;
public:
  Fl_Cairo_Graphics_Driver();
  virtual ~Fl_Cairo_Graphics_Driver();
  enum SHAPE {NONE=0, LINE, LOOP, POLYGON, POINTS};

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

  void check_status(void);

  enum SHAPE shape_;
  unsigned char cr_,cg_,cb_;
  char  linedash_[256];//should be enough
  void concat();  // transform ror scalable dradings...
  void reconcat(); //invert
  void recover(); //recovers the state after grestore (such as line styles...)
  void reset();

  float scale_x;
  float scale_y;
  float angle;
  int left_margin;
  int top_margin;
  static const cairo_format_t cairo_format;

  void surface_needs_commit() {
    if (needs_commit_tag_) *needs_commit_tag_ = true;
  }
  void needs_commit_tag(bool *tag) { needs_commit_tag_ = tag; }

  // implementation of drawing methods
  void color(Fl_Color c);
  void color(uchar r, uchar g, uchar b);
  Fl_Color color();

  void push_clip(int x, int y, int w, int h);
  void push_no_clip();
  void pop_clip();

  void line_style(int style, int width=0, char* dashes=0);

  void rect(int x, int y, int w, int h);
  void rectf(int x, int y, int w, int h);

  void xyline(int x, int y, int x1);
  void xyline(int x, int y, int x1, int y2);
  void xyline(int x, int y, int x1, int y2, int x3);

  void yxline(int x, int y, int y1);
  void yxline(int x, int y, int y1, int x2);
  void yxline(int x, int y, int y1, int x2, int y3);

  void line(int x1, int y1, int x2, int y2);
  void line(int x1, int y1, int x2, int y2, int x3, int y3);

  void loop(int x0, int y0, int x1, int y1, int x2, int y2);
  void loop(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3);
  void polygon(int x0, int y0, int x1, int y1, int x2, int y2);
  void polygon(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3);
  void point(int x, int y);
  void overlay_rect(int x, int y, int w , int h);
  int clip_box(int x, int y, int w, int h, int &X, int &Y, int &W, int &H);
  void restore_clip();
  int not_clipped(int x, int y, int w, int h);

  void begin_points();
  void begin_line();
  void begin_loop();
  void begin_polygon();
  void vertex(double x, double y);
  void curve(double x, double y, double x1, double y1, double x2, double y2, double x3, double y3);
  void circle(double x, double y, double r);
  void arc(double x, double y, double r, double start, double a);
  void arc(int x, int y, int w, int h, double a1, double a2);
  void pie(int x, int y, int w, int h, double a1, double a2);
  void end_points();
  void end_line();
  void end_loop();
  void end_polygon();
  void begin_complex_polygon() { begin_polygon(); }
  void gap() { gap_ = 1; }
  void end_complex_polygon() { end_polygon(); }
  void transformed_vertex(double x, double y);

  void draw_image_mono(const uchar* d, int x,int y,int w,int h, int delta=1, int ld=0);
  void draw_image(Fl_Draw_Image_Cb call, void* data, int x,int y, int w, int h, int delta=3);
  void draw_image_mono(Fl_Draw_Image_Cb call, void* data, int x,int y, int w, int h, int delta=1);

  void ps_origin(int x, int y);
  void ps_translate(int, int);
  void ps_untranslate();

  void draw_cached_pattern_(Fl_Image *img, cairo_pattern_t *pat, int X, int Y, int W, int H, int cx, int cy);
  void draw_image(const uchar *data, int ix, int iy, int iw, int ih, int D, int LD);
  void draw_rgb(Fl_RGB_Image *rgb,int XP, int YP, int WP, int HP, int cx, int cy);
  void cache(Fl_RGB_Image *rgb);
  void uncache(Fl_RGB_Image *img, fl_uintptr_t &id_, fl_uintptr_t &mask_);
  void draw_bitmap(Fl_Bitmap *bm,int XP, int YP, int WP, int HP, int cx, int cy);
  static cairo_pattern_t *bitmap_to_pattern(Fl_Bitmap *bm, bool complement,
                                            cairo_surface_t **p_surface);
  void cache(Fl_Bitmap *img);
  virtual void delete_bitmask(fl_uintptr_t bm);
  void cache(Fl_Pixmap *pxm);
  void draw_pixmap(Fl_Pixmap *rgb,int XP, int YP, int WP, int HP, int cx, int cy);
  void uncache_pixmap(fl_uintptr_t p);

  void font(Fl_Font fnum, Fl_Fontsize s);
  Fl_Font font() { return Fl_Graphics_Driver::font(); }
  void draw(const char* s, int nBytes, int x, int y) { draw(s, nBytes, float(x), float(y)); }
  void draw(const char* s, int nBytes, float x, float y);
  void draw(int angle, const char *str, int n, int x, int y);
  void rtl_draw(const char* str, int n, int x, int y);
  int height();
  int descent();
  double width(const char *str, int n);
  double width(unsigned c);
  void text_extents(const char* txt, int n, int& dx, int& dy, int& w, int& h);
  virtual PangoFontDescription* pango_font_description() {
    return ((Fl_Cairo_Font_Descriptor*)font_descriptor())->fontref;
  }
  static void init_built_in_fonts();
  virtual Fl_Font set_fonts(const char* pattern_name);
  virtual const char *font_name(int num);
  virtual void font_name(int num, const char *name);
  virtual const char* get_font_name(Fl_Font fnum, int* ap);
  virtual int get_font_sizes(Fl_Font fnum, int*& sizep);
  virtual Fl_Region XRectangleRegion(int x, int y, int w, int h);
  virtual void XDestroyRegion(Fl_Region r);
  virtual void add_rectangle_to_region(Fl_Region r, int X, int Y, int W, int H);
  virtual void cache_size(Fl_Image *img, int &width, int &height);
  virtual char can_do_alpha_blending();
  virtual float override_scale();
  virtual void restore_scale(float);
  virtual void antialias(int state);
  virtual int antialias();
};

#endif // FL_CAIRO_GRAPHICS_DRIVER_H
