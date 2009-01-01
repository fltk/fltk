//
// "$Id$"
//
// Iconize test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2009 by Bill Spitzak and others.
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
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <stdlib.h>

void iconize_cb(Fl_Widget *, void *v) {
  Fl_Window *w = (Fl_Window *)v;
  w->iconize();
}

void show_cb(Fl_Widget *, void *v) {
  Fl_Window *w = (Fl_Window *)v;
  w->show();
}

void hide_cb(Fl_Widget *, void *v) {
  Fl_Window *w = (Fl_Window *)v;
  w->hide();
}

void window_cb(Fl_Widget*, void*) {
  exit(0);
}

int main(int argc, char **argv) {

  Fl_Window mainw(200,200);
  mainw.end();
  mainw.show(argc,argv);

  Fl_Window control(120,120);

  Fl_Button hide_button(0,0,120,30,"hide()");
  hide_button.callback(hide_cb, &mainw);

  Fl_Button iconize_button(0,30,120,30,"iconize()");
  iconize_button.callback(iconize_cb, &mainw);

  Fl_Button show_button(0,60,120,30,"show()");
  show_button.callback(show_cb, &mainw);

  Fl_Button show_button2(0,90,120,30,"show this");
  show_button2.callback(show_cb, &control);

  //  Fl_Box box(FL_NO_BOX,0,60,120,30,"Also try running\nwith -i switch");

  control.end();
  control.show();
  control.callback(window_cb);
  return Fl::run();
}

//
// End of "$Id$".
//
