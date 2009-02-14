//
// "$Id$"
//
// Unit tests for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2009 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
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

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Hold_Browser.H>
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

Fl_Window       *mainwin = 0;
Fl_Hold_Browser *browser = 0;

//////////////////////////////
// UNIT TEST CODE STARTS HERE
//////////////////////////////

//
//------- test the circle drawing capabilities of this implementation ----------
//
class CircleTest : public Fl_Box {
public: CircleTest(int x, int y, int w, int h) : Fl_Box(x, y, w, h) {
    label("testing int drawing of circles and ovals (fl_arc, fl_pie)\n"
          "No red lines should be visible. "
          "If you see bright red pixels, the circle drawing alignment is off. "
          "If you see dark red pixels, your syste supports anti-aliasing "
          "which should be of no concern. "
          "The green rectangles should not be overwritten by circle drawings.");
    align(FL_ALIGN_INSIDE|FL_ALIGN_BOTTOM|FL_ALIGN_LEFT|FL_ALIGN_WRAP);
    box(FL_BORDER_BOX);
  }
  void draw() {
    Fl_Box::draw();
    int a = x()+10, b = y()+10; fl_color(FL_BLACK); fl_rect(a, b, 100, 100);
    // test fl_arc for full circles
    fl_color(FL_GREEN); fl_rect(a+ 9, b+ 9, 33, 33);
    fl_color(FL_RED); fl_xyline(a+24, b+10, a+27); fl_xyline(a+24, b+40, a+27);
    fl_yxline(a+10, b+24, b+27); fl_yxline(a+40, b+24, b+27);
    fl_color(FL_BLACK); fl_arc(a+10, b+10, 31, 31, 0.0, 360.0);
    // test fl_arc segmet 1
    fl_color(FL_GREEN); fl_rect(a+54, b+ 4, 43, 43);
    fl_rect(a+54, b+4, 18, 18); fl_rect(a+79, b+29, 18, 18);
    fl_color(FL_RED); fl_point(a+55, b+30); fl_point(a+70, b+45);
    fl_point(a+80, b+5); fl_point(a+95, b+20);
    fl_color(FL_BLACK); fl_arc(a+65, b+ 5, 31, 31, -35.0, 125.0);
    // test fl_arc segmet 2
    fl_color(FL_BLACK); fl_arc(a+55, b+15, 31, 31, 145.0, 305.0);
    // test fl_pie for full circles
    fl_color(FL_RED); fl_xyline(a+24, b+60, a+27); fl_xyline(a+24, b+90, a+27);
    fl_yxline(a+10, b+74, b+77); fl_yxline(a+40, b+74, b+77);
    fl_color(FL_GREEN); fl_rect(a+ 9, b+59, 33, 33);
    fl_color(FL_BLACK); fl_pie(a+10, b+60, 31, 31, 0.0, 360.0);
    // test fl_pie segmet 1
    fl_color(FL_GREEN); fl_rect(a+54, b+54, 43, 43);
    fl_rect(a+54, b+54, 18, 18); fl_rect(a+79, b+79, 18, 18);
    fl_point(a+79, b+71); fl_point(a+71, b+79);
    fl_color(FL_RED); fl_point(a+55, b+80); fl_point(a+70, b+95);
    fl_point(a+80, b+55); fl_point(a+95, b+70);
    fl_point(a+81, b+69); fl_point(a+69, b+81);
    fl_color(FL_BLACK); fl_pie(a+65, b+55, 31, 31, -30.0, 120.0);
    // test fl_pie segmet 2
    fl_color(FL_BLACK); fl_pie(a+55, b+65, 31, 31, 150.0, 300.0);
    //---- oval testing (horizontal squish)
    a +=120; b += 0; fl_color(FL_BLACK); fl_rect(a, b, 100, 100);
    fl_color(FL_GREEN);
    fl_rect(a+19, b+9, 63, 33); fl_rect(a+19, b+59, 63, 33);
    fl_color(FL_BLACK);
    fl_arc(a+20, b+10, 61, 31, 0, 360); fl_pie(a+20, b+60, 61, 31, 0, 360);
    //---- oval testing (horizontal squish)
    a += 120; b += 0; fl_color(FL_BLACK); fl_rect(a, b, 100, 100);
    fl_color(FL_GREEN);
    fl_rect(a+9, b+19, 33, 63); fl_rect(a+59, b+19, 33, 63);
    fl_color(FL_BLACK);
    fl_arc(a+10, b+20, 31, 61, 0, 360); fl_pie(a+60, b+20, 31, 61, 0, 360);
  }
};

