//
// "$Id$"
//
// Another button test program for the Fast Light Tool Kit (FLTK).
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
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Repeat_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Tooltip.H>

int main(int argc, char ** argv) {
  Fl_Window *window = new Fl_Window(320,130);
  Fl_Button *b = new Fl_Button(10, 10, 130, 30, "Fl_Button");
  b->tooltip("This is a Tooltip.");
  new Fl_Return_Button(150, 10, 160, 30, "Fl_Return_Button");
  new Fl_Repeat_Button(10,50,130,30,"Fl_Repeat_Button");
  new Fl_Light_Button(10,90,130,30,"Fl_Light_Button");
  new Fl_Round_Button(150,50,160,30,"Fl_Round_Button");
  new Fl_Check_Button(150,90,160,30,"Fl_Check_Button");
  window->end();
  window->show(argc,argv);
  return Fl::run();
}

//
// End of "$Id$".
//
