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


#include "../../config_lib.h"
#ifdef FL_CFG_GFX_QUARTZ

#include <FL/Fl.H>


/**
  \file quartz_rect.cxx
  \brief Apple Quartz specific line and polygon drawing with integer coordinates.
*/


#include "Fl_Quartz_Graphics_Driver.h"


extern float fl_quartz_line_width_;


// --- line and polygon drawing with integer coordinates

void Fl_Quartz_Graphics_Driver::point(int x, int y) {
  CGContextFillRect(gc, CGRectMake(x - 0.5, y - 0.5, 1, 1) );
}

void Fl_Quartz_Graphics_Driver::rect(int x, int y, int w, int h) {
  if (w<=0 || h<=0) return;
  if ( (!has_feature(PRINTER)) && fl_quartz_line_width_ > 1.5f) CGContextSetShouldAntialias(gc, true);
  CGRect rect = CGRectMake(x, y, w-1, h-1);
  CGContextStrokeRect(gc, rect);
  if ( (!has_feature(PRINTER)) && fl_quartz_line_width_ > 1.5f) CGContextSetShouldAntialias(gc, false);
}

void Fl_Quartz_Graphics_Driver::rectf(int x, int y, int w, int h) {
  if (w<=0 || h<=0) return;
  CGRect rect = CGRectMake(x - 0.5, y - 0.5, w , h);
  CGContextFillRect(gc, rect);
}

void Fl_Quartz_Graphics_Driver::line(int x, int y, int x1, int y1) {
  if (fl_quartz_line_width_ > 1.5f) CGContextSetShouldAntialias(gc, true);
  CGContextMoveToPoint(gc, x, y);
  CGContextAddLineToPoint(gc, x1, y1);
  CGContextStrokePath(gc);
  if (fl_quartz_line_width_ > 1.5f) CGContextSetShouldAntialias(gc, false);
}

void Fl_Quartz_Graphics_Driver::line(int x, int y, int x1, int y1, int x2, int y2) {
  if (fl_quartz_line_width_ > 1.5f) CGContextSetShouldAntialias(gc, true);
  CGContextMoveToPoint(gc, x, y);
  CGContextAddLineToPoint(gc, x1, y1);
  CGContextAddLineToPoint(gc, x2, y2);
  CGContextStrokePath(gc);
  if (fl_quartz_line_width_ > 1.5f) CGContextSetShouldAntialias(gc, false);
}

void Fl_Quartz_Graphics_Driver::xyline(int x, int y, int x1) {
  if (has_feature(PRINTER) || fl_quartz_line_width_ > 1.5f) CGContextSetShouldAntialias(gc, true);
  CGContextMoveToPoint(gc, x, y);
  CGContextAddLineToPoint(gc, x1, y);
  CGContextStrokePath(gc);
  if (Fl_Display_Device::high_resolution()) {
    /* On retina displays, all xyline() and yxline() functions produce lines that are half-unit
     (or one pixel) too short at both ends. This is corrected by filling at both ends rectangles
     of size one unit by line-width.
     */
    CGContextFillRect(gc, CGRectMake(x-0.5 , y  - fl_quartz_line_width_/2, 1 , fl_quartz_line_width_));
    CGContextFillRect(gc, CGRectMake(x1-0.5 , y  - fl_quartz_line_width_/2, 1 , fl_quartz_line_width_));
  }
  if (has_feature(PRINTER) || fl_quartz_line_width_ > 1.5f) CGContextSetShouldAntialias(gc, false);
}

void Fl_Quartz_Graphics_Driver::xyline(int x, int y, int x1, int y2) {
  if (has_feature(PRINTER) || fl_quartz_line_width_ > 1.5f) CGContextSetShouldAntialias(gc, true);
  CGContextMoveToPoint(gc, x, y);
  CGContextAddLineToPoint(gc, x1, y);
  CGContextAddLineToPoint(gc, x1, y2);
  CGContextStrokePath(gc);
  if (Fl_Display_Device::high_resolution()) {
    CGContextFillRect(gc, CGRectMake(x-0.5, y  - fl_quartz_line_width_/2, 1 , fl_quartz_line_width_));
    CGContextFillRect(gc, CGRectMake(x1  -  fl_quartz_line_width_/2, y2-0.5, fl_quartz_line_width_, 1));
  }
  if (has_feature(PRINTER) || fl_quartz_line_width_ > 1.5f) CGContextSetShouldAntialias(gc, false);
}

