//
// "$Id: file_chooser.cxx,v 1.4.2.3 2001/01/22 15:13:41 easysw Exp $"
//
// File chooser test program for the Fast Light Tool Kit (FLTK).
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

#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Input.H>
#include <FL/fl_file_chooser.H>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

Fl_Input *pattern, *current;

void pickfile(Fl_Widget *) {
  const char *p;
  p = fl_file_chooser("Pick a file",pattern->value(),current->value());
  if (p) current->value(p);
}

void thecb(const char *name) {
  printf("Callback '%s'\n",name);
}

int main(int argc, char **argv) {
  Fl_Window window(400,200);
  pattern = new Fl_Input(100,50,280,30,"Pattern:");
  pattern->static_value("*");
  current = new Fl_Input(100,90,280,30,"Current:");
  Fl_Button button(100,120,100,30,"&Choose file");
  button.callback(pickfile);
  window.end();
  window.show(argc, argv);
  fl_file_chooser_callback(thecb);
  return Fl::run();
}

//
// End of "$Id: file_chooser.cxx,v 1.4.2.3 2001/01/22 15:13:41 easysw Exp $".
//
