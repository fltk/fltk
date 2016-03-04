//
// "$Id: Fl_Xlib_Copy_Surface.cxx 11241 2016-02-27 13:52:27Z manolo $"
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

#ifdef FL_CFG_GFX_XLIB
#include "Fl_Xlib_Copy_Surface.H"
#include <FL/Fl.H>
#include <FL/fl_draw.H>

Fl_Copy_Surface::Helper::Helper(int w, int h) : Fl_Widget_Surface(NULL), width(w), height(h) {
  driver(new Fl_Translated_Xlib_Graphics_Driver());
  Fl::first_window()->make_current();
  oldwindow = fl_xid(Fl::first_window());
  xid = fl_create_offscreen(w,h);
  _ss = NULL;
  Fl_Surface_Device *present_surface = Fl_Surface_Device::surface();
  set_current();
  fl_color(FL_WHITE);
  fl_rectf(0, 0, w, h);
  present_surface->set_current();
}

Fl_Copy_Surface::Helper::~Helper() {
  fl_pop_clip();
  unsigned char *data = fl_read_image(NULL,0,0,width,height,0);
  fl_window = oldwindow;
  _ss->set_current();
  Fl::copy_image(data,width,height,1);
  delete[] data;
  fl_delete_offscreen(xid);
}

void Fl_Copy_Surface::Helper::set_current() {
  fl_window=xid;
  if (!_ss) _ss = Fl_Surface_Device::surface();
  Fl_Surface_Device::set_current();
  fl_push_no_clip();
}

void Fl_Copy_Surface::Helper::translate(int x, int y) {
  ((Fl_Translated_Xlib_Graphics_Driver*)driver())->translate_all(x, y);
}

void Fl_Copy_Surface::Helper::untranslate() {
  ((Fl_Translated_Xlib_Graphics_Driver*)driver())->untranslate_all();
}

#endif // FL_CFG_GFX_XLIB

//
// End of "$Id: Fl_Copy_Surface.H 11220 2016-02-26 12:51:47Z manolo $".
//
