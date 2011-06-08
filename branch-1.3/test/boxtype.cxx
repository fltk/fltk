//
// "$Id$"
//
// Boxtype test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
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

#include <stdlib.h>
#include <stdio.h>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>

int N = 0;
#define W 200
#define H 50
#define ROWS 14

Fl_Double_Window *window;

void bt(const char *name, Fl_Boxtype type, int square=0) {
  int x = N%4;
  int y = N/4;
  N++;
  x = x*W+10;
  y = y*H+10;
  Fl_Box *b = new Fl_Box(type,x,y,square ? H-20 : W-20,H-20,name);
  b->labelsize(11);
  if (square) b->align(FL_ALIGN_RIGHT);
}

int main(int argc, char ** argv) {
  window = new Fl_Double_Window(4*W,ROWS*H);
  window->box(FL_FLAT_BOX);
#if 0 // this code uses the command line arguments to set arbitrary color schemes
  Fl::args(argc, argv);
  Fl::get_system_colors();
#elif 0 // this code uses a single color to define a scheme
  Fl::args(argc, argv);
  Fl::get_system_colors();
  Fl::background(113,113,198);
#else // this code uses the nice bright blue background to show box vs. frame types
  Fl::args(argc, argv);
  Fl::get_system_colors();
  window->color(12);// light blue
#endif
  bt("FL_NO_BOX",FL_NO_BOX);
  bt("FL_FLAT_BOX",FL_FLAT_BOX);
  N += 2; // go to start of next row to line up boxes & frames
  bt("FL_UP_BOX",FL_UP_BOX);
  bt("FL_DOWN_BOX",FL_DOWN_BOX);
  bt("FL_UP_FRAME",FL_UP_FRAME);
  bt("FL_DOWN_FRAME",FL_DOWN_FRAME);
  bt("FL_THIN_UP_BOX",FL_THIN_UP_BOX);
  bt("FL_THIN_DOWN_BOX",FL_THIN_DOWN_BOX);
  bt("FL_THIN_UP_FRAME",FL_THIN_UP_FRAME);
  bt("FL_THIN_DOWN_FRAME",FL_THIN_DOWN_FRAME);
  bt("FL_ENGRAVED_BOX",FL_ENGRAVED_BOX);
  bt("FL_EMBOSSED_BOX",FL_EMBOSSED_BOX);
  bt("FL_ENGRAVED_FRAME",FL_ENGRAVED_FRAME);
  bt("FL_EMBOSSED_FRAME",FL_EMBOSSED_FRAME);
  bt("FL_BORDER_BOX",FL_BORDER_BOX);
  bt("FL_SHADOW_BOX",FL_SHADOW_BOX);
  bt("FL_BORDER_FRAME",FL_BORDER_FRAME);
  bt("FL_SHADOW_FRAME",FL_SHADOW_FRAME);
  bt("FL_ROUNDED_BOX",FL_ROUNDED_BOX);
  bt("FL_RSHADOW_BOX",FL_RSHADOW_BOX);
  bt("FL_ROUNDED_FRAME",FL_ROUNDED_FRAME);
  bt("FL_RFLAT_BOX",FL_RFLAT_BOX);
  bt("FL_OVAL_BOX",FL_OVAL_BOX);
  bt("FL_OSHADOW_BOX",FL_OSHADOW_BOX);
  bt("FL_OVAL_FRAME",FL_OVAL_FRAME);
  bt("FL_OFLAT_BOX",FL_OFLAT_BOX);
  bt("FL_ROUND_UP_BOX",FL_ROUND_UP_BOX);
  bt("FL_ROUND_DOWN_BOX",FL_ROUND_DOWN_BOX);
  bt("FL_DIAMOND_UP_BOX",FL_DIAMOND_UP_BOX);
  bt("FL_DIAMOND_DOWN_BOX",FL_DIAMOND_DOWN_BOX);

  bt("FL_PLASTIC_UP_BOX",FL_PLASTIC_UP_BOX);
  bt("FL_PLASTIC_DOWN_BOX",FL_PLASTIC_DOWN_BOX);
  bt("FL_PLASTIC_UP_FRAME",FL_PLASTIC_UP_FRAME);
  bt("FL_PLASTIC_DOWN_FRAME",FL_PLASTIC_DOWN_FRAME);
  bt("FL_PLASTIC_THIN_UP_BOX",FL_PLASTIC_THIN_UP_BOX);
  bt("FL_PLASTIC_THIN_DOWN_BOX",FL_PLASTIC_THIN_DOWN_BOX);
  N += 2;
  bt("FL_PLASTIC_ROUND_UP_BOX",FL_PLASTIC_ROUND_UP_BOX);
  bt("FL_PLASTIC_ROUND_DOWN_BOX",FL_PLASTIC_ROUND_DOWN_BOX);
  N += 2;

  bt("FL_GTK_UP_BOX",FL_GTK_UP_BOX);
  bt("FL_GTK_DOWN_BOX",FL_GTK_DOWN_BOX);
  bt("FL_GTK_UP_FRAME",FL_GTK_UP_FRAME);
  bt("FL_GTK_DOWN_FRAME",FL_GTK_DOWN_FRAME);
  bt("FL_GTK_THIN_UP_BOX",FL_GTK_THIN_UP_BOX);
  bt("FL_GTK_THIN_DOWN_BOX",FL_GTK_THIN_DOWN_BOX);
  bt("FL_GTK_THIN_UP_FRAME",FL_GTK_THIN_UP_FRAME);
  bt("FL_GTK_THIN_DOWN_FRAME",FL_GTK_THIN_DOWN_FRAME);
  bt("FL_GTK_ROUND_UP_BOX",FL_GTK_ROUND_UP_BOX);
  bt("FL_GTK_ROUND_DOWN_BOX",FL_GTK_ROUND_DOWN_BOX);
  window->resizable(window);
  window->end();
  window->show();
  return Fl::run();
}

//
// End of "$Id$".
//
