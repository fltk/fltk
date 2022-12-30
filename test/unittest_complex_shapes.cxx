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
#include <FL/Fl_Hor_Value_Slider.H>
#include <FL/Fl_Dial.H>
#include <FL/Fl_Positioner.H>
#include <FL/fl_draw.H>

#if HAVE_GL
#include <FL/Fl_Gl_Window.H>
#endif

//
// --- test drawing circles and arcs ------
//

class Ut_Complex_Shapes_Test;

void draw_complex(Ut_Complex_Shapes_Test *p);

#if HAVE_GL

class Ut_GL_Complex_Shapes_Test : public Fl_Gl_Window {
public:
  Ut_GL_Complex_Shapes_Test(int x, int y, int w, int h)
    : Fl_Gl_Window(x, y, w, h) {
    box(FL_FLAT_BOX);
    end();
  }
  void draw() FL_OVERRIDE {
    draw_begin();
    Fl_Window::draw();
    draw_complex((Ut_Complex_Shapes_Test*)parent());
    draw_end();
  }
};

#endif

class Ut_Native_Complex_Shapes_Test : public Fl_Window {
public:
  Ut_Native_Complex_Shapes_Test(int x, int y, int w, int h)
    : Fl_Window(x, y, w, h) {
    box(FL_FLAT_BOX);
    end();
  }
  void draw() FL_OVERRIDE {
    Fl_Window::draw();
    draw_complex((Ut_Complex_Shapes_Test*)parent());
  }
};

//
//------- test the compelx shape drawing capabilities of this implementation ----------
//
class Ut_Complex_Shapes_Test : public Fl_Group {
  Ut_Native_Complex_Shapes_Test* native_test_window;
#if HAVE_GL
  Ut_GL_Complex_Shapes_Test* gl_test_window;
#endif
  static void update_cb(Fl_Widget*, void *v) {
    Ut_Complex_Shapes_Test* This = (Ut_Complex_Shapes_Test*)v;
    This->native_test_window->redraw();
#if HAVE_GL
    This->gl_test_window->redraw();
#endif
  }
public:
  Fl_Hor_Value_Slider* scale;
  Fl_Dial* rotate;
  Fl_Positioner* position;
  void set_transformation() {
    fl_translate(position->xvalue(), position->yvalue());
    fl_rotate(-rotate->value());
    fl_scale(scale->value(), scale->value());
  }
  static Fl_Widget* create() {
    return new Ut_Complex_Shapes_Test(UT_TESTAREA_X, UT_TESTAREA_Y, UT_TESTAREA_W, UT_TESTAREA_H);
  }
  Ut_Complex_Shapes_Test(int x, int y, int w, int h) : Fl_Group(x, y, w, h) {
    label("Testing complex shape drawing.");
    align(FL_ALIGN_INSIDE|FL_ALIGN_BOTTOM|FL_ALIGN_LEFT|FL_ALIGN_WRAP);
    box(FL_BORDER_BOX);

    int a = x+16, b = y+34;
    Fl_Box *t = new Fl_Box(a, b-24, 80, 18, "native");
    t->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);

    native_test_window = new Ut_Native_Complex_Shapes_Test(a+23, b-1, 200, 200);

