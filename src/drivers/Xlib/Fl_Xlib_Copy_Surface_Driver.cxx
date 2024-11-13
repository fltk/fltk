//
// Copy-to-clipboard code for the Fast Light Tool Kit (FLTK).
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

#include <config.h>
#include "Fl_Xlib_Copy_Surface_Driver.H"
#include <FL/Fl.H>
#include <FL/platform.H>
#include <FL/Fl_Image_Surface.H>
#include <FL/fl_draw.H>
#include "../X11/Fl_X11_Screen_Driver.H"
#if FLTK_USE_CAIRO
#  include <cairo-xlib.h>
#  include "../Cairo/Fl_X11_Cairo_Graphics_Driver.H"
#  include <cairo/cairo.h>
#else
#  include "Fl_Xlib_Graphics_Driver.H"
#endif // FLTK_USE_CAIRO



Fl_Xlib_Copy_Surface_Driver::Fl_Xlib_Copy_Surface_Driver(int w, int h) : Fl_Copy_Surface_Driver(w, h) {
#if FLTK_USE_CAIRO
  driver(new Fl_X11_Cairo_Graphics_Driver());
#else
  driver(new Fl_Xlib_Graphics_Driver());
#endif
  float s = Fl_Graphics_Driver::default_driver().scale();
  driver()->scale(s);
  oldwindow = fl_window;
  xid = new Fl_Image_Surface(w, h, 1);
#if FLTK_USE_CAIRO
  cairo_surface_t *surf = cairo_xlib_surface_create(fl_display, xid->offscreen(), fl_visual->visual, w * s, h * s);
  cairo_ = cairo_create(surf);
  cairo_surface_destroy(surf);
  cairo_scale(cairo_, 1/s, 1/s);
  cairo_save(cairo_);
  ((Fl_X11_Cairo_Graphics_Driver*)driver())->set_cairo(cairo_);
#endif
  fl_window = xid->offscreen();
  driver()->color(FL_WHITE);
  driver()->rectf(0, 0, w, h);
  fl_window = oldwindow;
}


Fl_Xlib_Copy_Surface_Driver::~Fl_Xlib_Copy_Surface_Driver() {
  Window old_win = fl_window;
  fl_window = xid->offscreen();
  Fl_RGB_Image *rgb = Fl::screen_driver()->read_win_rectangle(0, 0, width, height, 0);
  fl_window = old_win;
  if (is_current()) end_current();
  Fl_X11_Screen_Driver::copy_image(rgb->array, rgb->w(), rgb->h(), 1);
  delete rgb;
  delete xid;
#if FLTK_USE_CAIRO
  cairo_destroy(cairo_);
#endif
  delete driver();
}


void Fl_Xlib_Copy_Surface_Driver::set_current() {
  Fl_Surface_Device::set_current();
  oldwindow = fl_window;
  fl_window = xid->offscreen();
#if FLTK_USE_CAIRO
  ((Fl_X11_Cairo_Graphics_Driver*)driver())->set_cairo(cairo_);
#endif
}

void Fl_Xlib_Copy_Surface_Driver::end_current() {
  fl_window = oldwindow;
  Fl_Surface_Device::end_current();
}

void Fl_Xlib_Copy_Surface_Driver::translate(int x, int y) {
#if FLTK_USE_CAIRO
  cairo_save(cairo_);
  cairo_translate(cairo_, x, y);
#else
  ((Fl_Xlib_Graphics_Driver*)driver())->translate_all(x, y);
#endif

}


void Fl_Xlib_Copy_Surface_Driver::untranslate() {
#if FLTK_USE_CAIRO
  cairo_restore(cairo_);
#else
  ((Fl_Xlib_Graphics_Driver*)driver())->untranslate_all();
#endif
}
