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


/**
 \file gdi_rect.cxx
 \brief MSWindows GDI specific line and polygon drawing with integer coordinates.
 */

#include <config.h>
#include "../../config_lib.h"
#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Printer.H>
#include <FL/fl_draw.H>
#include <FL/x.H>

#include "Fl_GDI_Graphics_Driver.h"


// --- line and polygon drawing with integer coordinates

void Fl_GDI_Graphics_Driver::point(int x, int y) {
  SetPixel(gc, x, y, fl_RGB());
}

void Fl_GDI_Graphics_Driver::rect(int x, int y, int w, int h) {
  if (w<=0 || h<=0) return;
  MoveToEx(gc, x, y, 0L);
  LineTo(gc, x+w-1, y);
  LineTo(gc, x+w-1, y+h-1);
  LineTo(gc, x, y+h-1);
  LineTo(gc, x, y);
}

void Fl_GDI_Graphics_Driver::focus_rect(int x, int y, int w, int h) {
  // Windows 95/98/ME do not implement the dotted line style, so draw
  // every other pixel around the focus area...
  w--; h--;
  int i=1, xx, yy;
  for (xx = 0; xx < w; xx++, i++) if (i & 1) point(x + xx, y);
  for (yy = 0; yy < h; yy++, i++) if (i & 1) point(x + w, y + yy);
  for (xx = w; xx > 0; xx--, i++) if (i & 1) point(x + xx, y + h);
  for (yy = h; yy > 0; yy--, i++) if (i & 1) point(x, y + yy);
}

void Fl_GDI_Graphics_Driver::rectf(int x, int y, int w, int h) {
  if (w<=0 || h<=0) return;
  RECT rect;
  rect.left = x; rect.top = y;
  rect.right = x + w; rect.bottom = y + h;
  FillRect(gc, &rect, fl_brush());
}

void Fl_GDI_Graphics_Driver::line(int x, int y, int x1, int y1) {
  MoveToEx(gc, x, y, 0L);
  LineTo(gc, x1, y1);
  SetPixel(gc, x1, y1, fl_RGB());
}

void Fl_GDI_Graphics_Driver::line(int x, int y, int x1, int y1, int x2, int y2) {
  MoveToEx(gc, x, y, 0L);
  LineTo(gc, x1, y1);
  LineTo(gc, x2, y2);
  SetPixel(gc, x2, y2, fl_RGB());
}

void Fl_GDI_Graphics_Driver::xyline(int x, int y, int x1) {
  MoveToEx(gc, x, y, 0L); LineTo(gc, x1+1, y);
}

void Fl_GDI_Graphics_Driver::xyline(int x, int y, int x1, int y2) {
  if (y2 < y) y2--;
  else y2++;
  MoveToEx(gc, x, y, 0L);
  LineTo(gc, x1, y);
  LineTo(gc, x1, y2);
}

void Fl_GDI_Graphics_Driver::xyline(int x, int y, int x1, int y2, int x3) {
  if(x3 < x1) x3--;
  else x3++;
  MoveToEx(gc, x, y, 0L);
  LineTo(gc, x1, y);
  LineTo(gc, x1, y2);
  LineTo(gc, x3, y2);
}

void Fl_GDI_Graphics_Driver::yxline(int x, int y, int y1) {
  if (y1 < y) y1--;
  else y1++;
  MoveToEx(gc, x, y, 0L); LineTo(gc, x, y1);
}

void Fl_GDI_Graphics_Driver::yxline(int x, int y, int y1, int x2) {
  if (x2 > x) x2++;
  else x2--;
  MoveToEx(gc, x, y, 0L);
  LineTo(gc, x, y1);
  LineTo(gc, x2, y1);
}

void Fl_GDI_Graphics_Driver::yxline(int x, int y, int y1, int x2, int y3) {
  if(y3<y1) y3--;
  else y3++;
  MoveToEx(gc, x, y, 0L);
  LineTo(gc, x, y1);
  LineTo(gc, x2, y1);
  LineTo(gc, x2, y3);
}

void Fl_GDI_Graphics_Driver::loop(int x, int y, int x1, int y1, int x2, int y2) {
  MoveToEx(gc, x, y, 0L);
  LineTo(gc, x1, y1);
  LineTo(gc, x2, y2);
  LineTo(gc, x, y);
}

void Fl_GDI_Graphics_Driver::loop(int x, int y, int x1, int y1, int x2, int y2, int x3, int y3) {
  MoveToEx(gc, x, y, 0L);
  LineTo(gc, x1, y1);
  LineTo(gc, x2, y2);
  LineTo(gc, x3, y3);
  LineTo(gc, x, y);
}

