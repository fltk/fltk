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

Fl_Double_Window *window;

// inner box dimensions
int Ax = 0, Bx = 35, Cx = 70, Dx = 105, Nx = 140, Aw = 35, Rw = 70, Mw = 140;
int Ay = 0, Ey = 35, Gy = 70, Iy = 105, My = 140, Ah = 35, Rh = 70, Nh = 175;

// resize box and arrow group dimensions
int TLx = 35, TRx = 245, TGx = 420, TLw = 175, TGw = 140, TAw = 35;
int TLy = 35, BLy = 245, LGy = 420, TLh = 175, LGh = 105, LAh = 35;

// window dimensions
int Ww = 560, Wh = 525;

class Resizebox : public Fl_Group {
public:
  Resizebox(int X, int Y, int W, int H, const char *T);
};

Resizebox::Resizebox(int X, int Y, int W, int H, const char *T)
  : Fl_Group(X, Y, W, H, T) {
  this->align(FL_ALIGN_TOP_LEFT);
  this->box(FL_UP_BOX);

  Fl_Box *b;
  b = new Fl_Box(FL_FRAME_BOX, X + Ax, Y + Ay, Aw, Ah, "A"); b->color(14);
  b = new Fl_Box(FL_FRAME_BOX, X + Bx, Y + Ay, Aw, Ah, "B"); b->color(9);
  b = new Fl_Box(FL_FRAME_BOX, X + Cx, Y + Ay, Aw, Ah, "C"); b->color(10);
  b = new Fl_Box(FL_FRAME_BOX, X + Dx, Y + Ay, Aw, Ah, "D"); b->color(11);
  b = new Fl_Box(FL_FRAME_BOX, X + Ax, Y + Ey, Aw, Ah, "E"); b->color(9);
  b = new Fl_Box(FL_FRAME_BOX, X + Bx, Y + Ey, Rw, Rh, " "); b->color(8);
  b = new Fl_Box(FL_FRAME_BOX, X + Dx, Y + Ey, Aw, Ah, "F"); b->color(12);
  b = new Fl_Box(FL_FRAME_BOX, X + Ax, Y + Gy, Aw, Ah, "G"); b->color(10);
  b = new Fl_Box(FL_FRAME_BOX, X + Dx, Y + Gy, Aw, Ah, "H"); b->color(13);
  b = new Fl_Box(FL_FRAME_BOX, X + Ax, Y + Iy, Aw, Ah, "I"); b->color(11);
  b = new Fl_Box(FL_FRAME_BOX, X + Bx, Y + Iy, Aw, Ah, "J"); b->color(12);
  b = new Fl_Box(FL_FRAME_BOX, X + Cx, Y + Iy, Aw, Ah, "K"); b->color(13);
  b = new Fl_Box(FL_FRAME_BOX, X + Dx, Y + Iy, Aw, Ah, "L"); b->color(14);
  b = new Fl_Box(FL_FRAME_BOX, X + Ax, Y + My, Mw, Ah, "M"); b->color(12);
  b = new Fl_Box(FL_FRAME_BOX, X + Nx, Y + Ay, Aw, Nh, "N"); b->color(13);

  this->end();
  this->resizable(this);
}

class Resizables : public Fl_Group {
public:
  Resizables(int X, int Y, int W, int H, const char *T = 0);
  Resizebox *TL, *TR, *BL, *BR; // topleft, topright, bottomleft, bottomright
  Harrow *LA, *RA;              // left arrow, right arrow
  Varrow *TA, *BA;              // top arrow, bottom arrow
};

Resizables::Resizables(int X, int Y, int W, int H, const char *T)
  : Fl_Group(X, Y, W, H, T) {
  this->box(FL_UP_BOX);
  this->color(FL_WHITE);

  TL = new Resizebox(X + TLx, Y + TLy, TLw, TLh, "Original Size");
  TL->resizable(0);
  TR = new Resizebox(X + TRx, Y + TLy, TLw, TLh, "Horizontally Resized");
  BL = new Resizebox(X + TLx, Y + BLy, TLw, TLh, "Vertically Resized");
  BR = new Resizebox(X + TRx, Y + BLy, TLw, TLh, "Horizontally and Vertically Resized");

  Fl_Group *LG = new Fl_Group(X + TLx, Y + LGy, TLw, LGh);
  LG->box(FL_NO_BOX);
  LG->color(FL_WHITE);
  LA = new Harrow(LG->x(), LG->y(), LG->w(), LAh, "Initial\nwidth");
  LG->resizable(LA);
  LG->end();

  Fl_Group *RG = new Fl_Group(X + TRx, Y + LGy, TLw, LGh);
  RG->box(FL_NO_BOX);
  RG->color(FL_WHITE);
  RA = new Harrow(RG->x(), RG->y(), RG->w(), LAh, "Resized\nwidth");
  RG->resizable(RA);
  RG->end();

  Fl_Group *TG = new Fl_Group(X + TGx, Y + TLy, TGw, TLh);
  TG->box(FL_NO_BOX);
  TG->color(FL_WHITE);
  TA = new Varrow(X + TGx, Y + TLy, TAw, TLh, "Initial\nheight");
  TG->resizable(TA);
  TG->end();

  Fl_Group *BG = new Fl_Group(X + TGx, Y + BLy, TGw, TLh);
  BG->box(FL_NO_BOX);
  BG->color(FL_WHITE);
  BA = new Varrow(X + TGx, Y + BLy, TAw, TLh, "Resized\nheight");
  BG->resizable(BA);
  BG->end();

  this->resizable(BR);
  this->end();
}

int main(int argc, char **argv) {
  window = new Fl_Double_Window(Ww, Wh, "resize-example1");
  window->color(FL_WHITE);
  Resizables *resizables = new Resizables(0, 0, Ww, Wh);
  window->end();
  window->resizable(resizables);
  window->size_range(Ww, Wh);
  window->show(argc, argv);
  window->size(Ww + 140, Wh + 35);
  return Fl::run();
}
