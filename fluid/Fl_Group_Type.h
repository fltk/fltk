//
// Widget type header file for the Fast Light Tool Kit (FLTK).
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

class igroup : public Fl_Group {
public:
  void resize(int,int,int,int) FL_OVERRIDE;
  void full_resize(int X, int Y, int W, int H) { Fl_Group::resize(X, Y, W, H); }
  igroup(int X,int Y,int W,int H) : Fl_Group(X,Y,W,H) {Fl_Group::current(0);}
};

class Fl_Group_Type : public Fl_Widget_Type {
public:
  const char *type_name() FL_OVERRIDE {return "Fl_Group";}
  const char *alt_type_name() FL_OVERRIDE {return "fltk::Group";}
  Fl_Widget *widget(int X,int Y,int W,int H) FL_OVERRIDE {
    igroup *g = new igroup(X,Y,W,H); Fl_Group::current(0); return g;}
  Fl_Widget_Type *_make() FL_OVERRIDE {return new Fl_Group_Type();}
  Fl_Type *make(Strategy strategy) FL_OVERRIDE;
  void write_code1(Fd_Code_Writer& f) FL_OVERRIDE;
  void write_code2(Fd_Code_Writer& f) FL_OVERRIDE;
  void add_child(Fl_Type*, Fl_Type*) FL_OVERRIDE;
  void move_child(Fl_Type*, Fl_Type*) FL_OVERRIDE;
  void remove_child(Fl_Type*) FL_OVERRIDE;
  int is_parent() const FL_OVERRIDE {return 1;}
  int is_group() const FL_OVERRIDE {return 1;}
  int pixmapID() FL_OVERRIDE { return 6; }
  Fl_Widget *enter_live_mode(int top=0) FL_OVERRIDE;
  void leave_live_mode() FL_OVERRIDE;
  void copy_properties() FL_OVERRIDE;
};

// ---- Fl_Pack_Type --------------------------------------------------- MARK: -

extern const char pack_type_name[];
extern Fl_Menu_Item pack_type_menu[];

class Fl_Pack_Type : public Fl_Group_Type {
  Fl_Menu_Item *subtypes() FL_OVERRIDE {return pack_type_menu;}
public:
  const char *type_name() FL_OVERRIDE {return pack_type_name;}
  const char *alt_type_name() FL_OVERRIDE {return "fltk::PackedGroup";}
  Fl_Widget_Type *_make() FL_OVERRIDE {return new Fl_Pack_Type();}
  int pixmapID() FL_OVERRIDE { return 22; }
  Fl_Widget *enter_live_mode(int top=0) FL_OVERRIDE;
  void copy_properties() FL_OVERRIDE;
};

// ---- Fl_Flex_Type --------------------------------------------------- MARK: -

extern const char flex_type_name[];
extern Fl_Menu_Item flex_type_menu[];

class Fl_Flex_Type : public Fl_Group_Type {
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
    Fl_Flex *g = new Fl_Flex(X,Y,W,H); Fl_Group::current(0); return g;}
  int pixmapID() FL_OVERRIDE { return 56; }
  void write_properties(Fd_Project_Writer &f) FL_OVERRIDE;
  void read_property(Fd_Project_Reader &f, const char *) FL_OVERRIDE;
  Fl_Widget *enter_live_mode(int top=0) FL_OVERRIDE;
  void copy_properties() FL_OVERRIDE;
  void postprocess_read() FL_OVERRIDE;
  void write_code2(Fd_Code_Writer& f) FL_OVERRIDE;
  void add_child(Fl_Type*, Fl_Type*) FL_OVERRIDE;
  void move_child(Fl_Type*, Fl_Type*) FL_OVERRIDE;
  void remove_child(Fl_Type*) FL_OVERRIDE;
  int is_flex() const FL_OVERRIDE {return 1;}
  void change_subtype_to(int n);
  static int parent_is_flex(Fl_Type*);
  static int size(Fl_Type*, char fixed_only=0);
  static int is_fixed(Fl_Type*);
};

// ---- Fl_Table_Type -------------------------------------------------- MARK: -

extern const char table_type_name[];

