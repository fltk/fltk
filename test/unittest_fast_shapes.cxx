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
#include <FL/fl_draw.H>         // fl_text_extents()
#if HAVE_GL
#include <FL/Fl_Gl_Window.H>
#endif

#if 0

// not testing yet:
void fl_line(int x, int y, int x1, int y1)
void fl_line(int x, int y, int x1, int y1, int x2, int y2)

Draw one or two lines between the given points.
void fl_loop(int x, int y, int x1, int y1, int x2, int y2)
void fl_loop(int x, int y, int x1, int y1, int x2, int y2, int x3, int y3)

Outline a 3 or 4-sided polygon with lines.
void fl_polygon(int x, int y, int x1, int y1, int x2, int y2)
void fl_polygon(int x, int y, int x1, int y1, int x2, int y2, int x3, int y3)

Draw vertical and horizontal lines. A vertical line is drawn first, then a horizontal, then a vertical.

#endif

//
// --- test drawing shapes that are not transformed by the drawing matrix ------
//

void draw_fast_shapes() {
  int a = 0, b = 0, i;
  // 1: draw a filled rectangle (fl_rectf)
  fl_color(FL_GREEN);
  for (i=0; i<=40; i++) { fl_point(a+i, b); fl_point(a+i, b+20); }
  for (i=0; i<=20; i++) { fl_point(a, b+i); fl_point(a+40, b+i); }
  fl_color(FL_RED);
  for (i=1; i<=39; i++) { fl_point(a+i, b+1); fl_point(a+i, b+19); }
  for (i=1; i<=19; i++) { fl_point(a+1, b+i); fl_point(a+39, b+i); }
  fl_color(FL_BLACK);
  fl_rectf(a+1, b+1, 39, 19);
  // 2: draw a one units wide frame
  b+=24;
  fl_color(FL_GREEN);
  for (i=0; i<=40; i++) { fl_point(a+i, b); fl_point(a+i, b+20); }
  for (i=0; i<=20; i++) { fl_point(a, b+i); fl_point(a+40, b+i); }
  fl_color(FL_GREEN);
  for (i=2; i<=38; i++) { fl_point(a+i, b+2); fl_point(a+i, b+18); }
  for (i=2; i<=18; i++) { fl_point(a+2, b+i); fl_point(a+38, b+i); }
  fl_color(FL_RED);
  for (i=1; i<=39; i++) { fl_point(a+i, b+1); fl_point(a+i, b+19); }
  for (i=1; i<=19; i++) { fl_point(a+1, b+i); fl_point(a+39, b+i); }
  fl_color(FL_BLACK);
  fl_rect(a+1, b+1, 39, 19);
  // 3: draw a three units wide frame
  b+=24;
  fl_color(FL_GREEN);
  fl_rect(a, b, 41, 21);
  fl_rect(a+4, b+4, 33, 13);
  fl_color(FL_RED);
  fl_rect(a+1, b+1, 39, 19);
  fl_rect(a+3, b+3, 35, 15);
  fl_color(FL_BLACK);
  fl_line_style(FL_SOLID, 3);
  fl_rect(a+2, b+2, 37, 17);
  fl_line_style(FL_SOLID, 1);
  // 4: draw fl_xyline
  b+=24;
  fl_color(FL_GREEN);
  fl_rect(a, b+8, 41, 3);   // single line
  fl_rect(a+45, b, 41, 3);  // horizontal, then vertical line
  fl_rect(a+83, b, 3, 21);
  fl_rect(a+90, b, 21, 3);  // horizontal, vertical, horizontal line
  fl_rect(a+109, b, 3, 21);
  fl_rect(a+109, b+18, 21, 3);
  fl_color(FL_RED);
  fl_rectf(a+1, b+9, 39, 1);  // single line
  fl_rectf(a+46, b+1, 39, 1); // two lines
  fl_rectf(a+84, b+1, 1, 19);
  fl_rectf(a+91, b+1, 20, 1); // three lines
  fl_rectf(a+110, b+1, 1, 19);
  fl_rectf(a+110, b+19, 19, 1); // three lines
  fl_color(FL_BLACK);
  fl_xyline(a+1, b+9, a+39);
  fl_xyline(a+46, b+1, a+84, b+19);
  fl_xyline(a+91, b+1, a+110, b+19, a+128);
  b+=24;
  fl_color(FL_GREEN);
  fl_rect(a, b+7, 41, 5);   // single line
  fl_rect(a+45, b, 41, 5);  // horizontal, then vertical line
  fl_rect(a+81, b, 5, 21);
  fl_rect(a+90, b, 22, 5);  // horizontal, vertical, horizontal line
  fl_rect(a+108, b, 5, 21);
  fl_rect(a+108, b+16, 22, 5);
  fl_color(FL_RED);
  fl_rectf(a+1, b+8, 39, 3);  // single line
  fl_rectf(a+46, b+1, 39, 3); // two lines
  fl_rectf(a+82, b+1, 3, 19);
  fl_rectf(a+91, b+1, 20, 3); // three lines
  fl_rectf(a+109, b+1, 3, 19);
  fl_rectf(a+109, b+17, 20, 3); // three lines
  fl_color(FL_BLACK);
  fl_line_style(FL_SOLID, 3);
  fl_xyline(a+1, b+9, a+39);
  fl_xyline(a+46, b+2, a+83, b+19);
  fl_xyline(a+91, b+2, a+110, b+18, a+128);
  fl_line_style(FL_SOLID, 1);
  // 5: draw fl_xyline
  b+=24;
  fl_color(FL_GREEN);
  fl_rect(a+9, b, 3, 21);   // single line
  fl_rect(a+45, b, 3, 21);  // horizontal, then vertical line
  fl_rect(a+45, b+18, 41, 3);
  fl_rect(a+90, b, 3, 11);  // horizontal, vertical, horizontal line
  fl_rect(a+90, b+9, 40, 3);
  fl_rect(a+127, b+9, 3, 12);
  fl_color(FL_RED);
  fl_rectf(a+10, b+1, 1, 19);  // single line
  fl_rectf(a+46, b+1, 1, 19); // two lines
  fl_rectf(a+46, b+19, 39, 1);
  fl_rectf(a+91, b+1, 1, 10); // three lines
  fl_rectf(a+91, b+10, 38, 1);
  fl_rectf(a+128, b+10, 1, 10); // three lines
  fl_color(FL_BLACK);
  fl_yxline(a+10, b+1, b+19);
  fl_yxline(a+46, b+1, b+19, a+84);
  fl_yxline(a+91, b+1, b+10, a+128, b+19);
  b+=24;
  fl_color(FL_GREEN);
  fl_rect(a+8, b, 5, 21);   // single line
  fl_rect(a+45, b, 5, 21);  // horizontal, then vertical line
  fl_rect(a+45, b+16, 41, 5);
  fl_rect(a+90, b, 5, 11);  // horizontal, vertical, horizontal line
  fl_rect(a+90, b+8, 40, 5);
  fl_rect(a+125, b+8, 5, 13);
  fl_color(FL_RED);
  fl_rectf(a+9, b+1, 3, 19);  // single line
  fl_rectf(a+46, b+1, 3, 19); // two lines
  fl_rectf(a+46, b+17, 39, 3);
  fl_rectf(a+91, b+1, 3, 10); // three lines
  fl_rectf(a+91, b+9, 38, 3);
  fl_rectf(a+126, b+9, 3, 11); // three lines
  fl_color(FL_BLACK);
  fl_line_style(FL_SOLID, 3);
  fl_yxline(a+10, b+1, b+19);
  fl_yxline(a+47, b+1, b+18, a+84);
  fl_yxline(a+92, b+1, b+10, a+127, b+19);
  fl_line_style(FL_SOLID, 1);
  // 6: fast diagonal lines
  b+=24;
  fl_color(FL_GREEN);
  fl_point(a, b); fl_point(a+1, b); fl_point(a, b+1);
  fl_point(a+20, b+20); fl_point(a+19, b+20); fl_point(a+20, b+19);
  fl_color(FL_RED);
  fl_point(a+1, b+1); fl_point(a+19, b+19);
  fl_color(FL_BLACK);
  fl_line(a+1, b+1, a+19, b+19);
  fl_color(FL_GREEN);
  fl_point(a+25+1, b); fl_point(a+25, b+1); fl_point(a+25+4, b); fl_point(a+25, b+4);
  fl_point(a+25+20, b+19); fl_point(a+25+19, b+20); fl_point(a+25+16, b+20); fl_point(a+25+20, b+16);
  fl_color(FL_RED);
  fl_point(a+25+2, b+2); fl_point(a+25+18, b+18);
  fl_color(FL_BLACK);
  fl_line_style(FL_SOLID, 5);
  fl_line(a+25+1, b+1, a+25+19, b+19);
  fl_line(a+50+1, b+1, a+50+20, b+20, a+50+39, b+1);
  fl_line_style(FL_SOLID, 1);
}

