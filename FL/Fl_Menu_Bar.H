//
// "$Id: Fl_Menu_Bar.H,v 1.5.2.4 2001/01/22 15:13:37 easysw Exp $"
//
// Menu bar header file for the Fast Light Tool Kit (FLTK).
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

#ifndef Fl_Menu_Bar_H
#define Fl_Menu_Bar_H

#include "Fl_Menu_.H"

class Fl_Menu_Bar : public Fl_Menu_ {
protected:
    FL_EXPORT void draw();
public:
    FL_EXPORT int handle(int);
    Fl_Menu_Bar(int x,int y,int w,int h,const char *l=0)
      : Fl_Menu_(x,y,w,h,l) {}
};

#endif

//
// End of "$Id: Fl_Menu_Bar.H,v 1.5.2.4 2001/01/22 15:13:37 easysw Exp $".
//