    t = new Fl_Box(a, b, 18, 18, "1");
    t->box(FL_ROUNDED_BOX); t->color(FL_YELLOW);
    t->tooltip(// Title:
               "Testing complex drawing with transformations.\n\n"
               // Description:
               "Draw a point pattern, an open line, a closed line, and a covenx polygon.\n\n"
               // Things to look out for:
               "Use the controls at the bottom right to scale, rotate, and move the patterns."
               );
    b+=44;
    t = new Fl_Box(a, b, 18, 18, "2");
    t->box(FL_ROUNDED_BOX); t->color(FL_YELLOW);
    t->tooltip(// Title:
               "Testing complex polygons.\n\n"
               // Description:
               "Draw polygons at different leves of complexity. "
               "All polygons should be within the blue boundaries\n\n"
               // Things to look out for:
               "1: a convex polygon\n"
               "2: a non-convex polygon\n"
               "3: two polygons in a single operation\n"
               "4: a polygon with a square hole in it"
               );
    b+=44;
    t = new Fl_Box(a, b, 18, 18, "3");
    t->box(FL_ROUNDED_BOX); t->color(FL_YELLOW);
    t->tooltip(// Title:
               "Testing complex polygons with arcs.\n\n"
               // Description:
               "Draw polygons with an arc section. "
               "All polygons should be within the blue boundaries\n\n"
               // Things to look out for:
               "1: a polygon with a camel hump\n"
               "2: a polygon with a camel dip"
               );
#if HAVE_GL

    a = x+16+250, b = y+34;
    t = new Fl_Box(a, b-24, 80, 18, "OpenGL");
    t->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);

    gl_test_window = new Ut_GL_Complex_Shapes_Test(a+31, b-1, 200, 200);

    t = new Fl_Box(a, b, 26, 18, "1a");
    t->box(FL_ROUNDED_BOX); t->color(FL_YELLOW);
    t->tooltip(// Title:
               "Testing complex drawing with transformations.\n\n"
               // Description:
               "Draw a point pattern, an open line, a closed line, and a convex polygon.\n\n"
               // Things to look out for:
               "Use the controls at the bottom right to scale, rotate, and move the patterns."
               );
    b+=44;
    t = new Fl_Box(a, b, 28, 18, "2a");
    t->box(FL_ROUNDED_BOX); t->color(FL_YELLOW);
    t->tooltip(// Title:
               "Testing complex polygons.\n\n"
               // Description:
               "Draw polygons at different leves of complexity. "
               "All polygons should be within the blue boundaries\n\n"
               // Things to look out for:
               "1: a convex polygon\n"
               "2: a non-convex polygon\n"
               "3: two polygons in a single operation\n"
               "4: a polygon with a square hole in it"
               );
    b+=44;
    t = new Fl_Box(a, b, 28, 18, "3a");
    t->box(FL_ROUNDED_BOX); t->color(FL_YELLOW);
    t->tooltip(// Title:
               "Testing complex polygons with arcs.\n\n"
               // Description:
               "Draw polygons with an arc section. "
               "All polygons should be within the blue boundaries\n\n"
               // Things to look out for:
               "1: a polygon with a camel hump\n"
               "2: a polygon with a camel dip"
               );
#endif

    a = UT_TESTAREA_X+UT_TESTAREA_W-250;
    b = UT_TESTAREA_Y+UT_TESTAREA_H-50;

    scale = new Fl_Hor_Value_Slider(a, b+10, 120, 20, "Scale:");
    scale->align(FL_ALIGN_TOP_LEFT);
    scale->range(0.8, 1.2);
    scale->value(1.0);
    scale->callback(update_cb, this);

    rotate = new Fl_Dial(a+140, b, 40, 40, "Rotate:");
    rotate->align(FL_ALIGN_TOP_LEFT);
    rotate->angles(0, 360);
    rotate->range(-180.0, 180.0);
    rotate->value(0.0);
    rotate->callback(update_cb, this);

    position = new Fl_Positioner(a+200, b, 40, 40, "Offset:");
    position->align(FL_ALIGN_TOP_LEFT);
    position->xbounds(-10, 10);
    position->ybounds(-10, 10);
    position->value(0.0, 0.0);
    position->callback(update_cb, this);

    t = new Fl_Box(a-1, b-1, 1, 1);
    resizable(t);
  }
};

void convex_shape(int w, int h) {
  fl_vertex(-w/2, -h);
  fl_vertex(w/2, -h);
  fl_vertex(w, 0);
  fl_vertex(w, h);
  fl_vertex(0, h);
  fl_vertex(-w, h/2);
  fl_vertex(-w, -h/2);
}

