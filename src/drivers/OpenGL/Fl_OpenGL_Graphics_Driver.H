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
  void point(int x, int y) override;
  void rect(int x, int y, int w, int h) override;
  void rectf(int x, int y, int w, int h) override;
  void line(int x, int y, int x1, int y1) override;
  void line(int x, int y, int x1, int y1, int x2, int y2) override;
  void xyline(int x, int y, int x1) override;
  void xyline(int x, int y, int x1, int y2) override;
  void xyline(int x, int y, int x1, int y2, int x3) override;
  void yxline(int x, int y, int y1) override;
  void yxline(int x, int y, int y1, int x2) override;
  void yxline(int x, int y, int y1, int x2, int y3) override;
  void loop(int x0, int y0, int x1, int y1, int x2, int y2) override;
  void loop(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) override;
  void polygon(int x0, int y0, int x1, int y1, int x2, int y2) override;
  void polygon(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) override;
  void focus_rect(int x, int y, int w, int h) override;
  // ---- clipping
  void push_clip(int x, int y, int w, int h) override;
  void pop_clip() override;
  void push_no_clip() override;
  Fl_Region clip_region() override;
  void clip_region(Fl_Region r) override;
  void restore_clip() override;
  int not_clipped(int x, int y, int w, int h) override;
  int clip_box(int x, int y, int w, int h, int &X, int &Y, int &W, int &H) override;
  // ---- matrix transformed drawing
  void transformed_vertex(double xf, double yf) override;
  void begin_points() override;
  void end_points() override;
  void begin_line() override;
  void end_line() override;
  void begin_loop() override;
  void end_loop() override;
  void begin_polygon() override;
  void end_polygon() override;
  void begin_complex_polygon() override;
  void gap() override;
  void end_complex_polygon() override;
  void fixloop() override;
  void circle(double x, double y, double r) override;
  void arc(int x, int y, int w, int h, double a1, double a2) override;
  void arc(double x, double y, double r, double start, double end) override;
  void pie(int x, int y, int w, int h, double a1, double a2) override;
  void line_style(int style, int width=0, char* dashes=0) override;
  void color(Fl_Color c) override;
  Fl_Color color() override { return color_; }
  void color(uchar r, uchar g, uchar b) override;
  // --- implementation is in Fl_OpenGL_Graphics_Driver_font.cxx
  void font(Fl_Font face, Fl_Fontsize fsize) override;
  Fl_Font font() override;
  void draw(const char *str, int n, int x, int y) override;
  void draw(const char *str, int n, float x, float y) override;
  void draw(int angle, const char *str, int n, int x, int y) override;
  double width(const char *str, int n) override;
  double width(unsigned int c) override;
  void text_extents(const char*, int n, int& dx, int& dy, int& w, int& h) override;
  int height() override;
  int descent() override;
};

#endif // FL_OPENGL_GRAPHICS_DRIVER_H
