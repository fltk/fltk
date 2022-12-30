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

#ifndef FL_GDI_COPY_SURFACE_DRIVER_H
#define FL_GDI_COPY_SURFACE_DRIVER_H

#include <FL/Fl_Copy_Surface.H>
#include <FL/platform.H>

class Fl_GDI_Copy_Surface_Driver : public Fl_Copy_Surface_Driver {
  friend class Fl_Copy_Surface_Driver;
protected:
  HDC oldgc;
  HDC gc;
  Fl_GDI_Copy_Surface_Driver(int w, int h);
  ~Fl_GDI_Copy_Surface_Driver();
  void set_current() FL_OVERRIDE;
  void translate(int x, int y) FL_OVERRIDE;
  void untranslate() FL_OVERRIDE;
};

#endif // FL_GDI_COPY_SURFACE_DRIVER_H
