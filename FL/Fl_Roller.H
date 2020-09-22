//
// Roller header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//

/* \file
   Fl_Roller widget . */

#ifndef Fl_Roller_H
#define Fl_Roller_H

#ifndef Fl_Valuator_H
#include "Fl_Valuator.H"
#endif

/**
  The Fl_Roller widget is a "dolly" control commonly used to
  move 3D objects.

  The roller can be controlled by clicking and dragging the mouse, by the
  corresponding arrow keys when the roller has the keyboard focus, or by the
  mousewheels when the mouse pointer is positioned over the roller widget.

  <P ALIGN=CENTER>\image html Fl_Roller.png
  \image latex Fl_Roller.png "Fl_Roller" width=4cm
*/
class FL_EXPORT Fl_Roller : public Fl_Valuator {
protected:
  void draw();
public:
  int handle(int);
  Fl_Roller(int X,int Y,int W,int H,const char* L=0);
};

#endif
