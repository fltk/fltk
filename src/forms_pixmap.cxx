//
// "$Id$"
//
// Forms pixmap drawing routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2008 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

#include <FL/forms.H>

/**
  Creates a new Fl_FormsPixmap widet using the given box type, position,
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
