//
// "$Id: Fl_Timer.H,v 1.5.2.3 2001/01/22 15:13:38 easysw Exp $"
//
// Timer header file for the Fast Light Tool Kit (FLTK).
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

#ifndef Fl_Timer_H
#define Fl_Timer_H

#ifndef Fl_Widget_H
#include "Fl_Widget.H"
#endif

// values for type():
#define FL_NORMAL_TIMER		0
#define FL_VALUE_TIMER		1
#define FL_HIDDEN_TIMER		2

class Fl_Timer : public Fl_Widget {
  static FL_EXPORT void stepcb(void *);
  FL_EXPORT void step();
  char on, direction_;
  double delay, total;
  long lastsec,lastusec;
protected:
  FL_EXPORT void draw();
public:
  FL_EXPORT int handle(int);
  FL_EXPORT Fl_Timer(uchar t,int x,int y,int w,int h, const char *l);
  FL_EXPORT ~Fl_Timer();
  FL_EXPORT void value(double);
  double value() const {return delay>0.0?delay:0.0;}
  char direction() const {return direction_;}
  void direction(char d) {direction_ = d;}
  char suspended() const {return !on;}
  FL_EXPORT void suspended(char d);
};

#endif

//
// End of "$Id: Fl_Timer.H,v 1.5.2.3 2001/01/22 15:13:38 easysw Exp $".
//

