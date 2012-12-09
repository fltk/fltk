//
// "$Id$"
//
// Fl_Pack test program for the Fast Light Tool Kit (FLTK).
//
// Rather crude test of the Fl_Pack object.
// Changing the type() of an Fl_Pack after it is displayed is not supported
// so I have to do a lot of resizing of things before that.
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
#include <FL/Fl_Button.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Pack.H>

Fl_Pack *pack;
Fl_Scroll *scroll;

void type_cb(Fl_Light_Button*, long v) {
  for (int i = 0; i < pack->children(); i++) {
    Fl_Widget* o = pack->child(i);
    o->resize(0,0,25,25);
  }
  pack->resize(scroll->x(),scroll->y(),scroll->w(),scroll->h());
  pack->parent()->redraw();
  pack->type(uchar(v));
  pack->redraw();
}

void spacing_cb(Fl_Value_Slider*o, long) {
  pack->spacing(int(o->value()));
  scroll->redraw();
}

int main(int argc, char **argv) {
 Fl_Double_Window *w;
 {Fl_Double_Window* o = new Fl_Double_Window(360, 370);
  w = o;
  scroll = new Fl_Scroll(10,10,340,285);
 {Fl_Pack* o = new Fl_Pack(10, 10, 340, 285);
  pack = o;
  o->box(FL_DOWN_FRAME);
  //o->box(FL_ENGRAVED_FRAME);
 new Fl_Button(35, 35, 25, 25, "b1");
 new Fl_Button(45, 45, 25, 25, "b2");
 new Fl_Button(55, 55, 25, 25, "b3");
 new Fl_Button(65, 65, 25, 25, "b4");
 new Fl_Button(75, 75, 25, 25, "b5");
 new Fl_Button(85, 85, 25, 25, "b6");
 new Fl_Button(95, 95, 25, 25, "b7");
 new Fl_Button(105, 105, 25, 25, "b8");
 new Fl_Button(115, 115, 25, 25, "b9");
 new Fl_Button(125, 125, 25, 25, "b10");
 new Fl_Button(135, 135, 25, 25, "b11");
 new Fl_Button(145, 145, 25, 25, "b12");
 new Fl_Button(155, 155, 25, 25, "b13");
 new Fl_Button(165, 165, 25, 25, "b14");
 new Fl_Button(175, 175, 25, 25, "b15");
 new Fl_Button(185, 185, 25, 25, "b16");
 new Fl_Button(195, 195, 25, 25, "b17");
 new Fl_Button(205, 205, 25, 25, "b18");
 new Fl_Button(215, 215, 25, 25, "b19");
 new Fl_Button(225, 225, 25, 25, "b20");
 new Fl_Button(235, 235, 25, 25, "b21");
 new Fl_Button(245, 245, 25, 25, "b22");
 new Fl_Button(255, 255, 25, 25, "b23");
 new Fl_Button(265, 265, 25, 25, "b24");
  o->end();
  w->resizable(o);
 }
 scroll->end();
 {Fl_Light_Button* o = new Fl_Light_Button(10, 305, 165, 25, "HORIZONTAL");
 o->type(FL_RADIO_BUTTON);
  o->callback((Fl_Callback*)type_cb, (void*)(Fl_Pack::HORIZONTAL));
 }
 {Fl_Light_Button* o = new Fl_Light_Button(185, 305, 165, 25, "VERTICAL");
 o->type(FL_RADIO_BUTTON);
 o->value(1);
  o->callback((Fl_Callback*)type_cb, (void*)(Fl_Pack::VERTICAL));
 }
 {Fl_Value_Slider* o = new Fl_Value_Slider(100, 335, 250, 25, "Spacing: ");
 o->align(FL_ALIGN_LEFT);
 o->type(FL_HORIZONTAL);
 o->range(0,30);
 o->step(1);
 o->callback((Fl_Callback*)spacing_cb);
 }
 w->end();
 }
 w->show(argc, argv);
 return Fl::run();
}

//
// End of "$Id$".
//
