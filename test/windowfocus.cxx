//
// "$Id$"
//
// Cross-window show/focus test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2015 by Bill Spitzak and others.
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
#include <FL/Fl_Box.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Input.H>

static Fl_Double_Window *win1, *win2;
static Fl_Input *input1;

static void popup(Fl_Widget *, void *) {

  win2->position(win1->x() + win1->w(), win1->y());

  win2->show();
  win2->wait_for_expose();
  input1->take_focus();
}

int main(int argc, char **argv) {

  win1 = new Fl_Double_Window(300, 200);
  win1->label("show() focus test");

  Fl_Box *b = new Fl_Box(10, 10, 280, 130);
  b->label("Type something to pop the subwindow up. "
	   "The focus should stay on the input, "
	   "and you should be able to continue typing.");
  b->align(FL_ALIGN_WRAP | FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

  input1 = new Fl_Input(10, 150, 150, 25);
  input1->when(FL_WHEN_CHANGED);
  input1->callback(popup);

  win1->end();

  win2 = new Fl_Double_Window(300, 200);
  win2->label("window2");
  win2->end();

  win1->show(argc,argv);

  return Fl::run();
}

//
// End of "$Id$".
//