void Fl_GDI_Graphics_Driver::polygon(int x, int y, int x1, int y1, int x2, int y2) {
  XPoint p[3];
  p[0].x = x;  p[0].y = y;
  p[1].x = x1; p[1].y = y1;
  p[2].x = x2; p[2].y = y2;
  SelectObject(gc, fl_brush());
  Polygon(gc, p, 3);
}

void Fl_GDI_Graphics_Driver::polygon(int x, int y, int x1, int y1, int x2, int y2, int x3, int y3) {
  XPoint p[4];
  p[0].x = x;  p[0].y = y;
  p[1].x = x1; p[1].y = y1;
  p[2].x = x2; p[2].y = y2;
  p[3].x = x3; p[3].y = y3;
  SelectObject(gc, fl_brush());
  Polygon(gc, p, 4);
}

// --- clipping

void Fl_GDI_Graphics_Driver::push_clip(int x, int y, int w, int h) {
  Fl_Region r;
  if (w > 0 && h > 0) {
    r = XRectangleRegion(x,y,w,h);
    Fl_Region current = rstack[rstackptr];
    if (current) {
      CombineRgn(r,r,current,RGN_AND);
    }
  } else { // make empty clip region:
    r = CreateRectRgn(0,0,0,0);
  }
  if (rstackptr < region_stack_max) rstack[++rstackptr] = r;
  else Fl::warning("Fl_GDI_Graphics_Driver::push_clip: clip stack overflow!\n");
  fl_restore_clip();
}

int Fl_GDI_Graphics_Driver::clip_box(int x, int y, int w, int h, int& X, int& Y, int& W, int& H){
  X = x; Y = y; W = w; H = h;
  Fl_Region r = rstack[rstackptr];
  if (!r) return 0;
  // The win32 API makes no distinction between partial and complete
  // intersection, so we have to check for partial intersection ourselves.
  // However, given that the regions may be composite, we have to do
  // some voodoo stuff...
  Fl_Region rr = XRectangleRegion(x,y,w,h);
  Fl_Region temp = CreateRectRgn(0,0,0,0);
  int ret;
  if (CombineRgn(temp, rr, r, RGN_AND) == NULLREGION) { // disjoint
    W = H = 0;
    ret = 2;
  } else if (EqualRgn(temp, rr)) { // complete
    ret = 0;
  } else {	// partial intersection
    RECT rect;
    GetRgnBox(temp, &rect);
    if (Fl_Surface_Device::surface() != Fl_Display_Device::display_device()) { // if print context, convert coords from device to logical
      POINT pt[2] = { {rect.left, rect.top}, {rect.right, rect.bottom} };
      DPtoLP(gc, pt, 2);
      X = pt[0].x; Y = pt[0].y; W = pt[1].x - X; H = pt[1].y - Y;
    }
    else {
      X = rect.left; Y = rect.top; W = rect.right - X; H = rect.bottom - Y;
    }
    ret = 1;
  }
  DeleteObject(temp);
  DeleteObject(rr);
  return ret;
}

int Fl_GDI_Graphics_Driver::not_clipped(int x, int y, int w, int h) {
  if (x+w <= 0 || y+h <= 0) return 0;
  Fl_Region r = rstack[rstackptr];
  if (!r) return 1;
  RECT rect;
  if (Fl_Surface_Device::surface() != Fl_Display_Device::display_device()) { // in case of print context, convert coords from logical to device
    POINT pt[2] = { {x, y}, {x + w, y + h} };
    LPtoDP(gc, pt, 2);
    rect.left = pt[0].x; rect.top = pt[0].y; rect.right = pt[1].x; rect.bottom = pt[1].y;
  } else {
    rect.left = x; rect.top = y; rect.right = x+w; rect.bottom = y+h;
  }
  return RectInRegion(r,&rect);
}

// make there be no clip (used by fl_begin_offscreen() only!)
void Fl_GDI_Graphics_Driver::push_no_clip() {
  if (rstackptr < region_stack_max) rstack[++rstackptr] = 0;
  else Fl::warning("Fl_GDI_Graphics_Driver::push_no_clip: clip stack overflow!\n");
  fl_restore_clip();
}

// pop back to previous clip:
void Fl_GDI_Graphics_Driver::pop_clip() {
  if (rstackptr > 0) {
    Fl_Region oldr = rstack[rstackptr--];
    if (oldr) XDestroyRegion(oldr);
  } else Fl::warning("Fl_GDI_Graphics_Driver::pop_clip: clip stack underflow!\n");
  fl_restore_clip();
}

void Fl_GDI_Graphics_Driver::restore_clip() {
  fl_clip_state_number++;
  Fl_Region r = rstack[rstackptr];
  SelectClipRgn(gc, r); //if r is NULL, clip is automatically cleared
}


//
// End of "$Id$".
//