#if HAVE_GL

class Ut_GL_Rect_Test : public Fl_Gl_Window {
public:
  Ut_GL_Rect_Test(int x, int y, int w, int h)
    : Fl_Gl_Window(x, y, w, h) {
    box(FL_FLAT_BOX);
  }
  void draw() FL_OVERRIDE {
    draw_begin();
    fl_color(color());
    fl_rectf(0, 0, w(), h());
    draw_fast_shapes();
    draw_end();
  }
};

#endif

class Ut_Native_Rect_Test : public Fl_Window {
public:
  Ut_Native_Rect_Test(int x, int y, int w, int h)
    : Fl_Window(x, y, w, h) {
    box(FL_FLAT_BOX);
    end();
  }
  void draw() FL_OVERRIDE {
    Fl_Window::draw();
    draw_fast_shapes();
  }
};

class Ut_Rect_Test : public Fl_Group { // 520 x 365
public:
  static Fl_Widget *create() {
    return new Ut_Rect_Test(UT_TESTAREA_X, UT_TESTAREA_Y, UT_TESTAREA_W, UT_TESTAREA_H);
  }
  Ut_Rect_Test(int x, int y, int w, int h)
    : Fl_Group(x, y, w, h) {
    label("Testing FLTK fast shape calls.\n"
          "These calls draw horizontal and vertical lines, frames, and rectangles.\n\n"
          "No red pixels should be visible. "
          "If you see bright red lines, or if parts of the green frames are hidden, "
          "drawing alignment is off.");
    align(FL_ALIGN_INSIDE|FL_ALIGN_BOTTOM|FL_ALIGN_LEFT|FL_ALIGN_WRAP);
    box(FL_BORDER_BOX);

    int a = x+16, b = y+34;
    Fl_Box *t = new Fl_Box(a, b-24, 80, 18, "native");
    t->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);

