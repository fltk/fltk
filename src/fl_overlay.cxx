//
// Overlay support for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2023 by Bill Spitzak and others.
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

// Extremely limited "overlay" support.  You can use this to drag out
// a rectangle in response to mouse events.  It is your responsibility
// to erase the overlay before drawing anything that might intersect
// it.

#include <FL/fl_config.h>
#include <FL/platform.H>
#include <FL/fl_draw.H>

static int px,py,pw,ph;

#include <stdlib.h>
#include "Fl_Screen_Driver.H"
#include <FL/Fl_RGB_Image.H>
static Fl_RGB_Image *s_bgN = 0, *s_bgS = 0, *s_bgE = 0, *s_bgW = 0;

static int bgx, bgy, bgw, bgh;

static void draw_current_rect() {
  if (s_bgN) { delete s_bgN; s_bgN = 0; }
  if (s_bgS) { delete s_bgS; s_bgS = 0; }
  if (s_bgE) { delete s_bgE; s_bgE = 0; }
  if (s_bgW) { delete s_bgW; s_bgW = 0; }
  if (pw>0 && ph>0) {
    s_bgE = Fl::screen_driver()->read_win_rectangle( px+pw-1, py, 1, ph, Fl_Window::current());
    if(s_bgE && s_bgE->w() && s_bgE->h()) {
      s_bgE->scale(1, ph,0,1);
    }
    s_bgW = Fl::screen_driver()->read_win_rectangle( px, py, 1, ph, Fl_Window::current());
    if(s_bgW && s_bgW->w() && s_bgW->h()) {
      s_bgW->scale(1, ph,0,1);
    }
    s_bgS = Fl::screen_driver()->read_win_rectangle( px, py+ph-1, pw, 1, Fl_Window::current());
    if(s_bgS && s_bgS->w() && s_bgS->h()) {
      s_bgS->scale(pw, 1,0,1);
    }
    s_bgN = Fl::screen_driver()->read_win_rectangle( px, py, pw, 1, Fl_Window::current());
    if(s_bgN && s_bgN->w() && s_bgN->h()) {
      s_bgN->scale(pw, 1,0,1);
    }
    bgx = px; bgy = py;
    bgw = pw; bgh = ph;
  }
  fl_color(FL_WHITE);
  fl_line_style(FL_SOLID);
  fl_graphics_driver->overlay_rect(px, py, pw, ph);

  fl_color(FL_BLACK);
  fl_line_style(FL_DOT);
  fl_graphics_driver->overlay_rect(px, py, pw, ph);
  fl_line_style(FL_SOLID);
}

static void erase_current_rect() {
  if (s_bgN) s_bgN->draw(bgx, bgy);
  if (s_bgS) s_bgS->draw(bgx, (bgy+bgh-1));
  if (s_bgW) s_bgW->draw(bgx, bgy);
  if (s_bgE) s_bgE->draw((bgx+bgw-1), bgy);
}

/**
 Erase a selection rectangle without drawing a new one.

 \see fl_overlay_rect(int x, int y, int w, int h)
 */
void fl_overlay_clear() {
  if (pw > 0) {erase_current_rect(); pw = 0;}
}

/**
 Draw a transient dotted selection rectangle.

 This function saves the current screen content and then draws a dotted
 selection rectangle into the front screen buffer. If another selection
 rectangle was drawn earlier, the previous screen graphics are restored first.

 To clear the selection rectangle, call `fl_overlay_clear()`.

 The typical (and only) use for this function is to draw a selection rectangle
 during a mouse drag event sequence without having to redraw the entire content
 of the widget.

 Your event handle should look similar to this (also see `test/mandelbrot.cxx`):
 ```
  int MyWidget::handle(int event) {
    static int ix, iy;
    switch (event) {
      case FL_PUSH:
        ix = Fl::event_x(); iy = Fl::event_y();
        return 1;
      case FL_DRAG:
        this->make_current();
        fl_overlay_rect(ix, iy, ix-Fl::event_x(), iy-Fl::event_y());
        return 1;
      case FL_RELEASE:
        this->make_current();
        fl_overlay_clear();
        // select the element under the rectangle
        return 1;
    }
    return MySuperWidget::handle(event);
  }
 ```

 \note Between drawing an overlay rect and clearing it, the content of the
    widget must not change.

 \note fl_overlay_rect() and fl_overlay_clear() should be called when the actual
    event occurs, and *not* within `MyWidget::draw()`.

 \note fl_overlay_rect() and fl_overlay_clear() should not be mixed with
    Fl_Overlay_Window. Fl_Overlay_Window provides an entirely different way of
    drawing selection outlines and is not limited to rectangles.

 \param x, y, w, h position and size of the overlay rectangle.

 \see fl_overlay_clear()
 */
void fl_overlay_rect(int x, int y, int w, int h) {
  // If there is already another overlay rect, erase it now
  if (pw > 0) {
    if (x==px && y==py && w==pw && h==ph) return;
    erase_current_rect();
  }
  // Width and hight must be positive, swap with coordinates if needed
  if (w < 0) {x += w; w = -w;}
  if (h < 0) {y += h; h = -h;}
  // Clip the overlay to the window rect, or reading the background will fail
  Fl_Window *win = Fl_Window::current();
  if (win) {
    int d;
    d = -x; if (d>0) { x += d; w -= d; }
    d = (x+w)-win->w(); if (d>0) { w -= d; }
    d = -y; if (d>0) { y += d; h -= d; }
    d = (y+h)-win->h(); if (d>0) { h -= d; }
  }
  //
  if (w<1) w = 1;
  if (h<1) h = 1;
  // Store the rect in global variables so we can erase it later
  px = x; py = y; pw = w; ph = h;
  // Draw it
  draw_current_rect();
}
