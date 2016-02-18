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
#include "Fl_Quartz_Graphics_Driver.h"


const char *Fl_Quartz_Graphics_Driver::class_id = "Fl_Quartz_Graphics_Driver";

/* Reference to the current CGContext
 For back-compatibility only. The preferred procedure to get this reference is
 Fl_Surface_Device::surface()->driver()->get_gc().
 */
CGContextRef fl_gc = 0;

void Fl_Graphics_Driver::global_gc()
{
  fl_gc = (CGContextRef)get_gc();
}

/*
 * By linking this module, the following static method will instatiate the 
 * OS X Quartz Graphics driver as the main display driver.
 */
Fl_Graphics_Driver *Fl_Graphics_Driver::newMainGraphicsDriver()
{
  return new Fl_Quartz_Graphics_Driver();
}


Fl_Offscreen Fl_Quartz_Graphics_Driver::create_offscreen_with_alpha(int w, int h) {
  void *data = calloc(w*h,4);
  CGColorSpaceRef lut = CGColorSpaceCreateDeviceRGB();
  CGContextRef ctx = CGBitmapContextCreate(data, w, h, 8, w*4, lut, kCGImageAlphaPremultipliedLast);
  CGColorSpaceRelease(lut);
  return (Fl_Offscreen)ctx;
}

char Fl_Quartz_Graphics_Driver::can_do_alpha_blending() {
  return 1;
}

static void bmProviderRelease (void *src, const void *data, size_t size) {
  CFIndex count = CFGetRetainCount(src);
  CFRelease(src);
  if(count == 1) free((void*)data);
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

/** \addtogroup fl_drawings
 @{
 */

// FIXME: driver system
/**
 Creation of an offscreen graphics buffer.
 \param w,h     width and height in pixels of the buffer.
 \return    the created graphics buffer.
 */
Fl_Offscreen fl_create_offscreen(int w, int h) {
  void *data = calloc(w*h,4);
  CGColorSpaceRef lut = CGColorSpaceCreateDeviceRGB();
  CGContextRef ctx = CGBitmapContextCreate(
                                           data, w, h, 8, w*4, lut, kCGImageAlphaNoneSkipLast);
  CGColorSpaceRelease(lut);
  return (Fl_Offscreen)ctx;
}

// FIXME: driver system
/**  Deletion of an offscreen graphics buffer.
 \param ctx     the buffer to be deleted.
 */
void fl_delete_offscreen(Fl_Offscreen ctx) {
  if (!ctx) return;
  void *data = CGBitmapContextGetData((CGContextRef)ctx);
  CFIndex count = CFGetRetainCount(ctx);
  CGContextRelease((CGContextRef)ctx);
  if(count == 1) free(data);
}

// FIXME: driver system
const int stack_max = 16;
static int stack_ix = 0;
static CGContextRef stack_gc[stack_max];
static Window stack_window[stack_max];
static Fl_Surface_Device *_ss;

// FIXME: driver system
/**  Send all subsequent drawing commands to this offscreen buffer.
 \param ctx     the offscreen buffer.
 */
void fl_begin_offscreen(Fl_Offscreen ctx) {
  _ss = Fl_Surface_Device::surface();
  Fl_Display_Device::display_device()->set_current();
  if (stack_ix<stack_max) {
    stack_gc[stack_ix] = (CGContextRef)fl_graphics_driver->get_gc();
    stack_window[stack_ix] = fl_window;
  } else
    fprintf(stderr, "FLTK CGContext Stack overflow error\n");
  stack_ix++;

  fl_graphics_driver->set_gc(ctx);
  fl_window = 0;
  CGContextSaveGState(ctx);
  fl_graphics_driver->push_no_clip();
}

// FIXME: driver system
/** Quit sending drawing commands to the current offscreen buffer.
 */
void fl_end_offscreen() {
  fl_graphics_driver->pop_clip();
  CGContextRef gc = (CGContextRef)fl_graphics_driver->get_gc();

  CGContextRestoreGState(gc); // matches CGContextSaveGState in fl_begin_offscreen()
  CGContextFlush(gc);
  if (stack_ix>0)
    stack_ix--;
  else
    fprintf(stderr, "FLTK CGContext Stack underflow error\n");
  if (stack_ix<stack_max) {
    fl_graphics_driver->set_gc(stack_gc[stack_ix]);
    fl_window = stack_window[stack_ix];
  }
  _ss->set_current();
}

/** @} */

//
// End of "$Id$".
//
