//
// Fl_Grid Example Program for the Fast Light Tool Kit (FLTK).
//
// Copyright 2021-2022 by Albrecht Schlosser.
// Copyright 2022-2023 by Bill Spitzak and others.
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

// This example program is also included in the documentation.
// See FL/Fl_Grid.H

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Grid.H>
#include <FL/Fl_Button.H>

int main(int argc, char **argv) {
  Fl_Double_Window *win = new Fl_Double_Window(320, 180, "3x3 Fl_Grid with Buttons");
  // create the Fl_Grid container with five buttons
  Fl_Grid *grid = new Fl_Grid(0, 0, win->w(), win->h());
  grid->layout(3, 3, 10, 10);
  grid->color(FL_WHITE);
  Fl_Button *b0 = new Fl_Button(0, 0, 0, 0, "New");
  Fl_Button *b1 = new Fl_Button(0, 0, 0, 0, "Options");
  Fl_Button *b3 = new Fl_Button(0, 0, 0, 0, "About");
  Fl_Button *b4 = new Fl_Button(0, 0, 0, 0, "Help");
  Fl_Button *b6 = new Fl_Button(0, 0, 0, 0, "Quit");
  // assign buttons to grid positions
  grid->widget(b0, 0, 0);
  grid->widget(b1, 0, 2);
  grid->widget(b3, 1, 1);
  grid->widget(b4, 2, 0);
  grid->widget(b6, 2, 2);
  // grid->show_grid(1);     // enable to display grid helper lines
  grid->end();
  win->end();
  win->resizable(grid);
  win->size_range(300, 100);
  win->show(argc, argv);
  return Fl::run();
}
