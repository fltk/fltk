//
// Rectangle drawing routines for the Fast Light Tool Kit (FLTK).
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


/**
 \file Fl_GDI_Graphics_Driver_rect.cxx
 \brief Windows GDI specific line and polygon drawing with integer coordinates.
 */

#include <config.h>
#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>
#include <FL/platform.H>

#include "Fl_GDI_Graphics_Driver.H"


// --- line and polygon drawing with integer coordinates

void Fl_GDI_Graphics_Driver::point(int x, int y) {
  rectf(x, y, 1, 1);
}

void Fl_GDI_Graphics_Driver::overlay_rect(int x, int y, int w , int h) {
  // make pen have a one-pixel width
  line_style_unscaled( (color()==FL_WHITE?FL_SOLID:FL_DOT), 1, NULL);
  int right = this->floor(x+w-1), bottom = this->floor(y+h-1);
  x = this->floor(x); y = this->floor(y);
  MoveToEx(gc_, x, y, 0L);
  LineTo(gc_, right, y);
  LineTo(gc_, right, bottom);
  LineTo(gc_, x, bottom);
  LineTo(gc_, x, y);
}

void Fl_GDI_Graphics_Driver::focus_rect(int x, int y, int w, int h) {
  // Windows 95/98/ME do not implement the dotted line style, so draw
  // every other pixel around the focus area...
  w = floor(x+w-1) - floor(x) + 1;
  h = floor(y+h-1) - floor(y) + 1;
  x = floor(x); y = floor(y);
  int i=1, xx, yy;
  COLORREF c = fl_RGB();
  for (xx = 0; xx < w; xx++, i++) if (i & 1) SetPixel(gc_, x+xx, y, c);
  for (yy = 0; yy < h; yy++, i++) if (i & 1) SetPixel(gc_, x+w, y+yy, c);
  for (xx = w; xx > 0; xx--, i++) if (i & 1) SetPixel(gc_, x+xx, y+h, c);
  for (yy = h; yy > 0; yy--, i++) if (i & 1) SetPixel(gc_, x, y+yy, c);
}

void Fl_GDI_Graphics_Driver::rect_unscaled(int x, int y, int w, int h) {
  if (is_solid_ && line_width_ > 1) {
    line_style_unscaled(FL_CAP_SQUARE, line_width_, 0); // see issue #1052
  }
  MoveToEx(gc_, x, y, 0L);
  LineTo(gc_, x+w, y);
  if (is_solid_ && line_width_ <= 1) LineTo(gc_, x+w, y+h+1); // see issue #1052
  LineTo(gc_, x+w, y+h);
  LineTo(gc_, x, y+h);
  LineTo(gc_, x, y);
  if (is_solid_ && line_width_ > 1) {
    line_style_unscaled(style_, line_width_, 0);
  }
}

void Fl_GDI_Graphics_Driver::rectf_unscaled(int x, int y, int w, int h) {
  RECT rect;
  rect.left = x; rect.top = y;
  rect.right = (x + w); rect.bottom = (y + h);
  FillRect(gc_, &rect, fl_brush());
}

void Fl_GDI_Graphics_Driver::line_unscaled(int x, int y, int x1, int y1) {
  MoveToEx(gc_, x, y, 0L);
  LineTo(gc_, x1, y1);
  SetPixel(gc_, x1, y1, fl_RGB());
}

void Fl_GDI_Graphics_Driver::line_unscaled(int x, int y, int x1, int y1, int x2, int y2) {
  MoveToEx(gc_, x, y, 0L);
  LineTo(gc_, x1, y1);
  LineTo(gc_, x2, y2);
  SetPixel(gc_, x2, y2, fl_RGB());
}

void* Fl_GDI_Graphics_Driver::change_pen_width(int width) { // set the width of the pen, return previous pen
  LOGBRUSH penbrush = {BS_SOLID, fl_RGB(), 0};
  HPEN newpen = ExtCreatePen(PS_GEOMETRIC | PS_ENDCAP_FLAT | PS_JOIN_ROUND, width, &penbrush, 0, 0);
  return SelectObject(gc_, newpen);
}

void Fl_GDI_Graphics_Driver::reset_pen_width(void *data) {
  DeleteObject(SelectObject(gc_, (HPEN)data));
}

