//
// "$Id$"
//
// Rectangle drawing routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2017 by Bill Spitzak and others.
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
 \file Fl_Xlib_Graphics_Driver_rect.cxx
 \brief X11 Xlib specific line and polygon drawing with integer coordinates.
 */

#include <config.h>
#include "../../config_lib.h"
#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Printer.H>
#include <FL/fl_draw.H>
#include <FL/x.H>

#include "Fl_Xlib_Graphics_Driver.H"


#ifndef SHRT_MAX
#define SHRT_MAX (32767)
#endif

// line_width_ must contain the absolute value of the current
// line width to be used for X11 clipping (see below).

/*
 We need to check some coordinates for areas for clipping before we
 use X functions, because X can't handle coordinates outside the 16-bit
 range. Since all windows use relative coordinates > 0, we do also
 check for negative values. X11 only, see also STR #2304.

 Note that this is only necessary for large objects, where only a
 part of the object is visible. The draw() functions (e.g. box
 drawing) must be clipped correctly. This is usually only a matter
 for large container widgets. The individual child widgets will be
 clipped completely.

 We define the usable X coordinate space as [ -LW : SHRT_MAX - LW ]
 where LW = current line width for drawing. This is done so that
 horizontal and vertical line drawing works correctly, even in real
 border cases, e.g. drawing a rectangle slightly outside the top left
 window corner, but with a line width so that a part of the line should
 be visible (in this case 2 of 5 pixels):

 fl_line_style (FL_SOLID,5);	// line width = 5
 fl_rect (-1,-1,100,100);	// top/left: 2 pixels visible

 In this example case, no clipping would be done, because X can
 handle it and clip unneeded pixels.

 Note that we must also take care of the case where line_width_
 is zero (maybe unitialized). If this is the case, we assume a line
 width of 1.

 Todo: Arbitrary line drawings (e.g. polygons) and clip regions
 are not yet done.

 Note:

 We could use max. screen coordinates instead of SHRT_MAX, but that
 would need more work and would probably be slower. We assume that
 all window coordinates are >= 0 and that no window extends up to
 32767 - LW (where LW = current line width). Thus it is safe to clip
 all coordinates to this range before calling X functions. If this
 is not true, then clip_to_short() and clip_x() must be redefined.

 It would be somewhat easier if we had fl_clip_w and fl_clip_h, as
 defined in FLTK 2.0 (for the upper clipping bounds)...
 */

/*
 clip_to_short() returns 1, if the area is invisible (clipped),
 because ...

 (a) w or h are <= 0		i.e. nothing is visible
 (b) x+w or y+h are < kmin	i.e. left of or above visible area
 (c) x or y are > kmax	i.e. right of or below visible area

 kmin and kmax are the minimal and maximal X coordinate values,
 as defined above. In this case x, y, w, and h are not changed.

 It returns 0, if the area is potentially visible and X can handle
 clipping. x, y, w, and h may have been adjusted to fit into the
 X coordinate space.

 Use this for clipping rectangles, as used in fl_rect() and
 fl_rectf().
 */
static int clip_to_short(int &x, int &y, int &w, int &h, int line_width) {

  int lw = (line_width > 0) ? line_width : 1;
  int kmin = -lw;
  int kmax = SHRT_MAX - lw;

  if (w <= 0 || h <= 0) return 1;		// (a)
  if (x+w < kmin || y+h < kmin) return 1;	// (b)
  if (x > kmax || y > kmax) return 1;		// (c)

  if (x < kmin) { w -= (kmin-x); x = kmin; }
  if (y < kmin) { h -= (kmin-y); y = kmin; }
  if (x+w > kmax) w = kmax - x;
  if (y+h > kmax) h = kmax - y;

  return 0;
}

/*
 clip_x() returns a coordinate value clipped to the 16-bit coordinate
 space (see above). This can be used to draw horizontal and vertical
 lines that can be handled by X11. Each single coordinate value can
 be clipped individually, and the result can be used directly, e.g.
 in fl_xyline() and fl_yxline(). Note that this can't be used for
 arbitrary lines (not horizontal or vertical).
 */
int Fl_Xlib_Graphics_Driver::clip_x (int x) {

  int lw = (line_width_ > 0) ? line_width_ : 1;
  int kmin = -lw;
  int kmax = SHRT_MAX - lw;

  if (x < kmin)
    x = kmin;
  else if (x > kmax)
    x = kmax;
  return x;
}

