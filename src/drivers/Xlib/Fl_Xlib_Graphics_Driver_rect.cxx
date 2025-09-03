//
// Rectangle drawing routines for the Fast Light Tool Kit (FLTK).
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
 \file Fl_Xlib_Graphics_Driver_rect.cxx
 \brief X11 Xlib specific line and polygon drawing with integer coordinates.
 */

#include <config.h>
#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>
#include <FL/platform.H>

#include "Fl_Xlib_Graphics_Driver.H"

// Arbitrary line clipping: clip line end points to 16-bit coordinate range.

// We clip at +/- 16-bit boundaries (+/- (2**15-K)) where K currently is 8
// to account for line width. We could also clip at lower bound 0 but that
// would make lines less accurate (rounding effects < 1 pixel, but anyway).
// Note that we're only clipping the line end points so we can send true
// 16-bit coordinates to X functions. Actual clipping to window borders
// and clip regions will be done by X.
// Note: the current (default) approach is to clip to the current window
// boundaries instead. This can avoid drawing unecessary (invisible) objects.

/*
  Liang-Barsky line clipping algorithm. For documentation see:
    https://en.wikipedia.org/wiki/Liang-Barsky_algorithm .
  Modified by AlbrechtS for FLTK.
*/
/*
  Returns new coordinates in function arguments x1, y1, x2, y2 if the line
  is within or intersects the valid coordinate space so it is potentially
  visible. Arguments are not changed if the entire line is outside the
  valid coordinate space.

  If the return value is non-zero (true) the line is entirely *clipped*
  and must not be drawn with X functions because the coordinates could be
  outside the valid coordinate space and the result would be undefined.

  Returns 1 (true)  if line is clipped, i.e. *not* visible.
  Returns 0 (false) if line is not clipped, i.e. potentially visible.
*/

int Fl_Xlib_Graphics_Driver::clip_line(int &x1, int &y1, int &x2, int &y2) {

  // define variables
  float p1 = float(-(x2 - x1));
  float p2 = float(-p1);
  float p3 = float(-(y2 - y1));
  float p4 = float(-p3);

  float q1 = float(x1 - clip_min());
  float q2 = float(clip_max() - x1);
  float q3 = float(y1 - clip_min());
  float q4 = float(clip_max() - y1);

  float posmin = 1; // positive minimum
  float negmax = 0; // negative maximum

  if ((p1 == 0 && q1 < 0) || (p3 == 0 && q3 < 0)) {
    return 1; // Line is parallel to clipping window (not visible)
  }
  if (p1 != 0) {
    float r1 = q1 / p1;
    float r2 = q2 / p2;
    if (p1 < 0) {
      if (r1 > negmax) negmax = r1;
      if (r2 < posmin) posmin = r2;
    } else {
      if (r2 > negmax) negmax = r2;
      if (r1 < posmin) posmin = r1;
    }
  }
  if (p3 != 0) {
    float r3 = q3 / p3;
    float r4 = q4 / p4;
    if (p3 < 0) {
      if (r3 > negmax) negmax = r3;
      if (r4 < posmin) posmin = r4;
    } else {
      if (r4 > negmax) negmax = r4;
      if (r3 < posmin) posmin = r3;
    }
  }

  if (negmax > posmin) // line outside clipping window
    return 1; // clipped

  // compute new points (note: order is important!)

  x2 = int(x1 + p2 * posmin);
  y2 = int(y1 + p4 * posmin);

  x1 = int(x1 + p2 * negmax);
  y1 = int(y1 + p4 * negmax);

  return 0; // not clipped

} // clip_line()

// "Old" line clipping: only horizontal and vertical lines and rectangles

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

  fl_line_style (FL_SOLID,5);   // line width = 5
  fl_rect (-1,-1,100,100);      // top/left: 2 pixels visible

  In this example case, no clipping would be done, because X can
  handle it and clip unneeded pixels.

  Note that we must also take care of the case where line_width_
  is zero (maybe unitialized). If this is the case, we assume a line
  width of 1.

  Todo: Arbitrary line drawings (e.g. polygons), circles and curves.
*/

/*
  clip_rect() returns 1 if the area is invisible (clipped) because ...

  (a) w <= 0 or h <= 0                          i.e. nothing is visible
  (b) x+w < clip_min() or y+h < clip_min()      i.e. left of or above visible area
  (c) x > clip_max() or y > clip_max()          i.e. right of or below visible area

  clip_min() and clip_max() are the minimal and maximal x/y coordinate values
  used for clipping.
  In the above cases x, y, w, and h are not changed and the return
  value is 1 (clipped).

  clip_rect() returns 0 if the area is potentially visible and X can
  handle clipping. x, y, w, and h may have been adjusted to fit into the
  valid coordinate space.

  Use this for clipping rectangles as in fl_rect() and fl_rectf().
  It is fast and convenient.
*/

