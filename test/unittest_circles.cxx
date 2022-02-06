//
// Unit tests for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
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
#include <FL/math.h>
#if HAVE_GL
#include <FL/Fl_Gl_Window.H>
#endif

//
// --- test drawing circles and arcs ------
//

void arc(int xi, int yi, int w, int h, double a1, double a2)
{
  if (a2<=a1) return;

  double rx = w/2.0;
  double ry = h/2.0;
  double x = xi + rx + 0.5;
  double y = yi + ry + 0.5;
  double circ = M_PI*0.5*(rx+ry);
  int i, segs = circ * (a2-a1) / 100;
  if (segs<3) segs = 3;

  int px, py;
  a1 = a1/180*M_PI;
  a2 = a2/180*M_PI;
  double step = (a2-a1)/segs;

  int nx = x + cos(a1)*rx;
  int ny = y - sin(a1)*ry;
  fl_point(nx, ny);
  for (i=segs; i>0; i--) {
    a1+=step;
    px = nx; py = ny;
    nx = x + cos(a1)*rx;
    ny = y - sin(a1)*ry;
    //fl_line(px, py, nx, ny);
    fl_point(nx, ny);
  }
}

void draw_circles() {
  int a = 0, b = 0, w=40, h=40;
  // ---- 1: draw a circle and a filled circle
  fl_color(FL_RED);
  arc(a+1, b+1, w-2, h-2, 0.0, 360.0);
  fl_color(FL_GREEN);
  arc(a, b, w, h, 0.0, 360.0);
  arc(a+2, b+2, w-4, h-4, 0.0, 360.0);
  fl_color(FL_BLACK);
  fl_arc(a+1, b+1, w-1, h-1, 0.0, 360.0);
  // ----
  fl_color(FL_RED);
  arc(a+1+50, b+1, w-2, h-2, 0.0, 360.0);
  fl_color(FL_GREEN);
  arc(a+50, b, w, h, 0.0, 360.0);
  fl_color(FL_BLACK);
  fl_pie(a+1+50, b+1, w-1, h-1, 0.0, 360.0);
  b+=44;
  // ---- 2: draw arcs and pies
  fl_color(FL_RED);
//  arc(a-5, b-5, w+10, h+10, 45.0, 315.0);
  arc(a+1, b+1, w-2, h-2, 45.0, 315.0);
//  arc(a+5, b+5, w-10, h-10, 45.0, 315.0);
//  arc(a+10, b+10, w-20, h-20, 45.0, 315.0);
  fl_color(FL_GREEN);
  arc(a, b, w, h, 45.0, 315.0);
  arc(a+2, b+2, w-4, h-4, 45.0, 315.0);
  fl_color(FL_BLACK);
//  fl_arc(a-5, b-5, w+10, h+10, 45.0, 315.0);
  fl_arc(a+1, b+1, w-1, h-1, 45.0, 315.0);
//  fl_arc(a+5, b+5, w-10, h-10, 45.0, 315.0);
//  fl_arc(a+10, b+10, w-20, h-20, 45.0, 315.0);
  fl_color(FL_RED);
  // ----
  arc(a+1+50, b+1, w-2, h-2, 45.0, 315.0);
  fl_line(a+50+20, b+20, a+50+20+14, b+20-14);
  fl_line(a+50+20, b+20, a+50+20+14, b+20+14);
  fl_color(FL_GREEN);
  arc(a+50, b, w, h, 45.0, 315.0);
  fl_line(a+50+21, b+20, a+50+21+14, b+20-14);
  fl_line(a+50+21, b+20, a+50+21+14, b+20+14);
  fl_color(FL_BLACK);
  fl_pie(a+1+50, b+1, w-1, h-1, 45.0, 315.0);
}

#if HAVE_GL

class GLCircleTest : public Fl_Gl_Window {
public:
  GLCircleTest(int x, int y, int w, int h)
  : Fl_Gl_Window(x, y, w, h) {
    box(FL_FLAT_BOX);
  }
  void draw() {
    draw_begin();
    Fl_Window::draw();
    draw_circles();
    draw_end();
  }
};

