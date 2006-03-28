//
// "$Id$"
//
// Scrolling routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2005 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

// Drawing function to move the contents of a rectangle.  This is passed
// a "callback" which is called to draw rectangular areas that are moved
// into the drawing area.

#include <config.h>
#include <FL/Fl.H>
#include <FL/x.H>
#include <FL/fl_draw.H>

// scroll a rectangle and redraw the newly exposed portions:
void fl_scroll(int X, int Y, int W, int H, int dx, int dy,
	       void (*draw_area)(void*, int,int,int,int), void* data)
{
  if (!dx && !dy) return;
  if (dx <= -W || dx >= W || dy <= -H || dy >= H) {
    // no intersection of old an new scroll
    draw_area(data,X,Y,W,H);
    return;
  }
  int src_x, src_w, dest_x, clip_x, clip_w;
  if (dx > 0) {
    src_x = X;
    dest_x = X+dx;
    src_w = W-dx;
    clip_x = X;
    clip_w = dx;
  } else {
    src_x = X-dx;
    dest_x = X;
    src_w = W+dx;
    clip_x = X+src_w;
    clip_w = W-src_w;
  }
  int src_y, src_h, dest_y, clip_y, clip_h;
  if (dy > 0) {
    src_y = Y;
    dest_y = Y+dy;
    src_h = H-dy;
    clip_y = Y;
    clip_h = dy;
  } else {
    src_y = Y-dy;
    dest_y = Y;
    src_h = H+dy;
    clip_y = Y+src_h;
    clip_h = H-src_h;
  }
#ifdef WIN32
  BitBlt(fl_gc, dest_x, dest_y, src_w, src_h, fl_gc, src_x, src_y,SRCCOPY);
  // NYI: need to redraw areas that the source of BitBlt was bad due to
  // overlapped windows, probably similar to X version:
  // MRS: basic code needs to redraw parts that scrolled from off-screen...
  int temp, limit;
  int wx, wy;

  // Compute the X position of the current window;
  // this only works when scrolling in response to
  // a user event; Fl_Window::x/y_root() do not work
  // on WIN32...
  wx = Fl::event_x_root() - Fl::event_x();
  wy = Fl::event_y_root() - Fl::event_y();

  temp = wx + src_x;
  if (temp < Fl::x()) {
    draw_area(data, dest_x, dest_y, Fl::x() - temp, src_h);
  }
  temp  = wx + src_x + src_w;
  limit = Fl::x() + Fl::w();
  if (temp > limit) {
    draw_area(data, dest_x + src_w - temp + limit, dest_y, temp - limit, src_h);
  }

  temp = wy + src_y;
  if (temp < Fl::y()) {
    draw_area(data, dest_x, dest_y, src_w, Fl::y() - temp);
  }
  temp  = wy + src_y + src_h;
  limit = Fl::y() + Fl::h();
  if (temp > limit) {
    draw_area(data, dest_x, dest_y + src_h - temp + limit, src_w, temp - limit);
  }
#elif defined(__APPLE_QD__)
  Rect src = { src_y, src_x, src_y+src_h, src_x+src_w };
  Rect dst = { dest_y, dest_x, dest_y+src_h, dest_x+src_w };
  static RGBColor bg = { 0xffff, 0xffff, 0xffff }; RGBBackColor( &bg );
  static RGBColor fg = { 0x0000, 0x0000, 0x0000 }; RGBForeColor( &fg );
  CopyBits( GetPortBitMapForCopyBits( GetWindowPort(fl_window) ),
            GetPortBitMapForCopyBits( GetWindowPort(fl_window) ), &src, &dst, srcCopy, 0L);
#elif defined(__APPLE_QUARTZ__)
  // warning: there does not seem to be an equivalent to this function in Quartz
  Rect src = { src_y, src_x, src_y+src_h, src_x+src_w };
  Rect dst = { dest_y, dest_x, dest_y+src_h, dest_x+src_w };
  static RGBColor bg = { 0xffff, 0xffff, 0xffff }; RGBBackColor( &bg );
  static RGBColor fg = { 0x0000, 0x0000, 0x0000 }; RGBForeColor( &fg );
  CopyBits( GetPortBitMapForCopyBits( GetWindowPort(fl_window) ),
            GetPortBitMapForCopyBits( GetWindowPort(fl_window) ), &src, &dst, srcCopy, 0L);
#else
  XCopyArea(fl_display, fl_window, fl_window, fl_gc,
	    src_x, src_y, src_w, src_h, dest_x, dest_y);
  // we have to sync the display and get the GraphicsExpose events! (sigh)
  for (;;) {
    XEvent e; XWindowEvent(fl_display, fl_window, ExposureMask, &e);
    if (e.type == NoExpose) break;
    // otherwise assumme it is a GraphicsExpose event:
    draw_area(data, e.xexpose.x, e.xexpose.y,
	      e.xexpose.width, e.xexpose.height);
    if (!e.xgraphicsexpose.count) break;
  }
#endif
  if (dx) draw_area(data, clip_x, dest_y, clip_w, src_h);
  if (dy) draw_area(data, X, clip_y, W, clip_h);
}

//
// End of "$Id$".
//
