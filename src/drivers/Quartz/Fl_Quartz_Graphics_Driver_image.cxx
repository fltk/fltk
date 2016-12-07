//
// "$Id$"
//
// MacOS image drawing code for the Fast Light Tool Kit (FLTK).
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

#include <config.h>
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Printer.H>
#include <FL/x.H>
#include <FL/Fl_Image_Surface.H>

#define MAXBUFFER 0x40000 // 256k

static void dataReleaseCB(void *info, const void *data, size_t size)
{
  delete[] (uchar *)data;
}

/*
 * draw an image based on the input parameters
 *
 * buf:       image source data
 * X, Y:      position (in buffer?!)
 * W, H:      size of picture (in pixel?)
 * delta:     distance from pixel to pixel in buf in bytes
 * linedelta: distance from line to line in buf in bytes
 * mono:      if set, pixel is one byte - if zero, pixel is 3 byte
 * cb:        callback to copy image data into (RGB?) buffer
 *   buf:       pointer to first byte in image source
 *   x, y:      position in buffer
 *   w:         width (in bytes?)
 *   dst:       destination buffer
 * userdata:  ?
 */
static void innards(const uchar *buf, int X, int Y, int W, int H,
		    int delta, int linedelta, int mono,
		    Fl_Draw_Image_Cb cb, void* userdata, CGContextRef gc, Fl_Quartz_Graphics_Driver *driver)
{
  if (!linedelta) linedelta = W*abs(delta);

  uchar *tmpBuf = 0;
  if (!cb) {
    if (delta < 0) buf -= (W-1)*(-delta);
    if (linedelta < 0) buf -= (H-1)*abs(linedelta);
  }
  const void *array = buf;
  if (cb || Fl_Surface_Device::surface() != Fl_Display_Device::display_device()) {
    tmpBuf = new uchar[ H*W*abs(delta) ];
    if (cb) {
      for (int i=0; i<H; i++) {
	cb(userdata, 0, i, W, tmpBuf+i*W*abs(delta));
      }
    } else {
      uchar *p = tmpBuf;
      for (int i=0; i<H; i++) {
	memcpy(p, buf+i*abs(linedelta), W*abs(delta));
	p += W*abs(delta);
	}
    }
    array = (void*)tmpBuf;
    linedelta = W*abs(delta);
  }
  // create an image context
  CGColorSpaceRef   lut = 0;
  if (abs(delta)<=2)
    lut = CGColorSpaceCreateDeviceGray();
  else
    lut = CGColorSpaceCreateDeviceRGB();
  // a release callback is necessary when the gc is a print context because the image data
  // must be kept until the page is closed. Thus tmpBuf can't be deleted here. It's too early.
  CGDataProviderRef src = CGDataProviderCreateWithData( 0L, array, abs(linedelta)*H,
						       tmpBuf ? dataReleaseCB : NULL
						       );
  CGImageRef        img = CGImageCreate( W, H, 8, 8*abs(delta), abs(linedelta),
                            lut, abs(delta)&1?kCGImageAlphaNone:kCGImageAlphaLast,
                            src, 0L, false, kCGRenderingIntentDefault);
  CGColorSpaceRelease(lut);
  CGDataProviderRelease(src);
  // draw the image into the destination context
  if (img) {
    CGContextSaveGState(gc);
    CGContextTranslateCTM(gc, X, Y);
    if (linedelta < 0) {
      CGContextTranslateCTM(gc, 0, H-1);
      CGContextScaleCTM(gc, 1, -1);
    }
    if (delta < 0) {
      CGContextTranslateCTM(gc, W-1, 0);
      CGContextScaleCTM(gc, -1, 1);
    }
    driver->draw_CGImage(img, 0,0,W,H, 0,0,W,H);
    CGImageRelease(img);
    CGContextRestoreGState(gc);
  }
}

void Fl_Quartz_Graphics_Driver::draw_image(const uchar* buf, int x, int y, int w, int h, int d, int l){
  innards(buf,x,y,w,h,d,l,(d<3&&d>-3),0,0,gc_,this);
}
void Fl_Quartz_Graphics_Driver::draw_image(Fl_Draw_Image_Cb cb, void* data,
		   int x, int y, int w, int h,int d) {
  innards(0,x,y,w,h,d,0,(d<3&&d>-3),cb,data,gc_,this);
}
void Fl_Quartz_Graphics_Driver::draw_image_mono(const uchar* buf, int x, int y, int w, int h, int d, int l){
  innards(buf,x,y,w,h,d,l,1,0,0,gc_,this);
}
void Fl_Quartz_Graphics_Driver::draw_image_mono(Fl_Draw_Image_Cb cb, void* data,
		   int x, int y, int w, int h,int d) {
  innards(0,x,y,w,h,d,0,1,cb,data,gc_,this);
}

