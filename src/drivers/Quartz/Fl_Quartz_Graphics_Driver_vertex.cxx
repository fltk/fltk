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

#include "../../config_lib.h"
#ifdef FL_CFG_GFX_QUARTZ

/**
  \file quartz_vertex.cxx
  \brief  Portable drawing code for drawing arbitrary shapes with
          simple 2D transformations, implemented for OS X Quartz.
*/

#include "Fl_Quartz_Graphics_Driver.h"

#include <FL/fl_draw.H>
#include <FL/x.H>
#include <FL/math.h>


void Fl_Quartz_Graphics_Driver::transformed_vertex(double xf, double yf) {
  transformed_vertex0(float(xf), float(yf));
}

void Fl_Quartz_Graphics_Driver::vertex(double x,double y) {
  transformed_vertex0(float(x*m.a + y*m.c + m.x), float(x*m.b + y*m.d + m.y));
}

void Fl_Quartz_Graphics_Driver::end_points() {
  if (quartz_line_width_ > 1.5f) CGContextSetShouldAntialias(gc_, true);
  for (int i=0; i<n; i++) { 
    CGContextMoveToPoint(gc_, p[i].x, p[i].y);
    CGContextAddLineToPoint(gc_, p[i].x, p[i].y);
    CGContextStrokePath(gc_);
  }
  if (quartz_line_width_ > 1.5f) CGContextSetShouldAntialias(gc_, false);
}

void Fl_Quartz_Graphics_Driver::end_line() {
  if (n < 2) {
    end_points();
    return;
  }
  if (n<=1) return;
  CGContextSetShouldAntialias(gc_, true);
  CGContextMoveToPoint(gc_, p[0].x, p[0].y);
  for (int i=1; i<n; i++)
    CGContextAddLineToPoint(gc_, p[i].x, p[i].y);
  CGContextStrokePath(gc_);
  CGContextSetShouldAntialias(gc_, false);
}

void Fl_Quartz_Graphics_Driver::end_loop() {
  fixloop();
  if (n>2) transformed_vertex((float)p[0].x, (float)p[0].y);
  end_line();
}

void Fl_Quartz_Graphics_Driver::end_polygon() {
  fixloop();
  if (n < 3) {
    end_line();
    return;
  }
  if (n<=1) return;
  CGContextSetShouldAntialias(gc_, true);
  CGContextMoveToPoint(gc_, p[0].x, p[0].y);
  for (int i=1; i<n; i++) 
    CGContextAddLineToPoint(gc_, p[i].x, p[i].y);
  CGContextClosePath(gc_);
  CGContextFillPath(gc_);
  CGContextSetShouldAntialias(gc_, false);
}

void Fl_Quartz_Graphics_Driver::begin_complex_polygon() {
  begin_polygon();
  gap_ = 0;
}

void Fl_Quartz_Graphics_Driver::gap() {
  while (n>gap_+2 && p[n-1].x == p[gap_].x && p[n-1].y == p[gap_].y) n--;
  if (n > gap_+2) {
    transformed_vertex((float)p[gap_].x, (float)p[gap_].y);
    gap_ = n;
  } else {
    n = gap_;
  }
}

void Fl_Quartz_Graphics_Driver::end_complex_polygon() {
  gap();
  if (n < 3) {
    end_line();
    return;
  }
  if (n<=1) return;
  CGContextSetShouldAntialias(gc_, true);
  CGContextMoveToPoint(gc_, p[0].x, p[0].y);
  for (int i=1; i<n; i++)
    CGContextAddLineToPoint(gc_, p[i].x, p[i].y);
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

void Fl_Quartz_Graphics_Driver::transformed_vertex0(float x, float y) {
  if (!n || x != p[n-1].x || y != p[n-1].y) {
   if (n >= p_size) {
   p_size = p ? 2*p_size : 16;
   p = (XPOINT*)realloc((void*)p, p_size*sizeof(*p));
   }
   p[n].x = x;
   p[n].y = y;
   n++;
   }
}

void Fl_Quartz_Graphics_Driver::fixloop() {  // remove equal points from closed path
  while (n>2 && p[n-1].x == p[0].x && p[n-1].y == p[0].y) n--;
}


#endif // FL_CFG_GFX_QUARTZ

//
// End of "$Id$".
//
