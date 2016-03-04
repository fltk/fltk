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

#include "config_lib.h"
#include <FL/Fl_Copy_Surface.H>

#if !defined(FL_DOXYGEN)
#ifdef __APPLE__
#include <src/drivers/Quartz/Fl_Quartz_Copy_Surface.H>

#elif defined(WIN32)
#include <src/drivers/GDI/Fl_GDI_Copy_Surface.H>

#elif defined(FL_PORTING)
# pragma message "FL_PORTING: implement class Fl_Copy_Surface::Helper for your platform"

class Fl_Copy_Surface::Helper : public Fl_Widget_Surface { // class model
  friend class Fl_Copy_Surface;
private:
  int width;
  int height;
  Helper(int w, int h) : Fl_Widget_Surface(NULL), width(w), height(h) {} // to implement
  ~Helper() {} // to implement
  void set_current(){} // to implement
  void translate(int x, int y) {} // to implement
  void untranslate() {} // to implement
  int w() {return width;}
  int h() {return height;}
  int printable_rect(int *w, int *h) {*w = width; *h = height; return 0;}
};

#else
#include <src/drivers/Xlib/Fl_Xlib_Copy_Surface.H>

#endif
#endif // !FL_DOXYGEN

/** the constructor */
Fl_Copy_Surface::Fl_Copy_Surface(int w, int h) : Fl_Widget_Surface(NULL) {
  platform_surface = new Helper(w, h);
  driver(platform_surface->driver());
}

Fl_Copy_Surface::~Fl_Copy_Surface() { delete platform_surface; }

void Fl_Copy_Surface::origin(int x, int y) {platform_surface->origin(x, y);}

void Fl_Copy_Surface::origin(int *x, int *y) {platform_surface->origin(x, y);}

void Fl_Copy_Surface::set_current() {platform_surface->set_current();}

void Fl_Copy_Surface::translate(int x, int y) {platform_surface->translate(x, y);}

void Fl_Copy_Surface::untranslate() {platform_surface->untranslate();}

int Fl_Copy_Surface::w() {return platform_surface->w();}

int Fl_Copy_Surface::h() {return platform_surface->h();}

int Fl_Copy_Surface::printable_rect(int *w, int *h)  {return platform_surface->printable_rect(w, h);}

//
// End of "$Id$".
//
