//
// Support for graphics output to PostScript file for the Fast Light Tool Kit (FLTK).
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

/** \file  Fl_Pango_PostScript_Graphics_Driver.H
 Declaration of class Fl_PostScript_Graphics_Driver.
*/

#ifndef FL_POSTSCRIPT_GRAPHICS_DRIVER_H
#define FL_POSTSCRIPT_GRAPHICS_DRIVER_H

#include <config.h>
#include <FL/Fl_PostScript.H>

#ifndef USE_PANGO
#define USE_PANGO 0
#endif

/**
 \cond DriverDev
 \addtogroup DriverDeveloper
 \{
 */

/**
 PostScript graphical backend.
 */
#if USE_PANGO

#include "../Cairo/Fl_Cairo_Graphics_Driver.H"

class Fl_PostScript_Graphics_Driver : public Fl_Cairo_Graphics_Driver {
public:
  FILE *output;
  Fl_PostScript_Close_Command close_cmd_;
  int page_format_;
  char *ps_filename_;
  int nPages;
  double pw_, ph_;

  Fl_PostScript_Graphics_Driver();
  ~Fl_PostScript_Graphics_Driver();
  void close_command(Fl_PostScript_Close_Command cmd){close_cmd_=cmd;}
  FILE * file() {return output;}
  inline void set_cairo(cairo_t *cr) { cairo_ = cr; }
  void page(double pw, double ph, int media = 0);
  void page(int format);
  int start_postscript (int pagecount, enum Fl_Paged_Device::Page_Format format, enum Fl_Paged_Device::Page_Layout layout);
  int start_eps(int width, int height);
  void draw_image(const uchar *data, int ix, int iy, int iw, int ih, int D, int LD) FL_OVERRIDE;
  void draw_image(Fl_Draw_Image_Cb call, void* data, int x,int y, int w, int h, int delta=3) FL_OVERRIDE {
    Fl_Cairo_Graphics_Driver::draw_image(call, data, x,y,w,h,delta);
  }
  void font(int f, int s) FL_OVERRIDE;
  Fl_Font font() FL_OVERRIDE;
  double width(const char *s, int n) FL_OVERRIDE;
  double width(unsigned u) FL_OVERRIDE;
  int height() FL_OVERRIDE;
  int descent() FL_OVERRIDE;
  void text_extents(const char *c, int n, int &dx, int &dy, int &w, int &h) FL_OVERRIDE;
  // draw text at width measured on display
  void transformed_draw(const char* s, int n, double x, double y);
  void draw(const char* s, int nBytes, int x, int y) FL_OVERRIDE {transformed_draw(s,nBytes,x,y); }
  void draw(const char* s, int nBytes, float x, float y) FL_OVERRIDE {transformed_draw(s,nBytes,x,y); }
  void draw(int angle, const char *str, int n, int x, int y) FL_OVERRIDE {Fl_Cairo_Graphics_Driver::draw(angle, str, n, x, y);}

  void point(int x, int y) FL_OVERRIDE;
  int not_clipped(int x, int y, int w, int h) FL_OVERRIDE;
  int clip_box(int x, int y, int w, int h, int &X, int &Y, int &W, int &H) FL_OVERRIDE;
  virtual int has_feature(driver_feature feature_mask) FL_OVERRIDE { return feature_mask & PRINTER; }
  // draw image classes without caching them
    void draw_rgb_bitmap_(Fl_Image *img,int XP, int YP, int WP, int HP, int cx, int cy);
    void draw_pixmap(Fl_Pixmap * pxm,int XP, int YP, int WP, int HP, int cx, int cy) FL_OVERRIDE;
    void draw_bitmap(Fl_Bitmap * bitmap,int XP, int YP, int WP, int HP, int cx, int cy) FL_OVERRIDE;
    void draw_rgb(Fl_RGB_Image * rgb,int XP, int YP, int WP, int HP, int cx, int cy) FL_OVERRIDE;
};

#else // ! USE_PANGO

class Fl_PostScript_Graphics_Driver : public Fl_Graphics_Driver {
private:
  void transformed_draw_extra(const char* str, int n, double x, double y, int w, bool rtl);
  void *prepare_rle85();
  void write_rle85(uchar b, void *data);
  void close_rle85(void *data);
  void *prepare85();
  void write85(void *data, const uchar *p, int len);
  void close85(void *data);
  int scale_for_image_(Fl_Image *img, int XP, int YP, int WP, int HP,int cx, int cy);
protected:
  uchar **mask_bitmap() FL_OVERRIDE {return &mask;}
public:
  Fl_PostScript_Graphics_Driver();

  class Clip {
  public:
    int x, y, w, h;
    Clip *prev;
  };
  Clip * clip_;

