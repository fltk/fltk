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
//------- test the circle drawing capabilities of this implementation ----------
//
class CircleTest : public Fl_Box {
public: 
  static Fl_Widget *create() { 
    return new CircleTest(TESTAREA_X, TESTAREA_Y, TESTAREA_W, TESTAREA_H);
  }
  CircleTest(int x, int y, int w, int h) : Fl_Box(x, y, w, h) {
    label("testing int drawing of circles and ovals (fl_arc, fl_pie)\n"
          "No red lines should be visible. "
          "If you see bright red pixels, the circle drawing alignment is off. "
          "If you see dark red pixels, your system supports anti-aliasing "
          "which should be of no concern. "
          "The green rectangles should not be overwritten by circle drawings.");
    align(FL_ALIGN_INSIDE|FL_ALIGN_BOTTOM|FL_ALIGN_LEFT|FL_ALIGN_WRAP);
    box(FL_BORDER_BOX);
  }
  void draw() {
    Fl_Box::draw();
    int a = x()+10, b = y()+10; fl_color(FL_BLACK); fl_rect(a, b, 100, 100);
    // test fl_arc for full circles
    fl_color(FL_GREEN); fl_rect(a+ 9, b+ 9, 33, 33);
    fl_color(FL_RED); fl_xyline(a+24, b+10, a+27); fl_xyline(a+24, b+40, a+27);
    fl_yxline(a+10, b+24, b+27); fl_yxline(a+40, b+24, b+27);
    fl_color(FL_BLACK); fl_arc(a+10, b+10, 31, 31, 0.0, 360.0);
    // test fl_arc segmet 1
    fl_color(FL_GREEN); fl_rect(a+54, b+ 4, 43, 43);
    fl_rect(a+54, b+4, 18, 18); fl_rect(a+79, b+29, 18, 18);
    fl_color(FL_RED); fl_point(a+55, b+30); fl_point(a+70, b+45);
    fl_point(a+80, b+5); fl_point(a+95, b+20);
    fl_color(FL_BLACK); fl_arc(a+65, b+ 5, 31, 31, -35.0, 125.0);
    // test fl_arc segmet 2
    fl_color(FL_BLACK); fl_arc(a+55, b+15, 31, 31, 145.0, 305.0);
    // test fl_pie for full circles
    fl_color(FL_RED); fl_xyline(a+24, b+60, a+27); fl_xyline(a+24, b+90, a+27);
    fl_yxline(a+10, b+74, b+77); fl_yxline(a+40, b+74, b+77);
    fl_color(FL_GREEN); fl_rect(a+ 9, b+59, 33, 33);
    fl_color(FL_BLACK); fl_pie(a+10, b+60, 31, 31, 0.0, 360.0);
    // test fl_pie segmet 1
    fl_color(FL_GREEN); fl_rect(a+54, b+54, 43, 43);
    fl_rect(a+54, b+54, 18, 18); fl_rect(a+79, b+79, 18, 18);
    fl_point(a+79, b+71); fl_point(a+71, b+79);
    fl_color(FL_RED); fl_point(a+55, b+80); fl_point(a+70, b+95);
    fl_point(a+80, b+55); fl_point(a+95, b+70);
    fl_point(a+81, b+69); fl_point(a+69, b+81);
    fl_color(FL_BLACK); fl_pie(a+65, b+55, 31, 31, -30.0, 120.0);
    // test fl_pie segmet 2
    fl_color(FL_BLACK); fl_pie(a+55, b+65, 31, 31, 150.0, 300.0);
    //---- oval testing (horizontal squish)
    a +=120; b += 0; fl_color(FL_BLACK); fl_rect(a, b, 100, 100);
    fl_color(FL_GREEN);
    fl_rect(a+19, b+9, 63, 33); fl_rect(a+19, b+59, 63, 33);
    fl_color(FL_BLACK);
    fl_arc(a+20, b+10, 61, 31, 0, 360); fl_pie(a+20, b+60, 61, 31, 0, 360);
    //---- oval testing (horizontal squish)
    a += 120; b += 0; fl_color(FL_BLACK); fl_rect(a, b, 100, 100);
    fl_color(FL_GREEN);
    fl_rect(a+9, b+19, 33, 63); fl_rect(a+59, b+19, 33, 63);
    fl_color(FL_BLACK);
    fl_arc(a+10, b+20, 31, 61, 0, 360); fl_pie(a+60, b+20, 31, 61, 0, 360);
  }
};

UnitTest circle("circles and arcs", CircleTest::create);

//
// End of "$Id$"
//
