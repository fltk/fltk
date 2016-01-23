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

#ifndef FL_CFG_GFX_GDI_VERTEX_CXX
#define FL_CFG_GFX_GDI_VERTEX_CXX

/**
 \file gdi_vertex.cxx
 \brief  Portable drawing code for drawing arbitrary shapes with
 simple 2D transformations, implemented for MSWindows GDI.
 */

#include "gdi.H"

#include <FL/fl_draw.H>
#include <FL/x.H>
#include <FL/math.h>


void Fl_GDI_Graphics_Driver::transformed_vertex(double xf, double yf) {
  transformed_vertex0(COORD_T(rint(xf)), COORD_T(rint(yf)));
}

void Fl_GDI_Graphics_Driver::vertex(double x,double y) {
  transformed_vertex0(COORD_T(x*m.a + y*m.c + m.x), COORD_T(x*m.b + y*m.d + m.y));
}

void Fl_GDI_Graphics_Driver::end_points() {
  for (int i=0; i<n; i++) SetPixel(fl_gc, p[i].x, p[i].y, fl_RGB());
}

void Fl_GDI_Graphics_Driver::end_line() {
  if (n < 2) {
    end_points();
    return;
  }
  if (n>1) Polyline(fl_gc, p, n);
}

void Fl_GDI_Graphics_Driver::end_loop() {
  fixloop();
  if (n>2) transformed_vertex((COORD_T)p[0].x, (COORD_T)p[0].y);
  end_line();
}

void Fl_GDI_Graphics_Driver::end_polygon() {
  fixloop();
  if (n < 3) {
    end_line();
    return;
  }
  if (n>2) {
    SelectObject(fl_gc, fl_brush());
    Polygon(fl_gc, p, n);
  }
}

void Fl_GDI_Graphics_Driver::begin_complex_polygon() {
  begin_polygon();
  gap_ = 0;
  numcount = 0;
}

void Fl_GDI_Graphics_Driver::gap() {
  while (n>gap_+2 && p[n-1].x == p[gap_].x && p[n-1].y == p[gap_].y) n--;
  if (n > gap_+2) {
    transformed_vertex((COORD_T)p[gap_].x, (COORD_T)p[gap_].y);
    counts[numcount++] = n-gap_;
    gap_ = n;
  } else {
    n = gap_;
  }
}

void Fl_GDI_Graphics_Driver::end_complex_polygon() {
  gap();
  if (n < 3) {
    end_line();
    return;
  }
  if (n>2) {
    SelectObject(fl_gc, fl_brush());
    PolyPolygon(fl_gc, p, counts, numcount);
  }
}

// shortcut the closed circles so they use XDrawArc:
// warning: these do not draw rotated ellipses correctly!
// See fl_arc.c for portable version.

void Fl_GDI_Graphics_Driver::circle(double x, double y,double r) {
  double xt = transform_x(x,y);
  double yt = transform_y(x,y);
  double rx = r * (m.c ? sqrt(m.a*m.a+m.c*m.c) : fabs(m.a));
  double ry = r * (m.b ? sqrt(m.b*m.b+m.d*m.d) : fabs(m.d));
  int llx = (int)rint(xt-rx);
  int w = (int)rint(xt+rx)-llx;
  int lly = (int)rint(yt-ry);
  int h = (int)rint(yt+ry)-lly;

  if (what==POLYGON) {
    SelectObject(fl_gc, fl_brush());
    Pie(fl_gc, llx, lly, llx+w, lly+h, 0,0, 0,0); 
  } else
    Arc(fl_gc, llx, lly, llx+w, lly+h, 0,0, 0,0); 
}

#endif // FL_CFG_GFX_GDI_VERTEX_CXX

//
// End of "$Id$".
//
