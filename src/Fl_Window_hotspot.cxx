//
// Common hotspot routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
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
#include <FL/Fl_Window.H>
#include "Fl_Window_Driver.H"

void Fl_Window::hotspot(int X, int Y, int offscreen) {
  int mx,my;

  // Update the screen position based on the mouse position.
  Fl::get_mouse(mx,my);
  X = mx-X; Y = my-Y;

  // If offscreen is 0 (the default), make sure that the window
  // stays on the screen, if possible.
  if (!offscreen) {
    int scr_x, scr_y, scr_w, scr_h;
    Fl::screen_work_area(scr_x, scr_y, scr_w, scr_h);

    int top = 0;
    int left = 0;
    int right = 0;
    int bottom = 0;

    if (border()) {
      pWindowDriver->decoration_sizes(&top, &left, &right, &bottom);
    }
    // now ensure contents are on-screen (more important than border):
    if (X+w()+right > scr_w+scr_x) X = scr_w+scr_x-right-w();
    if (X-left < scr_x) X = left + scr_x;
    if (Y+h()+bottom > scr_h+scr_y) Y = scr_h+scr_y-bottom-h();
    if (Y-top < scr_y) Y = top + scr_y;
    // make sure that we will force this position
    if (X==x()) x(X-1);
  }

  position(X,Y);
}

void Fl_Window::hotspot(const Fl_Widget *o, int offscreen) {
  int X = o->w()/2;
  int Y = o->h()/2;
  while (o != this && o) {
    X += o->x(); Y += o->y();
    o = o->window();
  }
  hotspot(X,Y,offscreen);
}
