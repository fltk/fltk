//
// Scrolling routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2021 by Bill Spitzak and others.
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

// Drawing function to move the contents of a rectangle.  This is passed
// a "callback" which is called to draw rectangular areas that are moved
// into the drawing area.

#include "Fl_Window_Driver.H"
#include <FL/fl_draw.H>

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
 \warning With FLTK 1.4 and above, it's recommended to put graphical elements in an Fl_Scroll
 widget rather than use function fl_scroll() to update those elements. That's because fl_scroll()
 may not produce a pixel-accurate result when the GUI scaling factor value is not a multiple of 100%.
 Alternatively, use fl_scroll() when the scaling factor value is a multiple of 100% and perform a
 full redraw of the graphical elements otherwise. Statement <tt>float s = Fl::screen_scale(win->screen_num());</tt>
 gives the current scaling factor value given Fl_Window *win.
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

  int retval = Fl_Window_Driver::driver(Fl_Window::current())->scroll(src_x, src_y, src_w, src_h,
                                                      dest_x, dest_y, draw_area, data);
  if (retval) {
    draw_area(data,X,Y,W,H);
    return;
  }
  if (dx) draw_area(data, clip_x, dest_y, clip_w, src_h);
  if (dy) draw_area(data, X, clip_y, W, clip_h);
}
