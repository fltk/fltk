//
// "$Id: fl_set_gray.cxx,v 1.5.2.3 2001/01/22 15:13:41 easysw Exp $"
//
// Background (gray) color routines for the Fast Light Tool Kit (FLTK).
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

// -fg, -bg, and -bg2 switches

#include <FL/Fl.H>

void Fl::background(uchar r, uchar g, uchar b) {
  // replace the gray ramp so that color 47 is this color
  int i;
  for (i = 32; i <= 47; i++) {
    int m = (i-32)*255/23;
    Fl::set_color((Fl_Color)i,r*m/166,g*m/166,b*m/166);
  }
  for (; i < 56; i++) {
    int m = 255-(i-32)*255/23;
    Fl::set_color((Fl_Color)i,255-(255-r)*m/89,255-(255-g)*m/89,255-(255-b)*m/89);
  }
}

static void set_others() {
  uchar r,g,b; Fl::get_color(FL_BLACK,r,g,b);
  uchar r1,g1,b1; Fl::get_color(FL_WHITE,r1,g1,b1);
  Fl::set_color(FL_INACTIVE_COLOR,(2*r+r1)/3, (2*g+g1)/3, (2*b+b1)/3);
  Fl::set_color(FL_SELECTION_COLOR,(2*r1+r)/3, (2*g1+g)/3, (2*b1+b)/3);
}

void Fl::foreground(uchar r, uchar g, uchar b) {
  Fl::set_color(FL_BLACK,r,g,b);
  set_others();
}

void Fl::background2(uchar r, uchar g, uchar b) {
  Fl::set_color(FL_WHITE,r,g,b);
  set_others();
}

//
// End of "$Id: fl_set_gray.cxx,v 1.5.2.3 2001/01/22 15:13:41 easysw Exp $".
//
