//
// "$Id: Fl_Image.H,v 1.5.2.3 2001/01/22 15:13:37 easysw Exp $"
//
// Image header file for the Fast Light Tool Kit (FLTK).
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

#ifndef Fl_Image_H
#define Fl_Image_H

class Fl_Widget;
struct Fl_Menu_Item;

struct Fl_Image {
  const uchar *array;
  int w, h, d, ld;
  ulong id; // for internal use
  FL_EXPORT Fl_Image(const uchar *bits, int W, int H, int D=3, int LD=0) :
    array(bits), w(W), h(H), d(D), ld(LD), id(0) {}
  FL_EXPORT ~Fl_Image();
  FL_EXPORT void label(Fl_Widget*);
  FL_EXPORT void label(Fl_Menu_Item*);
  FL_EXPORT void draw(int X, int Y, int W, int H, int cx=0, int cy=0);
  FL_EXPORT void draw(int X, int Y) {draw(X, Y, w, h, 0, 0);}
};

#endif

//
// End of "$Id: Fl_Image.H,v 1.5.2.3 2001/01/22 15:13:37 easysw Exp $".
//
