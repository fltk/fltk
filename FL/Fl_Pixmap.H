//
// "$Id: Fl_Pixmap.H,v 1.6.2.6 2001/04/24 19:35:38 easysw Exp $"
//
// Pixmap header file for the Fast Light Tool Kit (FLTK).
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

#ifndef Fl_Pixmap_H
#define Fl_Pixmap_H

class Fl_Widget;
struct Fl_Menu_Item;

struct Fl_Pixmap {
  const char * const * data;
  int w, h; // set by first draw...
  ulong id; // for internal use (the pixmap)
  ulong mask; // for internal use (mask bitmap)
#if defined(__sgi) || (__GNUC__ == 2 && __GNUC_MINOR__ == 8)
  Fl_Pixmap(char * const * d) : data((const char*const*)d), w(-1), h(0), id(0),mask(0) {}
  Fl_Pixmap(uchar* const * d) : data((const char*const*)d), w(-1), h(0), id(0),mask(0) {}
#endif /* __sgi || GCC 2.8.x */
  Fl_Pixmap(const char * const * d) : data(d), w(-1), h(0), id(0),mask(0) {}
  Fl_Pixmap(const uchar* const * d) : data((const char*const*)d), w(-1), h(0), id(0),mask(0) {}
  FL_EXPORT ~Fl_Pixmap();
  FL_EXPORT void label(Fl_Widget*);
  FL_EXPORT void label(Fl_Menu_Item*);
  FL_EXPORT void draw(int X, int Y, int W, int H, int cx=0, int cy=0);
  void draw(int X, int Y) {draw(X, Y, w, h, 0, 0);}
};

#endif

//
// End of "$Id: Fl_Pixmap.H,v 1.6.2.6 2001/04/24 19:35:38 easysw Exp $".
//
