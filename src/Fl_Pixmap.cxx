//
// "$Id"
//
// Pixmap drawing code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998 by Bill Spitzak and others.
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
// Please report all bugs and problems to "fltk-bugs@easysw.com".
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

extern uchar **fl_mask_bitmap; // used by fl_draw_pixmap.C to store mask
void fl_restore_clip(); // in fl_rect.C

void Fl_Pixmap::draw(int XP, int YP, int WP, int HP, int cx, int cy) {
  // ignore empty or bad pixmap data:
  if (w<0) fl_measure_pixmap(data, w, h);
  if (!w) return;
  // account for current clip region (faster on Irix):
  int X,Y,W,H; fl_clip_box(XP,YP,WP,HP,X,Y,W,H);
  cx += X-XP; cy += Y-YP;
  // clip the box down to the size of image, quit if empty:
  if (cx < 0) {W += cx; X -= cx; cx = 0;}
  if (cx+W > w) W = w-cx;
  if (W <= 0) return;
  if (cy < 0) {H += cy; Y -= cy; cy = 0;}
  if (cy+H > h) H = h-cy;
  if (H <= 0) return;
  if (!id) {
    id = (ulong)fl_create_offscreen(w, h);
    fl_begin_offscreen((Fl_Offscreen)id);
#ifdef WIN32 // mask is nyi, instead use a constant color
    fl_draw_pixmap(data, 0, 0, (Fl_Color)mask);
#else
    uchar *bitmap = 0;
    fl_mask_bitmap = &bitmap;
    fl_draw_pixmap(data, 0, 0, FL_BLACK);
    fl_mask_bitmap = 0;
    if (bitmap) {
      mask = XCreateBitmapFromData(fl_display, fl_window,
				   (const char*)bitmap, (w+7)&-8, h);
      delete[] bitmap;
    }
#endif
    fl_end_offscreen();
  }
#ifndef WIN32
  if (mask) {
    // I can't figure out how to combine a mask with existing region,
    // so cut the image down to a clipped rectangle:
    int nx, ny; fl_clip_box(X,Y,W,H,nx,ny,W,H);
    cx += nx-X; X = nx;
    cy += ny-Y; Y = ny;
    // make X use the bitmap as a mask:
    XSetClipMask(fl_display, fl_gc, mask);
    int ox = X-cx; if (ox < 0) ox += w;
    int oy = Y-cy; if (oy < 0) oy += h;
    XSetClipOrigin(fl_display, fl_gc, X-cx, Y-cy);
  }
#endif
  fl_copy_offscreen(X, Y, W, H, (Fl_Offscreen)id, cx, cy);
#ifndef WIN32
  if (mask) {
    // put the old clip region back
    XSetClipOrigin(fl_display, fl_gc, 0, 0);
    fl_restore_clip();
  }
#endif
}

Fl_Pixmap::~Fl_Pixmap() {
  if (id) fl_delete_offscreen((Fl_Offscreen)id);
#ifndef WIN32
  if (mask) fl_delete_offscreen((Fl_Offscreen)mask);
#endif
}

static void pixmap_labeltype(
    const Fl_Label* o, int x, int y, int w, int h, Fl_Align a)
{
  Fl_Pixmap* b = (Fl_Pixmap*)(o->value);
  if (b->w<0) fl_measure_pixmap(b->data, b->w, b->h);
  int cx;
  if (a & FL_ALIGN_LEFT) cx = 0;
  else if (a & FL_ALIGN_RIGHT) cx = b->w-w;
  else cx = (b->w-w)/2;
  int cy;
  if (a & FL_ALIGN_TOP) cy = 0;
  else if (a & FL_ALIGN_BOTTOM) cy = b->h-h;
  else cy = (b->h-h)/2;
  b->draw(x,y,w,h,cx,cy);
}

static void pixmap_measure(const Fl_Label* o, int& w, int& h) {
  Fl_Pixmap* b = (Fl_Pixmap*)(o->value);
  if (b->w<0) fl_measure_pixmap(b->data, b->w, b->h);
  w = b->w;
  h = b->h;
}

void Fl_Pixmap::label(Fl_Widget* o) {
#ifdef WIN32
  mask = o->color();
#endif
  Fl::set_labeltype(_FL_PIXMAP_LABEL, pixmap_labeltype, pixmap_measure);
  o->label(_FL_PIXMAP_LABEL, (const char*)this);
}

void Fl_Pixmap::label(Fl_Menu_Item* o) {
  Fl::set_labeltype(_FL_PIXMAP_LABEL, pixmap_labeltype, pixmap_measure);
  o->label(_FL_PIXMAP_LABEL, (const char*)this);
}

//
// End of "$Id: Fl_Pixmap.cxx,v 1.3 1998/10/19 20:45:58 mike Exp $".
//
