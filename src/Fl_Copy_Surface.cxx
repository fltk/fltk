//
// "$Id$"
//
// Copy-to-clipboard code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2016 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

#include <FL/Fl_Copy_Surface.H>

#if defined(FL_PORTING)
# pragma message "FL_PORTING: optionally implement class Fl_XXX_Copy_Surface_Driver for your platform"

Fl_Copy_Surface_Driver *Fl_Copy_Surface_Driver::newCopySurfaceDriver(int w, int h)
{
  return NULL;
}

#endif

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

//
// End of "$Id$".
//
