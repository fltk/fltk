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
#include <FL/Fl_Check_Button.H>
#include <FL/fl_draw.H>

//
// --- fl_text_extents() tests -----------------------------------------------
static void cb_base_bt(Fl_Widget *bt, void*) {
  bt->parent()->redraw();
}
//
class Ut_Text_Extents_Test : public Fl_Group
{
  Fl_Check_Button *base_bt;

  void draw_text_and_boxes(const char *txt, int X, int Y) {
    int wm = 0, hm = 0, wt = 0, ht = 0;
    int dx, dy;
    // measure text so we can draw the baseline first
    fl_measure(txt, wm, hm, 0);
    fl_text_extents(txt, dx, dy, wt, ht);
    // Draw a baseline before the boxes
    if (base_bt->value()) {
      fl_color(FL_BLUE);
      fl_line((X - 20), Y, (X + wt + 20), Y);
    }
    // Then we draw the bounding boxes (fl_measure and fl_text_extents)
    // draw fl_measure() typographical bounding box
    int desc = fl_descent();
    fl_color(FL_RED);
    fl_rect(X, Y-hm+desc, wm, hm);
    // draw fl_text_extents() glyph bounding box
    fl_color(FL_GREEN);
    fl_rect(X+dx, Y+dy, wt, ht);
    // Then we draw the text to show how it fits inside each of the two boxes
    fl_color(FL_BLACK);
    fl_draw(txt, X, Y);
  }
public:
  static Fl_Widget *create() {
    return new Ut_Text_Extents_Test(UT_TESTAREA_X, UT_TESTAREA_Y, UT_TESTAREA_W, UT_TESTAREA_H);
  }
  Ut_Text_Extents_Test(int x, int y, int w, int h) : Fl_Group(x, y, w, h) {
      base_bt = new Fl_Check_Button(x + w - 150, 50, 130, 20, "Show Baseline");
      base_bt->box(FL_FLAT_BOX);
      base_bt->down_box(FL_DOWN_BOX);
      base_bt->callback(cb_base_bt);

      Fl_Box *dummy = new Fl_Box ((x + w - 4), (y + h - 4), 2, 2);
      resizable(dummy);
      end();
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

      Fl_Group::draw();

      fl_font(FL_HELVETICA, 30);
      int xx = x0+55;
      int yy = y0+40;
      draw_text_and_boxes("!abcdeABCDE\"#A", xx, yy); yy += 50;     // mixed string
      draw_text_and_boxes("oacs",     xx, yy); xx += 100;           // small glyphs
      draw_text_and_boxes("qjgIPT",   xx, yy); yy += 50; xx -= 100; // glyphs with descenders
      draw_text_and_boxes("````````", xx, yy); yy += 50;            // high small glyphs
      draw_text_and_boxes("--------", xx, yy); yy += 50;            // mid small glyphs
      draw_text_and_boxes("________", xx, yy); yy += 50;            // low small glyphs

      fl_font(FL_HELVETICA, 14);
      fl_color(FL_RED);  fl_draw("fl_measure bounding box in RED",       xx, yy); yy += 20;
      fl_color(FL_GREEN); fl_draw("fl_text_extents bounding box in GREEN", xx, yy);
      fl_color(FL_BLACK);
      xx = x0 + 10;  yy += 30;
      fl_draw("NOTE: On systems with text anti-aliasing (e.g. macOS Quartz)", xx, yy);
      w0 = h0 = 0; fl_measure("NOTE: ", w0, h0, 0);
      xx += w0; yy += h0;
      fl_draw("text may leak slightly outside the fl_text_extents()", xx, yy);
    }
    fl_pop_clip(); // remove the local clip
  }
};

UnitTest textExtents(UT_TEST_TEXT, "Rendering Text", Ut_Text_Extents_Test::create);
