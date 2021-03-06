//
// Arc (integer) drawing functions for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2018 by Bill Spitzak and others.
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
  \file Fl_GDI_Graphics_Driver_arci.cxx
  \brief Utility functions for drawing circles using integers
*/

// "integer" circle drawing functions.  These draw the limited
// circle types provided by X and NT graphics.  The advantage of
// these is that small ones draw quite nicely (probably due to stored
// hand-drawn bitmaps of small circles!) and may be implemented by
// hardware and thus are fast.

#include "Fl_GDI_Graphics_Driver.H"

#include <FL/math.h>
#include <FL/platform.H>

void Fl_GDI_Graphics_Driver::arc_unscaled(float x, float y, float w, float h, double a1, double a2) {
  if (w <= 0 || h <= 0) return;
  int xa = int( x+w/2+int(w*cos(a1/180.0*M_PI)) );
  int ya = int( y+h/2-int(h*sin(a1/180.0*M_PI)) );
  int xb = int( x+w/2+int(w*cos(a2/180.0*M_PI)) );
  int yb = int( y+h/2-int(h*sin(a2/180.0*M_PI)) );
  if (fabs(a1 - a2) < 90) {
    if (xa == xb && ya == yb) SetPixel(gc_, xa, ya, fl_RGB());
    else Arc(gc_, int(x), int(y), int(x+w), int(y+h), xa, ya, xb, yb);
  } else Arc(gc_, int(x), int(y), int(x+w), int(y+h), xa, ya, xb, yb);
}

void Fl_GDI_Graphics_Driver::pie_unscaled(float x, float y, float w, float h, double a1, double a2) {
  if (w <= 0 || h <= 0) return;
  if (a1 == a2) return;
  int xa = int( x+w/2+int(w*cos(a1/180.0*M_PI)) );
  int ya = int( y+h/2-int(h*sin(a1/180.0*M_PI)) );
  int xb = int( x+w/2+int(w*cos(a2/180.0*M_PI)) );
  int yb = int( y+h/2-int(h*sin(a2/180.0*M_PI)) );
  SelectObject(gc_, fl_brush());
  if (fabs(a1 - a2) < 90) {
    if (xa == xb && ya == yb) {
      MoveToEx(gc_, int(x+w/2), int(y+h/2), 0L);
      LineTo(gc_, xa, ya);
      SetPixel(gc_, xa, ya, fl_RGB());
    } else Pie(gc_, int(x), int(y), int(x+w), int(y+h), xa, ya, xb, yb);
  } else Pie(gc_, int(x), int(y), int(x+w), int(y+h), xa, ya, xb, yb);
}
