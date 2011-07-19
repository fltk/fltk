//
// "$Id$"
//
// Fl_Scroll test program for the Fast Light Tool Kit (FLTK).
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
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Box.H>
#include <string.h>
#include <stdio.h>
#include <FL/fl_draw.H>
#include <FL/math.h>

class Drawing : public Fl_Widget {
  void draw();
public:
  Drawing(int X,int Y,int W,int H,const char* L) : Fl_Widget(X,Y,W,H,L) {
    align(FL_ALIGN_TOP);
    box(FL_FLAT_BOX);
    color(FL_WHITE);
  }
};

void Drawing::draw() {
  draw_box();
  fl_push_matrix();
  fl_translate(x()+w()/2, y()+h()/2);
  fl_scale(w()/2, h()/2);
  fl_color(FL_BLACK);
  for (int i = 0; i < 20; i++) {
    for (int j = i+1; j < 20; j++) {
      fl_begin_line();
      fl_vertex(cos(M_PI*i/10+.1), sin(M_PI*i/10+.1));
      fl_vertex(cos(M_PI*j/10+.1), sin(M_PI*j/10+.1));
      fl_end_line();
    }
  }
  fl_pop_matrix();
}

Fl_Scroll* thescroll;

void box_cb(Fl_Widget* o, void*) {
  thescroll->box(((Fl_Button*)o)->value() ? FL_DOWN_FRAME : FL_NO_BOX);
  thescroll->redraw();
}

void type_cb(Fl_Widget*, void* v) {
  thescroll->type((uchar)((fl_intptr_t)v));
  thescroll->redraw();
}

Fl_Menu_Item choices[] = {
  {"0", 0, type_cb, (void*)0},
  {"HORIZONTAL", 0, type_cb, (void*)Fl_Scroll::HORIZONTAL},
  {"VERTICAL", 0, type_cb, (void*)Fl_Scroll::VERTICAL},
  {"BOTH", 0, type_cb, (void*)Fl_Scroll::BOTH},
  {"HORIZONTAL_ALWAYS", 0, type_cb, (void*)Fl_Scroll::HORIZONTAL_ALWAYS},
  {"VERTICAL_ALWAYS", 0, type_cb, (void*)Fl_Scroll::VERTICAL_ALWAYS},
  {"BOTH_ALWAYS", 0, type_cb, (void*)Fl_Scroll::BOTH_ALWAYS},
  {0}
};

void align_cb(Fl_Widget*, void* v) {
  thescroll->scrollbar.align((uchar)((fl_intptr_t)v));
  thescroll->redraw();
}

Fl_Menu_Item align_choices[] = {
  {"left+top", 0, align_cb, (void*)(FL_ALIGN_LEFT+FL_ALIGN_TOP)},
  {"left+bottom", 0, align_cb, (void*)(FL_ALIGN_LEFT+FL_ALIGN_BOTTOM)},
  {"right+top", 0, align_cb, (void*)(FL_ALIGN_RIGHT+FL_ALIGN_TOP)},
  {"right+bottom", 0, align_cb, (void*)(FL_ALIGN_RIGHT+FL_ALIGN_BOTTOM)},
  {0}
};

int main(int argc, char** argv) {
  Fl_Window window(5*75,400);
  window.box(FL_NO_BOX);
  Fl_Scroll scroll(0,0,5*75,300);

  int n = 0;
  for (int y=0; y<16; y++) for (int x=0; x<5; x++) {
    char buf[20]; sprintf(buf,"%d",n++);
    Fl_Button* b = new Fl_Button(x*75,y*25+(y>=8?5*75:0),75,25);
    b->copy_label(buf);
    b->color(n);
    b->labelcolor(FL_WHITE);
  }
  Drawing drawing(0,8*25,5*75,5*75,0);
  scroll.end();
  window.resizable(scroll);

  Fl_Box box(0,300,5*75,window.h()-300); // gray area below the scroll
  box.box(FL_FLAT_BOX);

  Fl_Light_Button but1(150, 310, 200, 25, "box");
  but1.callback(box_cb);
  
  Fl_Choice choice(150, 335, 200, 25, "type():");
  choice.menu(choices);
  choice.value(3);

  Fl_Choice achoice(150, 360, 200, 25, "scrollbar.align():");
  achoice.menu(align_choices);
  achoice.value(3);

  thescroll = &scroll;

  //scroll.box(FL_DOWN_BOX);
  //scroll.type(Fl_Scroll::VERTICAL);
  window.end();
  window.show(argc,argv);
  return Fl::run();
}

//
// End of "$Id$".
//
