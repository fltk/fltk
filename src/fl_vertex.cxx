//
// "$Id$"
//
// Portable drawing routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2017 by Bill Spitzak and others.
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
  \file fl_vertex.cxx
  \brief  Portable drawing code for drawing arbitrary shapes with
          simple 2D transformations.
*/

// Portable code for drawing arbitrary shapes with simple 2D transformations.
// See also fl_arc.cxx

// matt: the Quartz implementation purposely doesn't use the Quartz matrix
//       operations for reasons of compatibility and maintainability

// -----------------------------------------------------------------------------
// all driver code is now in drivers/XXX/Fl_XXX_Graphics_Driver_xyz.cxx
// -----------------------------------------------------------------------------

#include <FL/Fl_Graphics_Driver.H>
#include <FL/Fl.H>
#include <FL/math.h>
#include <stdlib.h>

/**
 \cond DriverDev
 \addtogroup DriverDeveloper
 \{
 */

/** see fl_push_matrix() */
void Fl_Graphics_Driver::push_matrix() {
  if (sptr==matrix_stack_size)
    Fl::error("fl_push_matrix(): matrix stack overflow.");
  else
    stack[sptr++] = m;
}

/** see fl_pop_matrix() */
void Fl_Graphics_Driver::pop_matrix() {
  if (sptr==0)
    Fl::error("fl_pop_matrix(): matrix stack underflow.");
  else
    m = stack[--sptr];
}

/** see fl_mult_matrix() */
void Fl_Graphics_Driver::mult_matrix(double a, double b, double c, double d, double x, double y) {
  matrix o;
  o.a = a*m.a + b*m.c;
  o.b = a*m.b + b*m.d;
  o.c = c*m.a + d*m.c;
  o.d = c*m.b + d*m.d;
  o.x = x*m.a + y*m.c + m.x;
  o.y = x*m.b + y*m.d + m.y;
  m = o;
}

/** see fl_rotate() */
void Fl_Graphics_Driver::rotate(double d) {
  if (d) {
    double s, c;
    if (d == 0) {s = 0; c = 1;}
    else if (d == 90) {s = 1; c = 0;}
    else if (d == 180) {s = 0; c = -1;}
    else if (d == 270 || d == -90) {s = -1; c = 0;}
    else {s = sin(d*M_PI/180); c = cos(d*M_PI/180);}
    mult_matrix(c,-s,s,c,0,0);
  }
}

/** see fl_translate() */
void Fl_Graphics_Driver::translate(double x,double y) {
  mult_matrix(1,0,0,1,x,y);
}

/** see fl_begin_points() */
void Fl_Graphics_Driver::begin_points() {
  n = 0;
  what = POINT_;
}

/** see fl_begin_line() */
void Fl_Graphics_Driver::begin_line() {
  n = 0;
  what = LINE;
}

/** see fl_begin_loop() */
void Fl_Graphics_Driver::begin_loop() {
  n = 0;
  what = LOOP;
}

/** see fl_begin_polygon() */
void Fl_Graphics_Driver::begin_polygon() {
  n = 0;
  what = POLYGON;
}

/** see fl_transform_x() */
double Fl_Graphics_Driver::transform_x(double x, double y) {
  return x*m.a + y*m.c + m.x;
}

/** see fl_transform_y() */
double Fl_Graphics_Driver::transform_y(double x, double y) {
  return x*m.b + y*m.d + m.y;
}

/** see fl_transform_dx() */
double Fl_Graphics_Driver::transform_dx(double x, double y) {
  return x*m.a + y*m.c;
}

/** see fl_transform_dy() */
double Fl_Graphics_Driver::transform_dy(double x, double y) {
  return x*m.b + y*m.d;
}

/**
 \}
 \endcond
 */

//
// End of "$Id$".
//
