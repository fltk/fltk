//
// "$Id$"
//
// MacOS image drawing code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2012 by Bill Spitzak and others.
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

////////////////////////////////////////////////////////////////

#include <config.h>
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Printer.H>
#include <FL/x.H>

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
		    Fl_Draw_Image_Cb cb, void* userdata)
{
  if (!linedelta) linedelta = W*abs(delta);

  uchar *tmpBuf = 0;
  if (!cb) {
    if (delta < 0) buf -= (W-1)*(-delta);
    if (linedelta < 0) buf -= (H-1)*(-linedelta);
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
  if (abs(delta) <= 2)
    lut = CGColorSpaceCreateDeviceGray();
  else
    lut = CGColorSpaceCreateDeviceRGB();
  // a release callback is necessary when the fl_gc is a print context because the image data
  // must be kept until the page is closed. Thus tmpBuf can't be deleted here. It's too early.
  CGDataProviderRef src = CGDataProviderCreateWithData( 0L, array, abs(linedelta)*H,
						       tmpBuf ? dataReleaseCB : NULL
						       );
  CGImageRef        img = CGImageCreate( W, H, 8, 8*abs(delta), abs(linedelta),
                            lut, abs(delta)&1?kCGImageAlphaNone:kCGImageAlphaNoneSkipLast,
                            //lut, delta&1?kCGImageAlphaNone:kCGImageAlphaLast,
                            src, 0L, false, kCGRenderingIntentDefault);
  // draw the image into the destination context
  if (img) {
    CGRect rect = CGRectMake( X, Y,  W, H);
    Fl_X::q_begin_image(rect, 0, 0, W, H);
    if (linedelta < 0) {
      CGContextTranslateCTM(fl_gc, 0, H);
      CGContextScaleCTM(fl_gc, 1, -1);
    }
    if (delta < 0) {
      CGContextTranslateCTM(fl_gc, W, 0);
      CGContextScaleCTM(fl_gc, -1, 1);
    }
    CGContextDrawImage(fl_gc, rect, img);
    Fl_X::q_end_image();
    // release all allocated resources
    CGImageRelease(img);
  }
  CGColorSpaceRelease(lut);
  CGDataProviderRelease(src);
  if (img) return; // else fall through to slow mode
  // following the very save (and very slow) way to write the image into the give port
  CGContextSetShouldAntialias(fl_gc, false);
  if ( cb )
  {
    uchar *tmpBuf = new uchar[ W*4 ];
    for ( int i=0; i<H; i++ )
    {
      uchar *src = tmpBuf;
      cb( userdata, 0, i, W, tmpBuf );
      for ( int j=0; j<W; j++ )
      {
        if ( mono )
          { fl_color( src[0], src[0], src[0] ); }
        else
          { fl_color( src[0], src[1], src[2] ); }
        CGContextMoveToPoint(fl_gc, X+j, Y+i);
        CGContextAddLineToPoint(fl_gc, X+j, Y+i);
        CGContextStrokePath(fl_gc);
        src+=delta;
      }
    }
    delete[] tmpBuf;
  }
  else
  {
    for ( int i=0; i<H; i++ )
    {
      const uchar *src = buf+i*linedelta;
      for ( int j=0; j<W; j++ )
      {
        if ( mono )
          fl_color( src[0], src[0], src[0] );
        else
          fl_color( src[0], src[1], src[2] );
        CGContextMoveToPoint(fl_gc, X+j, Y+i);
        CGContextAddLineToPoint(fl_gc, X+j, Y+i);
        CGContextStrokePath(fl_gc);
        src += delta;
      }
    }
  }
  CGContextSetShouldAntialias(fl_gc, true);
}

void Fl_Quartz_Graphics_Driver::draw_image(const uchar* buf, int x, int y, int w, int h, int d, int l){
  innards(buf,x,y,w,h,d,l,(d<3&&d>-3),0,0);
}
void Fl_Quartz_Graphics_Driver::draw_image(Fl_Draw_Image_Cb cb, void* data,
		   int x, int y, int w, int h,int d) {
  innards(0,x,y,w,h,d,0,(d<3&&d>-3),cb,data);
}
void Fl_Quartz_Graphics_Driver::draw_image_mono(const uchar* buf, int x, int y, int w, int h, int d, int l){
  innards(buf,x,y,w,h,d,l,1,0,0);
}
void Fl_Quartz_Graphics_Driver::draw_image_mono(Fl_Draw_Image_Cb cb, void* data,
		   int x, int y, int w, int h,int d) {
  innards(0,x,y,w,h,d,0,1,cb,data);
}

void fl_rectf(int x, int y, int w, int h, uchar r, uchar g, uchar b) {
  fl_color(r,g,b);
  fl_rectf(x,y,w,h);
}

//
// End of "$Id$".
//
