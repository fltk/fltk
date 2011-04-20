//
// "$Id$:
//
// Unit tests for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
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
#include <FL/fl_draw.H>

//
//------- test the point drawing capabilities of this implementation ----------
//
class PointTest : public Fl_Box {
public:
  static Fl_Widget *create() {
    return new PointTest(TESTAREA_X, TESTAREA_Y, TESTAREA_W, TESTAREA_H);
  }
  PointTest(int x, int y, int w, int h) : Fl_Box(x, y, w, h) {
    label("testing the fl_point call\n"
	  "You should see four pixels each in black, red, green and blue. "
	  "Make sure that pixels are not anti-aliased (blurred across multiple pixels)!");
    align(FL_ALIGN_INSIDE|FL_ALIGN_BOTTOM|FL_ALIGN_LEFT|FL_ALIGN_WRAP);
    box(FL_BORDER_BOX);
  }
  void draw() {
    Fl_Box::draw();
    int a = x()+10, b = y()+10; 
    fl_color(FL_WHITE); fl_rectf(a, b, 90, 90);
    fl_color(FL_BLACK); fl_rect(a, b, 90, 90);
    fl_point(a+10, b+10); fl_point(a+20, b+20);
    fl_point(a+10, b+20); fl_point(a+20, b+10);
    fl_color(FL_RED); a = x()+70;
    fl_point(a+10, b+10); fl_point(a+20, b+20);
    fl_point(a+10, b+20); fl_point(a+20, b+10);
    fl_color(FL_GREEN); a = x()+10; b = y()+70;
    fl_point(a+10, b+10); fl_point(a+20, b+20);
    fl_point(a+10, b+20); fl_point(a+20, b+10);
    fl_color(FL_BLUE); a = x()+70;
    fl_point(a+10, b+10); fl_point(a+20, b+20);
    fl_point(a+10, b+20); fl_point(a+20, b+10);
  }
};

UnitTest points("drawing points", PointTest::create);

//
// End of "$Id$"
//
