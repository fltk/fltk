//
// "$Id$"
//
// Rectangle drawing routines for the Fast Light Tool Kit (FLTK).
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
#include "Fl_Quartz_Graphics_Driver.H"
#include "../Darwin/Fl_Darwin_System_Driver.H"
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

/*
 * By linking this module, the following static method will instantiate the
 * OS X Quartz Graphics driver as the main display driver.
 */
Fl_Graphics_Driver *Fl_Graphics_Driver::newMainGraphicsDriver()
{
  return new Fl_Quartz_Graphics_Driver();
}

Fl_Quartz_Graphics_Driver::Fl_Quartz_Graphics_Driver() : Fl_Graphics_Driver(), gc_(NULL), p_size(0), p(NULL) {
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

void Fl_Quartz_Graphics_Driver::copy_offscreen(int x, int y, int w, int h, Fl_Offscreen osrc, int srcx, int srcy) {
  // draw portion srcx,srcy,w,h of osrc to position x,y (top-left) of the graphics driver's surface
  CGContextRef src = (CGContextRef)osrc;
  void *data = CGBitmapContextGetData(src);
  int sw = CGBitmapContextGetWidth(src);
  int sh = CGBitmapContextGetHeight(src);
  CGImageAlphaInfo alpha = CGBitmapContextGetAlphaInfo(src);
  CGColorSpaceRef lut = CGColorSpaceCreateDeviceRGB();
  // when output goes to a Quartz printercontext, release of the bitmap must be
  // delayed after the end of the printed page
  CFRetain(src);
  CGDataProviderRef src_bytes = CGDataProviderCreateWithData( src, data, sw*sh*4, bmProviderRelease);
  CGImageRef img = CGImageCreate( sw, sh, 8, 4*8, 4*sw, lut, alpha,
                                 src_bytes, 0L, false, kCGRenderingIntentDefault);
  CGDataProviderRelease(src_bytes);
  CGColorSpaceRelease(lut);
  float s = scale_;
  Fl_Surface_Device *current = Fl_Surface_Device::surface();
  // test whether osrc was created by fl_create_offscreen()
  fl_begin_offscreen(osrc); // does nothing if osrc was not created by fl_create_offscreen()
  if (current != Fl_Surface_Device::surface()) { // osrc was created by fl_create_offscreen()
    Fl_Image_Surface *imgs = (Fl_Image_Surface*)Fl_Surface_Device::surface();
    int pw, ph;
    imgs->printable_rect(&pw, &ph);
    s = sw / float(pw);
    fl_end_offscreen();
  }
  draw_CGImage(img, x, y, w, h, srcx, srcy, sw/s, sh/s);
  CGImageRelease(img);
}

// so a CGRect matches exactly what is denoted x,y,w,h for clipping purposes
CGRect Fl_Quartz_Graphics_Driver::fl_cgrectmake_cocoa(int x, int y, int w, int h) {
  return CGRectMake(x - 0.5, y - 0.5, w, h);
}

void Fl_Quartz_Graphics_Driver::add_rectangle_to_region(Fl_Region r, int X, int Y, int W, int H) {
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
  Fl_Region R = (Fl_Region)malloc(sizeof(*R));
  R->count = 1;
  R->rects = (CGRect *)malloc(sizeof(CGRect));
  *(R->rects) = Fl_Quartz_Graphics_Driver::fl_cgrectmake_cocoa(x, y, w, h);
  return R;
}

void Fl_Quartz_Graphics_Driver::XDestroyRegion(Fl_Region r) {
  if(r) {
    free(r->rects);
    free(r);
  }
}

//
// End of "$Id$".
//
