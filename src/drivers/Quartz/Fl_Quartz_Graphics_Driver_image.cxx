//
// MacOS image drawing code for the Fast Light Tool Kit (FLTK).
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
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/platform.H>
#include <FL/Fl_Image_Surface.H>

#define MAXBUFFER 0x40000 // 256k

static void dataReleaseCB(void *info, const void *data, size_t size)
{
  delete[] (uchar *)data;
}

/*
 draw an image based on the input parameters

 buf:       image source data
 X, Y:      position (in buffer?!)
 W, H:      size of picture (in pixel?)
 delta:     distance from pixel to pixel in buf in bytes
 linedelta: distance from line to line in buf in bytes
 mono:      if set, pixel is one byte - if zero, pixel is 3 byte
 cb:        callback to copy image data into (RGB?) buffer
   buf:       pointer to first byte in image source
   x, y:      position in buffer
   w:         width (in bytes?)
   dst:       destination buffer
 userdata:  ?
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
  if (cb || driver->has_feature(Fl_Quartz_Graphics_Driver::PRINTER)) {
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
  d &= ~FL_IMAGE_WITH_ALPHA;
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


void Fl_Quartz_Graphics_Driver::draw_bitmap(Fl_Bitmap *bm, int XP, int YP, int WP, int HP, int cx, int cy) {
  int X, Y, W, H;
  if (!bm->array) {
    draw_empty(bm, XP, YP);
    return;
  }
  if (start_image(bm, XP,YP,WP,HP,cx,cy,X,Y,W,H)) return;
  if (!*id(bm))
    cache(bm);

  if (*Fl_Graphics_Driver::id(bm) && gc_) {
    draw_CGImage((CGImageRef)*Fl_Graphics_Driver::id(bm), X,Y,W,H, cx, cy, bm->w(), bm->h());
  }
}

void Fl_Quartz_Graphics_Driver::cache(Fl_RGB_Image *rgb) {
  CGColorSpaceRef lut = rgb->d()<=2 ? CGColorSpaceCreateDeviceGray() : CGColorSpaceCreateDeviceRGB();
  int ld = rgb->ld();
  if (!ld) ld = rgb->data_w() * rgb->d();
  CGDataProviderRef src;
  if ( has_feature(PRINTER) ) {
    // When printing or copying to clipboard, the data at rgb->array are used when
    // the PDF page is completed, that is, after return from this function.
    // At that stage, the rgb object has possibly been deleted. It is therefore necessary
    // to use a copy of rgb->array. The mask_ member of rgb
    // is used to avoid repeating the copy operation if rgb is drawn again.
    // The CGImage data provider deletes the copy at the latest of these two events:
    // deletion of rgb, and completion of the PDF page where rgb was drawn.
    size_t total = ld * rgb->data_h();
    uchar *copy = new uchar[total];
    memcpy(copy, rgb->array, total);
    src = CGDataProviderCreateWithData(NULL, copy, total, dataReleaseCB);
    *Fl_Graphics_Driver::mask(rgb) = 1;
  } else {
    // the CGImage data provider must not release the image data.
    src = CGDataProviderCreateWithData(NULL, rgb->array, ld * rgb->data_h(), NULL);
  }
  CGImageRef cgimg = CGImageCreate(rgb->data_w(), rgb->data_h(), 8, rgb->d()*8, ld,
                        lut, (rgb->d()&1)?kCGImageAlphaNone:kCGImageAlphaLast,
                        src, 0L, false, kCGRenderingIntentDefault);
  *Fl_Graphics_Driver::id(rgb) = (fl_uintptr_t)cgimg;
  CGColorSpaceRelease(lut);
  CGDataProviderRelease(src);
}

void Fl_Quartz_Graphics_Driver::draw_rgb(Fl_RGB_Image *img, int XP, int YP, int WP, int HP, int cx, int cy) {
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
    cache(img);
    cgimg = (CGImageRef)*Fl_Graphics_Driver::id(img);
  }
  if (cgimg && gc_) {
    draw_CGImage(cgimg, X,Y,W,H, cx,cy, img->w(), img->h());
  }
}

void Fl_Quartz_Graphics_Driver::draw_pixmap(Fl_Pixmap *pxm, int XP, int YP, int WP, int HP, int cx, int cy) {
  int X, Y, W, H;
  if (!pxm->data() || !pxm->w()) {
    draw_empty(pxm, XP, YP);
    return;
  }
  if ( start_image(pxm, XP,YP,WP,HP,cx,cy,X,Y,W,H) ) return;
  if (!*id(pxm)) {
    cache(pxm);
  }

  CGImageRef cgimg = (CGImageRef)*Fl_Graphics_Driver::id(pxm);
  draw_CGImage(cgimg, X,Y,W,H, cx,cy, pxm->w(), pxm->h());
}

CGImageRef Fl_Quartz_Graphics_Driver::create_bitmask(int w, int h, const uchar *array) {
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
  return id_;
}

void Fl_Quartz_Graphics_Driver::delete_bitmask(fl_uintptr_t bm) {
  if (bm) CGImageRelease((CGImageRef)bm);
}

void Fl_Quartz_Graphics_Driver::uncache(Fl_RGB_Image*, fl_uintptr_t &id_, fl_uintptr_t &mask_) {
  if (id_) {
    CGImageRelease((CGImageRef)id_);
    id_ = 0;
    mask_ = 0;
  }
}

void Fl_Quartz_Graphics_Driver::cache(Fl_Bitmap *bm) {
  *Fl_Graphics_Driver::id(bm) = (fl_uintptr_t)create_bitmask(bm->data_w(), bm->data_h(), bm->array);
}


static void pmProviderRelease (void *ctxt, const void *data, size_t size) {
  CFRelease(ctxt);
}

void Fl_Quartz_Graphics_Driver::cache(Fl_Pixmap *img) {
  Fl_Image_Surface *surf = new Fl_Image_Surface(img->data_w(), img->data_h());
  Fl_Surface_Device::push_current(surf);
  fl_draw_pixmap(img->data(), 0, 0, FL_BLACK);
  Fl_Surface_Device::pop_current();
  CGContextRef src = (CGContextRef)Fl_Graphics_Driver::get_offscreen_and_delete_image_surface(surf);
  void *cgdata = CGBitmapContextGetData(src);
  int sw = (int)CGBitmapContextGetWidth(src);
  int sh = (int)CGBitmapContextGetHeight(src);
  CGImageAlphaInfo alpha = CGBitmapContextGetAlphaInfo(src);
  CGColorSpaceRef lut = CGColorSpaceCreateDeviceRGB();
  CGDataProviderRef src_bytes = CGDataProviderCreateWithData(src, cgdata, sw*sh*4, pmProviderRelease);
  CGImageRef cgimg = CGImageCreate( sw, sh, 8, 4*8, 4*sw, lut, alpha,
                                 src_bytes, 0L, false, kCGRenderingIntentDefault);
  CGColorSpaceRelease(lut);
  CGDataProviderRelease(src_bytes);
  *Fl_Graphics_Driver::id(img) = (fl_uintptr_t)cgimg;
}

void Fl_Quartz_Graphics_Driver::draw_CGImage(CGImageRef cgimg, int x, int y, int w, int h, int srcx, int srcy, int sw, int sh)
{
  CGRect rect = CGRectMake(x, y, w, h);
  CGContextSaveGState(gc_);
  CGContextClipToRect(gc_, CGRectOffset(rect, -0.5, -0.5 ));
  // move graphics context to origin of vertically reversed image
  // The 0.5 here cancels the 0.5 offset present in Quartz graphics contexts.
  // Thus, image and surface pixels are in phase.
  CGContextTranslateCTM(gc_, rect.origin.x - srcx - 0.5, rect.origin.y - srcy + sh - 0.5);
  CGContextScaleCTM(gc_, 1, -1);
  CGContextDrawImage(gc_, CGRectMake(0, 0, sw, sh), cgimg);
  CGContextRestoreGState(gc_);
}

void Fl_Quartz_Graphics_Driver::uncache_pixmap(fl_uintptr_t pixmap_ref) {
  CGImageRelease((CGImageRef)pixmap_ref);
}
