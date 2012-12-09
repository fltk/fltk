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
// --- fl_text_extents() tests -----------------------------------------------
//
class TextExtentsTest : public Fl_Widget
{
  void DrawTextAndBoxes(const char *txt, int X, int Y) {
    int wo = 0, ho = 0;
    int dx, dy;
    // First, we draw the bounding boxes (fl_measure and fl_text_extents)
    // draw fl_measure() typographical bounding box
    fl_measure(txt, wo, ho, 0);
    int desc = fl_descent();
    fl_color(FL_RED);
    fl_rect(X, Y-ho+desc, wo, ho);
    // draw fl_text_extents() glyph bounding box
    fl_text_extents(txt, dx, dy, wo, ho);
    fl_color(FL_GREEN);
    fl_rect(X+dx, Y+dy, wo, ho);
    // Then we draw the text to show how it fits insode each of the two boxes
    fl_color(FL_BLACK);
    fl_draw(txt, X, Y);
  }
public: 
  static Fl_Widget *create() {
    return new TextExtentsTest(TESTAREA_X, TESTAREA_Y, TESTAREA_W, TESTAREA_H);
  }
  TextExtentsTest(int x, int y, int w, int h) : Fl_Widget(x, y, w, h) {}
  void draw(void) {
    int x0 = x(); // origin is current window position for Fl_Box
    int y0 = y();
    int w0 = w();
    int h0 = h();
    fl_push_clip(x0, y0, w0, h0); // reset local clipping
    {
      // set the background colour - slightly off-white to enhance the green bounding box
      fl_color(fl_gray_ramp(FL_NUM_GRAY - 3));
      fl_rectf(x0, y0, w0, h0);

      fl_font(FL_HELVETICA, 30);
      int xx = x0+55;
      int yy = y0+40;
      DrawTextAndBoxes("!abcdeABCDE\"#A", xx, yy); yy += 50;	// mixed string
      DrawTextAndBoxes("oacs",     xx, yy); xx += 100;		// small glyphs
      DrawTextAndBoxes("qjgIPT",   xx, yy); yy += 50; xx -= 100;	// glyphs with descenders
      DrawTextAndBoxes("````````", xx, yy); yy += 50;		// high small glyphs
      DrawTextAndBoxes("--------", xx, yy); yy += 50;		// mid small glyphs
      DrawTextAndBoxes("________", xx, yy); yy += 50;		// low small glyphs

      fl_font(FL_HELVETICA, 14);
      fl_color(FL_RED);  fl_draw("fl_measure bounding box in RED",       xx, yy); yy += 20;
      fl_color(FL_GREEN); fl_draw("fl_text_extents bounding box in GREEN", xx, yy);
      fl_color(FL_BLACK);
      xx = x0 + 10;  yy += 30;
      fl_draw("NOTE: On systems with text anti-aliasing (e.g. OSX Quartz)", xx, yy);
      w0 = h0 = 0; fl_measure("NOTE: ", w0, h0, 0);
      xx += w0; yy += h0;
      fl_draw("text may leak slightly outside the fl_text_extents()", xx, yy);
    }
    fl_pop_clip(); // remove the local clip
  }
};

UnitTest textExtents("rendering text", TextExtentsTest::create);

//
// End of "$Id$"
//
