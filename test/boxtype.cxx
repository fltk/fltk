//	produce the diagram of boxtypes for the documentation

#include <stdlib.h>
#include <stdio.h>
#include <FL/Fl.H>
#include <FL/Fl_Single_Window.H>
#include <FL/Fl_Box.H>

int N = 0;
#define W 150
#define H 50
#define ROWS 8

Fl_Window *window;

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
  window = new Fl_Single_Window(4*W,ROWS*H);
  window->box(FL_FLAT_BOX);
  window->color(12);// light blue
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
  window->resizable(window);
  window->end();
  window->show(argc,argv);
  return Fl::run();
}
