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

#include "Fl_Xlib_Graphics_Driver.H"
#include <FL/fl_draw.H>

/**
  \file Fl_Xlib_Graphics_Driver_arci.cxx
  \brief Utility functions for drawing circles using integers
*/

void Fl_Xlib_Graphics_Driver::arc(int x,int y,int w,int h,double a1,double a2) {
  if (w <= 0 || h <= 0) return;
  XDrawArc(fl_display, fl_window, gc_, x,y,w-1,h-1, int(a1*64),int((a2-a1)*64));
}

void Fl_Xlib_Graphics_Driver::pie(int x,int y,int w,int h,double a1,double a2) {
  if (w <= 0 || h <= 0) return;
  XDrawArc(fl_display, fl_window, gc_, x,y,w-1,h-1, int(a1*64),int((a2-a1)*64));
  XFillArc(fl_display, fl_window, gc_, x,y,w-1,h-1, int(a1*64),int((a2-a1)*64));
}

//
// End of "$Id$".
//
