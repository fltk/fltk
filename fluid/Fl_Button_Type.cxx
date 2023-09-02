//
// Button type factory code for the Fast Light Tool Kit (FLTK).
//
// Type classes for most of the fltk widgets.  Most of the work
// is done by code in Fl_Widget_Type.C.  Also a factory instance
// of each of these type classes.
//
// This file also contains the "new" menu, which has a pointer
// to a factory instance for every class (both the ones defined
// here and ones in other files)
//
// Copyright 1998-2023 by Bill Spitzak and others.
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

#include "Fl_Button_Type.h"

#include "Fd_Snap_Action.h"
#include "file.h"

#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Repeat_Button.H>
#include <FL/Fl_Round_Button.H>

#include <stdlib.h>



// ---- Button Types --------------------------------------------------- MARK: -


// ---- Button ----

static Fl_Menu_Item buttontype_menu[] = {
  {"Normal", 0, 0, (void*)0},
  {"Toggle", 0, 0, (void*)FL_TOGGLE_BUTTON},
  {"Radio", 0, 0, (void*)FL_RADIO_BUTTON},
  {0}
};

Fl_Menu_Item *Fl_Button_Type::subtypes() {
  return buttontype_menu;
}

void Fl_Button_Type::ideal_size(int &w, int &h) {
  h = layout->labelsize + 8;
  w = layout->labelsize * 4 + 8;
  Fd_Snap_Action::better_size(w, h);
}

Fl_Widget *Fl_Button_Type::widget(int x, int y, int w, int h) {
  return new Fl_Button(x, y, w, h, "Button");
}

void Fl_Button_Type::write_properties(Fd_Project_Writer &f) {
  Fl_Widget_Type::write_properties(f);
  Fl_Button *btn = (Fl_Button*)o;
  if (btn->compact()) {
    f.write_string("compact");
    f.write_string("%d", btn->compact());
  }
}

void Fl_Button_Type::read_property(Fd_Project_Reader &f, const char *c) {
  Fl_Button *btn = (Fl_Button*)o;
  if (!strcmp(c, "compact")) {
    btn->compact((uchar)atol(f.read_word()));
  } else {
    Fl_Widget_Type::read_property(f, c);
  }
}

void Fl_Button_Type::copy_properties() {
  Fl_Widget_Type::copy_properties();
  Fl_Button *s = (Fl_Button*)o, *d = (Fl_Button*)live_widget;
  d->compact(s->compact());
}

Fl_Button_Type Fl_Button_type;


// ---- Return Button ----

/**
 \brief The Return Button is simply a Button with the return key as a hotkey.
 */
class Fl_Return_Button_Type : public Fl_Button_Type
{
  typedef Fl_Button_Type super;
public:
  void ideal_size(int &w, int &h) FL_OVERRIDE {
    h = layout->labelsize + 8;
    w = layout->labelsize * 4 + 8 + h; // make room for the symbol
    Fd_Snap_Action::better_size(w, h);
  }
  const char *type_name() FL_OVERRIDE { return "Fl_Return_Button"; }
  const char *alt_type_name() FL_OVERRIDE { return "fltk::ReturnButton"; }
  Fl_Widget *widget(int x, int y, int w, int h) FL_OVERRIDE {
    return new Fl_Return_Button(x, y, w, h, "Button");
  }
  Fl_Widget_Type *_make() FL_OVERRIDE { return new Fl_Return_Button_Type(); }
  ID id() const FL_OVERRIDE { return ID_Return_Button; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Return_Button) ? true : super::is_a(inID); }
};

Fl_Return_Button_Type Fl_Return_Button_type;


// ---- Repeat Button ----

/**
 \brief Handler for Fl_Repeat_Button.
 \note Even though Fl_Repeat_Button is somewhat limited compared to Fl_Button,
    and some settings may not make much sense, it is still derived from it,
    so the wrapper should be as well.
 */
