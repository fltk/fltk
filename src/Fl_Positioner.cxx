//
// "$Id: Fl_Positioner.cxx,v 1.4.2.3 2001/01/22 15:13:40 easysw Exp $"
//
// Positioner widget for the Fast Light Tool Kit (FLTK).
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

// The positioner widget from Forms, gives 2D input
// Written by: Mark Overmars

#include <FL/Fl.H>
#include <FL/Fl_Positioner.H>
#include <FL/fl_draw.H>

static double flinear(double val, double smin, double smax, double gmin, double gmax)
{
  if (smin == smax) return gmax;
  else return gmin + (gmax - gmin) * (val - smin) / (smax - smin);
}

void Fl_Positioner::draw(int x, int y, int w, int h) {
  int x1 = x + 4;
  int y1 = y + 4;
  int w1 = w - 2 * 4;
  int h1 = h - 2 * 4;
  int xx = int(flinear(xvalue(), xmin, xmax, x1, x1+w1-1)+.5);
  int yy = int(flinear(yvalue(), ymin, ymax, y1, y1+h1-1)+.5);
  draw_box(box(), x, y, w, h, color());
  fl_color(selection_color());
  fl_xyline(x1, yy, x1+w1);
  fl_yxline(xx, y1, y1+h1);
}

void Fl_Positioner::draw() {
  draw(x(), y(), w(), h());
  draw_label();
}

int Fl_Positioner::value(double X, double Y) {
  clear_changed();
  if (X == xvalue_ && Y == yvalue_) return 0;
  xvalue_ = X; yvalue_ = Y;
  redraw();
  return 1;
}

int Fl_Positioner::xvalue(double X) {
  return(value(X, yvalue_));
}

int Fl_Positioner::yvalue(double Y) {
  return(value(xvalue_, Y));
}

int Fl_Positioner::handle(int event, int x, int y, int w, int h) {
  switch (event) {
  case FL_PUSH:
  case FL_DRAG:
  case FL_RELEASE: {
    double x1 = x + 4;
    double y1 = y + 4;
    double w1 = w - 2 * 4;
    double h1 = h - 2 * 4;
    double X = flinear(Fl::event_x(), x1, x1+w1-1.0, xmin, xmax);
    if (xstep_) X = int(X/xstep_+0.5) * xstep_;
    if (X < xmin) X = xmin;
    if (X > xmax) X = xmax;
    double Y = flinear(Fl::event_y(), y1, y1+h1-1.0, ymin, ymax);
    if (ystep_) Y = int(Y/ystep_+0.5) * ystep_;
    if (Y < ymin) Y = ymin;
    if (Y > ymax) Y = ymax;
    if (value(X, Y)) set_changed();}
    if (!(when() & FL_WHEN_CHANGED ||
	  when() & FL_WHEN_RELEASE && event == FL_RELEASE)) return 1;
    if (changed() || when()&FL_WHEN_NOT_CHANGED) {
      clear_changed(); do_callback();}
    return 1;
  default:
    return 0;
  }
}

int Fl_Positioner::handle(int e) {
  return handle(e, x(), y(), w(), h());
}

Fl_Positioner::Fl_Positioner(int x, int y, int w, int h, const char* l)
: Fl_Widget(x, y, w, h, l) {
  box(FL_DOWN_BOX);
  selection_color(FL_RED);
  align(FL_ALIGN_BOTTOM);
  when(FL_WHEN_CHANGED);
  xmin = ymin = 0;
  xmax = ymax = 1;
  xvalue_ = yvalue_ = .5;
  xstep_ = ystep_ = 0;
}

void Fl_Positioner::xbounds(double a, double b) {
  if (a != xmin || b != xmax) {
    xmin = a; xmax = b;
    redraw();
  }
}

void Fl_Positioner::ybounds(double a, double b) {
  if (a != ymin || b != ymax) {
    ymin = a; ymax = b;
    redraw();
  }
}

//
// End of "$Id: Fl_Positioner.cxx,v 1.4.2.3 2001/01/22 15:13:40 easysw Exp $".
//
