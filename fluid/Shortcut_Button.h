//
// Shortcut header file for the Fast Light Tool Kit (FLTK).
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

#include <FL/Fl_Button.H>

class Shortcut_Button : public Fl_Button {
public:
  int svalue;
  int handle(int);
  void draw();
  Shortcut_Button(int X,int Y,int W,int H, const char* l = 0) :
    Fl_Button(X,Y,W,H,l) {svalue = 0;}
};
