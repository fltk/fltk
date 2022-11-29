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
  \file Fl_Xlib_Graphics_Driver_vertex.cxx
  \brief  Portable drawing code for drawing arbitrary shapes with
  simple 2D transformations, implemented for X11 Xlib.
*/

#include <config.h>
#include "Fl_Xlib_Graphics_Driver.H"

#include <FL/fl_draw.H>
#include <FL/platform.H>
#include <FL/math.h>


void Fl_Xlib_Graphics_Driver::end_points() {
  if (n>1) XDrawPoints(fl_display, fl_window, gc_, short_point, n, 0);
}

void Fl_Xlib_Graphics_Driver::end_line() {
  if (n < 2) {
    end_points();
    return;
  }
  if (n>1) XDrawLines(fl_display, fl_window, gc_, short_point, n, 0);
}

void Fl_Xlib_Graphics_Driver::end_loop() {
  fixloop();
  if (n>2) {
    transformed_vertex0(short_point[0].x , short_point[0].y );
  }
  end_line();
}

void Fl_Xlib_Graphics_Driver::end_polygon() {
  fixloop();
  if (n < 3) {
    end_line();
    return;
  }
  if (n>2) XFillPolygon(fl_display, fl_window, gc_, short_point, n, Convex, 0);
}

void Fl_Xlib_Graphics_Driver::gap() {
  while (n>gap_+2 && short_point[n-1].x == short_point[gap_].x && short_point[n-1].y == short_point[gap_].y) n--;
  if (n > gap_+2) {
    transformed_vertex0(short_point[gap_].x, short_point[gap_].y);
    gap_ = n;
  } else {
    n = gap_;
  }
}

void Fl_Xlib_Graphics_Driver::end_complex_polygon() {
  gap();
  if (n < 3) {
    end_line();
    return;
  }
  if (n>2) XFillPolygon(fl_display, fl_window, gc_, short_point, n, 0, 0);
}

bool Fl_Xlib_Graphics_Driver::can_fill_non_convex_polygon() {
  return false;
}

// shortcut the closed circles so they use XDrawArc:
// warning: these do not draw rotated ellipses correctly!
// See fl_arc.c for portable version.
void Fl_Xlib_Graphics_Driver::ellipse_unscaled(double xt, double yt, double rx, double ry) {
  int llx = (int)rint(xt-rx);
  int w = (int)rint(xt+rx)-llx;
  int lly = (int)rint(yt-ry);
  int h = (int)rint(yt+ry)-lly;

  (what == POLYGON ? XFillArc : XDrawArc)
    (fl_display, fl_window, gc_, llx, lly, w, h, 0, 360*64);
}
