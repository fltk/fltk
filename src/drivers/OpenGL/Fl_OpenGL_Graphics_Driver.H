//
// Definition of OpenGL graphics driver
// for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2016 by Bill Spitzak and others.
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
 \file Fl_OpenGL_Graphics_Driver.H
 \brief Definition of OpenGL graphics driver.
 */

#ifndef FL_OPENGL_GRAPHICS_DRIVER_H
#define FL_OPENGL_GRAPHICS_DRIVER_H

#include <FL/Fl_Graphics_Driver.H>
#include <FL/fl_draw.H>

/**
 \brief OpenGL specific graphics class.
 */
class Fl_OpenGL_Graphics_Driver : public Fl_Graphics_Driver {
public:
  float pixels_per_unit_;
  float line_width_;
  int line_stipple_;
  Fl_OpenGL_Graphics_Driver() :
  pixels_per_unit_(1.0f),
  line_width_(1.0f),
  line_stipple_(FL_SOLID) { }
  // --- line and polygon drawing with integer coordinates
  virtual void point(int x, int y) FL_OVERRIDE;
  virtual void rect(int x, int y, int w, int h) FL_OVERRIDE;
  virtual void rectf(int x, int y, int w, int h) FL_OVERRIDE;
  virtual void line(int x, int y, int x1, int y1) FL_OVERRIDE;
  virtual void line(int x, int y, int x1, int y1, int x2, int y2) FL_OVERRIDE;
  virtual void xyline(int x, int y, int x1) FL_OVERRIDE;
  virtual void xyline(int x, int y, int x1, int y2) FL_OVERRIDE;
  virtual void xyline(int x, int y, int x1, int y2, int x3) FL_OVERRIDE;
  virtual void yxline(int x, int y, int y1) FL_OVERRIDE;
  virtual void yxline(int x, int y, int y1, int x2) FL_OVERRIDE;
  virtual void yxline(int x, int y, int y1, int x2, int y3) FL_OVERRIDE;
  virtual void loop(int x0, int y0, int x1, int y1, int x2, int y2) FL_OVERRIDE;
  virtual void loop(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) FL_OVERRIDE;
  virtual void polygon(int x0, int y0, int x1, int y1, int x2, int y2) FL_OVERRIDE;
  virtual void polygon(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) FL_OVERRIDE;
  virtual void focus_rect(int x, int y, int w, int h) FL_OVERRIDE;
  // ---- clipping
  virtual void push_clip(int x, int y, int w, int h) FL_OVERRIDE;
  virtual void pop_clip() FL_OVERRIDE;
  virtual void push_no_clip() FL_OVERRIDE;
  virtual Fl_Region clip_region() FL_OVERRIDE;
  virtual void clip_region(Fl_Region r) FL_OVERRIDE;
  virtual void restore_clip() FL_OVERRIDE;
  virtual int not_clipped(int x, int y, int w, int h) FL_OVERRIDE;
  virtual int clip_box(int x, int y, int w, int h, int &X, int &Y, int &W, int &H) FL_OVERRIDE;
  // ---- matrix transformed drawing
  virtual void transformed_vertex(double xf, double yf) FL_OVERRIDE;
  virtual void begin_points() FL_OVERRIDE;
  virtual void end_points() FL_OVERRIDE;
  virtual void begin_line() FL_OVERRIDE;
  virtual void end_line() FL_OVERRIDE;
  virtual void begin_loop() FL_OVERRIDE;
  virtual void end_loop() FL_OVERRIDE;
  virtual void begin_polygon() FL_OVERRIDE;
  virtual void end_polygon() FL_OVERRIDE;
  virtual void begin_complex_polygon() FL_OVERRIDE;
  virtual void gap() FL_OVERRIDE;
  virtual void end_complex_polygon() FL_OVERRIDE;
  virtual void fixloop() FL_OVERRIDE;
  virtual void circle(double x, double y, double r) FL_OVERRIDE;
  virtual void arc(int x, int y, int w, int h, double a1, double a2) FL_OVERRIDE;
  virtual void arc(double x, double y, double r, double start, double end) FL_OVERRIDE;
  virtual void pie(int x, int y, int w, int h, double a1, double a2) FL_OVERRIDE;
  virtual void line_style(int style, int width=0, char* dashes=0) FL_OVERRIDE;
  virtual void color(Fl_Color c) FL_OVERRIDE;
  virtual Fl_Color color() FL_OVERRIDE { return color_; }
  virtual void color(uchar r, uchar g, uchar b) FL_OVERRIDE;
  // --- implementation is in Fl_OpenGL_Graphics_Driver_font.cxx
  virtual void font(Fl_Font face, Fl_Fontsize fsize) FL_OVERRIDE;
  virtual Fl_Font font() FL_OVERRIDE;
  virtual void draw(const char *str, int n, int x, int y) FL_OVERRIDE;
  virtual void draw(const char *str, int n, float x, float y) FL_OVERRIDE;
  virtual void draw(int angle, const char *str, int n, int x, int y) FL_OVERRIDE;
  virtual double width(const char *str, int n) FL_OVERRIDE;
  virtual double width(unsigned int c) FL_OVERRIDE;
  virtual void text_extents(const char*, int n, int& dx, int& dy, int& w, int& h) FL_OVERRIDE;
  virtual int height() FL_OVERRIDE;
  virtual int descent() FL_OVERRIDE;
};

#endif // FL_OPENGL_GRAPHICS_DRIVER_H