void unittest_fl_circle_test_cb(const char *action, Fl_Group *grp) {
  if ( grp == 0 ) {
    abort();
  } else if ( strcmp(action, "create") == 0 ) {
    grp->begin(); {
      // Create this unit test's widgets
      new CircleTest(grp->x(), grp->y(), grp->w(), grp->h());
    }
    grp->end();
  } else if ( strcmp(action, "show") == 0 ) {
  } else if ( strcmp(action, "hide") == 0 ) {
  }
  return;
}

//
//------- test the point drawing capabilities of this implementation ----------
//
class PointTest : public Fl_Box {
public: PointTest(int x, int y, int w, int h) : Fl_Box(x, y, w, h) {
    label("testing the fl_point call\n"
	  "You should see four pixels each in black, red, green and blue. "
	  "Make sure that pixels are not anti-aliased (blurred across multiple pixels)!");
    align(FL_ALIGN_INSIDE|FL_ALIGN_BOTTOM|FL_ALIGN_LEFT|FL_ALIGN_WRAP);
    box(FL_BORDER_BOX);
  }
  void draw() {
    Fl_Box::draw();
    int a = x()+10, b = y()+10; fl_color(FL_BLACK); fl_rect(a, b, 90, 90);
    fl_point(a+10, b+10); fl_point(a+20, b+20);
    fl_point(a+10, b+20); fl_point(a+20, b+10);
    fl_color(FL_RED); a = x()+70;
    fl_point(a+10, b+10); fl_point(a+20, b+20);
    fl_point(a+10, b+20); fl_point(a+20, b+10);
    fl_color(FL_GREEN); a = x()+10; b = y()+70;
    fl_point(a+10, b+10); fl_point(a+20, b+20);
    fl_point(a+10, b+20); fl_point(a+20, b+10);
    fl_color(FL_BLUE); a = x()+70;
    fl_point(a+10, b+10); fl_point(a+20, b+20);
    fl_point(a+10, b+20); fl_point(a+20, b+10);
  }
};

void unittest_fl_point_test_cb(const char *action, Fl_Group *grp) {
  if ( grp == 0 ) {
    abort();
  } else if ( strcmp(action, "create") == 0 ) {
    grp->begin(); {
      // Create this unit test's widgets
      new PointTest(grp->x(), grp->y(), grp->w(), grp->h());
    }
    grp->end();
  } else if ( strcmp(action, "show") == 0 ) {
  } else if ( strcmp(action, "hide") == 0 ) {
  }
  return;
}

