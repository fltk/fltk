//
// "$Id: symbols.cxx,v 1.4.2.3.2.5 2003/01/30 21:46:03 easysw Exp $"
//
// Symbol test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2003 by Bill Spitzak and others.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <FL/Fl.H>
#include <FL/Fl_Single_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/fl_draw.H>

int N = 0;
#define W 70
#define H 70
#define ROWS 5
#define COLS 5

Fl_Window *window;

void slider_cb(Fl_Widget *w, void *) {
  static char buf[80];
  int val = (int)(((Fl_Value_Slider*)w)->value());
  Fl_Window *win = (Fl_Window*)w->parent();       // get parent window
  for (int i = win->children(); i--; ) {          // all window children
    Fl_Widget *wc = win->child(i);
    const char *l = wc->label();
    if ( *l == '@' ) {                            // all children with '@'
      if ( *(++l) == '@' ) {                      // ascii legend?
        l++;
	while (isdigit(*l)) { l++; }
        if (val == 0) { sprintf(buf, "@@%s", l); }
        else          { sprintf(buf, "@@%d%s", val, l); }
      } else {                                    // box with symbol
        while (isdigit(*l)) { l++; }
        if (val == 0) { sprintf(buf, "@%s", l); }
	else          { sprintf(buf, "@%d%s", val, l); }
      }
      free((void*)(wc->label()));
      wc->label(strdup(buf));
    }
  }
  win->redraw();
}

void bt(const char *name) {
  int x = N%COLS;
  int y = N/COLS;
  char buf[255];
  N++;
  x = x*W+10;
  y = y*H+10;
  sprintf(buf, "@%s", name);
  Fl_Box *a = new Fl_Box(FL_NO_BOX,x,y,W-20,H-20,strdup(buf));
  a->align(FL_ALIGN_BOTTOM);
  a->labelsize(11);
  Fl_Box *b = new Fl_Box(FL_UP_BOX,x,y,W-20,H-20,strdup(name));
  b->labelcolor(FL_DARK3);
}

int main(int argc, char ** argv) {
  window = new Fl_Single_Window(COLS*W,ROWS*H+60);
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

  Fl_Value_Slider slider((int)(window->w()*.10+.5),
                         window->h()-40,
                         (int)(window->w()*.80+.5),
                         16,
                         "Orientation");
  slider.type(FL_HORIZONTAL);
  slider.range(0.0, 9.0);
  slider.value(0.0);
  slider.step(1);
  slider.callback(slider_cb, &slider);

  window->resizable(window);
  window->show(argc,argv);
  return Fl::run();
}

//
// End of "$Id: symbols.cxx,v 1.4.2.3.2.5 2003/01/30 21:46:03 easysw Exp $".
//
