//
// Clock test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2017 by Bill Spitzak and others.
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
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Clock.H>
#include <FL/Fl_Round_Clock.H>

const int dev_test = 0;  // 1 = enable non-standard colors and no-shadow tests

// close all windows when the user closes one of the windows

void close_cb(Fl_Widget *w, void *v) {
  Fl_Window *win = Fl::first_window();
  while (win) {
    win->hide();
    win = Fl::first_window();
  }
  return;
}

int main(int argc, char **argv) {
  Fl_Double_Window window(220,220,"Fl_Clock");
  window.callback(close_cb);
  Fl_Clock c1(0,0,220,220); // c1.color(2,1);
  window.resizable(c1);
  window.end();
  Fl_Double_Window window2(220,220,"Fl_Round_Clock");
  window2.callback(close_cb);
  Fl_Round_Clock c2(0,0,220,220);
  if (dev_test) {
    c2.color(FL_YELLOW,FL_RED); // set background and hands colors, resp.
    c2.shadow(0); // disable shadows of the hands
  }
  window2.resizable(c2);
  window2.end();
  // my machine had a clock* Xresource set for another program, so
  // I don't want the class to be "clock":
  window.xclass("Fl_Clock");
  window2.xclass("Fl_Clock");
  window.show(argc,argv);
  window2.show();
  return Fl::run();
}