class Fl_Table_Type : public Fl_Group_Type {
public:
  const char *type_name() FL_OVERRIDE {return table_type_name;}
  const char *alt_type_name() FL_OVERRIDE {return "fltk::TableGroup";}
  Fl_Widget_Type *_make() FL_OVERRIDE {return new Fl_Table_Type();}
  Fl_Widget *widget(int X,int Y,int W,int H) FL_OVERRIDE;
  int pixmapID() FL_OVERRIDE { return 51; }
  Fl_Widget *enter_live_mode(int top=0) FL_OVERRIDE;
  void add_child(Fl_Type*, Fl_Type*) FL_OVERRIDE;
  void move_child(Fl_Type*, Fl_Type*) FL_OVERRIDE;
  void remove_child(Fl_Type*) FL_OVERRIDE;
};

// ---- Fl_Tabs_Type --------------------------------------------------- MARK: -

extern const char tabs_type_name[];

class itabs : public Fl_Tabs {
public:
  void resize(int,int,int,int) FL_OVERRIDE;
  void full_resize(int X, int Y, int W, int H) { Fl_Group::resize(X, Y, W, H); }
  itabs(int X,int Y,int W,int H) : Fl_Tabs(X,Y,W,H) {}
};

class Fl_Tabs_Type : public Fl_Group_Type {
public:
  void ideal_spacing(int &x, int &y) FL_OVERRIDE {
     x = 10;
     fl_font(o->labelfont(), o->labelsize());
     y = fl_height() + o->labelsize() - 6;
  }
  const char *type_name() FL_OVERRIDE {return tabs_type_name;}
  const char *alt_type_name() FL_OVERRIDE {return "fltk::TabGroup";}
  Fl_Widget *widget(int X,int Y,int W,int H) FL_OVERRIDE {
    itabs *g = new itabs(X,Y,W,H); Fl_Group::current(0); return g;}
  Fl_Widget_Type *_make() FL_OVERRIDE {return new Fl_Tabs_Type();}
  Fl_Type* click_test(int,int) FL_OVERRIDE;
  void add_child(Fl_Type*, Fl_Type*) FL_OVERRIDE;
  void remove_child(Fl_Type*) FL_OVERRIDE;
  int pixmapID() FL_OVERRIDE { return 13; }
  Fl_Widget *enter_live_mode(int top=0) FL_OVERRIDE;
};

// ---- Fl_Scroll_Type ------------------------------------------------- MARK: -

extern const char scroll_type_name[];
extern Fl_Menu_Item scroll_type_menu[];

class Fl_Scroll_Type : public Fl_Group_Type {
  Fl_Menu_Item *subtypes() FL_OVERRIDE {return scroll_type_menu;}
public:
  const char *type_name() FL_OVERRIDE {return scroll_type_name;}
  const char *alt_type_name() FL_OVERRIDE {return "fltk::ScrollGroup";}
  Fl_Widget_Type *_make() FL_OVERRIDE {return new Fl_Scroll_Type();}
  int pixmapID() FL_OVERRIDE { return 19; }
  Fl_Widget *enter_live_mode(int top=0) FL_OVERRIDE;
  void copy_properties() FL_OVERRIDE;
};

// ---- Fl_Tile_Type --------------------------------------------------- MARK: -

extern const char tile_type_name[];

class Fl_Tile_Type : public Fl_Group_Type {
public:
  const char *type_name() FL_OVERRIDE {return tile_type_name;}
  const char *alt_type_name() FL_OVERRIDE {return "fltk::TileGroup";}
  Fl_Widget_Type *_make() FL_OVERRIDE {return new Fl_Tile_Type();}
  int pixmapID() FL_OVERRIDE { return 20; }
  void copy_properties() FL_OVERRIDE;
};

// ---- Fl_Wizard_Type ------------------------------------------------- MARK: -

class iwizard : public Fl_Wizard {
public:
  void resize(int,int,int,int) FL_OVERRIDE;
  void full_resize(int X, int Y, int W, int H) { Fl_Group::resize(X, Y, W, H); }
  iwizard(int X,int Y,int W,int H) : Fl_Wizard(X,Y,W,H) {}
};

extern const char wizard_type_name[];

class Fl_Wizard_Type : public Fl_Group_Type {
public:
  const char *type_name() FL_OVERRIDE {return wizard_type_name;}
  const char *alt_type_name() FL_OVERRIDE {return "fltk::WizardGroup";}
  Fl_Widget *widget(int X,int Y,int W,int H) FL_OVERRIDE {
    iwizard *g = new iwizard(X,Y,W,H); Fl_Group::current(0); return g;}
  Fl_Widget_Type *_make() FL_OVERRIDE {return new Fl_Wizard_Type();}
  int pixmapID() FL_OVERRIDE { return 21; }
};

#endif // _FLUID_FL_GROUP_TYPE_H
