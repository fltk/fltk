//
// "$Id: Fl_Positioner.H,v 1.5.2.3 2001/01/22 15:13:38 easysw Exp $"
//
// Positioner header file for the Fast Light Tool Kit (FLTK).
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

#ifndef Fl_Positioner_H
#define Fl_Positioner_H

#ifndef Fl_Widget_H
#include "Fl_Widget.H"
#endif

class Fl_Positioner : public Fl_Widget {

  double xmin, ymin;
  double xmax, ymax;
  double xvalue_, yvalue_;
  double xstep_, ystep_;

protected:

  // these allow subclasses to put the dial in a smaller area:
  FL_EXPORT void draw(int, int, int, int);
  FL_EXPORT int handle(int, int, int, int, int);
  FL_EXPORT void draw();

public:

  FL_EXPORT int handle(int);
  FL_EXPORT Fl_Positioner(int x,int y,int w,int h, const char *l=0);
  double xvalue() const {return xvalue_;}
  double yvalue() const {return yvalue_;}
  FL_EXPORT int xvalue(double);
  FL_EXPORT int yvalue(double);
  FL_EXPORT int value(double,double);
  FL_EXPORT void xbounds(double, double);
  double xminimum() const {return xmin;}
  void xminimum(double a) {xbounds(a,xmax);}
  double xmaximum() const {return xmax;}
  void xmaximum(double a) {xbounds(xmin,a);}
  FL_EXPORT void ybounds(double, double);
  double yminimum() const {return ymin;}
  void yminimum(double a) {ybounds(a,ymax);}
  double ymaximum() const {return ymax;}
  void ymaximum(double a) {ybounds(ymin,a);}
  void xstep(double a) {xstep_ = a;}
  void ystep(double a) {ystep_ = a;}

};

#endif

//
// End of "$Id: Fl_Positioner.H,v 1.5.2.3 2001/01/22 15:13:38 easysw Exp $".
//
