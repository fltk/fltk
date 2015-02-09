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

// Fltk unit tests
// v0.1 - Greg combines Matthias + Ian's tests
// v0.2 - Ian's 02/12/09 fixes applied
// v0.3 - Fixes to circle desc, augmented extent tests, fixed indents, added show(argc,argv)
// v1.0 - Submit for svn
// v1.1 - Matthias seperated all tests into multiple source files for hopefully easier handling

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Help_View.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>		// fl_text_extents()

// WINDOW/WIDGET SIZES
#define MAINWIN_W	700				// main window w()
#define MAINWIN_H	400				// main window h()
#define BROWSER_X	10				// browser x()
#define BROWSER_Y	25				// browser y()
#define BROWSER_W	150				// browser w()
#define BROWSER_H	MAINWIN_H-35			// browser h()
#define TESTAREA_X	(BROWSER_W + 20)		// test area x()
#define TESTAREA_Y	25				// test area y()
#define TESTAREA_W	(MAINWIN_W - BROWSER_W - 30)	// test area w()
#define TESTAREA_H	BROWSER_H			// test area h()

typedef void (*UnitTestCallback)(const char*,Fl_Group*);

class MainWindow *mainwin = 0;
Fl_Hold_Browser *browser = 0;

// This class helps to automagically register a new test with the unittest app.
// Please see the examples on how this is used. 
class UnitTest {
public:
  UnitTest(const char *label, Fl_Widget* (*create)()) :
    fWidget(0L)
  {
    fLabel = strdup(label);
    fCreate = create;
    add(this);
  }
  ~UnitTest() {
    delete fWidget;
    free(fLabel);
  }
  const char *label() {
    return fLabel;
  }
  void create() {
    fWidget = fCreate();
    if (fWidget) fWidget->hide();
  }
  void show() {
    if (fWidget) fWidget->show();
  }
  void hide() {
    if (fWidget) fWidget->hide();
  }
  static int numTest() { return nTest; }
  static UnitTest *test(int i) { return fTest[i]; }
private:
  char *fLabel;
  Fl_Widget *(*fCreate)();
  Fl_Widget *fWidget;

  static void add(UnitTest *t) {
    fTest[nTest] = t;
    nTest++;
  }
  static int nTest;
  static UnitTest *fTest[];
};

int UnitTest::nTest = 0;
UnitTest *UnitTest::fTest[200];


// The main window needs an additional drawing feature in order to support 
// the viewport alignment test.
class MainWindow : public Fl_Double_Window {
public:
  MainWindow(int w, int h, const char *l=0L) :
    Fl_Double_Window(w, h, l),
    fTestAlignment(0)
  { }
  // this code is used by the viewport alignment test
  void drawAlignmentIndicators() {
    const int sze = 16;
    // top left corner
    fl_color(FL_GREEN); fl_yxline(0, sze, 0, sze);
    fl_color(FL_RED);   fl_yxline(-1, sze, -1, sze);
    fl_color(FL_WHITE); fl_rectf(3, 3, sze-2, sze-2);
    fl_color(FL_BLACK); fl_rect(3, 3, sze-2, sze-2);
    // bottom left corner
    fl_color(FL_GREEN); fl_yxline(0, h()-sze-1, h()-1, sze);
    fl_color(FL_RED);   fl_yxline(-1, h()-sze-1, h(), sze);
    fl_color(FL_WHITE); fl_rectf(3, h()-sze-1, sze-2, sze-2);
    fl_color(FL_BLACK); fl_rect(3, h()-sze-1, sze-2, sze-2);
    // bottom right corner
    fl_color(FL_GREEN); fl_yxline(w()-1, h()-sze-1, h()-1, w()-sze-1);
    fl_color(FL_RED);   fl_yxline(w(), h()-sze-1, h(), w()-sze-1);
    fl_color(FL_WHITE); fl_rectf(w()-sze-1, h()-sze-1, sze-2, sze-2);
    fl_color(FL_BLACK); fl_rect(w()-sze-1, h()-sze-1, sze-2, sze-2);
    // top right corner
    fl_color(FL_GREEN); fl_yxline(w()-1, sze, 0, w()-sze-1);
    fl_color(FL_RED);   fl_yxline(w(), sze, -1, w()-sze-1);
    fl_color(FL_WHITE); fl_rectf(w()-sze-1, 3, sze-2, sze-2);
    fl_color(FL_BLACK); fl_rect(w()-sze-1, 3, sze-2, sze-2);
  }
  void draw() {
    Fl_Double_Window::draw();
    if (fTestAlignment) {
      drawAlignmentIndicators();
    }
  }
  void testAlignment(int v) {
    fTestAlignment = v;
    redraw();
  }
  int fTestAlignment;
};

//------- include the various unit tests as inline code -------

#include "unittest_about.cxx"
#include "unittest_points.cxx"
#include "unittest_lines.cxx"
#include "unittest_rects.cxx"
#include "unittest_circles.cxx"
#include "unittest_text.cxx"
#include "unittest_symbol.cxx"
#include "unittest_images.cxx"
#include "unittest_viewport.cxx"
#include "unittest_scrollbarsize.cxx"
#include "unittest_schemes.cxx"

// callback whenever the browser value changes
void Browser_CB(Fl_Widget*, void*) {
  for ( int t=1; t<=browser->size(); t++ ) {
    UnitTest *ti = (UnitTest*)browser->data(t);
    if ( browser->selected(t) ) {
      ti->show();
    } else {
      ti->hide();
    }
  }
}


// this is the main call. It creates the window and adds all previously
// registered tests to the browser widget.
int main(int argc, char **argv) {
  Fl::args(argc,argv);
  Fl::get_system_colors();
  Fl::scheme(Fl::scheme()); // init scheme before instantiating tests
  Fl::visual(FL_RGB);
  mainwin = new MainWindow(MAINWIN_W, MAINWIN_H, "Fltk Unit Tests");
  browser = new Fl_Hold_Browser(BROWSER_X, BROWSER_Y, BROWSER_W, BROWSER_H, "Unit Tests");
  browser->align(FL_ALIGN_TOP|FL_ALIGN_LEFT);
  browser->when(FL_WHEN_CHANGED);
  browser->callback(Browser_CB);

  int i, n = UnitTest::numTest();
  for (i=0; i<n; i++) {
    UnitTest *t = UnitTest::test(i);
    mainwin->begin();
    t->create();
    mainwin->end();
    browser->add(t->label(), (void*)t);
  }

  /////
  mainwin->resizable(mainwin);
  mainwin->show(argc,argv);
  // Select first test in browser, and show that test.
  browser->select(1);
  Browser_CB(browser,0);
  return(Fl::run());
}

//
// End of "$Id$".
//
