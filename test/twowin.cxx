//
// "$Id$"
//
// Cross-window focus test program for the Fast Light Tool Kit (FLTK).
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
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>

static Fl_Input *b1, *b2;

static void cb1(Fl_Widget *, void *) {
  b2->take_focus();
}

static void cb2(Fl_Widget *, void *) {
  b1->take_focus();
}

int main(int argc, char **argv) {

  Fl_Double_Window *win1 = new Fl_Double_Window(200, 200);
  Fl_Button *bb1 = new Fl_Button(10, 10, 100, 100, "b1");
  bb1->callback(cb1);
  b1 = new Fl_Input(10, 150, 100, 25);
  win1->label("win1");
  win1->end();

  Fl_Double_Window *win2 = new Fl_Double_Window(200, 200);
  Fl_Button *bb2 = new Fl_Button(10, 10, 100, 100, "b2");
  bb2->callback(cb2);
  b2 = new Fl_Input(10, 150, 100, 25);
  win2->label("win2");
  win2->end();

  win1->position(200, 200);
  win2->position(400, 200);

  win1->show(argc,argv);
  win2->show();
  return Fl::run();
}

//
// End of "$Id$".
//
