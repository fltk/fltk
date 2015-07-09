//
// "$Id$"
//
// Diamond box code for the Fast Light Tool Kit (FLTK).
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

// Box drawing code for an obscure box type.
// These box types are in separate files so they are not linked
// in if not used.

// The diamond box draws best if the area is square!

#include <FL/Fl.H>
#include <FL/fl_draw.H>

extern const uchar* fl_gray_ramp();

static void fl_diamond_up_box(int x,int y,int w,int h,Fl_Color bgcolor) {
  w &= -2;
  h &= -2;
  int x1 = x+w/2;
  int y1 = y+h/2;
  Fl::set_box_color(bgcolor);
  fl_polygon(x+3, y1, x1,y+3, x+w-3,y1, x1,y+h-3);
  const uchar *g = fl_gray_ramp();
  fl_color(g[(int)'W']); fl_line(x+1, y1, x1, y+1, x+w-1, y1);
  fl_color(g[(int)'U']); fl_line(x+2, y1, x1, y+2, x+w-2, y1);
  fl_color(g[(int)'S']); fl_line(x+3, y1, x1, y+3, x+w-3, y1);
  fl_color(g[(int)'P']); fl_line(x+3, y1, x1, y+h-3, x+w-3, y1);
  fl_color(g[(int)'N']); fl_line(x+2, y1, x1, y+h-2, x+w-2, y1);
  fl_color(g[(int)'H']); fl_line(x+1, y1, x1, y+h-1, x+w-1, y1);
  fl_color(g[(int)'A']); fl_loop(x, y1, x1, y, x+w, y1, x1, y+h);
}

static void fl_diamond_down_box(int x,int y,int w,int h,Fl_Color bgcolor) {
  w &= -2;
  h &= -2;
  int x1 = x+w/2;
  int y1 = y+h/2;
  const uchar *g = fl_gray_ramp();
  fl_color(g[(int)'P']); fl_line(x+0, y1, x1, y+0, x+w-0, y1);
  fl_color(g[(int)'N']); fl_line(x+1, y1, x1, y+1, x+w-1, y1);
  fl_color(g[(int)'H']); fl_line(x+2, y1, x1, y+2, x+w-2, y1);
  fl_color(g[(int)'W']); fl_line(x+2, y1, x1, y+h-2, x+w-2, y1);
  fl_color(g[(int)'U']); fl_line(x+1, y1, x1, y+h-1, x+w-1, y1);
  fl_color(g[(int)'S']); fl_line(x+0, y1, x1, y+h-0, x+w-0, y1);
  Fl::set_box_color(bgcolor);
  fl_polygon(x+3, y1, x1,y+3, x+w-3,y1, x1,y+h-3);
  fl_color(g[(int)'A']); fl_loop(x+3, y1, x1, y+3, x+w-3, y1, x1, y+h-3);
}

extern void fl_internal_boxtype(Fl_Boxtype, Fl_Box_Draw_F*);
Fl_Boxtype fl_define_FL_DIAMOND_BOX() {
  fl_internal_boxtype(_FL_DIAMOND_DOWN_BOX, fl_diamond_down_box);
  fl_internal_boxtype(_FL_DIAMOND_UP_BOX,fl_diamond_up_box);
  return _FL_DIAMOND_UP_BOX;
}

//
// End of "$Id$".
//
