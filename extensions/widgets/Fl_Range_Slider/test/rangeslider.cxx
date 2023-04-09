//
// Range SLider test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 2023 by Bill Spitzak and others.
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

#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Range_Slider.H>
#include <FL/math.h>


class Star_Box : public Fl_Box {
  double h_pos_ = 0.0;
  double h_scl_ = 1.0;
  double v_pos_ = 0.0;
  double v_scl_ = 1.0;
  void draw() FL_OVERRIDE {
    const int n = 20;
    draw_box();
    fl_color(FL_BLACK);
    fl_push_matrix();
    fl_translate(w()/2, h()/2);
    fl_scale(w()/2 * h_scl_, h()/2 * v_scl_);
    fl_translate(-h_pos_, -v_pos_);
    for (int i = 0; i < n; i++) {
      for (int j = i+1; j < n; j++) {
        fl_begin_line();
        fl_vertex(cos(2*M_PI*i/n+.1), sin(2*M_PI*i/n+.1));
        fl_vertex(cos(2*M_PI*j/n+.1), sin(2*M_PI*j/n+.1));
        fl_end_line();
      }
    }
    fl_pop_matrix();
  }
public:
  Star_Box(int x, int y, int w, int h) : Fl_Box(x, y, w, h) {
    box(FL_FLAT_BOX);
    align(FL_ALIGN_CLIP);
  }
  void h_pos(int x, int w) {
    if (w < 1) w = 1;
    h_scl_ = 100.0 / w;
    h_pos_ = (x+0.5*w-50.0) / 50.0;
  }
  void v_pos(int x, int w) {
    if (w < 1) w = 1;
    v_scl_ = 100.0 / w;
    v_pos_ = (x+0.5*w-50.0) / 50.0;
  }
};

Star_Box *star_box;

void hs_cb(Fl_Widget *w, void *) {
  Fl_Range_Slider *self = (Fl_Range_Slider*)w;
  star_box->h_pos(self->value(), self->slider_size_i());
  star_box->redraw();
}

void vs_cb(Fl_Widget *w, void *) {
  Fl_Range_Slider *self = (Fl_Range_Slider*)w;
  star_box->v_pos(self->value(), self->slider_size_i());
  star_box->redraw();
}

int main(int argc, char **argv) {
  Fl_Window win(425, 425, "Fl_Range_Slider");
  star_box = new Star_Box(0, 0, 400, 400);
  Fl_Range_Slider hs(0, 400, 400, 25);
  hs.type(FL_HORIZONTAL);
  hs.value(10, 80, 0, 100);
  hs.callback(hs_cb);
  hs.do_callback();
  Fl_Range_Slider vs(400, 0, 25, 400);
  vs.type(FL_VERTICAL);
  vs.value(10, 80, 0, 100);
  vs.callback(vs_cb);
  vs.do_callback();
  win.show(argc, argv);
  return Fl::run();
}
