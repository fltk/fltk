//
// Draw-to-image code for the Fast Light Tool Kit (FLTK).
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

#ifndef FL_QUARTZ_IMAGE_SURFACE_DRIVER_H
#define FL_QUARTZ_IMAGE_SURFACE_DRIVER_H

#include <FL/Fl_Image_Surface.H>
#include <FL/platform.H>

class Fl_Quartz_Image_Surface_Driver : public Fl_Image_Surface_Driver {
  void end_current() override;
public:
  FLWindow *pre_window;
  Fl_Quartz_Image_Surface_Driver(int w, int h, int high_res, Fl_Offscreen off);
  ~Fl_Quartz_Image_Surface_Driver();
  void set_current() override;
  void translate(int x, int y) override;
  void untranslate() override;
  Fl_RGB_Image *image() override;
};

#endif // FL_QUARTZ_IMAGE_SURFACE_DRIVER_H