void Fl_Quartz_Graphics_Driver::xyline(int x, int y, int x1, int y2, int x3) {
  if (has_feature(PRINTER) || fl_quartz_line_width_ > 1.5f) CGContextSetShouldAntialias(gc, true);
  CGContextMoveToPoint(gc, x, y);
  CGContextAddLineToPoint(gc, x1, y);
  CGContextAddLineToPoint(gc, x1, y2);
  CGContextAddLineToPoint(gc, x3, y2);
  CGContextStrokePath(gc);
  if (Fl_Display_Device::high_resolution()) {
    CGContextFillRect(gc, CGRectMake(x-0.5, y  - fl_quartz_line_width_/2, 1 , fl_quartz_line_width_));
    CGContextFillRect(gc, CGRectMake(x3-0.5, y2  - fl_quartz_line_width_/2, 1 , fl_quartz_line_width_));
  }
  if (has_feature(PRINTER) || fl_quartz_line_width_ > 1.5f) CGContextSetShouldAntialias(gc, false);
}

void Fl_Quartz_Graphics_Driver::yxline(int x, int y, int y1) {
  if (has_feature(PRINTER) || fl_quartz_line_width_ > 1.5f) CGContextSetShouldAntialias(gc, true);
  CGContextMoveToPoint(gc, x, y);
  CGContextAddLineToPoint(gc, x, y1);
  CGContextStrokePath(gc);
  if (Fl_Display_Device::high_resolution()) {
    CGContextFillRect(gc, CGRectMake(x  -  fl_quartz_line_width_/2, y-0.5, fl_quartz_line_width_, 1));
    CGContextFillRect(gc, CGRectMake(x  -  fl_quartz_line_width_/2, y1-0.5, fl_quartz_line_width_, 1));
  }
  if (has_feature(PRINTER) || fl_quartz_line_width_ > 1.5f) CGContextSetShouldAntialias(gc, false);
}

void Fl_Quartz_Graphics_Driver::yxline(int x, int y, int y1, int x2) {
  if (has_feature(PRINTER) || fl_quartz_line_width_ > 1.5f) CGContextSetShouldAntialias(gc, true);
  CGContextMoveToPoint(gc, x, y);
  CGContextAddLineToPoint(gc, x, y1);
  CGContextAddLineToPoint(gc, x2, y1);
  CGContextStrokePath(gc);
  if (Fl_Display_Device::high_resolution()) {
    CGContextFillRect(gc, CGRectMake(x  -  fl_quartz_line_width_/2, y-0.5, fl_quartz_line_width_, 1));
    CGContextFillRect(gc, CGRectMake(x2-0.5, y1  - fl_quartz_line_width_/2, 1 , fl_quartz_line_width_));
  }
  if (has_feature(PRINTER) || fl_quartz_line_width_ > 1.5f) CGContextSetShouldAntialias(gc, false);
}

void Fl_Quartz_Graphics_Driver::yxline(int x, int y, int y1, int x2, int y3) {
  if (has_feature(PRINTER) || fl_quartz_line_width_ > 1.5f) CGContextSetShouldAntialias(gc, true);
  CGContextMoveToPoint(gc, x, y);
  CGContextAddLineToPoint(gc, x, y1);
  CGContextAddLineToPoint(gc, x2, y1);
  CGContextAddLineToPoint(gc, x2, y3);
  CGContextStrokePath(gc);
  if (Fl_Display_Device::high_resolution()) {
    CGContextFillRect(gc, CGRectMake(x  -  fl_quartz_line_width_/2, y-0.5, fl_quartz_line_width_, 1));
    CGContextFillRect(gc, CGRectMake(x2  -  fl_quartz_line_width_/2, y3-0.5, fl_quartz_line_width_, 1));
  }
  if (has_feature(PRINTER) || fl_quartz_line_width_ > 1.5f) CGContextSetShouldAntialias(gc, false);
}

