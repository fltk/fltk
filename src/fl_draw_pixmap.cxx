//
// "$Id: fl_draw_pixmap.cxx,v 1.4.2.8 2001/04/13 19:07:40 easysw Exp $"
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

// Implemented without using the xpm library (which I can't use because
// it interferes with the color cube used by fl_draw_image).
// Current implementation is cheap and slow, and works best on a full-color
// display.  Transparency is not handled, and colors are dithered to
// the color cube.  Color index is achieved by adding the id
// characters together!  Also mallocs a lot of temporary memory!
// Notice that there is no pixmap file interface.  This is on purpose,
// as I want to discourage programs that require support files to work.
// All data needed by a program ui should be compiled in!!!

#include <config.h>
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/x.H>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

static int ncolors, chars_per_pixel;

int fl_measure_pixmap(/*const*/ char* const* data, int &w, int &h) {
  return fl_measure_pixmap((const char*const*)data,w,h);
}

int fl_measure_pixmap(const char * const *data, int &w, int &h) {
  int i = sscanf(data[0],"%d %d %d %d",&w,&h,&ncolors,&chars_per_pixel);
  if (i<4 || w<=0 || h<=0 ||
      chars_per_pixel!=1 && chars_per_pixel!=2) return w=0;
  return 1;
}

#ifdef U64

// The callback from fl_draw_image to get a row of data passes this:
struct pixmap_data {
  int w, h;
  const uchar*const* data;
  union {
    U64 colors[256];
    U64* byte1[256];
  };
};

// callback for 1 byte per pixel:
static void cb1(void*v, int x, int y, int w, uchar* buf) {
  pixmap_data& d = *(pixmap_data*)v;
  const uchar* p = d.data[y]+x;
  U64* q = (U64*)buf;
  for (int X=(w+1)/2; X--; p += 2) {
#if WORDS_BIGENDIAN
    *q++ = (d.colors[p[0]]<<32) | d.colors[p[1]];
#else
    *q++ = (d.colors[p[1]]<<32) | d.colors[p[0]];
#endif
  }
}

// callback for 2 bytes per pixel:
static void cb2(void*v, int x, int y, int w, uchar* buf) {
  pixmap_data& d = *(pixmap_data*)v;
  const uchar* p = d.data[y]+2*x;
  U64* q = (U64*)buf;
  for (int X=(w+1)/2; X--;) {
    U64* colors = d.byte1[*p++];
    int index = *p++;
    U64* colors1 = d.byte1[*p++];
    int index1 = *p++;
#if WORDS_BIGENDIAN
    *q++ = (colors[index]<<32) | colors1[index1];
#else
    *q++ = (colors1[index1]<<32) | colors[index];
#endif
  }
}

#else

// The callback from fl_draw_image to get a row of data passes this:
struct pixmap_data {
  int w, h;
  const uchar*const* data;
  union {
    U32 colors[256];
    U32* byte1[256];
  };
};

// callback for 1 byte per pixel:
static void cb1(void*v, int x, int y, int w, uchar* buf) {
  pixmap_data& d = *(pixmap_data*)v;
  const uchar* p = d.data[y]+x;
  U32* q = (U32*)buf;
  for (int X=w; X--;) *q++ = d.colors[*p++];
}

// callback for 2 bytes per pixel:
static void cb2(void*v, int x, int y, int w, uchar* buf) {
  pixmap_data& d = *(pixmap_data*)v;
  const uchar* p = d.data[y]+2*x;
  U32* q = (U32*)buf;
  for (int X=w; X--;) {
    U32* colors = d.byte1[*p++];
    *q++ = colors[*p++];
  }
}

#endif

#ifdef WIN32
// this is in Fl_arg.C:
extern int fl_parse_color(const char*, uchar&, uchar&, uchar&);
#endif

uchar **fl_mask_bitmap; // if non-zero, create bitmap and store pointer here

int fl_draw_pixmap(/*const*/ char* const* data, int x,int y,Fl_Color bg) {
  return fl_draw_pixmap((const char*const*)data,x,y,bg);
}

