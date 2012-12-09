//
// "$Id$"
//
// Shortcut header file for the Fast Light Tool Kit (FLTK).
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

#include <FL/Fl_Button.H>

class Shortcut_Button : public Fl_Button {
public:
  int svalue;
  int handle(int);
  void draw();
  Shortcut_Button(int X,int Y,int W,int H, const char* l = 0) :
    Fl_Button(X,Y,W,H,l) {svalue = 0;}
};

//
// End of "$Id$".
//
