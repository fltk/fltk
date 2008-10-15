//
// "$Id$"
//
// Scroll bar header file for the Fast Light Tool Kit (FLTK).
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

/** \file
   Fl_Scrollbar widget . */

#ifndef Fl_Scrollbar_H
#define Fl_Scrollbar_H

#include "Fl_Slider.H"

/**
  The Fl_Scrollbar widget displays a slider with arrow buttons
  at the ends of the scrollbar. Clicking on the arrows move up/left and
  down/right by linesize().  Scrollbars also accept 
  FL_SHORTCUT events: the arrows move by linesize(), and
  vertical scrollbars take Page Up/Down (they move by the page size minus 
  linesize()) and Home/End (they jump to the top or bottom).
  <P>Scrollbars have step(1) preset (they always return
  integers). If desired you can set the step() to non-integer
  values.  You will then have to use casts to get at the floating-point
  versions of value() from Fl_Slider. </P>
  <P ALIGN=CENTER>\image html scrollbar.gif 
  \image latex  scrollbar.eps "Fl_Scrollbar" width=4cm
*/
class FL_EXPORT Fl_Scrollbar : public Fl_Slider {

  int linesize_;
  int pushed_;
  static void timeout_cb(void*);
  void increment_cb();
protected:
  void draw();

public:

  Fl_Scrollbar(int x,int y,int w,int h, const char *l = 0);
  ~Fl_Scrollbar();
  int handle(int);

  /**
    The first form returns the integer value of the scrollbar. You can get
    the floating point value with Fl_Slider::value().  The second
    form sets value(), range(), and slider_size()
    to make a variable-sized scrollbar. You should call this every time
    your window changes size, your data changes size, or your scroll
    position changes (even if in response to a callback from this
    scrollbar).  All necessary calls to redraw() are done.
  */
  int value() {return int(Fl_Slider::value());}
  /**    See int Fl_Scrollbar::value()   */
  int value(int p, int s, int top, int total) {
    return scrollvalue(p, s, top, total);
  }
  /**
    This number controls how big the steps are that the arrow keys do.  In
    addition page up/down move by the size last sent to value()
    minus one linesize().  The default is 16.
  */
  int linesize() const {return linesize_;}
  /**
    This number controls how big the steps are that the arrow keys do.  In
    addition page up/down move by the size last sent to value()
    minus one linesize().  The default is 16.
  */
  void linesize(int i) {linesize_ = i;}

};

#endif

//
// End of "$Id$".
//