// Missing X call: (is this the fastest way to init a 1-rectangle region?)
// MSWindows equivalent exists, implemented inline in win32.H
Fl_Region Fl_Xlib_Graphics_Driver::XRectangleRegion(int x, int y, int w, int h) {
  XRectangle R;
  clip_to_short(x, y, w, h, line_width_);
  R.x = x; R.y = y; R.width = w; R.height = h;
  Fl_Region r = XCreateRegion();
  XUnionRectWithRegion(&R, r, r);
  return r;
}

void Fl_Xlib_Graphics_Driver::XDestroyRegion(Fl_Region r) {
  ::XDestroyRegion(r);
}

// --- line and polygon drawing

// called only when scale_ has integer value
void Fl_Xlib_Graphics_Driver::rect_unscaled(float fx, float fy, float fw, float fh) {
  if (fw<=0 || fh<=0) return;
  int deltaf = scale_ >= 2 ? scale_-1 : 0;
  fx += offset_x_*scale_; fy += offset_y_*scale_;
  int x = fx; int y = fy;
  int w = int(fw) - 1 - deltaf;
  int h = int(fh) - 1 - deltaf;
  if (!clip_to_short(x, y, w, h, line_width_))
    XDrawRectangle(fl_display, fl_window, gc_, x+line_delta_, y+line_delta_, w, h);
}

void Fl_Xlib_Graphics_Driver::rectf_unscaled(float fx, float fy, float fw, float fh) {
  if (fw<=0 || fh<=0) return;
  int deltaf = scale_ >= 2 ? scale_/2 : 0;
  fx += offset_x_*scale_; fy += offset_y_*scale_;
  int x = fx-deltaf; int y = fy-deltaf;
  // make sure no unfilled area lies between rectf(x,y,w,h) and  rectf(x+w,y,1,h) or rectf(x,y+w,w,1)
  int w = int(int(fx/scale_+fw/scale_+0.5)*scale_) - int(fx);
  int h = int(int(fy/scale_+fh/scale_+0.5)*scale_) - int(fy);
  if (!clip_to_short(x, y, w, h, line_width_))
    XFillRectangle(fl_display, fl_window, gc_, x+line_delta_, y+line_delta_, w, h);
}

void Fl_Xlib_Graphics_Driver::point_unscaled(float fx, float fy) {
  int deltaf = scale_ >= 2 ? scale_/2 : 0;
  int x = fx+offset_x_*scale_-deltaf; int y = fy+offset_y_*scale_-deltaf;
  int width = scale_ >= 1 ? scale_ : 1;
  XFillRectangle(fl_display, fl_window, gc_, x+line_delta_, y+line_delta_, width, width);
}

void Fl_Xlib_Graphics_Driver::line_unscaled(float x, float y, float x1, float y1) {
  if (x == x1) yxline_unscaled(x, y, y1);
  else if (y == y1) xyline_unscaled(x, y, x1);
  else XDrawLine(fl_display, fl_window, gc_, x+offset_x_*scale_+line_delta_, y+offset_y_*scale_+line_delta_, x1+offset_x_*scale_+line_delta_, y1+offset_y_*scale_+line_delta_);
}

void Fl_Xlib_Graphics_Driver::line_unscaled(float x, float y, float x1, float y1, float x2, float y2) {
  XPoint p[3];
  p[0].x = x+offset_x_*scale_+line_delta_;  p[0].y = y+offset_y_*scale_+line_delta_;
  p[1].x = x1+offset_x_*scale_+line_delta_; p[1].y = y1+offset_y_*scale_+line_delta_;
  p[2].x = x2+offset_x_*scale_+line_delta_; p[2].y = y2+offset_y_*scale_+line_delta_;
  XDrawLines(fl_display, fl_window, gc_, p, 3, 0);
}

