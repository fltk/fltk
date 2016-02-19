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

#include "../../config_lib.h"
#ifdef FL_CFG_GFX_QUARTZ

#include "Fl_Quartz_Graphics_Driver.h"

/**
  \file quartz_arci.cxx
  \brief Utility functions for drawing circles using integers
*/

void Fl_Quartz_Graphics_Driver::arc(int x,int y,int w,int h,double a1,double a2) {
  if (w <= 0 || h <= 0) return;
  a1 = (-a1)/180.0f*M_PI; a2 = (-a2)/180.0f*M_PI;
  float cx = x + 0.5f*w - 0.5f, cy = y + 0.5f*h - 0.5f;
  CGContextSetShouldAntialias(gc_, true);
  if (w!=h) {
    CGContextSaveGState(gc_);
    CGContextTranslateCTM(gc_, cx, cy);
    CGContextScaleCTM(gc_, w-1.0f, h-1.0f);
    CGContextAddArc(gc_, 0, 0, 0.5, a1, a2, 1);
    CGContextRestoreGState(gc_);
  } else {
    float r = (w+h)*0.25f-0.5f;
    CGContextAddArc(gc_, cx, cy, r, a1, a2, 1);
  }
  CGContextStrokePath(gc_);
  CGContextSetShouldAntialias(gc_, false);
}

void Fl_Quartz_Graphics_Driver::pie(int x,int y,int w,int h,double a1,double a2) {
  if (w <= 0 || h <= 0) return;
  a1 = (-a1)/180.0f*M_PI; a2 = (-a2)/180.0f*M_PI;
  float cx = x + 0.5f*w - 0.5f, cy = y + 0.5f*h - 0.5f;
  CGContextSetShouldAntialias(gc_, true);
  if (w!=h) {
    CGContextSaveGState(gc_);
    CGContextTranslateCTM(gc_, cx, cy);
    CGContextScaleCTM(gc_, w, h);
    CGContextAddArc(gc_, 0, 0, 0.5, a1, a2, 1);
    CGContextAddLineToPoint(gc_, 0, 0);
    CGContextClosePath(gc_);
    CGContextRestoreGState(gc_);
  } else {
    float r = (w+h)*0.25f;
    CGContextAddArc(gc_, cx, cy, r, a1, a2, 1);
    CGContextAddLineToPoint(gc_, cx, cy);
    CGContextClosePath(gc_);
  }
  CGContextFillPath(gc_);
  CGContextSetShouldAntialias(gc_, false);
}

#endif // FL_CFG_GFX_QUARTZ

//
// End of "$Id$".
//
