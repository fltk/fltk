//
// "$Id: Fl_Tile.H,v 1.6.2.3 2001/01/22 15:13:38 easysw Exp $"
//
// Tile header file for the Fast Light Tool Kit (FLTK).
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

#ifndef Fl_Tile_H
#define Fl_Tile_H

#include "Fl_Group.H"

class Fl_Tile : public Fl_Group {
public:
  FL_EXPORT int handle(int);
  Fl_Tile(int X,int Y,int W,int H,const char*l=0) : Fl_Group(X,Y,W,H,l) {}
  FL_EXPORT void resize(int, int, int, int);
  FL_EXPORT void position(int, int, int, int);
};

#endif

//
// End of "$Id: Fl_Tile.H,v 1.6.2.3 2001/01/22 15:13:38 easysw Exp $".
//
