//
// Portable drawing routines for the Fast Light Tool Kit (FLTK).
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
  \file Fl_GDI_Graphics_Driver_vertex.cxx

  \brief  Portable drawing code for drawing arbitrary shapes with
          simple 2D transformations, implemented for Windows GDI.
*/

#include "Fl_GDI_Graphics_Driver.H"

#include <FL/fl_draw.H>
#include <FL/platform.H>
#include <FL/math.h>


void Fl_GDI_Graphics_Driver::end_points() {
  for (int i=0; i<n; i++) SetPixel(gc_, long_point[i].x, long_point[i].y, fl_RGB());
}

void Fl_GDI_Graphics_Driver::end_line() {
  if (n < 2) {
    end_points();
    return;
  }
  if (n>1) Polyline(gc_, long_point, n);
}

void Fl_GDI_Graphics_Driver::end_loop() {
  fixloop();
  if (n>2) transformed_vertex0(float(long_point[0].x), float(long_point[0].y));
  end_line();
}

void Fl_GDI_Graphics_Driver::end_polygon() {
  fixloop();
  if (n < 3) {
    end_line();
    return;
  }
  if (n>2) {
    SelectObject(gc_, fl_brush());
    Polygon(gc_, long_point, n);
  }
}

void Fl_GDI_Graphics_Driver::begin_complex_polygon() {
  Fl_Graphics_Driver::begin_complex_polygon();
  numcount = 0;
}

void Fl_GDI_Graphics_Driver::gap() {
  while (n>gap_+2 && long_point[n-1].x == long_point[gap_].x && long_point[n-1].y == long_point[gap_].y) n--;
  if (n > gap_+2) {
    transformed_vertex0(float(long_point[gap_].x), float(long_point[gap_].y));
    counts[numcount++] = n-gap_;
    gap_ = n;
  } else {
    n = gap_;
  }
}

void Fl_GDI_Graphics_Driver::end_complex_polygon() {
  gap();
  if (n < 3) {
    end_line();
    return;
  }
  if (n>2) {
    SelectObject(gc_, fl_brush());
    PolyPolygon(gc_, long_point, counts, numcount);
  }
}

void Fl_GDI_Graphics_Driver::ellipse_unscaled(double xt, double yt, double rx, double ry) {
  int llx = (int)rint(xt-rx);
  int w = (int)rint(xt+rx)-llx;
  int lly = (int)rint(yt-ry);
  int h = (int)rint(yt+ry)-lly;

  if (what==POLYGON) {
    SelectObject(gc_, fl_brush());
    Pie(gc_, llx, lly, llx+w, lly+h, 0,0, 0,0);
  } else
    Arc(gc_, llx, lly, llx+w, lly+h, 0,0, 0,0);
}

#if USE_GDIPLUS

void Fl_GDIplus_Graphics_Driver::transformed_vertex(double xf, double yf) {
  if (!active) return Fl_Scalable_Graphics_Driver::transformed_vertex(xf, yf);
  transformed_vertex0(float(xf) , float(yf) );
}

void Fl_GDIplus_Graphics_Driver::vertex(double x,double y) {
  if (!active) return Fl_Scalable_Graphics_Driver::vertex(x, y);
  transformed_vertex0(float(x*m.a + y*m.c + m.x) , float(x*m.b + y*m.d + m.y) );
}

void Fl_GDIplus_Graphics_Driver::end_points() {
  if (!active) return Fl_GDI_Graphics_Driver::end_points();
  for (int i = 0; i < n; i++) point(long_point[i].x, long_point[i].y);
}

void Fl_GDIplus_Graphics_Driver::end_line() {
  if (!active) return Fl_GDI_Graphics_Driver::end_line();
  if (n < 2) {
    end_points();
    return;
  }
  if (n>1) {
    Gdiplus::GraphicsPath path;
    Gdiplus::Point *gdi2_p = new Gdiplus::Point[n];
    for (int i = 0; i < n; i++) {
      gdi2_p[i] = Gdiplus::Point(long_point[i].x, long_point[i].y);
    }
    path.AddLines(gdi2_p, n);
    delete[] gdi2_p;
    Gdiplus::Graphics graphics_(gc_);
    graphics_.ScaleTransform(scale(), scale());
    graphics_.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    pen_->SetColor(gdiplus_color_);
    graphics_.DrawPath(pen_, &path);
  }
}

