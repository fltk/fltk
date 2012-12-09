//
// "$Id$"
//
// Check button header file for the Fast Light Tool Kit (FLTK).
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

/* \file
   Fl_Check_Button widget . */

#ifndef Fl_Check_Button_H
#define Fl_Check_Button_H

#include "Fl_Light_Button.H"

/**
  \class Fl_Check_Button
  \brief A button with an "checkmark" to show its status.

  \image html Fl_Check_Button.png
  \image latex Fl_Check_Button.png  "Fl_Check_Button" width=4cm
  Buttons generate callbacks when they are clicked by the user. You control
  exactly when and how by changing the values for type() and when().

  The Fl_Check_Button subclass displays its "ON" state by showing a "checkmark"
  rather than drawing itself pushed in.
  
  \todo Refactor Fl_Check_Button doxygen comments (add color() info etc?)
  \todo Generate Fl_Check_Button.gif with visible checkmark.
 */
class FL_EXPORT Fl_Check_Button : public Fl_Light_Button {
public:
  Fl_Check_Button(int X, int Y, int W, int H, const char *L = 0);
};

#endif

//
// End of "$Id$".
//
