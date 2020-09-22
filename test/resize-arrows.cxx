//
// Resize example for use in the Fast Light Tool Kit (FLTK) documentation.
//
//     See Article #415: How does resizing work?
//     https://www.fltk.org/articles.php?L415
//
// Copyright 1998-2020 by Bill Spitzak and others.
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

#include "resize-arrows.h"
#include <FL/fl_draw.H>

Harrow::Harrow(int X, int Y, int W, int H, const char *T)
  : Fl_Box(X, Y, W, H, T) {
  align(FL_ALIGN_BOTTOM);
  box(FL_NO_BOX);
  color(FL_WHITE);
}

void Harrow::draw() {
  Fl_Color old = fl_color();
  int dx = 5, dy = 4;
  int my = y() + h() / 2;
  fl_rectf(x(), y(), w(), h(), FL_WHITE);
  fl_color(FL_BLACK);
  fl_polygon(x(), my, x() + dx, my - dy, x() + dx, my + dy);
  fl_line(x() + dx, my, x() + w() - dx, my);
  fl_polygon(x() + w(), my, x() + w() - dx, my + dy, x() + w() - dx, my - dy);
  fl_color(old);
}

Varrow::Varrow(int X, int Y, int W, int H, const char *T)
  : Fl_Box(X, Y, W, H, T) {
  align(FL_ALIGN_RIGHT);
  box(FL_NO_BOX);
  color(FL_WHITE);
}

void Varrow::draw() {
  Fl_Color old = fl_color();
  int dx = 4, dy = 5;
  int mx = x() + w() / 2;
  fl_rectf(x(), y(), w(), h(), FL_WHITE);
  fl_color(FL_BLACK);
  fl_polygon(mx - dx, y() + dy, mx, y(), mx + dx, y() + dy);
  fl_line(mx, y() + dy, mx, y() + h() - dy);
  fl_polygon(mx - dx, y() + h() - dy, mx + dx, y() + h() - dy, mx, y() + h());
  fl_color(old);
}
