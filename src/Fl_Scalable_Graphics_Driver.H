//
// Declaration of Fl_Scalable_Graphics_Driver, Fl_Font_Descriptor, Fl_Fontdesc
// for the Fast Light Tool Kit (FLTK).
//
// Copyright 2025 by Bill Spitzak and others.
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

#ifndef FL_SCALABLE_GRAPHICS_DRIVER_H
#define FL_SCALABLE_GRAPHICS_DRIVER_H

#include <FL/Fl_Graphics_Driver.H>

#ifndef FL_DOXYGEN


/* Class Fl_Scalable_Graphics_Driver is platform-independent.
 It supports the scaling of all graphics coordinates by a
 float factor helpful to support HiDPI displays.
 This class does :
 - compute scaled coordinates
 - scale the cached offscreen of image objects
 - scale the pixel arrays used when performing direct image draws
 - call the member functions of a platform-specific,
 Fl_Scalable_Graphics_Driver-derived class that do the drawings with adequately
 scaled coordinates. These member functions are named with the _unscaled suffix.
 - scale and unscale the clipping region.

 This class is presently used by the X11 and Windows platforms.
 */
class FL_EXPORT Fl_Scalable_Graphics_Driver : public Fl_Graphics_Driver {
  Fl_Fontsize fontsize_; // scale-independent font size value
public:
  static int floor(int x, float s);
  inline int floor(int x) { return Fl_Scalable_Graphics_Driver::floor(x, scale()); }
protected:
  int line_width_;
  bool is_solid_;
  Fl_Scalable_Graphics_Driver();
  virtual Fl_Region scale_clip(float f);
  void unscale_clip(Fl_Region r);
  void point(int x, int y) FL_OVERRIDE;
  virtual void point_unscaled(float x, float y);
  void rect(int x, int y, int w, int h) FL_OVERRIDE;
  void rectf(int x, int y, int w, int h) FL_OVERRIDE;
  virtual void rect_unscaled(int x, int y, int w, int h);
  virtual void rectf_unscaled(int x, int y, int w, int h);
  void line(int x, int y, int x1, int y1) FL_OVERRIDE;
  virtual void line_unscaled(int x, int y, int x1, int y1);
  void line(int x, int y, int x1, int y1, int x2, int y2) FL_OVERRIDE;
  virtual void line_unscaled(int x, int y, int x1, int y1, int x2, int y2);
  void xyline(int x, int y, int x1) FL_OVERRIDE;
  virtual void xyline_unscaled(int x, int y, int x1);
  void xyline(int x, int y, int x1, int y2) FL_OVERRIDE {Fl_Graphics_Driver::xyline(x, y, x1, y2);}
  void xyline(int x, int y, int x1, int y2, int x3) FL_OVERRIDE {Fl_Graphics_Driver::xyline(x, y, x1, y2, x3);}
  void yxline(int x, int y, int y1) FL_OVERRIDE;
  virtual void yxline_unscaled(int x, int y, int y1);
  void yxline(int x, int y, int y1, int x2) FL_OVERRIDE {Fl_Graphics_Driver::yxline(x, y, y1, x2);}
  void yxline(int x, int y, int y1, int x2, int y3) FL_OVERRIDE {Fl_Graphics_Driver::yxline(x, y, y1, x2, y3);}
  void loop(int x0, int y0, int x1, int y1, int x2, int y2) FL_OVERRIDE;
  virtual void loop_unscaled(int x0, int y0, int x1, int y1, int x2, int y2);
  void loop(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) FL_OVERRIDE;
  virtual void loop_unscaled(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3);
  void polygon(int x0, int y0, int x1, int y1, int x2, int y2) FL_OVERRIDE;
  virtual void polygon_unscaled(int x0, int y0, int x1, int y1, int x2, int y2);
  void polygon(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) FL_OVERRIDE;
  virtual void polygon_unscaled(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3);
  void circle(double x, double y, double r) FL_OVERRIDE;
  virtual void ellipse_unscaled(double xt, double yt, double rx, double ry);
  void font(Fl_Font face, Fl_Fontsize size) FL_OVERRIDE;
  Fl_Font font() FL_OVERRIDE;
  virtual void font_unscaled(Fl_Font face, Fl_Fontsize size);
  double width(const char *str, int n) FL_OVERRIDE;
  double width(unsigned int c) FL_OVERRIDE;
  virtual double width_unscaled(const char *str, int n);
  virtual double width_unscaled(unsigned int c);
  Fl_Fontsize size() FL_OVERRIDE;
  virtual Fl_Fontsize size_unscaled();
  void text_extents(const char *str, int n, int &dx, int &dy, int &w, int &h) FL_OVERRIDE;
  virtual void text_extents_unscaled(const char *str, int n, int &dx, int &dy, int &w, int &h);
  int height() FL_OVERRIDE;
  int descent() FL_OVERRIDE;
  virtual int height_unscaled();
  virtual int descent_unscaled();
  void draw(const char *str, int n, int x, int y) FL_OVERRIDE;
  virtual void draw_unscaled(const char *str, int n, int x, int y);
  void draw(int angle, const char *str, int n, int x, int y) FL_OVERRIDE;
  virtual void draw_unscaled(int angle, const char *str, int n, int x, int y);
  void draw(const char *str, int nChars, float x, float y) FL_OVERRIDE;
  void rtl_draw(const char* str, int n, int x, int y) FL_OVERRIDE;
  virtual void rtl_draw_unscaled(const char* str, int n, int x, int y);
  void arc(double x, double y, double r, double start, double end) FL_OVERRIDE;
  void arc(int x, int y, int w, int h, double a1, double a2) FL_OVERRIDE;
  virtual void arc_unscaled(int x, int y, int w, int h, double a1, double a2);
  void pie(int x, int y, int w, int h, double a1, double a2) FL_OVERRIDE;
  virtual void pie_unscaled(int x, int y, int w, int h, double a1, double a2);
  void draw_circle(int x, int y, int d, Fl_Color c) FL_OVERRIDE;
  void line_style(int style, int width=0, char* dashes=0) FL_OVERRIDE;
  virtual void line_style_unscaled(int style, int width, char* dashes);
  void draw_image_rescale(void *buf, Fl_Draw_Image_Cb cb, int X, int Y, int W, int H, int D, int L, bool mono);
  virtual void draw_image_unscaled(const uchar* buf, int X,int Y,int W,int H, int D=3, int L=0);
  virtual void draw_image_unscaled(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=3);
  void draw_image(const uchar* buf, int X,int Y,int W,int H, int D=3, int L=0) FL_OVERRIDE;
  void draw_image(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=3) FL_OVERRIDE;
  virtual void draw_image_mono_unscaled(const uchar* buf, int x, int y, int w, int h, int d, int l);
  void draw_image_mono(const uchar* buf, int X,int Y,int W,int H, int D=1, int L=0) FL_OVERRIDE;
  virtual void draw_image_mono_unscaled(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=1);
  void draw_image_mono(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D=1) FL_OVERRIDE;

