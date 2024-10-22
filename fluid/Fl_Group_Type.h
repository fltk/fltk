//
// Group type header file for the Fast Light Tool Kit (FLTK).
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

#ifndef _FLUID_FL_GROUP_TYPE_H
#define _FLUID_FL_GROUP_TYPE_H

#include "Fl_Widget_Type.h"

#include <FL/Fl_Tabs.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Flex.H>
#include <FL/Fl_Wizard.H>

void group_cb(Fl_Widget *, void *);
void ungroup_cb(Fl_Widget *, void *);

// ---- Fl_Group_Type -------------------------------------------------- MARK: -

/**
 Proxy group to use in place of Fl_Group in the interactive window.

 In an interactive environment, groups should not automatically resize their
 children. This proxy disables the layout of children by default. Children
 layout propagation may be enable temporarily by incrementing `allow_layout`
 before resizing and decrementing it again afterwards.
 */
class Fl_Group_Proxy : public Fl_Group {
public:
  Fl_Group_Proxy(int X,int Y,int W,int H) : Fl_Group(X, Y, W, H) { Fl_Group::current(0); }
  void resize(int x, int y, int w, int h) FL_OVERRIDE;
  void draw() FL_OVERRIDE;
};

class Fl_Group_Type : public Fl_Widget_Type
{
  typedef Fl_Widget_Type super;
public:
  void ideal_size(int &w, int &h) FL_OVERRIDE;
  const char *type_name() FL_OVERRIDE {return "Fl_Group";}
  const char *alt_type_name() FL_OVERRIDE {return "fltk::Group";}
  Fl_Widget *widget(int X,int Y,int W,int H) FL_OVERRIDE {
    Fl_Group_Proxy *g = new Fl_Group_Proxy(X,Y,W,H); Fl_Group::current(0); return g;}
  Fl_Widget_Type *_make() FL_OVERRIDE {return new Fl_Group_Type();}
  void write_code1(Fd_Code_Writer& f) FL_OVERRIDE;
  void write_code2(Fd_Code_Writer& f) FL_OVERRIDE;
  void add_child(Fl_Type*, Fl_Type*) FL_OVERRIDE;
  void move_child(Fl_Type*, Fl_Type*) FL_OVERRIDE;
  void remove_child(Fl_Type*) FL_OVERRIDE;
  int can_have_children() const FL_OVERRIDE {return 1;}
  ID id() const FL_OVERRIDE { return ID_Group; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Group) ? true : super::is_a(inID); }
  Fl_Widget *enter_live_mode(int top=0) FL_OVERRIDE;
  void leave_live_mode() FL_OVERRIDE;
  void copy_properties() FL_OVERRIDE;
};

// ---- Fl_Pack_Type --------------------------------------------------- MARK: -

extern const char pack_type_name[];
extern Fl_Menu_Item pack_type_menu[];

class Fl_Pack_Type : public Fl_Group_Type
{
  typedef Fl_Group_Type super;
  Fl_Menu_Item *subtypes() FL_OVERRIDE {return pack_type_menu;}
public:
  const char *type_name() FL_OVERRIDE {return pack_type_name;}
  const char *alt_type_name() FL_OVERRIDE {return "fltk::PackedGroup";}
  Fl_Widget_Type *_make() FL_OVERRIDE {return new Fl_Pack_Type();}
  ID id() const FL_OVERRIDE { return ID_Pack; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Pack) ? true : super::is_a(inID); }
  Fl_Widget *enter_live_mode(int top=0) FL_OVERRIDE;
  void copy_properties() FL_OVERRIDE;
};

// ---- Fl_Flex_Type --------------------------------------------------- MARK: -

extern const char flex_type_name[];
extern Fl_Menu_Item flex_type_menu[];

