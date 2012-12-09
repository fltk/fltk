//
// "$Id$"
//
// Box widget for the Fast Light Tool Kit (FLTK).
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

#include <FL/Fl_Widget.H>
#include <FL/Fl_Box.H>


Fl_Box::Fl_Box(int X, int Y, int W, int H, const char *l)
: Fl_Widget(X,Y,W,H,l) 
{
}

Fl_Box::Fl_Box(Fl_Boxtype b, int X, int Y, int W, int H, const char *l)
: Fl_Widget(X,Y,W,H,l) 
{
  box(b);
}

void Fl_Box::draw() {
  draw_box();
  draw_label();
}

int Fl_Box::handle(int event) {
  if (event == FL_ENTER || event == FL_LEAVE) return 1;
  else return 0;
}


//
// End of "$Id$".
//
