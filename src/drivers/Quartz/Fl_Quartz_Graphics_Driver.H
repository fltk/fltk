//
// "$Id$"
//
// Definition of Apple Quartz graphics driver
// for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2016 by Bill Spitzak and others.
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
 \file Fl_Quartz_Graphics_Driver.H
 \brief Definition of Apple Quartz graphics driver.
 */

#ifndef FL_QUARTZ_GRAPHICS_DRIVER_H
#define FL_QUARTZ_GRAPHICS_DRIVER_H

#include <FL/x.H>
#include <FL/Fl_Graphics_Driver.H>
#include <ApplicationServices/ApplicationServices.h>

// condition for the ATSU API to be available at compile time
#define HAS_ATSU (!defined(__LP64__) || !__LP64__) && MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_11

struct Fl_Fontdesc;

/**
 \brief The Mac OS X-specific graphics class.
 *
 This class is implemented only on the Mac OS X platform.
 */
class Fl_Quartz_Graphics_Driver : public Fl_Graphics_Driver {
  friend class Fl_Font_Descriptor;
protected:
  CGContextRef gc_;
  int p_size;
  typedef struct { float x; float y; } XPOINT;
  XPOINT *p;
  bool high_resolution_;
// protected constructor to ensure only derived classes are allocated
  Fl_Quartz_Graphics_Driver() : Fl_Graphics_Driver(), gc_(NULL), p_size(0), p(NULL) {
    high_resolution_ = false;
  }
public:
  static const int CoreText_threshold; // min Mac OS version for CoreText
  virtual ~Fl_Quartz_Graphics_Driver() { if (p) free(p); }
  virtual int has_feature(driver_feature mask) { return mask & NATIVE; }
  virtual void gc(void *ctxt) {if (ctxt != gc_) global_gc(); gc_ = (CGContextRef)ctxt; }
  virtual void *gc() {return gc_;}
  char can_do_alpha_blending();

