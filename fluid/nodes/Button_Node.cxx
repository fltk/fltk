//
// Button Node code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2025 by Bill Spitzak and others.
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

/**
 \file Bottun_Node.cxx

 Node prototypes for Fl_Button based classes. Those are used by the Node
 Factory to generate the scene from project files or user input.
 */

#include "nodes/Button_Node.h"

#include "Fluid.h"
#include "app/Snap_Action.h"
#include "io/Project_Reader.h"
#include "io/Project_Writer.h"

#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Repeat_Button.H>
#include <FL/Fl_Round_Button.H>

#include <stdlib.h>

// ---- Button Nodes --------------------------------------------------- MARK: -


// ---- Button ----

Button_Node Button_Node::prototype;

static Fl_Menu_Item buttontype_menu[] = {
  {"Normal", 0, nullptr, (void*)nullptr},
  {"Toggle", 0, nullptr, (void*)FL_TOGGLE_BUTTON},
  {"Radio", 0, nullptr, (void*)FL_RADIO_BUTTON},
  {nullptr}
};

Fl_Menu_Item *Button_Node::subtypes() {
  return buttontype_menu;
}

void Button_Node::ideal_size(int &w, int &h) {
  auto layout = Fluid.proj.layout;
  h = layout->labelsize + 8;
  w = layout->labelsize * 4 + 8;
  fld::app::Snap_Action::better_size(w, h);
}

Fl_Widget *Button_Node::widget(int x, int y, int w, int h) {
  return new Fl_Button(x, y, w, h, "Button");
}

void Button_Node::write_properties(fld::io::Project_Writer &f) {
  Widget_Node::write_properties(f);
  Fl_Button *btn = (Fl_Button*)o;
  if (btn->compact()) {
    f.write_string("compact");
    f.write_string("%d", btn->compact());
  }
}

void Button_Node::read_property(fld::io::Project_Reader &f, const char *c) {
  Fl_Button *btn = (Fl_Button*)o;
  if (!strcmp(c, "compact")) {
    btn->compact((uchar)atol(f.read_word()));
  } else {
    Widget_Node::read_property(f, c);
  }
}

void Button_Node::copy_properties() {
  Widget_Node::copy_properties();
  Fl_Button *s = (Fl_Button*)o, *d = (Fl_Button*)live_widget;
  d->compact(s->compact());
}


// ---- Return Button ----

void Return_Button_Node::ideal_size(int &w, int &h) {
  auto layout = Fluid.proj.layout;
  h = layout->labelsize + 8;
  w = layout->labelsize * 4 + 8 + h; // make room for the symbol
  fld::app::Snap_Action::better_size(w, h);
}

Fl_Widget *Return_Button_Node::widget(int x, int y, int w, int h) {
  return new Fl_Return_Button(x, y, w, h, "Button");
}

Return_Button_Node Return_Button_Node::prototype;


// ---- Repeat Button ----

Fl_Widget *Repeat_Button_Node::widget(int x, int y, int w, int h) {
  return new Fl_Repeat_Button(x, y, w, h, "Button");
}

Repeat_Button_Node Repeat_Button_Node::prototype;


// ---- Light Button ----

void Light_Button_Node::ideal_size(int &w, int &h) {
  auto layout = Fluid.proj.layout;
  h = layout->labelsize + 8;
  w = layout->labelsize * 4 + 8 + layout->labelsize; // make room for the light
  fld::app::Snap_Action::better_size(w, h);
}

Fl_Widget *Light_Button_Node::widget(int x, int y, int w, int h) {
  return new Fl_Light_Button(x, y, w, h, "Button");
}

Light_Button_Node Light_Button_Node::prototype;


// ---- Check Button ----

void Check_Button_Node::ideal_size(int &w, int &h) {
  auto layout = Fluid.proj.layout;
  h = layout->labelsize + 8;
  w = layout->labelsize * 4 + 8 + layout->labelsize; // make room for the symbol
  fld::app::Snap_Action::better_size(w, h);
}

Fl_Widget *Check_Button_Node::widget(int x, int y, int w, int h) {
  return new Fl_Check_Button(x, y, w, h, "Button");
}

Check_Button_Node Check_Button_Node::prototype;


// ---- Round Button ----

void Round_Button_Node::ideal_size(int &w, int &h) {
  auto layout = Fluid.proj.layout;
  h = layout->labelsize + 8;
  w = layout->labelsize * 4 + 8 + layout->labelsize; // make room for the symbol
  fld::app::Snap_Action::better_size(w, h);
}

Fl_Widget *Round_Button_Node::widget(int x, int y, int w, int h) {
  return new Fl_Round_Button(x, y, w, h, "Button");
}

Round_Button_Node Round_Button_Node::prototype;

