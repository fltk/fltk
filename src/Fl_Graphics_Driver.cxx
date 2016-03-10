//
// "$Id$"
//
// implementation of Fl_Graphics_Driver class for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2012 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems to:
//
//     http://www.fltk.org/str.php
//

#include <FL/Fl.H>
#include "config_lib.h"
#include <FL/Fl_Graphics_Driver.H>
#include <FL/Fl_Image.H>
#include <FL/fl_draw.H>

FL_EXPORT Fl_Graphics_Driver *fl_graphics_driver; // the current target device of graphics operations

const Fl_Graphics_Driver::matrix Fl_Graphics_Driver::m0 = {1, 0, 0, 1, 0, 0};

Fl_Graphics_Driver::Fl_Graphics_Driver() {
  font_ = 0;
  size_ = 0;
  sptr=0; rstackptr=0; 
  rstack[0] = NULL;
  fl_clip_state_number=0;
  m = m0; 
  fl_matrix = &m; 
  p = (XPOINT *)0;
  font_descriptor_ = NULL;
  p_size = 0;
};

void Fl_Graphics_Driver::text_extents(const char*t, int n, int& dx, int& dy, int& w, int& h)
{
  w = (int)width(t, n);
  h = - height();
  dx = 0;
  dy = descent();
}

void Fl_Graphics_Driver::focus_rect(int x, int y, int w, int h)
{
  line_style(FL_DOT);
  rect(x, y, w, h);
  line_style(FL_SOLID);
}

/** Draws an Fl_Image scaled to width \p W & height \p H with top-left corner at \em X,Y
 \return zero when the graphics driver doesn't implement scaled drawing, non-zero if it does implement it.
 */
int Fl_Graphics_Driver::draw_scaled(Fl_Image *img, int X, int Y, int W, int H) {
  return 0;
}

/** see fl_copy_offscreen() */
void Fl_Graphics_Driver::copy_offscreen(int x, int y, int w, int h, Fl_Offscreen pixmap, int srcx, int srcy)
{
  fl_begin_offscreen(pixmap);
  uchar *img = fl_read_image(NULL, srcx, srcy, w, h, 0);
  fl_end_offscreen();
  fl_draw_image(img, x, y, w, h, 3, 0);
  delete[] img;
}


//
// End of "$Id$".
//
