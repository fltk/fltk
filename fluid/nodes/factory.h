//
// Node Factory header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2021 by Bill Spitzak and others.
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

#ifndef FLUID_NODES_FACTORY_H
#define FLUID_NODES_FACTORY_H

#include "nodes/Node.h"
#include "nodes/Widget_Node.h"
#include "Fluid.h"
#include "app/Snap_Action.h"

#include <FL/Fl_Valuator.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Scrollbar.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Spinner.H>

struct Fl_Menu_Item;

extern Fl_Menu_Item New_Menu[];

void fill_in_New_Menu();
Node *typename_to_prototype(const char *inName);

Node *add_new_widget_from_file(const char *inName, Strategy strategy);
Node *add_new_widget_from_user(Node *inPrototype, Strategy strategy, bool and_open=true);
Node *add_new_widget_from_user(const char *inName, Strategy strategy, bool and_open=true);

// ---- Declared here (rather than only in factory.cxx) so that other
// translation units can dynamic_cast to these concrete leaf types.

extern Fl_Menu_Item slider_type_menu[];
extern Fl_Menu_Item scrollbar_type_menu[];
extern Fl_Menu_Item input_type_menu[];
extern Fl_Menu_Item spinner_type_menu[];

/** \brief Manage generic Valuator widgets, base class of Slider, Counter, etc. */
class Valuator_Node : public Widget_Node
{
public:
  typedef Widget_Node super;
  static Valuator_Node prototype;
public:
  const char *type_name() override { return "Fl_Valuator"; }
  const char *alt_type_name() override { return "fltk::Valuator"; }
  Fl_Widget *widget(int x, int y, int w, int h) override {
    return new Fl_Slider(x, y, w, h, "Valuator");
  }
  Widget_Node *_make() override { return new Valuator_Node(); }
  Type type() const override { return Type::Valuator_; }
};

/**
 \brief Manage Slider widgets.
 They are vertical by default.
 Fl_Value_Slider has its own type.
 */
class Slider_Node : public Valuator_Node
{
public:
  typedef Valuator_Node super;
  static Slider_Node prototype;
private:
  Fl_Menu_Item *subtypes() override { return slider_type_menu; }
public:
  void ideal_size(int &w, int &h) override {
    auto layout = Fluid.proj.layout;
    w = layout->labelsize + 8;
    h = 4 * w;
    fluid::app::Snap_Action::better_size(w, h);
  }
  const char *type_name() override { return "Fl_Slider"; }
  const char *alt_type_name() override { return "fltk::Slider"; }
  Fl_Widget *widget(int x, int y, int w, int h) override {
    return new Fl_Slider(x, y, w, h, "slider:");
  }
  Widget_Node *_make() override { return new Slider_Node(); }
  Type type() const override { return Type::Slider; }
};

/**
 \brief Manage Scrollbars which are derived from Sliders.
 */
class Scrollbar_Node : public Slider_Node
{
public:
  typedef Slider_Node super;
  static Scrollbar_Node prototype;
private:
  Fl_Menu_Item *subtypes() override { return scrollbar_type_menu; }
public:
  const char *type_name() override { return "Fl_Scrollbar"; }
  const char *alt_type_name() override { return "fltk::Scrollbar"; }
  Fl_Widget *widget(int x, int y, int w, int h) override {
    return new Fl_Scrollbar(x, y, w, h);
  }
  Widget_Node *_make() override { return new Scrollbar_Node(); }
  Type type() const override { return Type::Scrollbar; }
};

/**
 \brief Manage Value Inputs and their text settings.
 */
class Value_Input_Node : public Valuator_Node
{
public:
  typedef Valuator_Node super;
  static Value_Input_Node prototype;
private:
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) override {
    Fl_Value_Input *myo = (Fl_Value_Input*)(w==4 ? ((Widget_Node*)factory)->o : o);
    switch (w) {
      case 4:
      case 0: f = myo->textfont(); s = myo->textsize(); c = myo->textcolor(); break;
      case 1: myo->textfont(f); break;
      case 2: myo->textsize(s); break;
      case 3: myo->textcolor(c); break;
    }
    return 1;
  }
public:
  void ideal_size(int &w, int &h) override {
    auto layout = Fluid.proj.layout;
    h = layout->textsize_not_null() + 8;
    w = layout->textsize_not_null() * 4 + 8;
    fluid::app::Snap_Action::better_size(w, h);
  }
  const char *type_name() override { return "Fl_Value_Input"; }
  const char *alt_type_name() override { return "fltk::ValueInput"; }
  Fl_Widget *widget(int x, int y, int w, int h) override {
    Fl_Value_Input *myo = new Fl_Value_Input(x, y, w, h, "value:");
    return myo;
  }
  Widget_Node *_make() override { return new Value_Input_Node(); }
  Type type() const override { return Type::Value_Input; }
};

/**
 \brief Manage simple text input widgets.
 The managed class is derived from Fl_Input_, but for simplicity, deriving from
 Widget_Node seems sufficient here.
 */