void Fl_Quartz_Graphics_Driver::loop(int x, int y, int x1, int y1, int x2, int y2) {
  CGContextSetShouldAntialias(gc, true);
  CGContextMoveToPoint(gc, x, y);
  CGContextAddLineToPoint(gc, x1, y1);
  CGContextAddLineToPoint(gc, x2, y2);
  CGContextClosePath(gc);
  CGContextStrokePath(gc);
  CGContextSetShouldAntialias(gc, false);
}

void Fl_Quartz_Graphics_Driver::loop(int x, int y, int x1, int y1, int x2, int y2, int x3, int y3) {
  CGContextSetShouldAntialias(gc, true);
  CGContextMoveToPoint(gc, x, y);
  CGContextAddLineToPoint(gc, x1, y1);
  CGContextAddLineToPoint(gc, x2, y2);
  CGContextAddLineToPoint(gc, x3, y3);
  CGContextClosePath(gc);
  CGContextStrokePath(gc);
  CGContextSetShouldAntialias(gc, false);
}

void Fl_Quartz_Graphics_Driver::polygon(int x, int y, int x1, int y1, int x2, int y2) {
  CGContextSetShouldAntialias(gc, true);
  CGContextMoveToPoint(gc, x, y);
  CGContextAddLineToPoint(gc, x1, y1);
  CGContextAddLineToPoint(gc, x2, y2);
  CGContextClosePath(gc);
  CGContextFillPath(gc);
  CGContextSetShouldAntialias(gc, false);
}

void Fl_Quartz_Graphics_Driver::polygon(int x, int y, int x1, int y1, int x2, int y2, int x3, int y3) {
  CGContextSetShouldAntialias(gc, true);
  CGContextMoveToPoint(gc, x, y);
  CGContextAddLineToPoint(gc, x1, y1);
  CGContextAddLineToPoint(gc, x2, y2);
  CGContextAddLineToPoint(gc, x3, y3);
  CGContextClosePath(gc);
  CGContextFillPath(gc);
  CGContextSetShouldAntialias(gc, false);
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
  restore_clip();
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
  restore_clip();
}

// pop back to previous clip:
void Fl_Quartz_Graphics_Driver::pop_clip() {
  if (rstackptr > 0) {
    Fl_Region oldr = rstack[rstackptr--];
    if (oldr) XDestroyRegion(oldr);
  } else Fl::warning("Fl_Quartz_Graphics_Driver::pop_clip: clip stack underflow!\n");
  restore_clip();
}

// helper function to manage the current CGContext gc
extern void fl_quartz_restore_line_style_(CGContextRef gc);

void Fl_Quartz_Graphics_Driver::restore_clip() {
  fl_clip_state_number++;
  Fl_Region r = rstack[rstackptr];
  if ( fl_window || gc ) { // clipping for a true window or an offscreen buffer
    if (gc) {
      CGContextRestoreGState(gc);
      CGContextSaveGState(gc);
    }
    // FLTK has only one global graphics state.
    // This copies the FLTK state into the current Quartz context
    if ( ! fl_window ) { // a bitmap context
      CGFloat hgt = CGBitmapContextGetHeight(gc);
      CGAffineTransform at = CGContextGetCTM(gc);
      CGFloat offset = 0.5;
      if (at.a != 1 && at.a == at.d && at.b == 0 && at.c == 0) { // proportional scaling, no rotation
        hgt /= at.a;
        offset /= at.a;
      }
      CGContextTranslateCTM(gc, offset, hgt-offset);
      CGContextScaleCTM(gc, 1.0f, -1.0f); // now 0,0 is top-left point of the context
    }
    color(color());
    fl_quartz_restore_line_style_(gc);
    if (r) { //apply program clip
      CGContextClipToRects(gc, r->rects, r->count);
    }
  }
}


#endif // FL_CFG_GFX_QUARTZ

//
// End of "$Id$".
//
