//
// Hello, World! program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2021 by Bill Spitzak and others.
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

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/names.h>


class Fl_Canvas : public Fl_Box {
public:
  Fl_Canvas(int x, int y, int w, int h, const char *l=nullptr)
  : Fl_Box(x, y, w, h, l)
  {
  }
  int handle(int event) {
    const char *name = fl_eventnames[event];
//    printf("Canvas event: %s\n", name ? name : "null");
    switch (event) {
//      case FL_ENTER: return 1;
      case FL_PUSH:
//        puts("...");
        break;
    }
    return 0;
  }
};


int main(int argc, char **argv) {
  Fl::Pen::State s = Fl::Pen::State::BUTTON0 | Fl::Pen::State::BUTTON2;
  Fl_Window *window = new Fl_Window(340, 180);
  auto *canvas = new Fl_Canvas(20, 40, 300, 100, "Hello, World!");
  canvas->box(FL_DOWN_BOX);
  canvas->align(FL_ALIGN_TOP);
  window->end();
  window->show(argc, argv);
  return Fl::run();
}
