//
// "$Id$"
//
//     A simple demo of drag+drop with FLTK. 
//     Originally from erco's cheat sheet, permission by author.
//     Inspired by Michael Sephton's original example posted on fltk.general.
//
//     When you run the program, just drag the red square over
//     to the green square to show a 'drag and drop' sequence.
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
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>

// SIMPLE SENDER CLASS
class Sender : public Fl_Box {
public:
  // Ctor
  Sender(int x,int y,int w,int h) : Fl_Box(x,y,w,h) {
    box(FL_FLAT_BOX);
    color(9);
    label("Drag\nfrom\nhere..");
  }
  // Sender event handler
  int handle(int event) {
    int ret = Fl_Box::handle(event);
    switch ( event ) {
      case FL_PUSH: {             // do 'copy/dnd' when someone clicks on box
        const char *msg = "It works!";
	Fl::copy(msg,strlen(msg),0);
	Fl::dnd();
	ret = 1;
	break;
      }
    }
    return(ret);
  }
};
// SIMPLE RECEIVER CLASS
class Receiver : public Fl_Box {
public:
  // Ctor
  Receiver(int x,int y,int w,int h) : Fl_Box(x,y,w,h) {
    box(FL_FLAT_BOX); color(10); label("..to\nhere");
  }
  // Receiver event handler
  int handle(int event) {
    int ret = Fl_Box::handle(event);
    switch ( event ) {
      case FL_DND_ENTER:          // return(1) for these events to 'accept' dnd
      case FL_DND_DRAG:
      case FL_DND_RELEASE:
	ret = 1;
	break;
      case FL_PASTE:              // handle actual drop (paste) operation
	label(Fl::event_text());
	fprintf(stderr, "Pasted '%s'\n", Fl::event_text());
	ret = 1;
	break;
    }
    return(ret);
  }
};
int main(int argc, char **argv) {
  // Create sender window and widget
  Fl_Window win_a(0,0,200,100,"Sender");
  Sender a(0,0,100,100);
  win_a.end();
  win_a.show();
  // Create receiver window and widget
  Fl_Window win_b(400,0,200,100,"Receiver");
  Receiver b(100,0,100,100);
  win_b.end();
  win_b.show();
  return(Fl::run());
}

//
// End of "$Id$".
//
