//
// "$Id: Fl_Counter.H,v 1.5.2.3 2001/01/22 15:13:37 easysw Exp $"
//
// Counter header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2001 by Bill Spitzak and others.
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
// Please report all bugs and problems to "fltk-bugs@fltk.org".
//

// A numerical value with up/down step buttons.  From Forms.

#ifndef Fl_Counter_H
#define Fl_Counter_H

#ifndef Fl_Valuator_H
#include "Fl_Valuator.H"
#endif

// values for type():
#define FL_NORMAL_COUNTER	0
#define FL_SIMPLE_COUNTER	1

class Fl_Counter : public Fl_Valuator {

  uchar textfont_, textsize_, textcolor_;
  double lstep_;
  uchar mouseobj;
  static FL_EXPORT void repeat_callback(void *);
  FL_EXPORT int calc_mouseobj();
  FL_EXPORT void increment_cb();

protected:

  FL_EXPORT void draw();

public:

  FL_EXPORT int handle(int);
  FL_EXPORT Fl_Counter(int,int,int,int,const char * = 0);
  FL_EXPORT ~Fl_Counter();
  void lstep(double a) {lstep_ = a;}
  void step(double a,double b) {Fl_Valuator::step(a); lstep_ = b;}
  void step(double a) {Fl_Valuator::step(a);}
  Fl_Font textfont() const {return (Fl_Font)textfont_;}
  void textfont(uchar s) {textfont_ = s;}
  uchar textsize() const {return textsize_;}
  void textsize(uchar s) {textsize_ = s;}
  Fl_Color textcolor() const {return (Fl_Color)textcolor_;}
  void textcolor(uchar s) {textcolor_ = s;}

};

#endif

//
// End of "$Id: Fl_Counter.H,v 1.5.2.3 2001/01/22 15:13:37 easysw Exp $".
//
