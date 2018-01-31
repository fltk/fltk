//
// "$Id$"
//
// Portable drawing routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2018 by Bill Spitzak and others.
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
  \file Fl_GDI_Graphics_Driver_vertex.cxx

  \brief  Portable drawing code for drawing arbitrary shapes with
	  simple 2D transformations, implemented for MSWindows GDI.
*/

#include "Fl_GDI_Graphics_Driver.H"

#include <FL/fl_draw.H>
#include <FL/platform.H>
#include <FL/math.h>


void Fl_GDI_Graphics_Driver::end_points() {
  for (int i=0; i<n; i++) SetPixel(gc_, p[i].x, p[i].y, fl_RGB());
}

void Fl_GDI_Graphics_Driver::end_line() {
  if (n < 2) {
    end_points();
    return;
  }
  if (n>1) Polyline(gc_, p, n);
}

void Fl_GDI_Graphics_Driver::end_loop() {
  fixloop();
  if (n>2) transformed_vertex0(p[0].x, p[0].y);
  end_line();
}

void Fl_GDI_Graphics_Driver::end_polygon() {
  fixloop();
  if (n < 3) {
    end_line();
    return;
  }
  if (n>2) {
    SelectObject(gc_, fl_brush());
    Polygon(gc_, p, n);
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
    transformed_vertex0(p[gap_].x, p[gap_].y);
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
    SelectObject(gc_, fl_brush());
    PolyPolygon(gc_, p, counts, numcount);
  }
}

void Fl_GDI_Graphics_Driver::ellipse_unscaled(double xt, double yt, double rx, double ry) {
  int llx = (int)rint(xt-rx);
  int w = (int)rint(xt+rx)-llx;
  int lly = (int)rint(yt-ry);
  int h = (int)rint(yt+ry)-lly;

  if (what==POLYGON) {
    SelectObject(gc_, fl_brush());
    Pie(gc_, llx, lly, llx+w, lly+h, 0,0, 0,0);
  } else
    Arc(gc_, llx, lly, llx+w, lly+h, 0,0, 0,0); 
}

//
// End of "$Id$".
//
