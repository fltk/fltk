//
// "$Id: Fl_Bitmap.cxx,v 1.5.2.4.2.9 2002/01/01 15:11:29 easysw Exp $"
//
// Bitmap drawing routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2002 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to "fltk-bugs@fltk.org".
//

#include <FL/Fl.H>
#include <FL/x.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Bitmap.H>
#include <string.h>

#ifdef __APPLE__ // MacOS bitmask functions
Fl_Bitmask fl_create_bitmask(int w, int h, const uchar *array) {
  Rect srcRect;
  srcRect.left = 0; srcRect.right = w;
  srcRect.top = 0; srcRect.bottom = h;
  GrafPtr savePort;

  GetPort(&savePort); // remember the current port

  Fl_Bitmask gw;
  NewGWorld( &gw, 1, &srcRect, 0L, 0L, 0 );
  PixMapHandle pm = GetGWorldPixMap( gw );
  if ( pm ) 
  {
    LockPixels( pm );
    if ( *pm ) 
    {
      uchar *base = (uchar*)GetPixBaseAddr( pm );
      if ( base ) 
      {
        PixMapPtr pmp = *pm;
        // verify the parameters for direct memory write
        if ( pmp->pixelType == 0 || pmp->pixelSize == 1 || pmp->cmpCount == 1 || pmp->cmpSize == 1 ) 
        {
          static uchar reverse[16] =	/* Bit reversal lookup table */
          { 0x00, 0x88, 0x44, 0xcc, 0x22, 0xaa, 0x66, 0xee, 0x11, 0x99, 0x55, 0xdd, 0x33, 0xbb, 0x77, 0xff };
          uchar *dst = base;
          const uchar *src = array;
          int rowBytesSrc = (w+7)>>3 ;
          int rowPatch = (pmp->rowBytes&0x3fff) - rowBytesSrc;
          for ( int j=0; j<h; j++,dst+=rowPatch )
            for ( int i=0; i<rowBytesSrc; i++,src++ )
              *dst++ = (reverse[*src & 0x0f] & 0xf0) | (reverse[(*src >> 4) & 0x0f] & 0x0f);
        }
      }
      UnlockPixels( pm );
    }
  }

  SetPort(savePort);
  return gw;               /* tell caller we succeeded! */
}

void fl_delete_bitmask(Fl_Bitmask id) {
  if (id) DisposeGWorld(id);
}
#elif defined(WIN32) // Windows bitmask functions...
// 'fl_create_bitmap()' - Create a 1-bit bitmap for drawing...
static Fl_Bitmask fl_create_bitmap(int w, int h, const uchar *data) {
  // we need to pad the lines out to words & swap the bits
  // in each byte.
  int w1 = (w+7)/8;
  int w2 = ((w+15)/16)*2;
  uchar* newarray = new uchar[w2*h];
  const uchar* src = data;
  uchar* dest = newarray;
  Fl_Bitmask id;
  static uchar reverse[16] =	/* Bit reversal lookup table */
  	      { 0x00, 0x88, 0x44, 0xcc, 0x22, 0xaa, 0x66, 0xee,
		0x11, 0x99, 0x55, 0xdd, 0x33, 0xbb, 0x77, 0xff };

  for (int y=0; y < h; y++) {
    for (int n = 0; n < w1; n++, src++)
      *dest++ = (reverse[*src & 0x0f] & 0xf0) |
	        (reverse[(*src >> 4) & 0x0f] & 0x0f);
    dest += w2-w1;
  }

  id = CreateBitmap(w, h, 1, 1, newarray);

  delete[] newarray;

  return id;
}

