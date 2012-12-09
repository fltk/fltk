//
// "$Id$"
//
// Colormap color selection dialog for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

#include <FL/Fl.H>
#include <FL/Fl_Single_Window.H>
#include <FL/fl_draw.H>
#include <FL/fl_show_colormap.H>
#include <config.h>

#define BOXSIZE 14
#define BORDER 4

/** 
 This widget creates a modal window for selecting a color from the colormap.
 Pretty much unchanged from Forms.
*/
class ColorMenu : public Fl_Window {
  Fl_Color initial;
  Fl_Color which, previous;
  int done;
  void drawbox(Fl_Color);
  void draw();
  int handle(int);
public:
  ColorMenu(Fl_Color oldcol);
  Fl_Color run();
};

ColorMenu::ColorMenu(Fl_Color oldcol) :
  Fl_Window(BOXSIZE*8+1+2*BORDER, BOXSIZE*32+1+2*BORDER) {
  clear_border();
  set_modal();
  initial = which = oldcol;
}

void ColorMenu::drawbox(Fl_Color c) {
  if (c > 255) return;
  int X = (c%8)*BOXSIZE+BORDER;
  int Y = (c/8)*BOXSIZE+BORDER;
#if BORDER_WIDTH < 3
  if (c == which) fl_draw_box(FL_DOWN_BOX, X+1, Y+1, BOXSIZE-1, BOXSIZE-1, c);
  else fl_draw_box(FL_BORDER_BOX, X, Y, BOXSIZE+1, BOXSIZE+1, c);
#else
  fl_draw_box(c == which ? FL_DOWN_BOX : FL_BORDER_BOX,
	      X, Y, BOXSIZE+1, BOXSIZE+1, c);
#endif
}

void ColorMenu::draw() {
  if (damage() != FL_DAMAGE_CHILD) {
    fl_draw_box(FL_UP_BOX,0,0,w(),h(),color());
    for (int c = 0; c < 256; c++) drawbox((Fl_Color)c);
  } else {
    drawbox(previous);
    drawbox(which);
  }
  previous = which;
}

int ColorMenu::handle(int e) {
  Fl_Color c = which;
  switch (e) {
  case FL_PUSH:
  case FL_DRAG: {
    int X = (Fl::event_x_root() - x() - BORDER);
    if (X >= 0) X = X/BOXSIZE;
    int Y = (Fl::event_y_root() - y() - BORDER);
    if (Y >= 0) Y = Y/BOXSIZE;
    if (X >= 0 && X < 8 && Y >= 0 && Y < 32)
      c = 8*Y + X;
    else
      c = initial;
    } break;
  case FL_RELEASE:
    done = 1;
    return 1;
  case FL_KEYBOARD:
    switch (Fl::event_key()) {
    case FL_Up: if (c > 7) c -= 8; break;
    case FL_Down: if (c < 256-8) c += 8; break;
    case FL_Left: if (c > 0) c--; break;
    case FL_Right: if (c < 255) c++; break;
    case FL_Escape: which = initial; done = 1; return 1;
    case FL_KP_Enter:
    case FL_Enter: done = 1; return 1;
    default: return 0;
    }
    break;
  default:
    return 0;
  }
  if (c != which) {
    which = (Fl_Color)c; damage(FL_DAMAGE_CHILD);
    int bx = (c%8)*BOXSIZE+BORDER;
    int by = (c/8)*BOXSIZE+BORDER;
    int px = x();
    int py = y();
    int scr_x, scr_y, scr_w, scr_h;
    Fl::screen_xywh(scr_x, scr_y, scr_w, scr_h);
    if (px < scr_x) px = scr_x;
    if (px+bx+BOXSIZE+BORDER >= scr_x+scr_w) px = scr_x+scr_w-bx-BOXSIZE-BORDER;
    if (py < scr_y) py = scr_y;
    if (py+by+BOXSIZE+BORDER >= scr_y+scr_h) py = scr_y+scr_h-by-BOXSIZE-BORDER;
    if (px+bx < BORDER) px = BORDER-bx;
    if (py+by < BORDER) py = BORDER-by;
    position(px,py);
  }
  return 1;
}

extern char fl_override_redirect; // hack for menus

#ifdef _MSC_VER
#pragma optimize("a",off) // needed to get the done check to work
#endif
Fl_Color ColorMenu::run() {
  if (which > 255) {
    position(Fl::event_x_root()-w()/2, Fl::event_y_root()-y()/2);
  } else {
    position(Fl::event_x_root()-(initial%8)*BOXSIZE-BOXSIZE/2-BORDER,
	     Fl::event_y_root()-(initial/8)*BOXSIZE-BOXSIZE/2-BORDER);
  }
  show();
  Fl::grab(*this);
  done = 0;
  while (!done) Fl::wait();
  Fl::grab(0);
  return which;
}

Fl_Color fl_show_colormap(Fl_Color oldcol) {
  ColorMenu m(oldcol);
  return m.run();
}

//
// End of "$Id$".
//
