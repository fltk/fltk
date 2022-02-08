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

// window, complex and arrow dimensions
int TLx = 35, TRx = 320, TLw = 260, Ww = 620;
int TLy = 35, LGy = 100, TLh = 65, LGh = 80, LAh = 35, Wh = 200;

Fl_Double_Window *window = 0;

class Complex : public Fl_Group {
public:
  Complex(int X, int Y, int W, int H, const char *T = 0);
  Fl_Box *m_button1, *m_input1, *m_button2, *m_input2;
};

Complex::Complex(int X, int Y, int W, int H, const char *T)
  : Fl_Group(X, Y, W, H, T) {
  this->box(FL_UP_BOX);
  Fl_Group *LG = new Fl_Group(X + 10, Y + 10, 120, 45);
  LG->box(FL_UP_BOX);
  m_button1 = new Fl_Box(X + 20, Y + 20, 40, 25, "btn");
  m_button1->box(FL_UP_BOX);
  m_input1 = new Fl_Box(X + 70, Y + 20, 50, 25, "input");
  m_input1->box(FL_UP_BOX);
  m_input1->color(FL_YELLOW);
  LG->resizable(m_input1);
  LG->end();

  Fl_Group *RG = new Fl_Group(X + 130, Y + 10, 120, 45);
  RG->box(FL_UP_BOX);
  m_button2 = new Fl_Box(X + 140, Y + 20, 40, 25, "btn");
  m_button2->box(FL_UP_BOX);
  m_input2 = new Fl_Box(X + 190, Y + 20, 50, 25, "input");
  m_input2->box(FL_UP_BOX);
  m_input2->color(FL_YELLOW);
  RG->resizable(m_input2);
  RG->end();
  this->end();
}

class Resizables : public Fl_Group {
public:
  Resizables(int X, int Y, int W, int H, const char *T = 0);
  Complex *TL, *TR; // topleft, topright
  Harrow *LA, *RA;  // left arrow, right arrow
};

Resizables::Resizables(int X, int Y, int W, int H, const char *T)
  : Fl_Group(X, Y, W, H, T) {
  TL = new Complex(X + TLx, Y + TLy, TLw, TLh, "Original");
  TL->align(FL_ALIGN_TOP_LEFT);

  TR = new Complex(X + TRx, Y + TLy, TLw, TLh, "Horizonally Resized");
  TR->align(FL_ALIGN_TOP_LEFT);

  Fl_Group *LG = new Fl_Group(X + TLx, Y + LGy, TLw, LGh);
  LG->box(FL_NO_BOX);
  LG->color(FL_WHITE);
  LA = new Harrow(TL->m_input2->x(), LG->y(), TL->m_input2->w(), LAh, "Initial\nwidth");
  LG->resizable(LA);
  LG->end();

  Fl_Group *RG = new Fl_Group(X + TRx, Y + LGy, TLw, LGh);
  RG->box(FL_NO_BOX);
  RG->color(FL_WHITE);

  Fl_Group *RG0 = new Fl_Group(X + TRx, Y + LGy, TLw / 2, LGh);
  RG0->box(FL_NO_BOX);
  RG0->color(FL_WHITE);
  RG0->end();

  Fl_Group *RG1 = new Fl_Group(X + TRx + TLw / 2, Y + LGy, TLw / 2, LGh);
  RG1->box(FL_NO_BOX);
  RG1->color(FL_WHITE);
  RA = new Harrow(TR->m_input2->x(), RG1->y(), TR->m_input2->w(), LAh, "Resized\nwidth");
  RG1->resizable(RA);
  RG1->end();

  RG->end();

  this->resizable(TR);
  this->end();
}


int main(int argc, char **argv) {
  window = new Fl_Double_Window(Ww, Wh, "resize-example4b");
  window->color(FL_WHITE);
  Resizables *resizables = new Resizables(0, 0, Ww, Wh);
  window->end();
  window->resizable(resizables);
  window->size_range(Ww, Wh);
  window->show(argc, argv);
  window->size(Ww + 90, Wh);
  return Fl::run();
}