//
//------- test the line drawing capabilities of this implementation ----------
//
class LineTest : public Fl_Box {
public: LineTest(int x, int y, int w, int h) : Fl_Box(x, y, w, h) {
    label("testing the integer based fl_line calls\n"
	  "No red pixels should be visible.\n"
	  "If you see bright red pixels, the line drawing alignment is off,\n"
	  "or the last pixel in a line does not get drawn.\n"
	  "If you see dark red pixels, anti-aliasing must be switched off.");
    align(FL_ALIGN_INSIDE|FL_ALIGN_BOTTOM|FL_ALIGN_LEFT|FL_ALIGN_WRAP);
    box(FL_BORDER_BOX);
  }
  void draw() {
    Fl_Box::draw();
    int a = x()+10, b = y()+10; fl_color(FL_BLACK); fl_rect(a, b, 100, 100);
    // testing fl_xyline(x, y, x1)
    fl_color(FL_RED); fl_point(a+10, b+10); fl_point(a+20, b+10);
    fl_color(FL_BLACK); fl_xyline(a+10, b+10, a+20);
    // testing fl_xyline(x, y, x1, y2);
    fl_color(FL_RED); fl_point(a+10, b+20); fl_point(a+20, b+20);
    fl_point(a+20, b+30);
    fl_color(FL_BLACK); fl_xyline(a+10, b+20, a+20, b+30);
    // testing fl_xyline(x, y, x1, y2, x3);
    fl_color(FL_RED); fl_point(a+10, b+40); fl_point(a+20, b+40);
    fl_point(a+20, b+50); fl_point(a+30, b+50);
    fl_color(FL_BLACK); fl_xyline(a+10, b+40, a+20, b+50, a+30);
    //+++ add testing for the fl_yxline commands!
    // testing fl_loop(x,y, x,y, x,y, x, y)
    fl_color(FL_RED); fl_point(a+60, b+60); fl_point(a+90, b+60);
    fl_point(a+60, b+90); fl_point(a+90, b+90);
    fl_color(FL_BLACK);
    fl_loop(a+60, b+60, a+90, b+60, a+90, b+90, a+60, b+90);
  }
};

void unittest_fl_line_test_cb(const char *action, Fl_Group *grp) {
  if ( grp == 0 ) {
    abort();
  } else if ( strcmp(action, "create") == 0 ) {
    grp->begin(); {
      // Create this unit test's widgets
      new LineTest(grp->x(), grp->y(), grp->w(), grp->h());
    }
    grp->end();
  } else if ( strcmp(action, "show") == 0 ) {
  } else if ( strcmp(action, "hide") == 0 ) {
  }
  return;
}

//
//------- test the rectangle drawing capabilities of this implementation ----------
//
class RectTest : public Fl_Box {
public: RectTest(int x, int y, int w, int h) : Fl_Box(x, y, w, h) {
    label("testing the fl_rect call\n"
	  "No red pixels should be visible. "
	  "If you see bright red lines, or if parts of the green frames are hidden, "
	  "the rect drawing alignment is off.");
    align(FL_ALIGN_INSIDE|FL_ALIGN_BOTTOM|FL_ALIGN_LEFT|FL_ALIGN_WRAP);
    box(FL_BORDER_BOX);
  }
  void draw() {
    Fl_Box::draw();
    int a = x()+10, b = y()+10; fl_color(FL_BLACK); fl_rect(a, b, 100, 100);
    // testing fl_rect() with positive size
    fl_color(FL_RED);   fl_loop(a+10, b+10, a+40, b+10, a+40, b+40, a+10, b+40);
    fl_color(FL_GREEN); fl_loop(a+ 9, b+ 9, a+41, b+ 9, a+41, b+41, a+ 9, b+41);
    fl_color(FL_GREEN); fl_loop(a+11, b+11, a+39, b+11, a+39, b+39, a+11, b+39);
    fl_color(FL_BLACK); fl_rect(a+10, b+10, 31, 31);
    // testing fl_rect() with positive size
    fl_color(FL_RED);   fl_loop(a+60, b+60, a+90, b+60, a+90, b+90, a+60, b+90);
    fl_color(FL_GREEN); fl_loop(a+59, b+59, a+91, b+59, a+91, b+91, a+59, b+91);
    fl_color(FL_BLACK); fl_rectf(a+60, b+60, 31, 31);
  }
};

void unittest_fl_rect_test_cb(const char *action, Fl_Group *grp) {
  if ( grp == 0 ) {
    abort();
  } else if ( strcmp(action, "create") == 0 ) {
    grp->begin(); {
      // Create this unit test's widgets
      new RectTest(grp->x(), grp->y(), grp->w(), grp->h());
    }
    grp->end();
  } else if ( strcmp(action, "show") == 0 ) {
  } else if ( strcmp(action, "hide") == 0 ) {
  }
  return;
}

