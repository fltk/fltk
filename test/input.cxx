//
// "$Id: input.cxx,v 1.4 1998/11/08 15:05:47 mike Exp $"
//
// Input field test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to "fltk-bugs@easysw.com".
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
  }
}

void button_cb(Fl_Widget *,void *) {
  for (int i=0; i<5; i++) test(input[i]);
}

void color_cb(Fl_Widget* button, void* v) {
  Fl_Color c;
  switch ((int)v) {
  case 0: c = FL_WHITE; break;
  case 1: c = FL_SELECTION_COLOR; break;
  default: c = FL_BLACK; break;
  }
  uchar r,g,b; Fl::get_color(c, r,g,b);
  if (fl_color_chooser(0,r,g,b)) {
    Fl::set_color(c,r,g,b); Fl::redraw();
    button->labelcolor(contrast(FL_BLACK,c));
    button->redraw();
  }
}

int main(int argc, char **argv) {
  Fl_Window *window = new Fl_Window(400,400);

  int y = 10;
  input[0] = new Fl_Input(70,y,300,30,"Normal:"); y += 35;
  // input[0]->cursor_color(FL_SELECTION_COLOR);
  //  input[0]->maximum_size(20);
  // input[0]->static_value("this is a testgarbage");
  input[1] = new Fl_Float_Input(70,y,300,30,"Float:"); y += 35;
  input[2] = new Fl_Int_Input(70,y,300,30,"Int:"); y += 35;
  input[3] = new Fl_Secret_Input(70,y,300,30,"Secret:"); y += 35;
  input[4] = new Fl_Multiline_Input(70,y,300,100,"Multiline:"); y += 105;

  for (int i = 0; i < 4; i++) {
    input[i]->when(0); input[i]->callback(cb);
  }
  int y1 = y;

  Fl_Button *b;
  b = new Fl_Toggle_Button(10,y,200,25,"FL_WHEN_&CHANGED");
  b->callback(toggle_cb, FL_WHEN_CHANGED); y += 25;
  b = new Fl_Toggle_Button(10,y,200,25,"FL_WHEN_&RELEASE");
  b->callback(toggle_cb, FL_WHEN_RELEASE); y += 25;
  b = new Fl_Toggle_Button(10,y,200,25,"FL_WHEN_&ENTER_KEY");
  b->callback(toggle_cb, FL_WHEN_ENTER_KEY); y += 25;
  b = new Fl_Toggle_Button(10,y,200,25,"FL_WHEN_&NOT_CHANGED");
  b->callback(toggle_cb, FL_WHEN_NOT_CHANGED); y += 25;
  y += 5;
  b = new Fl_Button(10,y,200,25,"&print changed()");
  b->callback(button_cb);

  b = new Fl_Button(220,y1,100,25,"color"); y1 += 25;
  b->color(input[0]->color()); b->callback(color_cb, (void*)0);
  b = new Fl_Button(220,y1,100,25,"selection_color"); y1 += 25;
  b->color(input[0]->selection_color()); b->callback(color_cb, (void*)1);
  b = new Fl_Button(220,y1,100,25,"textcolor"); y1 += 25;
  b->color(input[0]->textcolor()); b->callback(color_cb, (void*)2);
  b->labelcolor(contrast(FL_BLACK,b->color()));

  window->end();
  window->show(argc,argv);
  return Fl::run();
}

//
// End of "$Id: input.cxx,v 1.4 1998/11/08 15:05:47 mike Exp $".
//
