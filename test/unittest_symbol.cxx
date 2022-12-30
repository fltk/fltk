//
// Unit tests for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2022 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//

#include "unittests.h"

#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>

//
// Test symbol rendering
//
class Ut_Symbol_Test : public Fl_Widget
{
  void draw_text_and_boxes(const char *txt, int X, int Y) {
    int wo = 0, ho = 0;
    fl_measure(txt, wo, ho, 1);
    // Draw fl_measure() rect
    fl_color(FL_RED);
    fl_rect(X, Y, wo, ho);
    // //////////////////////////////////////////////////////////////////////
    // NOTE: fl_text_extents() currently does not support multiline strings..
    //       until it does, let's leave this out, as we do multiline tests..
    // //////////////////////////////////////////////////////////////////////
    // // draw fl_text_extents() glyph bounding box
    // int dx,dy;
    // fl_text_extents(txt, dx, dy, wo, ho);
    // fl_color(FL_GREEN);
    // fl_rect(X+dx, Y+dy, wo, ho);
    //
    // Draw text with symbols enabled
    fl_color(FL_BLACK);
    fl_draw(txt, X, Y, 10, 10, FL_ALIGN_INSIDE|FL_ALIGN_TOP|FL_ALIGN_LEFT, 0, 1);
  }
public:
  static Fl_Widget *create() {
    return new Ut_Symbol_Test(UT_TESTAREA_X, UT_TESTAREA_Y, UT_TESTAREA_W, UT_TESTAREA_H);
  }
  Ut_Symbol_Test(int x, int y, int w, int h)
    : Fl_Widget(x, y, w, h) {
  }
  void draw(void) FL_OVERRIDE {
    int x0 = x(); // origin is current window position for Fl_Box
    int y0 = y();
    int w0 = w();
    int h0 = h();
    fl_push_clip(x0, y0, w0, h0); // reset local clipping
    {
      // set the background colour - slightly off-white to enhance the green bounding box
      fl_color(fl_gray_ramp(FL_NUM_GRAY - 3));
      fl_rectf(x0, y0, w0, h0);
      int fsize = 25;
      fl_font(FL_HELVETICA, fsize);
      int xx = x0+10;
      int yy = y0+10;
      draw_text_and_boxes("Text"            ,xx,yy); yy += fsize+10;       // check no symbols
      draw_text_and_boxes("@->"             ,xx,yy); yy += fsize+10;       // check symbol alone
      draw_text_and_boxes("@-> "            ,xx,yy); yy += fsize+10;       // check symbol with trailing space
      draw_text_and_boxes("@-> Rt Arrow"    ,xx,yy); yy += fsize+10;       // check symbol at left edge
      draw_text_and_boxes("Lt Arrow @<-"    ,xx,yy); yy += fsize+10;       // check symbol at right edge
      draw_text_and_boxes("@-> Rt/Lt @<-"   ,xx,yy); yy += fsize+10;       // check symbol at lt+rt edges
      draw_text_and_boxes("@@ At/Lt @<-"    ,xx,yy); yy += fsize+10;       // check @@ at left, symbol at right
      draw_text_and_boxes("@-> Lt/At @@"    ,xx,yy); yy += fsize+10;       // check symbol at left, @@ at right
      draw_text_and_boxes("@@ At/At @@"     ,xx,yy); yy += fsize+10;       // check @@ at left+right
      xx = x0+200;
      yy = y0+10;
      draw_text_and_boxes("Line1\nLine2"               ,xx,yy); yy += (fsize+10)*2; // check 2 lines, no symbol
      draw_text_and_boxes("@-> Line1\nLine2 @<-"       ,xx,yy); yy += (fsize+10)*2; // check 2 lines, lt+rt symbols
      draw_text_and_boxes("@-> Line1\nLine2\nLine3 @<-",xx,yy); yy += (fsize+10)*3; // check 3 lines, lt+rt symbols
      draw_text_and_boxes("@@@@"                       ,xx,yy); yy += (fsize+10);   // check abutting @@'s
      draw_text_and_boxes("@@ @@"                      ,xx,yy); yy += (fsize+10);   // check @@'s with space sep

      fl_font(FL_HELVETICA, 14);
      fl_color(FL_RED);
      fl_draw("fl_measure bounding box in RED", x0+10,y0+h0-20);
    }
    fl_pop_clip(); // remove the local clip
  }
};

UnitTest symbolExtents(UT_TEST_SYBOL, "Symbol Text", Ut_Symbol_Test::create);
