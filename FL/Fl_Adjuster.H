//
// "$Id: Fl_Adjuster.H,v 1.5.2.3 2001/01/22 15:13:37 easysw Exp $"
//
// Adjuster widget header file for the Fast Light Tool Kit (FLTK).
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

// 3-button "slider", made for Nuke

#ifndef Fl_Adjuster_H
#define Fl_Adjuster_H

#ifndef Fl_Valuator_H
#include "Fl_Valuator.H"
#endif

class Fl_Adjuster : public Fl_Valuator {
  int drag;
  int ix;
  int soft_;
protected:
  FL_EXPORT void draw();
  FL_EXPORT int handle(int);
  FL_EXPORT void value_damage();
public:
  FL_EXPORT Fl_Adjuster(int x,int y,int w,int h,const char *l=0);
  FL_EXPORT void soft(int x) {soft_ = x;}
  FL_EXPORT int soft() const {return soft_;}
};

#endif

//
// End of "$Id: Fl_Adjuster.H,v 1.5.2.3 2001/01/22 15:13:37 easysw Exp $".
//
