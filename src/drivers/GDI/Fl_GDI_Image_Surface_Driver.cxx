//
// "$Id$"
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


#include "../../config_lib.h"

#ifdef FL_CFG_GFX_GDI
#include "Fl_GDI_Graphics_Driver.H"
#include <FL/Fl_Image_Surface.H>
#include <FL/fl_draw.H>
#include <FL/x.H>
#include <windows.h>

class Fl_GDI_Image_Surface_Driver : public Fl_Image_Surface_Driver {
  friend class Fl_Image_Surface;
  virtual void end_current_();
public:
  Fl_Surface_Device *previous;
  Window pre_window;
  HDC _sgc;
  int _savedc;
  Fl_GDI_Image_Surface_Driver(int w, int h, int high_res);
  ~Fl_GDI_Image_Surface_Driver();
  void set_current();
  void translate(int x, int y);
  void untranslate();
  Fl_RGB_Image *image();
};


Fl_Image_Surface_Driver *Fl_Image_Surface_Driver::newImageSurfaceDriver(int w, int h, int high_res, Fl_Offscreen off)
{
  return new Fl_GDI_Image_Surface_Driver(w, h, high_res);
}


Fl_GDI_Image_Surface_Driver::Fl_GDI_Image_Surface_Driver(int w, int h, int high_res) : Fl_Image_Surface_Driver(w, h, high_res, 0) {
  previous = 0;
  offscreen = CreateCompatibleBitmap( (fl_graphics_driver->gc() ? (HDC)fl_graphics_driver->gc() : fl_GetDC(0) ) , w, h);
  driver(new Fl_Translated_GDI_Graphics_Driver);
  _sgc = NULL;
}


Fl_GDI_Image_Surface_Driver::~Fl_GDI_Image_Surface_Driver() {
  if (offscreen) DeleteObject(offscreen);
  delete driver();
}


void Fl_GDI_Image_Surface_Driver::set_current() {
  pre_window = fl_window;
  if (!previous) previous = Fl_Surface_Device::surface();
  if (!_sgc) _sgc = (HDC)previous->driver()->gc();
  HDC gc = fl_makeDC(offscreen);
  Fl_Surface_Device::set_current();
  driver()->gc(gc);
  _savedc = SaveDC(gc);
  fl_window=(HWND)offscreen;
  fl_push_no_clip();
}


void Fl_GDI_Image_Surface_Driver::translate(int x, int y) {
  ((Fl_Translated_GDI_Graphics_Driver*)driver())->translate_all(x, y);
}


void Fl_GDI_Image_Surface_Driver::untranslate() {
  ((Fl_Translated_GDI_Graphics_Driver*)driver())->untranslate_all();
}


Fl_RGB_Image* Fl_GDI_Image_Surface_Driver::image()
{
  unsigned char *data;
  data = fl_read_image(NULL, 0, 0, width, height, 0);
  previous->driver()->gc(_sgc);
  Fl_RGB_Image *image = new Fl_RGB_Image(data, width, height);
  image->alloc_array = 1;
  return image;
}


void Fl_GDI_Image_Surface_Driver::end_current_()
{
  HDC gc = (HDC)driver()->gc();
  RestoreDC(gc, _savedc);
  DeleteDC(gc);
  fl_pop_clip();
  fl_window = pre_window;
}

#endif // FL_CFG_GFX_GDI

//
// End of "$Id$".
//
