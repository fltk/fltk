//
// "$Id: Fl_Bitmap.H,v 1.5.2.3.2.1 2001/08/05 23:58:54 easysw Exp $"
//
// Bitmap header file for the Fast Light Tool Kit (FLTK).
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

#ifndef Fl_Bitmap_H
#define Fl_Bitmap_H
#  include "Fl_Image.H"

class Fl_Widget;
struct Fl_Menu_Item;

class FL_EXPORT Fl_Bitmap : public Fl_Image {
  public:

  const uchar *array;
  ulong id; // for internal use

  Fl_Bitmap(const uchar *bits, int W, int H) :
    Fl_Image(W,H), array(bits), id(0) {}
  Fl_Bitmap(const char *bits, int W, int H) :
    Fl_Image(W,H), array((const uchar *)bits), id(0) {}
  virtual ~Fl_Bitmap();
  virtual void draw(int X, int Y, int W, int H, int cx=0, int cy=0);
  void draw(int X, int Y) {draw(X, Y, w(), h(), 0, 0);}
  virtual void label(Fl_Widget*w);
  virtual void label(Fl_Menu_Item*m);
};

#endif

//
// End of "$Id: Fl_Bitmap.H,v 1.5.2.3.2.1 2001/08/05 23:58:54 easysw Exp $".
//
