//
// "$Id$"
//
// Input field test program for the Fast Light Tool Kit (FLTK).
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

#include <stdio.h>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Secret_Input.H>
#include <FL/Fl_Multiline_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Toggle_Button.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Color_Chooser.H>

void cb(Fl_Widget *ob) {
  printf("Callback for %s '%s'\n",ob->label(),((Fl_Input*)ob)->value());
}

int when = 0;
Fl_Input *input[5];

void toggle_cb(Fl_Widget *o, long v) {
  if (((Fl_Toggle_Button*)o)->value()) when |= v; else when &= ~v;
  for (int i=0; i<5; i++) input[i]->when(when);
}

void test(Fl_Input *i) {
  if (i->changed()) {
    i->clear_changed(); printf("%s '%s'\n",i->label(),i->value());
    char utf8buf[10];
    int last = fl_utf8encode(i->index(i->position()), utf8buf);
    utf8buf[last] = 0;
    printf("Symbol at cursor position: %s\n", utf8buf);
  }
}

void button_cb(Fl_Widget *,void *) {
  for (int i=0; i<5; i++) test(input[i]);
}

void color_cb(Fl_Widget* button, void* v) {
  Fl_Color c;
  switch ((fl_intptr_t)v) {
  case 0: c = FL_BACKGROUND2_COLOR; break;
  case 1: c = FL_SELECTION_COLOR; break;
  default: c = FL_FOREGROUND_COLOR; break;
  }
  uchar r,g,b; Fl::get_color(c, r,g,b);
  if (fl_color_chooser(0,r,g,b)) {
    Fl::set_color(c,r,g,b); Fl::redraw();
    button->labelcolor(fl_contrast(FL_BLACK,c));
    button->redraw();
  }
}

void tabnav_cb(Fl_Widget *w, void *v) {
  Fl_Light_Button *b = (Fl_Light_Button*)w;
  Fl_Multiline_Input *fmi = (Fl_Multiline_Input*)v;
  fmi->tab_nav(b->value() ? 1 : 0);
}

void arrownav_cb(Fl_Widget *w, void *v) {
  Fl_Light_Button *b = (Fl_Light_Button*)w;
  Fl::option(Fl::OPTION_ARROW_FOCUS, b->value() ? true : false);
}

int main(int argc, char **argv) {
  // the following two lines set the correct color scheme, so that 
  // calling fl_contrast below will return good results
  Fl::args(argc, argv);
  Fl::get_system_colors();
  Fl_Window *window = new Fl_Window(400,420);

  int y = 10;
  input[0] = new Fl_Input(70,y,300,30,"Normal:"); y += 35;
  input[0]->tooltip("Normal input field");
  // input[0]->cursor_color(FL_SELECTION_COLOR);
  // input[0]->maximum_size(20);
  // input[0]->static_value("this is a testgarbage");
  input[1] = new Fl_Float_Input(70,y,300,30,"Float:"); y += 35;
  input[1]->tooltip("Input field for floating-point number (F1)");
  input[1]->shortcut(FL_F+1);
  input[2] = new Fl_Int_Input(70,y,300,30,"Int:"); y += 35;
  input[2]->tooltip("Input field for integer number (F2)");
  input[2]->shortcut(FL_F+2);
  input[3] = new Fl_Secret_Input(70,y,300,30,"&Secret:"); y += 35;
  input[3]->tooltip("Input field for password (Alt-S)");
  input[4] = new Fl_Multiline_Input(70,y,300,100,"&Multiline:"); y += 105;
  input[4]->tooltip("Input field for short text with newlines (Alt-M)");
  input[4]->wrap(1);

  for (int i = 0; i < 4; i++) {
    input[i]->when(0); input[i]->callback(cb);
  }
  int y1 = y;

  Fl_Button *b;
  b = new Fl_Toggle_Button(10,y,200,25,"FL_WHEN_CHANGED");
  b->callback(toggle_cb, FL_WHEN_CHANGED); y += 25;
  b->tooltip("Do callback each time the text changes");
  b = new Fl_Toggle_Button(10,y,200,25,"FL_WHEN_RELEASE");
  b->callback(toggle_cb, FL_WHEN_RELEASE); y += 25;
  b->tooltip("Do callback when widget loses focus");
  b = new Fl_Toggle_Button(10,y,200,25,"FL_WHEN_ENTER_KEY");
  b->callback(toggle_cb, FL_WHEN_ENTER_KEY); y += 25;
  b->tooltip("Do callback when user hits Enter key");
  b = new Fl_Toggle_Button(10,y,200,25,"FL_WHEN_NOT_CHANGED");
  b->callback(toggle_cb, FL_WHEN_NOT_CHANGED); y += 25;
  b->tooltip("Do callback even if the text is not changed");
  y += 5;
  b = new Fl_Button(10,y,200,25,"&print changed()"); y += 25;
  b->callback(button_cb);
  b->tooltip("Print widgets that have changed() flag set");

  b = new Fl_Light_Button(10,y,100,25," Tab Nav");
  b->tooltip("Control tab navigation for the multiline input field");
  b->callback(tabnav_cb, (void*)input[4]);
  b->value(input[4]->tab_nav() ? 1 : 0);
  b = new Fl_Light_Button(110,y,100,25," Arrow Nav"); y += 25;
  b->tooltip("Control horizontal arrow key focus navigation behavior.\n"
             "e.g. Fl::OPTION_ARROW_FOCUS");
  b->callback(arrownav_cb);
  b->value(input[4]->tab_nav() ? 1 : 0);
  b->value(Fl::option(Fl::OPTION_ARROW_FOCUS) ? 1 : 0);

  b = new Fl_Button(220,y1,120,25,"color"); y1 += 25;
  b->color(input[0]->color()); b->callback(color_cb, (void*)0);
  b->tooltip("Color behind the text");
  b = new Fl_Button(220,y1,120,25,"selection_color"); y1 += 25;
  b->color(input[0]->selection_color()); b->callback(color_cb, (void*)1);
  b->labelcolor(fl_contrast(FL_BLACK,b->color()));
  b->tooltip("Color behind selected text");
  b = new Fl_Button(220,y1,120,25,"textcolor"); y1 += 25;
  b->color(input[0]->textcolor()); b->callback(color_cb, (void*)2);
  b->labelcolor(fl_contrast(FL_BLACK,b->color()));
  b->tooltip("Color of the text");

  window->end();
  window->show(argc,argv);
  return Fl::run();
}

//
// End of "$Id$".
//
