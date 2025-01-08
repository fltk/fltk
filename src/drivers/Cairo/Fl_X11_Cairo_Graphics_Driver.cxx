//
// Support for using Cairo to draw into X11 windows for the Fast Light Tool Kit (FLTK).
//
// Copyright 2022-2023 by Bill Spitzak and others.
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

/* \file
    Implementation of class Fl_X11_Cairo_Graphics_Driver.
*/

#include "Fl_X11_Cairo_Graphics_Driver.H"
#include <FL/platform.H>
#include <cairo/cairo.h>
#include <pango/pangocairo.h>
#include <stdlib.h>


void *Fl_X11_Cairo_Graphics_Driver::gc_ = NULL;
GC fl_gc;


ulong fl_xpixel(uchar r,uchar g,uchar b) {
  return 0;
}
ulong fl_xpixel(Fl_Color i) {
    return 0;
}


void Fl_X11_Cairo_Graphics_Driver::scale(float f) {
  Fl_Graphics_Driver::scale(f);
  if (cairo_) {
    cairo_restore(cairo_);
    cairo_save(cairo_);
    cairo_scale(cairo_, f, f);
    cairo_translate(cairo_, 0.5, 0.5);
  }
}


void Fl_X11_Cairo_Graphics_Driver::copy_offscreen(int x, int y, int w, int h,
                                                  Fl_Offscreen pixmap, int srcx, int srcy) {
  cairo_matrix_t mat;
  if (cairo_) cairo_get_matrix(cairo_, &mat);
  else cairo_matrix_init_identity(&mat);
  XCopyArea(fl_display, pixmap, fl_window, (GC)Fl_Graphics_Driver::default_driver().gc(), int(srcx*scale()), int(srcy*scale()), int(w*scale()), int(h*scale()),
            int(x*scale()) + mat.x0, int(y*scale()) + mat.y0);
}


void Fl_X11_Cairo_Graphics_Driver::gc(void *value) {
  gc_ = value;
  fl_gc = (GC)gc_;
}


void *Fl_X11_Cairo_Graphics_Driver::gc() {
  return gc_;
}

extern FL_EXPORT cairo_t* fl_cairo_gc() {
  return ((Fl_Cairo_Graphics_Driver*)fl_graphics_driver)->cr();
}
