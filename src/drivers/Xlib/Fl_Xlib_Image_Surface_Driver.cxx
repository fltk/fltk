//
// "$Id$"
//
// Draw-to-image code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2017 by Bill Spitzak and others.
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
#include <FL/Fl_Image_Surface.H>
#include "Fl_Xlib_Graphics_Driver.H"
#include <FL/Fl_Screen_Driver.H>

class Fl_Xlib_Image_Surface_Driver : public Fl_Image_Surface_Driver {
  virtual void end_current_(Fl_Surface_Device *next_current);
public:
  Window pre_window;
  int was_high;
  Fl_Xlib_Image_Surface_Driver(int w, int h, int high_res, Fl_Offscreen off);
  ~Fl_Xlib_Image_Surface_Driver();
  void set_current();
  void translate(int x, int y);
  void untranslate();
  Fl_RGB_Image *image();
};

Fl_Image_Surface_Driver *Fl_Image_Surface_Driver::newImageSurfaceDriver(int w, int h, int high_res, Fl_Offscreen off)
{
  return new Fl_Xlib_Image_Surface_Driver(w, h, high_res, off);
}

Fl_Xlib_Image_Surface_Driver::Fl_Xlib_Image_Surface_Driver(int w, int h, int high_res, Fl_Offscreen off) : Fl_Image_Surface_Driver(w, h, high_res, off) {
  float d = 1;
  if (!off) {
    fl_open_display();
    d =  fl_graphics_driver->scale();
    if (d != 1 && high_res) {
      w = int(w*d);
      h = int(h*d);
    }
    offscreen = XCreatePixmap(fl_display, RootWindow(fl_display, fl_screen), w, h, fl_visual->depth);
  }
  driver(new Fl_Xlib_Graphics_Driver());
  if (d != 1 && high_res) ((Fl_Xlib_Graphics_Driver*)driver())->scale(d);
}

Fl_Xlib_Image_Surface_Driver::~Fl_Xlib_Image_Surface_Driver() {
  if (offscreen) XFreePixmap(fl_display, offscreen);
  delete driver();
}

void Fl_Xlib_Image_Surface_Driver::set_current() {
  pre_window = fl_window;
  Fl_Surface_Device::set_current();
  fl_window = offscreen;
  fl_push_no_clip();
}

void Fl_Xlib_Image_Surface_Driver::translate(int x, int y) {
  ((Fl_Xlib_Graphics_Driver*)driver())->translate_all(x, y);
}

void Fl_Xlib_Image_Surface_Driver::untranslate() {
  ((Fl_Xlib_Graphics_Driver*)driver())->untranslate_all();
}

Fl_RGB_Image* Fl_Xlib_Image_Surface_Driver::image()
{
  Fl_RGB_Image *image = Fl::screen_driver()->read_win_rectangle(NULL, 0, 0, width, height, 0);
  return image;
}

void Fl_Xlib_Image_Surface_Driver::end_current_(Fl_Surface_Device *next_current)
{
  fl_pop_clip();
  next_current->driver()->restore_clip();
  fl_window = pre_window;
}

#endif // FL_CFG_GFX_XLIB

//
// End of "$Id$".
//
