//
// "$Id: fl_shadow_box.cxx,v 1.4.2.3 2001/01/22 15:13:41 easysw Exp $"
//
// Shadow box drawing routines for the Fast Light Tool Kit (FLTK).
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

#include <FL/Fl.H>
#include <FL/fl_draw.H>

#define BW 3

static void fl_shadow_frame(int x, int y, int w, int h, Fl_Color c) {
  fl_color(FL_DARK3);
  fl_rectf(x+BW, y+h,  w, BW);
  fl_rectf(x+w,  y+BW, BW,  h);
  fl_color(c);
  fl_rect(x,y,w,h);
}

static void fl_shadow_box(int x, int y, int w, int h, Fl_Color c) {
  fl_color(c);
  fl_rectf(x+1,y+1,w-2,h-2);
  fl_shadow_frame(x,y,w,h,FL_GRAY0);
}

extern void fl_internal_boxtype(Fl_Boxtype, Fl_Box_Draw_F*);
Fl_Boxtype define_FL_SHADOW_BOX() {
  fl_internal_boxtype(_FL_SHADOW_FRAME, fl_shadow_frame);
  fl_internal_boxtype(_FL_SHADOW_BOX, fl_shadow_box);
  return _FL_SHADOW_BOX;
}

//
// End of "$Id: fl_shadow_box.cxx,v 1.4.2.3 2001/01/22 15:13:41 easysw Exp $".
//
