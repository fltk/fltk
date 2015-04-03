//
// "$Id$"
//
// Arc drawing test program for the Fast Light Tool Kit (FLTK).
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

double args[6] = {140, 140, 50, 0, 360, 0};
const char* name[6] = {"X", "Y", "R", "start", "end", "rotate"};

class Drawing : public Fl_Widget {
  void draw() {
    fl_push_clip(x(),y(),w(),h());
    fl_color(FL_DARK3);
    fl_rectf(x(),y(),w(),h());
    fl_push_matrix();
    if (args[5]) {
      fl_translate(x()+w()/2.0, y()+h()/2.0);
      fl_rotate(args[5]);
      fl_translate(-(x()+w()/2.0), -(y()+h()/2.0));
    }
    fl_color(FL_WHITE);
    fl_translate(x(),y());
    fl_begin_complex_polygon();
    fl_arc(args[0],args[1],args[2],args[3],args[4]);
    fl_gap();
    fl_arc(140,140,20,0,-360);
    fl_end_complex_polygon();
    fl_color(FL_RED);
    fl_begin_line();
    fl_arc(args[0],args[1],args[2],args[3],args[4]);
    fl_end_line();
    fl_pop_matrix();
    fl_pop_clip();
  }
public:
  Drawing(int X,int Y,int W,int H) : Fl_Widget(X,Y,W,H) {}
};

Drawing *d;

void slider_cb(Fl_Widget* o, void* v) {
  Fl_Slider* s = (Fl_Slider*)o;
  args[fl_intptr_t(v)] = s->value();
  d->redraw();
}

int main(int argc, char** argv) {
  Fl_Double_Window window(300,500);
  Drawing drawing(10,10,280,280);
  d = &drawing;

  int y = 300;
  for (int n = 0; n<6; n++) {
    Fl_Slider* s = new Fl_Hor_Value_Slider(50,y,240,25,name[n]); y += 25;
    if (n<3) {s->minimum(0); s->maximum(300);}
    else if (n==5) {s->minimum(0); s->maximum(360);}
    else {s->minimum(-360); s->maximum(360);}
    s->step(1);
    s->value(args[n]);
    s->align(FL_ALIGN_LEFT);
    s->callback(slider_cb, (void*)(fl_intptr_t)n);
  }

  window.end();
  window.show(argc,argv);
  return Fl::run();
}


//
// End of "$Id$".
//
