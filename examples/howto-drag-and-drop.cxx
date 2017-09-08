//
// "$Id$"
//
// A simple demo of 'drag and drop' with FLTK.
// Originally from erco's cheat sheet, permission by author.
// Inspired by Michael Sephton's original example posted on fltk.general.
//
// When you run the program, just drag the red square over
// to the green square to show a 'drag and drop' sequence.
//
// You can also drag and drop text from another application over
// to the green square to show a 'drag and drop' sequence.
//
// Copyright 1998-2017 by Bill Spitzak and others.
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
#include <FL/fl_ask.H> // fl_message()

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
  int dnd_inside;
  char *dnd_text;
public:
  // Ctor
  Receiver(int x,int y,int w,int h) : Fl_Box(x,y,w,h) {
    box(FL_FLAT_BOX); color(10); label("..to\nhere");
    dnd_inside = 0;
    dnd_text = 0;
  }
  // Receiver event handler
  int handle(int event) {
    int ret = Fl_Box::handle(event);
    int len;
    switch ( event ) {
      case FL_DND_ENTER:	// return(1) for this event to 'accept' dnd
	label("ENTER");		// visible only if you stop the mouse at the widget's border
	fprintf(stderr, "FL_DND_ENTER\n");
	dnd_inside = 1;		// status: inside the widget, accept drop
	ret = 1;
	break;
      case FL_DND_DRAG:		// return(1) for this event to 'accept' dnd
	label("drop\nhere");
	fprintf(stderr, "FL_DND_DRAG\n");
	ret = 1;
	break;
      case FL_DND_RELEASE:	// return(1) for this event to 'accept' the payload (drop)
	fprintf(stderr, "FL_DND_RELEASE\n");
	if (dnd_inside) {
	  ret = 1;		// return(1) and expect FL_PASTE event to follow
	  label("RELEASE");
	} else {
	  ret = 0;		// return(0) to reject the DND payload (drop)
	  label("DND\nREJECTED!");
	}
	break;
      case FL_PASTE:              // handle actual drop (paste) operation
	fprintf(stderr, "FL_PASTE\n");
	copy_label(Fl::event_text());
	fprintf(stderr, "Pasted '%s'\n", Fl::event_text());

	// Don't pop up dialog windows in FL_DND_* or FL_PASTE event handling
	// resulting from DND operations. This may hang or even crash the
	// application on *some* platforms. Use a short timer to delay the
	// message display after the event processing is completed.

	delete[] dnd_text;	// don't leak (just in case)
	dnd_text = 0;

	len = Fl::event_length();
	if (len && Fl::event_text()) {
	  dnd_text = new char[len + 1];
	  memcpy(dnd_text, Fl::event_text(), len);
	  dnd_text[len] = '\0';
	  Fl::add_timeout(0.001, timer_cb, this); // delay message popup
	}
	ret = 1;
	break;
      case FL_DND_LEAVE:	// not strictly necessary to return(1) for this event
	label("..to\nhere");	// reset label
	fprintf(stderr, "FL_DND_LEAVE\n");
	dnd_inside = 0;		// status: mouse is outside, don't accept drop
	ret = 1;		// return(1) anyway..
	break;
    }
    return(ret);
  }

  // static timer callback
  static void timer_cb(void *data) {
    Receiver *r = (Receiver *)data;
    r->dnd_cb();
  }

  // dnd (FL_PASTE) popup method
  void dnd_cb() {
    if (dnd_text) {
      fl_message("%s", dnd_text);
      delete[] dnd_text;
      dnd_text = 0;
    }
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