//
//------- test viewport clipping ----------
//
class ViewportTest : public Fl_Widget {
  int pos;
public: ViewportTest(int x, int y, int w, int h, int p) : Fl_Widget(x, y, w, h), pos(p) { }
  void draw() {
    if (pos&1) {
      fl_color(FL_RED);   fl_yxline(x()+w(), y(), y()+h());
      fl_color(FL_GREEN); fl_yxline(x()+w()-1, y(), y()+h());
    } else {
      fl_color(FL_RED);   fl_yxline(x()-1, y(), y()+h());
      fl_color(FL_GREEN); fl_yxline(x(), y(), y()+h());
    }
    if (pos&2) {
      fl_color(FL_RED);   fl_xyline(x(), y()+h(), x()+w());
      fl_color(FL_GREEN); fl_xyline(x(), y()+h()-1, x()+w());
    } else {
      fl_color(FL_RED);   fl_xyline(x(), y()-1, x()+w());
      fl_color(FL_GREEN); fl_xyline(x(), y(), x()+w());
    }
    fl_color(FL_BLACK);
    fl_loop(x()+3, y()+3, x()+w()-4, y()+3, x()+w()-4, y()+h()-4, x()+3, y()+h()-4);
  }
};

void unittest_viewport_test_cb(const char *action, Fl_Group *grp) {
  static Fl_Window *win = 0;
  if ( grp == 0 ) {
    abort();
  } else if ( strcmp(action, "create") == 0 ) {
    // Create this unit test's widgets
    grp->begin();
    grp->label("See separate window for this test");
    grp->align(FL_ALIGN_INSIDE|FL_ALIGN_CENTER);
    grp->end();
    // MAKE ENTIRELY SEPARATE WINDOW
    win = new Fl_Window(grp->w(), grp->h(), "Viewport Test"); {
      Fl_Box *msg = new Fl_Box(0,0,win->w(),win->h());
      msg->box(FL_NO_BOX);
      msg->label("testing viewport alignment\n"
	          "Only green lines should be visible.\n"
	          "If red lines are visible in the corners of this window,\n"
	          "your viewport alignment and clipping is off.\n"
	          "If there is a space between the green lines and the window border,\n"
	          "the viewport is off, but some clipping may be working.\n"
	          "Also, your window size may be off to begin with.");
      msg->align(FL_ALIGN_INSIDE|FL_ALIGN_CENTER|FL_ALIGN_WRAP);
      new ViewportTest(0,           0,           20, 20, 0);
      new ViewportTest(win->w()-20, 0,           20, 20, 1);
      new ViewportTest(0,           win->h()-20, 20, 20, 2);
      new ViewportTest(win->w()-20, win->h()-20, 20, 20, 3);
      win->resizable(win);
      win->end();
    }
    grp->end();
  } else if ( strcmp(action, "show") == 0 ) {
    if ( win ) win->show();
  } else if ( strcmp(action, "hide") == 0 ) {
    if ( win ) win->hide();
  }
  return;
}

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
public: TextExtentsTest(int x, int y, int w, int h) : Fl_Widget(x, y, w, h) {}
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
      fl_draw("text may leach slightly outside the fl_text_extents()", xx, yy);
    }
    fl_pop_clip(); // remove the local clip
  }
};

void unittest_fl_text_extents_test_cb(const char *action, Fl_Group *grp) {
  if ( grp == 0 ) {
    abort();
  } else if ( strcmp(action, "create") == 0 ) {
    grp->begin(); {
      // Create this unit test's widgets
      new TextExtentsTest(grp->x(), grp->y(), grp->w(), grp->h());
    }
    grp->end();
  } else if ( strcmp(action, "show") == 0 ) {
  } else if ( strcmp(action, "hide") == 0 ) {
  }
  return;
}

