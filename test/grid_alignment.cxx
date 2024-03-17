//
// Fl_Grid demo program for the Fast Light Tool Kit (FLTK).
//
// Copyright 2021 by Albrecht Schlosser
// Copyright 2022-2024 by Bill Spitzak and others.
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

//
// This program tests several different alignment features of Fl_Grid.
//

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Grid.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Box.H>

// Test function to change the layout (executed by timer callback)

#define TEST_RELAYOUT (0)
#define TEST_REMOVE_NOTIFY (0)

#if (TEST_RELAYOUT)
void relayout_cb(void *v) {
  Fl_Grid *grid = (Fl_Grid *)v;
  grid->layout(5, 5, 8, 4);
  grid->margin(10, 20, 30, 40);
  grid->layout();
  grid->redraw();
}
#endif

#if TEST_REMOVE_NOTIFY

void remove_cb(void *v) {
  static int n = 10;
  n--;
  Fl_Grid *grid = (Fl_Grid *)v;
#if (0)             // test 1: remove() the widget -- leaks memory (!)
  grid->remove(n);
#else               // test 2: delete the widget -- no leak
  delete grid->child(0);
#endif
  if (n > 0)
    Fl::repeat_timeout(0.3, remove_cb, v);
}

#endif

int main(int argc, char **argv) {
  Fl_Grid::Cell *c;
  Fl_Box *b;
  Fl_Double_Window *win = new Fl_Double_Window(440, 350, "Fl_Grid Alignment Test");
  Fl_Grid *grid = new Fl_Grid(10, 10, 420, 330);
  grid->layout(7, 7, 8, 4);       // cols, rows, margin, gap
  grid->box(FL_FLAT_BOX);
  grid->color(FL_WHITE);

  // add boxes (top and bottom rows)

  for (int col = 0; col < 7; col++) {
    grid->col_width(col, 50);
    b = new Fl_Box(0, 0, 20, 20);   // variable size
    if (col == 5) {
      b->size(4, 20);               // reduce width
      grid->col_width(col, 4);      // new min. width
      grid->col_weight(col, 0);     // no hor. resizing
    }
    b->box(FL_FLAT_BOX);
    b->color(FL_BLUE);
    grid->widget(b, 0, col);

    if (col == 5)
      b = new Fl_Box(0, 0, 4, 20);   // variable size
    else
      b = new Fl_Box(0, 0, 20, 20);   // variable size
    b->box(FL_FLAT_BOX);
    b->color(FL_RED);
    grid->widget(b, 6, col);
  }

  // add boxes (left and right columns)

  grid->row_height(0, 40);
  grid->row_height(6, 40);

  for (int row = 1; row < 6; row++) {
    grid->row_height(row, 40);
    b = new Fl_Box(0, 0, 20, 20);           // fixed size, see alignment below
    b->box(FL_FLAT_BOX);
    b->color(FL_RED);
    switch(row) {
      case 1: grid->widget(b, row, 0, FL_GRID_FILL); break;
      case 2: grid->widget(b, row, 0, FL_ALIGN_CENTER); break;
      case 3: grid->widget(b, row, 0, FL_ALIGN_BOTTOM_RIGHT); break;
      case 4: grid->widget(b, row, 0, FL_ALIGN_BOTTOM_RIGHT); break;
      case 5: grid->widget(b, row, 0, FL_ALIGN_TOP_RIGHT); break;
      default: break;
    }

    b = new Fl_Box(0, 0, 20, 20);
    b->box(FL_FLAT_BOX);
    b->color(FL_GREEN);
    c = grid->widget(b, row, 6, FL_ALIGN_CENTER);
  }

  // two more boxes to demonstrate widget alignment inside the cell

  for (int row = 4; row < 6; row++) {
    b = new Fl_Box(0, 0, 20, 20);           // fixed size, see alignment below
    b->box(FL_FLAT_BOX);
    b->color(FL_MAGENTA);
    c = grid->widget(b, row, 1);            // default alignment: FL_GRID_FILL
    if (row == 4)
      c->align(FL_ALIGN_BOTTOM_LEFT);       // alignment uses widget size
    if (row == 5)
      c->align(FL_ALIGN_TOP_LEFT);          // alignment uses widget size
  }

  // one vertical box (line), spanning 5 rows

  b = new Fl_Box(0, 0, 2, 2);               // extends vertically
  b->box(FL_FLAT_BOX);
  b->color(FL_BLACK);
  grid->widget(b, 1, 5, 5, 1, FL_GRID_VERTICAL | FL_ALIGN_RIGHT);

  // add a textbox with label or title, spanning 5 cells, centered

  b = new Fl_Box(0, 0, 1, 1);               // variable size
  b->label("Hello, Fl_Grid !");
  b->labelfont(FL_BOLD + FL_ITALIC);
  b->labelsize(30);
  b->labeltype(FL_SHADOW_LABEL);
  grid->widget(b, 1, 1, 1, 5);              // rowspan = 1, colspan = 5

  // add a footer textbox, spanning 3 cells, right aligned

  b = new Fl_Box(0, 0, 1, 10);               // variable size
  b->label("FLTK/test/grid_alignment.cxx");
  b->labelfont(FL_COURIER);
  b->labelsize(11);
  b->align(FL_ALIGN_INSIDE | FL_ALIGN_RIGHT);
  grid->widget(b, 5, 2, 1, 3, FL_GRID_HORIZONTAL | FL_ALIGN_BOTTOM);

  // input widgets with fixed size and alignment inside the cell

  Fl_Input *i1 = new Fl_Input(0, 0, 100, 30, "Username:");
  c = grid->widget(i1, 2, 3, 1, 2);         // widget, col, row, colspan, rowspan
  c->align(FL_GRID_HORIZONTAL);             // widget alignment in cell

  Fl_Input *i2 = new Fl_Input(0, 0, 100, 30, "Password:");
  c = grid->widget(i2, 3, 3, 1, 2);         // widget, col, row, colspan, rowspan
  c->align(FL_GRID_HORIZONTAL);             // widget alignment in cell

  // the login button spans 2 columns

  Fl_Button *bt = new Fl_Button(0, 0, 10, 30, "Login");
  grid->widget(bt, 4, 3, 1, 2, FL_GRID_HORIZONTAL); // widget, col, row, colspan, rowspan, alignment

  grid->row_weight(1, 90);
  grid->row_weight(2,  0);
  grid->row_weight(3,  0);
  grid->row_weight(4,  0);

  grid->col_weight(0, 30);
  grid->col_weight(4, 90);

  grid->row_gap(5, 12);

  grid->end();
  grid->layout();
  grid->debug(0);
  // grid->show_grid(1);     // enable to display grid helper lines
  win->end();
  win->resizable(grid);
  win->size_range(440, 350);
  win->show(argc, argv);

#if (TEST_RELAYOUT)
  Fl::add_timeout(5.0, relayout_cb, grid);
#endif

#if (TEST_REMOVE_NOTIFY)
  Fl::add_timeout(3.0, remove_cb, grid);
#endif

  // return Fl::run();
  int ret = Fl::run();
  grid->clear_layout();
  delete win;
  return ret;
}
