//
// Very simple Fl_Flex demo program for the Fast Light Tool Kit (FLTK).
//
// Copyright 2022 by Bill Spitzak and others.
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
#include <FL/Fl_Flex.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/fl_ask.H>

// the 'Exit' button callback closes the window and terminates the program
void exit_cb(Fl_Widget *w, void *) {
  fl_message("The '%s' button closes the window\nand terminates the program.", w->label());
  w->window()->hide();
}

// common callback for all other buttons
void button_cb(Fl_Widget *w, void *) {
  fl_message("The '%s' button does nothing.", w->label());
}

int main(int argc, char **argv) {
  Fl_Double_Window window(410, 40, "Simple Fl_Flex Demo");
  Fl_Flex flex(5, 5, window.w() - 10, window.h() - 10, Fl_Flex::HORIZONTAL);
  Fl_Button b1(0, 0, 0, 0, "File");
  Fl_Button b2(0, 0, 0, 0, "New");
  Fl_Button b3(0, 0, 0, 0, "Save");
  Fl_Box    bx(0, 0, 0, 0); // empty space
  Fl_Button eb(0, 0, 0, 0, "Exit");

  // assign callbacks to buttons
  b1.callback(button_cb);
  b2.callback(button_cb);
  b3.callback(button_cb);
  eb.callback(exit_cb);

  // set gap between adjacent buttons and extra spacing (invisible box size)
  flex.gap(10);
  flex.fixed(bx, 30); // total 50: 2 * gap + 30

  // end() groups
  flex.end();
  window.end();

  // set resizable, minimal window size, show() window, and execute event loop
  window.resizable(flex);
  window.size_range(300, 30);
  window.show(argc, argv);
  return Fl::run();
}
