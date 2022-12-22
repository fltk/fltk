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

class igroup : public Fl_Group {
public:
  void resize(int,int,int,int) override;
  void full_resize(int X, int Y, int W, int H) { Fl_Group::resize(X, Y, W, H); }
  igroup(int X,int Y,int W,int H) : Fl_Group(X,Y,W,H) {Fl_Group::current(0);}
};

class itabs : public Fl_Tabs {
public:
  void resize(int,int,int,int) override;
  void full_resize(int X, int Y, int W, int H) { Fl_Group::resize(X, Y, W, H); }
  itabs(int X,int Y,int W,int H) : Fl_Tabs(X,Y,W,H) {}
};

class iwizard : public Fl_Wizard {
public:
  void resize(int,int,int,int) override;
  void full_resize(int X, int Y, int W, int H) { Fl_Group::resize(X, Y, W, H); }
  iwizard(int X,int Y,int W,int H) : Fl_Wizard(X,Y,W,H) {}
};

class Fl_Group_Type : public Fl_Widget_Type {
public:
  const char *type_name() override {return "Fl_Group";}
  const char *alt_type_name() override {return "fltk::Group";}
  Fl_Widget *widget(int X,int Y,int W,int H) override {
    igroup *g = new igroup(X,Y,W,H); Fl_Group::current(0); return g;}
  Fl_Widget_Type *_make() override {return new Fl_Group_Type();}
  Fl_Type *make(Strategy strategy) override;
  void write_code1() override;
  void write_code2() override;
  void add_child(Fl_Type*, Fl_Type*) override;
  void move_child(Fl_Type*, Fl_Type*) override;
  void remove_child(Fl_Type*) override;
  int is_parent() const override {return 1;}
  int is_group() const override {return 1;}
  int pixmapID() override { return 6; }

  Fl_Widget *enter_live_mode(int top=0) override;
  void leave_live_mode() override;
  void copy_properties() override;
};

extern const char pack_type_name[];
extern Fl_Menu_Item pack_type_menu[];

class Fl_Pack_Type : public Fl_Group_Type {
  Fl_Menu_Item *subtypes() override {return pack_type_menu;}
public:
  const char *type_name() override {return pack_type_name;}
  const char *alt_type_name() override {return "fltk::PackedGroup";}
  Fl_Widget_Type *_make() override {return new Fl_Pack_Type();}
  int pixmapID() override { return 22; }
  void copy_properties() override;
};

extern const char flex_type_name[];
extern Fl_Menu_Item flex_type_menu[];

class Fl_Flex_Type : public Fl_Group_Type {
  Fl_Menu_Item *subtypes() override {return flex_type_menu;}
  int fixedSizeTupleSize; /* number of pairs in array */
  int *fixedSizeTuple; /* [ index, size, index2, size2, ... ] */
  int suspend_auto_layout;
public:
  Fl_Flex_Type() : fixedSizeTupleSize(0), fixedSizeTuple(NULL), suspend_auto_layout(0) { }
  const char *type_name() override {return flex_type_name;}
  const char *alt_type_name() override {return "fltk::FlexGroup";}
  Fl_Widget_Type *_make() override { return new Fl_Flex_Type(); }
  Fl_Widget *widget(int X,int Y,int W,int H) override {
    Fl_Flex *g = new Fl_Flex(X,Y,W,H); Fl_Group::current(0); return g;}
  int pixmapID() override { return 56; }
  void write_properties() override;
  void read_property(const char *) override;
  Fl_Widget *enter_live_mode(int top=0) override;
  void copy_properties() override;
  void postprocess_read() override;
  void write_code2() override;
  void add_child(Fl_Type*, Fl_Type*) override;
  void move_child(Fl_Type*, Fl_Type*) override;
  void remove_child(Fl_Type*) override;
  int is_flex() const override {return 1;}
  void change_subtype_to(int n);
  static int parent_is_flex(Fl_Type*);
  static int size(Fl_Type*, char fixed_only=0);
  static int is_fixed(Fl_Type*);
};

extern const char table_type_name[];

class Fl_Table_Type : public Fl_Group_Type {
public:
  const char *type_name() override {return table_type_name;}
  const char *alt_type_name() override {return "fltk::TableGroup";}
  Fl_Widget_Type *_make() override {return new Fl_Table_Type();}
  Fl_Widget *widget(int X,int Y,int W,int H) override;
  int pixmapID() override { return 51; }
  Fl_Widget *enter_live_mode(int top=0) override;
  void add_child(Fl_Type*, Fl_Type*) override;
  void move_child(Fl_Type*, Fl_Type*) override;
  void remove_child(Fl_Type*) override;
};

extern const char tabs_type_name[];

class Fl_Tabs_Type : public Fl_Group_Type {
public:
  void ideal_spacing(int &x, int &y) override {
     x = 10;
     fl_font(o->labelfont(), o->labelsize());
     y = fl_height() + o->labelsize() - 6;
  }
  const char *type_name() override {return tabs_type_name;}
  const char *alt_type_name() override {return "fltk::TabGroup";}
  Fl_Widget *widget(int X,int Y,int W,int H) override {
    itabs *g = new itabs(X,Y,W,H); Fl_Group::current(0); return g;}
  Fl_Widget_Type *_make() override {return new Fl_Tabs_Type();}
  Fl_Type* click_test(int,int) override;
  void add_child(Fl_Type*, Fl_Type*) override;
  void remove_child(Fl_Type*) override;
  int pixmapID() override { return 13; }
  Fl_Widget *enter_live_mode(int top=0) override;
};

extern const char scroll_type_name[];
extern Fl_Menu_Item scroll_type_menu[];

class Fl_Scroll_Type : public Fl_Group_Type {
  Fl_Menu_Item *subtypes() override {return scroll_type_menu;}
public:
  const char *type_name() override {return scroll_type_name;}
  const char *alt_type_name() override {return "fltk::ScrollGroup";}
  Fl_Widget_Type *_make() override {return new Fl_Scroll_Type();}
  int pixmapID() override { return 19; }
  Fl_Widget *enter_live_mode(int top=0) override;
  void copy_properties() override;
};

extern const char tile_type_name[];

class Fl_Tile_Type : public Fl_Group_Type {
public:
  const char *type_name() override {return tile_type_name;}
  const char *alt_type_name() override {return "fltk::TileGroup";}
  Fl_Widget_Type *_make() override {return new Fl_Tile_Type();}
  int pixmapID() override { return 20; }
  void copy_properties() override;
};

extern const char wizard_type_name[];

class Fl_Wizard_Type : public Fl_Group_Type {
public:
  const char *type_name() override {return wizard_type_name;}
  const char *alt_type_name() override {return "fltk::WizardGroup";}
  Fl_Widget *widget(int X,int Y,int W,int H) override {
    iwizard *g = new iwizard(X,Y,W,H); Fl_Group::current(0); return g;}
  Fl_Widget_Type *_make() override {return new Fl_Wizard_Type();}
  int pixmapID() override { return 21; }
};

#endif // _FLUID_FL_GROUP_TYPE_H
