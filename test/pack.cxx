//
// Fl_Pack or Fl_Flex test program for the Fast Light Tool Kit (FLTK).
//
// Rather crude test of the Fl_Pack or Fl_Flex class.
// Changing the type() of an Fl_Pack after it is displayed is not supported
// so I have to do a lot of resizing of things before that.
//
// Copyright 1998-2022 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//

#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Flex.H>

// This test program can be modified by two #define's below to test Fl_Flex
// as a drop-in replacement of Fl_Pack. The committed code should always use
// the default settings documented below.
//
// Edit the following 2 #define's to modify the test scenario:

#define USE_FLEX   0 // default 0 = use Fl_Pack, 1 = use Fl_Flex
#define USE_SCROLL 1 // default 1 = put Fl_Pack or Fl_Flex inside Fl_Scroll

// Do not edit #define's below

#if USE_FLEX
#define CONTAINER Fl_Flex
#define USE_PACK 0
#else
#define CONTAINER Fl_Pack
#define USE_PACK 1
#endif

CONTAINER *pack; // either Fl_Pack or Fl_Flex

#if USE_SCROLL
Fl_Scroll *scroll;
#endif

void type_cb(Fl_Light_Button *, long v) {
  for (int i = 0; i < pack->children(); i++) {
    Fl_Widget *o = pack->child(i);
    o->resize(0, 0, 25, 25);
  }
  pack->type(uchar(v));

#if USE_SCROLL
  pack->resize(scroll->x(), scroll->y(), scroll->w(), scroll->h());
#endif

#if USE_FLEX
  pack->layout();
#else
  pack->parent()->redraw();
  pack->redraw();
#endif
}

void spacing_cb(Fl_Value_Slider *o, long) {
  int s = int(o->value());
  pack->spacing(s);
#if USE_FLEX
  if (s > 4)
    pack->margin(4);
  else
    pack->margin(s);
  pack->layout();
#else
  pack->parent()->redraw();
#endif
}

int main(int argc, char **argv) {
  Fl_Double_Window *w = new Fl_Double_Window(360, 370);
#if USE_SCROLL
  scroll = new Fl_Scroll(10, 10, 340, 285);
#endif

  int nbuttons = 24;
  pack = new CONTAINER(10, 10, 340, 285);
#if (USE_FLEX)
  pack->box(FL_DOWN_BOX);
  nbuttons = 12;
#else
  pack->box(FL_DOWN_FRAME);
#endif

  // create buttons: position (xx, xx) will be "fixed" by Fl_Pack/Fl_Flex
  int xx = 35;
  for (int i = 0; i < nbuttons; i++) {
    char ltxt[8];
    snprintf(ltxt, 8, "b%d", i + 1);
    Fl_Button *b = new Fl_Button(xx, xx, 25, 25);
    b->copy_label(ltxt);
    xx += 10;
  }

  pack->end();
  w->resizable(pack);

#if USE_SCROLL
  scroll->end();
#endif
  {
    Fl_Light_Button *o = new Fl_Light_Button(10, 305, 165, 25, "HORIZONTAL");
    o->type(FL_RADIO_BUTTON);
    o->callback((Fl_Callback *)type_cb, (void *)(CONTAINER::HORIZONTAL));
  }
  {
    Fl_Light_Button *o = new Fl_Light_Button(185, 305, 165, 25, "VERTICAL");
    o->type(FL_RADIO_BUTTON);
    o->value(1);
    o->callback((Fl_Callback *)type_cb, (void *)(CONTAINER::VERTICAL));
  }
  {
    Fl_Value_Slider *o = new Fl_Value_Slider(100, 335, 250, 25, "Spacing: ");
    o->align(FL_ALIGN_LEFT);
    o->type(FL_HORIZONTAL);
    o->range(0, 30);
    o->step(1);
    o->callback((Fl_Callback *)spacing_cb);
  }
  w->end();
  w->size_range(300, 300);
  w->show(argc, argv);
  return Fl::run();
}
