//
// "$Id: Fl_Light_Button.cxx,v 1.4.2.3 2001/01/22 15:13:40 easysw Exp $"
//
// Lighted button widget for the Fast Light Tool Kit (FLTK).
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

// Subclass of Fl_Button where the "box" indicates whether it is
// pushed or not, and the "down box" is drawn small and square on
// the left to indicate the current state.

// The default down_box of zero draws a rectangle designed to look
// just like Flame's buttons.

#include <FL/Fl.H>
#include <FL/Fl_Light_Button.H>
#include <FL/fl_draw.H>

void Fl_Light_Button::draw() {
  if (box()) draw_box(this==Fl::pushed() ? down(box()) : box(), color());
  Fl_Color col = value() ? selection_color() : color();
  int d = h()/6;
  int W = w()<h() ? w() : h();
  if (down_box()) {
    // draw other down_box() styles:
    draw_box(down_box(), x()+d, y()+d+1, W-2*d-2, W-2*d-2, col);
  } else {
    // if down_box() is zero, draw light button style:
    int hh = h()-2*d;
    int ww = hh/2+1;
    int xx = d*2;
    if (w()<ww+2*xx) xx = (w()-ww)/2;
    draw_box(FL_THIN_DOWN_BOX, x()+xx, y()+d, ww, hh, col);
  }
  draw_label(x()+W-d, y(), w()-W+d, h());
}

int Fl_Light_Button::handle(int event) {
  switch (event) {
  case FL_RELEASE:
    if (box()) redraw();
  default:
    return Fl_Button::handle(event);
  }
}

Fl_Light_Button::Fl_Light_Button(int x, int y, int w, int h, const char* l)
: Fl_Button(x, y, w, h, l) {
  type(FL_TOGGLE_BUTTON);
  selection_color(FL_YELLOW);
  align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
}

//
// End of "$Id: Fl_Light_Button.cxx,v 1.4.2.3 2001/01/22 15:13:40 easysw Exp $".
//
