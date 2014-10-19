//
// "$Id$"
//
// Check button widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2014 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file. If this
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

/**
  \class Fl_Check_Button
  \brief A button with a "checkmark" to show its status.

  \image html Fl_Check_Button.png
  \image latex Fl_Check_Button.png  "Fl_Check_Button" width=4cm

  Buttons generate callbacks when they are clicked by the user. You control
  exactly when and how by changing the values for type() and when().

  The Fl_Check_Button subclass displays its "ON" state by showing a "checkmark"
  rather than drawing itself pushed in.
 */

/**
  Creates a new Fl_Check_Button widget using the given position, size, and label string.

  The default box type is FL_NO_BOX, which draws the label w/o a box
  right of the checkmark.

  The selection_color() sets the color of the checkmark.
  Default is FL_FOREGROUND_COLOR (usually black).

  You can use down_box() to change the box type of the checkmark.
  Default is FL_DOWN_BOX.

  \param[in] X, Y, W, H position and size of the widget
  \param[in] L widget label, default is no label
 */
Fl_Check_Button::Fl_Check_Button(int X, int Y, int W, int H, const char *L)
: Fl_Light_Button(X, Y, W, H, L) {
  box(FL_NO_BOX);
  down_box(FL_DOWN_BOX);
  selection_color(FL_FOREGROUND_COLOR);
}
