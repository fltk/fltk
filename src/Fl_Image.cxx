//
// "$Id: Fl_Image.cxx,v 1.5.2.3.2.4 2001/11/19 20:59:59 easysw Exp $"
//
// Image drawing code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2001 by Bill Spitzak and others.
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
#include <FL/fl_draw.H>
#include <FL/x.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Image.H>
#include <string.h>

void fl_restore_clip(); // in fl_rect.cxx

Fl_Image::~Fl_Image() {
}

void Fl_Image::draw(int XP, int YP, int WP, int HP, int cx, int cy) {
}

Fl_Image *Fl_Image::copy(int W, int H) {
  return new Fl_Image(W, H);
}

void Fl_Image::color_average(Fl_Color c, float i) {
}

void Fl_Image::desaturate() {
}

void Fl_Image::label(Fl_Widget* w) {
  w->image(this);
}

void Fl_Image::label(Fl_Menu_Item* m) {
}

Fl_RGB_Image::~Fl_RGB_Image() {
  if (id) fl_delete_offscreen((Fl_Offscreen)id);
  if (alloc_array) delete[] (uchar *)array;
}

Fl_Image *Fl_RGB_Image::copy(int W, int H) {
  // Optimize the simple copy where the width and height are the same...
  if (W == w() && H == h()) return new Fl_RGB_Image(array, w(), h(), d, ld);

  // OK, need to resize the image data; allocate memory and 
  Fl_RGB_Image	*new_image;	// New RGB image
  uchar		*new_array,	// New array for image data
		*new_ptr;	// Pointer into new array
  const uchar	*old_ptr;	// Pointer into old array
  int		c,		// Channel number
		sy,		// Source coordinate
		dx, dy,		// Destination coordinates
		xerr, yerr,	// X & Y errors
		xmod, ymod,	// X & Y moduli
		xstep, ystep;	// X & Y step increments


  // Figure out Bresenheim step/modulus values...
  xmod   = w() % W;
  xstep  = (w() / W) * d;
  ymod   = h() % H;
  ystep  = h() / H;

  // Allocate memory for the new image...
  new_array = new uchar [W * H * d];
  new_image = new Fl_RGB_Image(new_array, W, H, d);
  new_image->alloc_array = 1;

  // Scale the image using a nearest-neighbor algorithm...
  for (dy = h(), sy = 0, yerr = H / 2, new_ptr = new_array; dy > 0; dy --) {
    for (dx = w(), xerr = W / 2, old_ptr = array + sy * (w() * d + ld);
	 dx > 0;
	 dx --) {
      for (c = 0; c < d; c ++) *new_ptr++ = old_ptr[c];

      old_ptr += xstep;
      xerr    -= xmod;

      if (xerr <= 0) {
	xerr    += W;
	old_ptr += d;
      }
    }

    sy   += ystep;
    yerr -= ymod;
    if (yerr <= 0) {
      yerr += H;
      sy ++;
    }
  }

  return new_image;
}

void Fl_RGB_Image::color_average(Fl_Color c, float i) {
  // Delete any existing pixmap/mask objects...
  if (id) {
    fl_delete_offscreen(id);
    id = 0;
  }

  if (mask) {
    fl_delete_bitmask(mask);
    mask = 0;
  }

  // Allocate memory as needed...
  uchar		*new_array,
		*new_ptr;

  if (!alloc_array) new_array = new uchar[h() * w() * d];
  else new_array = (uchar *)array;

  // Get the color to blend with...
  uchar		r, g, b;
  unsigned	ia, ir, ig, ib;

  Fl::get_color(c, r, g, b);
  if (i < 0.0f) i = 0.0f;
  else if (i > 1.0f) i = 1.0f;

  ia = (unsigned)(256 * i);
  ir = r * (256 - ia);
  ig = g * (256 - ia);
  ib = b * (256 - ia);

  // Update the image data to do the blend...
  const uchar	*old_ptr;
  int		x, y;

  if (d < 3) {
    ig = (r * 31 + g * 61 + b * 8) / 100 * (256 - ia);

    for (new_ptr = new_array, old_ptr = array, y = 0; y < h(); y ++, old_ptr += ld)
      for (x = 0; x < w(); x ++) {
	*new_ptr++ = (*old_ptr++ * ia + ig) >> 8;
	if (d > 1) *new_ptr++ = *old_ptr++;
      }
  } else {
    for (new_ptr = new_array, old_ptr = array, y = 0; y < h(); y ++, old_ptr += ld)
      for (x = 0; x < w(); x ++) {
	*new_ptr++ = (*old_ptr++ * ia + ir) >> 8;
	*new_ptr++ = (*old_ptr++ * ia + ig) >> 8;
	*new_ptr++ = (*old_ptr++ * ia + ib) >> 8;
	if (d > 3) *new_ptr++ = *old_ptr++;
      }
  }

  // Set the new pointers/values as needed...
  if (!alloc_array) {
    array       = new_array;
    alloc_array = 1;
    ld          = 0;
  }
}

