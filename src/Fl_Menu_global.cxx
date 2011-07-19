//
// "$Id$"
//
// Global menu shortcut code for the Fast Light Tool Kit (FLTK).
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

// Make all the shortcuts in this menu global.
// Currently only one menu at a time and you cannot destruct the menu,
// is this sufficient?

#include <FL/Fl.H>
#include <FL/Fl_Menu_.H>

static Fl_Menu_* the_widget;

static int handler(int e) {
  if (e != FL_SHORTCUT || Fl::modal()) return 0;
  Fl::first_window(the_widget->window());
  return the_widget->handle(e);
}

/**
  Make the shortcuts for this menu work no matter what window has the
  focus when you type it.  This is done by using 
  Fl::add_handler().  This Fl_Menu_ widget does not
  have to be visible (ie the window it is in can be hidden, or it does
  not have to be put in a window at all).
  <P>Currently there can be only one global()menu.  Setting a new
  one will replace the old one.  There is no way to remove the 
  global() setting (so don't destroy the widget!)
*/
void Fl_Menu_::global() {
  if (!the_widget) Fl::add_handler(handler);
  the_widget = this;
}

//
// End of "$Id$".
//
