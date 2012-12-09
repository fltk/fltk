//
// "$Id$"
//
// Forms compatible bitmap function for the Fast Light Tool Kit (FLTK).
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

#include <FL/forms.H>
/** Creates a bitmap widget from a box type, position, size and optional label specification */
Fl_FormsBitmap::Fl_FormsBitmap(
  Fl_Boxtype t, int X, int Y, int W, int H, const char* l)
: Fl_Widget(X, Y, W, H, l) {
  box(t);
  b = 0;
  color(FL_BLACK);
  align(FL_ALIGN_BOTTOM);
}
/** Sets a new bitmap bits with size W,H. Deletes the previous one.*/
void Fl_FormsBitmap::set(int W, int H, const uchar *bits) {
  delete b;
  bitmap(new Fl_Bitmap(bits, W, H));
}

/** Draws the bitmap and its associated box. */
void Fl_FormsBitmap::draw() {
  draw_box(box(), selection_color());
  if (b) {fl_color(color()); b->draw(x(), y(), w(), h());}
  draw_label();
}

//
// End of "$Id$".
//