    /* NativeRectTest *nr = */ new Ut_Native_Rect_Test(a+23, b-1, 200, 200);

    t = new Fl_Box(a, b, 18, 18, "1");
    t->box(FL_ROUNDED_BOX); t->color(FL_YELLOW);
    t->tooltip(// Title:
               "Testing filled rectangle alignment.\n\n"
               // Description:
               "This draws a black rectangle, surrounded by a green frame.\n\n"
               // Things to look out for:
               "If green pixels are missing, filled rectangles draw too big (see fl_rectf).\n\n"
               "If red pixels are showing, filled rectangles are drawn too small."
               );
    b+=24;
    t = new Fl_Box(a, b, 18, 18, "2");
    t->box(FL_ROUNDED_BOX); t->color(FL_YELLOW);
    t->tooltip(// Title:
               "Testing rectangle frame alignment.\n\n"
               // Description:
               "This draws a black frame, surrounded on the inside and outside by a green frame.\n\n"
               // Things to look out for:
               "If green pixels are missing or red pixels are showing, rectangular frame drawing schould be adjusted (see fl_rect).\n\n"
               "If red pixels show in the corners of the frame in hidpi mode, line endings should be adjusted."
               );
    b+=24;
    t = new Fl_Box(a, b, 18, 18, "3");
    t->box(FL_ROUNDED_BOX); t->color(FL_YELLOW);
    t->tooltip(// Title:
               "Testing scaled frame alignment.\n\n"
               // Description:
               "This draws a 3 units wide black frame, surrounded on the inside and outside by a green frame.\n\n"
               // Things to look out for:
               "If green pixels are missing or red pixels are showing, line width schould be adjusted (see fl_line_style).\n\n"
               "If red pixels show in the corners of the frame in hidpi mode, line endings should be adjusted."
               );
    b+=24;
    t = new Fl_Box(a, b, 18, 18, "4");
    t->box(FL_ROUNDED_BOX); t->color(FL_YELLOW);
    t->tooltip(// Title:
               "Testing fl_xyline.\n\n"
               // Description:
               "This draws 3 versions of fl_xyline surronded with a green outline.\n\n"
               // Things to look out for:
               "If green pixels are missing or red pixels are showing, fl_xyline must be adjusted."
               );
    b+=48;
    t = new Fl_Box(a, b, 18, 18, "5");
    t->box(FL_ROUNDED_BOX); t->color(FL_YELLOW);
    t->tooltip(// Title:
               "Testing fl_yxline.\n\n"
               // Description:
               "This draws 3 versions of fl_yxline surronded with a green outline.\n\n"
               // Things to look out for:
               "If green pixels are missing or red pixels are showing, fl_yxline must be adjusted."
               );
    b+=48;
    t = new Fl_Box(a, b, 18, 18, "6");
    t->box(FL_ROUNDED_BOX); t->color(FL_YELLOW);
    t->tooltip(// Title:
               "Testing fl_line(int...).\n\n"
               // Description:
               "This draws 2 lines at differnet widths, and one connected line.\n\n"
               // Things to look out for:
               "Green and red pixels mark the beginning and end of single lines."
               "The line caps should be flat, the joints should be of type \"miter\"."
               );

#if HAVE_GL