int Fl_Xlib_Graphics_Driver::clip_rect(int &x, int &y, int &w, int &h) {

  if (w <= 0 || h <= 0) return 1;                       // (a)
  if (x+w < clip_min() || y+h < clip_min()) return 1;   // (b)
  if (x > clip_max() || y > clip_max()) return 1;       // (c)

  if (x < clip_min()) { w -= (clip_min()-x); x = clip_min(); }
  if (y < clip_min()) { h -= (clip_min()-y); y = clip_min(); }
  if (x+w > clip_max()) w = clip_max() - x;
  if (y+h > clip_max()) h = clip_max() - y;

  return 0;
}


// Missing X call: (is this the fastest way to init a 1-rectangle region?)
// Windows equivalent exists, implemented inline in win32.H

// Note: if the entire region is outside the 16-bit X coordinate space an empty
// clipping region is returned which means that *nothing* will be visible.

Fl_Region Fl_Xlib_Graphics_Driver::XRectangleRegion(int x, int y, int w, int h) {
  XRectangle R;
  Region r = XCreateRegion();    // create an empty region
  if (clip_rect(x, y, w, h))     // outside valid coordinate space
    return r;                    // empty region
  R.x = x; R.y = y; R.width = w; R.height = h;
  XUnionRectWithRegion(&R, r, r);
  return r;
}

void Fl_Xlib_Graphics_Driver::XDestroyRegion(Fl_Region r) {
  ::XDestroyRegion((Region)r);
}

// --- line and polygon drawing

void Fl_Xlib_Graphics_Driver::focus_rect(int x, int y, int w, int h) {
  w = this->floor(x + w) - this->floor(x);
  h = this->floor(y + h) - this->floor(y);
  x = this->floor(x) + floor(offset_x_);
  y = this->floor(y) + floor(offset_y_);
  if (!clip_rect(x, y, w, h)) {
    int lw_save = line_width_;       // preserve current line_width
    if (line_width_ == 0)
      line_style(FL_DOT, 1);
    else
      line_style(FL_DOT);
    XDrawRectangle(fl_display, fl_window, gc_, x, y, w, h);
    if (lw_save == 0)
      line_style(FL_SOLID, 0);       // restore line type and width
    else
      line_style(FL_SOLID);
  }
}

void Fl_Xlib_Graphics_Driver::rect_unscaled(int x, int y, int w, int h) {
  void *old = NULL;
  if (line_width_ == 0) old = change_pen_width(1); // #156, #1052
  XDrawRectangle(fl_display, fl_window, gc_, x, y, w, h);
  if (old) reset_pen_width(old);
}

void Fl_Xlib_Graphics_Driver::rectf_unscaled(int x, int y, int w, int h) {
  x += floor(offset_x_);
  y += floor(offset_y_);
  if (!clip_rect(x, y, w, h))
    XFillRectangle(fl_display, fl_window, gc_, x, y, w, h);
}

void Fl_Xlib_Graphics_Driver::line_unscaled(int x, int y, int x1, int y1) {
   draw_clipped_line(x + this->floor(offset_x_) , y + this->floor(offset_y_) ,
                    x1 + this->floor(offset_x_) , y1 + this->floor(offset_y_) );
}

void Fl_Xlib_Graphics_Driver::line_unscaled(int x, int y, int x1, int y1, int x2, int y2) {
  if (!clip_line(x1, y1, x, y) && !clip_line(x1, y1, x2, y2)) {
    XPoint p[3];
    int x_offset = floor(offset_x_);
    int y_offset = floor(offset_y_);
    p[0].x = x + x_offset;  p[0].y = y + y_offset;
    p[1].x = x1 + x_offset; p[1].y = y1 + y_offset;
    p[2].x = x2 + x_offset; p[2].y = y2 + y_offset;
    XDrawLines(fl_display, fl_window, gc_, p, 3, 0);
  }
}

void Fl_Xlib_Graphics_Driver::xyline_unscaled(int x, int y, int x1) {
  if (line_width_ >= 2) x1++;
  x += floor(offset_x_) ;
  y += floor(offset_y_) ;
  x1 += floor(offset_x_) ;
  draw_clipped_line(x, y, x1, y);
}

void Fl_Xlib_Graphics_Driver::yxline_unscaled(int x, int y, int y1) {
  if (line_width_ >= 2) y1++;
  x += floor(offset_x_) ;
  y += floor(offset_y_) ;
  y1 += floor(offset_y_) ;
  draw_clipped_line(x, y, x, y1);
}

void Fl_Xlib_Graphics_Driver::loop_unscaled(int x, int y, int x1, int y1, int x2, int y2) {
  XPoint p[4];
  p[0].x = x + floor(offset_x_) ;  p[0].y = y + floor(offset_y_) ;
  p[1].x = x1 + floor(offset_x_) ; p[1].y = y1 + floor(offset_y_) ;
  p[2].x = x2 + floor(offset_x_) ; p[2].y = y2 + floor(offset_y_) ;
  p[3].x = p[0].x;  p[3].y = p[0].y;
  // *FIXME* This needs X coordinate clipping!
  XDrawLines(fl_display, fl_window, gc_, p, 4, 0);
}

