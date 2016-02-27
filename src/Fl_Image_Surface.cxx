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
 \param high_res if non-zero, the surface pixel size is twice as high and wide as w and h,
 which is useful to draw it later on a high resolution display (e.g., retina display).
 This is implemented for the Mac OS platform only.
 If \p highres is non-zero, use Fl_Image_Surface::highres_image() to get the image data.
 \version 1.3.4 (1.3.3 without the highres parameter)
 */
Fl_Image_Surface::Fl_Image_Surface(int w, int h, int high_res) : Fl_Widget_Surface(NULL) {
  initialize_(NULL, w, h, high_res);
}

void Fl_Image_Surface::initialize_(Fl_Offscreen pixmap, int w, int h, int high_res) {
  width = w;
  height = h;
  previous = 0;
#ifdef __APPLE__ // PORTME: platform image surface
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
#elif defined(WIN32)
  offscreen = CreateCompatibleBitmap( (fl_graphics_driver->gc() ? (HDC)fl_graphics_driver->gc() : fl_GetDC(0) ) , w, h);
  driver(new Fl_Translated_GDI_Graphics_Driver);
  _sgc = NULL;
#elif defined(FL_PORTING)
#  pragma message "FL_PORTING: implement Fl_Image_Surface"
#else
  offscreen = pixmap ? pixmap : XCreatePixmap(fl_display, RootWindow(fl_display, fl_screen), w, h, fl_visual->depth);
  driver(new Fl_Translated_Xlib_Graphics_Driver());
#endif
}

#if USE_X11
// private constructor for X11 only
Fl_Image_Surface::Fl_Image_Surface(Fl_Offscreen pixmap, int w, int h) : Fl_Widget_Surface(NULL) {
    initialize_(pixmap, w, h, 0);
  }
#endif

/** The destructor.
 */
Fl_Image_Surface::~Fl_Image_Surface() {
#ifdef __APPLE__ // PORTME: Fl_Surface_Driver - platform image surface
  void *data = CGBitmapContextGetData((CGContextRef)offscreen);
  free(data);
  CGContextRelease((CGContextRef)offscreen);
#elif defined(WIN32)
  DeleteObject(offscreen);
#elif defined(FL_PORTING)
#  pragma message "FL_PORTING: implement Fl_Image_Surface"
#else
  XFreePixmap(fl_display, offscreen);
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
  data = fl_read_image(NULL, 0, 0, W, H, 0);
#elif defined(WIN32)
  data = fl_read_image(NULL, 0, 0, width, height, 0);
  end_current();
  previous->driver()->gc(_sgc);
#elif defined(FL_PORTING)
#  pragma message "FL_PORTING: implement Fl_Image_Surface"
#else
  data = fl_read_image(NULL, 0, 0, width, height, 0);
  end_current();
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
  pre_window = fl_window;
  if (!previous) previous = Fl_Surface_Device::surface();
#if defined(__APPLE__) // PORTME: Fl_Surface_Driver - platform image surface
  driver()->gc(offscreen);
  fl_window = 0;
  Fl_Surface_Device::set_current();
  was_high = Fl_Display_Device::high_resolution();
  Fl_X::set_high_resolution( CGBitmapContextGetWidth(offscreen) > width );
#elif defined(WIN32)
  if (!_sgc) _sgc = (HDC)previous->driver()->gc();
  HDC gc = fl_makeDC(offscreen);
  Fl_Surface_Device::set_current();
  driver()->gc(gc);
  _savedc = SaveDC(gc);
  fl_window=(HWND)offscreen;
  fl_push_no_clip();
#elif defined(FL_PORTING)
#  pragma message "FL_PORTING: implement Fl_Image_Surface"
#else
  fl_window = offscreen;
  Fl_Surface_Device::set_current();
  fl_push_no_clip();
#endif
}

void Fl_Image_Surface::end_current()
{
#if defined(__APPLE__)
  Fl_X::set_high_resolution(was_high);
#elif defined(WIN32)
  HDC gc = (HDC)driver()->gc();
  RestoreDC(gc, _savedc);
  DeleteDC(gc);
  fl_pop_clip();
#elif defined(FL_PORTING)
#  pragma message "FL_PORTING: implement Fl_Image_Surface"
#else
  fl_pop_clip();
#endif
  previous->Fl_Surface_Device::set_current();
  fl_window = pre_window;
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

static Fl_Image_Surface *offscreen_api_surface[20];
static int count = 0;
static int current;

static int find_slot() { // return an available slot to memorize an Fl_Image_Surface object
    for (int num = 0; num < count; num++) {
        if (!offscreen_api_surface[num]) return num;
      }
    if (count >= sizeof(offscreen_api_surface)/sizeof(Fl_Image_Surface*)) return -1;
    return count++;
  }

/** \addtogroup fl_drawings
   @{
   */

/**
   Creation of an offscreen graphics buffer.
   \param w,h     width and height in pixels of the buffer.
   \return    the created graphics buffer.
   */
Fl_Offscreen fl_create_offscreen(int w, int h) {
  int rank = find_slot();
  if (rank < 0) return NULL;
  offscreen_api_surface[rank] = new Fl_Image_Surface(w, h);
  return offscreen_api_surface[rank]->offscreen;
}

#if USE_X11
Fl_Offscreen fl_create_offscreen_with_alpha(int w, int h) {
  int rank = find_slot();
  if (rank < 0) return NULL;
  Fl_Offscreen pixmap = XCreatePixmap(fl_display, RootWindow(fl_display, fl_screen), w, h, 32);
  offscreen_api_surface[rank] = new Fl_Image_Surface(pixmap, w, h);
  return pixmap;
}
#endif

/**  Deletion of an offscreen graphics buffer.
   \param ctx     the buffer to be deleted.
   */
void fl_delete_offscreen(Fl_Offscreen ctx) {
  if (!ctx) return;
  for (int i = 0; i < count; i++) {
    if (offscreen_api_surface[i] && offscreen_api_surface[i]->offscreen == ctx) {
      delete offscreen_api_surface[i];
      offscreen_api_surface[i] = NULL;
    }
  }
}

/**  Send all subsequent drawing commands to this offscreen buffer.
   \param ctx     the offscreen buffer.
   */
void fl_begin_offscreen(Fl_Offscreen ctx) {
  for (current = 0; current < count; current++) {
    if (offscreen_api_surface[current] && offscreen_api_surface[current]->offscreen == ctx) {
      offscreen_api_surface[current]->set_current();
      return;
    }
  }
}

/** Quit sending drawing commands to the current offscreen buffer.
   */
void fl_end_offscreen() {
  offscreen_api_surface[current]->end_current();
}

/** @} */


//
// End of "$Id$".
//
