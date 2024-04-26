//
// Oval box drawing code for the Fast Light Tool Kit (FLTK).
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

// Less-used box types are in separate files so they are not linked
// in if not used.

#include <FL/Fl.H>
#include <FL/fl_draw.H>

// Global parameters for box drawing algorithm:
//
//  BW = box shadow width
#define BW (Fl::box_shadow_width())

static void fl_oval_flat_box(int x, int y, int w, int h, Fl_Color c) {
  Fl::set_box_color(c);
  fl_pie(x, y, w, h, 0, 360);
}

static void fl_oval_frame(int x, int y, int w, int h, Fl_Color c) {
  Fl::set_box_color(c);
  fl_arc(x, y, w, h, 0, 360);
}

static void fl_oval_box(int x, int y, int w, int h, Fl_Color c) {
  fl_oval_flat_box(x,y,w,h,c);
  fl_oval_frame(x,y,w,h,FL_BLACK);
}

static void fl_oval_shadow_box(int x, int y, int w, int h, Fl_Color c) {
  fl_oval_flat_box(x+BW,y+BW,w,h,FL_DARK3);
  fl_oval_box(x,y,w,h,c);
}

void fl_oval_focus(Fl_Boxtype bt, int x, int y, int w, int h, Fl_Color fg, Fl_Color bg) {
  x += Fl::box_dx(bt)+1;
  y += Fl::box_dy(bt)+1;
  w -= Fl::box_dw(bt)+2;
  h -= Fl::box_dh(bt)+2;
  Fl_Color savecolor = fl_color();
  fl_color(fl_contrast(fg, bg));
  fl_line_style(FL_DOT);
  fl_arc(x, y, w, h, 0, 360);
  fl_line_style(FL_SOLID);
  fl_color(savecolor);
}

extern void fl_internal_boxtype(Fl_Boxtype, Fl_Box_Draw_F*, Fl_Box_Draw_Focus_F* =NULL);
Fl_Boxtype fl_define_FL_OVAL_BOX() {
  fl_internal_boxtype(_FL_OSHADOW_BOX, fl_oval_shadow_box, fl_oval_focus);
  fl_internal_boxtype(_FL_OVAL_FRAME, fl_oval_frame, fl_oval_focus);
  fl_internal_boxtype(_FL_OFLAT_BOX, fl_oval_flat_box, fl_oval_focus);
  fl_internal_boxtype(_FL_OVAL_BOX, fl_oval_box, fl_oval_focus);
  return _FL_OVAL_BOX;
}
