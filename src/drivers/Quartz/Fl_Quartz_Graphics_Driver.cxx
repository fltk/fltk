//
// Rectangle drawing routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2018 by Bill Spitzak and others.
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

#include <config.h>
#include "Fl_Quartz_Graphics_Driver.H"
#include "../Darwin/Fl_Darwin_System_Driver.H"
#include "../Cocoa/Fl_Cocoa_Screen_Driver.H"
#include <FL/platform.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Image_Surface.H>

#if HAS_ATSU && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
Fl_Quartz_Graphics_Driver::pter_to_draw_member Fl_Quartz_Graphics_Driver::CoreText_or_ATSU_draw;
Fl_Quartz_Graphics_Driver::pter_to_width_member Fl_Quartz_Graphics_Driver::CoreText_or_ATSU_width;

int Fl_Quartz_Graphics_Driver::CoreText_or_ATSU = 0;

void Fl_Quartz_Graphics_Driver::init_CoreText_or_ATSU()
{
  if (Fl_Darwin_System_Driver::calc_mac_os_version() < 100500) {
    // before Mac OS 10.5, only ATSU is available
    CoreText_or_ATSU = use_ATSU;
    CoreText_or_ATSU_draw = &Fl_Quartz_Graphics_Driver::draw_ATSU;
    CoreText_or_ATSU_width = &Fl_Quartz_Graphics_Driver::width_ATSU;
  } else {
    CoreText_or_ATSU = use_CoreText;
    CoreText_or_ATSU_draw = &Fl_Quartz_Graphics_Driver::draw_CoreText;
    CoreText_or_ATSU_width = &Fl_Quartz_Graphics_Driver::width_CoreText;
  }
}
#endif


void Fl_Quartz_Graphics_Driver::antialias(int state) {
}

int Fl_Quartz_Graphics_Driver::antialias() {
  return 1;
}

Fl_Quartz_Graphics_Driver::Fl_Quartz_Graphics_Driver() : Fl_Graphics_Driver(), gc_(NULL) {
  quartz_line_width_ = 1.f;
  quartz_line_cap_ = kCGLineCapButt;
  quartz_line_join_ = kCGLineJoinMiter;
  quartz_line_pattern = 0;
  quartz_line_pattern_size = 0;
  high_resolution_ = false;
#if HAS_ATSU && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
  if (!CoreText_or_ATSU) init_CoreText_or_ATSU();
#endif
}

char Fl_Quartz_Graphics_Driver::can_do_alpha_blending() {
  return 1;
}

static void bmProviderRelease (void *src, const void *data, size_t size) {
  CFIndex count = CFGetRetainCount(src);
  CFRelease(src);
  if(count == 1) free((void*)data);
}

/* Reference to the current CGContext
 For back-compatibility only. The preferred procedure to get this reference is
 Fl_Surface_Device::surface()->driver()->gc().
 */
CGContextRef fl_gc = 0;

void Fl_Quartz_Graphics_Driver::global_gc()
{
  fl_gc = (CGContextRef)gc();
}


CGContextRef fl_mac_gc() { return fl_gc; }


void Fl_Quartz_Graphics_Driver::copy_offscreen(int x, int y, int w, int h, Fl_Offscreen osrc, int srcx, int srcy) {
  // draw portion srcx,srcy,w,h of osrc to position x,y (top-left) of the graphics driver's surface
  CGContextRef src = (CGContextRef)osrc;
  void *data = CGBitmapContextGetData(src);
  int sw = (int)CGBitmapContextGetWidth(src);
  int sh = (int)CGBitmapContextGetHeight(src);
  CGImageRef img;
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
  if (fl_mac_os_version >= 100400) img = CGBitmapContextCreateImage(src);  // requires 10.4
  else
#endif
  {
    CGImageAlphaInfo alpha = CGBitmapContextGetAlphaInfo(src);
    CGColorSpaceRef lut = CGColorSpaceCreateDeviceRGB();
    // when output goes to a Quartz printercontext, release of the bitmap must be
    // delayed after the end of the printed page
    CFRetain(src);
    CGDataProviderRef src_bytes = CGDataProviderCreateWithData( src, data, sw*sh*4, bmProviderRelease);
    img = CGImageCreate( sw, sh, 8, 4*8, 4*sw, lut, alpha,
                        src_bytes, 0L, false, kCGRenderingIntentDefault);
    CGDataProviderRelease(src_bytes);
    CGColorSpaceRelease(lut);
  }
  CGAffineTransform at = CGContextGetCTM(src);
  float s = at.a;
  draw_CGImage(img, x, y, w, h, srcx, srcy, sw/s, sh/s);
  CGImageRelease(img);
}

// so a CGRect matches exactly what is denoted x,y,w,h for clipping purposes
CGRect Fl_Quartz_Graphics_Driver::fl_cgrectmake_cocoa(int x, int y, int w, int h) {
  return CGRectMake(x - 0.5, y - 0.5, w, h);
}

void Fl_Quartz_Graphics_Driver::add_rectangle_to_region(Fl_Region r_, int X, int Y, int W, int H) {
  struct flCocoaRegion *r = (struct flCocoaRegion*)r_;
  CGRect arg = Fl_Quartz_Graphics_Driver::fl_cgrectmake_cocoa(X, Y, W, H);
  int j; // don't add a rectangle totally inside the Fl_Region
  for(j = 0; j < r->count; j++) {
    if(CGRectContainsRect(r->rects[j], arg)) break;
  }
  if( j >= r->count) {
    r->rects = (CGRect*)realloc(r->rects, (++(r->count)) * sizeof(CGRect));
    r->rects[r->count - 1] = arg;
  }
}

Fl_Region Fl_Quartz_Graphics_Driver::XRectangleRegion(int x, int y, int w, int h) {
  struct flCocoaRegion* R = (struct flCocoaRegion*)malloc(sizeof(struct flCocoaRegion));
  R->count = 1;
  R->rects = (CGRect *)malloc(sizeof(CGRect));
  *(R->rects) = Fl_Quartz_Graphics_Driver::fl_cgrectmake_cocoa(x, y, w, h);
  return R;
}

void Fl_Quartz_Graphics_Driver::XDestroyRegion(Fl_Region r_) {
  if (r_) {
    struct flCocoaRegion *r = (struct flCocoaRegion*)r_;
    free(r->rects);
    free(r);
  }
}

void Fl_Quartz_Graphics_Driver::cache_size(Fl_Image *img, int &width, int &height) {
  width *= 2 * scale();
  height *= 2 * scale();
}

float Fl_Quartz_Graphics_Driver::override_scale() {
  float s = scale();
  if (s != 1.f && Fl_Display_Device::display_device()->is_current()) {
    CGContextScaleCTM(gc_, 1./s, 1./s);
    Fl_Graphics_Driver::scale(1);
  }
  return s;
}

void Fl_Quartz_Graphics_Driver::restore_scale(float s) {
  if (s != 1.f && Fl_Display_Device::display_device()->is_current()) {
    CGContextScaleCTM(gc_, s, s);
    Fl_Graphics_Driver::scale(s);
  }
}
