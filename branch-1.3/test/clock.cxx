//
// "$Id$"
//
// Clock test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Clock.H>
#include <FL/Fl_Round_Clock.H>

int main(int argc, char **argv) {
  Fl_Double_Window window(220,220,"Fl_Clock");
  Fl_Clock c1(0,0,220,220); // c1.color(2,1);
  window.resizable(c1);
  window.end();
  Fl_Double_Window window2(220,220,"Fl_Round_Clock");
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
