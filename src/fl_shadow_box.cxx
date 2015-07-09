//
// "$Id$"
//
// Shadow box drawing routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2011 by Bill Spitzak and others.
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

#include <FL/Fl.H>
#include <FL/fl_draw.H>

#define BW 3

static void fl_shadow_frame(int x, int y, int w, int h, Fl_Color c) {
  fl_color(FL_DARK3);
  fl_rectf(x+BW, y+h-BW,  w - BW, BW);
  fl_rectf(x+w-BW,  y+BW, BW,  h - BW);
  Fl::set_box_color(c);
  fl_rect(x,y,w-BW,h-BW);
}

static void fl_shadow_box(int x, int y, int w, int h, Fl_Color c) {
  Fl::set_box_color(c);
  fl_rectf(x+1,y+1,w-2-BW,h-2-BW);
  fl_shadow_frame(x,y,w,h,FL_GRAY0);
}

extern void fl_internal_boxtype(Fl_Boxtype, Fl_Box_Draw_F*);
Fl_Boxtype fl_define_FL_SHADOW_BOX() {
  fl_internal_boxtype(_FL_SHADOW_FRAME, fl_shadow_frame);
  fl_internal_boxtype(_FL_SHADOW_BOX, fl_shadow_box);
  return _FL_SHADOW_BOX;
}

//
// End of "$Id$".
//
