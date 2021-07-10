//
// Resize example for use in the Fast Light Tool Kit (FLTK) documentation.
//
//     See Article #415: How does resizing work?
//     https://www.fltk.org/articles.php?L415
//
// Copyright 1998-2020 by Bill Spitzak and others.
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

#include "resize-arrows.h"
#include <FL/Fl_Double_Window.H>

// window, simplex and arrow dimensions
int TLx = 35, TRx = 320, TLw = 260, Ww = 620;
int TLy = 35, LGy = 125, TLh = 90, LGh = 70, LAh = 35, Wh = 200;

Fl_Double_Window *window = 0;

class Simplex : public Fl_Group {
public:
  Simplex(int X, int Y, int W, int H, const char *T = 0);
  Fl_Box *m_boxA, *m_boxB, *m_boxC, *m_boxI;
  Fl_Group *m_group, *m_groupL, *m_groupR;
};

Simplex::Simplex(int X, int Y, int W, int H, const char *T)
  : Fl_Group(X, Y, W, H, T) {
  this->box(FL_UP_BOX);
  m_group = new Fl_Group(X + 10, Y + 10, 240, 70);
  m_group->box(FL_UP_BOX);

  m_groupL = new Fl_Group(X + 10, Y + 15, 145, 35, "AB group");
  m_groupL->align(FL_ALIGN_BOTTOM_LEFT);
  m_groupL->box(FL_UP_BOX);
  m_groupL->color(FL_RED);
  m_groupL->labelcolor(FL_RED);
  m_boxA = new Fl_Box(X + 20, Y + 20, 80, 25, "A");
  m_boxA->box(FL_UP_BOX);
  m_boxA->color(FL_YELLOW);
  m_boxB = new Fl_Box(X + 110, Y + 20, 40, 25, "B");
  m_boxB->box(FL_UP_BOX);
  m_groupL->resizable(m_boxA);
  m_groupL->end();

  m_groupR = new Fl_Group(X + 155, Y + 15, 95, 35, "C group");
  m_groupR->align(FL_ALIGN_BOTTOM_RIGHT);
  m_groupR->box(FL_UP_BOX);
  m_groupR->color(FL_BLUE);
  m_groupR->labelcolor(FL_BLUE);
  m_boxC = new Fl_Box(X + 160, Y + 20, 80, 25, "C");
  m_boxC->box(FL_UP_BOX);
  m_boxC->color(FL_YELLOW);
  m_groupR->resizable(m_boxC);
  m_groupR->end();

  int d = 20;
  m_boxI = new Fl_Box(X + 155 - d, Y + 55, 2 * d, 10);
  m_boxI->box(FL_UP_BOX);
  m_boxI->color(FL_YELLOW);

  m_group->resizable(m_boxI);
  m_group->end();

  this->resizable(m_group);
  this->end();
}

class Resizables : public Fl_Group {
public:
  Resizables(int X, int Y, int W, int H, const char *T = 0);
  Simplex *TL, *TR; // top left, top right
  Harrow *LA, *RA;  // left arrow, right arrow
};

Resizables::Resizables(int X, int Y, int W, int H, const char *T)
  : Fl_Group(X, Y, W, H, T) {
  TL = new Simplex(X + TLx, Y + TLy, TLw, TLh, "Original");
  TL->align(FL_ALIGN_TOP_LEFT);

  TR = new Simplex(X + TRx, Y + TLy, TLw, TLh, "Horizontally Resized");
  TR->align(FL_ALIGN_TOP_LEFT);

  Fl_Group *LG = new Fl_Group(X + TLx, Y + LGy, TLw, LGh);
  LG->box(FL_NO_BOX);
  LG->color(FL_WHITE);
  LA = new Harrow(TL->m_boxI->x(), LG->y(), TL->m_boxI->w(), LAh, "Initial\nwidth");
  LG->resizable(LA);
  LG->end();

  Fl_Group *RG = new Fl_Group(X + TRx, Y + LGy, TLw, LGh);
  RG->box(FL_NO_BOX);
  RG->color(FL_WHITE);
  RA = new Harrow(TR->m_boxI->x(), RG->y(), TR->m_boxI->w(), LAh, "Resized\nwidth");
  RG->resizable(RA);
  RG->end();

  this->resizable(TR);
  this->end();
}


int main(int argc, char **argv) {
  window = new Fl_Double_Window(Ww, Wh, "resize-example5b");
  window->color(FL_WHITE);
  Resizables *resizables = new Resizables(0, 0, Ww, Wh);
  window->end();
  window->resizable(resizables);
  window->size_range(Ww, Wh);
  window->show(argc, argv);
  window->size(Ww + 90, Wh);
  return Fl::run();
}
