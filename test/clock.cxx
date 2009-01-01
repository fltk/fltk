//
// "$Id$"
//
// Clock test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2009 by Bill Spitzak and others.
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
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Clock.H>
#include <FL/Fl_Round_Clock.H>

int main(int argc, char **argv) {
  Fl_Window window(220,220,"Fl_Clock");
  Fl_Clock c1(0,0,220,220); // c1.color(2,1);
  window.resizable(c1);
  window.end();
  Fl_Window window2(220,220,"Fl_Round_Clock");
  Fl_Round_Clock c2(0,0,220,220); // c2.color(3,4);
  window2.resizable(c2);
  window2.end();
  // my machine had a clock* Xresource set for another program, so
  // I don't want the class to be "clock":
  window.xclass("Fl_Clock");
  window2.xclass("Fl_Clock");
  window.show(argc,argv);
  window2.show();
  return Fl::run();
}

//
// End of "$Id$".
//
