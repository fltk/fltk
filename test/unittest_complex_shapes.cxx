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

#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>

#if 0

// TODO:
void fl_push_matrix()
void fl_pop_matrix()

void fl_scale(double x,double y)
void fl_scale(double x)
void fl_translate(double x,double y)
void fl_rotate(double d)
void fl_mult_matrix(double a,double b,double c,double d,double x,double y)

double fl_transform_x(double x, double y)
double fl_transform_y(double x, double y)
double fl_transform_dx(double x, double y)
double fl_transform_dy(double x, double y)
void fl_transformed_vertex(double xf, double yf)

void fl_begin_points()
void fl_end_points()

void fl_begin_line()
void fl_end_line()

void fl_begin_loop()
void fl_end_loop()

void fl_begin_polygon()
void fl_end_polygon()

void fl_begin_complex_polygon()
void fl_gap()
void fl_end_complex_polygon()

void fl_vertex(double x,double y)

void fl_curve(double X0, double Y0, double X1, double Y1, double X2, double Y2, double X3, double Y3)

void fl_arc(double x, double y, double r, double start, double end)
void fl_circle(double x, double y, double r)


#endif

//
//------- test Complex Shape drawing capabilities of this implementation ----------
//
class ComplexShapesTest : public Fl_Box {
public:
  static Fl_Widget *create() {
    return new ComplexShapesTest(TESTAREA_X, TESTAREA_Y, TESTAREA_W, TESTAREA_H);
  }
  ComplexShapesTest(int x, int y, int w, int h) : Fl_Box(x, y, w, h) {
    label("Testing complex shape drawing.\n\n"
          "Complex Shapes in FLTK are rendered using floating point coordinates "
          "which can be transformed through a matrix.");
    align(FL_ALIGN_INSIDE|FL_ALIGN_BOTTOM|FL_ALIGN_LEFT|FL_ALIGN_WRAP);
    box(FL_BORDER_BOX);
  }
  void draw() {
    Fl_Box::draw();

    int i, a = x()+10, b = y()+10; fl_color(FL_BLACK); fl_rect(a, b, 100, 100);
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

    a = x()+120, b = y()+10; fl_color(FL_BLACK); fl_rect(a, b, 203, 203);
    a += 101; b += 101;
    fl_color(0xff888800);
    for (i=-80; i<=80; i+=20) fl_line(a, b, a+i, b-100);
    fl_color(0xff444400);
    for (i=-80; i<=80; i+=20) fl_line(a, b, a+i, b+100);
    fl_color(0x88ff8800);
    for (i=-80; i<=80; i+=20) fl_line(a, b, a-100, b+i);
    fl_color(0x44ff4400);
    for (i=-80; i<=80; i+=20) fl_line(a, b, a+100, b+i);
    fl_color(0x8888ff00);
    fl_line(a, b, a-100, b-100);
    fl_line(a, b, a+100, b-100);
    fl_line(a, b, a+100, b+100);
    fl_line(a, b, a-100, b+100);
  }
};

//UnitTest lines(kTestComplexShapes, "Complex Shapes", ComplexShapesTest::create);
