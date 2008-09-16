//
// "$Id$"
//
// Pixmap label test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2008 by Bill Spitzak and others.
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
#include <FL/Fl_Pixmap.H>
#include <stdio.h>

#include "pixmaps/porsche.xpm"

#include <FL/Fl_Toggle_Button.H>

Fl_Toggle_Button *leftb,*rightb,*topb,*bottomb,*insideb,*overb,*inactb;
Fl_Button *b;
Fl_Window *w;

void button_cb(Fl_Widget *,void *) {
  int i = 0;
  if (leftb->value()) i |= FL_ALIGN_LEFT;
  if (rightb->value()) i |= FL_ALIGN_RIGHT;
  if (topb->value()) i |= FL_ALIGN_TOP;
  if (bottomb->value()) i |= FL_ALIGN_BOTTOM;
  if (insideb->value()) i |= FL_ALIGN_INSIDE;
  if (overb->value()) i |= FL_ALIGN_TEXT_OVER_IMAGE;
  b->align(i);
  if (inactb->value()) b->deactivate();
  else b->activate();
  w->redraw();
}

int dvisual = 0;
int arg(int, char **argv, int &i) {
  if (argv[i][1] == '8') {dvisual = 1; i++; return 1;}
  return 0;
}

#include <FL/Fl_Multi_Label.H>

int main(int argc, char **argv) {
  int i = 1;
  if (Fl::args(argc,argv,i,arg) < argc)
    Fl::fatal(" -8 # : use default visual\n%s\n",Fl::help);

  Fl_Window window(400,400); ::w = &window;
  Fl_Button b(140,160,120,120,"Pixmap"); ::b = &b;
  Fl_Pixmap *pixmap = new Fl_Pixmap(porsche_xpm);
  Fl_Pixmap *depixmap;
  depixmap = (Fl_Pixmap *)pixmap->copy();
  depixmap->inactive();

  b.image(pixmap);
  b.deimage(depixmap);

  leftb = new Fl_Toggle_Button(25,50,50,25,"left");
  leftb->callback(button_cb);
  rightb = new Fl_Toggle_Button(75,50,50,25,"right");
  rightb->callback(button_cb);
  topb = new Fl_Toggle_Button(125,50,50,25,"top");
  topb->callback(button_cb);
  bottomb = new Fl_Toggle_Button(175,50,50,25,"bottom");
  bottomb->callback(button_cb);
  insideb = new Fl_Toggle_Button(225,50,50,25,"inside");
  insideb->callback(button_cb);
  overb = new Fl_Toggle_Button(25,75,100,25,"text over");
  overb->callback(button_cb);
  inactb = new Fl_Toggle_Button(125,75,100,25,"inactive");
  inactb->callback(button_cb);
  if (!dvisual) Fl::visual(FL_RGB);
  window.resizable(window);
  window.end();
  window.show(argc,argv);
  return Fl::run();
}

//
// End of "$Id$".
//
