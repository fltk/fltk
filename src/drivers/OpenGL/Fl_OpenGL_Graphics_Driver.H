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
  void point(int x, int y) FL_OVERRIDE;
  void rect(int x, int y, int w, int h) FL_OVERRIDE;
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
  void focus_rect(int x, int y, int w, int h) FL_OVERRIDE;
  // ---- clipping
  void push_clip(int x, int y, int w, int h) FL_OVERRIDE;
  void pop_clip() FL_OVERRIDE;
  void push_no_clip() FL_OVERRIDE;
  Fl_Region clip_region() FL_OVERRIDE;
  void clip_region(Fl_Region r) FL_OVERRIDE;
  void restore_clip() FL_OVERRIDE;
  int not_clipped(int x, int y, int w, int h) FL_OVERRIDE;
  int clip_box(int x, int y, int w, int h, int &X, int &Y, int &W, int &H) FL_OVERRIDE;
  // ---- matrix transformed drawing
  void transformed_vertex(double xf, double yf) FL_OVERRIDE;
  void begin_points() FL_OVERRIDE;
  void end_points() FL_OVERRIDE;
  void begin_line() FL_OVERRIDE;
  void end_line() FL_OVERRIDE;
  void begin_loop() FL_OVERRIDE;
  void end_loop() FL_OVERRIDE;
  void begin_polygon() FL_OVERRIDE;
  void end_polygon() FL_OVERRIDE;
  void begin_complex_polygon() FL_OVERRIDE;
  void gap() FL_OVERRIDE;
  void end_complex_polygon() FL_OVERRIDE;
  void fixloop() FL_OVERRIDE;
  void circle(double x, double y, double r) FL_OVERRIDE;
  void arc(int x, int y, int w, int h, double a1, double a2) FL_OVERRIDE;
  void arc(double x, double y, double r, double start, double end) FL_OVERRIDE;
  void pie(int x, int y, int w, int h, double a1, double a2) FL_OVERRIDE;
  void line_style(int style, int width=0, char* dashes=0) FL_OVERRIDE;
  void color(Fl_Color c) FL_OVERRIDE;
  Fl_Color color() FL_OVERRIDE { return color_; }
  void color(uchar r, uchar g, uchar b) FL_OVERRIDE;
  // --- implementation is in Fl_OpenGL_Graphics_Driver_font.cxx
  void font(Fl_Font face, Fl_Fontsize fsize) FL_OVERRIDE;
  Fl_Font font() FL_OVERRIDE;
  void draw(const char *str, int n, int x, int y) FL_OVERRIDE;
  void draw(const char *str, int n, float x, float y) FL_OVERRIDE;
  void draw(int angle, const char *str, int n, int x, int y) FL_OVERRIDE;
  double width(const char *str, int n) FL_OVERRIDE;
  double width(unsigned int c) FL_OVERRIDE;
  void text_extents(const char*, int n, int& dx, int& dy, int& w, int& h) FL_OVERRIDE;
  int height() FL_OVERRIDE;
  int descent() FL_OVERRIDE;
};

#endif // FL_OPENGL_GRAPHICS_DRIVER_H
