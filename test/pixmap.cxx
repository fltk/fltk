//	test how pixmap label type draws

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Pixmap.H>
#include <stdio.h>

#include "porsche.xpm"

#include <FL/Fl_Toggle_Button.H>

Fl_Toggle_Button *leftb,*rightb,*topb,*bottomb,*insideb;
Fl_Button *b;
Fl_Window *w;

void button_cb(Fl_Widget *,void *) {
  int i = 0;
  if (leftb->value()) i |= FL_ALIGN_LEFT;
  if (rightb->value()) i |= FL_ALIGN_RIGHT;
  if (topb->value()) i |= FL_ALIGN_TOP;
  if (bottomb->value()) i |= FL_ALIGN_BOTTOM;
  if (insideb->value()) i |= FL_ALIGN_INSIDE;
  b->align(i);
  w->redraw();
}

int dvisual = 0;
int arg(int, char **argv, int &i) {
  if (argv[i][1] == '8') {dvisual = 1; i++; return 1;}
  return 0;
}

#include <FL/Fl_Multi_Label.H>

Fl_Multi_Label multi = {
  0, "This is the text", 0, FL_NORMAL_LABEL
};

int main(int argc, char **argv) {
  int i = 1;
  if (Fl::args(argc,argv,i,arg) < argc)
    Fl::fatal(" -8 # : use default visual\n%s\n",Fl::help);

  Fl_Window window(400,400); ::w = &window;
  Fl_Button b(140,160,120,120,0); ::b = &b;
  (new Fl_Pixmap(porsche_xpm))->label(&b);
  multi.labela = b.label(); multi.typea = b.labeltype(); multi.label(&b);
  leftb = new Fl_Toggle_Button(50,75,50,25,"left");
  leftb->callback(button_cb);
  rightb = new Fl_Toggle_Button(100,75,50,25,"right");
  rightb->callback(button_cb);
  topb = new Fl_Toggle_Button(150,75,50,25,"top");
  topb->callback(button_cb);
  bottomb = new Fl_Toggle_Button(200,75,50,25,"bottom");
  bottomb->callback(button_cb);
  insideb = new Fl_Toggle_Button(250,75,50,25,"inside");
  insideb->callback(button_cb);
  if (!dvisual) Fl::visual(FL_RGB);
  window.resizable(window);
  window.end();
  window.show(argc,argv);
  return Fl::run();
}
