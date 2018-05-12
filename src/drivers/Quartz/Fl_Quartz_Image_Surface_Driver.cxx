//
// "$Id$"
//
// Draw-to-image code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2018 by Bill Spitzak and others.
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

#ifdef FL_CFG_GFX_QUARTZ
#include <FL/platform.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Image_Surface.H>
#include "Fl_Quartz_Graphics_Driver.H"
#include "../Cocoa/Fl_Cocoa_Window_Driver.H"
#include <ApplicationServices/ApplicationServices.h>

class Fl_Quartz_Image_Surface_Driver : public Fl_Image_Surface_Driver {
  friend class Fl_Image_Surface;
  virtual void end_current_();
public:
  Window pre_window;
  Fl_Quartz_Image_Surface_Driver(int w, int h, int high_res, Fl_Offscreen off);
  ~Fl_Quartz_Image_Surface_Driver();
  void set_current();
  void translate(int x, int y);
  void untranslate();
  Fl_RGB_Image *image();
};


Fl_Image_Surface_Driver *Fl_Image_Surface_Driver::newImageSurfaceDriver(int w, int h, int high_res, Fl_Offscreen off)
{
  return new Fl_Quartz_Image_Surface_Driver(w, h, high_res, off);
}


Fl_Quartz_Image_Surface_Driver::Fl_Quartz_Image_Surface_Driver(int w, int h, int high_res, Fl_Offscreen off) : Fl_Image_Surface_Driver(w, h, high_res, 0) {
  int W = w, H = h;
  float s = 1;
  if (high_res) {
    s = Fl_Graphics_Driver::default_driver().scale();
    Fl_Window *cw = Fl_Window::current();
    if (cw && Fl_Cocoa_Window_Driver::driver(cw)->mapped_to_retina()) s *= 2;
    W *= s; H *= s;
  }
  CGColorSpaceRef lut = CGColorSpaceCreateDeviceRGB();
  offscreen = off ? off : CGBitmapContextCreate(calloc(W*H,4), W, H, 8, W*4, lut, kCGImageAlphaPremultipliedLast);
  CGColorSpaceRelease(lut);
  driver(new Fl_Quartz_Graphics_Driver);
  CGContextTranslateCTM(offscreen, 0.5, -0.5); // as when drawing to a window
  if (high_res) {
    CGContextScaleCTM(offscreen, s, s);
  }
  CGContextSetShouldAntialias(offscreen, false);
  CGContextTranslateCTM(offscreen, 0, height);  
  CGContextScaleCTM(offscreen, 1.0f, -1.0f);
  CGContextSaveGState(offscreen);
  CGContextSetRGBFillColor(offscreen, 1, 1, 1, 0);
  CGContextFillRect(offscreen, CGRectMake(0,0,w,h));
}

Fl_Quartz_Image_Surface_Driver::~Fl_Quartz_Image_Surface_Driver() {
  if (offscreen && !external_offscreen) {
    void *data = CGBitmapContextGetData(offscreen);
    free(data);
    CGContextRelease((CGContextRef)offscreen);
  }
  delete driver();
}

void Fl_Quartz_Image_Surface_Driver::set_current() {
  Fl_Surface_Device::set_current();
  pre_window = fl_window;
  driver()->gc(offscreen);
  fl_window = 0;
  ((Fl_Quartz_Graphics_Driver*)driver())->high_resolution( CGBitmapContextGetWidth(offscreen) > width );
}

void Fl_Quartz_Image_Surface_Driver::translate(int x, int y) {
  CGContextRestoreGState(offscreen);
  CGContextSaveGState(offscreen);
  CGContextTranslateCTM(offscreen, x, y);
  CGContextSaveGState(offscreen);
}

void Fl_Quartz_Image_Surface_Driver::untranslate() {
  CGContextRestoreGState(offscreen);
}

Fl_RGB_Image* Fl_Quartz_Image_Surface_Driver::image()
{
  CGContextFlush(offscreen);
  int W = CGBitmapContextGetWidth(offscreen);
  int H = CGBitmapContextGetHeight(offscreen);
  int save_w = width, save_h = height;
  width = W; height = H;
  unsigned char *data = fl_read_image(NULL, 0, 0, W, H, 0);
  width = save_w; height = save_h;
  Fl_RGB_Image *image = new Fl_RGB_Image(data, W, H);
  image->alloc_array = 1;
  return image;
}

void Fl_Quartz_Image_Surface_Driver::end_current_()
{
  fl_window = pre_window;
}

#endif // FL_CFG_GFX_QUARTZ

//
// End of "$Id$".
//
