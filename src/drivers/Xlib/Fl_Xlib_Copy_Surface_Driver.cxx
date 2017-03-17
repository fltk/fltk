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

#include "../../config_lib.h"

#ifdef FL_CFG_GFX_XLIB
#include <FL/Fl_Copy_Surface.H>
#include <FL/Fl.H>
#include <FL/x.H>
#include <FL/fl_draw.H>
#include "Fl_Xlib_Graphics_Driver.H"
#include "../X11/Fl_X11_Screen_Driver.H"

class Fl_Xlib_Copy_Surface_Driver : public Fl_Copy_Surface_Driver {
  friend class Fl_Copy_Surface_Driver;
  virtual void end_current_();
protected:
  Fl_Offscreen xid;
  Window oldwindow;
  Fl_Xlib_Copy_Surface_Driver(int w, int h);
  ~Fl_Xlib_Copy_Surface_Driver();
  void set_current();
  void translate(int x, int y);
  void untranslate();
  int w() {return width;}
  int h() {return height;}
  int printable_rect(int *w, int *h) {*w = width; *h = height; return 0;}
};


Fl_Copy_Surface_Driver *Fl_Copy_Surface_Driver::newCopySurfaceDriver(int w, int h)
{
  return new Fl_Xlib_Copy_Surface_Driver(w, h);
}


Fl_Xlib_Copy_Surface_Driver::Fl_Xlib_Copy_Surface_Driver(int w, int h) : Fl_Copy_Surface_Driver(w, h) {
  driver(new Fl_Xlib_Graphics_Driver());
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
  bool need_push = (Fl_Surface_Device::surface() != this);
  if (need_push) Fl_Surface_Device::push_current(this);
  unsigned char *data = fl_read_image(NULL,0,0,width,height,0);
  if (need_push) Fl_Surface_Device::pop_current();
  Fl_X11_Screen_Driver::copy_image(data, width, height, 1);
  delete[] data;
  fl_delete_offscreen(xid);
  delete driver();
}


void Fl_Xlib_Copy_Surface_Driver::set_current() {
  oldwindow = fl_window;
  fl_window = xid;
  Fl_Surface_Device::set_current();
}

void Fl_Xlib_Copy_Surface_Driver::end_current_() {
  fl_window = oldwindow;
}

void Fl_Xlib_Copy_Surface_Driver::translate(int x, int y) {
  ((Fl_Xlib_Graphics_Driver*)driver())->translate_all(x, y);
}


void Fl_Xlib_Copy_Surface_Driver::untranslate() {
  ((Fl_Xlib_Graphics_Driver*)driver())->untranslate_all();
}

#endif // FL_CFG_GFX_XLIB

//
// End of "$Id$".
//
