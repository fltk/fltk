//
// "$Id$"
//
// Scrolling routines for the Fast Light Tool Kit (FLTK).
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

// Drawing function to move the contents of a rectangle.  This is passed
// a "callback" which is called to draw rectangular areas that are moved
// into the drawing area.

#include <Fl/Fl_Window_Driver.H>

// scroll a rectangle and redraw the newly exposed portions:
/**
  Scroll a rectangle and draw the newly exposed portions.
  \param[in] X,Y       position of top-left of rectangle
  \param[in] W,H       size of rectangle
  \param[in] dx,dy     pixel offsets for shifting rectangle
  \param[in] draw_area callback function to draw rectangular areas
  \param[in] data      pointer to user data for callback
  The contents of the rectangular area is first shifted by \p dx
  and \p dy pixels. The \p draw_area callback is then called for
  every newly exposed rectangular area.
  */
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

  int retval = Fl_Window::current()->driver()->scroll(src_x, src_y, src_w, src_h,
                                                      dest_x, dest_y, draw_area, data);
  if (retval) {
    draw_area(data,X,Y,W,H);
    return;
  }
  if (dx) draw_area(data, clip_x, dest_y, clip_w, src_h);
  if (dy) draw_area(data, X, clip_y, W, clip_h);
}

//
// End of "$Id$".
//
