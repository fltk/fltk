//
// "$Id: Fl_get_system_colors.cxx,v 1.3 1998/10/21 14:20:34 mike Exp $"
//
// System color support for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998 by Bill Spitzak and others.
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
// Please report all bugs and problems to "fltk-bugs@easysw.com".
//

#include <FL/Fl.H>
#include <FL/x.H>
#include <FL/math.h>

void Fl::background(uchar r, uchar g, uchar b) {
  // replace the gray ramp so that FL_GRAY is this color
  if (!r) r = 1; else if (r==255) r = 254;
  double powr = log(r/255.0)/log((FL_GRAY-FL_GRAY_RAMP)/(FL_NUM_GRAY-1.0));
  if (!g) g = 1; else if (g==255) g = 254;
  double powg = log(g/255.0)/log((FL_GRAY-FL_GRAY_RAMP)/(FL_NUM_GRAY-1.0));
  if (!b) b = 1; else if (b==255) b = 254;
  double powb = log(b/255.0)/log((FL_GRAY-FL_GRAY_RAMP)/(FL_NUM_GRAY-1.0));
  for (int i = 0; i < FL_NUM_GRAY; i++) {
    double gray = i/(FL_NUM_GRAY-1.0);
    Fl::set_color(fl_gray_ramp(i),
		  uchar(pow(gray,powr)*255+.5),
		  uchar(pow(gray,powg)*255+.5),
		  uchar(pow(gray,powb)*255+.5));
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

#ifdef WIN32

static void getsyscolor(int what, void (*func)(uchar,uchar,uchar)) {
  DWORD x = GetSysColor(what);
  uchar r = uchar(x&255);
  uchar g = uchar(x>>8);
  uchar b = uchar(x>>16);
  func(r,g,b);
}

void Fl::get_system_colors() {
  getsyscolor(COLOR_WINDOWTEXT, Fl::foreground);
  getsyscolor(COLOR_BTNFACE, Fl::background);
  getsyscolor(COLOR_WINDOW, Fl::background2);
}

#else
// For X we should do something. KDE stores these colors in some standard
// place, where?

void Fl::get_system_colors()
{
  fl_open_display();
}

#endif

//
// End of "$Id: Fl_get_system_colors.cxx,v 1.3 1998/10/21 14:20:34 mike Exp $".
//
