//
// "$Id$"
//
// Rectangle drawing routines for the Fast Light Tool Kit (FLTK).
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


#ifndef FL_CFG_GFX_QUARTZ_CXX
#define FL_CFG_GFX_QUARTZ_CXX


/**
  \file quartz_rect.cxx
  \brief Apple Quartz specific line and polygon drawing with integer coordinates.
*/


#include "quartz.h"


extern float fl_quartz_line_width_;

// FIXME: the use of the macro below can be avoided by adding a specific class
//        for drawing to the prinitng context
#define USINGQUARTZPRINTER  (Fl_Surface_Device::surface()->class_name() == Fl_Printer::class_id)


// --- line and polygon drawing with integer coordinates

void Fl_Quartz_Graphics_Driver::point(int x, int y) {
  CGContextFillRect(fl_gc, CGRectMake(x - 0.5, y - 0.5, 1, 1) );
}

void Fl_Quartz_Graphics_Driver::rect(int x, int y, int w, int h) {
  if (w<=0 || h<=0) return;
  // FIXME: there should be a quartz graphics driver for the printer device that makes the USINGQUARTZPRINTER obsolete
  if ( (!USINGQUARTZPRINTER) && fl_quartz_line_width_ > 1.5f) CGContextSetShouldAntialias(fl_gc, true);
  CGRect rect = CGRectMake(x, y, w-1, h-1);
  CGContextStrokeRect(fl_gc, rect);
  if ( (!USINGQUARTZPRINTER) && fl_quartz_line_width_ > 1.5f) CGContextSetShouldAntialias(fl_gc, false);
}

void Fl_Quartz_Graphics_Driver::rectf(int x, int y, int w, int h) {
  if (w<=0 || h<=0) return;
  CGRect rect = CGRectMake(x - 0.5, y - 0.5, w , h);
  CGContextFillRect(fl_gc, rect);
}

void Fl_Quartz_Graphics_Driver::line(int x, int y, int x1, int y1) {
  if (fl_quartz_line_width_ > 1.5f) CGContextSetShouldAntialias(fl_gc, true);
  CGContextMoveToPoint(fl_gc, x, y);
  CGContextAddLineToPoint(fl_gc, x1, y1);
  CGContextStrokePath(fl_gc);
  if (fl_quartz_line_width_ > 1.5f) CGContextSetShouldAntialias(fl_gc, false);
}

void Fl_Quartz_Graphics_Driver::line(int x, int y, int x1, int y1, int x2, int y2) {
  if (fl_quartz_line_width_ > 1.5f) CGContextSetShouldAntialias(fl_gc, true);
  CGContextMoveToPoint(fl_gc, x, y);
  CGContextAddLineToPoint(fl_gc, x1, y1);
  CGContextAddLineToPoint(fl_gc, x2, y2);
  CGContextStrokePath(fl_gc);
  if (fl_quartz_line_width_ > 1.5f) CGContextSetShouldAntialias(fl_gc, false);
}

void Fl_Quartz_Graphics_Driver::xyline(int x, int y, int x1) {
  if (USINGQUARTZPRINTER || fl_quartz_line_width_ > 1.5f) CGContextSetShouldAntialias(fl_gc, true);
  CGContextMoveToPoint(fl_gc, x, y);
  CGContextAddLineToPoint(fl_gc, x1, y);
  CGContextStrokePath(fl_gc);
  if (Fl_Display_Device::high_resolution()) {
    /* On retina displays, all xyline() and yxline() functions produce lines that are half-unit
     (or one pixel) too short at both ends. This is corrected by filling at both ends rectangles
     of size one unit by line-width.
     */
    CGContextFillRect(fl_gc, CGRectMake(x-0.5 , y  - fl_quartz_line_width_/2, 1 , fl_quartz_line_width_));
    CGContextFillRect(fl_gc, CGRectMake(x1-0.5 , y  - fl_quartz_line_width_/2, 1 , fl_quartz_line_width_));
  }
  if (USINGQUARTZPRINTER || fl_quartz_line_width_ > 1.5f) CGContextSetShouldAntialias(fl_gc, false);
}

void Fl_Quartz_Graphics_Driver::xyline(int x, int y, int x1, int y2) {
  if (USINGQUARTZPRINTER || fl_quartz_line_width_ > 1.5f) CGContextSetShouldAntialias(fl_gc, true);
  CGContextMoveToPoint(fl_gc, x, y);
  CGContextAddLineToPoint(fl_gc, x1, y);
  CGContextAddLineToPoint(fl_gc, x1, y2);
  CGContextStrokePath(fl_gc);
  if (Fl_Display_Device::high_resolution()) {
    CGContextFillRect(fl_gc, CGRectMake(x-0.5, y  - fl_quartz_line_width_/2, 1 , fl_quartz_line_width_));
    CGContextFillRect(fl_gc, CGRectMake(x1  -  fl_quartz_line_width_/2, y2-0.5, fl_quartz_line_width_, 1));
  }
  if (USINGQUARTZPRINTER || fl_quartz_line_width_ > 1.5f) CGContextSetShouldAntialias(fl_gc, false);
}

