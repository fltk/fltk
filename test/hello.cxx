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
#include <FL/Fl_Button.H>

class MyData : public Fl_Callback_User_Data {
public:
  MyData()
  {
  }
};


void cb1a(Fl_Widget* w, void* data) { }
void cb1b(Fl_Widget* w, MyData* data) { }
void cb2(Fl_Widget* w, long data) { }
void cb3(Fl_Widget* w) { }
void cb4a(void* data) { }
void cb4b(MyData* data) { }
void cb5(long data) { }
void cb6() { }



int main(int argc, char **argv) {
  Fl_Window *window = new Fl_Window(340, 180);
  Fl_Box *box = new Fl_Box(20, 40, 300, 100, "Hello, World!");
  box->box(FL_UP_BOX);
  box->labelfont(FL_BOLD + FL_ITALIC);
  box->labelsize(36);
  box->labeltype(FL_SHADOW_LABEL);

  auto* btn = new Fl_Button(20, 150, 100, 30, "Click Me");

  MyData* my_data = new MyData();

  btn->callback(nullptr);
  btn->callback(cb1a);
  btn->callback((Fl_Callback*)cb1b, my_data, true);
  btn->callback(cb2, 0);
  btn->callback(cb3);
  btn->callback(cb4a, (char*)"test");
  btn->callback((Fl_Callback*)cb4b, my_data, true);
  btn->callback(cb5, 0);
  btn->callback(cb6);

  btn->callback([]() { });


  window->end();
  window->show(argc, argv);
  return Fl::run();
}
