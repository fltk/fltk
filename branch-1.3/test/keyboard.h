//
// "$Id$"
//
// Keyboard/event test header for the Fast Light Tool Kit (FLTK).
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

#ifndef keyboard_h
#  define keyboard_h
#  include <stdio.h>
#  include <FL/Fl.H>
#  include <FL/Fl_Window.H>

class MyWindow : public Fl_Window {
  int handle(int);
public:
  MyWindow(int w, int h, const char *t=0L) 
    : Fl_Window( w, h, t ) { }
};

#endif // !keyboard_h


//
// End of "$Id$".
//
