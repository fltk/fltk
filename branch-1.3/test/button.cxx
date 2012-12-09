//
// "$Id$"
//
// Button/callback test program for the Fast Light Tool Kit (FLTK).
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

#include <stdlib.h>
#include <stdio.h>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/fl_ask.H>

void beepcb(Fl_Widget *, void *) {
  fl_beep();
  fflush(stdout);
}

void exitcb(Fl_Widget *, void *) {
  exit(0);
}

#if 0
// test Fl::add_fd()...
void stdin_cb(int, void*) {
  char buf[1000];
  fgets(buf, sizeof(buf), stdin);
  printf("stdin callback\n");
}
#endif

int main(int argc, char ** argv) {
  Fl_Window *window = new Fl_Window(320,65);
  Fl_Button *b1 = new Fl_Button(20, 20, 80, 25, "&Beep");
  b1->callback(beepcb,0);
  /*Fl_Button *b2 =*/ new Fl_Button(120,20, 80, 25, "&no op");
  Fl_Button *b3 = new Fl_Button(220,20, 80, 25, "E&xit");
  b3->callback(exitcb,0);
  window->end();
  window->show(argc,argv);
#if 0
  Fl::add_fd(0, stdin_cb);
#endif
  return Fl::run();
}

//
// End of "$Id$".
//
