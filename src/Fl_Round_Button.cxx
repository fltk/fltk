//
// "$Id$"
//
// Round button for the Fast Light Tool Kit (FLTK).
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

// A subclass of Fl_Button that always draws as a round circle.  This
// circle is smaller than the widget size and can be surrounded by
// another box type, for compatibility with Forms.

#include <FL/Fl.H>
#include <FL/Fl_Round_Button.H>

/**
  Creates a new Fl_Round_Button widget using the given
  position, size, and label string.
*/
Fl_Round_Button::Fl_Round_Button(int X,int Y,int W,int H, const char *l)
: Fl_Light_Button(X,Y,W,H,l) {
  box(FL_NO_BOX);
  down_box(FL_ROUND_DOWN_BOX);
  selection_color(FL_FOREGROUND_COLOR);
}

//
// End of "$Id$".
//
