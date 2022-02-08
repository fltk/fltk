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
#define MAINWIN_W       700                             // main window w()
#define MAINWIN_H       400                             // main window h()
#define BROWSER_X       10                              // browser x()
#define BROWSER_Y       25                              // browser y()
#define BROWSER_W       150                             // browser w()
#define BROWSER_H       MAINWIN_H-35                    // browser h()
#define TESTAREA_X      (BROWSER_W + 20)                // test area x()
#define TESTAREA_Y      25                              // test area y()
#define TESTAREA_W      (MAINWIN_W - BROWSER_W - 30)    // test area w()
#define TESTAREA_H      BROWSER_H                       // test area h()

typedef void (*UnitTestCallback)(const char*, class Fl_Group*);

extern class MainWindow *mainwin;
extern class Fl_Hold_Browser *browser;

enum {
  kTestAbout = 0,
  kTestPoints,
  kTestFastShapes,
  kTestCircles,
//  kTestComplexShapes,
  kTestText,
  kTestSymbol,
  kTestImages,
  kTestViewport,
  kTestScrollbarsize,
  kTestSchemes,
  kTestSimpleTerminal
};

// This class helps to automatically register a new test with the unittest app.
// Please see the examples on how this is used.
class UnitTest {
public:
  UnitTest(int index, const char *label, Fl_Widget* (*create)());
  ~UnitTest();
  const char *label();
  void create();
  void show();
  void hide();
  static int numTest() { return nTest; }
  static UnitTest *test(int i) { return fTest[i]; }
private:
  char *fLabel;
  Fl_Widget *(*fCreate)();
  Fl_Widget *fWidget;
  static void add(int index, UnitTest *t);
  static int nTest;
  static UnitTest *fTest[];
};

// The main window needs an additional drawing feature in order to support
// the viewport alignment test.
class MainWindow : public Fl_Double_Window {
public:
  MainWindow(int w, int h, const char *l=0L);
  void drawAlignmentIndicators();
  void draw();
  void testAlignment(int v);
  int fTestAlignment;
};

#endif