class Fl_Flex_Proxy : public Fl_Flex {
public:
  Fl_Flex_Proxy(int X,int Y,int W,int H) : Fl_Flex(X, Y, W, H) { Fl_Group::current(0); }
  void resize(int x, int y, int w, int h) FL_OVERRIDE;
  void draw() FL_OVERRIDE;
};

class Fl_Flex_Type : public Fl_Group_Type
{
  typedef Fl_Group_Type super;
  Fl_Menu_Item *subtypes() FL_OVERRIDE {return flex_type_menu;}
  int fixedSizeTupleSize; /* number of pairs in array */
  int *fixedSizeTuple; /* [ index, size, index2, size2, ... ] */
  int suspend_auto_layout;
public:
  Fl_Flex_Type() : fixedSizeTupleSize(0), fixedSizeTuple(NULL), suspend_auto_layout(0) { }
  const char *type_name() FL_OVERRIDE {return flex_type_name;}
  const char *alt_type_name() FL_OVERRIDE {return "fltk::FlexGroup";}
  Fl_Widget_Type *_make() FL_OVERRIDE { return new Fl_Flex_Type(); }
  Fl_Widget *widget(int X,int Y,int W,int H) FL_OVERRIDE {
    Fl_Flex *g = new Fl_Flex_Proxy(X,Y,W,H); Fl_Group::current(0); return g;}
  ID id() const FL_OVERRIDE { return ID_Flex; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Flex) ? true : super::is_a(inID); }
  void write_properties(Fd_Project_Writer &f) FL_OVERRIDE;
  void read_property(Fd_Project_Reader &f, const char *) FL_OVERRIDE;
  Fl_Widget *enter_live_mode(int top=0) FL_OVERRIDE;
  void copy_properties() FL_OVERRIDE;
  void copy_properties_for_children() FL_OVERRIDE;
  void postprocess_read() FL_OVERRIDE;
  void write_code2(Fd_Code_Writer& f) FL_OVERRIDE;
//  void add_child(Fl_Type*, Fl_Type*) FL_OVERRIDE;
//  void move_child(Fl_Type*, Fl_Type*) FL_OVERRIDE;
  void remove_child(Fl_Type*) FL_OVERRIDE;
  void layout_widget() FL_OVERRIDE;
  void change_subtype_to(int n);
  void insert_child_at(Fl_Widget *child, int x, int y);
  void keyboard_move_child(Fl_Widget_Type*, int key);
  static int parent_is_flex(Fl_Type*);
  static int size(Fl_Type*, char fixed_only=0);
  static int is_fixed(Fl_Type*);
};

// ---- Fl_Table_Type -------------------------------------------------- MARK: -

class Fl_Table_Type : public Fl_Group_Type
{
  typedef Fl_Group_Type super;
public:
  void ideal_size(int &w, int &h) FL_OVERRIDE;
  const char *type_name() FL_OVERRIDE { return "Fl_Table"; }
  const char *alt_type_name() FL_OVERRIDE { return "fltk::TableGroup"; }
  Fl_Widget_Type *_make() FL_OVERRIDE { return new Fl_Table_Type(); }
  Fl_Widget *widget(int X, int Y, int W, int H) FL_OVERRIDE;
  ID id() const FL_OVERRIDE { return ID_Table; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Table) ? true : super::is_a(inID); }
  Fl_Widget *enter_live_mode(int top=0) FL_OVERRIDE;
  void add_child(Fl_Type*, Fl_Type*) FL_OVERRIDE;
  void move_child(Fl_Type*, Fl_Type*) FL_OVERRIDE;
  void remove_child(Fl_Type*) FL_OVERRIDE;
};

// ---- Fl_Tabs_Type --------------------------------------------------- MARK: -

extern const char tabs_type_name[];

class Fl_Tabs_Proxy : public Fl_Tabs {
public:
  Fl_Tabs_Proxy(int X,int Y,int W,int H) : Fl_Tabs(X,Y,W,H) {}
  void resize(int,int,int,int) FL_OVERRIDE;
  void draw() FL_OVERRIDE;
};

