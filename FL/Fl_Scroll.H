//
// "$Id: Fl_Scroll.H,v 1.5.2.3 2001/01/22 15:13:38 easysw Exp $"
//
// Scroll header file for the Fast Light Tool Kit (FLTK).
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

#ifndef Fl_Scroll_H
#define Fl_Scroll_H

#include "Fl_Group.H"
#include "Fl_Scrollbar.H"

class Fl_Scroll : public Fl_Group {

  int xposition_, yposition_;
  int width_, height_;
  int oldx, oldy;
  static FL_EXPORT void hscrollbar_cb(Fl_Widget*, void*);
  static FL_EXPORT void scrollbar_cb(Fl_Widget*, void*);
  FL_EXPORT void fix_scrollbar_order();
  static FL_EXPORT void draw_clip(void*,int,int,int,int);
  FL_EXPORT void bbox(int&,int&,int&,int&);

protected:

  FL_EXPORT void draw();

public:

  Fl_Scrollbar scrollbar;
  Fl_Scrollbar hscrollbar;

  FL_EXPORT void resize(int,int,int,int);
  FL_EXPORT int handle(int);

  FL_EXPORT Fl_Scroll(int X,int Y,int W,int H,const char*l=0);

  enum { // values for type()
    HORIZONTAL = 1,
    VERTICAL = 2,
    BOTH = 3,
    ALWAYS_ON = 4,
    HORIZONTAL_ALWAYS = 5,
    VERTICAL_ALWAYS = 6,
    BOTH_ALWAYS = 7
  };

  int xposition() const {return xposition_;}
  int yposition() const {return yposition_;}
  FL_EXPORT void position(int, int);
};

#endif

//
// End of "$Id: Fl_Scroll.H,v 1.5.2.3 2001/01/22 15:13:38 easysw Exp $".
//