void Fl_GDI_Graphics_Driver::xyline_unscaled(int x, int y, int x1) {
  MoveToEx(gc_, x, y, 0L);
  LineTo(gc_, x1+1 , y);
}

void Fl_GDI_Graphics_Driver::yxline_unscaled(int x, int y, int y1) {
  MoveToEx(gc_, x, y, 0L);
  LineTo(gc_, x, y1+1);
}

void Fl_GDI_Graphics_Driver::loop_unscaled(int x, int y, int x1, int y1, int x2, int y2) {
  MoveToEx(gc_, x, y, 0L);
  LineTo(gc_, x1, y1);
  LineTo(gc_, x2, y2);
  LineTo(gc_, x, y);
}

void Fl_GDI_Graphics_Driver::loop_unscaled(int x, int y, int x1, int y1, int x2, int y2, int x3, int y3) {
  MoveToEx(gc_, x, y, 0L);
  LineTo(gc_, x1, y1);
  LineTo(gc_, x2, y2);
  LineTo(gc_, x3, y3);
  LineTo(gc_, x, y);
}

void Fl_GDI_Graphics_Driver::polygon_unscaled(int x, int y, int x1, int y1, int x2, int y2) {
  POINT p[3];
  p[0].x = x;  p[0].y = y;
  p[1].x = x1; p[1].y = y1;
  p[2].x = x2; p[2].y = y2;
  SelectObject(gc_, fl_brush());
  Polygon(gc_, p, 3);
}

