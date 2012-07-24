//
// "$Id$"
//
// Repeat button widget for the Fast Light Tool Kit (FLTK).
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
#include <FL/Fl_Repeat_Button.H>

#define INITIALREPEAT .5
#define REPEAT .1

void Fl_Repeat_Button::repeat_callback(void *v) {
  Fl_Button *b = (Fl_Button*)v;
  Fl::add_timeout(REPEAT,repeat_callback,b);
  b->do_callback();
}

int Fl_Repeat_Button::handle(int event) {
  int newval;
  switch (event) {
  case FL_HIDE:
  case FL_DEACTIVATE:
  case FL_RELEASE:
    newval = 0; goto J1;
  case FL_PUSH:
  case FL_DRAG:
    if (Fl::visible_focus()) Fl::focus(this);
    newval = Fl::event_inside(this);
  J1:
    if (!active()) 
      newval = 0;
    if (value(newval)) {
      if (newval) {
	Fl::add_timeout(INITIALREPEAT,repeat_callback,this);
	do_callback();
      } else {
	Fl::remove_timeout(repeat_callback,this);
      }
    }
    return 1;
  default:
    return Fl_Button::handle(event);
  }
}


Fl_Repeat_Button::Fl_Repeat_Button(int X,int Y,int W,int H,const char *l)
: Fl_Button(X,Y,W,H,l) 
{
}


//
// End of "$Id$".
//