class Fl_Repeat_Button_Type : public Fl_Button_Type
{
  typedef Fl_Button_Type super;
public:
  const char *type_name() FL_OVERRIDE { return "Fl_Repeat_Button"; }
  const char *alt_type_name() FL_OVERRIDE { return "fltk::RepeatButton"; }
  Fl_Widget *widget(int x, int y, int w, int h) FL_OVERRIDE {
    return new Fl_Repeat_Button(x, y, w, h, "Button");
  }
  Fl_Widget_Type *_make() FL_OVERRIDE { return new Fl_Repeat_Button_Type(); }
  ID id() const FL_OVERRIDE { return ID_Repeat_Button; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Repeat_Button) ? true : super::is_a(inID); }
};

Fl_Repeat_Button_Type Fl_Repeat_Button_type;


// ---- Light Button ----

/**
 \brief A handler for a toggle button with an indicator light.
 */
class Fl_Light_Button_Type : public Fl_Button_Type
{
  typedef Fl_Button_Type super;
public:
  void ideal_size(int &w, int &h) FL_OVERRIDE {
    h = layout->labelsize + 8;
    w = layout->labelsize * 4 + 8 + layout->labelsize; // make room for the light
    Fd_Snap_Action::better_size(w, h);
  }
  const char *type_name() FL_OVERRIDE { return "Fl_Light_Button"; }
  const char *alt_type_name() FL_OVERRIDE { return "fltk::LightButton"; }
  Fl_Widget *widget(int x, int y, int w, int h) FL_OVERRIDE {
    return new Fl_Light_Button(x, y, w, h, "Button");
  }
  Fl_Widget_Type *_make() FL_OVERRIDE { return new Fl_Light_Button_Type(); }
  ID id() const FL_OVERRIDE { return ID_Light_Button; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Light_Button) ? true : super::is_a(inID); }
};

Fl_Light_Button_Type Fl_Light_Button_type;


// ---- Check Button ----

/**
 \brief Manage buttons with a check mark on its left.
 */
class Fl_Check_Button_Type : public Fl_Button_Type
{
  typedef Fl_Button_Type super;
public:
  void ideal_size(int &w, int &h) FL_OVERRIDE {
    h = layout->labelsize + 8;
    w = layout->labelsize * 4 + 8 + layout->labelsize; // make room for the symbol
    Fd_Snap_Action::better_size(w, h);
  }
  const char *type_name() FL_OVERRIDE { return "Fl_Check_Button"; }
  const char *alt_type_name() FL_OVERRIDE { return "fltk::CheckButton"; }
  Fl_Widget *widget(int x, int y, int w, int h) FL_OVERRIDE {
    return new Fl_Check_Button(x, y, w, h, "Button");
  }
  Fl_Widget_Type *_make() FL_OVERRIDE { return new Fl_Check_Button_Type(); }
  ID id() const FL_OVERRIDE { return ID_Check_Button; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Check_Button) ? true : super::is_a(inID); }
};

Fl_Check_Button_Type Fl_Check_Button_type;


// ---- Round Button ----

/**
 \brief Manage buttons with a round indicator on its left.
 */
class Fl_Round_Button_Type : public Fl_Button_Type
{
  typedef Fl_Button_Type super;
public:
  void ideal_size(int &w, int &h) FL_OVERRIDE {
    h = layout->labelsize + 8;
    w = layout->labelsize * 4 + 8 + layout->labelsize; // make room for the symbol
    Fd_Snap_Action::better_size(w, h);
  }
  const char *type_name() FL_OVERRIDE { return "Fl_Round_Button"; }
  const char *alt_type_name() FL_OVERRIDE { return "fltk::RadioButton"; }
  Fl_Widget *widget(int x, int y, int w, int h) FL_OVERRIDE {
    return new Fl_Round_Button(x, y, w, h, "Button");
  }
  Fl_Widget_Type *_make() FL_OVERRIDE { return new Fl_Round_Button_Type(); }
  ID id() const FL_OVERRIDE { return ID_Round_Button; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Round_Button) ? true : super::is_a(inID); }
};

Fl_Round_Button_Type Fl_Round_Button_type;

