//
// "$Id: Fl_Light_Button.cxx,v 1.4.2.3.2.3 2001/09/04 13:13:29 easysw Exp $"
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
  Fl_Color col = value() ? (active_r() ? selection_color() :
                            fl_inactive(selection_color())) : color();
  int d = h()/6;
  int W = w()<h() ? w() : h();
  if (down_box()) {
    // draw other down_box() styles:
    switch (down_box()) {
      case FL_DOWN_BOX :
      case FL_UP_BOX :
        // Check box...
        draw_box(down_box(), x()+d + 1, y()+d+2, W-2*d-4, W-2*d-4, color());
	if (value()) {
	  fl_color(col);
          fl_line_style(FL_SOLID, 2);
	  fl_line(x() + W - d - 7, y() + d + 5,
	          x() + W / 2 - 1, y() + W - d - 7,
	          x() + d + 5, y() + W / 2);
          fl_line_style(FL_SOLID);
	}
        break;
      case _FL_ROUND_DOWN_BOX :
      case _FL_ROUND_UP_BOX :
        // Radio button...
        draw_box(down_box(), x()+d + 1, y()+d+2, W-2*d-4, W-2*d-4, color());
	if (value()) {
	  int size = W - 2 * d - 4;
	  fl_color(col);
	  if (size > 14) {
            fl_pie(x() + d + 5, y() + d + 6, size - 9, size - 9, 0.0, 360.0);
	  } else {
            // Small circles don't draw well with some X servers...
	    fl_rectf(x() + d + 6, y() + d + 6, 2, 4);
	    fl_rectf(x() + d + 5, y() + d + 7, 4, 2);
	  }
	}
        break;
      default :
        draw_box(down_box(), x()+d + 1, y()+d+2, W-2*d-4, W-2*d-4, col);
        break;
    }
  } else {
    // if down_box() is zero, draw light button style:
    int hh = h()-2*d - 2;
    int ww = hh/2+1;
    int xx = d*2;
    if (w()<ww+2*xx) xx = (w()-ww)/2;
    draw_box(FL_THIN_DOWN_BOX, x()+xx, y()+d+1, ww, hh, col);
  }
  draw_label(x()+W-d, y(), w()-W+d, h());
  if (Fl::focus() == this) draw_focus();
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
// End of "$Id: Fl_Light_Button.cxx,v 1.4.2.3.2.3 2001/09/04 13:13:29 easysw Exp $".
//
