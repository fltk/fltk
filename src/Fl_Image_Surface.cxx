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

#include <FL/Fl_Image_Surface.H>
#include <FL/Fl_Printer.H>
#include <FL/Fl.H>

#include "config_lib.h"
#ifdef FL_CFG_GFX_QUARTZ
#include "drivers/Quartz/Fl_Quartz_Graphics_Driver.H"
#endif
#ifdef FL_CFG_GFX_GDI
#include "drivers/GDI/Fl_GDI_Graphics_Driver.H"
#endif
#ifdef FL_CFG_GFX_XLIB
#include "drivers/Xlib/Fl_Translated_Xlib_Graphics_Driver.H"
#endif

#if defined(WIN32)
#elif defined(__APPLE__) // PORTME: Fl_Surface_Driver - platform image surface
#elif defined(FL_PORTING)
#  pragma message "FL_PORTING: implement image surface handling here"
#else
#endif

/** Constructor with optional high resolution.
 \param w and \param h give the size in pixels of the resulting image.
 \param highres if non-zero, the surface pixel size is twice as high and wide as w and h,
 which is useful to draw it later on a high resolution display (e.g., retina display).
 This is implemented for the Mac OS platform only.
 If \p highres is non-zero, use Fl_Image_Surface::highres_image() to get the image data.
 \version 1.3.4 (1.3.3 without the highres parameter)
 */
Fl_Image_Surface::Fl_Image_Surface(int w, int h, int highres) : Fl_Widget_Surface(NULL) {
  width = w;
  height = h;
#ifdef __APPLE__ // PORTME: platform image surface
  offscreen = fl_create_offscreen(highres ? 2*w : w, highres ? 2*h : h);
  driver(new Fl_Quartz_Graphics_Driver);
  if (highres) {
    CGContextScaleCTM(offscreen, 2, 2);
  }
  CGContextSetShouldAntialias(offscreen, false);
  CGContextSaveGState(offscreen);
  CGContextTranslateCTM(offscreen, 0, height);
  CGContextScaleCTM(offscreen, 1.0f, -1.0f);
  CGContextSetRGBFillColor(offscreen, 1, 1, 1, 1);
  CGContextFillRect(offscreen, CGRectMake(0,0,w,h));
#elif defined(WIN32)
  offscreen = fl_create_offscreen(w, h);
  driver(new Fl_Translated_GDI_Graphics_Driver);
#elif defined(FL_PORTING)
#  pragma message "FL_PORTING: implement Fl_Image_Surface"
#else
  offscreen = fl_create_offscreen(w, h);
  driver(new Fl_Translated_Xlib_Graphics_Driver());
#endif
}

/** The destructor.
 */
Fl_Image_Surface::~Fl_Image_Surface() {
#ifdef __APPLE__ // PORTME: Fl_Surface_Driver - platform image surface
  void *data = CGBitmapContextGetData((CGContextRef)offscreen);
  free(data);
  CGContextRelease((CGContextRef)offscreen);
#elif defined(WIN32)
  fl_delete_offscreen(offscreen);
#elif defined(FL_PORTING)
#  pragma message "FL_PORTING: implement Fl_Image_Surface"
#else
  fl_delete_offscreen(offscreen);
#endif
}

/** Returns an image made of all drawings sent to the Fl_Image_Surface object.
 The returned object contains its own copy of the RGB data.
 The caller is responsible for deleting the image.
 */
Fl_RGB_Image* Fl_Image_Surface::image()
{
  unsigned char *data;
  int W = width, H = height;
#ifdef __APPLE__ // PORTME: platform image surface
  CGContextFlush(offscreen);
  W = CGBitmapContextGetWidth(offscreen);
  H = CGBitmapContextGetHeight(offscreen);
  Fl_X::set_high_resolution(0);
  data = fl_read_image(NULL, 0, 0, W, H, 0);
#elif defined(WIN32)
  fl_pop_clip(); 
  data = fl_read_image(NULL, 0, 0, width, height, 0);
  HDC gc = (HDC)driver()->gc();
  RestoreDC(gc, _savedc);
  DeleteDC(gc);
  _ss->set_current(); 
  fl_window=_sw; 
  _ss->driver()->gc(_sgc);
#elif defined(FL_PORTING)
#  pragma message "FL_PORTING: implement Fl_Image_Surface"
#else
  fl_pop_clip(); 
  data = fl_read_image(NULL, 0, 0, width, height, 0);
  fl_window = pre_window; 
  previous->set_current();
#endif
  Fl_RGB_Image *image = new Fl_RGB_Image(data, W, H);
  image->alloc_array = 1;
  return image;
}


/** Returns a possibly high resolution image made of all drawings sent to the Fl_Image_Surface object.
 The Fl_Image_Surface object should have been constructed with Fl_Image_Surface(W, H, 1).
 The returned image is scaled to a size of WxH drawing units and may have a pixel size twice as wide and high.
 The returned object should be deallocated with Fl_Shared_Image::release() after use.
 \version 1.3.4
 */
Fl_Shared_Image* Fl_Image_Surface::highres_image()
{
  Fl_Shared_Image *s_img = Fl_Shared_Image::get(image());
  s_img->scale(width, height);
  return s_img;
}


void Fl_Image_Surface::set_current()
{
#if defined(__APPLE__) // PORTME: Fl_Surface_Driver - platform image surface
  driver()->gc(offscreen);
  fl_window = 0;
  Fl_Surface_Device::set_current();
  Fl_X::set_high_resolution( CGBitmapContextGetWidth(offscreen) > width );
#elif defined(WIN32)
  _sw = fl_window;
  _ss = Fl_Surface_Device::surface(); 
  _sgc = (HDC)_ss->driver()->gc();
  HDC gc = fl_makeDC(offscreen);
  Fl_Surface_Device::set_current();
  driver()->gc(gc);
   _savedc = SaveDC(gc);
  fl_window=(HWND)offscreen; 
  fl_push_no_clip();
#elif defined(FL_PORTING)
#  pragma message "FL_PORTING: implement Fl_Image_Surface"
#else
  pre_window = fl_window; 
  fl_window = offscreen; 
  previous = Fl_Surface_Device::surface(); 
  Fl_Surface_Device::set_current();
  fl_push_no_clip();
#endif
}

#if defined(__APPLE__) // PORTME: Fl_Surface_Driver - platform image surface

void Fl_Image_Surface::translate(int x, int y) {
  CGContextRef gc = (CGContextRef)driver()->gc();
  CGContextRestoreGState(gc);
  CGContextSaveGState(gc);
  CGContextTranslateCTM(gc, x, -y);
  CGContextSaveGState(gc);
  CGContextTranslateCTM(gc, 0, height);
  CGContextScaleCTM(gc, 1.0f, -1.0f);
}

void Fl_Image_Surface::untranslate() {
  CGContextRestoreGState((CGContextRef)driver()->gc());
}

#elif defined(WIN32)

void Fl_Image_Surface::translate(int x, int y) {
  ((Fl_Translated_GDI_Graphics_Driver*)driver())->translate_all(x, y);
}

void Fl_Image_Surface::untranslate() {
  ((Fl_Translated_GDI_Graphics_Driver*)driver())->untranslate_all();
}

#else

void Fl_Image_Surface::translate(int x, int y) {
  ((Fl_Translated_Xlib_Graphics_Driver*)driver())->translate_all(x, y);
}

void Fl_Image_Surface::untranslate() {
  ((Fl_Translated_Xlib_Graphics_Driver*)driver())->untranslate_all();
}

#endif



//
// End of "$Id$".
//
