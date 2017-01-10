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
 \file Fl_OpenGL_Graphics_Driver.H
 \brief Definition of OpenGL graphics driver.
 */

#ifndef FL_CFG_GFX_OPENGL_H
#define FL_CFG_GFX_OPENGL_H

#include <FL/Fl_Graphics_Driver.H>


/**
 \brief OpenGL specific graphics class.
 */
class FL_EXPORT Fl_OpenGL_Graphics_Driver : public Fl_Graphics_Driver {
public:
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
  //void transformed_vertex0(double x, double y);
  void transformed_vertex(double xf, double yf);
  void vertex(double x, double y);
  void begin_points();
  void end_points();
  void begin_line();
  void end_line();
  void begin_loop();
  void end_loop();
  void begin_polygon();
  void end_polygon();
  void begin_complex_polygon();
  void gap();
  void end_complex_polygon();
  void fixloop();
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
  Fl_Color color() { return color_; }
  void color(uchar r, uchar g, uchar b);
  // --- implementation is in Fl_OpenGL_Graphics_Driver_font.cxx
  void font(Fl_Font face, Fl_Fontsize fsize);
  void draw(const char *str, int n, int x, int y);
  double width(const char *str, int n);
  void text_extents(const char*, int n, int& dx, int& dy, int& w, int& h);
  int height();
  int descent();
  // ---
  Fl_Bitmask create_bitmask(int w, int h, const uchar *array) { return 0L; }
  void delete_bitmask(Fl_Bitmask bm) { };
};


#endif // FL_CFG_GFX_OPENGL_H

//
// End of "$Id$".
//
