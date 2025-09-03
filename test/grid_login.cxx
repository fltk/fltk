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

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Grid.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Box.H>

int main(int argc, char **argv) {

  Fl_Double_Window *win = new Fl_Double_Window(480, 200, "Fl_Grid \"Login\" Layout");

  // Fl_Grid of 6 x 6 cells, margin 2 and gap 2

  Fl_Grid *grid = new Fl_Grid(5, 5, 470, 190);
  grid->layout(6, 6, 2, 2);  // 6 rows, 6 columns, margin 2, gap 2

  // image (150x200) in left column

  Fl_Box *ibox = new Fl_Box(0, 0, 150, 200, "Image");
  ibox->box(FL_BORDER_BOX);
  ibox->color(fl_rgb_color(0, 200, 0));
  grid->widget(ibox, 1, 1, 4, 1, FL_GRID_CENTER);

  // the title spans 2 columns (3 - 4)

  Fl_Box *title = new Fl_Box(0, 0, 200, 60);
  title->label("Welcome to Fl_Grid");
  title->align(FL_ALIGN_CENTER);
  title->labelfont(FL_BOLD + FL_ITALIC);
  title->labelsize(16);
  grid->widget(title, 1, 3, 1, 2, FL_GRID_HORIZONTAL | FL_GRID_CENTER);

  grid->col_width(2, 90);  // placeholder for labels

  // input widgets with fixed height and horizontal stretching

  Fl_Input *i1 = new Fl_Input(0, 0, 150, 30, "Username:");
  grid->widget(i1, 2, 3, 1, 2, FL_GRID_HORIZONTAL);
  grid->row_gap(2, 10); // gap below username

  Fl_Input *i2 = new Fl_Input(0, 0, 150, 30, "Password:");
  grid->widget(i2, 3, 3, 1, 2, FL_GRID_HORIZONTAL);
  grid->row_gap(3, 10); // gap below password

  // register and login buttons

  Fl_Button *btr = new Fl_Button(0, 0, 80, 30, "Register");
  grid->widget(btr, 4, 3, 1, 1, FL_GRID_HORIZONTAL);
  grid->col_gap(3, 20); // gap right of the register button

  Fl_Button *btl = new Fl_Button(0, 0, 80, 30, "Login");
  grid->widget(btl, 4, 4, 1, 1, FL_GRID_HORIZONTAL);

  // set column and row weights for resizing behavior (optional)

  int cw[] = { 20,  0,  0, 10, 10, 20};  // column weights
  int rw[] = { 10,  0,  0,  0,  0, 10};  // row weights
  grid->col_weight(cw, 6);
  grid->row_weight(rw, 6);

  grid->end();
  grid->layout();
  // grid->debug(1);
  // grid->show_grid(1);     // enable to display grid helper lines
  win->end();
  grid->color(fl_rgb_color(250, 250, 250));
  win->color(fl_rgb_color(250, 250, 250));

  win->resizable(grid);
  win->resize(0, 0, 600, 300);   // same size as flex_login
  win->size_range(550, 250);
  win->show(argc, argv);

  int ret = Fl::run();
  delete win; // not necessary but useful to test for memory leaks
  return ret;
}
