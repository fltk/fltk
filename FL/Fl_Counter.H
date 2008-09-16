//
// "$Id$"
//
// Counter header file for the Fast Light Tool Kit (FLTK).
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
   Fl_Counter widget . */

// A numerical value with up/down step buttons.  From Forms.

#ifndef Fl_Counter_H
#define Fl_Counter_H

#ifndef Fl_Valuator_H
#include "Fl_Valuator.H"
#endif

// values for type():
#define FL_NORMAL_COUNTER	0
#define FL_SIMPLE_COUNTER	1

/**
  \class Fl_Counter
  \brief Fl_Counter widget for Forms compatibility.

  The Fl_Counter widget is provided for forms compatibility.
  It controls a single floating point value.

  \todo Refactor the doxygen comments for Fl_Counter type() documentation.

  The type of an Fl_Counter object can be set using type(uchar t) to:
  \li \c FL_NORMAL_COUNTER: Displays a counter with 4 arrow buttons.
  \li \c FL_SIMPLE_COUNTER: Displays a counter with only 2 arrow buttons.
 */
class FL_EXPORT Fl_Counter : public Fl_Valuator {

  Fl_Font textfont_;
  Fl_Fontsize textsize_;
  unsigned textcolor_;
  double lstep_;
  uchar mouseobj;
  static void repeat_callback(void *);
  int calc_mouseobj();
  void increment_cb();

protected:

  void draw();

public:

  int handle(int);

  Fl_Counter(int X, int Y, int W, int H, const char* L = 0);
  ~Fl_Counter();

  /**
    Set the increment for the double-arrow buttons.
    The default value is 1.0.
    \param[in] a increment value
   */
  void lstep(double a) {lstep_ = a;}

  void step(double a,double b) {Fl_Valuator::step(a); lstep_ = b;}
  void step(double a) {Fl_Valuator::step(a);}
  Fl_Font textfont() const {return textfont_;}
  void textfont(Fl_Font s) {textfont_ = s;}
  Fl_Fontsize textsize() const {return textsize_;}
  void textsize(Fl_Fontsize s) {textsize_ = s;}
  Fl_Color textcolor() const {return (Fl_Color)textcolor_;}
  void textcolor(unsigned s) {textcolor_ = s;}

};

#endif

//
// End of "$Id$".
//
