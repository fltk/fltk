//
// "$Id$"
//
// Portable drawing routines for the Fast Light Tool Kit (FLTK).
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

#ifndef FL_CFG_GFX_OPENGL_VERTEX_CXX
#define FL_CFG_GFX_OPENGL_VERTEX_CXX

/**
  \file Fl_OpenGL_Graphics_Driver_vertex.cxx
  \brief  Portable drawing code for drawing arbitrary shapes with
          simple 2D transformations, implemented for OpenGL.
*/

#include "Fl_OpenGL_Graphics_Driver.H"

#include <FL/fl_draw.H>
#include <FL/gl.h>
#include <FL/math.h>


// Event though there are faster versions of the functions in OpenGL,
// we use the default FLTK implementation for compatibility in the
// following functions.

// void Fl_OpenGL_Graphics_Driver::push_matrix()
// void Fl_OpenGL_Graphics_Driver::pop_matrix()
// void Fl_OpenGL_Graphics_Driver::mult_matrix(double a, double b, double c, double d, double x, double y)
// void Fl_OpenGL_Graphics_Driver::rotate(double d)
// double Fl_OpenGL_Graphics_Driver::transform_x(double x, double y)
// double Fl_OpenGL_Graphics_Driver::transform_y(double x, double y)
// double Fl_OpenGL_Graphics_Driver::transform_dx(double x, double y)
// double Fl_OpenGL_Graphics_Driver::transform_dy(double x, double y)

void Fl_OpenGL_Graphics_Driver::begin_points() {
  glBegin(GL_POINTS);
}

void Fl_OpenGL_Graphics_Driver::end_points() {
  glEnd();
}

void Fl_OpenGL_Graphics_Driver::begin_line() {
  glBegin(GL_LINE_STRIP);
}

void Fl_OpenGL_Graphics_Driver::end_line() {
  glEnd();
}

void Fl_OpenGL_Graphics_Driver::begin_loop() {
  glBegin(GL_LINE_LOOP);
}

void Fl_OpenGL_Graphics_Driver::end_loop() {
  glEnd();
}

void Fl_OpenGL_Graphics_Driver::begin_polygon() {
  glBegin(GL_POLYGON);
}

void Fl_OpenGL_Graphics_Driver::end_polygon() {
  glEnd();
}

void Fl_OpenGL_Graphics_Driver::begin_complex_polygon() {
  glBegin(GL_POLYGON);
}

void Fl_OpenGL_Graphics_Driver::gap() {
  glEnd();
  glBegin(GL_POLYGON);
}

// FXIME: non-convex polygons are not supported yet
// use gluTess* functions to do this; search for gluBeginPolygon
void Fl_OpenGL_Graphics_Driver::end_complex_polygon() {
  glEnd();
}

// remove equal points from closed path
void Fl_OpenGL_Graphics_Driver::fixloop() { }

void Fl_OpenGL_Graphics_Driver::transformed_vertex(double xf, double yf) {
  glVertex2d(xf, yf);
}

void Fl_OpenGL_Graphics_Driver::vertex(double x,double y) {
  transformed_vertex(x*m.a + y*m.c + m.x, x*m.b + y*m.d + m.y);
}

void Fl_OpenGL_Graphics_Driver::circle(double cx, double cy, double r) {
  double rx = r * (m.c ? sqrt(m.a*m.a+m.c*m.c) : fabs(m.a));
  double ry = r * (m.b ? sqrt(m.b*m.b+m.d*m.d) : fabs(m.d));
  double rMax;
  if (ry>rx) rMax = ry; else rMax = rx;

  // from http://slabode.exofire.net/circle_draw.shtml and many other places
  int num_segments = (int)(10 * sqrt(rMax))+1;
  double theta = 2 * M_PI / float(num_segments);
  double tangetial_factor = tan(theta);
  double radial_factor = cosf(theta);//calculate the radial factor
  double x = r; //we start at angle = 0
  double y = 0;

  glBegin(GL_LINE_LOOP);
  for(int ii = 0; ii < num_segments; ii++) {
    vertex(x + cx, y + cy); // output vertex
    double tx = -y;
    double ty = x;
    x += tx * tangetial_factor;
    y += ty * tangetial_factor;
    x *= radial_factor;
    y *= radial_factor;
  } 
  glEnd(); 

}

#endif // FL_CFG_GFX_OPENGL_VERTEX_CXX

//
// End of "$Id$".
//