void complex_shape(int w, int h) {
  fl_vertex(-w/2, -h);
  fl_vertex(0, -h/2);
  fl_vertex(w/2, -h);
  fl_vertex(w, 0);
  fl_vertex(w, h);
  fl_vertex(0, h);
  fl_vertex(-w, h/2);
  fl_vertex(-w/2, 0);
  fl_vertex(-w, -h/2);
}

void two_complex_shapes(int w, int h) {
  fl_vertex(-w/2, -h);
  fl_vertex(w/2, -h);
  fl_vertex(w, 0);
  fl_vertex(w, h-3);
  fl_gap();
  fl_vertex(w-3, h);
  fl_vertex(0, h);
  fl_vertex(-w, h/2);
  fl_vertex(-w, -h/2);
}

void complex_shape_with_hole(int w, int h) {
  int w2 = w/3, h2 = h/3;
  // clockwise
  fl_vertex(-w/2, -h);
  fl_vertex(w/2, -h);
  fl_vertex(w, 0);
  fl_vertex(w, h);
  fl_vertex(0, h);
  fl_vertex(-w, h/2);
  fl_vertex(-w, -h/2);
  fl_gap();
  // counterclockwise
  fl_vertex(-w2, -h2);
  fl_vertex(-w2,  h2);
  fl_vertex( w2,  h2);
  fl_vertex( w2, -h2);
}

