//
// "$Id$"
//
// Minimal update test program for the Fast Light Tool Kit (FLTK).
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
// Copyright 1998-2005 by Bill Spitzak and others.
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

#include <stdlib.h>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Return_Button.H>

int main(int argc, char **argv) {
  Fl_Window *window = new Fl_Window(400,320,argv[0]);
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
