//
// Portable drawing routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2022 by Bill Spitzak and others.
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
  \file quartz_vertex.cxx
  \brief  Portable drawing code for drawing arbitrary shapes with
          simple 2D transformations, implemented for OS X Quartz.
*/

#include "Fl_Quartz_Graphics_Driver.H"

#include <FL/fl_draw.H>
#include <FL/platform.H>
#include <FL/math.h>


void Fl_Quartz_Graphics_Driver::end_points() {
  for (int i = 0; i < n; i++) {
    point(xpoint[i].x, xpoint[i].y);
  }
}

void Fl_Quartz_Graphics_Driver::end_line() {
  if (n < 2) {
    end_points();
    return;
  }
  if (n<=1) return;
  CGContextSetShouldAntialias(gc_, true);
  CGContextMoveToPoint(gc_, xpoint[0].x, xpoint[0].y);
  for (int i=1; i<n; i++)
    CGContextAddLineToPoint(gc_, xpoint[i].x, xpoint[i].y);
  CGContextStrokePath(gc_);
  CGContextSetShouldAntialias(gc_, false);
}

void Fl_Quartz_Graphics_Driver::end_polygon() {
  fixloop();
  if (n < 3) {
    end_line();
    return;
  }
  if (n<=1) return;
  CGContextSetShouldAntialias(gc_, true);
  CGContextMoveToPoint(gc_, xpoint[0].x, xpoint[0].y);
  for (int i=1; i<n; i++)
    CGContextAddLineToPoint(gc_, xpoint[i].x, xpoint[i].y);
  CGContextClosePath(gc_);
  CGContextFillPath(gc_);
  CGContextSetShouldAntialias(gc_, false);
}

void Fl_Quartz_Graphics_Driver::end_complex_polygon() {
  gap();
  if (n < 3) {
    end_line();
    return;
  }
  if (n<=1) return;
  CGContextSetShouldAntialias(gc_, true);
  CGContextMoveToPoint(gc_, xpoint[0].x, xpoint[0].y);
  for (int i=1; i<n; i++)
    CGContextAddLineToPoint(gc_, xpoint[i].x, xpoint[i].y);
  CGContextClosePath(gc_);
  CGContextFillPath(gc_);
  CGContextSetShouldAntialias(gc_, false);
}

void Fl_Quartz_Graphics_Driver::circle(double x, double y,double r) {
  double xt = transform_x(x,y);
  double yt = transform_y(x,y);
  double rx = r * (m.c ? sqrt(m.a*m.a+m.c*m.c) : fabs(m.a));
  double ry = r * (m.b ? sqrt(m.b*m.b+m.d*m.d) : fabs(m.d));
  int llx = (int)rint(xt-rx);
  int w = (int)rint(xt+rx)-llx;
  int lly = (int)rint(yt-ry);
  int h = (int)rint(yt+ry)-lly;

  // Quartz warning: circle won't scale to current matrix!
  // Last argument must be 0 (counter-clockwise) or it draws nothing under __LP64__ !!!!
  CGContextSetShouldAntialias(gc_, true);
  CGContextAddArc(gc_, xt, yt, (w+h)*0.25f, 0, 2.0f*M_PI, 0);
  (what == POLYGON ? CGContextFillPath : CGContextStrokePath)(gc_);
  CGContextSetShouldAntialias(gc_, false);
}
