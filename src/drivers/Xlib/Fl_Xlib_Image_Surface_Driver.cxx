//
// Draw-to-image code for the Fast Light Tool Kit (FLTK).
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

#include <FL/platform.H>
#include "Fl_Xlib_Image_Surface_Driver.H"
#include "../../Fl_Screen_Driver.H"
#if FLTK_USE_CAIRO
#  include <cairo-xlib.h>
#  include "../Cairo/Fl_Display_Cairo_Graphics_Driver.H"
#else
#  include "Fl_Xlib_Graphics_Driver.H"
#endif // FLTK_USE_CAIRO



Fl_Xlib_Image_Surface_Driver::Fl_Xlib_Image_Surface_Driver(int w, int h, int high_res, Fl_Offscreen off) : Fl_Image_Surface_Driver(w, h, high_res, off) {
  float d = 1;
  if (!off) {
    fl_open_display();
    d =  Fl_Graphics_Driver::default_driver().scale();
    if (d != 1 && high_res) {
      w = int(w*d);
      h = int(h*d);
    }
    offscreen = (Fl_Offscreen)XCreatePixmap(fl_display, RootWindow(fl_display, fl_screen), w, h, fl_visual->depth);
  }
#if FLTK_USE_CAIRO
  driver(new Fl_Display_Cairo_Graphics_Driver());
  cairo_surface_t *s = cairo_xlib_surface_create(fl_display, offscreen, fl_visual->visual, w, h);
  cairo_ = cairo_create(s);
  cairo_surface_destroy(s);
  cairo_save(cairo_);
  ((Fl_Display_Cairo_Graphics_Driver*)driver())->set_cairo(cairo_);
#else
  driver(new Fl_Xlib_Graphics_Driver());
#endif
  if (d != 1 && high_res) driver()->scale(d);
}

Fl_Xlib_Image_Surface_Driver::~Fl_Xlib_Image_Surface_Driver() {
#if FLTK_USE_CAIRO
  cairo_destroy(cairo_);
#endif
  if (offscreen && !external_offscreen) XFreePixmap(fl_display, (Pixmap)offscreen);
  delete driver();
}

void Fl_Xlib_Image_Surface_Driver::set_current() {
  Fl_Surface_Device::set_current();
  pre_window = fl_window;
  fl_window = offscreen;
#if FLTK_USE_CAIRO
  ((Fl_Display_Cairo_Graphics_Driver*)driver())->set_cairo(cairo_);
#endif
}

void Fl_Xlib_Image_Surface_Driver::translate(int x, int y) {
#if FLTK_USE_CAIRO
  cairo_save(cairo_);
  cairo_translate(cairo_, x, y);
#else
  ((Fl_Xlib_Graphics_Driver*)driver())->translate_all(x, y);
#endif
}

void Fl_Xlib_Image_Surface_Driver::untranslate() {
#if FLTK_USE_CAIRO
  cairo_restore(cairo_);
#else
  ((Fl_Xlib_Graphics_Driver*)driver())->untranslate_all();
#endif
}

Fl_RGB_Image* Fl_Xlib_Image_Surface_Driver::image()
{
  Fl_RGB_Image *image = Fl::screen_driver()->read_win_rectangle(0, 0, width, height, 0);
  return image;
}

void Fl_Xlib_Image_Surface_Driver::end_current()
{
  fl_window = pre_window;
  Fl_Surface_Device::end_current();
}
