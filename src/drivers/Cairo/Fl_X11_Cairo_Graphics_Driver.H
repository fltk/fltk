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
    Declaration of class Fl_X11_Cairo_Graphics_Driver.
*/

#ifndef FL_DISPLAY_CAIRO_GRAPHICS_DRIVER_H
# define FL_DISPLAY_CAIRO_GRAPHICS_DRIVER_H

#include "Fl_Cairo_Graphics_Driver.H"

class Fl_X11_Cairo_Graphics_Driver : public Fl_Cairo_Graphics_Driver {
private:
  static void *gc_;
public:
  void scale(float f) FL_OVERRIDE;
  float scale() {return Fl_Graphics_Driver::scale();}
  void gc(void *value) FL_OVERRIDE;
  void* gc() FL_OVERRIDE;
  void copy_offscreen(int x, int y, int w, int h, Fl_Offscreen pixmap, int srcx, int srcy) FL_OVERRIDE;
 };

#endif // FL_DISPLAY_CAIRO_GRAPHICS_DRIVER_H
