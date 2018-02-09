//
// "$Id$"
//
// Overlay support for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2018 by Bill Spitzak and others.
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

// Extremely limited "overlay" support.  You can use this to drag out
// a rectangle in response to mouse events.  It is your responsibility
// to erase the overlay before drawing anything that might intersect
// it.

#include <FL/platform.H>
#include <FL/fl_draw.H>

//#define USE_XOR
// unless USE_XOR is defined, this source file is cross-platform

#ifdef USE_XOR
#include <config.h>
#endif

static int px,py,pw,ph;

#ifndef USE_XOR
#include <stdlib.h>
static uchar *bgN = 0L, *bgS = 0L, *bgE = 0L, *bgW = 0L;

#include <FL/Fl_Screen_Driver.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/Fl_Shared_Image.H>
static Fl_Shared_Image *s_bgN = 0, *s_bgS = 0, *s_bgE = 0, *s_bgW = 0;

static int bgx, bgy, bgw, bgh;
#endif

static void draw_current_rect() {
#ifdef USE_XOR
# if defined(USE_X11)
  GC gc = (GC)fl_graphics_driver->gc();
  XSetFunction(fl_display, gc, GXxor);
  XSetForeground(fl_display, gc, 0xffffffff);
  XDrawRectangle(fl_display, fl_window, gc, px, py, pw, ph);
  XSetFunction(fl_display, gc, GXcopy);
# elif defined(_WIN32)
  int old = SetROP2(fl_graphics_driver->gc(), R2_NOT);
  fl_rect(px, py, pw, ph);
  SetROP2(fl_graphics_driver->gc(), old);
# elif defined(__APPLE__)
  // warning: Quartz does not support xor drawing
  // Use the Fl_Overlay_Window instead.
  fl_color(FL_WHITE);
  fl_rect(px, py, pw, ph);
# else
#  error unsupported platform
# endif
#else
  bool unscaled = fl_graphics_driver->overlay_rect_unscaled();
  if (unscaled) {
    if (bgN) { free(bgN); bgN = 0L; }
    if (bgS) { free(bgS); bgS = 0L; }
    if (bgE) { free(bgE); bgE = 0L; }
    if (bgW) { free(bgW); bgW = 0L; }
  } else {
    if (s_bgN) { s_bgN->release(); s_bgN = 0; }
    if (s_bgS) { s_bgS->release(); s_bgS = 0; }
    if (s_bgE) { s_bgE->release(); s_bgE = 0; }
    if (s_bgW) { s_bgW->release(); s_bgW = 0; }
  }
  if (pw>0 && ph>0) {
    if (unscaled) {
      bgE = fl_read_image(0L, px+pw-1, py, 1, ph);
      bgW = fl_read_image(0L, px, py, 1, ph);
      bgS = fl_read_image(0L, px, py+ph-1, pw, 1);
      bgN = fl_read_image(0L, px, py, pw, 1);
    } else {
      Fl_RGB_Image *tmp;
      tmp = Fl::screen_driver()->read_win_rectangle( px+pw-1, py, 1, ph);
      if(tmp && tmp->w() && tmp->h()) {
        s_bgE = Fl_Shared_Image::get(tmp);
        s_bgE->scale(1, ph,0,1);
      }
      tmp = Fl::screen_driver()->read_win_rectangle( px, py, 1, ph);
      if(tmp && tmp->w() && tmp->h()) {
        s_bgW = Fl_Shared_Image::get(tmp);
        s_bgW->scale(1, ph,0,1);
      }
      tmp = Fl::screen_driver()->read_win_rectangle( px, py+ph-1, pw, 1);
      if(tmp && tmp->w() && tmp->h()) {
        s_bgS = Fl_Shared_Image::get(tmp);
        s_bgS->scale(pw, 1,0,1);
      }
      tmp = Fl::screen_driver()->read_win_rectangle( px, py, pw, 1);
      if(tmp && tmp->w() && tmp->h()) {
        s_bgN = Fl_Shared_Image::get(tmp);
        s_bgN->scale(pw, 1,0,1);
      }
    }
    bgx = px; bgy = py;
    bgw = pw; bgh = ph;
  }
  fl_color(FL_WHITE);
  fl_line_style(FL_SOLID);
  if (unscaled) fl_rect(px, py, pw, ph);
  else fl_graphics_driver->overlay_rect(px, py, pw, ph);

  fl_color(FL_BLACK);
  fl_line_style(FL_DOT);
  if (unscaled) fl_rect(px, py, pw, ph);
  else fl_graphics_driver->overlay_rect(px, py, pw, ph);
  fl_line_style(FL_SOLID);
#endif // USE_XOR
}

static void erase_current_rect() {
#ifdef USE_XOR
# ifdef __APPLE_QUARTZ__ // PORTME: Fl_Window_Driver - platform overlay
  fl_rect(px, py, pw, ph);
# else
  draw_current_rect();
# endif
#else
  bool unscaled = fl_graphics_driver->overlay_rect_unscaled();
  if (unscaled) {
    if (bgN) fl_draw_image(bgN, bgx, bgy, bgw, 1);
    if (bgS) fl_draw_image(bgS, bgx, bgy+bgh-1, bgw, 1);
    if (bgW) fl_draw_image(bgW, bgx, bgy, 1, bgh);
    if (bgE) fl_draw_image(bgE, bgx+bgw-1, bgy, 1, bgh);
  } else {
    if (s_bgN) s_bgN->draw(bgx, bgy);
    if (s_bgS) s_bgS->draw(bgx, (bgy+bgh-1));
    if (s_bgW) s_bgW->draw(bgx, bgy);
    if (s_bgE) s_bgE->draw((bgx+bgw-1), bgy);
  }
#endif
}

/**
  Erase a selection rectangle without drawing a new one
  */
void fl_overlay_clear() {
  if (pw > 0) {erase_current_rect(); pw = 0;}
}

/**
  Draws a selection rectangle, erasing a previous one by XOR'ing it first.
  */
void fl_overlay_rect(int x, int y, int w, int h) {
  if (w < 0) {x += w; w = -w;} else if (!w) w = 1;
  if (h < 0) {y += h; h = -h;} else if (!h) h = 1;
  if (pw > 0) {
    if (x==px && y==py && w==pw && h==ph) return;
    erase_current_rect();
  }
  px = x; py = y; pw = w; ph = h;
  draw_current_rect();
}

//
// End of "$Id$".
//
