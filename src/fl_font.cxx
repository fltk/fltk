//
// "$Id$"
//
// Font selection code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2016 by Bill Spitzak and others.
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


// Select fonts from the FLTK font table.
#include "flstring.h"
#include <FL/fl_draw.H>

// -----------------------------------------------------------------------------
// all driver code is now in drivers/XXX/Fl_XXX_Graphics_Driver_xyz.cxx
// -----------------------------------------------------------------------------

double fl_width(const char* c) {
  if (c) return fl_width(c, (int) strlen(c));
  else return 0.0f;
}

void fl_draw(const char* str, int x, int y) {
  fl_draw(str, (int) strlen(str), x, y);
}

void fl_draw(int angle, const char* str, int x, int y) {
  fl_draw(angle, str, (int) strlen(str), x, y);//must be fixed!
}

void fl_text_extents(const char *c, int &dx, int &dy, int &w, int &h) {
  if (c)  fl_text_extents(c, (int) strlen(c), dx, dy, w, h);
  else {
    w = 0; h = 0;
    dx = 0; dy = 0;
  }
} // fl_text_extents


void fl_draw(const char* str, int l, float x, float y) {
  fl_graphics_driver->draw(str, l, x, y);
}

void fl_set_spot(int font, int size, int X, int Y, int W, int H, Fl_Window *win)
{
  fl_graphics_driver->set_spot(font, size, X, Y, W, H, win);
}

void fl_reset_spot()
{
  fl_graphics_driver->reset_spot();
}

//
// End of "$Id$".
//