void Fl_Xlib_Graphics_Driver::xyline_unscaled(float x, float y, float x1) {
  x+=offset_x_*scale_; y+=offset_y_*scale_; x1 += offset_x_*scale_;
  int tw = line_width_ ? line_width_ : 1; // true line width
  if (x > x1) { float exch = x; x = x1; x1 = exch; }
  int ix = clip_x(x+line_delta_); if (scale_ >= 2) ix -= int(scale_/2);
  int iy = clip_x(y+line_delta_);
  // make sure that line output by xyline(a,b,c) extends to pixel just at left of where xyline(c+1,b,d) begins
  int ix1 = int(x1/scale_+1.5)*scale_-1;
  ix1 += line_delta_; if (scale_ >= 4) ix1 -= 1;
  XDrawLine(fl_display, fl_window, gc_, ix, iy, ix1, iy);
  // make sure no unfilled area lies between xyline(x,y,x1) and xyline(x,y+1,x1)
  if (y+line_delta_ + scale_ >= iy + tw+1 - 0.001 ) XDrawLine(fl_display, fl_window, gc_, ix, iy+1, ix1, iy+1);
}

void Fl_Xlib_Graphics_Driver::yxline_unscaled(float x, float y, float y1) {
  x+=offset_x_*scale_; y+=offset_y_*scale_; y1 += offset_y_*scale_;
  int tw = line_width_ ? line_width_ : 1; // true line width
  if (y > y1) { float exch = y; y = y1; y1 = exch; }
  int ix = clip_x(x+line_delta_);
  int iy = clip_x(y+line_delta_); if (scale_ >= 2) iy -= int(scale_/2);
  int iy1 = int(y1/scale_+1.5)*scale_-1;
  // make sure that line output by yxline(a,b,c) extends to pixel just above where yxline(a,c+1,d) begins
  iy1 += line_delta_; if (scale_ >= 4) iy1 -= 1;
  XDrawLine(fl_display, fl_window, gc_, ix, iy, ix, iy1);
  // make sure no unfilled area lies between yxline(x,y,y1) and yxline(x+1,y,y1)
  if (x+line_delta_+scale_ >= ix + tw+1 -0.001) XDrawLine(fl_display, fl_window, gc_, ix+1, iy, ix+1, iy1);
}

void Fl_Xlib_Graphics_Driver::loop_unscaled(float x, float y, float x1, float y1, float x2, float y2) {
  XPoint p[4];
  p[0].x = x +offset_x_*scale_+line_delta_;  p[0].y = y +offset_y_*scale_+line_delta_;
  p[1].x = x1 +offset_x_*scale_+line_delta_; p[1].y = y1 +offset_y_*scale_+line_delta_;
  p[2].x = x2 +offset_x_*scale_+line_delta_; p[2].y = y2 +offset_y_*scale_+line_delta_;
  p[3].x = x +offset_x_*scale_+line_delta_;  p[3].y = y +offset_y_*scale_+line_delta_;
  XDrawLines(fl_display, fl_window, gc_, p, 4, 0);
}

void Fl_Xlib_Graphics_Driver::loop_unscaled(float x, float y, float x1, float y1, float x2, float y2, float x3, float y3) {
  XPoint p[5];
  p[0].x = x+offset_x_*scale_+line_delta_;  p[0].y = y+offset_y_*scale_+line_delta_;
  p[1].x = x1 +offset_x_*scale_+line_delta_; p[1].y = y1+offset_y_*scale_+line_delta_;
  p[2].x = x2+offset_x_*scale_+line_delta_; p[2].y = y2+offset_y_*scale_+line_delta_;
  p[3].x = x3+offset_x_*scale_+line_delta_; p[3].y = y3+offset_y_*scale_+line_delta_;
  p[4].x = x+offset_x_*scale_+line_delta_;  p[4].y = y+offset_y_*scale_+line_delta_;
  XDrawLines(fl_display, fl_window, gc_, p, 5, 0);
}

void Fl_Xlib_Graphics_Driver::polygon_unscaled(float x, float y, float x1, float y1, float x2, float y2) {
  XPoint p[4];
  p[0].x = x+offset_x_*scale_+line_delta_;  p[0].y = y+offset_y_*scale_+line_delta_;
  p[1].x = x1+offset_x_*scale_+line_delta_; p[1].y = y1+offset_y_*scale_+line_delta_;
  p[2].x = x2+offset_x_*scale_+line_delta_; p[2].y = y2+offset_y_*scale_+line_delta_;
  p[3].x = x+offset_x_*scale_+line_delta_;  p[3].y = y+offset_y_*scale_+line_delta_;
  XFillPolygon(fl_display, fl_window, gc_, p, 3, Convex, 0);
  XDrawLines(fl_display, fl_window, gc_, p, 4, 0);
}