  void transformed_vertex(double xf, double yf) FL_OVERRIDE;
  void vertex(double x, double y) FL_OVERRIDE;
  float override_scale() FL_OVERRIDE;
  void restore_scale(float) FL_OVERRIDE;
  virtual void *change_pen_width(int lwidth);
  virtual void reset_pen_width(void *data);
};


/*
 Platforms usually define a derived class called Fl_XXX_Font_Descriptor
 containing extra platform-specific data/functions.
 This is a class for an actual system font, with junk to
 help choose it and info on character sizes.  Each Fl_Fontdesc has a
 linked list of these.  These are created the first time each system
 font/size combination is used.
 */
class Fl_Font_Descriptor {
public:
  /** linked list for this Fl_Fontdesc */
  Fl_Font_Descriptor *next;
  Fl_Fontsize size; /**< font size */
  Fl_Font_Descriptor(const char* fontname, Fl_Fontsize size);
  virtual FL_EXPORT ~Fl_Font_Descriptor() {}
  int ascent, descent;
  unsigned int listbase;// base of display list, 0 = none
};


struct Fl_Fontdesc {
  const char *name;
  char fontname[128];  // "Pretty" font name
  Fl_Font_Descriptor *first;  // linked list of sizes of this style
};

#endif // FL_DOXYGEN

#endif // FL_SCALABLE_GRAPHICS_DRIVER_H
