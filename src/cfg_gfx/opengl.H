//
// "$Id$"
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
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

/**
 \file opengl.H
 \brief Definition of OpenGL graphics driver.
 */

#ifndef FL_CFG_GFX_OPENGL_H
#define FL_CFG_GFX_OPENGL_H

#include <FL/Fl_Device.H>


/**
 \brief OpenGL specific graphics class.
 */
class FL_EXPORT Fl_OpenGL_Graphics_Driver : public Fl_Graphics_Driver {
public:
  static const char *class_id;
  const char *class_name() {return class_id;};
  void draw(const char* str, int n, int x, int y);
  void color(Fl_Color c);
  void color(uchar r, uchar g, uchar b);

  // --- line and polygon drawing with integer coordinates
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
  void push_clip(int x, int y, int w, int h);
  int clip_box(int x, int y, int w, int h, int &X, int &Y, int &W, int &H);
  int not_clipped(int x, int y, int w, int h);
  void push_no_clip();
  void pop_clip();
  void restore_clip();
  // --- implementation is in src/fl_vertex.cxx which includes src/cfg_gfx/xxx_rect.cxx
//  void transformed_vertex0(COORD_T x, COORD_T y);
//  void fixloop();
//  void push_matrix();
//  void pop_matrix();
//  void mult_matrix(double a, double b, double c, double d, double x, double y);
//  void rotate(double d);
//  void begin_points();
//  void begin_line();
//  void begin_loop();
//  void begin_polygon();
//  void begin_complex_polygon();
//  double transform_x(double x, double y);
//  double transform_y(double x, double y);
//  double transform_dx(double x, double y);
//  double transform_dy(double x, double y);
//  void transformed_vertex(double xf, double yf);
//  void vertex(double x, double y);
//  void end_points();
//  void end_line();
//  void end_loop();
//  void end_polygon();
//  void end_complex_polygon();
//  void gap();
//  void circle(double x, double y, double r);
  void begin_complex_polygon() { }
  void transformed_vertex(double xf, double yf) { }
  void vertex(double x, double y) { }
  void end_points() { }
  void end_line() { }
  void end_loop() { }
  void end_polygon() { }
  void end_complex_polygon() { }
  void gap() { }
  void circle(double x, double y, double r) { }
};


#endif // FL_CFG_GFX_OPENGL_H

//
// End of "$Id$".
//