void Fl_RGB_Image::desaturate() {
  // Can only desaturate color images...
  if (d < 3) return;

  // Delete any existing pixmap/mask objects...
  if (id) {
    fl_delete_offscreen(id);
    id = 0;
  }

  if (mask) {
    fl_delete_bitmask(mask);
    mask = 0;
  }

  // Allocate memory for a grayscale image...
  uchar		*new_array,
		*new_ptr;
  int		new_d;

  new_d     = d - 2;
  new_array = new uchar[h() * w() * new_d];

  // Copy the image data, converting to grayscale...
  const uchar	*old_ptr;
  int		x, y;

  for (new_ptr = new_array, old_ptr = array, y = 0; y < h(); y ++, old_ptr += ld)
    for (x = 0; x < w(); x ++, old_ptr += d) {
      *new_ptr++ = (31 * old_ptr[0] + 61 * old_ptr[1] + 8 * old_ptr[2]) / 100;
      if (d > 3) *new_ptr++ = old_ptr[3];
    }

  // Free the old array as needed, and then set the new pointers/values...
  if (alloc_array) delete[] (uchar *)array;

  array       = new_array;
  alloc_array = 1;
  d           = new_d;
  ld          = 0;
}

void Fl_RGB_Image::draw(int XP, int YP, int WP, int HP, int cx, int cy) {
  // account for current clip region (faster on Irix):
  int X,Y,W,H; fl_clip_box(XP,YP,WP,HP,X,Y,W,H);
  cx += X-XP; cy += Y-YP;
  // clip the box down to the size of image, quit if empty:
  if (cx < 0) {W += cx; X -= cx; cx = 0;}
  if (cx+W > w()) W = w()-cx;
  if (W <= 0) return;
  if (cy < 0) {H += cy; Y -= cy; cy = 0;}
  if (cy+H > h()) H = h()-cy;
  if (H <= 0) return;
  if (!id) {
    id = fl_create_offscreen(w(), h());
    fl_begin_offscreen((Fl_Offscreen)id);
    fl_draw_image(array, 0, 0, w(), h(), d, ld);
    fl_end_offscreen();

    if (d == 2 || d == 4) {
      // Create alpha mask...
      int bmw = (w() + 7) / 8;
      uchar *bitmap = new uchar[bmw * h()];
      uchar *bitptr, bit;
      const uchar *dataptr;
      int x, y;
      static uchar dither[16][16] = { // Simple 16x16 Floyd dither
	{ 0,   128, 32,  160, 8,   136, 40,  168,
	  2,   130, 34,  162, 10,  138, 42,  170 },
	{ 192, 64,  224, 96,  200, 72,  232, 104,
	  194, 66,  226, 98,  202, 74,  234, 106 },
	{ 48,  176, 16,  144, 56,  184, 24,  152,
	  50,  178, 18,  146, 58,  186, 26,  154 },
	{ 240, 112, 208, 80,  248, 120, 216, 88,
	  242, 114, 210, 82,  250, 122, 218, 90 },
	{ 12,  140, 44,  172, 4,   132, 36,  164,
	  14,  142, 46,  174, 6,   134, 38,  166 },
	{ 204, 76,  236, 108, 196, 68,  228, 100,
	  206, 78,  238, 110, 198, 70,  230, 102 },
	{ 60,  188, 28,  156, 52,  180, 20,  148,
	  62,  190, 30,  158, 54,  182, 22,  150 },
	{ 252, 124, 220, 92,  244, 116, 212, 84,
	  254, 126, 222, 94,  246, 118, 214, 86 },
	{ 3,   131, 35,  163, 11,  139, 43,  171,
	  1,   129, 33,  161, 9,   137, 41,  169 },
	{ 195, 67,  227, 99,  203, 75,  235, 107,
	  193, 65,  225, 97,  201, 73,  233, 105 },
	{ 51,  179, 19,  147, 59,  187, 27,  155,
	  49,  177, 17,  145, 57,  185, 25,  153 },
	{ 243, 115, 211, 83,  251, 123, 219, 91,
	  241, 113, 209, 81,  249, 121, 217, 89 },
	{ 15,  143, 47,  175, 7,   135, 39,  167,
	  13,  141, 45,  173, 5,   133, 37,  165 },
	{ 207, 79,  239, 111, 199, 71,  231, 103,
	  205, 77,  237, 109, 197, 69,  229, 101 },
	{ 63,  191, 31,  159, 55,  183, 23,  151,
	  61,  189, 29,  157, 53,  181, 21,  149 },
	{ 254, 127, 223, 95,  247, 119, 215, 87,
	  253, 125, 221, 93,  245, 117, 213, 85 }
      };

      // Right now do a "screen door" alpha mask; not always pretty, but
      // definitely fast...
      memset(bitmap, 0, bmw * h());

      for (dataptr = array + d - 1, y = 0; y < h(); y ++, dataptr += ld)
        for (bitptr = bitmap + y * bmw, bit = 128, x = 0; x < w(); x ++, dataptr += d) {
	  if (*dataptr > dither[x & 15][y & 15])
	    *bitptr |= bit;
	  if (bit > 1) bit >>= 1;
	  else {
	    bit = 128;
	    bitptr ++;
	  }
	}

#if 0 // MRS: Don't think this is necessary; try using new fl_create_bitmask code...
#ifdef WIN32 // Matt: mask done
      // this won't work ehen the user changes display mode during run or
      // has two screens with differnet depths
      static uchar hiNibble[16] =
      { 0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
	0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0 };
      static uchar loNibble[16] =
      { 0x00, 0x08, 0x04, 0x0c, 0x02, 0x0a, 0x06, 0x0e,
	0x01, 0x09, 0x05, 0x0d, 0x03, 0x0b, 0x07, 0x0f };
      int np  = GetDeviceCaps(fl_gc, PLANES);	//: was always one on sample machines
      int bpp = GetDeviceCaps(fl_gc, BITSPIXEL);//: 1,4,8,16,24,32 and more odd stuff?
      int Bpr = (bpp*w()+7)/8;			//: bytes per row
      int pad = Bpr&1, w1 = (w()+7)/8, shr = ((w()-1)&7)+1;
      if (bpp==4) shr = (shr+1)/2;
      uchar *newarray = new uchar[(Bpr+pad)*h()], *dst = newarray, *src = bitmap;
      for (int i=0; i<h(); i++) {
	//: this is slooow, but we do it only once per pixmap
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
      mask = (ulong)CreateBitmap(w(), h(), np, bpp, newarray);
      delete[] newarray;
#else
      mask = XCreateBitmapFromData(fl_display, fl_window,
				   (const char*)bitmap, (w()+7)&-8, h());
#endif
#endif // 0

      mask = fl_create_bitmask(w(), h(), bitmap);
      delete[] bitmap;
    }
  }
#ifdef WIN32
  if (mask) {
    HDC new_gc = CreateCompatibleDC(fl_gc);
    SelectObject(new_gc, (void*)mask);
    BitBlt(fl_gc, X, Y, W, H, new_gc, cx, cy, SRCAND);
    SelectObject(new_gc, (void*)id);
    BitBlt(fl_gc, X, Y, W, H, new_gc, cx, cy, SRCPAINT);
    DeleteDC(new_gc);
  } else {
    fl_copy_offscreen(X, Y, W, H, id, cx, cy);
  }
#else
  if (mask) {
    // I can't figure out how to combine a mask with existing region,
    // so cut the image down to a clipped rectangle:
    int nx, ny; fl_clip_box(X,Y,W,H,nx,ny,W,H);
    cx += nx-X; X = nx;
    cy += ny-Y; Y = ny;
    // make X use the bitmap as a mask:
    XSetClipMask(fl_display, fl_gc, mask);
    int ox = X-cx; if (ox < 0) ox += w();
    int oy = Y-cy; if (oy < 0) oy += h();
    XSetClipOrigin(fl_display, fl_gc, X-cx, Y-cy);
  }
  fl_copy_offscreen(X, Y, W, H, id, cx, cy);
  if (mask) {
    // put the old clip region back
    XSetClipOrigin(fl_display, fl_gc, 0, 0);
    fl_restore_clip();
  }
#endif
}

void Fl_RGB_Image::label(Fl_Widget* w) {
  w->image(this);
}

void Fl_RGB_Image::label(Fl_Menu_Item* m) {
}


//
// End of "$Id: Fl_Image.cxx,v 1.5.2.3.2.4 2001/11/19 20:59:59 easysw Exp $".
//