void Fl_Quartz_Graphics_Driver::xyline(int x, int y, int x1, int y2, int x3) {
  if (USINGQUARTZPRINTER || fl_quartz_line_width_ > 1.5f) CGContextSetShouldAntialias(fl_gc, true);
  CGContextMoveToPoint(fl_gc, x, y);
  CGContextAddLineToPoint(fl_gc, x1, y);
  CGContextAddLineToPoint(fl_gc, x1, y2);
  CGContextAddLineToPoint(fl_gc, x3, y2);
  CGContextStrokePath(fl_gc);
  if (Fl_Display_Device::high_resolution()) {
    CGContextFillRect(fl_gc, CGRectMake(x-0.5, y  - fl_quartz_line_width_/2, 1 , fl_quartz_line_width_));
    CGContextFillRect(fl_gc, CGRectMake(x3-0.5, y2  - fl_quartz_line_width_/2, 1 , fl_quartz_line_width_));
  }
  if (USINGQUARTZPRINTER || fl_quartz_line_width_ > 1.5f) CGContextSetShouldAntialias(fl_gc, false);
}

void Fl_Quartz_Graphics_Driver::yxline(int x, int y, int y1) {
  if (USINGQUARTZPRINTER || fl_quartz_line_width_ > 1.5f) CGContextSetShouldAntialias(fl_gc, true);
  CGContextMoveToPoint(fl_gc, x, y);
  CGContextAddLineToPoint(fl_gc, x, y1);
  CGContextStrokePath(fl_gc);
  if (Fl_Display_Device::high_resolution()) {
    CGContextFillRect(fl_gc, CGRectMake(x  -  fl_quartz_line_width_/2, y-0.5, fl_quartz_line_width_, 1));
    CGContextFillRect(fl_gc, CGRectMake(x  -  fl_quartz_line_width_/2, y1-0.5, fl_quartz_line_width_, 1));
  }
  if (USINGQUARTZPRINTER || fl_quartz_line_width_ > 1.5f) CGContextSetShouldAntialias(fl_gc, false);
}

void Fl_Quartz_Graphics_Driver::yxline(int x, int y, int y1, int x2) {
  if (USINGQUARTZPRINTER || fl_quartz_line_width_ > 1.5f) CGContextSetShouldAntialias(fl_gc, true);
  CGContextMoveToPoint(fl_gc, x, y);
  CGContextAddLineToPoint(fl_gc, x, y1);
  CGContextAddLineToPoint(fl_gc, x2, y1);
  CGContextStrokePath(fl_gc);
  if (Fl_Display_Device::high_resolution()) {
    CGContextFillRect(fl_gc, CGRectMake(x  -  fl_quartz_line_width_/2, y-0.5, fl_quartz_line_width_, 1));
    CGContextFillRect(fl_gc, CGRectMake(x2-0.5, y1  - fl_quartz_line_width_/2, 1 , fl_quartz_line_width_));
  }
  if (USINGQUARTZPRINTER || fl_quartz_line_width_ > 1.5f) CGContextSetShouldAntialias(fl_gc, false);
}

void Fl_Quartz_Graphics_Driver::yxline(int x, int y, int y1, int x2, int y3) {
  if (USINGQUARTZPRINTER || fl_quartz_line_width_ > 1.5f) CGContextSetShouldAntialias(fl_gc, true);
  CGContextMoveToPoint(fl_gc, x, y);
  CGContextAddLineToPoint(fl_gc, x, y1);
  CGContextAddLineToPoint(fl_gc, x2, y1);
  CGContextAddLineToPoint(fl_gc, x2, y3);
  CGContextStrokePath(fl_gc);
  if (Fl_Display_Device::high_resolution()) {
    CGContextFillRect(fl_gc, CGRectMake(x  -  fl_quartz_line_width_/2, y-0.5, fl_quartz_line_width_, 1));
    CGContextFillRect(fl_gc, CGRectMake(x2  -  fl_quartz_line_width_/2, y3-0.5, fl_quartz_line_width_, 1));
  }
  if (USINGQUARTZPRINTER || fl_quartz_line_width_ > 1.5f) CGContextSetShouldAntialias(fl_gc, false);
}

void Fl_Quartz_Graphics_Driver::loop(int x, int y, int x1, int y1, int x2, int y2) {
  CGContextSetShouldAntialias(fl_gc, true);
  CGContextMoveToPoint(fl_gc, x, y);
  CGContextAddLineToPoint(fl_gc, x1, y1);
  CGContextAddLineToPoint(fl_gc, x2, y2);
  CGContextClosePath(fl_gc);
  CGContextStrokePath(fl_gc);
  CGContextSetShouldAntialias(fl_gc, false);
}

