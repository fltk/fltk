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
#include <FL/fl_draw.H>

//
//------- test the line drawing capabilities of this implementation ----------
//
class LineTest : public Fl_Box {
public: 
  static Fl_Widget *create() {
    return new LineTest(TESTAREA_X, TESTAREA_Y, TESTAREA_W, TESTAREA_H);
  }
  LineTest(int x, int y, int w, int h) : Fl_Box(x, y, w, h) {
    label("testing the integer based fl_line calls\n"
	  "No red pixels should be visible.\n"
	  "If you see bright red pixels, the line drawing alignment is off,\n"
	  "or the last pixel in a line does not get drawn.\n"
	  "If you see dark red pixels, anti-aliasing must be switched off.");
    align(FL_ALIGN_INSIDE|FL_ALIGN_BOTTOM|FL_ALIGN_LEFT|FL_ALIGN_WRAP);
    box(FL_BORDER_BOX);
  }
  void draw() {
    Fl_Box::draw();
    int a = x()+10, b = y()+10; fl_color(FL_BLACK); fl_rect(a, b, 100, 100);
    // testing fl_xyline(x, y, x1)
    fl_color(FL_RED); fl_point(a+10, b+10); fl_point(a+20, b+10);
    fl_color(FL_BLACK); fl_xyline(a+10, b+10, a+20);
    // testing fl_xyline(x, y, x1, y2);
    fl_color(FL_RED); fl_point(a+10, b+20); fl_point(a+20, b+20);
    fl_point(a+20, b+30);
    fl_color(FL_BLACK); fl_xyline(a+10, b+20, a+20, b+30);
    // testing fl_xyline(x, y, x1, y2, x3);
    fl_color(FL_RED); fl_point(a+10, b+40); fl_point(a+20, b+40);
    fl_point(a+20, b+50); fl_point(a+30, b+50);
    fl_color(FL_BLACK); fl_xyline(a+10, b+40, a+20, b+50, a+30);
    //+++ add testing for the fl_yxline commands!
    // testing fl_loop(x,y, x,y, x,y, x, y)
    fl_color(FL_RED); fl_point(a+60, b+60); fl_point(a+90, b+60);
    fl_point(a+60, b+90); fl_point(a+90, b+90);
    fl_color(FL_BLACK);
    fl_loop(a+60, b+60, a+90, b+60, a+90, b+90, a+60, b+90);
  }
};

UnitTest lines("drawing lines", LineTest::create);

//
// End of "$Id$"
//