#endif

class NativeCircleTest : public Fl_Window {
public:
  NativeCircleTest(int x, int y, int w, int h)
  : Fl_Window(x, y, w, h) {
    box(FL_FLAT_BOX);
    end();
  }
  void draw() {
    Fl_Window::draw();
    draw_circles();
  }
};

//
//------- test the circle drawing capabilities of this implementation ----------
//
class CircleTest : public Fl_Group {
public:
  static Fl_Widget *create() {
    return new CircleTest(TESTAREA_X, TESTAREA_Y, TESTAREA_W, TESTAREA_H);
  }
  CircleTest(int x, int y, int w, int h) : Fl_Group(x, y, w, h) {
    label("Testing fast circle, arc, and pie drawing\n\n"
          "No red lines should be visible. "
          "The green outlines should not be overwritten by circle drawings.");
    align(FL_ALIGN_INSIDE|FL_ALIGN_BOTTOM|FL_ALIGN_LEFT|FL_ALIGN_WRAP);
    box(FL_BORDER_BOX);

    int a = x+16, b = y+34;
    Fl_Box *t = new Fl_Box(a, b-24, 80, 18, "native");
    t->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);

    /* NativeCircleTest *nr = */ new NativeCircleTest(a+23, b-1, 200, 200);

    t = new Fl_Box(a, b, 18, 18, "1");
    t->box(FL_ROUNDED_BOX); t->color(FL_YELLOW);
    t->tooltip(// Title:
               "Testing circle alignment.\n\n"
               // Description:
               "This draws a black circle and a black disc, surrounded by a green frame.\n\n"
               // Things to look out for:
               "If green pixels are missing, circle drawing must be adjusted (see fl_arc, fl_pie).\n\n"
               "If red pixels are showing, line width or aligment may be off."
               );
    b+=44;
    t = new Fl_Box(a, b, 18, 18, "2");
    t->box(FL_ROUNDED_BOX); t->color(FL_YELLOW);
    t->tooltip(// Title:
               "Testing arc and pie drawing.\n\n"
               // Description:
               "This draws a black frame, surrounded on the inside and outside by a green frame.\n\n"
               // Things to look out for:
               "If green pixels are missing or red pixels are showing, rectangular frame drawing schould be adjusted (see fl_rect).\n\n"
               "If red pixels show in the corners of the frame in hidpi mode, line endings should be adjusted."
               );

#if HAVE_GL

    a = x+16+250, b = y+34;
    t = new Fl_Box(a, b-24, 80, 18, "OpenGL");
    t->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);

    /* GLCircleTest *glr = */ new GLCircleTest(a+31, b-1, 200, 200);

    t = new Fl_Box(a, b, 26, 18, "1a");
    t->box(FL_ROUNDED_BOX); t->color(FL_YELLOW);
    t->tooltip(// Title:
               "Testing circle alignment.\n\n"
               // Description:
               "This draws a black circle and a black disc, surrounded by a green frame.\n\n"
               // Things to look out for:
               "If green pixels are missing, circle drawing must be adjusted (see fl_arc, fl_pie).\n\n"
               "If red pixels are showing, line width or aligment may be off."
               );
    b+=44;
    t = new Fl_Box(a, b, 26, 18, "2a");
    t->box(FL_ROUNDED_BOX); t->color(FL_YELLOW);
    t->tooltip(// Title:
               "Testing arc and pie drawing.\n\n"
               // Description:
               "This draws a black frame, surrounded on the inside and outside by a green frame.\n\n"
               // Things to look out for:
               "If green pixels are missing or red pixels are showing, rectangular frame drawing schould be adjusted (see fl_rect).\n\n"
               "If red pixels show in the corners of the frame in hidpi mode, line endings should be adjusted."
               );
#endif

    t = new Fl_Box(x+w-1,y+h-1, 1, 1);
    resizable(t);
  }
};

UnitTest circle(kTestCircles, "Circles and Arcs", CircleTest::create);
