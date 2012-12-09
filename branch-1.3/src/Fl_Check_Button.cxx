//
// "$Id$"
//
// Check button widget for the Fast Light Tool Kit (FLTK).
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
#include <FL/Fl_Check_Button.H>

// TODO Correct incorrect Fl_Check_Button comments.
// A subclass of Fl_Button that always draws as a diamond box.  This
// diamond is smaller than the widget size and can be surchecked by
// another box type, for compatibility with Forms.

/**
  Creates a new Fl_Check_Button widget using the given position, size and
  label string.
  \param[in] X, Y, W, H position and size of the widget
  \param[in] L widget label, default is no label
 */
Fl_Check_Button::Fl_Check_Button(int X, int Y, int W, int H, const char *L)
: Fl_Light_Button(X, Y, W, H, L) {
  box(FL_NO_BOX);
  down_box(FL_DOWN_BOX);
  selection_color(FL_FOREGROUND_COLOR);
}
