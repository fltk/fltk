//
// Support for Cairo graphics for the Fast Light Tool Kit (FLTK).
//
// Copyright 2021 by Bill Spitzak and others.
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

typedef struct _cairo cairo_t;
typedef struct _PangoLayout  PangoLayout;
typedef struct _PangoFontDescription PangoFontDescription;

class FL_EXPORT Fl_Cairo_Graphics_Driver : public Fl_Graphics_Driver {
protected:
  cairo_t *cairo_;
  PangoLayout *pango_layout_;
  void draw_rgb_bitmap_(Fl_Image *img,int XP, int YP, int WP, int HP, int cx, int cy);
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

  int lang_level_;
  int gap_;
  int pages_;
  cairo_t *cr() { return cairo_; }
  PangoLayout *pango_layout() {return pango_layout_;};

  void check_status(void);

  enum SHAPE shape_;
  int linewidth_;// need for clipping, lang level 1-2
  int linestyle_;//
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

  double pw_, ph_;

  uchar bg_r, bg_g, bg_b;
  void transformed_draw(const char* s, int n, double x, double y); //precise text placing

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
  void begin_complex_polygon(){begin_polygon();};
  void gap(){gap_=1;};
  void end_complex_polygon(){end_polygon();};
  void transformed_vertex(double x, double y);

  void draw_image_mono(const uchar* d, int x,int y,int w,int h, int delta=1, int ld=0);
  void draw_image(Fl_Draw_Image_Cb call, void* data, int x,int y, int w, int h, int delta=3);
  void draw_image_mono(Fl_Draw_Image_Cb call, void* data, int x,int y, int w, int h, int delta=1);

  void draw(const char* s, int nBytes, int x, int y) {transformed_draw(s,nBytes,x,y); };
  void draw(const char* s, int nBytes, float x, float y) {transformed_draw(s,nBytes,x,y); };
  void draw(int angle, const char *str, int n, int x, int y);
  void rtl_draw(const char* s, int n, int x, int y);
  void draw_pixmap(Fl_Pixmap * pxm,int XP, int YP, int WP, int HP, int cx, int cy);
  void draw_bitmap(Fl_Bitmap * bitmap,int XP, int YP, int WP, int HP, int cx, int cy);
  void draw_rgb(Fl_RGB_Image * rgb,int XP, int YP, int WP, int HP, int cx, int cy);
  // ---
  Fl_Bitmask create_bitmask(int w, int h, const uchar *array) { return 0L; }

  void ps_origin(int x, int y);
  void ps_translate(int, int);
  void ps_untranslate();
};

#endif // FL_CAIRO_GRAPHICS_DRIVER_H
