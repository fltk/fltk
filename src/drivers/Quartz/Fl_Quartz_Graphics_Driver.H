//
// Definition of Apple Quartz graphics driver
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
 \file Fl_Quartz_Graphics_Driver.H
 \brief Definition of Apple Quartz graphics driver.
 */

#ifndef FL_QUARTZ_GRAPHICS_DRIVER_H
#define FL_QUARTZ_GRAPHICS_DRIVER_H

#include <FL/platform.H>
#include <FL/Fl_Graphics_Driver.H>
#include <ApplicationServices/ApplicationServices.h>

// condition for the ATSU API to be available at compile time
#if (    (!defined(__LP64__) || !__LP64__) \
      && MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_11 \
      && MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5 \
    )
#define HAS_ATSU 1
#else
#define HAS_ATSU 0
#endif

struct Fl_Fontdesc;
class Fl_Quartz_Font_Descriptor;

/**
 \brief The Mac OS X-specific graphics class.

 This class is implemented only on the Mac OS X platform.
 */
class Fl_Quartz_Graphics_Driver : public Fl_Graphics_Driver {
  friend class Fl_Cocoa_Printer_Driver;
  friend class Fl_Quartz_Font_Descriptor;
protected:
  CGContextRef gc_;
  bool high_resolution_;
  float quartz_line_width_;
  CGLineCap quartz_line_cap_;
  CGLineJoin quartz_line_join_;
  CGFloat *quartz_line_pattern;
  int quartz_line_pattern_size;
  void cache_size(Fl_Image* img, int &width, int &height) FL_OVERRIDE;
public:
  Fl_Quartz_Graphics_Driver();
  int has_feature(driver_feature mask) FL_OVERRIDE { return mask & NATIVE; }
  void gc(void *ctxt) FL_OVERRIDE { gc_ = (CGContextRef)ctxt; global_gc(); }
  void *gc() FL_OVERRIDE {return gc_;}
  char can_do_alpha_blending() FL_OVERRIDE;

  // --- bitmap stuff
  static CGImageRef create_bitmask(int w, int h, const uchar *array); // NOT virtual
  void delete_bitmask(fl_uintptr_t bm) FL_OVERRIDE;
  void draw_pixmap(Fl_Pixmap *pxm, int XP, int YP, int WP, int HP, int cx, int cy) FL_OVERRIDE;
  void draw_bitmap(Fl_Bitmap *pxm, int XP, int YP, int WP, int HP, int cx, int cy) FL_OVERRIDE;
  void draw_rgb(Fl_RGB_Image *img, int XP, int YP, int WP, int HP, int cx, int cy) FL_OVERRIDE;
  void draw_image(const uchar* buf, int X,int Y,int W,int H, int D=3, int L=0) FL_OVERRIDE;
  void draw_image(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=3) FL_OVERRIDE;
  void draw_image_mono(const uchar* buf, int X,int Y,int W,int H, int D=1, int L=0) FL_OVERRIDE;
  void draw_image_mono(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=1) FL_OVERRIDE;
  void cache(Fl_Pixmap *img) FL_OVERRIDE;
  void cache(Fl_Bitmap *img) FL_OVERRIDE;
  void cache(Fl_RGB_Image *img) FL_OVERRIDE;
  void uncache(Fl_RGB_Image *img, fl_uintptr_t &id_, fl_uintptr_t &mask_) FL_OVERRIDE;
  void copy_offscreen(int x, int y, int w, int h, Fl_Offscreen pixmap, int srcx, int srcy) FL_OVERRIDE;
  void draw_CGImage(CGImageRef cgimg, int x, int y, int w, int h, int srcx, int srcy, int sw, int sh);
  static CGRect fl_cgrectmake_cocoa(int x, int y, int w, int h);
  void add_rectangle_to_region(Fl_Region r, int x, int y, int w, int h) FL_OVERRIDE;
  Fl_Region XRectangleRegion(int x, int y, int w, int h) FL_OVERRIDE;
  void XDestroyRegion(Fl_Region r) FL_OVERRIDE;
  void high_resolution(bool b) { high_resolution_ = b; }
protected:
  void point(int x, int y) FL_OVERRIDE;
  void rect(int x, int y, int w, int h) FL_OVERRIDE;
  void focus_rect(int x, int y, int w, int h) FL_OVERRIDE;
  void rectf(int x, int y, int w, int h) FL_OVERRIDE;
  void line(int x, int y, int x1, int y1) FL_OVERRIDE;
  void line(int x, int y, int x1, int y1, int x2, int y2) FL_OVERRIDE;
  void xyline(int x, int y, int x1) FL_OVERRIDE;
  void xyline(int x, int y, int x1, int y2) FL_OVERRIDE;
  void xyline(int x, int y, int x1, int y2, int x3) FL_OVERRIDE;
  void yxline(int x, int y, int y1) FL_OVERRIDE;
  void yxline(int x, int y, int y1, int x2) FL_OVERRIDE;
  void yxline(int x, int y, int y1, int x2, int y3) FL_OVERRIDE;
  void loop(int x0, int y0, int x1, int y1, int x2, int y2) FL_OVERRIDE;
  void loop(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) FL_OVERRIDE;
  void polygon(int x0, int y0, int x1, int y1, int x2, int y2) FL_OVERRIDE;
  void polygon(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) FL_OVERRIDE;
  // --- clipping
  void push_clip(int x, int y, int w, int h) FL_OVERRIDE;
  int clip_box(int x, int y, int w, int h, int &X, int &Y, int &W, int &H) FL_OVERRIDE;
  int not_clipped(int x, int y, int w, int h) FL_OVERRIDE;
  void restore_clip() FL_OVERRIDE;
  void end_points() FL_OVERRIDE;
  void end_line() FL_OVERRIDE;
  void end_polygon() FL_OVERRIDE;
  void end_complex_polygon() FL_OVERRIDE;
  void circle(double x, double y, double r) FL_OVERRIDE;
  void arc(int x, int y, int w, int h, double a1, double a2) FL_OVERRIDE;
  void pie(int x, int y, int w, int h, double a1, double a2) FL_OVERRIDE;
  void line_style(int style, int width=0, char* dashes=0) FL_OVERRIDE;
  void color(Fl_Color c) FL_OVERRIDE;
  Fl_Color color() FL_OVERRIDE { return color_; }
  void color(uchar r, uchar g, uchar b) FL_OVERRIDE;
  void draw(const char *str, int n, int x, int y) FL_OVERRIDE;
  void draw(const char *str, int n, float x, float y) FL_OVERRIDE;
  void draw(int angle, const char *str, int n, int x, int y) FL_OVERRIDE;
  double width(const UniChar* txt, int n);
  void rtl_draw(const char *str, int n, int x, int y) FL_OVERRIDE;
  void font(Fl_Font face, Fl_Fontsize fsize) FL_OVERRIDE;
  double width(const char *str, int n) FL_OVERRIDE;
  double width(unsigned int c) FL_OVERRIDE;
  int height() FL_OVERRIDE;
  int descent() FL_OVERRIDE;
  virtual bool high_resolution() { return high_resolution_; }
  void global_gc() FL_OVERRIDE;
  void quartz_restore_line_style();
  inline Fl_Quartz_Font_Descriptor *valid_font_descriptor();
  const char* get_font_name(Fl_Font fnum, int* ap) FL_OVERRIDE;
  int get_font_sizes(Fl_Font fnum, int*& sizep) FL_OVERRIDE;
  const char *font_name(int num) FL_OVERRIDE;
  void font_name(int num, const char *name) FL_OVERRIDE;
  Fl_Fontdesc* calc_fl_fonts(void) FL_OVERRIDE;

