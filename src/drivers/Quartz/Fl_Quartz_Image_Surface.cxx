//
// "$Id: Fl_Quartz_Image_Surface.cxx 11278 2016-03-03 19:16:22Z manolo $"
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

#include "config_lib.h"


#ifdef FL_CFG_GFX_QUARTZ
#include "Fl_Quartz_Graphics_Driver.H"
#include "Fl_Quartz_Image_Surface.H"

#include <ApplicationServices/ApplicationServices.h>


Fl_Image_Surface::Helper::Helper(int w, int h, int high_res) : Fl_Widget_Surface(NULL), width(w), height(h) {
  previous = 0;
  int W = high_res ? 2*w : w;
  int H = high_res ? 2*h : h;
  CGColorSpaceRef lut = CGColorSpaceCreateDeviceRGB();
  offscreen = CGBitmapContextCreate(calloc(W*H,4), W, H, 8, W*4, lut, kCGImageAlphaPremultipliedLast);
  CGColorSpaceRelease(lut);
  driver(new Fl_Quartz_Graphics_Driver);
  CGContextTranslateCTM(offscreen, -0.5, 0.5); // as when drawing to a window
  if (high_res) {
    CGContextScaleCTM(offscreen, 2, 2);
  }
  CGContextSetShouldAntialias(offscreen, false);
  CGContextSaveGState(offscreen);
  CGContextTranslateCTM(offscreen, 0, height);
  CGContextScaleCTM(offscreen, 1.0f, -1.0f);
  CGContextSetRGBFillColor(offscreen, 1, 1, 1, 0);
  CGContextFillRect(offscreen, CGRectMake(0,0,w,h));
}

Fl_Image_Surface::Helper::~Helper() {
  if (offscreen) {
    void *data = CGBitmapContextGetData((CGContextRef)offscreen);
    free(data);
    CGContextRelease((CGContextRef)offscreen);
  }
}

void Fl_Image_Surface::Helper::set_current() {
  pre_window = fl_window;
  if (!previous) previous = Fl_Surface_Device::surface();
  driver()->gc(offscreen);
  fl_window = 0;
  Fl_Surface_Device::set_current();
  was_high = Fl_Display_Device::high_resolution();
  Fl_X::set_high_resolution( CGBitmapContextGetWidth(offscreen) > width );
}

void Fl_Image_Surface::Helper::translate(int x, int y) {
  CGContextRef gc = (CGContextRef)driver()->gc();
  CGContextRestoreGState(gc);
  CGContextSaveGState(gc);
  CGContextTranslateCTM(gc, x, -y);
  CGContextSaveGState(gc);
  CGContextTranslateCTM(gc, 0, height);
  CGContextScaleCTM(gc, 1.0f, -1.0f);
}

void Fl_Image_Surface::Helper::untranslate() {
  CGContextRestoreGState((CGContextRef)driver()->gc());
}

Fl_RGB_Image* Fl_Image_Surface::Helper::image()
{
  unsigned char *data;
  int W = width, H = height;
  CGContextFlush(offscreen);
  W = CGBitmapContextGetWidth(offscreen);
  H = CGBitmapContextGetHeight(offscreen);
  data = fl_read_image(NULL, 0, 0, W, H, 0);
  Fl_RGB_Image *image = new Fl_RGB_Image(data, W, H);
  image->alloc_array = 1;
  return image;
}

void Fl_Image_Surface::Helper::end_current()
{
  Fl_X::set_high_resolution(was_high);
  previous->Fl_Surface_Device::set_current();
  fl_window = pre_window;
}

#endif // FL_CFG_GFX_QUARTZ

//
// End of "$Id: Fl_Quartz_Image_Surface.cxx 11220 2016-02-26 12:51:47Z manolo $".
//
