//
// Fl_Grid demo program for the Fast Light Tool Kit (FLTK).
//
// Copyright 2021 by Albrecht Schlosser
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

// Q: How to achieve a spaced out layout?
// https://groups.google.com/g/fltkgeneral/c/haet7hOQR0g

// A: We use an Fl_Grid with 1 x 7 cells (5 buttons) as requested:
// [New] [Options] <gap> [About] [Help] <gap> [Quit]

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Grid.H>
#include <FL/Fl_Button.H>

int main(int argc, char **argv) {

  Fl_Double_Window *win = new Fl_Double_Window(460, 200, "Fl_Grid Row with 5 Buttons");

  Fl_Grid *grid = new Fl_Grid(0, 0, win->w(), 50);
  grid->layout(1, 7, 10, 10);

  // create the buttons

  Fl_Button *b0 = new Fl_Button(0, 0, 80, 30, "New");
  Fl_Button *b1 = new Fl_Button(0, 0, 80, 30, "Options");
  Fl_Button *b3 = new Fl_Button(0, 0, 80, 30, "About");
  Fl_Button *b4 = new Fl_Button(0, 0, 80, 30, "Help");
  Fl_Button *b6 = new Fl_Button(0, 0, 80, 30, "Quit");

  grid->end();

  // assign buttons to grid positions

  grid->widget(b0, 0, 0);
  grid->widget(b1, 0, 1); grid->col_gap(1, 0);
  grid->widget(b3, 0, 3);
  grid->widget(b4, 0, 4); grid->col_gap(4, 0);
  grid->widget(b6, 0, 6);

  // set column weights for resizing (only empty columns resize)

  int weight[] = { 0, 0, 50, 0, 0, 50, 0 };
  grid->col_weight(weight, 7);

  grid->end();
  // grid->show_grid(1);     // enable to display grid helper lines

  // add content ...

  Fl_Group *g1 = new Fl_Group(0, 50, win->w(), win->h() - 50);
  // add more widgets ...

  win->end();
  win->resizable(g1);
  win->size_range(win->w(), 100);
  win->show(argc, argv);

  int ret = Fl::run();
  delete win; // not necessary but useful to test for memory leaks
  return ret;
}
