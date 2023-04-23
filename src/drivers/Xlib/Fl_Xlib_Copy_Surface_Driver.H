//
// Copy-to-clipboard code for the Fast Light Tool Kit (FLTK).
//
// Copyright 2022 by Bill Spitzak and others.
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

#ifndef FL_XLIB_COPY_SURFACE_DRIVER_H
#define FL_XLIB_COPY_SURFACE_DRIVER_H

#include <FL/Fl_Copy_Surface.H>
#include <FL/platform.H>
#if FLTK_USE_CAIRO
#  include <cairo/cairo.h>
#endif // FLTK_USE_CAIRO

class Fl_Image_Surface;

class Fl_Xlib_Copy_Surface_Driver : public Fl_Copy_Surface_Driver {
  friend class Fl_Copy_Surface_Driver;
  void end_current() FL_OVERRIDE;
protected:
  Fl_Image_Surface *xid;
  Window oldwindow;
  Fl_Xlib_Copy_Surface_Driver(int w, int h);
  ~Fl_Xlib_Copy_Surface_Driver();
  void set_current() FL_OVERRIDE;
  void translate(int x, int y) FL_OVERRIDE;
  void untranslate() FL_OVERRIDE;
#if FLTK_USE_CAIRO
  cairo_t *cairo_;
#endif
};

#endif // FL_XLIB_COPY_SURFACE_DRIVER_H
