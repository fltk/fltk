//
// "$Id: Fl_Image.H,v 1.5.2.3.2.3 2001/11/19 01:06:45 easysw Exp $"
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

#  include "x.H"

class Fl_Widget;
struct Fl_Menu_Item;

class FL_EXPORT Fl_Image {
  int w_, h_;

  public:

  int w() const {return w_;}
  void w(int W) {w_ = W;}
  int h() const {return h_;}
  void h(int H) {h_ = H;}

  Fl_Image(int W, int H) {w_ = W; h_ = H;}
  virtual ~Fl_Image();
  virtual Fl_Image *copy(int W, int H);
  Fl_Image *copy() { return copy(w(), h()); }
  virtual void color_average(Fl_Color c, float i);
  void inactive() { color_average(FL_GRAY, .33f); }
  virtual void desaturate();
  virtual void label(Fl_Widget*w);
  virtual void label(Fl_Menu_Item*m);
  virtual void draw(int X, int Y, int W, int H, int cx=0, int cy=0);
  void draw(int X, int Y) {draw(X, Y, w(), h(), 0, 0);}
};

class FL_EXPORT Fl_RGB_Image : public Fl_Image {
  public:

  const uchar *array;
  int alloc_array; // Non-zero if array was allocated
  int d, ld;
  Fl_Offscreen id; // for internal use
  Fl_Bitmask mask; // for internal use (mask bitmap)

  Fl_RGB_Image(const uchar *bits, int W, int H, int D=3, int LD=0) :
    Fl_Image(W,H), array(bits), alloc_array(0), d(D), ld(LD), id(0) {}
  virtual ~Fl_RGB_Image();
  virtual Fl_Image *copy(int W, int H);
  Fl_Image *copy() { return copy(w(), h()); }
  virtual void color_average(Fl_Color c, float i);
  virtual void desaturate();
  virtual void draw(int X, int Y, int W, int H, int cx=0, int cy=0);
  void draw(int X, int Y) {draw(X, Y, w(), h(), 0, 0);}
  virtual void label(Fl_Widget*w);
  virtual void label(Fl_Menu_Item*m);
};

#endif

//
// End of "$Id: Fl_Image.H,v 1.5.2.3.2.3 2001/11/19 01:06:45 easysw Exp $".
//