void Fl_GDI_Graphics_Driver::polygon_unscaled(int x, int y, int x1, int y1, int x2, int y2, int x3, int y3) {
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
  HRGN r;
  if (w > 0 && h > 0) {
    r = (HRGN)XRectangleRegion(x,y,w,h);
    HRGN current = (HRGN)rstack[rstackptr];
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
  HRGN r = (HRGN)rstack[rstackptr];
  if (!r) return 0;
  // The win32 API makes no distinction between partial and complete
  // intersection, so we have to check for partial intersection ourselves.
  // However, given that the regions may be composite, we have to do
  // some voodoo stuff...
  HRGN rr = (HRGN)XRectangleRegion(x,y,w,h);
  HRGN temp = CreateRectRgn(0,0,0,0);
  int ret;
  if (CombineRgn(temp, rr, r, RGN_AND) == NULLREGION) { // disjoint
    W = H = 0;
    ret = 2;
  } else if (EqualRgn(temp, rr)) { // complete
    ret = 0;
  } else {      // partial intersection
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
  HRGN r = (HRGN)rstack[rstackptr];
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

void Fl_GDI_Graphics_Driver::restore_clip() {
  fl_clip_state_number++;
  if (gc_) {
    HRGN r = NULL;
    if (rstack[rstackptr]) r = (HRGN)scale_clip(scale());
    SelectClipRgn(gc_, (HRGN)rstack[rstackptr]); // if region is NULL, clip is automatically cleared
    if (r) unscale_clip(r);
  }
}

#if USE_GDIPLUS

void Fl_GDIplus_Graphics_Driver::line(int x, int y, int x1, int y1) {
  if (!active) return Fl_Scalable_Graphics_Driver::line(x, y, x1, y1);
  bool AA = !(x == x1 || y == y1);
  Gdiplus::Graphics graphics_(gc_);
  graphics_.ScaleTransform(scale(), scale());
  pen_->SetColor(gdiplus_color_);
  if (AA) graphics_.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
  graphics_.DrawLine(pen_, x, y, x1, y1);
}

void Fl_GDIplus_Graphics_Driver::line(int x, int y, int x1, int y1, int x2, int y2) {
  if (!active) return Fl_Scalable_Graphics_Driver::line(x, y, x1, y1, x2, y2);
  line(x, y, x1, y1);
  line(x1, y1, x2, y2);
}

void Fl_GDIplus_Graphics_Driver::loop(int x0, int y0, int x1, int y1, int x2, int y2) {
  if (!active) return Fl_Scalable_Graphics_Driver::loop(x0, y0, x1, y1, x2, y2);
  Gdiplus::GraphicsPath path;
  Gdiplus::Point gdi2_p[3] = {Gdiplus::Point(x0, y0), Gdiplus::Point(x1, y1), Gdiplus::Point(x2, y2)};
  path.AddLines(gdi2_p, 3);
  path.CloseFigure();
  Gdiplus::Graphics graphics_(gc_);
  graphics_.ScaleTransform(scale(), scale());
  pen_->SetColor(gdiplus_color_);
  graphics_.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
  graphics_.DrawPath(pen_, &path);
}

#define fl_min(a,b) (a < b ? a : b)
#define fl_max(a,b) (a > b ? a : b)
void Fl_GDIplus_Graphics_Driver::loop(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) {
  if ( (x0 == x3 && x1 == x2 && y0 == y1 && y3 == y2) ||
      (x0 == x1 && y1 == y2 && x2 == x3 && y3 == y0) ) { // rectangular loop
    int left = fl_min(x0, fl_min(x1, fl_min(x2, x3)));
    int right = fl_max(x0, fl_max(x1, fl_max(x2, x3)));
    int top = fl_min(y0, fl_min(y1, fl_min(y2, y3)));
    int bottom = fl_max(y0, fl_max(y1, fl_max(y2, y3)));
    rect(left, top, right-left+1, bottom-top+1);
  } else {
    if (!active) return Fl_Scalable_Graphics_Driver::loop(x0, y0, x1, y1, x2, y2, x3, y3);
    Gdiplus::GraphicsPath path;
    Gdiplus::PointF gdi2_p[4] = {Gdiplus::PointF(x0+1-line_width_/2.f, y0+1-line_width_/2.f), Gdiplus::PointF(x1+1-line_width_/2.f, y1+1-line_width_/2.f), Gdiplus::PointF(x2+1-line_width_/2.f, y2+1-line_width_/2.f), Gdiplus::PointF(x3+1-line_width_/2.f, y3+1-line_width_/2.f)};
    path.AddLines(gdi2_p, 4);
    path.CloseFigure();
    Gdiplus::Graphics graphics_(gc_);
    graphics_.ScaleTransform(scale(), scale());
    pen_->SetColor(gdiplus_color_);
    graphics_.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    graphics_.DrawPath(pen_, &path);
  }
}

void Fl_GDIplus_Graphics_Driver::polygon(int x0, int y0, int x1, int y1, int x2, int y2) {
  if (!active) return Fl_Scalable_Graphics_Driver::polygon(x0, y0, x1, y1, x2, y2);
  Gdiplus::GraphicsPath path;
  path.AddLine(x0, y0, x1, y1);
  path.AddLine(x1, y1, x2, y2);
  path.CloseFigure();
  Gdiplus::Graphics graphics_(gc_);
  graphics_.ScaleTransform(scale(), scale());
  brush_->SetColor(gdiplus_color_);
  graphics_.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
  graphics_.FillPath(brush_, &path);
}

void Fl_GDIplus_Graphics_Driver::polygon(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) {
  if ( (x0 == x3 && x1 == x2 && y0 == y1 && y3 == y2) ||
      (x0 == x1 && y1 == y2 && x2 == x3 && y3 == y0) ) {
    int left = fl_min(x0, fl_min(x1, fl_min(x2, x3)));
    int right = fl_max(x0, fl_max(x1, fl_max(x2, x3)));
    int top = fl_min(y0, fl_min(y1, fl_min(y2, y3)));
    int bottom = fl_max(y0, fl_max(y1, fl_max(y2, y3)));
    rectf(left, top, right-left, bottom-top);
  } else {
    if (!active) return Fl_Scalable_Graphics_Driver::polygon(x0, y0, x1, y1, x2, y2, x3, y3);
    Gdiplus::GraphicsPath path;
    path.AddLine(x0, y0, x1, y1);
    path.AddLine(x1, y1, x2, y2);
    path.AddLine(x2, y2, x3, y3);
    path.CloseFigure();
    Gdiplus::Graphics graphics_(gc_);
    graphics_.ScaleTransform(scale(), scale());
    brush_->SetColor(gdiplus_color_);
    graphics_.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    graphics_.FillPath(brush_, &path);
  }
}
#endif
