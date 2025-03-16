//
// Group Node header file for the Fast Light Tool Kit (FLTK).
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

#ifndef FLUID_NODES_GROUP_NODE_H
#define FLUID_NODES_GROUP_NODE_H

#include "nodes/Widget_Node.h"

#include <FL/Fl_Tabs.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Flex.H>
#include <FL/Fl_Wizard.H>

void group_cb(Fl_Widget *, void *);
void ungroup_cb(Fl_Widget *, void *);

// ---- Group_Node -------------------------------------------------- MARK: -

/**
 Proxy group to use in place of Fl_Group in the interactive window.

 In an interactive environment, groups should not automatically resize their
 children. This proxy disables the layout of children by default. Children
 layout propagation may be enable temporarily by incrementing `allow_layout`
 before resizing and decrementing it again afterwards.
 */
class Fl_Group_Proxy : public Fl_Group {
public:
  Fl_Group_Proxy(int X,int Y,int W,int H) : Fl_Group(X, Y, W, H) { Fl_Group::current(nullptr); }
  void resize(int x, int y, int w, int h) override;
  void draw() override;
};

class Group_Node : public Widget_Node
{
public:
  typedef Widget_Node super;
  static Group_Node prototype;
public:
  void ideal_size(int &w, int &h) override;
  const char *type_name() override {return "Fl_Group";}
  const char *alt_type_name() override {return "fltk::Group";}
  Fl_Widget *widget(int X,int Y,int W,int H) override {
    Fl_Group_Proxy *g = new Fl_Group_Proxy(X,Y,W,H); Fl_Group::current(nullptr); return g;}
  Widget_Node *_make() override {return new Group_Node();}
  void write_code1(fld::io::Code_Writer& f) override;
  void write_code2(fld::io::Code_Writer& f) override;
  void add_child(Node*, Node*) override;
  void move_child(Node*, Node*) override;
  void remove_child(Node*) override;
  int can_have_children() const override {return 1;}
  Type type() const override { return Type::Group; }
  bool is_a(Type inType) const override { return (inType==Type::Group) ? true : super::is_a(inType); }
  Fl_Widget *enter_live_mode(int top=0) override;
  void leave_live_mode() override;
  void copy_properties() override;
};

// ---- Pack_Node --------------------------------------------------- MARK: -

extern const char pack_type_name[];
extern Fl_Menu_Item pack_type_menu[];

class Pack_Node : public Group_Node
{
public:
  typedef Group_Node super;
  static Pack_Node prototype;
private:
  Fl_Menu_Item *subtypes() override {return pack_type_menu;}
public:
  const char *type_name() override {return pack_type_name;}
  const char *alt_type_name() override {return "fltk::PackedGroup";}
  Widget_Node *_make() override {return new Pack_Node();}
  Type type() const override { return Type::Pack; }
  bool is_a(Type inType) const override { return (inType==Type::Pack) ? true : super::is_a(inType); }
  Fl_Widget *enter_live_mode(int top=0) override;
  void copy_properties() override;
};

// ---- Flex_Node --------------------------------------------------- MARK: -

extern const char flex_type_name[];
extern Fl_Menu_Item flex_type_menu[];

class Fl_Flex_Proxy : public Fl_Flex {
public:
  Fl_Flex_Proxy(int X,int Y,int W,int H) : Fl_Flex(X, Y, W, H) { Fl_Group::current(nullptr); }
  void resize(int x, int y, int w, int h) override;
  void draw() override;
};

class Flex_Node : public Group_Node
{
public:
  typedef Group_Node super;
  static Flex_Node prototype;
private:
  Fl_Menu_Item *subtypes() override {return flex_type_menu;}
  int fixedSizeTupleSize; /* number of pairs in array */
  int *fixedSizeTuple; /* [ index, size, index2, size2, ... ] */
  int suspend_auto_layout;
public:
  Flex_Node() : fixedSizeTupleSize(0), fixedSizeTuple(nullptr), suspend_auto_layout(0) { }
  const char *type_name() override {return flex_type_name;}
  const char *alt_type_name() override {return "fltk::FlexGroup";}
  Widget_Node *_make() override { return new Flex_Node(); }
  Fl_Widget *widget(int X,int Y,int W,int H) override {
    Fl_Flex *g = new Fl_Flex_Proxy(X,Y,W,H); Fl_Group::current(nullptr); return g;}
  Type type() const override { return Type::Flex; }
  bool is_a(Type inType) const override { return (inType==Type::Flex) ? true : super::is_a(inType); }
  void write_properties(fld::io::Project_Writer &f) override;
  void read_property(fld::io::Project_Reader &f, const char *) override;
  Fl_Widget *enter_live_mode(int top=0) override;
  void copy_properties() override;
  void copy_properties_for_children() override;
  void postprocess_read() override;
  void write_code2(fld::io::Code_Writer& f) override;
//  void add_child(Node*, Node*) override;
//  void move_child(Node*, Node*) override;
  void remove_child(Node*) override;
  void layout_widget() override;
  void change_subtype_to(int n);
  void insert_child_at(Fl_Widget *child, int x, int y);
  void keyboard_move_child(Widget_Node*, int key);
  static int parent_is_flex(Node*);
  static int size(Node*, char fixed_only=0);
  static int is_fixed(Node*);
};

