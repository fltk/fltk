//
// "$Id$"
//
// Iconize test program for the Fast Light Tool Kit (FLTK).
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
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <stdlib.h>

void iconize_cb(Fl_Widget *, void *v) {
  Fl_Window *w = (Fl_Window *)v;
  w->iconize();
}

void show_cb(Fl_Widget *, void *v) {
  Fl_Window *w = (Fl_Window *)v;
  w->show();
}

void hide_cb(Fl_Widget *, void *v) {
  Fl_Window *w = (Fl_Window *)v;
  w->hide();
}

void window_cb(Fl_Widget*, void*) {
  exit(0);
}

int main(int argc, char **argv) {

  Fl_Window mainw(200,200);
  mainw.end();
  mainw.show(argc,argv);

  Fl_Window control(120,120);

  Fl_Button hide_button(0,0,120,30,"hide()");
  hide_button.callback(hide_cb, &mainw);

  Fl_Button iconize_button(0,30,120,30,"iconize()");
  iconize_button.callback(iconize_cb, &mainw);

  Fl_Button show_button(0,60,120,30,"show()");
  show_button.callback(show_cb, &mainw);

  Fl_Button show_button2(0,90,120,30,"show this");
  show_button2.callback(show_cb, &control);

  //  Fl_Box box(FL_NO_BOX,0,60,120,30,"Also try running\nwith -i switch");

  control.end();
  control.show();
  control.callback(window_cb);
  return Fl::run();
}

//
// End of "$Id$".
//
