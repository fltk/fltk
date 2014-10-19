//
// "$Id$"
//
// Round button for the Fast Light Tool Kit (FLTK).
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

// A subclass of Fl_Button that always draws as a round circle.  This
// circle is smaller than the widget size and can be surrounded by
// another box type, for compatibility with Forms.

#include <FL/Fl.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Radio_Round_Button.H>

/**
  Creates a new Fl_Round_Button widget using the given position, size, and label string.

  \image html Fl_Round_Button.png
  \image latex Fl_Round_Button.png " Fl_Round_Button" width=4cm

  The Fl_Round_Button subclass displays the "ON" state by
  turning on a light, rather than drawing pushed in.

  The default box type is FL_NO_BOX, which draws the label w/o a box
  right of the checkmark.

  The shape of the "light" is set with down_box() and its default
  value is FL_ROUND_DOWN_BOX.

  The color of the light when on is controlled with selection_color(),
  which defaults to FL_FOREGROUND_COLOR (usually black).

  \param[in] X, Y, W, H position and size of the widget
  \param[in] L widget label, default is no label
*/
Fl_Round_Button::Fl_Round_Button(int X,int Y,int W,int H, const char *L)
: Fl_Light_Button(X,Y,W,H,L) {
  box(FL_NO_BOX);
  down_box(FL_ROUND_DOWN_BOX);
  selection_color(FL_FOREGROUND_COLOR);
}

/**
  Creates a new Fl_Radio_Button widget using the given position, size, and label string.

  The button type() is set to FL_RADIO_BUTTON.

  \param[in] X, Y, W, H position and size of the widget
  \param[in] L widget label, default is no label
*/

Fl_Radio_Round_Button::Fl_Radio_Round_Button(int X,int Y,int W,int H,const char *L)
: Fl_Round_Button(X,Y,W,H,L)
{
  type(FL_RADIO_BUTTON);
}


//
// End of "$Id$".
//