    a = x+16+250, b = y+34;
    t = new Fl_Box(a, b-24, 80, 18, "OpenGL");
    t->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);

    /*GLRectTest *glr = */ new Ut_GL_Rect_Test(a+31, b-1, 200, 200);

    t = new Fl_Box(a, b, 26, 18, "1a");
    t->box(FL_ROUNDED_BOX); t->color(FL_YELLOW);
    t->tooltip(// Title:
               "Testing filled rectangle alignment.\n\n"
               // Description:
               "This draws a black rectangle, surrounded by a green frame.\n\n"
               // Things to look out for:
               "If green pixels are missing, filled rectangles draw too big (see fl_rectf).\n\n"
               "If red pixels are showing, filled rectangles are drawn too small."
               );

    b+=24;
    t = new Fl_Box(a, b, 26, 18, "2a");
    t->box(FL_ROUNDED_BOX); t->color(FL_YELLOW);
    t->tooltip(// Title:
               "Testing rectangle frame alignment.\n\n"
               // Description:
               "This draws a black frame, surrounded on the inside and outside by a green frame.\n\n"
               // Things to look out for:
               "If green pixels are missing or red pixels are showing, rectangular frame drawing schould be adjusted (see fl_rect).\n\n"
               "If red pixels show in the corners of the frame in hidpi mode, line endings should be adjusted."
               );

    b+=24;
    t = new Fl_Box(a, b, 26, 18, "3a");
    t->box(FL_ROUNDED_BOX); t->color(FL_YELLOW);
    t->tooltip(// Title:
               "Testing scaled frame alignment.\n\n"
               // Description:
               "This draws a 3 units wide black frame, surrounded on the inside and outside by a green frame.\n\n"
               // Things to look out for:
               "If green pixels are missing or red pixels are showing, line width schould be adjusted (see fl_line_style).\n\n"
               "If red pixels show in the corners of the frame in hidpi mode, line endings should be adjusted."
               );
    b+=24;
    t = new Fl_Box(a, b, 26, 18, "4a");
    t->box(FL_ROUNDED_BOX); t->color(FL_YELLOW);
    t->tooltip(// Title:
               "Testing fl_xyline.\n\n"
               // Description:
               "This draws 3 versions of fl_xyline surronded with a green outline.\n\n"
               // Things to look out for:
               "If green pixels are missing or red pixels are showing, fl_xyline must be adjusted."
               );
    b+=48;
    t = new Fl_Box(a, b, 26, 18, "5a");
    t->box(FL_ROUNDED_BOX); t->color(FL_YELLOW);
    t->tooltip(// Title:
               "Testing fl_yxline.\n\n"
               // Description:
               "This draws 3 versions of fl_yxline surronded with a green outline.\n\n"
               // Things to look out for:
               "If green pixels are missing or red pixels are showing, fl_yxline must be adjusted."
               );
    b+=48;
    t = new Fl_Box(a, b, 26, 18, "6a");
    t->box(FL_ROUNDED_BOX); t->color(FL_YELLOW);
    t->tooltip(// Title:
               "Testing fl_line(int...).\n\n"
               // Description:
               "This draws 2 lines at differnet widths, and one connected line.\n\n"
               // Things to look out for:
               "Green and red pixels mark the beginning and end of single lines."
               "The line caps should be flat, the joints should be of type \"miter\"."
               );
#endif

    t = new Fl_Box(x+w-1,y+h-1, 1, 1);
    resizable(t);
  }
};

UnitTest rects(UT_TEST_FAST_SHAPES, "Fast Shapes", Ut_Rect_Test::create);
