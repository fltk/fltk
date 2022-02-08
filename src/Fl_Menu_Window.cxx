//
// Menu window code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2016 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//

// This is the window type used by Fl_Menu to make the pop-ups.
// It draws in the overlay planes if possible.

// Also here is the implementation of the mouse & keyboard grab,
// which are used so that clicks outside the program's windows
// can be used to dismiss the menus.

#include <FL/Fl_Menu_Window.H>
#include "Fl_Window_Driver.H"

/**  Destroys the window and all of its children.*/
Fl_Menu_Window::~Fl_Menu_Window() {
  hide();
}


Fl_Menu_Window::Fl_Menu_Window(int W, int H, const char *l)
: Fl_Single_Window(W,H,l)
{
  image(0);
}


Fl_Menu_Window::Fl_Menu_Window(int X, int Y, int W, int H, const char *l)
: Fl_Single_Window(X,Y,W,H,l) {
  image(0);
}
