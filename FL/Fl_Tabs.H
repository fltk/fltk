//
// "$Id: Fl_Tabs.H,v 1.5.2.4 2001/01/22 15:13:38 easysw Exp $"
//
// Tab header file for the Fast Light Tool Kit (FLTK).
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

#ifndef Fl_Tabs_H
#define Fl_Tabs_H

#include "Fl_Group.H"

class Fl_Tabs : public Fl_Group {
  Fl_Widget *value_;
  Fl_Widget *push_;
  FL_EXPORT int tab_positions(int*, int*);
  FL_EXPORT int tab_height();
  FL_EXPORT void draw_tab(int x1, int x2, int W, int H, Fl_Widget* o, int sel=0);
protected:
  FL_EXPORT void draw();

public:
  FL_EXPORT int handle(int);
  FL_EXPORT Fl_Widget *value();
  FL_EXPORT int value(Fl_Widget *);
  Fl_Widget *push() const {return push_;}
  FL_EXPORT int push(Fl_Widget *);
  FL_EXPORT Fl_Tabs(int,int,int,int,const char * = 0);
  FL_EXPORT Fl_Widget *which(int event_x, int event_y);
};

#endif

//
// End of "$Id: Fl_Tabs.H,v 1.5.2.4 2001/01/22 15:13:38 easysw Exp $".
//
