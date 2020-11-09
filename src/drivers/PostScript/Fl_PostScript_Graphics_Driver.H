//
// Support for graphics output to PostScript file for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2020 by Bill Spitzak and others.
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

/** \file  Fl_Pango_PostScript_Graphics_Driver.H
 Declaration of class Fl_PostScript_Graphics_Driver.
*/

#ifndef FL_POSTSCRIPT_GRAPHICS_DRIVER_H
#define FL_POSTSCRIPT_GRAPHICS_DRIVER_H

#include "../../config_lib.h"
#include <FL/Fl_PostScript.H>

#ifndef USE_PANGO
#define USE_PANGO 0
#endif

#if USE_PANGO
typedef struct _cairo cairo_t;
typedef struct _PangoLayout  PangoLayout;
#endif

/**
 \cond DriverDev
 \addtogroup DriverDeveloper
 \{
 */

/**
 PostScript graphical backend.
 */

class FL_EXPORT Fl_PostScript_Graphics_Driver : public Fl_Graphics_Driver {
private:
#if USE_PANGO
  cairo_t *cairo_;
  PangoLayout *pango_layout_;
  void draw_rgb_bitmap_(Fl_Image *img,int XP, int YP, int WP, int HP, int cx, int cy);
#else
  void transformed_draw_extra(const char* str, int n, double x, double y, int w, bool rtl);
  void *prepare_rle85();
  void write_rle85(uchar b, void *data);
  void close_rle85(void *data);
  void *prepare85();
  void write85(void *data, const uchar *p, int len);
  void close85(void *data);
  int scale_for_image_(Fl_Image *img, int XP, int YP, int WP, int HP,int cx, int cy);
#endif
protected:
  uchar **mask_bitmap() {return &mask;}
public:
  Fl_PostScript_Graphics_Driver();
#ifndef FL_DOXYGEN
  enum SHAPE{NONE=0, LINE, LOOP, POLYGON, POINTS};

  class Clip {
  public:
    int x, y, w, h;
    Clip *prev;
  };
  Clip * clip_;

  int lang_level_;
  int gap_;
  int pages_;

  int shape_;
  int linewidth_;// need for clipping, lang level 1-2
  int linestyle_;//
  int interpolate_; //interpolation of images
  unsigned char cr_,cg_,cb_;
  char  linedash_[256];//should be enough
  void concat();  // transform ror scalable dradings...
  void reconcat(); //invert
  void recover(); //recovers the state after grestore (such as line styles...)
  void reset();

  uchar * mask;
  int mx; // width of mask;
  int my; // mask lines
  Fl_PostScript_Close_Command close_cmd_;
  int page_policy_;
  int nPages;
  int orientation_;

  float scale_x;
  float scale_y;
  float angle;
  int left_margin;
  int top_margin;

  FILE *output;
  double pw_, ph_;

  uchar bg_r, bg_g, bg_b;
  int start_postscript (int pagecount, enum Fl_Paged_Device::Page_Format format, enum Fl_Paged_Device::Page_Layout layout);
  int start_eps(int width, int height);
  /*  int alpha_mask(const uchar * data, int w, int h, int D, int LD=0);
   */
  void transformed_draw(const char* s, int n, double x, double y); //precise text placing
  void transformed_draw(const char* s, double x, double y);
  int alpha_mask(const uchar * data, int w, int h, int D, int LD=0);

  enum Fl_Paged_Device::Page_Format page_format_;
  char *ps_filename_;

  void page_policy(int p);
  int page_policy(){return page_policy_;};
  void close_command(Fl_PostScript_Close_Command cmd){close_cmd_=cmd;};
  FILE * file() {return output;};
  //void orientation (int o);
  //Fl_PostScript_Graphics_Driver(FILE *o, int lang_level, int pages = 0); // ps (also multi-page) constructor
  void interpolate(int i){interpolate_=i;};
  int interpolate(){return interpolate_;}

  void page(double pw, double ph, int media = 0);
  void page(int format);
#endif // FL_DOXYGEN

  // implementation of drawing methods
  void color(Fl_Color c);
  void color(uchar r, uchar g, uchar b);

  void push_clip(int x, int y, int w, int h);
  int clip_box(int x, int y, int w, int h, int &X, int &Y, int &W, int &H);
  int not_clipped(int x, int y, int w, int h);
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

  void draw_image(const uchar* d, int x,int y,int w,int h, int delta=3, int ldelta=0);
  void draw_image_mono(const uchar* d, int x,int y,int w,int h, int delta=1, int ld=0);
  void draw_image(Fl_Draw_Image_Cb call, void* data, int x,int y, int w, int h, int delta=3);
  void draw_image_mono(Fl_Draw_Image_Cb call, void* data, int x,int y, int w, int h, int delta=1);

  void draw(const char* s, int nBytes, int x, int y) {transformed_draw(s,nBytes,x,y); };
  void draw(const char* s, int nBytes, float x, float y) {transformed_draw(s,nBytes,x,y); };
  void draw(int angle, const char *str, int n, int x, int y);
  void rtl_draw(const char* s, int n, int x, int y);
  void font(int face, int size);
  double width(const char *, int);
  double width(unsigned int u);
  void text_extents(const char *c, int n, int &dx, int &dy, int &w, int &h);
  int height();
  int descent();
  void draw_pixmap(Fl_Pixmap * pxm,int XP, int YP, int WP, int HP, int cx, int cy);
  void draw_bitmap(Fl_Bitmap * bitmap,int XP, int YP, int WP, int HP, int cx, int cy);
  void draw_rgb(Fl_RGB_Image * rgb,int XP, int YP, int WP, int HP, int cx, int cy);
#if USE_PANGO
  cairo_t *cr() { return cairo_; }
  PangoLayout *pango_layout() {return pango_layout_;};
  void check_status(void);
#else
  /** Shields output PostScript data from modifications of the current locale.
   It typically avoids PostScript errors caused if the current locale uses comma instead of dot
   as "decimal point".
   \param format  directives controlling output PostScript data
   \return value returned by vfprintf() call
   */
  int clocale_printf(const char *format, ...);
#endif
  ~Fl_PostScript_Graphics_Driver();
  // ---
  Fl_Bitmask create_bitmask(int w, int h, const uchar *array) { return 0L; }
  virtual int has_feature(driver_feature feature_mask) { return feature_mask & PRINTER; }

  void ps_origin(int x, int y);
  void ps_translate(int, int);
  void ps_untranslate();
};

/**
\}
\endcond
*/

#endif // FL_POSTSCRIPT_GRAPHICS_DRIVER_H
