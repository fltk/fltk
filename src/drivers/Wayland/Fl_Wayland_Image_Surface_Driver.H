//
// Draw-to-image code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2022 by Bill Spitzak and others.
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

#ifndef FL_WAYLAND_IMAGE_SURFACE_DRIVER_H
#define FL_WAYLAND_IMAGE_SURFACE_DRIVER_H

#include <FL/Fl_Image_Surface.H>

class Fl_Wayland_Image_Surface_Driver : public Fl_Image_Surface_Driver {
  void end_current() FL_OVERRIDE;
  struct wld_window *pre_window;
public:
  Fl_Wayland_Image_Surface_Driver(int w, int h, int high_res, Fl_Offscreen off);
  ~Fl_Wayland_Image_Surface_Driver();
  void mask(const Fl_RGB_Image *) FL_OVERRIDE;
  struct shape_data_type {
    double scale;
    cairo_pattern_t *mask_pattern_;
    cairo_t *bg_cr;
  } *shape_data_;
  void set_current() FL_OVERRIDE;
  void translate(int x, int y) FL_OVERRIDE;
  void untranslate() FL_OVERRIDE;
  Fl_RGB_Image *image() FL_OVERRIDE;
};

#endif // FL_WAYLAND_IMAGE_SURFACE_DRIVER_H
