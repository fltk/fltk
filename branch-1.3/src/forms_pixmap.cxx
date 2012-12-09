//
// "$Id$"
//
// Forms pixmap drawing routines for the Fast Light Tool Kit (FLTK).
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

/**
  Creates a new Fl_FormsPixmap widget using the given box type, position,
  size and label string.
  \param[in] t box type
  \param[in] X, Y, W, H position and size
  \param[in] L widget label, default is no label
*/
Fl_FormsPixmap::Fl_FormsPixmap(
  Fl_Boxtype t, int X, int Y, int W, int H, const char* L)
: Fl_Widget(X, Y, W, H, L) {
  box(t);
  b = 0;
  color(FL_BLACK);
  align(FL_ALIGN_BOTTOM);
}

/**
  Set/create the internal pixmap using raw data.
  \param[in] bits raw data
*/
void Fl_FormsPixmap::set(char*const* bits) {
  delete b;
  b = new Fl_Pixmap(bits);
}

void Fl_FormsPixmap::draw() {
  draw_box(box(), selection_color());
  if (b) {fl_color(color()); b->draw(x(), y(), w(), h());}
  draw_label();
}

//
// End of "$Id$".
//
