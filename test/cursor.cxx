//
// "$Id$"
//
// Cursor test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Hor_Value_Slider.H>
#include <FL/Fl_Choice.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Box.H>

Fl_Cursor cursor = FL_CURSOR_DEFAULT;

Fl_Hor_Value_Slider *cursor_slider;

void choice_cb(Fl_Widget *, void *v) {
  cursor = (Fl_Cursor)(fl_intptr_t)v;
  cursor_slider->value(cursor);
  fl_cursor(cursor);
}

Fl_Menu_Item choices[] = {
  {"FL_CURSOR_DEFAULT",0,choice_cb,(void*)FL_CURSOR_DEFAULT},
  {"FL_CURSOR_ARROW",0,choice_cb,(void*)FL_CURSOR_ARROW},
  {"FL_CURSOR_CROSS",0,choice_cb,(void*)FL_CURSOR_CROSS},
  {"FL_CURSOR_WAIT",0,choice_cb,(void*)FL_CURSOR_WAIT},
  {"FL_CURSOR_INSERT",0,choice_cb,(void*)FL_CURSOR_INSERT},
  {"FL_CURSOR_HAND",0,choice_cb,(void*)FL_CURSOR_HAND},
  {"FL_CURSOR_HELP",0,choice_cb,(void*)FL_CURSOR_HELP},
  {"FL_CURSOR_MOVE",0,choice_cb,(void*)FL_CURSOR_MOVE},
  {"FL_CURSOR_NS",0,choice_cb,(void*)FL_CURSOR_NS},
  {"FL_CURSOR_WE",0,choice_cb,(void*)FL_CURSOR_WE},
  {"FL_CURSOR_NWSE",0,choice_cb,(void*)FL_CURSOR_NWSE},
  {"FL_CURSOR_NESW",0,choice_cb,(void*)FL_CURSOR_NESW},
  {"FL_CURSOR_N",0,choice_cb,(void*)FL_CURSOR_N},
  {"FL_CURSOR_NE",0,choice_cb,(void*)FL_CURSOR_NE},
  {"FL_CURSOR_E",0,choice_cb,(void*)FL_CURSOR_E},
  {"FL_CURSOR_SE",0,choice_cb,(void*)FL_CURSOR_SE},
  {"FL_CURSOR_S",0,choice_cb,(void*)FL_CURSOR_S},
  {"FL_CURSOR_SW",0,choice_cb,(void*)FL_CURSOR_SW},
  {"FL_CURSOR_W",0,choice_cb,(void*)FL_CURSOR_W},
  {"FL_CURSOR_NW",0,choice_cb,(void*)FL_CURSOR_NW},
  {"FL_CURSOR_NONE",0,choice_cb,(void*)FL_CURSOR_NONE},
  {0}
};

void setcursor(Fl_Widget *o, void *) {
  Fl_Hor_Value_Slider *slider = (Fl_Hor_Value_Slider *)o;
  cursor = Fl_Cursor((int)slider->value());
  fl_cursor(cursor);
}

// draw the label without any ^C or \nnn conversions:
class CharBox : public Fl_Box {
  void draw() {
    fl_font(FL_FREE_FONT,14);
    fl_draw(label(), x()+w()/2, y()+h()/2);
  }
public:
  CharBox(int X, int Y, int W, int H, const char* L) : Fl_Box(X,Y,W,H,L) {}
};

int main(int argc, char **argv) {
  Fl_Double_Window window(400,300);

  Fl_Choice choice(80,100,200,25,"Cursor:");
  choice.menu(choices);
  choice.callback(choice_cb);
  choice.when(FL_WHEN_RELEASE|FL_WHEN_NOT_CHANGED);

  Fl_Hor_Value_Slider slider1(80,180,310,30,"Cursor:");
  cursor_slider = &slider1;
  slider1.align(FL_ALIGN_LEFT);
  slider1.step(1);
  slider1.precision(0);
  slider1.bounds(0,255);
  slider1.value(0);
  slider1.callback(setcursor);
  slider1.value(cursor);

#if 0
  // draw the manual's diagram of cursors...
  window.size(400,800);
  int y = 300;
  Fl::set_font(FL_FREE_FONT, "cursor");
  char buf[100]; char *p = buf;
  for (Fl_Menu* m = choices; m->label(); m++) {
    Fl_Box* b = new Fl_Box(35,y,150,25,m->label());
    b->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    int n = (int)(m->argument());
    if (n == FL_CURSOR_NONE) break;
    if (n == FL_CURSOR_DEFAULT) n = FL_CURSOR_ARROW;
    p[0] = (char)((n-1)*2);
    p[1] = 0;
    b = new CharBox(15,y,20,20,p); p+=2;
    y += 25;
  }
#endif

  window.resizable(window);
  window.end();
  window.show(argc,argv);
  return Fl::run();
}

//
// End of "$Id$".
//