  // --- bitmap stuff
  Fl_Bitmask create_bitmask(int w, int h, const uchar *array);
  void delete_bitmask(Fl_Bitmask bm);
  void draw(Fl_Pixmap *pxm, int XP, int YP, int WP, int HP, int cx, int cy);
  void draw(Fl_Bitmap *pxm, int XP, int YP, int WP, int HP, int cx, int cy);
  void draw(Fl_RGB_Image *img, int XP, int YP, int WP, int HP, int cx, int cy);
  int draw_scaled(Fl_Image *img, int XP, int YP, int WP, int HP);
  void draw_image(const uchar* buf, int X,int Y,int W,int H, int D=3, int L=0);
  void draw_image(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=3);
  void draw_image_mono(const uchar* buf, int X,int Y,int W,int H, int D=1, int L=0);
  void draw_image_mono(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=1);
  fl_uintptr_t cache(Fl_Pixmap *img, int w, int h, const char *const*array);
  fl_uintptr_t cache(Fl_Bitmap *img, int w, int h, const uchar *array);
  void uncache(Fl_RGB_Image *img, fl_uintptr_t &id_, fl_uintptr_t &mask_);
  void copy_offscreen(int x, int y, int w, int h, Fl_Offscreen pixmap, int srcx, int srcy);
  void draw_CGImage(CGImageRef cgimg, int x, int y, int w, int h, int srcx, int srcy, int sw, int sh);
  static CGRect fl_cgrectmake_cocoa(int x, int y, int w, int h);
  void add_rectangle_to_region(Fl_Region r, int x, int y, int w, int h);
  Fl_Region XRectangleRegion(int x, int y, int w, int h);
  void XDestroyRegion(Fl_Region r);
  void high_resolution(bool b) { high_resolution_ = b; }
protected:
  void transformed_vertex0(float x, float y);
  void fixloop();
  // --- implementation is in src/fl_rect.cxx which includes src/cfg_gfx/quartz_rect.cxx
  void point(int x, int y);
  void rect(int x, int y, int w, int h);
  void rectf(int x, int y, int w, int h);
  void line(int x, int y, int x1, int y1);
  void line(int x, int y, int x1, int y1, int x2, int y2);
  void xyline(int x, int y, int x1);
  void xyline(int x, int y, int x1, int y2);
  void xyline(int x, int y, int x1, int y2, int x3);
  void yxline(int x, int y, int y1);
  void yxline(int x, int y, int y1, int x2);
  void yxline(int x, int y, int y1, int x2, int y3);
  void loop(int x0, int y0, int x1, int y1, int x2, int y2);
  void loop(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3);
  void polygon(int x0, int y0, int x1, int y1, int x2, int y2);
  void polygon(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3);
  // --- clipping
  void push_clip(int x, int y, int w, int h);
  int clip_box(int x, int y, int w, int h, int &X, int &Y, int &W, int &H);
  int not_clipped(int x, int y, int w, int h);
  void push_no_clip();
  void pop_clip();
  void restore_clip();
  // --- implementation is in src/fl_vertex.cxx which includes src/cfg_gfx/xxx_rect.cxx
  void begin_complex_polygon();
  void transformed_vertex(double xf, double yf);
  void vertex(double x, double y);
  void end_points();
  void end_line();
  void end_loop();
  void end_polygon();
  void end_complex_polygon();
  void gap();
  void circle(double x, double y, double r);
  // --- implementation is in src/fl_arc.cxx which includes src/cfg_gfx/xxx_arc.cxx if needed
  // using void Fl_Graphics_Driver::arc(double x, double y, double r, double start, double end);
  // --- implementation is in src/fl_arci.cxx which includes src/cfg_gfx/xxx_arci.cxx
  void arc(int x, int y, int w, int h, double a1, double a2);
  void pie(int x, int y, int w, int h, double a1, double a2);
  // --- implementation is in src/fl_line_style.cxx which includes src/cfg_gfx/xxx_line_style.cxx
  void line_style(int style, int width=0, char* dashes=0);
  // --- implementation is in src/fl_color.cxx which includes src/cfg_gfx/xxx_color.cxx
  void color(Fl_Color c);
  void set_color(Fl_Color i, unsigned int c);
  Fl_Color color() { return color_; }
  void color(uchar r, uchar g, uchar b);
  // --- implementation is in src/fl_font.cxx which includes src/cfg_gfx/xxx_font.cxx
  void draw(const char *str, int n, int x, int y);
  void draw(const char *str, int n, float x, float y);
  void draw(int angle, const char *str, int n, int x, int y);
  void rtl_draw(const char *str, int n, int x, int y);
  void font(Fl_Font face, Fl_Fontsize fsize);
  double width(const char *str, int n);
  double width(unsigned int c);
  int height();
  int descent();
  virtual bool high_resolution() { return high_resolution_; }
  virtual void global_gc();
  inline Fl_Font_Descriptor *valid_font_descriptor();
  virtual const char* get_font_name(Fl_Font fnum, int* ap);
  virtual int get_font_sizes(Fl_Font fnum, int*& sizep);
  virtual unsigned font_desc_size();
  virtual const char *font_name(int num);
  virtual void font_name(int num, const char *name);
  // Next group of virtual functions have at least one alternative
  // CoreText- or ATSU-based implementation.
  virtual double width(const UniChar* txt, int n) {return 0;}
  virtual Fl_Font set_fonts(const char* xstarname) {return 0;}
  virtual void set_fontname_in_fontdesc(Fl_Fontdesc *f);
  virtual void descriptor_init(const char* name, Fl_Fontsize Size, Fl_Font_Descriptor *d) {}
  // end of function group
};

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
class Fl_CoreText_Graphics_Driver : public Fl_Quartz_Graphics_Driver {
  void text_extents(const char*, int n, int& dx, int& dy, int& w, int& h);
  virtual void draw(const char *str, int n, float x, float y);
  virtual double width(const UniChar* txt, int n);
  virtual Fl_Font set_fonts(const char* xstarname);
  virtual Fl_Fontdesc* calc_fl_fonts(void);
  virtual void set_fontname_in_fontdesc(Fl_Fontdesc *f);
  virtual void descriptor_init(const char* name, Fl_Fontsize Size, Fl_Font_Descriptor *d);
};
#endif

#ifdef HAS_ATSU
class Fl_ATSU_Graphics_Driver : public Fl_Quartz_Graphics_Driver {
  void text_extents(const char*, int n, int& dx, int& dy, int& w, int& h);
  virtual void draw(const char *str, int n, float x, float y);
  virtual double width(const UniChar* txt, int n);
  virtual Fl_Font set_fonts(const char* xstarname);
  virtual Fl_Fontdesc* calc_fl_fonts(void);
  virtual void descriptor_init(const char* name, Fl_Fontsize Size, Fl_Font_Descriptor *d);
};
#endif

extern float fl_quartz_line_width_;

#endif // FL_QUARTZ_GRAPHICS_DRIVER_H

//
// End of "$Id$".
//
