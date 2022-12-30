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

#include <config.h>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>
#if HAVE_GL
#include <FL/Fl_Gl_Window.H>
#endif

//
//------- test the point drawing capabilities of this implementation ----------
//

class Ut_Native_Point_Test : public Fl_Window {
public:
  Ut_Native_Point_Test(int x, int y, int w, int h)
    : Fl_Window(x, y, w, h) {
    end();
  }
  void draw() FL_OVERRIDE {
    int i;
    fl_color(FL_WHITE);
    fl_rectf(0, 0, 10, 10);
    fl_color(FL_BLACK);
    for (i=0; i<10; i++) {
      fl_point(i, 0);
      fl_point(i, 9);
    }
    for (i=0; i<10; i++) {
      fl_point(0, i);
      fl_point(9, i);
    }
  }
};

#if HAVE_GL

class Ut_GL_Point_Test : public Fl_Gl_Window {
public:
  Ut_GL_Point_Test(int x, int y, int w, int h)
    : Fl_Gl_Window(x, y, w, h) {
    box(FL_FLAT_BOX);
    end();
  }
  void draw() FL_OVERRIDE {
    Fl_Gl_Window::draw_begin();
    Fl_Window::draw();

    int a = -24, b = 5-9, i, j;
    // Test 1a: pixel size
    fl_color(FL_WHITE); fl_rectf(a+24, b+9-5, 10, 10);
    fl_color(FL_BLACK);
    for (i=0; i<8; i++)
      for (j=0; j<8; j++)
        if ((i+j)&1)
          fl_point(a+i+24+1, b+j+9-5+1);
    // Test 2a: pixel color
    for (int n=0; n<3; n++) {
      static Fl_Color lut[3] = { FL_RED, FL_GREEN, FL_BLUE };
      int yy = b+9-5+24 + 16*n;
      fl_color(FL_WHITE); fl_rectf(a+24, yy, 10, 10);
      fl_color(lut[n]);
      for (i=0; i<8; i++)
        for (j=0; j<8; j++)
          fl_point(a+i+24+1, yy+j+1);
    }
    // Test 3a: pixel alignment inside windows (drawing happens in PointTestWin)
    int xx = a+24, yy = b+2*24+2*16+9-5;
    fl_color(FL_RED);
    for (i=0; i<10; i++) {
      fl_point(xx-1, yy+i);
      fl_point(xx+10, yy+i);
    }
    fl_color(FL_BLACK);
    for (i=0; i<10; i++) {
      fl_point(xx+i, yy);
      fl_point(xx+i, yy+9);
    }
    for (i=0; i<10; i++) {
      fl_point(xx, yy+i);
      fl_point(xx+9, yy+i);
    }
    fl_color(FL_WHITE);
    for (i=0; i<8; i++)
      for (j=0; j<8; j++)
        fl_point(xx+i+1, yy+j+1);
    // Test 4a: check pixel clipping
    xx = a+24; yy = b+3*24+2*16+9-5;
    fl_push_clip(xx+1, yy+1, 9, 9);
    fl_color(FL_RED);
    for (i=0; i<10; i++) {
      fl_point(xx+i, yy);
      fl_point(xx+i, yy+9);
    }
    for (i=0; i<10; i++) {
      fl_point(xx, yy+i);
      fl_point(xx+9, yy+i);
    }
    fl_color(FL_BLACK);
    for (i=1; i<9; i++) {
      fl_point(xx+i, yy+1);
      fl_point(xx+i, yy+8);
    }
    for (i=1; i<9; i++) {
      fl_point(xx+1, yy+i);
      fl_point(xx+8, yy+i);
    }
    fl_color(FL_WHITE);
    for (i=1; i<7; i++)
      for (j=1; j<7; j++)
        fl_point(xx+i+1, yy+j+1);
    fl_pop_clip();

    Fl_Gl_Window::draw_end();
  }
};

#endif