// 'fl_create_bitmask()' - Create an N-bit bitmap for masking...
Fl_Bitmask fl_create_bitmask(int w, int h, const uchar *data) {
  // this won't work when the user changes display mode during run or
  // has two screens with differnet depths
  Fl_Bitmask id;
  static uchar hiNibble[16] =
  { 0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
    0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0 };
  static uchar loNibble[16] =
  { 0x00, 0x08, 0x04, 0x0c, 0x02, 0x0a, 0x06, 0x0e,
    0x01, 0x09, 0x05, 0x0d, 0x03, 0x0b, 0x07, 0x0f };
  int np  = GetDeviceCaps(fl_gc, PLANES);	//: was always one on sample machines
  int bpp = GetDeviceCaps(fl_gc, BITSPIXEL);//: 1,4,8,16,24,32 and more odd stuff?
  int Bpr = (bpp*w+7)/8;			//: bytes per row
  int pad = Bpr&1, w1 = (w+7)/8, shr = ((w-1)&7)+1;
  if (bpp==4) shr = (shr+1)/2;
  uchar *newarray = new uchar[(Bpr+pad)*h];
  uchar *dst = newarray;
  const uchar *src = data;

  for (int i=0; i<h; i++) {
    // This is slooow, but we do it only once per pixmap
    for (int j=w1; j>0; j--) {
      uchar b = *src++;
      if (bpp==1) {
        *dst++ = ( hiNibble[b&15] ) | ( loNibble[(b>>4)&15] );
      } else if (bpp==4) {
        for (int k=(j==1)?shr:4; k>0; k--) {
          *dst++ = "\377\360\017\000"[b&3];
          b = b >> 2;
        }
      } else {
        for (int k=(j==1)?shr:8; k>0; k--) {
          if (b&1) {
            *dst++=0;
	    if (bpp>8) *dst++=0;
            if (bpp>16) *dst++=0;
	    if (bpp>24) *dst++=0;
	  } else {
	    *dst++=0xff;
	    if (bpp>8) *dst++=0xff;
	    if (bpp>16) *dst++=0xff;
	    if (bpp>24) *dst++=0xff;
	  }

	  b = b >> 1;
        }
      }
    }

    dst += pad;
  }

  id = CreateBitmap(w, h, np, bpp, newarray);
  delete[] newarray;

  return id;
}

Fl_Bitmask fl_create_bitmask(int w, int h, const uchar *data, int for_mask) {
  // we need to pad the lines out to words & swap the bits
  // in each byte.
  int w1 = (w+7)/8;
  int w2 = ((w+15)/16)*2;
  uchar* newarray = new uchar[w2*h];
  const uchar* src = data;
  uchar* dest = newarray;
  Fl_Bitmask id;
  static uchar reverse[16] =	/* Bit reversal lookup table */
  	      { 0x00, 0x88, 0x44, 0xcc, 0x22, 0xaa, 0x66, 0xee,
		0x11, 0x99, 0x55, 0xdd, 0x33, 0xbb, 0x77, 0xff };

  for (int y=0; y < h; y++) {
    for (int n = 0; n < w1; n++, src++)
      *dest++ = (reverse[*src & 0x0f] & 0xf0) |
	        (reverse[(*src >> 4) & 0x0f] & 0x0f);
    dest += w2-w1;
  }

  id = CreateBitmap(w, h, 1, 1, newarray);

  delete[] newarray;

  return (id);
}

void fl_delete_bitmask(Fl_Bitmask bm) {
  DeleteObject((HGDIOBJ)bm);
}
#else // X11 bitmask functions
Fl_Bitmask fl_create_bitmask(int w, int h, const uchar *data) {
  return XCreateBitmapFromData(fl_display, fl_window, (const char *)data,
                               (w+7)&-8, h);
}

void fl_delete_bitmask(Fl_Bitmask bm) {
  fl_delete_offscreen((Fl_Offscreen)bm);
}
#endif // __APPLE__

