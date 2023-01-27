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
//------- test viewport clipping ----------
//
class Ut_Viewport_Test : public Fl_Box {
public:
  static Fl_Widget *create() {
    return new Ut_Viewport_Test(UT_TESTAREA_X, UT_TESTAREA_Y, UT_TESTAREA_W, UT_TESTAREA_H);
  }
  Ut_Viewport_Test(int x, int y, int w, int h) : Fl_Box(x, y, w, h) {
    label("Testing Viewport Alignment\n\n"
          "Only green lines should be visible.\n"
          "If red lines are visible in the corners of this window,\n"
          "your viewport alignment and clipping is off.\n"
          "If there is a space between the green lines and the window border,\n"
          "the viewport is off, but some clipping may be working.\n"
          "Also, your window size may be off to begin with.");
    align(FL_ALIGN_INSIDE|FL_ALIGN_CENTER|FL_ALIGN_WRAP);
    box(FL_BORDER_BOX);
  }
  void show() FL_OVERRIDE {
    Fl_Box::show();
    mainwin->test_alignment(1);
  }
  void hide() FL_OVERRIDE {
    Fl_Box::hide();
    mainwin->test_alignment(0);
  }
};

UnitTest viewport(UT_TEST_VIEWPORT, "Viewport Test", Ut_Viewport_Test::create);
