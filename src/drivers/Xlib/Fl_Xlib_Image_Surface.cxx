//
// "$Id: Fl_Xlib_Image_Surface.cxx 11278 2016-03-03 19:16:22Z manolo $"
//
// Draw-to-image code for the Fast Light Tool Kit (FLTK).
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

#include <FL/fl_draw.H>

#include "../../config_lib.h"


#ifdef FL_CFG_GFX_XLIB
#include "Fl_Xlib_Graphics_Driver.H"
#include "Fl_Xlib_Image_Surface.H"
#include "Fl_Translated_Xlib_Graphics_Driver.H"

Fl_Image_Surface::Helper::Helper(int w, int h, int high_res) : Fl_Widget_Surface(NULL) {
  width = w;
  height = h;
  previous = 0;
  offscreen = XCreatePixmap(fl_display, RootWindow(fl_display, fl_screen), w, h, fl_visual->depth);
  driver(new Fl_Translated_Xlib_Graphics_Driver());
}

Fl_Image_Surface::Helper::Helper(Fl_Offscreen pixmap, int w, int h) : Fl_Widget_Surface(NULL) {
  width = w;
  height = h;
  previous = 0;
  offscreen = pixmap;
  driver(new Fl_Translated_Xlib_Graphics_Driver());
}

Fl_Image_Surface::Helper::~Helper() {
  if (offscreen) XFreePixmap(fl_display, offscreen);
}

void Fl_Image_Surface::Helper::set_current() {
  pre_window = fl_window;
  if (!previous) previous = Fl_Surface_Device::surface();
  fl_window = offscreen;
  Fl_Surface_Device::set_current();
  fl_push_no_clip();
}

void Fl_Image_Surface::Helper::translate(int x, int y) {
  ((Fl_Translated_Xlib_Graphics_Driver*)driver())->translate_all(x, y);
}

void Fl_Image_Surface::Helper::untranslate() {
  ((Fl_Translated_Xlib_Graphics_Driver*)driver())->untranslate_all();
}

Fl_RGB_Image* Fl_Image_Surface::Helper::image()
{
  unsigned char *data;
  data = fl_read_image(NULL, 0, 0, width, height, 0);
  end_current();
  Fl_RGB_Image *image = new Fl_RGB_Image(data, width, height);
  image->alloc_array = 1;
  return image;
}

void Fl_Image_Surface::Helper::end_current()
{
  fl_pop_clip();
  previous->Fl_Surface_Device::set_current();
  fl_window = pre_window;
}

#endif // FL_CFG_GFX_XLIB

//
// End of "$Id: Fl_Xlib_Image_Surface.cxx 11220 2016-02-26 12:51:47Z manolo $".
//