int fl_draw_pixmap(const char*const* di, int x, int y, Fl_Color bg) {
  pixmap_data d;
  if (!fl_measure_pixmap(di, d.w, d.h)) return 0;
  const uchar*const* data = (const uchar*const*)(di+1);
  int transparent_index = -1;

  if (ncolors < 0) {	// fltk (non standard) compressed colormap
    ncolors = -ncolors;
    const uchar *p = *data++;
    // if first color is ' ' it is transparent (put it later to make
    // it not be transparent):
    if (*p == ' ') {
      uchar* c = (uchar*)&d.colors[' '];
#ifdef U64
      *(U64*)c = 0;
#if WORDS_BIGENDIAN
      c += 4;
#endif
#endif
      transparent_index = ' ';
      Fl::get_color(bg, c[0], c[1], c[2]); c[3] = 0;
      p += 4;
      ncolors--;
    }
    // read all the rest of the colors:
    for (int i=0; i < ncolors; i++) {
      uchar* c = (uchar*)&d.colors[*p++];
#ifdef U64
      *(U64*)c = 0;
#if WORDS_BIGENDIAN
      c += 4;
#endif
#endif
      *c++ = *p++;
      *c++ = *p++;
      *c++ = *p++;
      *c = 0;
    }
  } else {	// normal XPM colormap with names
    if (chars_per_pixel>1) memset(d.byte1, 0, sizeof(d.byte1));
    for (int i=0; i<ncolors; i++) {
      const uchar *p = *data++;
      // the first 1 or 2 characters are the color index:
      int index = *p++;
      uchar* c;
      if (chars_per_pixel>1) {
#ifdef U64
	U64* colors = d.byte1[index];
	if (!colors) colors = d.byte1[index] = new U64[256];
#else
	U32* colors = d.byte1[index];
	if (!colors) colors = d.byte1[index] = new U32[256];
#endif
	c = (uchar*)&colors[*p];
	index = (index<<8)+*p++;
      } else {
	c = (uchar *)&d.colors[index];
      }
      // look for "c word", or last word if none:
      const uchar *previous_word = p;
      for (;;) {
	while (*p && isspace(*p)) p++; uchar what = *p++;
	while (*p && !isspace(*p)) p++;
	while (*p && isspace(*p)) p++;
	if (!*p) {p = previous_word; break;}
	if (what == 'c') break;
	previous_word = p;
	while (*p && !isspace(*p)) p++;
      }
#ifdef U64
      *(U64*)c = 0;
#if WORDS_BIGENDIAN
      c += 4;
#endif
#endif
#ifdef WIN32
      if (fl_parse_color((const char*)p, c[0], c[1], c[2])) {;
#else
      XColor x;
      if (XParseColor(fl_display, fl_colormap, (const char*)p, &x)) {
	c[0] = x.red>>8; c[1] = x.green>>8; c[2] = x.blue>>8;
#endif
      } else { // assumme "None" or "#transparent" for any errors
	// this should be transparent...
	Fl::get_color(bg, c[0], c[1], c[2]);
	transparent_index = index;
      }
    }
  }
  d.data = data;

  // build the mask bitmap used by Fl_Pixmap:
  if (fl_mask_bitmap && transparent_index >= 0) {
    int W = (d.w+7)/8;
    uchar* bitmap = new uchar[W * d.h];
    *fl_mask_bitmap = bitmap;
    for (int y = 0; y < d.h; y++) {
      const uchar* p = data[y];
      if (chars_per_pixel <= 1) {
	for (int x = 0; x < W; x++) {
	  int b = (*p++ != transparent_index);
	  if (*p++ != transparent_index) b |= 2;
	  if (*p++ != transparent_index) b |= 4;
	  if (*p++ != transparent_index) b |= 8;
	  if (*p++ != transparent_index) b |= 16;
	  if (*p++ != transparent_index) b |= 32;
	  if (*p++ != transparent_index) b |= 64;
	  if (*p++ != transparent_index) b |= 128;
	  *bitmap++ = b;
	}
      } else {
	for (int x = 0; x < W; x++) {
	  int b = 0;
	  for (int i = 0; i < 8; i++) {
	    int index = *p++;
	    index = (index<<8) | (*p++);
	    if (index != transparent_index) b |= (1<<i);
	  }
	  *bitmap++ = b;
	}
      }
    }
  }

  fl_draw_image(chars_per_pixel==1 ? cb1 : cb2, &d, x, y, d.w, d.h, 4);
  if (chars_per_pixel > 1) for (int i = 0; i < 256; i++) delete d.byte1[i];
  return 1;
}

//
// End of "$Id: fl_draw_pixmap.cxx,v 1.4.2.8 2001/04/13 19:07:40 easysw Exp $".
//
