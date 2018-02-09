//
// "$Id$"
//
// Rectangle drawing routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2018 by Bill Spitzak and others.
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
 \file Fl_GDI_Graphics_Driver_rect.cxx
 \brief Windows GDI specific line and polygon drawing with integer coordinates.
 */

#include <config.h>
#include "../../config_lib.h"
#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Printer.H>
#include <FL/fl_draw.H>
#include <FL/platform.H>

#include "Fl_GDI_Graphics_Driver.H"


// --- line and polygon drawing with integer coordinates

void Fl_GDI_Graphics_Driver::point_unscaled(float fx, float fy) {
  int width = scale_ >= 1 ? scale_ : 1;
  RECT rect;
  rect.left = fx; rect.top = fy;
  rect.right = fx + width; rect.bottom = fy + width;
  FillRect(gc_, &rect, fl_brush());
}

void Fl_GDI_Graphics_Driver::overlay_rect(int x, int y, int w , int h) {
  // make pen have a one-pixel width
  line_style_unscaled( (color()==FL_WHITE?FL_SOLID:FL_DOT), 1, NULL);
  loop(x, y, x+w-1, y, x+w-1, y+h-1, x, y+h-1);
}

void Fl_GDI_Graphics_Driver::rect_unscaled(float x, float y, float w, float h) {
  if (w<=0 || h<=0) return;
  int line_delta_ =  (scale_ > 1.75 ? 1 : 0);//TMP
  x += line_delta_; y += line_delta_;
  int tw = line_width_ ? line_width_ : 1; // true line width
  MoveToEx(gc_, x, y, 0L);
  LineTo(gc_, x+w-tw, y);
  LineTo(gc_, x+w-tw, y+h-tw);
  LineTo(gc_, x, y+h-tw);
  LineTo(gc_, x, y);
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

void Fl_GDI_Graphics_Driver::rectf_unscaled(float x, float y, float w, float h) {
  if (w<=0 || h<=0) return;
  RECT rect;
  rect.left = x; rect.top = y;
  rect.right = x + w; rect.bottom = y + h;
  FillRect(gc_, &rect, fl_brush());
}

void Fl_GDI_Graphics_Driver::line_unscaled(float x, float y, float x1, float y1) {
  MoveToEx(gc_, x, y, 0L);
  LineTo(gc_, x1, y1);
  SetPixel(gc_, x1, y1, fl_RGB());
}

void Fl_GDI_Graphics_Driver::line_unscaled(float x, float y, float x1, float y1, float x2, float y2) {
  MoveToEx(gc_, x, y, 0L);
  LineTo(gc_, x1, y1);
  LineTo(gc_, x2, y2);
  SetPixel(gc_, x2, y2, fl_RGB());
}

//extern FILE*LOG;
void Fl_GDI_Graphics_Driver::xyline_unscaled(float x, float y, float x1) {
  int line_delta_ =  (scale_ > 1.75 ? 1 : 0);
  int tw = line_width_ ? line_width_ : 1; // true line width
  if (x > x1) { float exch = x; x = x1; x1 = exch; }
  int ix = x+line_delta_; if (scale_ >= 2) ix -= int(scale_/2);
  int iy = y+line_delta_;
  int ix1 = int(x1/scale_+1.5)*scale_-1; // extend line to pixel before line beginning at x1/scale_ + 1
  ix1 += line_delta_; if (scale_ >= 2) ix1 -= 1;; if (scale_ >= 4) ix1 -= 1;
  MoveToEx(gc_, ix, iy, 0L); LineTo(gc_, ix1+1, iy);
  // try and make sure no unfilled area lies between xyline(x,y,x1) and xyline(x,y+1,x1)
  if (int(scale_) != scale_ && y+line_delta_ + scale_ >= iy + tw+1 - 0.001 ) {
    MoveToEx(gc_, ix, iy+1, 0L); LineTo(gc_, ix1+1, iy+1);
  }
//fprintf(LOG,"xyline_unscaled tw=%d s=%f gc_=%p\n",tw,scale_,gc_);fflush(LOG);
}

void Fl_GDI_Graphics_Driver::yxline_unscaled(float x, float y, float y1) {
  if (y1 < y) { float exch = y; y = y1; y1 = exch;}
  int line_delta_ =  (scale_ > 1.75 ? 1 : 0);
  int tw = line_width_ ? line_width_ : 1; // true line width

  int ix = x+line_delta_;
  int iy = y+line_delta_; if (scale_ >= 2) iy -= int(scale_/2);
  int iy1 = int(y1/scale_+1.5)*scale_-1;
  iy1 += line_delta_; if (scale_ >= 2) iy1 -= 1;; if (scale_ >= 4) iy1 -= 1; // extend line to pixel before line beginning at y1/scale_ + 1
  MoveToEx(gc_, ix, iy, 0L); LineTo(gc_, ix, iy1+1);
  // try and make sure no unfilled area lies between yxline(x,y,y1) and yxline(x+1,y,y1)
  if (int(scale_) != scale_ && x+line_delta_+scale_ >= ix + tw+1 -0.001) {
    MoveToEx(gc_, ix+1, iy, 0L); LineTo(gc_, ix+1, iy1+1);
  }

}

void Fl_GDI_Graphics_Driver::loop_unscaled(float x, float y, float x1, float y1, float x2, float y2) {
  MoveToEx(gc_, x, y, 0L);
  LineTo(gc_, x1, y1);
  LineTo(gc_, x2, y2);
  LineTo(gc_, x, y);
}

void Fl_GDI_Graphics_Driver::loop_unscaled(float x, float y, float x1, float y1, float x2, float y2, float x3, float y3) {
  MoveToEx(gc_, x, y, 0L);
  LineTo(gc_, x1, y1);
  LineTo(gc_, x2, y2);
  LineTo(gc_, x3, y3);
  LineTo(gc_, x, y);
}

void Fl_GDI_Graphics_Driver::polygon_unscaled(float x, float y, float x1, float y1, float x2, float y2) {
  POINT p[3];
  p[0].x = x;  p[0].y = y;
  p[1].x = x1; p[1].y = y1;
  p[2].x = x2; p[2].y = y2;
  SelectObject(gc_, fl_brush());
  Polygon(gc_, p, 3);
}

void Fl_GDI_Graphics_Driver::polygon_unscaled(float x, float y, float x1, float y1, float x2, float y2, float x3, float y3) {
  POINT p[4];
  p[0].x = x;  p[0].y = y;
  p[1].x = x1; p[1].y = y1;
  p[2].x = x2; p[2].y = y2;
  p[3].x = x3; p[3].y = y3;
  SelectObject(gc_, fl_brush());
  Polygon(gc_, p, 4);
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
      DPtoLP(gc_, pt, 2);
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
    LPtoDP(gc_, pt, 2);
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
  if (gc_) {
    HRGN r = NULL;
    if (rstack[rstackptr]) r = scale_clip(scale_);
    SelectClipRgn(gc_, rstack[rstackptr]); // if region is NULL, clip is automatically cleared
    if (r) unscale_clip(r);
  }
}

//
// End of "$Id$".
//
