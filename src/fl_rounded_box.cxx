//
// Rounded box drawing routines for the Fast Light Tool Kit (FLTK).
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

#include <FL/Fl.H>
#include <FL/fl_draw.H>

// Global parameters for rounded corner drawing algorithm:
//
//  RN = number of segments per corner (must match offset array size)
//  RS = max. corner radius
//  BW = box shadow width

#define RS (Fl::box_border_radius_max())
#define BW (Fl::box_shadow_width())

static void rbox(int fill, int x, int y, int w, int h) {
  int rs, rsy;
  rs = w*2/5; rsy = h*2/5;
  if (rs > rsy) rs = rsy; // use smaller radius
  if (rs > RS) rs = RS;
  if (fill)
    fl_rounded_rectf(x, y, w, h, rs);
  else
    fl_rounded_rect(x, y, w, h, rs);
}

static void fl_rflat_box(int x, int y, int w, int h, Fl_Color c) {
  Fl::set_box_color(c);
  rbox(1, x, y, w, h); rbox(0, x, y, w, h);
}

static void fl_rounded_frame(int x, int y, int w, int h, Fl_Color c) {
  Fl::set_box_color(c);
  rbox(0, x, y, w, h);
}

static void fl_rounded_box(int x, int y, int w, int h, Fl_Color c) {
  Fl::set_box_color(c);
  rbox(1, x, y, w, h);
  fl_color(FL_BLACK); rbox(0, x, y, w, h);
}

static void fl_rshadow_box(int x, int y, int w, int h, Fl_Color c) {
  // draw shadow:
  fl_color(FL_DARK3);
  rbox(1, x+BW, y+BW, w, h);
  rbox(0, x+BW, y+BW, w, h);
  // draw the box:
  fl_rounded_box(x, y, w, h, c);
}

void fl_rounded_focus(Fl_Boxtype bt, int x, int y, int w, int h, Fl_Color fg, Fl_Color bg) {
  x += Fl::box_dx(bt);
  y += Fl::box_dy(bt);
  w -= Fl::box_dw(bt)+1;
  h -= Fl::box_dh(bt)+1;
  Fl_Color savecolor = fl_color();
  fl_color(fl_contrast(fg, bg));
  fl_line_style(FL_DOT);
  rbox(0, x+1, y+1, w-1, h-1);
  fl_line_style(FL_SOLID);
  fl_color(savecolor);
}

extern void fl_internal_boxtype(Fl_Boxtype, Fl_Box_Draw_F*, Fl_Box_Draw_Focus_F* =NULL);

Fl_Boxtype fl_define_FL_ROUNDED_BOX() {
  fl_internal_boxtype(_FL_ROUNDED_FRAME, fl_rounded_frame, fl_rounded_focus);
  fl_internal_boxtype(_FL_ROUNDED_BOX, fl_rounded_box, fl_rounded_focus);
  return _FL_ROUNDED_BOX;
}

Fl_Boxtype fl_define_FL_RFLAT_BOX() {
  fl_internal_boxtype(_FL_RFLAT_BOX, fl_rflat_box, fl_rounded_focus);
  return _FL_RFLAT_BOX;
}

Fl_Boxtype fl_define_FL_RSHADOW_BOX() {
  fl_internal_boxtype(_FL_RSHADOW_BOX, fl_rshadow_box, fl_rounded_focus);
  return _FL_RSHADOW_BOX;
}
