//
// "$Id: Fl_Pixmap.cxx,v 1.9.2.4.2.4 2001/11/19 01:06:45 easysw Exp $"
//
// Pixmap drawing code for the Fast Light Tool Kit (FLTK).
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

// Draws X pixmap data, keeping it stashed in a server pixmap so it
// redraws fast.

// See fl_draw_pixmap.C for code used to get the actual data into pixmap.
// Implemented without using the xpm library (which I can't use because
// it interferes with the color cube used by fl_draw_image).

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/x.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Pixmap.H>

#include <stdio.h>
#include <string.h>
#include <ctype.h>

extern uchar **fl_mask_bitmap; // used by fl_draw_pixmap.cxx to store mask
void fl_restore_clip(); // in fl_rect.cxx

void Fl_Pixmap::measure() {
  int W, H;

  // ignore empty or bad pixmap data:
  if (w()<0) {
    fl_measure_pixmap(data, W, H);
    w(W); h(H);
  }
}

void Fl_Pixmap::draw(int XP, int YP, int WP, int HP, int cx, int cy) {
  // ignore empty or bad pixmap data:
  if (w()<0) {
    measure();
    if (WP==-1) { WP = w(); HP = h(); }
  }
  if (!w()) return;
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
    fl_begin_offscreen(id);
    uchar *bitmap = 0;
    fl_mask_bitmap = &bitmap;
    fl_draw_pixmap(data, 0, 0, FL_BLACK);
    fl_mask_bitmap = 0;
    if (bitmap) {
      mask = fl_create_bitmask(w(), h(), bitmap);
#if 0 // Don't think this is needed; try using fl_create_bitmask()...
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
      delete[] bitmap;
    }

    fl_end_offscreen();
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
    fl_copy_offscreen(X, Y, W, H, (Fl_Offscreen)id, cx, cy);
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
  fl_copy_offscreen(X, Y, W, H, (Fl_Offscreen)id, cx, cy);
  if (mask) {
    // put the old clip region back
    XSetClipOrigin(fl_display, fl_gc, 0, 0);
    fl_restore_clip();
  }
#endif
}

Fl_Pixmap::~Fl_Pixmap() {
  if (id) fl_delete_offscreen(id);
  if (mask) fl_delete_bitmask(mask);
  delete_data();
}

void Fl_Pixmap::label(Fl_Widget* w) {
  w->image(this);
}

void Fl_Pixmap::label(Fl_Menu_Item* m) {
}

void Fl_Pixmap::copy_data() {
  if (alloc_data) return;

  char		**new_data,	// New data array
		**new_row;	// Current row in image
  int		i,		// Looping var
		ncolors,	// Number of colors in image
		chars_per_pixel,// Characters per color
		chars_per_line;	// Characters per line 

  // Figure out how many colors there are, and how big they are...
  sscanf(data[0],"%*d%*d%d%d", &ncolors, &chars_per_pixel);
  chars_per_line = chars_per_pixel * w() + 1;

  // Allocate memory for the new array...
  new_data    = new char *[h() + ncolors + 1];
  new_data[0] = new char[strlen(data[0]) + 1];
  strcpy(new_data[0], data[0]);

  // Copy colors...
  if (ncolors < 0) {
    // Copy FLTK colormap values...
    ncolors = -ncolors;
    for (i = 0, new_row = new_data + 1; i < ncolors; i ++, new_row ++) {
      *new_row = new char[4];
      memcpy(*new_row, data[i + 1], 4);
    }
  } else {
    // Copy standard XPM colormap values...
    for (i = 0, new_row = new_data + 1; i < ncolors; i ++, new_row ++) {
      *new_row = new char[strlen(data[i + 1]) + 1];
      strcpy(*new_row, data[i + 1]);
    }
  }

  // Copy image data...
  for (i = 0; i < h(); i ++, new_row ++) {
    *new_row = new char[chars_per_line];
    memcpy(*new_row, data[i + ncolors + 1], chars_per_line);
  }

  // Update pointers...
  data       = new_data;
  alloc_data = 1;  
}