void draw_complex(Ut_Complex_Shapes_Test *p) {
  int a = 0, b = 0, dx = 20, dy = 20, w = 10, h = 10;
  int w2 = w/3, h2 = h/3;
  // ---- 1: draw a random shape
  fl_color(FL_BLACK);
  // -- points
  fl_push_matrix();
  fl_translate(a+dx, b+dy);
  p->set_transformation();
  fl_begin_points();
  convex_shape(w, h);
  fl_end_points();
  fl_pop_matrix();
  // -- lines
  fl_push_matrix();
  fl_translate(a+dx+50, b+dy);
  p->set_transformation();
  fl_begin_line();
  convex_shape(w, h);
  fl_end_line();
  fl_pop_matrix();
  // -- line loop
  fl_push_matrix();
  fl_translate(a+dx+100, b+dy);
  p->set_transformation();
  fl_begin_loop();
  convex_shape(w, h);
  fl_end_loop();
  fl_pop_matrix();
  // -- polygon
  fl_push_matrix();
  fl_translate(a+dx+150, b+dy);
  p->set_transformation();
  fl_begin_polygon();
  convex_shape(w, h);
  fl_end_polygon();
  fl_pop_matrix();

  // ---- 2: draw a complex shape
  b += 44;
  // -- covex polygon drawn in complex mode
  fl_push_matrix();
  fl_translate(a+dx, b+dy);
  p->set_transformation();
  fl_color(FL_DARK2);
  fl_begin_complex_polygon();
  convex_shape(w, h);
  fl_end_complex_polygon();
  fl_color(FL_BLUE);
  fl_begin_loop();
  convex_shape(w, h);
  fl_end_loop();
  fl_pop_matrix();
  // -- non-convex polygon drawn in complex mode
  fl_push_matrix();
  fl_translate(a+dx+50, b+dy);
  p->set_transformation();
  fl_color(FL_DARK2);
  fl_begin_complex_polygon();
  complex_shape(w, h);
  fl_end_complex_polygon();
  fl_color(FL_BLUE);
  fl_begin_loop();
  complex_shape(w, h);
  fl_end_loop();
  fl_pop_matrix();
  // -- two part polygon with gap
  fl_push_matrix();
  fl_translate(a+dx+100, b+dy);
  p->set_transformation();
  fl_color(FL_DARK2);
  fl_begin_complex_polygon();
  two_complex_shapes(w, h);
  fl_end_complex_polygon();
  fl_color(FL_BLUE);
  fl_begin_loop();
  fl_vertex(-w/2, -h);
  fl_vertex(w/2, -h);
  fl_vertex(w, 0);
  fl_vertex(w, h-3);
  fl_end_loop();
  fl_begin_loop();
  fl_vertex(w-3, h);
  fl_vertex(0, h);
  fl_vertex(-w, h/2);
  fl_vertex(-w, -h/2);
  fl_end_loop();
  fl_pop_matrix();
  // -- polygon with a hole
  fl_push_matrix();
  fl_translate(a+dx+150, b+dy);
  p->set_transformation();
  fl_color(FL_DARK2);
  fl_begin_complex_polygon();
  complex_shape_with_hole(w, h);
  fl_end_complex_polygon();
  fl_color(FL_BLUE);
  fl_begin_loop();
  fl_vertex(-w/2, -h);
  fl_vertex(w/2, -h);
  fl_vertex(w, 0);
  fl_vertex(w, h);
  fl_vertex(0, h);
  fl_vertex(-w, h/2);
  fl_vertex(-w, -h/2);
  fl_end_loop();
  fl_begin_loop();
  fl_vertex(-w2, -h2);
  fl_vertex(-w2,  h2);
  fl_vertex( w2,  h2);
  fl_vertex( w2, -h2);
  fl_end_loop();
  fl_pop_matrix();

  // ---- 3: draw polygons with arcs
  b += 44;
  // -- a rectangle with a camel hump
  fl_push_matrix();
  fl_translate(a+dx, b+dy);
  p->set_transformation();
  fl_color(FL_DARK2);
  fl_begin_complex_polygon();
  fl_vertex(-w, 0); fl_arc(0, 0, w-3, 180.0, 0.0); fl_vertex(w, 0);
  fl_vertex(w, h); fl_vertex(-w, h);
  fl_end_complex_polygon();
  fl_color(FL_BLUE);
  fl_begin_loop();
  fl_vertex(-w, 0); fl_arc(0, 0, w-3, 180.0, 0.0); fl_vertex(w, 0);
  fl_vertex(w, h); fl_vertex(-w, h);
  fl_end_loop();
  fl_pop_matrix();
  // -- a rectangle with a camel dip
  fl_push_matrix();
  fl_translate(a+dx+50, b+dy);
  p->set_transformation();
  fl_color(FL_DARK2);
  fl_begin_complex_polygon();
  fl_vertex(-w, 0); fl_arc(0, 0, w-3, 180.0, 360.0); fl_vertex(w, 0);
  fl_vertex(w, h); fl_vertex(-w, h);
  fl_end_complex_polygon();
  fl_color(FL_BLUE);
  fl_begin_loop();
  fl_vertex(-w, 0); fl_arc(0, 0, w-3, 180.0, 360.0); fl_vertex(w, 0);
  fl_vertex(w, h); fl_vertex(-w, h);
  fl_end_loop();
  fl_pop_matrix();
  // -- a rectangle with a bezier curve top
  fl_push_matrix();
  fl_translate(a+dx+100, b+dy);
  p->set_transformation();
  fl_color(FL_DARK2);
  fl_begin_complex_polygon();
  fl_vertex(-w, 0);
  fl_curve(-w+3, 0, 0, -h, 0, h, w-3, 0);
  fl_vertex(w, 0);
  fl_vertex(w, h); fl_vertex(-w, h);
  fl_end_complex_polygon();
  fl_color(FL_BLUE);
  fl_begin_loop();
  fl_vertex(-w, 0);
  fl_curve(-w+3, 0, 0, -h, 0, h, w-3, 0);
  fl_vertex(w, 0);
  fl_vertex(w, h); fl_vertex(-w, h);
  fl_end_loop();
  fl_pop_matrix();
}

UnitTest complex_shapes(UT_TEST_COMPLEX_SHAPES, "Complex Shapes", Ut_Complex_Shapes_Test::create);
