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
#include <FL/Fl_Button.H>

// resize dialog and arrow dimensions
int TLx = 35, TRx = 270, TGx = 470, TLw = 200, TGw = 120, TAw = 35;
int TLy = 35, BLy = 160, LGy = 250, TLh = 90, LGh = 90, LAh = 35;

// window dimensions
int Ww = 590, Wh = 340;

Fl_Double_Window *window;

class ResizeDialog : public Fl_Group {
public:
  ResizeDialog(int X, int Y, int W, int H, const char *T = 0);
  Fl_Box *m_icon;
  Fl_Box *m_message;
  Fl_Button *m_button;
};

ResizeDialog::ResizeDialog(int X, int Y, int W, int H, const char *T)
  : Fl_Group(X, Y, W, H, T) {
  this->align(FL_ALIGN_TOP_LEFT);
  this->box(FL_UP_BOX);

  m_icon = new Fl_Box(X + 10, Y + 10, 30, 30, "!");
  m_icon->box(FL_DOWN_BOX);
  m_message = new Fl_Box(X + 50, Y + 10, 140, 30, "Out of Memory");
  m_message->box(FL_DOWN_BOX);
  m_message->color(FL_YELLOW);

  Fl_Group *group = new Fl_Group(X + 50, Y + 50, 140, 30);
  Fl_Box *b = new Fl_Box(X + 50, Y + 50, 90, 30, "R");
  b->box(FL_UP_BOX);
  // b->color(FL_YELLOW);
  m_button = new Fl_Button(X + 140, Y + 50, 50, 30, "Darn!");
  group->resizable(b);
  group->end();

  this->end();
  this->resizable(m_message);
}

class Resizables : public Fl_Group {
public:
  Resizables(int X, int Y, int W, int H, const char *T = 0);
  ResizeDialog *TL, *TR, *BL, *BR; // topleft, topright, bottomleft, bottomright
  Harrow *LA, *RA;                 // left arrow, right arrow
  Varrow *TA, *BA;                 // top arrow, bottom arrow
};

Resizables::Resizables(int X, int Y, int W, int H, const char *T)
  : Fl_Group(X, Y, W, H, T) {
  this->box(FL_UP_BOX);
  this->color(FL_WHITE);

  TL = new ResizeDialog(X + TLx, Y + TLy, TLw, TLh, "Original Size");
  TL->resizable(0);
  TR = new ResizeDialog(X + TRx, Y + TLy, TLw, TLh, "Horizontally Resized");
  BL = new ResizeDialog(X + TLx, Y + BLy, TLw, TLh, "Vertically Resized");
  BR = new ResizeDialog(X + TRx, Y + BLy, TLw, TLh, "Horizontally and Vertically Resized");

  Fl_Group *LG = new Fl_Group(X + TLx, Y + LGy, TLw, LGh);
  LG->box(FL_NO_BOX);
  LG->color(FL_WHITE);
  LA = new Harrow(BL->m_message->x(), LG->y(), BL->m_message->w(), LAh, "Initial\nwidth");
  LG->resizable(LA);
  LG->end();

  Fl_Group *RG = new Fl_Group(X + TRx, Y + LGy, TLw, LGh);
  RG->box(FL_NO_BOX);
  RG->color(FL_WHITE);
  RA = new Harrow(BR->m_message->x(), LG->y(), BL->m_message->w(), LAh, "Resized\nwidth");
  RG->resizable(RA);
  RG->end();

  Fl_Group *TG = new Fl_Group(X + TGx, Y + TLy, TGw, TLh);
  TG->box(FL_NO_BOX);
  TG->color(FL_WHITE);
  TA = new Varrow(TG->x(), TR->m_message->y(), TAw, TR->m_message->h(), "Initial\nheight");
  TG->resizable(TA);
  TG->end();

  Fl_Group *BG = new Fl_Group(X + TGx, Y + BLy, TGw, TLh);
  BG->box(FL_NO_BOX);
  BG->color(FL_WHITE);
  BA = new Varrow(BG->x(), BR->m_message->y(), TAw, BR->m_message->h(), "Resized\nheight");
  BG->resizable(BA);
  BG->end();

  this->resizable(BR);
  this->end();
}

int main(int argc, char **argv) {
  window = new Fl_Double_Window(Ww, Wh, "resize-example3c");
  window->color(FL_WHITE);
  Resizables *resizables = new Resizables(0, 0, Ww, Wh);
  window->end();
  window->resizable(resizables);
  window->size_range(Ww, Wh);
  window->show(argc, argv);
  window->size(Ww + 50, Wh + 35);
  Fl::visible_focus(0); // suppress focus box
  return Fl::run();
}
