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
//------- test viewport clipping ----------
//
class ViewportTest : public Fl_Box {
public: 
  static Fl_Widget *create() {
    return new ViewportTest(TESTAREA_X, TESTAREA_Y, TESTAREA_W, TESTAREA_H);
  }
  ViewportTest(int x, int y, int w, int h) : Fl_Box(x, y, w, h) { 
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
  void show() {
    Fl_Box::show();
    mainwin->testAlignment(1);
  }
  void hide() {
    Fl_Box::hide();
    mainwin->testAlignment(0);
  }
};

UnitTest viewport("viewport test", ViewportTest::create);

//
// End of "$Id$
//
