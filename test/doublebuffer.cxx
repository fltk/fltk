//
// "$Id$"
//
// Double-buffering test program for the Fast Light Tool Kit (FLTK).
//
// This demo shows how double buffering helps, by drawing the
// window in a particularily bad way.
//
// The single-buffered window will blink as it updates.  The
// double buffered one will not.  It will take just as long
// (or longer) to update, but often it will appear to be faster.
//
// This demo should work for both the GL and X versions of Fl,
// even though the double buffering mechanism is totally different.
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

#include <FL/Fl.H>
#include <FL/Fl_Single_Window.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Hor_Slider.H>
#include <stdlib.h>
#include <FL/math.h>
#include <stdio.h>

// this purposely draws each line 10 times to be slow:
void star(int w, int h, int n) {
  fl_push_matrix();
  fl_translate(w/2, h/2);
  fl_scale(w/2, h/2);
  for (int i = 0; i < n; i++) {
    for (int j = i+1; j < n; j++)/* for (int k=0; k<10; k++)*/ {
      fl_begin_line();
      fl_vertex(cos(2*M_PI*i/n+.1), sin(2*M_PI*i/n+.1));
      fl_vertex(cos(2*M_PI*j/n+.1), sin(2*M_PI*j/n+.1));
      fl_end_line();
    }
  }
  fl_pop_matrix();
}

int sides[2] = {20,20};

void slider_cb(Fl_Widget* o, long v) {
  sides[v] = int(((Fl_Slider*)o)->value());
  o->parent()->redraw();
}

void bad_draw(int w,int h,int which) {
//   for (int i=0; i<10; i++) {
//     fl_color(7); fl_rectf(0,0,w,h); fl_color(0); star(w,h);
//     fl_color(0); fl_rectf(0,0,w,h); fl_color(7); star(w,h);
//   }
  fl_color(FL_BLACK); fl_rectf(0,0,w,h);
  fl_color(FL_WHITE); star(w,h,sides[which]);
  //  for (int x=0; x<sides[which]; x++) for (int y=0; y<sides[which]; y++)
  //fl_draw_box(FL_UP_BOX, 10*x, 10*y, 25,25, FL_GRAY);
}

class single_blink_window : public Fl_Single_Window {
  void draw() {bad_draw(w(),h(),0); draw_child(*child(0));}
public:
  single_blink_window(int x, int y,int w,int h,const char *l)
    : Fl_Single_Window(x,y,w,h,l) {resizable(this);}
};

class double_blink_window : public Fl_Double_Window {
  void draw() {bad_draw(w(),h(),1); draw_child(*child(0));}
public:
  double_blink_window(int x, int y, int w,int h,const char *l)
    : Fl_Double_Window(x,y,w,h,l) {resizable(this);}
};

int main(int argc, char **argv) {
  if (!Fl::visual(FL_DOUBLE))
    printf("Xdbe not supported, faking double buffer with pixmaps.\n");
  Fl_Window w01(420,420,"Fl_Single_Window"); w01.box(FL_FLAT_BOX);
  single_blink_window w1(10,10,400,400,"Fl_Single_Window");
  w1.box(FL_FLAT_BOX); w1.color(FL_BLACK); //w1.position(100,200);
  Fl_Hor_Slider slider0(20,370,360,25);
  slider0.range(2,30);
  slider0.step(1);
  slider0.value(sides[0]);
  slider0.callback(slider_cb, 0);
  w1.end();
  w01.end();
  Fl_Window w02(420,420,"Fl_Double_Window"); w02.box(FL_FLAT_BOX);
  double_blink_window w2(10,10,400,400,"Fl_Double_Window");
  w2.box(FL_FLAT_BOX); w2.color(FL_BLACK); //w2.position(600,200);
  Fl_Hor_Slider slider1(20,370,360,25);
  slider1.range(2,30);
  slider1.step(1);
  slider1.value(sides[0]);
  slider1.callback(slider_cb, 1);
  w2.end();
  w02.end();
  w01.show(argc, argv);
  w1.show();
  w02.show();
  w2.show();
  return Fl::run();
}

//
// End of "$Id$".
//
