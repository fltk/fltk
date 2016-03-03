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
#include <FL/fl_draw.H>

#include "config_lib.h"


#if defined(__APPLE__)
#ifdef FL_CFG_GFX_QUARTZ
#include "drivers/Quartz/Fl_Quartz_Graphics_Driver.H"
#endif

#include <ApplicationServices/ApplicationServices.h>

class Fl_Image_Surface::Helper : public Fl_Widget_Surface {
  friend class Fl_Image_Surface;
public:
  Fl_Offscreen offscreen;
  Fl_Surface_Device *previous;
  Window pre_window;
  int was_high;
  int width;
  int height;
  Helper(int w, int h, int high_res);
  ~Helper();
  void set_current();
  void translate(int x, int y);
  void untranslate();
  int printable_rect(int *w, int *h) {*w = width; *h = height; return 0;}
  Fl_RGB_Image *image();
  void end_current();
};

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
  void *data = CGBitmapContextGetData((CGContextRef)offscreen);
  free(data);
  CGContextRelease((CGContextRef)offscreen);
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

#elif defined(WIN32)
#ifdef FL_CFG_GFX_GDI
#include "drivers/GDI/Fl_GDI_Graphics_Driver.H"
#endif

class Fl_Image_Surface::Helper : public Fl_Widget_Surface {
  friend class Fl_Image_Surface;
public:
  Fl_Offscreen offscreen;
  int width;
  int height;
  Fl_Surface_Device *previous;
  Window pre_window;
  HDC _sgc;
  int _savedc;
  Helper(int w, int h, int high_res);
  ~Helper();
  void set_current();
  void translate(int x, int y);
  void untranslate();
  Fl_RGB_Image *image();
  void end_current();
  int printable_rect(int *w, int *h) {*w = width; *h = height; return 0;}
};

Fl_Image_Surface::Helper::Helper(int w, int h, int high_res) : Fl_Widget_Surface(NULL), width(w), height(h) {
  previous = 0;
  offscreen = CreateCompatibleBitmap( (fl_graphics_driver->gc() ? (HDC)fl_graphics_driver->gc() : fl_GetDC(0) ) , w, h);
  driver(new Fl_Translated_GDI_Graphics_Driver);
  _sgc = NULL;
}

Fl_Image_Surface::Helper::~Helper() {
  DeleteObject(offscreen);
}

void Fl_Image_Surface::Helper::set_current() {
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

void Fl_Image_Surface::Helper::translate(int x, int y) {
  ((Fl_Translated_GDI_Graphics_Driver*)driver())->translate_all(x, y);
}

void Fl_Image_Surface::Helper::untranslate() {
  ((Fl_Translated_GDI_Graphics_Driver*)driver())->untranslate_all();
}

Fl_RGB_Image* Fl_Image_Surface::Helper::image()
{
  unsigned char *data;
  data = fl_read_image(NULL, 0, 0, width, height, 0);
  end_current();
  previous->driver()->gc(_sgc);
  Fl_RGB_Image *image = new Fl_RGB_Image(data, width, height);
  image->alloc_array = 1;
  return image;
}

void Fl_Image_Surface::Helper::end_current()
{
  HDC gc = (HDC)driver()->gc();
  RestoreDC(gc, _savedc);
  DeleteDC(gc);
  fl_pop_clip();
  previous->Fl_Surface_Device::set_current();
  fl_window = pre_window;
}

#elif defined(USE_SDL)


#elif defined(FL_PORTING)
# pragma message "FL_PORTING: implement class Fl_Image_Surface::Helper for your platform"

class Fl_Image_Surface::Helper : public Fl_Widget_Surface { // class model
  friend class Fl_Image_Surface;
public:
  int width;
  int height;
  Helper(int w, int h, int high_res) : Fl_Widget_Surface(NULL), width(w), height(h) {} // to implement
  ~Helper() {} // to implement
  void set_current(){} // to implement
  void translate(int x, int y) {} // to implement
  void untranslate() {} // to implement
  Fl_RGB_Image *image() {} // to implement
  void end_current() {} // to implement
  int printable_rect(int *w, int *h) {*w = width; *h = height; return 0;}
};


#else

#ifdef FL_CFG_GFX_XLIB
#include "drivers/Xlib/Fl_Translated_Xlib_Graphics_Driver.H"
#endif

class Fl_Image_Surface::Helper : public Fl_Widget_Surface {
public:
  Fl_Offscreen offscreen;
  Fl_Surface_Device *previous;
  Window pre_window;
  int was_high;
  int width;
  int height;
  Helper(int w, int h, int high_res);
  Helper(Fl_Offscreen pixmap, int w, int h);
  ~Helper();
  void set_current();
  void translate(int x, int y);
  void untranslate();
  int printable_rect(int *w, int *h) {*w = width; *h = height; return 0;}
  Fl_RGB_Image *image();
  void end_current();
  public:
};

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
  XFreePixmap(fl_display, offscreen);
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
  platform_surface = new Helper(w, h, high_res);
  driver(platform_surface->driver());
}

