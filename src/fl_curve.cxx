//
// Bézier curve functions for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
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
  \file fl_curve.cxx
  \brief Utility for drawing Bézier curves, adding the points to the
         current fl_begin/fl_vertex/fl_end path.

  Incremental math implementation:
  I very much doubt this is optimal!  From Foley/vanDam page 511.
  If anybody has a better algorithm, please send it!
*/

#include <FL/fl_draw.H>
#include <math.h>

/**
 \cond DriverDev
 \addtogroup DriverDeveloper
 \{
 */

/** see fl_curve() */
void Fl_Graphics_Driver::curve(double X0, double Y0,
              double X1, double Y1,
              double X2, double Y2,
              double X3, double Y3) {

  double x = fl_transform_x(X0,Y0);
  double y = fl_transform_y(X0,Y0);

  // draw point 0:
  fl_transformed_vertex(x,y);

  double x1 = fl_transform_x(X1,Y1);
  double y1 = fl_transform_y(X1,Y1);
  double x2 = fl_transform_x(X2,Y2);
  double y2 = fl_transform_y(X2,Y2);
  double x3 = fl_transform_x(X3,Y3);
  double y3 = fl_transform_y(X3,Y3);

  // find the area:
  double a = fabs((x-x2)*(y3-y1)-(y-y2)*(x3-x1));
  double b = fabs((x-x3)*(y2-y1)-(y-y3)*(x2-x1));
  if (b > a) a = b;

  // use that to guess at the number of segments:
  int nSeg = int(sqrt(a)/4);
  if (nSeg > 1) {
    if (nSeg > 100) nSeg = 100; // make huge curves not hang forever
    if (nSeg < 9) nSeg = 9; // make tiny curevs look bearable

    double e = 1.0/nSeg;

    // calculate the coefficients of 3rd order equation:
    double xa = (x3-3*x2+3*x1-x);
    double xb = 3*(x2-2*x1+x);
    double xc = 3*(x1-x);
    // calculate the forward differences:
    double dx1 = ((xa*e+xb)*e+xc)*e;
    double dx3 = 6*xa*e*e*e;
    double dx2 = dx3 + 2*xb*e*e;

    // calculate the coefficients of 3rd order equation:
    double ya = (y3-3*y2+3*y1-y);
    double yb = 3*(y2-2*y1+y);
    double yc = 3*(y1-y);
    // calculate the forward differences:
    double dy1 = ((ya*e+yb)*e+yc)*e;
    double dy3 = 6*ya*e*e*e;
    double dy2 = dy3 + 2*yb*e*e;

    // draw points 1 .. nSeg-2:
    for (int i=2; i<nSeg; i++) {
      x += dx1;
      dx1 += dx2;
      dx2 += dx3;
      y += dy1;
      dy1 += dy2;
      dy2 += dy3;
      fl_transformed_vertex(x,y);
    }

    // draw point nSeg-1:
    fl_transformed_vertex(x+dx1, y+dy1);
  }

  // draw point nSeg:
  fl_transformed_vertex(x3,y3);
}

/**
 \}
 \endcond
 */
