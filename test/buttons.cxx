//
// Another button test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2022 by Bill Spitzak and others.
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

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Repeat_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Scheme_Choice.H>

int main(int argc, char **argv) {
  Fl_Window *window = new Fl_Window(420, 170);
  Fl_Button *b1 = new Fl_Button(10, 10, 130, 30, "Fl_Button");
  b1->tooltip("Fl_Button");
  Fl_Button *b2 = new Fl_Return_Button(150, 10, 160, 30, "Fl_Return_Button");
  b2->tooltip("Fl_Return_Button");
  Fl_Button *b3 = new Fl_Repeat_Button(10, 50, 130, 30, "Fl_Repeat_Button");
  b3->tooltip("Fl_Repeat_Button");
  Fl_Button *b4 = new Fl_Round_Button(150, 50, 160, 30, "Fl_Round_Button");
  b4->tooltip("Fl_Round_Button");
  Fl_Button *b5 = new Fl_Light_Button(10, 90, 130, 30, "Fl_Light_Button");
  b5->tooltip("Fl_Light_Button");
  Fl_Button *b6 = new Fl_Check_Button(150, 90, 160, 30, "Fl_Check_Button");
  b6->tooltip("Fl_Check_Button");

  Fl_Group *keypad = new Fl_Group(320, 10, 90, 120);
  Fl_Button *kp[11];
  kp[7] = new Fl_Button(320,  10, 30, 30, "7");
  kp[8] = new Fl_Button(350,  10, 30, 30, "8");
  kp[9] = new Fl_Button(380,  10, 30, 30, "9");
  kp[4] = new Fl_Button(320,  40, 30, 30, "4");
  kp[5] = new Fl_Button(350,  40, 30, 30, "5");
  kp[6] = new Fl_Button(380,  40, 30, 30, "6");
  kp[1] = new Fl_Button(320,  70, 30, 30, "1");
  kp[2] = new Fl_Button(350,  70, 30, 30, "2");
  kp[3] = new Fl_Button(380,  70, 30, 30, "3");
  kp[0] = new Fl_Button(320, 100, 60, 30, "0");
  kp[10] = new Fl_Button(380, 100, 30, 30, ".");
  for (int i=0; i<11; i++) {
    kp[i]->compact(1);
    kp[i]->selection_color(FL_SELECTION_COLOR);
  }
  keypad->end();

  // Add a scheme choice widget for easier testing. Position the widget at
  // the right window border so the menu popup doesn't cover the check boxes etc.
  Fl_Scheme_Choice *scheme_choice = new Fl_Scheme_Choice(180, 130, 130, 30, "Active FLTK Scheme:");
  scheme_choice->tooltip("Fl_Scheme_Choice");

  window->end();
  window->resizable(window);
  window->size_range(320, 130);
  window->show(argc, argv);
  return Fl::run();
}
