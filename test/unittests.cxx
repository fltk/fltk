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

#include "unittests.h"

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Help_View.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>     // fl_text_extents()
#include <FL/fl_string_functions.h>   // fl_strdup()
#include <stdlib.h>         // malloc, free

class MainWindow *mainwin = 0;
class Fl_Hold_Browser *browser = 0;

UnitTest::UnitTest(int index, const char *label, Fl_Widget* (*create)()) :
  fWidget(0L)
{
  fLabel = fl_strdup(label);
  fCreate = create;
  add(index, this);
}

UnitTest::~UnitTest() {
  delete fWidget;
  free(fLabel);
}

const char *UnitTest::label() {
  return fLabel;
}

void UnitTest::create() {
  fWidget = fCreate();
  if (fWidget) fWidget->hide();
}

void UnitTest::show() {
  if (fWidget) fWidget->show();
}

void UnitTest::hide() {
  if (fWidget) fWidget->hide();
}

void UnitTest::add(int index, UnitTest *t) {
  fTest[index] = t;
  if (index>=nTest)
    nTest = index+1;
}

int UnitTest::nTest = 0;
UnitTest *UnitTest::fTest[200] = { 0 };

MainWindow::MainWindow(int w, int h, const char *l) :
Fl_Double_Window(w, h, l),
fTestAlignment(0)
{ }

void MainWindow::drawAlignmentIndicators() {
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

void MainWindow::draw() {
  Fl_Double_Window::draw();
  if (fTestAlignment) {
    drawAlignmentIndicators();
  }
}

void MainWindow::testAlignment(int v) {
  fTestAlignment = v;
  redraw();
}

//------- include the various unit tests as inline code -------

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


// This is the main call. It creates the window and adds all previously
// registered tests to the browser widget.
int main(int argc, char **argv) {
  Fl::args(argc,argv);
  Fl::get_system_colors();
  Fl::scheme(Fl::scheme()); // init scheme before instantiating tests
  Fl::visual(FL_RGB);
  Fl::use_high_res_GL(1);
  mainwin = new MainWindow(MAINWIN_W, MAINWIN_H, "FLTK Unit Tests");
  mainwin->size_range(MAINWIN_W, MAINWIN_H);
  browser = new Fl_Hold_Browser(BROWSER_X, BROWSER_Y, BROWSER_W, BROWSER_H, "Unit Tests");
  browser->align(FL_ALIGN_TOP|FL_ALIGN_LEFT);
  browser->when(FL_WHEN_CHANGED);
  browser->callback(Browser_CB);

  int i, n = UnitTest::numTest();
  for (i=0; i<n; i++) {
    UnitTest *t = UnitTest::test(i);
    if (t) {
      mainwin->begin();
      t->create();
      mainwin->end();
      browser->add(t->label(), (void*)t);
    }
  }

  mainwin->resizable(mainwin);
  mainwin->show(argc,argv);
  // Select first test in browser, and show that test.
  browser->select(kTestAbout+1);
  Browser_CB(browser,0);
  return(Fl::run());
}
