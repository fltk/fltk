//
// Keyboard/event test header for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
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

#ifndef keyboard_h
#  define keyboard_h
#  include <stdio.h>
#  include <FL/Fl.H>
#  include <FL/Fl_Window.H>

class MyWindow : public Fl_Window {
  int handle(int) FL_OVERRIDE;
public:
  MyWindow(int w, int h, const char *t=0L)
    : Fl_Window( w, h, t ) { }
};

#endif // !keyboard_h
