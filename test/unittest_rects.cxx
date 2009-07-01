//
// "$Id$"
//
// Unit tests for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2009 by Bill Spitzak and others.
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
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>		// fl_text_extents()

//
//------- test the rectangle drawing capabilities of this implementation ----------
//
class RectTest : public Fl_Box {
public: 
  static Fl_Widget *create() {
    return new RectTest(TESTAREA_X, TESTAREA_Y, TESTAREA_W, TESTAREA_H);
  }
  RectTest(int x, int y, int w, int h) : Fl_Box(x, y, w, h) {
    label("testing the fl_rect call\n"
	  "No red pixels should be visible. "
	  "If you see bright red lines, or if parts of the green frames are hidden, "
	  "the rect drawing alignment is off.");
    align(FL_ALIGN_INSIDE|FL_ALIGN_BOTTOM|FL_ALIGN_LEFT|FL_ALIGN_WRAP);
    box(FL_BORDER_BOX);
  }
  void draw() {
    Fl_Box::draw();
    int a = x()+10, b = y()+10; fl_color(FL_BLACK); fl_rect(a, b, 100, 100);
    // testing fl_rect() with positive size
    fl_color(FL_RED);   fl_loop(a+10, b+10, a+40, b+10, a+40, b+40, a+10, b+40);
    fl_color(FL_GREEN); fl_loop(a+ 9, b+ 9, a+41, b+ 9, a+41, b+41, a+ 9, b+41);
    fl_color(FL_GREEN); fl_loop(a+11, b+11, a+39, b+11, a+39, b+39, a+11, b+39);
    fl_color(FL_BLACK); fl_rect(a+10, b+10, 31, 31);
    // testing fl_rect() with positive size
    fl_color(FL_RED);   fl_loop(a+60, b+60, a+90, b+60, a+90, b+90, a+60, b+90);
    fl_color(FL_GREEN); fl_loop(a+59, b+59, a+91, b+59, a+91, b+91, a+59, b+91);
    fl_color(FL_BLACK); fl_rectf(a+60, b+60, 31, 31);
  }
};

UnitTest rects("rectangles", RectTest::create);

//
// End of "$Id$"
//
