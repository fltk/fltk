//	produce diagram used in the documentation:

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