class Fl_Tabs_Type : public Fl_Group_Type
{
  typedef Fl_Group_Type super;
public:
  const char *type_name() FL_OVERRIDE {return tabs_type_name;}
  const char *alt_type_name() FL_OVERRIDE {return "fltk::TabGroup";}
  Fl_Widget *widget(int X,int Y,int W,int H) FL_OVERRIDE {
    Fl_Tabs_Proxy *g = new Fl_Tabs_Proxy(X,Y,W,H); Fl_Group::current(0); return g;}
  Fl_Widget_Type *_make() FL_OVERRIDE {return new Fl_Tabs_Type();}
  Fl_Type* click_test(int,int) FL_OVERRIDE;
  void add_child(Fl_Type*, Fl_Type*) FL_OVERRIDE;
  void remove_child(Fl_Type*) FL_OVERRIDE;
  ID id() const FL_OVERRIDE { return ID_Tabs; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Tabs) ? true : super::is_a(inID); }
  Fl_Widget *enter_live_mode(int top=0) FL_OVERRIDE;
};

// ---- Fl_Scroll_Type ------------------------------------------------- MARK: -

extern const char scroll_type_name[];
extern Fl_Menu_Item scroll_type_menu[];

class Fl_Scroll_Type : public Fl_Group_Type
{
  typedef Fl_Group_Type super;
  Fl_Menu_Item *subtypes() FL_OVERRIDE {return scroll_type_menu;}
public:
  const char *type_name() FL_OVERRIDE {return scroll_type_name;}
  const char *alt_type_name() FL_OVERRIDE {return "fltk::ScrollGroup";}
  Fl_Widget_Type *_make() FL_OVERRIDE {return new Fl_Scroll_Type();}
  ID id() const FL_OVERRIDE { return ID_Scroll; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Scroll) ? true : super::is_a(inID); }
  Fl_Widget *enter_live_mode(int top=0) FL_OVERRIDE;
  void copy_properties() FL_OVERRIDE;
};

// ---- Fl_Tile_Type --------------------------------------------------- MARK: -

extern const char tile_type_name[];

class Fl_Tile_Type : public Fl_Group_Type
{
  typedef Fl_Group_Type super;
public:
  const char *type_name() FL_OVERRIDE {return tile_type_name;}
  const char *alt_type_name() FL_OVERRIDE {return "fltk::TileGroup";}
  Fl_Widget_Type *_make() FL_OVERRIDE {return new Fl_Tile_Type();}
  ID id() const FL_OVERRIDE { return ID_Tile; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Tile) ? true : super::is_a(inID); }
  void copy_properties() FL_OVERRIDE;
};

// ---- Fl_Wizard_Type ------------------------------------------------- MARK: -

class Fl_Wizard_Proxy : public Fl_Wizard {
public:
  Fl_Wizard_Proxy(int X,int Y,int W,int H) : Fl_Wizard(X,Y,W,H) {}
  void resize(int,int,int,int) FL_OVERRIDE;
  void draw() FL_OVERRIDE;
};

extern const char wizard_type_name[];

class Fl_Wizard_Type : public Fl_Group_Type
{
  typedef Fl_Group_Type super;
public:
  const char *type_name() FL_OVERRIDE {return wizard_type_name;}
  const char *alt_type_name() FL_OVERRIDE {return "fltk::WizardGroup";}
  Fl_Widget *widget(int X,int Y,int W,int H) FL_OVERRIDE {
    Fl_Wizard_Proxy *g = new Fl_Wizard_Proxy(X,Y,W,H); Fl_Group::current(0); return g;}
  Fl_Widget_Type *_make() FL_OVERRIDE {return new Fl_Wizard_Type();}
  ID id() const FL_OVERRIDE { return ID_Wizard; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Wizard) ? true : super::is_a(inID); }
};

#endif // _FLUID_FL_GROUP_TYPE_H
