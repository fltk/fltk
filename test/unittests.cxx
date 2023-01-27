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

// Fltk unit tests
// v0.1 - Greg combines Matthias + Ian's tests
// v0.2 - Ian's 02/12/09 fixes applied
// v0.3 - Fixes to circle desc, augmented extent tests, fixed indents, added show(argc,argv)
// v1.0 - Submit for svn
// v1.1 - Matthias seperated all tests into multiple source files for hopefully easier handling

// NOTE: See README-unittests.txt for how to add new tests.

#include "unittests.h"

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Help_View.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>               // fl_text_extents()
#include <FL/fl_string_functions.h>   // fl_strdup()
#include <stdlib.h>                   // malloc, free

class Ut_Main_Window *mainwin = NULL;
class Fl_Hold_Browser *browser = NULL;

int UnitTest::num_tests_ = 0;
UnitTest *UnitTest::test_list_[200] = { 0 };

UnitTest::UnitTest(int index, const char* label, Fl_Widget* (*create)())
  : widget_(0L)
{
  label_ = fl_strdup(label);
  create_ = create;
  add(index, this);
}

UnitTest::~UnitTest() {
  delete widget_;
  free(label_);
}

const char *UnitTest::label() {
  return label_;
}

void UnitTest::create() {
  widget_ = create_();
  if (widget_) widget_->hide();
}

void UnitTest::show() {
  if (widget_) widget_->show();
}

void UnitTest::hide() {
  if (widget_) widget_->hide();
}

void UnitTest::add(int index, UnitTest* t) {
  test_list_[index] = t;
  if (index >= num_tests_)
    num_tests_ = index+1;
}

Ut_Main_Window::Ut_Main_Window(int w, int h, const char *l)
  : Fl_Double_Window(w, h, l),
    draw_alignment_test_(0)
{ }

void Ut_Main_Window::draw_alignment_indicators() {
  const int SZE = 16;
  // top left corner
  fl_color(FL_GREEN); fl_yxline(0, SZE, 0, SZE);
  fl_color(FL_RED);   fl_yxline(-1, SZE, -1, SZE);
  fl_color(FL_WHITE); fl_rectf(3, 3, SZE-2, SZE-2);
  fl_color(FL_BLACK); fl_rect(3, 3, SZE-2, SZE-2);
  // bottom left corner
  fl_color(FL_GREEN); fl_yxline(0, h()-SZE-1, h()-1, SZE);
  fl_color(FL_RED);   fl_yxline(-1, h()-SZE-1, h(), SZE);
  fl_color(FL_WHITE); fl_rectf(3, h()-SZE-1, SZE-2, SZE-2);
  fl_color(FL_BLACK); fl_rect(3, h()-SZE-1, SZE-2, SZE-2);
  // bottom right corner
  fl_color(FL_GREEN); fl_yxline(w()-1, h()-SZE-1, h()-1, w()-SZE-1);
  fl_color(FL_RED);   fl_yxline(w(), h()-SZE-1, h(), w()-SZE-1);
  fl_color(FL_WHITE); fl_rectf(w()-SZE-1, h()-SZE-1, SZE-2, SZE-2);
  fl_color(FL_BLACK); fl_rect(w()-SZE-1, h()-SZE-1, SZE-2, SZE-2);
  // top right corner
  fl_color(FL_GREEN); fl_yxline(w()-1, SZE, 0, w()-SZE-1);
  fl_color(FL_RED);   fl_yxline(w(), SZE, -1, w()-SZE-1);
  fl_color(FL_WHITE); fl_rectf(w()-SZE-1, 3, SZE-2, SZE-2);
  fl_color(FL_BLACK); fl_rect(w()-SZE-1, 3, SZE-2, SZE-2);
}

void Ut_Main_Window::draw() {
  Fl_Double_Window::draw();
  if (draw_alignment_test_) {
    draw_alignment_indicators();
  }
}

void Ut_Main_Window::test_alignment(int v) {
  draw_alignment_test_ = v;
  redraw();
}

//------- include the various unit tests as inline code -------

// callback whenever the browser value changes
void UT_BROWSER_CB(Fl_Widget*, void*) {
  for ( int t=1; t<=browser->size(); t++ ) {
    UnitTest* ti = (UnitTest*)browser->data(t);
    if ( browser->selected(t) ) {
      ti->show();
    } else {
      ti->hide();
    }
  }
}


// This is the main call. It creates the window and adds all previously
// registered tests to the browser widget.
int main(int argc, char** argv) {
  Fl::args(argc, argv);
  Fl::get_system_colors();
  Fl::scheme(Fl::scheme()); // init scheme before instantiating tests
  Fl::visual(FL_RGB);
  Fl::use_high_res_GL(1);
  mainwin = new Ut_Main_Window(UT_MAINWIN_W, UT_MAINWIN_H, "FLTK Unit Tests");
  mainwin->size_range(UT_MAINWIN_W, UT_MAINWIN_H);
  browser = new Fl_Hold_Browser(UT_BROWSER_X, UT_BROWSER_Y, UT_BROWSER_W, UT_BROWSER_H, "Unit Tests");
  browser->align(FL_ALIGN_TOP|FL_ALIGN_LEFT);
  browser->when(FL_WHEN_CHANGED);
  browser->callback(UT_BROWSER_CB);

  int i, n = UnitTest::num_tests();
  for (i=0; i<n; i++) {
    UnitTest* t = UnitTest::test(i);
    if (t) {
      mainwin->begin();
      t->create();
      mainwin->end();
      browser->add(t->label(), (void*)t);
    }
  }

  mainwin->resizable(mainwin);
  mainwin->show(argc, argv);
  // Select first test in browser, and show that test.
  browser->select(UT_TEST_ABOUT+1);
  UT_BROWSER_CB(browser, 0);
  return Fl::run();
}
