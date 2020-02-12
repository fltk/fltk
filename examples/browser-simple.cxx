//
// "$Id$"
//
//	Simple Fl_Browser widget example. - erco 07/26/2019
//
// Copyright 2019 Greg Ercolano.
// Copyright 1998-2016 by Bill Spitzak and others.
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
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Multi_Browser.H>

// Hold Browser's callback
//    Invoked whenever an item clicked
//
void HoldBrowserCallback(Fl_Widget *w, void *data) {
  Fl_Hold_Browser *brow = (Fl_Hold_Browser*)w;
  int line = brow->value();
  printf("[hold browser] item %d picked: %s\n", line, brow->text(line));
}

// Multi Browser's callback
//    Invoked whenever an item(s) clicked/selected
//
void MultiBrowserCallback(Fl_Widget *w, void *data) {
  Fl_Multi_Browser *brow = (Fl_Multi_Browser*)w;
  // Multi browser can have many items selected, so print all selected
  for ( int t=1; t<=brow->size(); t++ )
    if ( brow->selected(t) ) 
      printf("[multi browser] item %d selected: %s\n", t, brow->text(t));
  printf("\n");
}

int main(int argc, char *argv[]) {
  Fl::scheme("gtk+");
  Fl_Double_Window *win = new Fl_Double_Window(250, 220, "Simple Browser");
  win->begin();
  {
    {
      // Create Hold Browser
      Fl_Hold_Browser *brow = new Fl_Hold_Browser(10, 10, win->w()-20, 80, "Hold");
      brow->callback(HoldBrowserCallback);	// callback for hold browser
      // Add some items
      brow->add("One");
      brow->add("Two");
      brow->add("Three");
      brow->add("Four");
      // Preselect first item "One"
      brow->select(1);
    }
    {
      // Create Multi Browser
      Fl_Multi_Browser *brow = new Fl_Multi_Browser(10, 120, win->w()-20, 80, "Multi");
      brow->callback(MultiBrowserCallback);	// callback for multi browser
      // Add some items
      brow->add("Aaa");
      brow->add("Bbb");
      brow->add("Ccc");
      brow->add("Ddd");
      // Preselect first two items "Aaa" and "Bbb"
      brow->select(1);
      brow->select(2);
    }
  }
  win->end();
  win->resizable(win);
  win->show(argc, argv);
  return(Fl::run());
}

//
// End of "$Id$".
//
