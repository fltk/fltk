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
private:
# if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
  CGImageRef mask_;
  void mask(const Fl_RGB_Image *) FL_OVERRIDE;
#endif
  void end_current() FL_OVERRIDE;
public:
  FLWindow *pre_window;
  Fl_Quartz_Image_Surface_Driver(int w, int h, int high_res, Fl_Offscreen off);
  ~Fl_Quartz_Image_Surface_Driver();
  void set_current() FL_OVERRIDE;
  void translate(int x, int y) FL_OVERRIDE;
  void untranslate() FL_OVERRIDE;
  Fl_RGB_Image *image() FL_OVERRIDE;
};

#endif // FL_QUARTZ_IMAGE_SURFACE_DRIVER_H