class Ut_Point_Test : public Fl_Group {
  Ut_Native_Point_Test *align_test_win;
#if HAVE_GL
  Ut_GL_Point_Test *gl_test_win;
#endif
public:
  static Fl_Widget *create() {
    return new Ut_Point_Test(UT_TESTAREA_X, UT_TESTAREA_Y, UT_TESTAREA_W, UT_TESTAREA_H);
    // 520x365, resizable
  }
  Ut_Point_Test(int x, int y, int w, int h)
    : Fl_Group(x, y, w, h) {
    label("Testing the fl_point call.");
    align(FL_ALIGN_INSIDE|FL_ALIGN_BOTTOM|FL_ALIGN_LEFT|FL_ALIGN_WRAP);
    box(FL_BORDER_BOX);
    int a = x+16, b = y+34;
    Fl_Box *t = new Fl_Box(a, b-24, 80, 18, "native");
    t->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);

    t = new Fl_Box(a, b, 18, 18, "1");
    t->box(FL_ROUNDED_BOX); t->color(FL_YELLOW);
    t->tooltip(// Title:
               "Testing pixel size and antialiasing.\n\n"
               // Description:
               "This draws a checker board of black points on a white background.\n\n"
               // Things to look out for:
               "Black and white points should be the same size of one unit (1 pixel in regular mode, 2x2 pixels in hidpi mode)."
               "If black points are smaller than white in hidpi mode, point size must be increased.\n\n"
               "Points should not be blurry. Antialiasing should be switched of and the point coordinates should be centered on the pixel(s).\n\n"
               "If parts of the white border are missing, the size of fl_rect should be adjusted."
               );
    t = new Fl_Box(a, b+24, 18, 18, "2");
    t->box(FL_ROUNDED_BOX); t->color(FL_YELLOW);
    t->tooltip(// Title:
               "Testing pixels color.\n\n"
               // Description:
               "This draws three squares in red, green, and blue.\n\n"
               // Things to look out for:
               "If the order of colors is different, the byte order when writing into the pixel buffer should be fixed."
               );
    t = new Fl_Box(a, b+2*24+2*16, 18, 18, "3");
    t->box(FL_ROUNDED_BOX); t->color(FL_YELLOW);
    t->tooltip(// Title:
               "Testing pixels alignment in windows.\n\n"
               // Description:
               "This draws a black frame around a white square.\n\n"
               // Things to look out for:
               "If parts of the black frame are clipped by the window and not visible, pixel offsets must be adjusted."
               );
    align_test_win = new Ut_Native_Point_Test(a+24, b+2*24+2*16+9-5, 10, 10);

    t = new Fl_Box(a, b+3*24+2*16, 18, 18, "4");
    t->box(FL_ROUNDED_BOX); t->color(FL_YELLOW);
    t->tooltip(// Title:
               "Testing pixels clipping.\n\n"
               // Description:
               "This draws a black frame around a white square.\n\n"
               // Things to look out for:
               "If red pixels are visible or black pixels are missing, graphics clipping is misaligned."
               );

