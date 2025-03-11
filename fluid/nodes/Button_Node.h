//
// Button type header file for the Fast Light Tool Kit (FLTK).
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

#ifndef FLUID_NODES_BUTTON_NODE_H
#define FLUID_NODES_BUTTON_NODE_H

#include "nodes/Widget_Node.h"

/**
 \brief A handler for the simple push button and a base class for all other buttons.
 */
class Button_Node : public Widget_Node
{
public:
  typedef Widget_Node super;
  static Button_Node prototype;
private:
  Fl_Menu_Item *subtypes() FL_OVERRIDE;
public:
  void ideal_size(int &w, int &h) FL_OVERRIDE;
  const char *type_name() FL_OVERRIDE { return "Fl_Button"; }
  const char *alt_type_name() FL_OVERRIDE { return "fltk::Button"; }
  Fl_Widget *widget(int x, int y, int w, int h) FL_OVERRIDE;
  Widget_Node *_make() FL_OVERRIDE { return new Button_Node(); }
  int is_button() const FL_OVERRIDE { return 1; }
  ID id() const FL_OVERRIDE { return ID_Button; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Button) ? true : super::is_a(inID); }
  void write_properties(fld::io::Project_Writer &f) FL_OVERRIDE;
  void read_property(fld::io::Project_Reader &f, const char *) FL_OVERRIDE;
  void copy_properties() FL_OVERRIDE;
};

// ---- Return Button ----

/**
 \brief The Return Button is simply a Button with the return key as a hotkey.
 */
class Return_Button_Node : public Button_Node
{
public:
  typedef Button_Node super;
  static Return_Button_Node prototype;
public:
  void ideal_size(int &w, int &h) FL_OVERRIDE;
  const char *type_name() FL_OVERRIDE { return "Fl_Return_Button"; }
  const char *alt_type_name() FL_OVERRIDE { return "fltk::ReturnButton"; }
  Fl_Widget *widget(int x, int y, int w, int h) FL_OVERRIDE;
  Widget_Node *_make() FL_OVERRIDE { return new Return_Button_Node(); }
  ID id() const FL_OVERRIDE { return ID_Return_Button; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Return_Button) ? true : super::is_a(inID); }
};

// ---- Repeat Button ----

/**
 \brief Handler for Fl_Repeat_Button.
 \note Even though Fl_Repeat_Button is somewhat limited compared to Fl_Button,
 and some settings may not make much sense, it is still derived from it,
 so the wrapper should be as well.
 */
class Repeat_Button_Node : public Button_Node
{
public:
  typedef Button_Node super;
  static Repeat_Button_Node prototype;
public:
  const char *type_name() FL_OVERRIDE { return "Fl_Repeat_Button"; }
  const char *alt_type_name() FL_OVERRIDE { return "fltk::RepeatButton"; }
  Fl_Widget *widget(int x, int y, int w, int h) FL_OVERRIDE;
  Widget_Node *_make() FL_OVERRIDE { return new Repeat_Button_Node(); }
  ID id() const FL_OVERRIDE { return ID_Repeat_Button; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Repeat_Button) ? true : super::is_a(inID); }
};

// ---- Light Button ----

/**
 \brief A handler for a toggle button with an indicator light.
 */
class Light_Button_Node : public Button_Node
{
public:
  typedef Button_Node super;
  static Light_Button_Node prototype;
public:
  void ideal_size(int &w, int &h) FL_OVERRIDE;
  const char *type_name() FL_OVERRIDE { return "Fl_Light_Button"; }
  const char *alt_type_name() FL_OVERRIDE { return "fltk::LightButton"; }
  Fl_Widget *widget(int x, int y, int w, int h) FL_OVERRIDE;
  Widget_Node *_make() FL_OVERRIDE { return new Light_Button_Node(); }
  ID id() const FL_OVERRIDE { return ID_Light_Button; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Light_Button) ? true : super::is_a(inID); }
};

// ---- Check Button ----

/**
 \brief Manage buttons with a check mark on its left.
 */
class Check_Button_Node : public Button_Node
{
public:
  typedef Button_Node super;
  static Check_Button_Node prototype;
public:
  void ideal_size(int &w, int &h) FL_OVERRIDE;
  const char *type_name() FL_OVERRIDE { return "Fl_Check_Button"; }
  const char *alt_type_name() FL_OVERRIDE { return "fltk::CheckButton"; }
  Fl_Widget *widget(int x, int y, int w, int h) FL_OVERRIDE;
  Widget_Node *_make() FL_OVERRIDE { return new Check_Button_Node(); }
  ID id() const FL_OVERRIDE { return ID_Check_Button; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Check_Button) ? true : super::is_a(inID); }
};

// ---- Round Button ----

/**
 \brief Manage buttons with a round indicator on its left.
 */
class Round_Button_Node : public Button_Node
{
public:
  typedef Button_Node super;
  static Round_Button_Node prototype;
public:
  void ideal_size(int &w, int &h) FL_OVERRIDE;
  const char *type_name() FL_OVERRIDE { return "Fl_Round_Button"; }
  const char *alt_type_name() FL_OVERRIDE { return "fltk::RadioButton"; }
  Fl_Widget *widget(int x, int y, int w, int h) FL_OVERRIDE;
  Widget_Node *_make() FL_OVERRIDE { return new Round_Button_Node(); }
  ID id() const FL_OVERRIDE { return ID_Round_Button; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Round_Button) ? true : super::is_a(inID); }
};



#endif // FLUID_NODES_BUTTON_NODE_H