void Fl_Bitmap::draw(int XP, int YP, int WP, int HP, int cx, int cy) {
  if (!array) {
    draw_empty(XP, YP);
    return;
  }

  // account for current clip region (faster on Irix):
  int X,Y,W,H; fl_clip_box(XP,YP,WP,HP,X,Y,W,H);
  cx += X-XP; cy += Y-YP;
  // clip the box down to the size of image, quit if empty:
  if (cx < 0) {W += cx; X -= cx; cx = 0;}
  if ((cx+W) > w()) W = w()-cx;
  if (W <= 0) return;
  if (cy < 0) {H += cy; Y -= cy; cy = 0;}
  if ((cy+H) > h()) H = h()-cy;
  if (H <= 0) return;
#ifdef WIN32
  if (!id) id = fl_create_bitmap(w(), h(), array);

  HDC tempdc = CreateCompatibleDC(fl_gc);
  SelectObject(tempdc, (HGDIOBJ)id);
  SelectObject(fl_gc, fl_brush());
  // secret bitblt code found in old MSWindows reference manual:
  BitBlt(fl_gc, X, Y, W, H, tempdc, cx, cy, 0xE20746L);
  DeleteDC(tempdc);
#elif defined(__APPLE__)
  if (!id) id = fl_create_bitmask(w(), h(), array);
  GrafPtr dstPort;
  GetPort( &dstPort );
  Rect src, dst;
  GetPortBounds( id, &src );
  SetRect( &src, cx, cy, cx+W, cy+H );
  SetRect( &dst, X, Y, X+W, Y+H );
  CopyBits(
    GetPortBitMapForCopyBits(id),		// srcBits
    GetPortBitMapForCopyBits(dstPort),	// dstBits
    &src,		 			// src bounds
    &dst, 				// dst bounds
    srcOr, 				// mode
    0L);					// mask region
#else
  if (!id) id = fl_create_bitmask(w(), h(), array);

  XSetStipple(fl_display, fl_gc, id);
  int ox = X-cx; if (ox < 0) ox += w();
  int oy = Y-cy; if (oy < 0) oy += h();
  XSetTSOrigin(fl_display, fl_gc, ox, oy);
  XSetFillStyle(fl_display, fl_gc, FillStippled);
  XFillRectangle(fl_display, fl_window, fl_gc, X, Y, W, H);
  XSetFillStyle(fl_display, fl_gc, FillSolid);
#endif
}

Fl_Bitmap::~Fl_Bitmap() {
  if (id) fl_delete_bitmask(id);
  if (alloc_array) delete[] (uchar *)array;
}

void Fl_Bitmap::label(Fl_Widget* w) {
  w->image(this);
}

void Fl_Bitmap::label(Fl_Menu_Item* m) {
}

Fl_Image *Fl_Bitmap::copy(int W, int H) {
  // Optimize the simple copy where the width and height are the same...
  if (W == w() && H == h()) return new Fl_Bitmap(array, w(), h());

  // OK, need to resize the image data; allocate memory and 
  Fl_Bitmap	*new_image;	// New RGB image
  uchar		*new_array,	// New array for image data
		*new_ptr,	// Pointer into new array
		new_bit,	// Bit for new array
		old_bit;	// Bit for old array
  const uchar	*old_ptr;	// Pointer into old array
  int		sx, sy,		// Source coordinates
		dx, dy,		// Destination coordinates
		xerr, yerr,	// X & Y errors
		xmod, ymod,	// X & Y moduli
		xstep, ystep;	// X & Y step increments


  // Figure out Bresenheim step/modulus values...
  xmod   = w() % W;
  xstep  = w() / W;
  ymod   = h() % H;
  ystep  = h() / H;

  // Allocate memory for the new image...
  new_array = new uchar [H * (W + 7) / 8];
  new_image = new Fl_Bitmap(new_array, W, H);
  new_image->alloc_array = 1;

  memset(new_array, 0, H * (W + 7) / 8);

  // Scale the image using a nearest-neighbor algorithm...
  for (dy = H, sy = 0, yerr = H / 2, new_ptr = new_array; dy > 0; dy --) {
    for (dx = W, xerr = W / 2, old_ptr = array + sy * (w() + 7) / 8, sx = 0, new_bit = 128;
	 dx > 0;
	 dx --) {
      old_bit = 128 >> (sx & 7);
      if (old_ptr[sx / 8] & old_bit) *new_ptr |= new_bit;

      if (new_bit > 1) new_bit >>= 1;
      else {
        new_bit = 128;
	new_ptr ++;
      }

      sx   += xstep;
      xerr -= xmod;

      if (xerr <= 0) {
	xerr += W;
	sx ++;
      }
    }

    if (new_bit < 128) new_ptr ++;

    sy   += ystep;
    yerr -= ymod;
    if (yerr <= 0) {
      yerr += H;
      sy ++;
    }
  }

  return new_image;
}


//
// End of "$Id: Fl_Bitmap.cxx,v 1.5.2.4.2.9 2002/01/01 15:11:29 easysw Exp $".
//
