//
// "$Id: Fl_Dial.H,v 1.5 1999/02/16 21:59:37 mike Exp $"
//
// Dial header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-1999 by Bill Spitzak and others.
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
// Please report all bugs and problems to "fltk-bugs@easysw.com".
//

#ifndef Fl_Dial_H
#define Fl_Dial_H

#ifndef Fl_Valuator_H
#include "Fl_Valuator.H"
#endif

// values for type():
#define FL_NORMAL_DIAL	0
#define FL_LINE_DIAL	1
#define FL_FILL_DIAL	2

#define FL_DIAL_CW	0
#define FL_DIAL_CCW	1

class Fl_Dial : public Fl_Valuator {

  short a1,a2;
  uchar direction_;

protected:

  // these allow subclasses to put the dial in a smaller area:
  FL_EXPORT void draw(int, int, int, int);
  FL_EXPORT int handle(int, int, int, int, int);
  FL_EXPORT void draw();

public:

  FL_EXPORT int handle(int);
  FL_EXPORT Fl_Dial(int x,int y,int w,int h, const char *l = 0);
  void angles(short a, short b) {a1=a; a2=b;}
  void direction(uchar d) {direction_ = d;}
  uchar direction() const {return direction_;}

};

#endif

//
// End of "$Id: Fl_Dial.H,v 1.5 1999/02/16 21:59:37 mike Exp $".
//
