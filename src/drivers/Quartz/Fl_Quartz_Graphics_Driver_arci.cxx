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

#include "Fl_Quartz_Graphics_Driver.H"
#include <FL/platform.H>

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
