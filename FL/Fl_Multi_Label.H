//
// "$Id: Fl_Multi_Label.H,v 1.5.2.3.2.3 2003/01/30 21:39:41 easysw Exp $"
//
// Multi-label header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2003 by Bill Spitzak and others.
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

#ifndef Fl_Multi_Label_H
#define Fl_Multi_Label_H

class Fl_Widget;
struct Fl_Menu_Item;

struct FL_EXPORT Fl_Multi_Label {
  const char* labela;
  const char* labelb;
  uchar typea;
  uchar typeb;
  void label(Fl_Widget*);
  void label(Fl_Menu_Item*);
};

#endif

//
// End of "$Id: Fl_Multi_Label.H,v 1.5.2.3.2.3 2003/01/30 21:39:41 easysw Exp $".
//