//////////////////////////////////////////////////////////////////////////
// UNIT TEST CODE *ENDS* HERE
//////////////////////////////////////////////////////////////////////////

//
// --- SAMPLE UNIT TEST TEMPLATE ---------------------------------------------
//
void unittest_sample_test_cb(const char *action, Fl_Group *grp) {
  if ( grp == 0 ) {
    abort();
  } else if ( strcmp(action, "create") == 0 ) {
    // CREATE YOUR UNIT TEST'S WIDGETS
    //     This is where you create your widgets for the test.
    //     The Fl_Group 'grp' has been created for you.
    //
    grp->begin(); {
      // EXAMPLE: Create a green box.
      Fl_Box *box = new Fl_Box(grp->x(), grp->y(), grp->w(), grp->h(), "Sample Unit Test");
      box->align(FL_ALIGN_CENTER);
      box->box(FL_BORDER_BOX);
      box->color(0x50805000);	// green
    }
    grp->end();
  } else if ( strcmp(action, "show") == 0 ) {
    // YOUR UNIT TEST HAS JUST BEEN SELECTED TO BE SHOWN
  } else if ( strcmp(action, "hide") == 0 ) {
    // YOUR UNIT TEST HAS JUST BEEN HIDDEN
  }
  return;
}

// UNIT TEST MANAGEMENT
class TestInfo {
public:
  const char *name;
  Fl_Group *grp;
  UnitTestCallback cb;
  // CTOR
  TestInfo(const char *name, Fl_Group *grp, UnitTestCallback cb) {
    this->name = name;
    this->grp = grp;
    this->cb = cb;
  }
  void DoCallback(const char *action) {
    if ( ! cb ) return;
    (cb)(action, grp);
  }
};
// START TEST THE USER CLICKED ON
void Browser_CB(Fl_Widget*, void*) {
  for ( int t=1; t<=browser->size(); t++ ) {
    TestInfo *ti = (TestInfo*)browser->data(t);
    if ( browser->selected(t) ) {
      ti->grp->show();
      ti->DoCallback("show");	// show the selected test
    } else {
      ti->grp->hide();
      ti->DoCallback("hide");	// hide the deselected test(s)
    }
  }
}
// ADD A UNIT TEST TO THE BROWSER
void AddTest(const char *testname, UnitTestCallback unit_cb) {
  // Create new group for the test area
  mainwin->begin();
  Fl_Group *grp = new Fl_Group(TESTAREA_X,TESTAREA_Y,TESTAREA_W,TESTAREA_H,testname);
  grp->end();
  grp->hide();
  // New test info instance for this test
  TestInfo *ti = new TestInfo(testname, grp, unit_cb);
  // Add new browser item
  browser->add(testname, (void*)ti);
  // Tell test to create widgets
  ti->DoCallback("create");
}
int main(int argc, char **argv) {
  Fl::args(argc,argv);
  mainwin = new Fl_Window(MAINWIN_W, MAINWIN_H, "Fltk Unit Tests");
  browser = new Fl_Hold_Browser(BROWSER_X, BROWSER_Y, BROWSER_W, BROWSER_H, "Unit Tests");
  browser->align(FL_ALIGN_TOP|FL_ALIGN_LEFT);
  browser->when(FL_WHEN_CHANGED);
  browser->callback(Browser_CB);
  //// START: ADD CALLS TO UNIT TESTS HERE
  ////    List in alphabetical order please..
  ////
  AddTest("fl_circle()",       &unittest_fl_circle_test_cb);
  AddTest("fl_line()",         &unittest_fl_line_test_cb);
  AddTest("fl_point()",        &unittest_fl_point_test_cb);
  AddTest("fl_rect()",         &unittest_fl_rect_test_cb);
  AddTest("fl_text_extents()", &unittest_fl_text_extents_test_cb);
  AddTest("Viewport test",     &unittest_viewport_test_cb);
  AddTest("Sample test",       &unittest_sample_test_cb);
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