void Fl_Quartz_Graphics_Driver::loop(int x, int y, int x1, int y1, int x2, int y2, int x3, int y3) {
  CGContextSetShouldAntialias(fl_gc, true);
  CGContextMoveToPoint(fl_gc, x, y);
  CGContextAddLineToPoint(fl_gc, x1, y1);
  CGContextAddLineToPoint(fl_gc, x2, y2);
  CGContextAddLineToPoint(fl_gc, x3, y3);
  CGContextClosePath(fl_gc);
  CGContextStrokePath(fl_gc);
  CGContextSetShouldAntialias(fl_gc, false);
}

void Fl_Quartz_Graphics_Driver::polygon(int x, int y, int x1, int y1, int x2, int y2) {
  CGContextSetShouldAntialias(fl_gc, true);
  CGContextMoveToPoint(fl_gc, x, y);
  CGContextAddLineToPoint(fl_gc, x1, y1);
  CGContextAddLineToPoint(fl_gc, x2, y2);
  CGContextClosePath(fl_gc);
  CGContextFillPath(fl_gc);
  CGContextSetShouldAntialias(fl_gc, false);
}

void Fl_Quartz_Graphics_Driver::polygon(int x, int y, int x1, int y1, int x2, int y2, int x3, int y3) {
  CGContextSetShouldAntialias(fl_gc, true);
  CGContextMoveToPoint(fl_gc, x, y);
  CGContextAddLineToPoint(fl_gc, x1, y1);
  CGContextAddLineToPoint(fl_gc, x2, y2);
  CGContextAddLineToPoint(fl_gc, x3, y3);
  CGContextClosePath(fl_gc);
  CGContextFillPath(fl_gc);
  CGContextSetShouldAntialias(fl_gc, false);
}

// --- clipping

void Fl_Quartz_Graphics_Driver::push_clip(int x, int y, int w, int h) {
  Fl_Region r;
  if (w > 0 && h > 0) {
    r = XRectangleRegion(x,y,w,h);
    Fl_Region current = rstack[rstackptr];
    if (current) {
      XDestroyRegion(r);
      r = Fl_X::intersect_region_and_rect(current, x,y,w,h);
    }
  } else { // make empty clip region:
    r = XRectangleRegion(0,0,0,0);
  }
  if (rstackptr < region_stack_max) rstack[++rstackptr] = r;
  else Fl::warning("Fl_Quartz_Graphics_Driver::push_clip: clip stack overflow!\n");
  fl_restore_clip();
}

int Fl_Quartz_Graphics_Driver::clip_box(int x, int y, int w, int h, int& X, int& Y, int& W, int& H){
  X = x; Y = y; W = w; H = h;
  Fl_Region r = rstack[rstackptr];
  if (!r) return 0;
  CGRect arg = fl_cgrectmake_cocoa(x, y, w, h);
  CGRect u = CGRectMake(0,0,0,0);
  CGRect test;
  for (int i = 0; i < r->count; i++) {
    test = CGRectIntersection(r->rects[i], arg);
    if ( !CGRectIsEmpty(test) ) {
      if(CGRectIsEmpty(u)) u = test;
      else u = CGRectUnion(u, test);
    }
  }
  X = int(u.origin.x + 0.5); // reverse offset introduced by fl_cgrectmake_cocoa()
  Y = int(u.origin.y + 0.5);
  W = int(u.size.width + 0.5); // round to nearest integer
  H = int(u.size.height + 0.5);
  if (CGRectIsEmpty(u)) W = H = 0;
  return !CGRectEqualToRect(arg, u);
}

int Fl_Quartz_Graphics_Driver::not_clipped(int x, int y, int w, int h) {
  if (x+w <= 0 || y+h <= 0) return 0;
  Fl_Region r = rstack[rstackptr];
  if (!r) return 1;
  CGRect arg = fl_cgrectmake_cocoa(x, y, w, h);
  for (int i = 0; i < r->count; i++) {
    CGRect test = CGRectIntersection(r->rects[i], arg);
    if (!CGRectIsEmpty(test)) return 1;
  }
  return 0;
}

// make there be no clip (used by fl_begin_offscreen() only!)
void Fl_Quartz_Graphics_Driver::push_no_clip() {
  if (rstackptr < region_stack_max) rstack[++rstackptr] = 0;
  else Fl::warning("Fl_Quartz_Graphics_Driver::push_no_clip: clip stack overflow!\n");
  fl_restore_clip();
}

// pop back to previous clip:
void Fl_Quartz_Graphics_Driver::pop_clip() {
  if (rstackptr > 0) {
    Fl_Region oldr = rstack[rstackptr--];
    if (oldr) XDestroyRegion(oldr);
  } else Fl::warning("Fl_Quartz_Graphics_Driver::pop_clip: clip stack underflow!\n");
  fl_restore_clip();
}

void Fl_Quartz_Graphics_Driver::restore_clip() {
  fl_clip_state_number++;
  Fl_Region r = rstack[rstackptr];
  if ( fl_window || fl_gc ) { // clipping for a true window or an offscreen buffer
    Fl_X::q_clear_clipping();
    Fl_X::q_fill_context();//flip coords if bitmap context
                           //apply program clip
    if (r) {
      CGContextClipToRects(fl_gc, r->rects, r->count);
    }
  }
}


#endif

//
// End of "$Id$".
//