void fl_rectf(int x, int y, int w, int h, uchar r, uchar g, uchar b) {
  fl_color(r,g,b);
  fl_rectf(x,y,w,h);
}

void Fl_Quartz_Graphics_Driver::draw(Fl_Bitmap *bm, int XP, int YP, int WP, int HP, int cx, int cy) {
  int X, Y, W, H;
  if (Fl_Graphics_Driver::prepare(bm, XP, YP, WP, HP, cx, cy, X, Y, W, H)) {
    return;
  }
  if (*Fl_Graphics_Driver::id(bm) && gc_) {
    draw_CGImage((CGImageRef)*Fl_Graphics_Driver::id(bm), X,Y,W,H, cx, cy, bm->w(), bm->h());
  }
}


void Fl_Quartz_Graphics_Driver::draw(Fl_RGB_Image *img, int XP, int YP, int WP, int HP, int cx, int cy) {
  int X, Y, W, H;
  // Don't draw an empty image...
  if (!img->d() || !img->array) {
    Fl_Graphics_Driver::draw_empty(img, XP, YP);
    return;
  }
  if (start_image(img, XP, YP, WP, HP, cx, cy, X, Y, W, H)) {
    return;
  }
  CGImageRef cgimg = (CGImageRef)*Fl_Graphics_Driver::id(img);
  if (cgimg && has_feature(PRINTER) && !*Fl_Graphics_Driver::mask(img)) {
    CGImageRelease(cgimg);
    *Fl_Graphics_Driver::id(img) = 0;
    cgimg = NULL;
  }
  if (!cgimg) {
    CGColorSpaceRef lut = img->d()<=2 ? CGColorSpaceCreateDeviceGray() : CGColorSpaceCreateDeviceRGB();
    int ld = img->ld();
    if (!ld) ld = img->w() * img->d();
    CGDataProviderRef src;
    if ( has_feature(PRINTER) ) {
      // When printing, the image data is used when the printed page is completed.
      // At that stage, the image has possibly been deleted. It is therefore necessary
      // to print a copy of the image data. The mask_ member of the Fl_RGB_Image is used to avoid
      // repeating the copy operation if the image is printed again.
      // The CGImage data provider deletes the copy when the Fl_RGB_Image is deleted.
      uchar *copy = new uchar[ld * img->h()];
      memcpy(copy, img->array, ld * img->h());
      src = CGDataProviderCreateWithData(NULL, copy, ld * img->h(), dataReleaseCB);
      *Fl_Graphics_Driver::mask(img) = 1;
    } else {
    // the CGImage data provider need not release the image data.
      src = CGDataProviderCreateWithData(NULL, img->array, ld * img->h(), NULL);
      }
    cgimg = CGImageCreate(img->w(), img->h(), 8, img->d()*8, ld,
                                           lut, (img->d()&1)?kCGImageAlphaNone:kCGImageAlphaLast,
                                           src, 0L, false, kCGRenderingIntentDefault);
    *Fl_Graphics_Driver::id(img) = (fl_uintptr_t)cgimg;
    CGColorSpaceRelease(lut);
    CGDataProviderRelease(src);
  }
  if (cgimg && gc_) {
    draw_CGImage(cgimg, X,Y,W,H, cx,cy, img->w(), img->h());
  }
}

int Fl_Quartz_Graphics_Driver::draw_scaled(Fl_Image *img, int XP, int YP, int WP, int HP) {
  int X, Y, W, H;
  fl_clip_box(XP,YP,WP,HP,X,Y,W,H); // X,Y,W,H will give the unclipped area of XP,YP,WP,HP
  if (W == 0 || H == 0) return 1;
  fl_push_no_clip(); // remove the FLTK clip that can't be rescaled
  CGContextSaveGState(gc_);
  CGContextClipToRect(gc_, CGRectMake(X, Y, W, H)); // this clip path will be rescaled & translated
  CGContextTranslateCTM(gc_, XP, YP);
  CGContextScaleCTM(gc_, float(WP)/img->w(), float(HP)/img->h());
  img->draw(0, 0, img->w(), img->h(), 0, 0);
  CGContextRestoreGState(gc_);
  fl_pop_clip(); // restore FLTK's clip
  return 1;
}

void Fl_Quartz_Graphics_Driver::draw(Fl_Pixmap *pxm, int XP, int YP, int WP, int HP, int cx, int cy) {
  int X, Y, W, H;
  if (Fl_Graphics_Driver::prepare(pxm, XP, YP, WP, HP, cx, cy, X, Y, W, H)) return;
  CGImageRef cgimg = (CGImageRef)*Fl_Graphics_Driver::id(pxm);
  draw_CGImage(cgimg, X,Y,W,H, cx,cy, pxm->w(), pxm->h());
}

