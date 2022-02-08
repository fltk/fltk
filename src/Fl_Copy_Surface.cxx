//
// Copy-to-clipboard code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2016 by Bill Spitzak and others.
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

#include <FL/Fl_Copy_Surface.H>

/** the constructor
\param w, h Width and height of the drawing surface in FLTK units */
Fl_Copy_Surface::Fl_Copy_Surface(int w, int h) : Fl_Widget_Surface(NULL) {
  platform_surface = Fl_Copy_Surface_Driver::newCopySurfaceDriver(w, h);
  if (platform_surface) driver(platform_surface->driver());
}

/** the destructor */
Fl_Copy_Surface::~Fl_Copy_Surface() { delete platform_surface; }

void Fl_Copy_Surface::origin(int x, int y) {platform_surface->origin(x, y);}

void Fl_Copy_Surface::origin(int *x, int *y) {
  if (platform_surface) platform_surface->origin(x, y);
}

void Fl_Copy_Surface::set_current() {
  if (platform_surface) platform_surface->set_current();
}

bool Fl_Copy_Surface::is_current() {
  return surface() == platform_surface;
}

void Fl_Copy_Surface::translate(int x, int y) {
  if (platform_surface) platform_surface->translate(x, y);
}

void Fl_Copy_Surface::untranslate() {
  if (platform_surface) platform_surface->untranslate();
}

int Fl_Copy_Surface::w() {return platform_surface ? platform_surface->width : 0;}

int Fl_Copy_Surface::h() {return platform_surface ? platform_surface->height : 0;}

int Fl_Copy_Surface::printable_rect(int *w, int *h)  {
  if (platform_surface)
    return platform_surface->printable_rect(w, h);
  else {
    *w  = *h = 0;
  }
  return 1;
}

/**
 \cond DriverDev
 \addtogroup DriverDeveloper
 \{
 */
int Fl_Copy_Surface_Driver::printable_rect(int *w, int *h) {
  *w = width; *h = height;
  return 0;
}

/**
 \}
 \endcond
 */
