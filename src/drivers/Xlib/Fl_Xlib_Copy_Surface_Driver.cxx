//
// Copy-to-clipboard code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2018 by Bill Spitzak and others.
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

#include <config.h>
#include <FL/Fl_Copy_Surface.H>
#include <FL/Fl.H>
#include <FL/platform.H>
#include <FL/fl_draw.H>
#include "Fl_Xlib_Graphics_Driver.H"
#include "../X11/Fl_X11_Screen_Driver.H"

class Fl_Xlib_Copy_Surface_Driver : public Fl_Copy_Surface_Driver {
  friend class Fl_Copy_Surface_Driver;
  virtual void end_current();
protected:
  Fl_Offscreen xid;
  Window oldwindow;
  Fl_Xlib_Copy_Surface_Driver(int w, int h);
  ~Fl_Xlib_Copy_Surface_Driver();
  void set_current();
  void translate(int x, int y);
  void untranslate();
};


Fl_Copy_Surface_Driver *Fl_Copy_Surface_Driver::newCopySurfaceDriver(int w, int h)
{
  return new Fl_Xlib_Copy_Surface_Driver(w, h);
}


Fl_Xlib_Copy_Surface_Driver::Fl_Xlib_Copy_Surface_Driver(int w, int h) : Fl_Copy_Surface_Driver(w, h) {
  driver(new Fl_Xlib_Graphics_Driver());
  float s = Fl_Graphics_Driver::default_driver().scale();
  ((Fl_Xlib_Graphics_Driver*)driver())->scale(s);
  oldwindow = fl_window;
  xid = fl_create_offscreen(w,h);
  driver()->push_no_clip();
  fl_window = xid;
  driver()->color(FL_WHITE);
  driver()->rectf(0, 0, w, h);
  fl_window = oldwindow;
}


Fl_Xlib_Copy_Surface_Driver::~Fl_Xlib_Copy_Surface_Driver() {
  driver()->pop_clip();
  Window old_win = fl_window;
  fl_window = xid;
  Fl_RGB_Image *rgb = Fl::screen_driver()->read_win_rectangle(0, 0, width, height, 0);
  fl_window = old_win;
  if (is_current()) end_current();
  Fl_X11_Screen_Driver::copy_image(rgb->array, rgb->w(), rgb->h(), 1);
  delete rgb;
  fl_delete_offscreen(xid);
  delete driver();
}


void Fl_Xlib_Copy_Surface_Driver::set_current() {
  Fl_Surface_Device::set_current();
  oldwindow = fl_window;
  fl_window = xid;
}

void Fl_Xlib_Copy_Surface_Driver::end_current() {
  fl_window = oldwindow;
  Fl_Surface_Device::end_current();
}

void Fl_Xlib_Copy_Surface_Driver::translate(int x, int y) {
  ((Fl_Xlib_Graphics_Driver*)driver())->translate_all(x, y);
}


void Fl_Xlib_Copy_Surface_Driver::untranslate() {
  ((Fl_Xlib_Graphics_Driver*)driver())->untranslate_all();
}
