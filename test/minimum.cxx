//
// "$Id$"
//
// Minimal update test program for the Fast Light Tool Kit (FLTK).
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

//
// This is a test of the minimal update code.  The right slider has a
// label that extends outside it's border, and the minimal update
// assummes this does not happen.  Thus there is *supposed* to be
// display errors when you move the right-most or any other slider.
// If you *don't* see these errors, then the minimal update is
// broken!!!
//
// I cannot emphasize how important it is to test this and make sure
// any changes have not broken the minimal update.  These sort of bugs
// are extremely hard to fix and must be detected right away!
//
// The reason it is important to fix this is that otherwise you will
// swiftly end up with a toolkit that thinks it has to draw the window
// 20 times each time the display changes.  I don't care how fast the
// machine is, this is an insane waste of resources, and should be
// stopped!
//

#include <stdlib.h>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Return_Button.H>

int main(int argc, char **argv) {
  Fl_Double_Window *window = new Fl_Double_Window(400,320,argv[0]);
  window->resizable(*(new Fl_Box(FL_ENGRAVED_FRAME,10,10,300,300,
"MINIMUM UPDATE TEST\n"
"\n"
"The slider on the right purposely\n"
"draws outside it's boundaries.\n"
"Moving it should leave old copies\n"
"of the label.  These copies should\n"
"*not* be erased by any actions\n"
"other than hiding and showing\n"
"of that portion of the window\n"
"or changing the button that\n"
"intesects them.")));

  Fl_Slider *s;
  s = new Fl_Slider(320,10,20,300,"Too_Big_Label");
  s->align(0);

  new Fl_Button(20,270,100,30,"Button");
  new Fl_Return_Button(200,270,100,30,"Button");

  window->show(argc, argv);
  return Fl::run();
}

//
// End of "$Id$".
//
