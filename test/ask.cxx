//
// "$Id$"
//
// Standard dialog test program for the Fast Light Tool Kit (FLTK).
//
// Demonstrates how to use readqueue to see if a button has been
// pushed, and to see if a window has been closed, thus avoiding
// the need to define callbacks.
//
// This also demonstrates how to trap attempts by the user to
// close the last window by overriding Fl::exit
//
// Copyright 1998-2008 by Bill Spitzak and others.
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

#include <stdio.h>
#include <string.h>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Return_Button.H>

#include <FL/fl_ask.H>
#include <stdlib.h>

void update_input_text(Fl_Widget* o, const char *input) {
  if (input) {
    o->copy_label(input);
    o->redraw();
  }
}

void rename_me(Fl_Widget*o) {
  const char *input = fl_input("Input:", o->label());
  update_input_text(o, input);
}

void rename_me_pwd(Fl_Widget*o) {
  const char *input = fl_password("Input PWD:", o->label());
  update_input_text(o, input);
}

void window_callback(Fl_Widget*, void*) {
  int rep = fl_choice("Are you sure you want to quit?", 
		      "Cancel", "Quit", "Dunno");
  if (rep==1) 
    exit(0);
  else if (rep==2)
    fl_message("Well, maybe you should know before we quit.");
}

int main(int argc, char **argv) {
  char buffer[128] = "Test text";
  char buffer2[128] = "MyPassword";

// this is a test to make sure automatic destructors work.  Pop up
// the question dialog several times and make sure it doesn't crash.
// fc: added more fl_ask common dialogs for test cases purposes.

  Fl_Window window(200, 105);
  Fl_Return_Button b(20, 10, 160, 35, buffer); b.callback(rename_me);
  Fl_Button b2(20, 50, 160, 35, buffer2); b2.callback(rename_me_pwd);
  window.end();
  window.resizable(&b);
  window.show(argc, argv);

// Also we test to see if the exit callback works:
  window.callback(window_callback);

  return Fl::run();
}
    
//
// End of "$Id$".
//
