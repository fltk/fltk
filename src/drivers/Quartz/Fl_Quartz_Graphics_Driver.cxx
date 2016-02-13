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
    stack_gc[stack_ix] = fl_gc;
    stack_window[stack_ix] = fl_window;
  } else
    fprintf(stderr, "FLTK CGContext Stack overflow error\n");
  stack_ix++;

  fl_gc = (CGContextRef)ctx;
  fl_window = 0;
  CGContextSaveGState(fl_gc);
  fl_graphics_driver->push_no_clip();
}

// FIXME: driver system
/** Quit sending drawing commands to the current offscreen buffer.
 */
void fl_end_offscreen() {
  fl_graphics_driver->pop_clip();
  CGContextRestoreGState(fl_gc); // matches CGContextSaveGState in fl_begin_offscreen()
  CGContextFlush(fl_gc);
  if (stack_ix>0)
    stack_ix--;
  else
    fprintf(stderr, "FLTK CGContext Stack underflow error\n");
  if (stack_ix<stack_max) {
    fl_gc = stack_gc[stack_ix];
    fl_window = stack_window[stack_ix];
  }
  _ss->set_current();
}

/** @} */

void Fl_Quartz_Graphics_Driver::draw_CGImage(CGImageRef cgimg, int x, int y, int w, int h, int srcx, int srcy, int sw, int sh)
{
  CGRect rect = CGRectMake(x, y, w, h);
  CGContextSaveGState(fl_gc);
  CGContextClipToRect(fl_gc, CGRectOffset(rect, -0.5, -0.5 ));
  // move graphics context to origin of vertically reversed image
  // The 0.5 here cancels the 0.5 offset present in Quartz graphics contexts.
  // Thus, image and surface pixels are in phase if there's no scaling.
  CGContextTranslateCTM(fl_gc, rect.origin.x - srcx - 0.5, rect.origin.y - srcy + sh - 0.5);
  CGContextScaleCTM(fl_gc, 1, -1);
  CGAffineTransform at = CGContextGetCTM(fl_gc);
  if (at.a == at.d && at.b == 0 && at.c == 0) { // proportional scaling, no rotation
    // We handle x2 and /2 scalings that occur when drawing to
    // a double-resolution bitmap, and when drawing a double-resolution bitmap to display.
    bool doit = false;
    // phase image with display pixels
    CGFloat deltax = 0, deltay = 0;
    if (at.a == 2) { // make .tx and .ty have even values
      deltax = (at.tx/2 - round(at.tx/2));
      deltay = (at.ty/2 - round(at.ty/2));
      doit = true;
    } else if (at.a == 0.5) {
      doit = true;
      if (Fl_Display_Device::high_resolution()) { // make .tx and .ty have int or half-int values
        deltax = (at.tx*2 - round(at.tx*2));
        deltay = (at.ty*2 - round(at.ty*2));
      } else { // make .tx and .ty have integral values
        deltax = (at.tx - round(at.tx))*2;
        deltay = (at.ty - round(at.ty))*2;
      }
    }
    if (doit) CGContextTranslateCTM(fl_gc, -deltax, -deltay);
  }
  CGContextDrawImage(fl_gc, CGRectMake(0, 0, sw, sh), cgimg);
  CGContextRestoreGState(fl_gc);
}

//
// End of "$Id$".
//