class Input_Node : public Widget_Node
{
public:
  typedef Widget_Node super;
  static Input_Node prototype;
private:
  Fl_Menu_Item *subtypes() override { return input_type_menu; }
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) override {
    Fl_Input_ *myo = (Fl_Input_*)(w==4 ? ((Widget_Node*)factory)->o : o);
    switch (w) {
      case 4:
      case 0: f = myo->textfont(); s = myo->textsize(); c = myo->textcolor(); break;
      case 1: myo->textfont(f); break;
      case 2: myo->textsize(s); break;
      case 3: myo->textcolor(c); break;
    }
    return 1;
  }
public:
  void ideal_size(int &w, int &h) override {
    auto layout = Fluid.proj.layout;
    h = layout->textsize_not_null() + 8;
    w = layout->textsize_not_null() * 6 + 8;
    fluid::app::Snap_Action::better_size(w, h);
  }
  const char *type_name() override { return "Fl_Input"; }
  const char *alt_type_name() override { return "fltk::Input"; }
  Fl_Widget *widget(int x, int y, int w, int h) override {
    Fl_Input *myo = new Fl_Input(x, y, w, h, "input:");
    myo->value("Text Input");
    return myo;
  }
  Widget_Node *_make() override { return new Input_Node(); }
  Type type() const override { return Type::Input; }
  void copy_properties() override {
    Widget_Node::copy_properties();
    Fl_Input_ *d = (Fl_Input_*)live_widget, *s = (Fl_Input_*)o;
    d->textfont(s->textfont());
    d->textsize(s->textsize());
    d->textcolor(s->textcolor());
    d->shortcut(s->shortcut());
  }
};

/**
 \brief Manage the Text Display as a base class.
 Fl_Text_Display is actually derived from Fl_Group, but for FLUID, deriving
 the type from Widget is better.
 */
class Text_Display_Node : public Widget_Node
{
public:
  typedef Widget_Node super;
  static Text_Display_Node prototype;
private:
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) override {
    Fl_Text_Display *myo = (Fl_Text_Display*)(w==4 ? ((Widget_Node*)factory)->o : o);
    switch (w) {
      case 4:
      case 0: f = myo->textfont(); s = myo->textsize(); c = myo->textcolor(); break;
      case 1: myo->textfont(f); break;
      case 2: myo->textsize(s); break;
      case 3: myo->textcolor(c); break;
    }
    return 1;
  }
public:
  void ideal_size(int &w, int &h) override {
    auto layout = Fluid.proj.layout;
    h = layout->textsize_not_null() * 4 + 8;
    w = layout->textsize_not_null() * 10 + 8;
    fluid::app::Snap_Action::better_size(w, h);
  }
  const char *type_name() override { return "Fl_Text_Display"; }
  const char *alt_type_name() override { return "fltk::TextDisplay"; }
  Fl_Widget *widget(int x, int y, int w, int h) override {
    Fl_Text_Display *myo = new Fl_Text_Display(x, y, w, h);
    if (!Fluid.batch_mode) {
      Fl_Text_Buffer *b = new Fl_Text_Buffer();
      b->text("Lorem ipsum dolor\nsit amet, consetetur\nsadipscing elitr");
      myo->buffer(b);
    }
    return myo;
  }
  Widget_Node *_make() override { return new Text_Display_Node(); }
  Type type() const override { return Type::Text_Display; }
};

/**
 \brief Manage Spinner widgets.
 \note Fl_Spinner is derived from Fl_Group, *not* Fl_Valuator as one may expect.
    For FLUID, this means some special handling and no Group support.
 */
class Spinner_Node : public Widget_Node
{
public:
  typedef Widget_Node super;
  static Spinner_Node prototype;
private:
  Fl_Menu_Item *subtypes() override { return spinner_type_menu; }
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) override {
    Fl_Spinner *myo = (Fl_Spinner*)(w==4 ? ((Widget_Node*)factory)->o : o);
    switch (w) {
      case 4:
      case 0: f = (Fl_Font)myo->textfont(); s = myo->textsize(); c = myo->textcolor(); break;
      case 1: myo->textfont(f); break;
      case 2: myo->textsize(s); break;
      case 3: myo->textcolor(c); break;
    }
    return 1;
  }
public:
  void ideal_size(int &w, int &h) override {
    auto layout = Fluid.proj.layout;
    h = layout->textsize_not_null() + 8;
    w = layout->textsize_not_null() * 4 + 8;
    fluid::app::Snap_Action::better_size(w, h);
  }
  const char *type_name() override { return "Fl_Spinner"; }
  const char *alt_type_name() override { return "fltk::Spinner"; }
  Fl_Widget *widget(int x, int y, int w, int h) override {
    return new Fl_Spinner(x, y, w, h, "spinner:");
  }
  Widget_Node *_make() override { return new Spinner_Node(); }
  Type type() const override { return Type::Spinner; }
};

#endif // FLUID_NODES_FACTORY_H
