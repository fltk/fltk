//
// "$Id: Fl_Bitmap.cxx,v 1.5.2.4.2.2 2001/11/18 20:52:27 easysw Exp $"
//
// Bitmap drawing routines for the Fast Light Tool Kit (FLTK).
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
#include <FL/x.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Bitmap.H>

#ifdef WIN32 // Windows bitmask functions...
Fl_Bitmask fl_create_bitmask(int w, int h, const uchar *data) {
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
#endif // WIN32

void Fl_Bitmap::draw(int XP, int YP, int WP, int HP, int cx, int cy) {
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
  if (!id) id = fl_create_bitmask(w(), h(), array);

#ifdef WIN32
  HDC tempdc = CreateCompatibleDC(fl_gc);
  SelectObject(tempdc, (HGDIOBJ)id);
  SelectObject(fl_gc, fl_brush());
  // secret bitblt code found in old MSWindows reference manual:
  BitBlt(fl_gc, X, Y, W, H, tempdc, cx, cy, 0xE20746L);
  DeleteDC(tempdc);
#else
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
}

void Fl_Bitmap::label(Fl_Widget* w) {
  w->image(this);
}

void Fl_Bitmap::label(Fl_Menu_Item* m) {
}

//
// End of "$Id: Fl_Bitmap.cxx,v 1.5.2.4.2.2 2001/11/18 20:52:27 easysw Exp $".
//
