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

#ifndef UNITTESTS_H
#define UNITTESTS_H 1

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>

// WINDOW/WIDGET SIZES
const int UT_MAINWIN_W  = 700;                                // main window w()
const int UT_MAINWIN_H  = 400;                                // main window h()
const int UT_BROWSER_X  = 10;                                 // browser x()
const int UT_BROWSER_Y  = 25;                                 // browser y()
const int UT_BROWSER_W  = 150;                                // browser w()
const int UT_BROWSER_H  = (UT_MAINWIN_H-35);                  // browser h()
const int UT_TESTAREA_X = (UT_BROWSER_W + 20);                // test area x()
const int UT_TESTAREA_Y = 25;                                 // test area y()
const int UT_TESTAREA_W = (UT_MAINWIN_W - UT_BROWSER_W - 30); // test area w()
const int UT_TESTAREA_H = UT_BROWSER_H;                       // test area h()

typedef void (*UnitTestCallback)(const char*, class Fl_Group*);

extern class Ut_Main_Window*  mainwin;
extern class Fl_Hold_Browser* browser;

enum {
  UT_TEST_ABOUT = 0,
  UT_TEST_POINTS,
  UT_TEST_FAST_SHAPES,
  UT_TEST_CIRCLES,
  UT_TEST_COMPLEX_SHAPES,
  UT_TEST_TEXT,
  UT_TEST_UNICODE,
  UT_TEST_SYBOL,
  UT_TEST_IMAGES,
  UT_TEST_VIEWPORT,
  UT_TEST_SCROLLBARSIZE,
  UT_TEST_SCHEMES,
  UT_TEST_SIMPLE_TERMINAL
};

// This class helps to automatically register a new test with the unittest app.
// Please see the examples on how this is used.
class UnitTest {
public:
  UnitTest(int index, const char *label, Fl_Widget* (*create)());
  ~UnitTest();
  const char* label();
  void create();
  void show();
  void hide();
  static int num_tests() { return num_tests_; }
  static UnitTest* test(int i) { return test_list_[i]; }
private:
  char* label_;
  Fl_Widget* (*create_)();
  Fl_Widget* widget_;
  static void add(int index, UnitTest* t);
  static int num_tests_;
  static UnitTest* test_list_[];
};

// The main window needs an additional drawing feature in order to support
// the viewport alignment test.
class Ut_Main_Window : public Fl_Double_Window {
public:
  Ut_Main_Window(int w, int h, const char *l=0L);
  void draw_alignment_indicators();
  void draw();
  void test_alignment(int v);
private:
  int draw_alignment_test_;
};

#endif
