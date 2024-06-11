//
// Draw-to-image code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2023 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//

#include <FL/platform.H>
#include <FL/fl_draw.H>
#include "Fl_Quartz_Image_Surface_Driver.H"
#include "Fl_Quartz_Graphics_Driver.H"
#include "../Cocoa/Fl_Cocoa_Window_Driver.H"
#include <ApplicationServices/ApplicationServices.h>


Fl_Quartz_Image_Surface_Driver::Fl_Quartz_Image_Surface_Driver(int w, int h, int high_res, Fl_Offscreen off) : Fl_Image_Surface_Driver(w, h, high_res, off) {
# if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
  mask_ = NULL;
#endif
  int W = w, H = h;
  float s = 1;
  if (high_res) {
    s = Fl_Graphics_Driver::default_driver().scale();
    Fl_Window *cw = Fl_Window::current();
    Fl_Cocoa_Window_Driver *dr = cw ? Fl_Cocoa_Window_Driver::driver(cw) : NULL;
    if (dr && dr->mapped_to_retina()) s *= 2;
    W *= s; H *= s;
  }
  CGColorSpaceRef lut = CGColorSpaceCreateDeviceRGB();
  offscreen = off ? off : (Fl_Offscreen)CGBitmapContextCreate(calloc(W*H,4), W, H, 8, W*4, lut, kCGImageAlphaPremultipliedLast);
  CGColorSpaceRelease(lut);
  driver(new Fl_Quartz_Graphics_Driver);
  CGContextTranslateCTM((CGContextRef)offscreen, 0.5*s, -0.5*s); // as when drawing to a window
  if (high_res) {
    CGContextScaleCTM((CGContextRef)offscreen, s, s);
    driver()->scale(s);
  }
  CGContextSetShouldAntialias((CGContextRef)offscreen, false);
  CGContextTranslateCTM((CGContextRef)offscreen, 0, height);
  CGContextScaleCTM((CGContextRef)offscreen, 1.0f, -1.0f);
  CGContextSaveGState((CGContextRef)offscreen);
  CGContextSetRGBFillColor((CGContextRef)offscreen, 1, 1, 1, 0);
  CGContextFillRect((CGContextRef)offscreen, CGRectMake(0,0,w,h));
}

Fl_Quartz_Image_Surface_Driver::~Fl_Quartz_Image_Surface_Driver() {
# if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
  if (mask_) {
    CGImageRelease(mask_);
  }
#endif
  if (offscreen) CGContextRestoreGState((CGContextRef)offscreen);
  if (offscreen && !external_offscreen) {
    void *data = CGBitmapContextGetData((CGContextRef)offscreen);
    free(data);
    CGContextRelease((CGContextRef)offscreen);
  }
  delete driver();
}


void Fl_Quartz_Image_Surface_Driver::set_current() {
  Fl_Surface_Device::set_current();
  pre_window = fl_window;
  driver()->gc((CGContextRef)offscreen);
  fl_window = 0;
  ((Fl_Quartz_Graphics_Driver*)driver())->high_resolution( CGBitmapContextGetWidth((CGContextRef)offscreen) > (size_t)width );
# if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
  if (mask_) {
    int W, H;
    printable_rect(&W, &H);
    CGContextSaveGState((CGContextRef)offscreen);
    CGContextClipToMask((CGContextRef)offscreen, CGRectMake(0,0,W,H), mask_); // 10.4
    CGContextSaveGState((CGContextRef)offscreen);
  }
# endif
}

void Fl_Quartz_Image_Surface_Driver::translate(int x, int y) {
  CGContextRestoreGState((CGContextRef)offscreen);
  CGContextSaveGState((CGContextRef)offscreen);
  CGContextTranslateCTM((CGContextRef)offscreen, x, y);
  CGContextSaveGState((CGContextRef)offscreen);
}

void Fl_Quartz_Image_Surface_Driver::untranslate() {
  CGContextRestoreGState((CGContextRef)offscreen);
}

Fl_RGB_Image* Fl_Quartz_Image_Surface_Driver::image()
{
  CGContextFlush((CGContextRef)offscreen);
# if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
  if (mask_) {
    CGContextRestoreGState((CGContextRef)offscreen);
    CGImageRelease(mask_);
    mask_ = NULL;
  }
#endif
  int W = (int)CGBitmapContextGetWidth((CGContextRef)offscreen);
  int H = (int)CGBitmapContextGetHeight((CGContextRef)offscreen);
  int bpr = (int)CGBitmapContextGetBytesPerRow((CGContextRef)offscreen);
  int bpp = (int)CGBitmapContextGetBitsPerPixel((CGContextRef)offscreen)/8;
  uchar *base = (uchar*)CGBitmapContextGetData((CGContextRef)offscreen);
  int idx, idy;
  uchar *pdst, *psrc;
  unsigned char *data = new uchar[W * H * 3];
  for (idy = 0, pdst = data; idy < H; idy ++) {
    for (idx = 0, psrc = base + idy * bpr; idx < W; idx ++, psrc += bpp, pdst += 3) {
      pdst[0] = psrc[0];  // R
      pdst[1] = psrc[1];  // G
      pdst[2] = psrc[2];  // B
    }
  }
  Fl_RGB_Image *image = new Fl_RGB_Image(data, W, H);
  image->alloc_array = 1;
  return image;
}

void Fl_Quartz_Image_Surface_Driver::end_current()
{
# if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
  if (mask_) {
    CGContextRestoreGState((CGContextRef)offscreen);
    CGContextRestoreGState((CGContextRef)offscreen);
  }
# endif
  fl_window = pre_window;
  Fl_Surface_Device::end_current();
}


# if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4

static void MyProviderReleaseData (void *info, const void *data, size_t size) {
  delete[] (uchar*)data;
}


void Fl_Quartz_Image_Surface_Driver::mask(const Fl_RGB_Image *img) {
  if (!&CGContextClipToMask) return;
  int W = (int)CGBitmapContextGetWidth((CGContextRef)offscreen);
  int H = (int)CGBitmapContextGetHeight((CGContextRef)offscreen);
  bool using_copy = false;
  if (W != img->data_w() || H != img->data_h()) {
    Fl_RGB_Image *copy = (Fl_RGB_Image*)img->copy(W, H);
    img = copy;
    using_copy = true;
  }

  int i, d = img->d(), w = img->data_w(), h = img->data_h();
  // reverse top and bottom and convert to gray scale if img->d() == 3 and complement bits
  int bytes_per_row = (img->ld() ? img->ld() : w * d);
  uchar *from = new uchar[w * h];
  for ( i = 0; i < h; i++) {
    const uchar *p = img->array + bytes_per_row * i;
    const uchar *last = p + bytes_per_row;
    uchar *q = from + (h - 1 - i) * w;
    while (p < last) {
      unsigned u = *p++;
      u += *p++;
      u += *p++;
      *q++ = ~(u/3);
    }
  }
  CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, from, w * h, MyProviderReleaseData);
  mask_ = CGImageMaskCreate(w, h, 8, 8, w, provider, NULL, false);
  CFRelease(provider);
  if (using_copy) delete img;
}

#endif
