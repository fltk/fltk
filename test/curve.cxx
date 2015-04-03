//
// "$Id$"
//
// Curve test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2015 by Bill Spitzak and others.
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

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Hor_Value_Slider.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Toggle_Button.H>

double args[9] = {
  20,20, 50,200, 100,20, 200,200, 0};
const char* name[9] = {
  "X0", "Y0", "X1", "Y1", "X2", "Y2", "X3", "Y3", "rotate"};

int points;

class Drawing : public Fl_Widget {
  void draw() {
    fl_push_clip(x(),y(),w(),h());
    fl_color(FL_DARK3);
    fl_rectf(x(),y(),w(),h());
    fl_push_matrix();
    if (args[8]) {
      fl_translate(x()+w()/2.0, y()+h()/2.0);
      fl_rotate(args[8]);
      fl_translate(-(x()+w()/2.0), -(y()+h()/2.0));
    }
    fl_translate(x(),y());
    if (!points) {
    fl_color(FL_WHITE);
    fl_begin_complex_polygon();
    fl_curve(args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7]);
    fl_end_complex_polygon();
    }
    fl_color(FL_BLACK);
    fl_begin_line();
    fl_vertex(args[0],args[1]);
    fl_vertex(args[2],args[3]);
    fl_vertex(args[4],args[5]);
    fl_vertex(args[6],args[7]);
    fl_end_line();
    fl_color(points ? FL_WHITE : FL_RED);
    points ? fl_begin_points() : fl_begin_line();
    fl_curve(args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7]);
    points ? fl_end_points() : fl_end_line();
    fl_pop_matrix();
    fl_pop_clip();
  }
public:
  Drawing(int X,int Y,int W,int H) : Fl_Widget(X,Y,W,H) {}
};

Drawing *d;

void points_cb(Fl_Widget* o, void*) {
  points = ((Fl_Toggle_Button*)o)->value();
  d->redraw();
}

void slider_cb(Fl_Widget* o, void* v) {
  Fl_Slider* s = (Fl_Slider*)o;
  args[fl_intptr_t(v)] = s->value();
  d->redraw();
}

int main(int argc, char** argv) {
  Fl_Double_Window window(300,555);
  Drawing drawing(10,10,280,280);
  d = &drawing;

  int y = 300;
  for (int n = 0; n<9; n++) {
    Fl_Slider* s = new Fl_Hor_Value_Slider(50,y,240,25,name[n]); y += 25;
    s->minimum(0); s->maximum(280);
    if (n == 8) s->maximum(360);
    s->step(1);
    s->value(args[n]);
    s->align(FL_ALIGN_LEFT);
    s->callback(slider_cb, (void*)(fl_intptr_t)n);
  }
  Fl_Toggle_Button but(50,y,50,25,"points");
  but.callback(points_cb);

  window.end();
  window.show(argc,argv);
  return Fl::run();
}

//
// End of "$Id$".
//