  int lang_level_;
  int gap_;
  int pages_;
  int interpolate_; //interpolation of images
  uchar * mask;
  int mx; // width of mask;
  int my; // mask lines
  int page_policy_;
  void page_policy(int p);
  int page_policy() { return page_policy_; }
  int alpha_mask(const uchar * data, int w, int h, int D, int LD=0);
  /** Shields output PostScript data from modifications of the current locale.
   It typically avoids PostScript errors caused if the current locale uses comma instead of dot
   as "decimal point".
   \param format  directives controlling output PostScript data
   \return value returned by vfprintf() call
   */
  int clocale_printf(const char *format, ...);

  int linewidth_;// need for clipping, lang level 1-2
  int linestyle_;//
  unsigned char cr_,cg_,cb_;
  char  linedash_[256];//should be enough
  void concat();  // transform ror scalable dradings...
  void reconcat(); //invert
  void recover(); //recovers the state after grestore (such as line styles...)
  void reset();

  Fl_PostScript_Close_Command close_cmd_;
  int nPages;
  //int orientation_;

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
  void transformed_draw(const char* s, int n, double x, double y); //precise text placing
  //void transformed_draw(const char* s, double x, double y);

  int page_format_;
  char *ps_filename_;

  void close_command(Fl_PostScript_Close_Command cmd){close_cmd_=cmd;}
  FILE * file() {return output;}
  //void orientation (int o);
  //Fl_PostScript_Graphics_Driver(FILE *o, int lang_level, int pages = 0); // ps (also multi-page) constructor
  //void interpolate(int i){interpolate_=i;}
  //int interpolate(){return interpolate_;}

  void page(double pw, double ph, int media = 0);
  void page(int format);

  // implementation of drawing methods
  void color(Fl_Color c) FL_OVERRIDE;
  void color(uchar r, uchar g, uchar b) FL_OVERRIDE;
  Fl_Color color() FL_OVERRIDE;

  void push_clip(int x, int y, int w, int h) FL_OVERRIDE;
  int clip_box(int x, int y, int w, int h, int &X, int &Y, int &W, int &H) FL_OVERRIDE;
  int not_clipped(int x, int y, int w, int h) FL_OVERRIDE;
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

  void begin_points() FL_OVERRIDE;
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
  void begin_complex_polygon() FL_OVERRIDE {begin_polygon();}
  void gap() FL_OVERRIDE {gap_=1;}
  void end_complex_polygon() FL_OVERRIDE {end_polygon();}
  void transformed_vertex(double x, double y) FL_OVERRIDE;

  void draw_image(const uchar* d, int x,int y,int w,int h, int delta=3, int ldelta=0) FL_OVERRIDE;
  void draw_image_mono(const uchar* d, int x,int y,int w,int h, int delta=1, int ld=0) FL_OVERRIDE;
  void draw_image(Fl_Draw_Image_Cb call, void* data, int x,int y, int w, int h, int delta=3) FL_OVERRIDE;
  void draw_image_mono(Fl_Draw_Image_Cb call, void* data, int x,int y, int w, int h, int delta=1) FL_OVERRIDE;

  void draw(const char* s, int nBytes, int x, int y) FL_OVERRIDE {transformed_draw(s,nBytes,x,y); }
  void draw(const char* s, int nBytes, float x, float y) FL_OVERRIDE {transformed_draw(s,nBytes,x,y); }
  void draw(int angle, const char *str, int n, int x, int y) FL_OVERRIDE;
  void rtl_draw(const char* s, int n, int x, int y) FL_OVERRIDE;
  void font(int face, int size) FL_OVERRIDE;
  Fl_Font font() FL_OVERRIDE;
  double width(const char *, int) FL_OVERRIDE;
  double width(unsigned int u) FL_OVERRIDE;
  void text_extents(const char *c, int n, int &dx, int &dy, int &w, int &h) FL_OVERRIDE;
  int height() FL_OVERRIDE;
  int descent() FL_OVERRIDE;
  void draw_pixmap(Fl_Pixmap * pxm,int XP, int YP, int WP, int HP, int cx, int cy) FL_OVERRIDE;
  void draw_bitmap(Fl_Bitmap * bitmap,int XP, int YP, int WP, int HP, int cx, int cy) FL_OVERRIDE;
  void draw_rgb(Fl_RGB_Image * rgb,int XP, int YP, int WP, int HP, int cx, int cy) FL_OVERRIDE;
  ~Fl_PostScript_Graphics_Driver();
  int has_feature(driver_feature feature_mask) FL_OVERRIDE { return feature_mask & PRINTER; }

  void ps_origin(int x, int y);
  void ps_translate(int, int);
  void ps_untranslate();
};

#endif // USE_PANGO
/**
\}
\endcond
*/

#endif // FL_POSTSCRIPT_GRAPHICS_DRIVER_H