// ---- Table_Node -------------------------------------------------- MARK: -

class Table_Node : public Group_Node
{
public:
  typedef Group_Node super;
  static Table_Node prototype;
public:
  void ideal_size(int &w, int &h) override;
  const char *type_name() override { return "Fl_Table"; }
  const char *alt_type_name() override { return "fltk::TableGroup"; }
  Widget_Node *_make() override { return new Table_Node(); }
  Fl_Widget *widget(int X, int Y, int W, int H) override;
  Type type() const override { return Type::Table; }
  bool is_a(Type inType) const override { return (inType==Type::Table) ? true : super::is_a(inType); }
  Fl_Widget *enter_live_mode(int top=0) override;
  void add_child(Node*, Node*) override;
  void move_child(Node*, Node*) override;
  void remove_child(Node*) override;
};

// ---- Tabs_Node --------------------------------------------------- MARK: -

extern const char tabs_type_name[];

class Fl_Tabs_Proxy : public Fl_Tabs {
public:
  Fl_Tabs_Proxy(int X,int Y,int W,int H) : Fl_Tabs(X,Y,W,H) {}
  void resize(int,int,int,int) override;
  void draw() override;
};

class Tabs_Node : public Group_Node
{
public:
  typedef Group_Node super;
  static Tabs_Node prototype;
public:
  const char *type_name() override {return tabs_type_name;}
  const char *alt_type_name() override {return "fltk::TabGroup";}
  Fl_Widget *widget(int X,int Y,int W,int H) override {
    Fl_Tabs_Proxy *g = new Fl_Tabs_Proxy(X,Y,W,H); Fl_Group::current(nullptr); return g;}
  Widget_Node *_make() override {return new Tabs_Node();}
  Node* click_test(int,int) override;
  void add_child(Node*, Node*) override;
  void remove_child(Node*) override;
  Type type() const override { return Type::Tabs; }
  bool is_a(Type inType) const override { return (inType==Type::Tabs) ? true : super::is_a(inType); }
  Fl_Widget *enter_live_mode(int top=0) override;
};

// ---- Scroll_Node ------------------------------------------------- MARK: -

extern const char scroll_type_name[];
extern Fl_Menu_Item scroll_type_menu[];

class Scroll_Node : public Group_Node
{
public:
  typedef Group_Node super;
  static Scroll_Node prototype;
private:
  Fl_Menu_Item *subtypes() override {return scroll_type_menu;}
public:
  const char *type_name() override {return scroll_type_name;}
  const char *alt_type_name() override {return "fltk::ScrollGroup";}
  Widget_Node *_make() override {return new Scroll_Node();}
  Type type() const override { return Type::Scroll; }
  bool is_a(Type inType) const override { return (inType==Type::Scroll) ? true : super::is_a(inType); }
  Fl_Widget *enter_live_mode(int top=0) override;
  void copy_properties() override;
};

// ---- Tile_Node --------------------------------------------------- MARK: -

extern const char tile_type_name[];

class Tile_Node : public Group_Node
{
public:
  typedef Group_Node super;
  static Tile_Node prototype;
public:
  const char *type_name() override {return tile_type_name;}
  const char *alt_type_name() override {return "fltk::TileGroup";}
  Widget_Node *_make() override {return new Tile_Node();}
  Type type() const override { return Type::Tile; }
  bool is_a(Type inType) const override { return (inType==Type::Tile) ? true : super::is_a(inType); }
  void copy_properties() override;
};

// ---- Wizard_Node ------------------------------------------------- MARK: -

class Fl_Wizard_Proxy : public Fl_Wizard {
public:
  Fl_Wizard_Proxy(int X,int Y,int W,int H) : Fl_Wizard(X,Y,W,H) {}
  void resize(int,int,int,int) override;
  void draw() override;
};

extern const char wizard_type_name[];

class Wizard_Node : public Group_Node
{
public:
  typedef Group_Node super;
  static Wizard_Node prototype;
public:
  const char *type_name() override {return wizard_type_name;}
  const char *alt_type_name() override {return "fltk::WizardGroup";}
  Fl_Widget *widget(int X,int Y,int W,int H) override {
    Fl_Wizard_Proxy *g = new Fl_Wizard_Proxy(X,Y,W,H); Fl_Group::current(nullptr); return g;}
  Widget_Node *_make() override {return new Wizard_Node();}
  Type type() const override { return Type::Wizard; }
  bool is_a(Type inType) const override { return (inType==Type::Wizard) ? true : super::is_a(inType); }
};

#endif // FLUID_NODES_GROUP_NODE_H
