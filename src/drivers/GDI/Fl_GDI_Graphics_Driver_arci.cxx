//
// "$Id$"
//
// Arc (integer) drawing functions for the Fast Light Tool Kit (FLTK).
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

#ifndef FL_CFG_GFX_GDI_ARCI_CXX
#define FL_CFG_GFX_GDI_ARCI_CXX

/**
  \file gdi_arci.cxx
  \brief Utility functions for drawing circles using integers
*/

#include "Fl_GDI_Graphics_Driver.h"

#include <FL/math.h>

void Fl_GDI_Graphics_Driver::arc(int x,int y,int w,int h,double a1,double a2) {
  if (w <= 0 || h <= 0) return;
  int xa = x+w/2+int(w*cos(a1/180.0*M_PI));
  int ya = y+h/2-int(h*sin(a1/180.0*M_PI));
  int xb = x+w/2+int(w*cos(a2/180.0*M_PI));
  int yb = y+h/2-int(h*sin(a2/180.0*M_PI));
  if (fabs(a1 - a2) < 90) {
    if (xa == xb && ya == yb) SetPixel(fl_gc, xa, ya, fl_RGB());
    else Arc(fl_gc, x, y, x+w, y+h, xa, ya, xb, yb);
  } else Arc(fl_gc, x, y, x+w, y+h, xa, ya, xb, yb);
}

void Fl_GDI_Graphics_Driver::pie(int x,int y,int w,int h,double a1,double a2) {
  if (w <= 0 || h <= 0) return;
  if (a1 == a2) return;
  int xa = x+w/2+int(w*cos(a1/180.0*M_PI));
  int ya = y+h/2-int(h*sin(a1/180.0*M_PI));
  int xb = x+w/2+int(w*cos(a2/180.0*M_PI));
  int yb = y+h/2-int(h*sin(a2/180.0*M_PI));
  SelectObject(fl_gc, fl_brush());
  if (fabs(a1 - a2) < 90) {
    if (xa == xb && ya == yb) {
      MoveToEx(fl_gc, x+w/2, y+h/2, 0L); 
      LineTo(fl_gc, xa, ya);
      SetPixel(fl_gc, xa, ya, fl_RGB());
    } else Pie(fl_gc, x, y, x+w, y+h, xa, ya, xb, yb);
  } else Pie(fl_gc, x, y, x+w, y+h, xa, ya, xb, yb); 
}

#endif // FL_CFG_GFX_GDI_ARCI_CXX

//
// End of "$Id$".
//