void Fl_Xlib_Graphics_Driver::loop_unscaled(int x, int y, int x1, int y1, int x2, int y2, int x3, int y3) {
  XPoint p[5];
  p[0].x = x + floor(offset_x_) ;  p[0].y = y + floor(offset_y_) ;
  p[1].x = x1 + floor(offset_x_) ; p[1].y = y1 + floor(offset_y_) ;
  p[2].x = x2 + floor(offset_x_) ; p[2].y = y2 + floor(offset_y_) ;
  p[3].x = x3 + floor(offset_x_) ; p[3].y = y3 + floor(offset_y_) ;
  p[4].x = p[0].x;  p[4].y = p[0].y;
  // *FIXME* This needs X coordinate clipping!
  XDrawLines(fl_display, fl_window, gc_, p, 5, 0);
}

void Fl_Xlib_Graphics_Driver::polygon_unscaled(int x, int y, int x1, int y1, int x2, int y2) {
  XPoint p[4];
  p[0].x = x + floor(offset_x_) ;  p[0].y = y + floor(offset_y_) ;
  p[1].x = x1 + floor(offset_x_) ; p[1].y = y1 + floor(offset_y_) ;
  p[2].x = x2 + floor(offset_x_) ; p[2].y = y2 + floor(offset_y_) ;
  p[3].x = p[0].x;  p[3].y = p[0].y;
  // *FIXME* This needs X coordinate clipping!
  XFillPolygon(fl_display, fl_window, gc_, p, 3, Convex, 0);
  XDrawLines(fl_display, fl_window, gc_, p, 4, 0);
}

void Fl_Xlib_Graphics_Driver::polygon_unscaled(int x, int y, int x1, int y1, int x2, int y2, int x3, int y3) {
  XPoint p[5];
  p[0].x = x + floor(offset_x_) ;  p[0].y = y + floor(offset_y_) ;
  p[1].x = x1 + floor(offset_x_) ; p[1].y = y1 + floor(offset_y_) ;
  p[2].x = x2 + floor(offset_x_) ; p[2].y = y2 + floor(offset_y_) ;
  p[3].x = x3 + floor(offset_x_) ; p[3].y = y3 + floor(offset_y_) ;
  p[4].x = p[0].x;  p[4].y = p[0].y;
  // *FIXME* This needs X coordinate clipping!
  XFillPolygon(fl_display, fl_window, gc_, p, 4, Convex, 0);
  XDrawLines(fl_display, fl_window, gc_, p, 5, 0);
}

// --- clipped line drawing in X coordinate space (16-bit)

// Draw an arbitrary line with coordinates clipped to the X coordinate space.
// This draws nothing if the line is entirely outside the X coordinate space.

void Fl_Xlib_Graphics_Driver::draw_clipped_line(int x1, int y1, int x2, int y2) {
  if (!clip_line(x1, y1, x2, y2))
    XDrawLine(fl_display, fl_window, gc_, x1, y1, x2, y2);
}

// --- clipping

void Fl_Xlib_Graphics_Driver::push_clip(int x, int y, int w, int h) {
  Region r;
  if (w > 0 && h > 0) {
    r = (Region)XRectangleRegion(x, y, w, h); // does X coordinate clipping
    Region current = (Region)rstack[rstackptr];
    if (current) {
      Region temp = XCreateRegion();
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

int Fl_Xlib_Graphics_Driver::clip_box(int x, int y, int w, int h, int& X, int& Y, int& W, int& H) {
  X = x; Y = y; W = w; H = h;
  // pre-clip rectangle to 16-bit coordinates (STR #3134)
  if (clip_rect(X, Y, W, H)) { // entirely clipped (outside)
    W = H = 0;
    return 2;
  }
  Region r = (Region)rstack[rstackptr];
  if (!r) { // no clipping region
    if (X != x || Y != y || W != w || H != h) // pre-clipped
      return 1; // partially outside, region differs
    return 0;
  }
  switch (XRectInRegion(r, X, Y, W, H)) {
    case 0: // completely outside
      W = H = 0;
      return 2;
    case 1: // completely inside:
      return 0;
    default: // partial:
      break;
  }
  Region rr = (Region)XRectangleRegion(X, Y, W, H);
  Region temp = XCreateRegion();
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
  Region r = (Region)rstack[rstackptr];
  if (!r) return 1;
  // get rid of coordinates outside the 16-bit range the X calls take.
  if (clip_rect(x,y,w,h)) return 0;     // clipped
  return XRectInRegion(r, x, y, w, h);
}

void Fl_Xlib_Graphics_Driver::restore_clip() {
  fl_clip_state_number++;
  if (gc_) {
    Region r = (Region)rstack[rstackptr];
    if (r) {
      Region r2 = (Region)scale_clip(scale());
      XSetRegion(fl_display, gc_, (Region)rstack[rstackptr]);
      unscale_clip(r2);
    }
    else XSetClipMask(fl_display, gc_, 0);
  }
}
