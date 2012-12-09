//
// "$Id$"
//
// Unit tests for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
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