/** The destructor.
 */
Fl_Image_Surface::~Fl_Image_Surface() { delete platform_surface; }

void Fl_Image_Surface::origin(int x, int y) {platform_surface->origin(x, y);}

void Fl_Image_Surface::origin(int *x, int *y) {platform_surface->origin(x, y);}

void Fl_Image_Surface::set_current() {platform_surface->set_current();}

/** Stop sending graphics commands to the surface */
void Fl_Image_Surface::end_current() {platform_surface->end_current();}

void Fl_Image_Surface::translate(int x, int y) {platform_surface->translate(x, y);}

void Fl_Image_Surface::untranslate() {platform_surface->untranslate();}

Fl_Offscreen Fl_Image_Surface::offscreen() {return platform_surface->offscreen;}

int Fl_Image_Surface::printable_rect(int *w, int *h)  {return platform_surface->printable_rect(w, h);}

/** Returns an image made of all drawings sent to the Fl_Image_Surface object.
 The returned object contains its own copy of the RGB data.
 The caller is responsible for deleting the image.
 */
Fl_RGB_Image *Fl_Image_Surface::image() {return platform_surface->image();}

/** Returns a possibly high resolution image made of all drawings sent to the Fl_Image_Surface object.
 The Fl_Image_Surface object should have been constructed with Fl_Image_Surface(W, H, 1).
 The returned image is scaled to a size of WxH drawing units and may have a pixel size twice as wide and high.
 The returned object should be deallocated with Fl_Shared_Image::release() after use.
 \version 1.3.4
 */
Fl_Shared_Image* Fl_Image_Surface::highres_image()
{
  Fl_Shared_Image *s_img = Fl_Shared_Image::get(platform_surface->image());
  int width, height;
  printable_rect(&width, &height);
  s_img->scale(width, height);
  return s_img;
}

// implementation of the fl_XXX_offscreen() functions

static void **offscreen_api_surface = NULL;
static int count_offscreens = 0;
static int current_offscreen;

static int find_slot(void) { // return an available slot to memorize an Fl_Image_Surface::Helper object
  static int max = 0;
  for (int num = 0; num < count_offscreens; num++) {
    if (!offscreen_api_surface[num]) return num;
  }
  if (count_offscreens >= max) {
    max += 20;
    offscreen_api_surface = (void**)realloc(offscreen_api_surface, max * sizeof(void *));
    return find_slot();
  }
  return count_offscreens++;
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
  offscreen_api_surface[rank] = new Fl_Image_Surface::Helper::Helper(w, h, 0);
  return ((Fl_Image_Surface::Helper**)offscreen_api_surface)[rank]->offscreen;
}

#ifdef USE_X11
Fl_Offscreen fl_create_offscreen_with_alpha(int w, int h) {
  int rank = find_slot();
  Fl_Offscreen pixmap = XCreatePixmap(fl_display, RootWindow(fl_display, fl_screen), w, h, 32);
  offscreen_api_surface[rank] = new Fl_Image_Surface::Helper::Helper(pixmap, w, h);
  return pixmap;
}
#endif

/**  Deletion of an offscreen graphics buffer.
   \param ctx     the buffer to be deleted.
   */
void fl_delete_offscreen(Fl_Offscreen ctx) {
  if (!ctx) return;
  for (int i = 0; i < count_offscreens; i++) {
    if (offscreen_api_surface[i] && ((Fl_Image_Surface::Helper**)offscreen_api_surface)[i]->offscreen == ctx) {
      delete ((Fl_Image_Surface::Helper**)offscreen_api_surface)[i];
      offscreen_api_surface[i] = NULL;
    }
  }
}

/**  Send all subsequent drawing commands to this offscreen buffer.
   \param ctx     the offscreen buffer.
   */
void fl_begin_offscreen(Fl_Offscreen ctx) {
  for (current_offscreen = 0; current_offscreen < count_offscreens; current_offscreen++) {
    if (offscreen_api_surface[current_offscreen] &&
        ((Fl_Image_Surface::Helper**)offscreen_api_surface)[current_offscreen]->offscreen == ctx) {
      ((Fl_Image_Surface::Helper**)offscreen_api_surface)[current_offscreen]->set_current();
      return;
    }
  }
}

/** Quit sending drawing commands to the current offscreen buffer.
   */
void fl_end_offscreen() {
  ((Fl_Image_Surface::Helper**)offscreen_api_surface)[current_offscreen]->end_current();
}

/** @} */


//
// End of "$Id$".
//