void Fl_Xlib_Graphics_Driver::polygon_unscaled(float x, float y, float x1, float y1, float x2, float y2, float x3, float y3) {
  XPoint p[5];
  p[0].x = x+offset_x_*scale_+line_delta_;  p[0].y = y+offset_y_*scale_+line_delta_;
  p[1].x = x1+offset_x_*scale_+line_delta_; p[1].y = y1+offset_y_*scale_+line_delta_;
  p[2].x = x2+offset_x_*scale_+line_delta_; p[2].y = y2+offset_y_*scale_+line_delta_;
  p[3].x = x3+offset_x_*scale_+line_delta_; p[3].y = y3+offset_y_*scale_+line_delta_;
  p[4].x = x+offset_x_*scale_+line_delta_;  p[4].y = y+offset_y_*scale_+line_delta_;
  XFillPolygon(fl_display, fl_window, gc_, p, 4, Convex, 0);
  XDrawLines(fl_display, fl_window, gc_, p, 5, 0);
}

// --- clipping

void Fl_Xlib_Graphics_Driver::push_clip(int x, int y, int w, int h) {
  Fl_Region r;
  if (w > 0 && h > 0) {
    r = XRectangleRegion(x,y,w,h);
    Fl_Region current = rstack[rstackptr];
    if (current) {
      Fl_Region temp = XCreateRegion();
      XIntersectRegion(current, r, temp);
      XDestroyRegion(r);
      r = temp;
    }
  } else { // make empty clip region:
    r = XCreateRegion();
  }
  if (rstackptr < region_stack_max) rstack[++rstackptr] = r;
  else Fl::warning("Fl_Xlib_Graphics_Driver::push_clip: clip stack overflow!\n");
  restore_clip();
}

int Fl_Xlib_Graphics_Driver::clip_box(int x, int y, int w, int h, int& X, int& Y, int& W, int& H){
  X = x; Y = y; W = w; H = h;
  Fl_Region r = rstack[rstackptr];
  if (!r) return 0;
  switch (XRectInRegion(r, x, y, w, h)) {
    case 0: // completely outside
      W = H = 0;
      return 2;
    case 1: // completely inside:
      return 0;
    default: // partial:
      break;
  }
  Fl_Region rr = XRectangleRegion(x,y,w,h);
  Fl_Region temp = XCreateRegion();
  XIntersectRegion(r, rr, temp);
  XRectangle rect;
  XClipBox(temp, &rect);
  X = rect.x; Y = rect.y; W = rect.width; H = rect.height;
  XDestroyRegion(temp);
  XDestroyRegion(rr);
  return 1;
}

int Fl_Xlib_Graphics_Driver::not_clipped(int x, int y, int w, int h) {
  if (x+w <= 0 || y+h <= 0) return 0;
  Fl_Region r = rstack[rstackptr];
  if (!r) return 1;
  // get rid of coordinates outside the 16-bit range the X calls take.
  if (clip_to_short(x,y,w,h, line_width_)) return 0;	// clipped
  return XRectInRegion(r, x, y, w, h);
}

// make there be no clip (used by fl_begin_offscreen() only!)
void Fl_Xlib_Graphics_Driver::push_no_clip() {
  if (rstackptr < region_stack_max) rstack[++rstackptr] = 0;
  else Fl::warning("Fl_Xlib_Graphics_Driver::push_no_clip: clip stack overflow!\n");
  restore_clip();
}

// pop back to previous clip:
void Fl_Xlib_Graphics_Driver::pop_clip() {
  if (rstackptr > 0) {
    Fl_Region oldr = rstack[rstackptr--];
    if (oldr) XDestroyRegion(oldr);
  } else Fl::warning("Fl_Xlib_Graphics_Driver::pop_clip: clip stack underflow!\n");
  restore_clip();
}

void Fl_Xlib_Graphics_Driver::restore_clip() {
  fl_clip_state_number++;
  if (gc_) {
    Region r = rstack[rstackptr];
    if (r) {
      Region r2 = scale_clip(scale_);
      XSetRegion(fl_display, gc_, rstack[rstackptr]);
      unscale_clip(r2);
    }
    else XSetClipMask(fl_display, gc_, 0);
  }
}

//
// End of "$Id$".
//
