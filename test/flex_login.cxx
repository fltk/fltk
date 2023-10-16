//
// Fl_Flex demo program for the Fast Light Tool Kit (FLTK).
//
// Copyright 2020 by Karsten Pedersen
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
#include <FL/Fl_Flex.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>

Fl_Button *create_button(const char *caption) {
  Fl_Button *rtn = new Fl_Button(0, 0, 100, 25, caption);
  rtn->color(fl_rgb_color(225, 225, 225));
  return rtn;
}

// create widgets inside a column, i.e. parent is type(COLUMN)

void buttons_panel(Fl_Flex *parent) {
  new Fl_Box(0, 0, 0, 0, "");
  Fl_Box *title = new Fl_Box(0, 0, 0, 0, "Welcome to Fl_Flex");
  title->align(FL_ALIGN_CENTER);
  title->labelfont(FL_BOLD + FL_ITALIC);
  title->labelsize(16);

  Fl_Flex *urow = new Fl_Flex(Fl_Flex::ROW);
  {
    Fl_Box *b = new Fl_Box(0, 0, 0, 0, "Username:");
    b->align(FL_ALIGN_INSIDE | FL_ALIGN_RIGHT);
    Fl_Input *username = new Fl_Input(0, 0, 0, 0, "");

    urow->fixed(username, 180);
    urow->end();
  }

  Fl_Flex *prow = new Fl_Flex(Fl_Flex::ROW);
  {
    Fl_Box *b = new Fl_Box(0, 0, 0, 0, "Password:");
    b->align(FL_ALIGN_INSIDE | FL_ALIGN_RIGHT);
    Fl_Input *password = new Fl_Input(0, 0, 0, 0, "");

    prow->fixed(password, 180);
    prow->end();
  }

  Fl_Box *pad = new Fl_Box(0, 0, 0, 0, "");

  Fl_Flex *brow = new Fl_Flex(Fl_Flex::ROW);
  {
    new Fl_Box(0, 0, 0, 0, "");
    Fl_Button *reg = create_button("Register");
    Fl_Button *login = create_button("Login");

    brow->fixed(reg, 80);
    brow->fixed(login, 80);
    brow->gap(20);

    brow->end();
  }

  Fl_Box *b = new Fl_Box(0, 0, 0, 0, "");

  parent->fixed(title, 60);
  parent->fixed(urow, 30);
  parent->fixed(prow, 30);
  parent->fixed(pad, 1);
  parent->fixed(brow, 30);
  parent->fixed(b, 30);
}

// create widgets inside a row, i.e. parent is type(ROW)

void middle_panel(Fl_Flex *parent) {
  new Fl_Box(0, 0, 0, 0, "");

  Fl_Box *box = new Fl_Box(0, 0, 0, 0, "Image");
  box->box(FL_BORDER_BOX);
  box->color(fl_rgb_color(0, 200, 0));
  Fl_Box *spacer = new Fl_Box(0, 0, 0, 0, "");

  Fl_Flex *bp = new Fl_Flex(Fl_Flex::COLUMN);
  buttons_panel(bp);
  bp->end();

  new Fl_Box(0, 0, 0, 0, "");

  parent->fixed(box, 150);
  parent->fixed(spacer, 10);
  parent->fixed(bp, 300);
}

// The main panel consists of three "rows" inside a column, i.e. parent is
// type(COLUMN). The middle panel has a fixed size (200) such that the two
// boxes take the remaining space and middle_panel has all widgets.

void mainPanel(Fl_Flex *parent) {

  new Fl_Box(0, 0, 0, 0, ""); // flexible separator

  Fl_Flex *mp = new Fl_Flex(Fl_Flex::ROW);
  middle_panel(mp);
  mp->end();

  new Fl_Box(0, 0, 0, 0, ""); // flexible separator

  parent->fixed(mp, 200);
}

int main(int argc, char **argv) {

  Fl_Window *win = new Fl_Double_Window(100, 100, "Fl_Flex \"Login\" Layout");

  Fl_Flex *col = new Fl_Flex(5, 5, 90, 90, Fl_Flex::COLUMN);
  mainPanel(col);
  col->end();

  win->resizable(col);
  win->color(fl_rgb_color(250, 250, 250));
  win->end();

  win->resize(0, 0, 600, 300);   // same size as grid_login
  win->size_range(550, 250);
  win->show(argc, argv);

  int ret = Fl::run();
  delete win; // not necessary but useful to test for memory leaks
  return ret;
}