Fl_Image *Fl_Pixmap::copy(int W, int H) {
  // Optimize the simple copy where the width and height are the same...
  if (W == w() && H == h()) return new Fl_Pixmap(data);

  // OK, need to resize the image data; allocate memory and 
  Fl_Pixmap	*new_image;	// New pixmap
  char		**new_data,	// New array for image data
		**new_row,	// Pointer to row in image data
		*new_ptr,	// Pointer into new array
		new_info[255];	// New information line
  const char	*old_ptr;	// Pointer into old array
  int		i,		// Looping var
		c,		// Channel number
		sy,		// Source coordinate
		dx, dy,		// Destination coordinates
		xerr, yerr,	// X & Y errors
		xmod, ymod,	// X & Y moduli
		xstep, ystep;	// X & Y step increments
  int		ncolors,	// Number of colors in image
		chars_per_pixel,// Characters per color
		chars_per_line;	// Characters per line 

  // Figure out how many colors there are, and how big they are...
  sscanf(data[0],"%*d%*d%d%d", &ncolors, &chars_per_pixel);
  chars_per_line = chars_per_pixel * W + 1;

  sprintf(new_info, "%d %d %d %d", W, H, ncolors, chars_per_pixel);

  // Figure out Bresenheim step/modulus values...
  xmod   = w() % W;
  xstep  = (w() / W) * chars_per_pixel;
  ymod   = h() % H;
  ystep  = h() / H;

  // Allocate memory for the new array...
  new_data    = new char *[H + ncolors + 1];
  new_data[0] = new char[strlen(new_info) + 1];
  strcpy(new_data[0], new_info);

  // Copy colors...
  if (ncolors < 0) {
    // Copy FLTK colormap values...
    ncolors = -ncolors;
    for (i = 0, new_row = new_data + 1; i < ncolors; i ++, new_row ++) {
      *new_row = new char[4];
      memcpy(*new_row, data[i + 1], 4);
    }
  } else {
    // Copy standard XPM colormap values...
    for (i = 0, new_row = new_data + 1; i < ncolors; i ++, new_row ++) {
      *new_row = new char[strlen(data[i + 1]) + 1];
      strcpy(*new_row, data[i + 1]);
    }
  }

  // Copy image data...
  for (i = 0; i < h(); i ++, new_row ++) {
    *new_row = new char[chars_per_line];
    memcpy(*new_row, data[i + ncolors + 1], chars_per_line);
  }
  // Scale the image using a nearest-neighbor algorithm...
  for (dy = h(), sy = 0, yerr = H / 2; dy > 0; dy --, new_row ++) {
    *new_row = new char[chars_per_line];
    new_ptr  = *new_row;

    for (dx = w(), xerr = W / 2, old_ptr = data[sy + ncolors + 1];
	 dx > 0;
	 dx --) {
      for (c = 0; c < chars_per_pixel; c ++) *new_ptr++ = old_ptr[c];

      old_ptr += xstep;
      xerr    -= xmod;

      if (xerr <= 0) {
	xerr    += W;
	old_ptr += chars_per_pixel;
      }
    }

    *new_ptr = '\0';
    sy       += ystep;
    yerr     -= ymod;
    if (yerr <= 0) {
      yerr += H;
      sy ++;
    }
  }

  new_image = new Fl_Pixmap((const char * const *)data);
  new_image->alloc_data = 1;

  return new_image;
}

