//
// "$Id$"
//
// Single-buffered window for the Fast Light Tool Kit (FLTK).
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

//	A window with a single-buffered context
//
//	This is provided for systems where the base class is double
//	buffered.  You can turn it off using this subclass in case
//	your display looks better without it.

#include <FL/Fl_Single_Window.H>


void Fl_Single_Window::show() 
{
  Fl_Window::show();
}


void Fl_Single_Window::flush() 
{
  Fl_Window::flush();
}


Fl_Single_Window::Fl_Single_Window(int W, int H, const char *l)
: Fl_Window(W,H,l) 
{
}


Fl_Single_Window::Fl_Single_Window(int X, int Y, int W, int H, const char *l)
: Fl_Window(X,Y,W,H,l) 
{
}


//
// End of "$Id$".
//
