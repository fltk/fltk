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

#include <stdio.h>
#include <string.h>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
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
  int hotspot = fl_message_hotspot();
  fl_message_hotspot(0);
  fl_message_title("note: no hotspot set for this dialog");
  int rep = fl_choice("Are you sure you want to quit?", 
		      "Cancel", "Quit", "Dunno");
  fl_message_hotspot(hotspot);
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

  Fl_Double_Window window(200, 105);
  Fl_Return_Button b(20, 10, 160, 35, buffer); b.callback(rename_me);
  Fl_Button b2(20, 50, 160, 35, buffer2); b2.callback(rename_me_pwd);
  window.end();
  window.resizable(&b);
  window.show(argc, argv);

// Also we test to see if the exit callback works:
  window.callback(window_callback);

// set default message window title
  // fl_message_title_default("Default Window Title");

  return Fl::run();
}
    
//
// End of "$Id$".
//