void Fl_GDIplus_Graphics_Driver::end_loop() {
  if (!active) return Fl_GDI_Graphics_Driver::end_loop();
  fixloop();
  if (n >= 2) {
    Gdiplus::GraphicsPath path;
    Gdiplus::Point *gdi2_p = new Gdiplus::Point[n];
    for (int i = 0; i < n; i++) {
      gdi2_p[i] = Gdiplus::Point(long_point[i].x, long_point[i].y);
    }
    path.AddLines(gdi2_p, n);
    path.CloseFigure();
    delete[] gdi2_p;
    Gdiplus::Graphics graphics_(gc_);
    graphics_.ScaleTransform(scale(), scale());
    graphics_.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    pen_->SetColor(gdiplus_color_);
    graphics_.DrawPath(pen_, &path);
  }
}

void Fl_GDIplus_Graphics_Driver::end_polygon() {
  if (!active) return Fl_GDI_Graphics_Driver::end_polygon();
  fixloop();
  if (n < 3) {
    end_line();
    return;
  }
  if (n>2) {
    Gdiplus::GraphicsPath path;
    Gdiplus::Point *gdi2_p = new Gdiplus::Point[n];
    for (int i = 0; i < n; i++) {
      gdi2_p[i] = Gdiplus::Point(long_point[i].x, long_point[i].y);
    }
    path.AddPolygon(gdi2_p, n);
    delete[] gdi2_p;
    path.CloseFigure();
    Gdiplus::Graphics graphics_(gc_);
    graphics_.ScaleTransform(scale(), scale());
    graphics_.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    brush_->SetColor(gdiplus_color_);
    graphics_.FillPath(brush_, &path);
  }
}

void Fl_GDIplus_Graphics_Driver::end_complex_polygon() {
  if (!active) return Fl_GDI_Graphics_Driver::end_complex_polygon();
  gap();
  if (n < 3) {
    end_line();
    return;
  }
  if (n>2) {
    Gdiplus::GraphicsPath path;
    Gdiplus::Point *gdi2_p = new Gdiplus::Point[n];
    for (int i = 0; i < n; i++) {
      gdi2_p[i] = Gdiplus::Point(long_point[i].x, long_point[i].y);
    }
    path.AddPolygon(gdi2_p, n);
    delete[] gdi2_p;
    path.CloseFigure();
    Gdiplus::Graphics graphics_(gc_);
    graphics_.ScaleTransform(scale(), scale());
    graphics_.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    brush_->SetColor(gdiplus_color_);
    graphics_.FillPath(brush_, &path);
  }
}

void Fl_GDIplus_Graphics_Driver::circle(double x, double y, double r) {
  if (!active) return Fl_Scalable_Graphics_Driver::circle(x, y, r);
  double xt = transform_x(x,y);
  double yt = transform_y(x,y);
  double rx = r * (m.c ? sqrt(m.a*m.a+m.c*m.c) : fabs(m.a));
  double ry = r * (m.b ? sqrt(m.b*m.b+m.d*m.d) : fabs(m.d));
  int llx = (int)rint(xt-rx);
  int w = (int)rint(xt+rx)-llx;
  int lly = (int)rint(yt-ry);
  int h = (int)rint(yt+ry)-lly;
  Gdiplus::Graphics graphics_(gc_);
  graphics_.ScaleTransform(scale(), scale());
  graphics_.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
  if (what==POLYGON) {
    brush_->SetColor(gdiplus_color_);
    graphics_.FillPie(brush_, llx, lly, w, h, 0, 360);
  } else {
    pen_->SetColor(gdiplus_color_);
    graphics_.DrawArc(pen_, llx, lly, w, h, 0, 360);
  }
}
#endif

