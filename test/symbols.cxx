//
// "$Id: symbols.cxx,v 1.4.2.3 2001/01/22 15:13:41 easysw Exp $"
//
// Symbol test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2001 by Bill Spitzak and others.
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
// Please report all bugs and problems to "fltk-bugs@fltk.org".
//

#include <stdlib.h>
#include <stdio.h>
#include <FL/Fl.H>
#include <FL/Fl_Single_Window.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>

int N = 0;
#define W 60
#define H 60
#define ROWS 5
#define COLS 5

Fl_Window *window;

void bt(const char *name) {
  int x = N%COLS;
  int y = N/COLS;
  N++;
  x = x*W+10;
  y = y*H+10;
  Fl_Box *a = new Fl_Box(FL_NO_BOX,x,y,W-20,H-20,name);
  a->align(FL_ALIGN_BOTTOM);
  a->labelsize(11);
  Fl_Box *b = new Fl_Box(FL_UP_BOX,x,y,W-20,H-20,name);
  b->labeltype(FL_SYMBOL_LABEL);
  b->labelcolor(FL_DARK3);
}

int main(int argc, char ** argv) {
  window = new Fl_Single_Window(COLS*W,ROWS*H+20);
bt("@->");
bt("@>");
bt("@>>");
bt("@>|");
bt("@>[]");
bt("@|>");
bt("@<-");
bt("@<");
bt("@<<");
bt("@|<");
bt("@[]<");
bt("@<|");
bt("@<->");
bt("@-->");
bt("@+");
bt("@->|");
bt("@||");
bt("@arrow");
bt("@returnarrow");
bt("@square");
bt("@circle");
bt("@line");
bt("@menu");
bt("@UpArrow");
bt("@DnArrow");
  window->resizable(window);
  window->show(argc,argv);
  return Fl::run();
}

//
// End of "$Id: symbols.cxx,v 1.4.2.3 2001/01/22 15:13:41 easysw Exp $".
//