    a+=100;
#if HAVE_GL
    t = new Fl_Box(a, b-24, 80, 18, "OpenGL");
    t->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);

    t = new Fl_Box(a, b, 26, 18, "1a");
    t->box(FL_ROUNDED_BOX); t->color(FL_YELLOW);
    t->tooltip(// Title:
               "Testing pixel size and antialiasing.\n\n"
               // Description:
               "This draws a checker board of black points on a white background.\n\n"
               // Things to look out for:
               "Black and white points should be the same size of one unit (1 pixel in regular mode, 2x2 pixels in hidpi mode)."
               "If black points are smaller than white in hidpi mode, point size must be increased.\n\n"
               "Points should not be blurry. Antialiasing should be switched of and the point coordinates should be centered on the pixel(s).\n\n"
               "If parts of the white border are missing, the size of fl_rect should be adjusted."
               );
    t = new Fl_Box(a, b+24, 26, 18, "2a");
    t->box(FL_ROUNDED_BOX); t->color(FL_YELLOW);
    t->tooltip(// Title:
               "Testing pixels color.\n\n"
               // Description:
               "This draws three squares in red, green, and blue.\n\n"
               // Things to look out for:
               "If the order of colors is different, the color component order when writing into the pixel buffer should be fixed."
               );
    t = new Fl_Box(a, b+2*24+2*16, 26, 18, "3a");
    t->box(FL_ROUNDED_BOX); t->color(FL_YELLOW);
    t->tooltip(// Title:
               "Testing pixels alignment in windows.\n\n"
               // Description:
               "This draws a black frame around a white square, extending to both sides.\n\n"
               // Things to look out for:
               "If parts of the black frame are clipped by the window and not visible, pixel offsets must be adjusted horizontally.\n\n"
               "If the horizontal lines are misaligned, vertical pixel offset should be adjusted."
               );

    t = new Fl_Box(a, b+3*24+2*16, 26, 18, "4a");
    t->box(FL_ROUNDED_BOX); t->color(FL_YELLOW);
    t->tooltip(// Title:
               "Testing pixels clipping.\n\n"
               // Description:
               "This draws a black frame around a white square. The square is slightly smaller.\n\n"
               // Things to look out for:
               "If red pixels are visible or black pixels are missing, graphics clipping is misaligned."
               );

    gl_test_win = new Ut_GL_Point_Test(a+24+8, b+9-5, 10, 4*24+2*16);
#endif

    t = new Fl_Box(x+w-1,y+h-1, 1, 1);
    resizable(t);
  }
  void draw() FL_OVERRIDE {
    Fl_Group::draw();
    int a = x()+16, b = y()+34, i, j;
    // Test 1: pixel size
    fl_color(FL_WHITE); fl_rectf(a+24, b+9-5, 10, 10);
    fl_color(FL_BLACK);
    for (i=0; i<8; i++)
      for (j=0; j<8; j++)
        if ((i+j)&1)
          fl_point(a+i+24+1, b+j+9-5+1);
    // Test 2: pixel color
    for (int n=0; n<3; n++) {
      static Fl_Color lut[3] = { FL_RED, FL_GREEN, FL_BLUE };
      int yy = b+9-5+24 + 16*n;
      fl_color(FL_WHITE); fl_rectf(a+24, yy, 10, 10);
      fl_color(lut[n]);
      for (i=0; i<8; i++)
        for (j=0; j<8; j++)
          fl_point(a+i+24+1, yy+j+1);
    }
    // Test 3: pixel alignment inside windows (drawing happens in PointTestWin)
    // Test 4: check pixel clipping
    int xx = a+24, yy = b+3*24+2*16+9-5;
    fl_push_clip(xx, yy, 10, 10);
    fl_color(FL_RED);
    for (i=-1; i<11; i++) {
      fl_point(xx+i, yy-1);
      fl_point(xx+i, yy+10);
    }
    for (i=-1; i<11; i++) {
      fl_point(xx-1, yy+i);
      fl_point(xx+10, yy+i);
    }
    fl_color(FL_BLACK);
    for (i=0; i<10; i++) {
      fl_point(xx+i, yy);
      fl_point(xx+i, yy+9);
    }
    for (i=0; i<10; i++) {
      fl_point(xx, yy+i);
      fl_point(xx+9, yy+i);
    }
    fl_color(FL_WHITE);
    for (i=0; i<8; i++)
      for (j=0; j<8; j++)
        fl_point(xx+i+1, yy+j+1);
    fl_pop_clip();
    // Test 3a: pixel alignment inside the OpenGL window
#if HAVE_GL
    xx = a+24+108; yy = b+2*24+2*16+9-5;
    fl_color(FL_BLACK);
    for (i=-4; i<14; i++) {
      fl_point(xx+i, yy);
      fl_point(xx+i, yy+9);
    }
#endif
  }
};

UnitTest points(UT_TEST_POINTS, "Drawing Points", Ut_Point_Test::create);
