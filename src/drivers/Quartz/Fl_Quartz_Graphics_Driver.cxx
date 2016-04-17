//
// "$Id$"
//
// Rectangle drawing routines for the Fast Light Tool Kit (FLTK).
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
#include "Fl_Quartz_Graphics_Driver.H"
#include <FL/x.H>

/*
 * By linking this module, the following static method will instantiate the
 * OS X Quartz Graphics driver as the main display driver.
 */
Fl_Graphics_Driver *Fl_Graphics_Driver::newMainGraphicsDriver()
{
  return new Fl_Quartz_Graphics_Driver();
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

void Fl_Quartz_Graphics_Driver::copy_offscreen(int x,int y,int w,int h,Fl_Offscreen osrc,int srcx,int srcy) {
  CGContextRef src = (CGContextRef)osrc;
  void *data = CGBitmapContextGetData(src);
  int sw = CGBitmapContextGetWidth(src);
  int sh = CGBitmapContextGetHeight(src);
  CGImageAlphaInfo alpha = CGBitmapContextGetAlphaInfo(src);
  CGColorSpaceRef lut = CGColorSpaceCreateDeviceRGB();
  // when output goes to a Quartz printercontext, release of the bitmap must be
  // delayed after the end of the print page
  CFRetain(src);
  CGDataProviderRef src_bytes = CGDataProviderCreateWithData( src, data, sw*sh*4, bmProviderRelease);
  CGImageRef img = CGImageCreate( sw, sh, 8, 4*8, 4*sw, lut, alpha,
                                 src_bytes, 0L, false, kCGRenderingIntentDefault);
  draw_CGImage(img, x, y, w, h, srcx, srcy,  sw, sh);

  CGImageRelease(img);
  CGColorSpaceRelease(lut);
  CGDataProviderRelease(src_bytes);
}

// so a CGRect matches exactly what is denoted x,y,w,h for clipping purposes
CGRect Fl_Quartz_Graphics_Driver::fl_cgrectmake_cocoa(int x, int y, int w, int h) {
  return CGRectMake(x - 0.5, y - 0.5, w, h);
}

void Fl_Graphics_Driver::add_rectangle_to_region(Fl_Region r, int X, int Y, int W, int H) {
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

Fl_Region Fl_Graphics_Driver::XRectangleRegion(int x, int y, int w, int h) {
  Fl_Region R = (Fl_Region)malloc(sizeof(*R));
  R->count = 1;
  R->rects = (CGRect *)malloc(sizeof(CGRect));
  *(R->rects) = Fl_Quartz_Graphics_Driver::fl_cgrectmake_cocoa(x, y, w, h);
  return R;
}

void Fl_Graphics_Driver::XDestroyRegion(Fl_Region r) {
  if(r) {
    free(r->rects);
    free(r);
  }
}

//
// End of "$Id$".
//