Fl_Bitmask Fl_Quartz_Graphics_Driver::create_bitmask(int w, int h, const uchar *array) {
  static uchar reverse[16] =    /* Bit reversal lookup table */
  { 0x00, 0x88, 0x44, 0xcc, 0x22, 0xaa, 0x66, 0xee,
    0x11, 0x99, 0x55, 0xdd, 0x33, 0xbb, 0x77, 0xff };
  int rowBytes = (w+7)>>3 ;
  uchar *bmask = new uchar[rowBytes*h];
  uchar *dst = bmask;
  const uchar *src = array;
  for ( int i=rowBytes*h; i>0; i--,src++ ) {
    *dst++ = ((reverse[*src & 0x0f] & 0xf0) | (reverse[(*src >> 4) & 0x0f] & 0x0f))^0xff;
  }
  CGDataProviderRef srcp = CGDataProviderCreateWithData( NULL, bmask, rowBytes*h, dataReleaseCB);
  CGImageRef id_ = CGImageMaskCreate( w, h, 1, 1, rowBytes, srcp, 0L, false);
  CGDataProviderRelease(srcp);
  return (Fl_Bitmask)id_;
}

void Fl_Quartz_Graphics_Driver::delete_bitmask(Fl_Bitmask bm) {
  if (bm) CGImageRelease((CGImageRef)bm);
}

void Fl_Quartz_Graphics_Driver::uncache(Fl_RGB_Image*, fl_uintptr_t &id_, fl_uintptr_t &mask_) {
  if (id_) {
    CGImageRelease((CGImageRef)id_);
    id_ = 0;
    mask_ = 0;
  }
}

fl_uintptr_t Fl_Quartz_Graphics_Driver::cache(Fl_Bitmap*, int w, int h, const uchar *array) {
  return (fl_uintptr_t)create_bitmask(w, h, array);
}


static void pmProviderRelease (void *ctxt, const void *data, size_t size) {
  CFRelease(ctxt);
}

fl_uintptr_t Fl_Quartz_Graphics_Driver::cache(Fl_Pixmap *img, int w, int h, const char *const*data) {
  Fl_Image_Surface *surf = new Fl_Image_Surface(w, h);
  Fl_Surface_Device::push_current(surf);
  fl_draw_pixmap(data, 0, 0, FL_BLACK);
  CGContextRef src = surf->get_offscreen_before_delete();
  Fl_Surface_Device::pop_current();
  delete surf;
  void *cgdata = CGBitmapContextGetData(src);
  int sw = CGBitmapContextGetWidth(src);
  int sh = CGBitmapContextGetHeight(src);
  CGImageAlphaInfo alpha = CGBitmapContextGetAlphaInfo(src);
  CGColorSpaceRef lut = CGColorSpaceCreateDeviceRGB();
  CGDataProviderRef src_bytes = CGDataProviderCreateWithData(src, cgdata, sw*sh*4, pmProviderRelease);
  CGImageRef cgimg = CGImageCreate( sw, sh, 8, 4*8, 4*sw, lut, alpha,
                                 src_bytes, 0L, false, kCGRenderingIntentDefault);
  CGColorSpaceRelease(lut);
  CGDataProviderRelease(src_bytes);
  return (fl_uintptr_t)cgimg;
}

void Fl_Quartz_Graphics_Driver::draw_CGImage(CGImageRef cgimg, int x, int y, int w, int h, int srcx, int srcy, int sw, int sh)
{
  CGRect rect = CGRectMake(x, y, w, h);
  CGContextSaveGState(gc_);
  CGContextClipToRect(gc_, CGRectOffset(rect, -0.5, -0.5 ));
  // move graphics context to origin of vertically reversed image
  // The 0.5 here cancels the 0.5 offset present in Quartz graphics contexts.
  // Thus, image and surface pixels are in phase if there's no scaling.
  CGContextTranslateCTM(gc_, rect.origin.x - srcx - 0.5, rect.origin.y - srcy + sh - 0.5);
  CGContextScaleCTM(gc_, 1, -1);
  CGAffineTransform at = CGContextGetCTM(gc_);
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
      if (high_resolution()) { // make .tx and .ty have int or half-int values
        deltax = -(at.tx*2 - round(at.tx*2));
        deltay = (at.ty*2 - round(at.ty*2));
      } else { // make .tx and .ty have integral values
        deltax = (at.tx - round(at.tx))*2;
        deltay = (at.ty - round(at.ty))*2;
      }
    }
    if (doit) CGContextTranslateCTM(gc_, -deltax, -deltay);
  }
  CGContextDrawImage(gc_, CGRectMake(0, 0, sw, sh), cgimg);
  CGContextRestoreGState(gc_);
}

void Fl_Quartz_Graphics_Driver::uncache_pixmap(fl_uintptr_t p) {
  CGImageRelease((CGImageRef)p);
}

//
// End of "$Id$".
//
