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


#ifndef FL_CFG_GFX_XLIB_RECT_CXX
#define FL_CFG_GFX_XLIB_RECT_CXX


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

// fl_line_width_ must contain the absolute value of the current
// line width to be used for X11 clipping (see below).
// This is defined in src/fl_line_style.cxx
extern int fl_line_width_;

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

 Note that we must also take care of the case where fl_line_width_
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
static int clip_to_short(int &x, int &y, int &w, int &h) {

  int lw = (fl_line_width_ > 0) ? fl_line_width_ : 1;
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
static int clip_x (int x) {

  int lw = (fl_line_width_ > 0) ? fl_line_width_ : 1;
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
Fl_Region XRectangleRegion(int x, int y, int w, int h) {
  XRectangle R;
  clip_to_short(x, y, w, h);
  R.x = x; R.y = y; R.width = w; R.height = h;
  Fl_Region r = XCreateRegion();
  XUnionRectWithRegion(&R, r, r);
  return r;
}

// --- line and polygon drawing with integer coordinates

void Fl_Xlib_Graphics_Driver::point(int x, int y) {
  XDrawPoint(fl_display, fl_window, gc_, clip_x(x), clip_x(y));
}

void Fl_Xlib_Graphics_Driver::rect(int x, int y, int w, int h) {
  if (w<=0 || h<=0) return;
  if (!clip_to_short(x, y, w, h))
    XDrawRectangle(fl_display, fl_window, gc_, x, y, w-1, h-1);
}

void Fl_Xlib_Graphics_Driver::rectf(int x, int y, int w, int h) {
  if (w<=0 || h<=0) return;
  if (!clip_to_short(x, y, w, h))
    XFillRectangle(fl_display, fl_window, gc_, x, y, w, h);
}

void Fl_Xlib_Graphics_Driver::line(int x, int y, int x1, int y1) {
  XDrawLine(fl_display, fl_window, gc_, x, y, x1, y1);
}

void Fl_Xlib_Graphics_Driver::line(int x, int y, int x1, int y1, int x2, int y2) {
  XPoint p[3];
  p[0].x = x;  p[0].y = y;
  p[1].x = x1; p[1].y = y1;
  p[2].x = x2; p[2].y = y2;
  XDrawLines(fl_display, fl_window, gc_, p, 3, 0);
}

void Fl_Xlib_Graphics_Driver::xyline(int x, int y, int x1) {
  XDrawLine(fl_display, fl_window, gc_, clip_x(x), clip_x(y), clip_x(x1), clip_x(y));
}

void Fl_Xlib_Graphics_Driver::xyline(int x, int y, int x1, int y2) {
  XPoint p[3];
  p[0].x = clip_x(x);  p[0].y = p[1].y = clip_x(y);
  p[1].x = p[2].x = clip_x(x1); p[2].y = clip_x(y2);
  XDrawLines(fl_display, fl_window, gc_, p, 3, 0);
}

void Fl_Xlib_Graphics_Driver::xyline(int x, int y, int x1, int y2, int x3) {
  XPoint p[4];
  p[0].x = clip_x(x);  p[0].y = p[1].y = clip_x(y);
  p[1].x = p[2].x = clip_x(x1); p[2].y = p[3].y = clip_x(y2);
  p[3].x = clip_x(x3);
  XDrawLines(fl_display, fl_window, gc_, p, 4, 0);
}

void Fl_Xlib_Graphics_Driver::yxline(int x, int y, int y1) {
  XDrawLine(fl_display, fl_window, gc_, clip_x(x), clip_x(y), clip_x(x), clip_x(y1));
}

void Fl_Xlib_Graphics_Driver::yxline(int x, int y, int y1, int x2) {
  XPoint p[3];
  p[0].x = p[1].x = clip_x(x);  p[0].y = clip_x(y);
  p[1].y = p[2].y = clip_x(y1); p[2].x = clip_x(x2);
  XDrawLines(fl_display, fl_window, gc_, p, 3, 0);
}

void Fl_Xlib_Graphics_Driver::yxline(int x, int y, int y1, int x2, int y3) {
  XPoint p[4];
  p[0].x = p[1].x = clip_x(x);  p[0].y = clip_x(y);
  p[1].y = p[2].y = clip_x(y1); p[2].x = p[3].x = clip_x(x2);
  p[3].y = clip_x(y3);
  XDrawLines(fl_display, fl_window, gc_, p, 4, 0);
}

void Fl_Xlib_Graphics_Driver::loop(int x, int y, int x1, int y1, int x2, int y2) {
  XPoint p[4];
  p[0].x = x;  p[0].y = y;
  p[1].x = x1; p[1].y = y1;
  p[2].x = x2; p[2].y = y2;
  p[3].x = x;  p[3].y = y;
  XDrawLines(fl_display, fl_window, gc_, p, 4, 0);
}

void Fl_Xlib_Graphics_Driver::loop(int x, int y, int x1, int y1, int x2, int y2, int x3, int y3) {
  XPoint p[5];
  p[0].x = x;  p[0].y = y;
  p[1].x = x1; p[1].y = y1;
  p[2].x = x2; p[2].y = y2;
  p[3].x = x3; p[3].y = y3;
  p[4].x = x;  p[4].y = y;
  XDrawLines(fl_display, fl_window, gc_, p, 5, 0);
}

void Fl_Xlib_Graphics_Driver::polygon(int x, int y, int x1, int y1, int x2, int y2) {
  XPoint p[4];
  p[0].x = x;  p[0].y = y;
  p[1].x = x1; p[1].y = y1;
  p[2].x = x2; p[2].y = y2;
  p[3].x = x;  p[3].y = y;
  XFillPolygon(fl_display, fl_window, gc_, p, 3, Convex, 0);
  XDrawLines(fl_display, fl_window, gc_, p, 4, 0);
}

void Fl_Xlib_Graphics_Driver::polygon(int x, int y, int x1, int y1, int x2, int y2, int x3, int y3) {
  XPoint p[5];
  p[0].x = x;  p[0].y = y;
  p[1].x = x1; p[1].y = y1;
  p[2].x = x2; p[2].y = y2;
  p[3].x = x3; p[3].y = y3;
  p[4].x = x;  p[4].y = y;
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
  if (clip_to_short(x,y,w,h)) return 0;	// clipped
  return XRectInRegion(r, x, y, w, h);
}

// make there be no clip (used by fl_begin_offscreen() only!)
void Fl_Xlib_Graphics_Driver::push_no_clip() {
  if (rstackptr < region_stack_max) rstack[++rstackptr] = 0;
  else Fl::warning("fl_push_no_cFl_Xlib_Graphics_Driver::push_no_cliplip: clip stack overflow!\n");
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
    Fl_Region r = rstack[rstackptr];
    if (r) XSetRegion(fl_display, gc_, r);
    else XSetClipMask(fl_display, gc_, 0);
  }
}

#endif // FL_CFG_GFX_XLIB_RECT_CXX

//
// End of "$Id$".
//