void Fl_Pixmap::color_average(Fl_Color c, float i) {
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
  copy_data();

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

  // Update the colormap to do the blend...
  char		line[255];	// New colormap line
  int		color,		// Looping var
		ncolors,	// Number of colors in image
		chars_per_pixel;// Characters per color


  sscanf(data[0],"%*d%*d%d%d", &ncolors, &chars_per_pixel);

  if (ncolors < 0) {
    // Update FLTK colormap...
    ncolors = -ncolors;
    for (color = 0; color < ncolors; color ++) {
      ((char *)data[color + 1])[1] = (ia * data[color + 1][1] + ir) >> 8;
      ((char *)data[color + 1])[2] = (ia * data[color + 1][2] + ig) >> 8;
      ((char *)data[color + 1])[3] = (ia * data[color + 1][3] + ib) >> 8;
    }
  } else {
    // Update standard XPM colormap...
    for (color = 0; color < ncolors; color ++) {
      // look for "c word", or last word if none:
      const char *p = data[color + 1] + chars_per_pixel + 1;
      const char *previous_word = p;
      for (;;) {
	while (*p && isspace(*p)) p++;
	char what = *p++;
	while (*p && !isspace(*p)) p++;
	while (*p && isspace(*p)) p++;
	if (!*p) {p = previous_word; break;}
	if (what == 'c') break;
	previous_word = p;
	while (*p && !isspace(*p)) p++;
      }

#ifdef WIN32
      if (fl_parse_color(p, r, g, b) {
#else
      XColor x;
      if (XParseColor(fl_display, fl_colormap, p, &x)) {
	r = x.red>>8;
	g = x.green>>8;
	b = x.blue>>8;
#endif

        r = (ia * r + ir) >> 8;
        g = (ia * g + ig) >> 8;
        b = (ia * b + ib) >> 8;

        if (chars_per_pixel > 1) sprintf(line, "%c%c c #%02X%02X%02X",
	                                 data[color + 1][0],
	                                 data[color + 1][1], r, g, b);
        else sprintf(line, "%c c #%02X%02X%02X", data[color + 1][0], r, g, b);

        delete[] data[color + 1];
	((char **)data)[color + 1] = new char[strlen(line) + 1];
	strcpy((char *)data[color + 1], line);
      }
    }
  }
}

void Fl_Pixmap::delete_data() {
  if (alloc_data) {
    for (int i = 0; data[i]; i ++) delete[] data[i];
    delete[] data;
  }
}

void Fl_Pixmap::desaturate() {
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
  copy_data();

  // Update the colormap to grayscale...
  char		line[255];	// New colormap line
  int		i,		// Looping var
		ncolors,	// Number of colors in image
		chars_per_pixel;// Characters per color
  uchar		r, g, b;

  sscanf(data[0],"%*d%*d%d%d", &ncolors, &chars_per_pixel);

  if (ncolors < 0) {
    // Update FLTK colormap...
    ncolors = -ncolors;
    for (i = 0; i < ncolors; i ++) {
      g = (data[i + 1][1] * 31 + data[i + 1][2] * 61 + data[i + 1][3] * 8) / 100;
      ((char *)data[i + 1])[1] =
      ((char *)data[i + 1])[2] =
      ((char *)data[i + 1])[3] = g;
    }
  } else {
    // Update standard XPM colormap...
    for (i = 0; i < ncolors; i ++) {
      // look for "c word", or last word if none:
      const char *p = data[i + 1] + chars_per_pixel + 1;
      const char *previous_word = p;
      for (;;) {
	while (*p && isspace(*p)) p++;
	char what = *p++;
	while (*p && !isspace(*p)) p++;
	while (*p && isspace(*p)) p++;
	if (!*p) {p = previous_word; break;}
	if (what == 'c') break;
	previous_word = p;
	while (*p && !isspace(*p)) p++;
      }

#ifdef WIN32
      if (fl_parse_color(p, r, g, b) {
#else
      XColor x;
      if (XParseColor(fl_display, fl_colormap, p, &x)) {
	r = x.red>>8;
	g = x.green>>8;
	b = x.blue>>8;
#endif

        g = (r * 31 + g * 61 + b * 8) / 100;

        if (chars_per_pixel > 1) sprintf(line, "%c%c c #%02X%02X%02X", data[i + 1][0],
	                                 data[i + 1][1], g, g, g);
        else sprintf(line, "%c c #%02X%02X%02X", data[i + 1][0], g, g, g);

        delete[] data[i + 1];
	((char **)data)[i + 1] = new char[strlen(line) + 1];
	strcpy((char *)data[i + 1], line);
      }
    }
  }
}

//
// End of "$Id: Fl_Pixmap.cxx,v 1.9.2.4.2.4 2001/11/19 01:06:45 easysw Exp $".
//