  void text_extents(const char*, int n, int& dx, int& dy, int& w, int& h) FL_OVERRIDE;
  Fl_Font set_fonts(const char* xstarname) FL_OVERRIDE;
  void set_fontname_in_fontdesc(Fl_Fontdesc *f);
  void uncache_pixmap(fl_uintptr_t p) FL_OVERRIDE;

#if HAS_ATSU && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
  enum {use_CoreText = 1, use_ATSU};
  static int CoreText_or_ATSU;
  static void init_CoreText_or_ATSU();
  void draw_CoreText(const char *str, int n, float x, float y);
  double width_CoreText(const UniChar* txt, int n);
  void descriptor_init_CoreText(const char* name, Fl_Fontsize Size, Fl_Quartz_Font_Descriptor *d);
  void text_extents_CoreText(const char*, int n, int& dx, int& dy, int& w, int& h);
  Fl_Font set_fonts_CoreText(const char* xstarname);

  void draw_ATSU(const char *str, int n, float x, float y);
  double width_ATSU(const UniChar* txt, int n);
  void descriptor_init_ATSU(const char* name, Fl_Fontsize Size, Fl_Quartz_Font_Descriptor *d);
  void text_extents_ATSU(const char*, int n, int& dx, int& dy, int& w, int& h);
  Fl_Font set_fonts_ATSU(const char* xstarname);

  // define 2 kinds of pointers to member function
  typedef void (Fl_Quartz_Graphics_Driver::*pter_to_draw_member)(const char *str, int n, float x, float y);
  typedef double (Fl_Quartz_Graphics_Driver::*pter_to_width_member)(const UniChar *str, int n);
  static pter_to_draw_member CoreText_or_ATSU_draw;
  static pter_to_width_member CoreText_or_ATSU_width;
#else
  void descriptor_init(const char* name, Fl_Fontsize Size, Fl_Quartz_Font_Descriptor *d);
#endif
  void overlay_rect(int x, int y, int w , int h) FL_OVERRIDE;
  float override_scale() FL_OVERRIDE;
  void restore_scale(float) FL_OVERRIDE;
  void antialias(int state) FL_OVERRIDE;
  int antialias() FL_OVERRIDE;
};

class Fl_Quartz_Printer_Graphics_Driver : public Fl_Quartz_Graphics_Driver {
public:
  int has_feature(driver_feature mask) FL_OVERRIDE { return mask & (NATIVE | PRINTER); }
};

#endif // FL_QUARTZ_GRAPHICS_DRIVER_H
