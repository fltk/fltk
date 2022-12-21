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
  virtual void resize(int,int,int,int) FL_OVERRIDE;
  void full_resize(int X, int Y, int W, int H) { Fl_Group::resize(X, Y, W, H); }
  igroup(int X,int Y,int W,int H) : Fl_Group(X,Y,W,H) {Fl_Group::current(0);}
};

class itabs : public Fl_Tabs {
public:
  virtual void resize(int,int,int,int) FL_OVERRIDE;
  void full_resize(int X, int Y, int W, int H) { Fl_Group::resize(X, Y, W, H); }
  itabs(int X,int Y,int W,int H) : Fl_Tabs(X,Y,W,H) {}
};

class iwizard : public Fl_Wizard {
public:
  virtual void resize(int,int,int,int) FL_OVERRIDE;
  void full_resize(int X, int Y, int W, int H) { Fl_Group::resize(X, Y, W, H); }
  iwizard(int X,int Y,int W,int H) : Fl_Wizard(X,Y,W,H) {}
};

class Fl_Group_Type : public Fl_Widget_Type {
public:
  virtual const char *type_name() FL_OVERRIDE {return "Fl_Group";}
  virtual const char *alt_type_name() FL_OVERRIDE {return "fltk::Group";}
  virtual Fl_Widget *widget(int X,int Y,int W,int H) FL_OVERRIDE {
    igroup *g = new igroup(X,Y,W,H); Fl_Group::current(0); return g;}
  virtual Fl_Widget_Type *_make() FL_OVERRIDE {return new Fl_Group_Type();}
  virtual Fl_Type *make(Strategy strategy) FL_OVERRIDE;
  virtual void write_code1() FL_OVERRIDE;
  virtual void write_code2() FL_OVERRIDE;
  virtual void add_child(Fl_Type*, Fl_Type*) FL_OVERRIDE;
  virtual void move_child(Fl_Type*, Fl_Type*) FL_OVERRIDE;
  virtual void remove_child(Fl_Type*) FL_OVERRIDE;
  virtual int is_parent() const FL_OVERRIDE {return 1;}
  virtual int is_group() const FL_OVERRIDE {return 1;}
  virtual int pixmapID() FL_OVERRIDE { return 6; }

  virtual Fl_Widget *enter_live_mode(int top=0) FL_OVERRIDE;
  virtual void leave_live_mode() FL_OVERRIDE;
  virtual void copy_properties() FL_OVERRIDE;
};

extern const char pack_type_name[];
extern Fl_Menu_Item pack_type_menu[];

class Fl_Pack_Type : public Fl_Group_Type {
  virtual Fl_Menu_Item *subtypes() FL_OVERRIDE {return pack_type_menu;}
public:
  virtual const char *type_name() FL_OVERRIDE {return pack_type_name;}
  virtual const char *alt_type_name() FL_OVERRIDE {return "fltk::PackedGroup";}
  virtual Fl_Widget_Type *_make() FL_OVERRIDE {return new Fl_Pack_Type();}
  virtual int pixmapID() FL_OVERRIDE { return 22; }
  virtual void copy_properties() FL_OVERRIDE;
};

extern const char flex_type_name[];
extern Fl_Menu_Item flex_type_menu[];

class Fl_Flex_Type : public Fl_Group_Type {
  virtual Fl_Menu_Item *subtypes() FL_OVERRIDE {return flex_type_menu;}
  int fixedSizeTupleSize; /* number of pairs in array */
  int *fixedSizeTuple; /* [ index, size, index2, size2, ... ] */
  int suspend_auto_layout;
public:
  Fl_Flex_Type() : fixedSizeTupleSize(0), fixedSizeTuple(NULL), suspend_auto_layout(0) { }
  virtual const char *type_name() FL_OVERRIDE {return flex_type_name;}
  virtual const char *alt_type_name() FL_OVERRIDE {return "fltk::FlexGroup";}
  virtual Fl_Widget_Type *_make() FL_OVERRIDE { return new Fl_Flex_Type(); }
  virtual Fl_Widget *widget(int X,int Y,int W,int H) FL_OVERRIDE {
    Fl_Flex *g = new Fl_Flex(X,Y,W,H); Fl_Group::current(0); return g;}
  virtual int pixmapID() FL_OVERRIDE { return 56; }
  virtual void write_properties() FL_OVERRIDE;
  virtual void read_property(const char *) FL_OVERRIDE;
  virtual Fl_Widget *enter_live_mode(int top=0) FL_OVERRIDE;
  virtual void copy_properties() FL_OVERRIDE;
  virtual void postprocess_read() FL_OVERRIDE;
  virtual void write_code2() FL_OVERRIDE;
  virtual void add_child(Fl_Type*, Fl_Type*) FL_OVERRIDE;
  virtual void move_child(Fl_Type*, Fl_Type*) FL_OVERRIDE;
  virtual void remove_child(Fl_Type*) FL_OVERRIDE;
  virtual int is_flex() const FL_OVERRIDE {return 1;}
  void change_subtype_to(int n);
  static int parent_is_flex(Fl_Type*);
  static int size(Fl_Type*, char fixed_only=0);
  static int is_fixed(Fl_Type*);
};

extern const char table_type_name[];

class Fl_Table_Type : public Fl_Group_Type {
public:
  virtual const char *type_name() FL_OVERRIDE {return table_type_name;}
  virtual const char *alt_type_name() FL_OVERRIDE {return "fltk::TableGroup";}
  virtual Fl_Widget_Type *_make() FL_OVERRIDE {return new Fl_Table_Type();}
  virtual Fl_Widget *widget(int X,int Y,int W,int H) FL_OVERRIDE;
  virtual int pixmapID() FL_OVERRIDE { return 51; }
  virtual Fl_Widget *enter_live_mode(int top=0) FL_OVERRIDE;
  virtual void add_child(Fl_Type*, Fl_Type*) FL_OVERRIDE;
  virtual void move_child(Fl_Type*, Fl_Type*) FL_OVERRIDE;
  virtual void remove_child(Fl_Type*) FL_OVERRIDE;
};

extern const char tabs_type_name[];

class Fl_Tabs_Type : public Fl_Group_Type {
public:
  virtual void ideal_spacing(int &x, int &y) FL_OVERRIDE {
     x = 10;
     fl_font(o->labelfont(), o->labelsize());
     y = fl_height() + o->labelsize() - 6;
  }
  virtual const char *type_name() FL_OVERRIDE {return tabs_type_name;}
  virtual const char *alt_type_name() FL_OVERRIDE {return "fltk::TabGroup";}
  virtual Fl_Widget *widget(int X,int Y,int W,int H) FL_OVERRIDE {
    itabs *g = new itabs(X,Y,W,H); Fl_Group::current(0); return g;}
  virtual Fl_Widget_Type *_make() FL_OVERRIDE {return new Fl_Tabs_Type();}
  virtual Fl_Type* click_test(int,int) FL_OVERRIDE;
  virtual void add_child(Fl_Type*, Fl_Type*) FL_OVERRIDE;
  virtual void remove_child(Fl_Type*) FL_OVERRIDE;
  virtual int pixmapID() FL_OVERRIDE { return 13; }
  virtual Fl_Widget *enter_live_mode(int top=0) FL_OVERRIDE;
};

extern const char scroll_type_name[];
extern Fl_Menu_Item scroll_type_menu[];

class Fl_Scroll_Type : public Fl_Group_Type {
  virtual Fl_Menu_Item *subtypes() FL_OVERRIDE {return scroll_type_menu;}
public:
  virtual const char *type_name() FL_OVERRIDE {return scroll_type_name;}
  virtual const char *alt_type_name() FL_OVERRIDE {return "fltk::ScrollGroup";}
  virtual Fl_Widget_Type *_make() FL_OVERRIDE {return new Fl_Scroll_Type();}
  virtual int pixmapID() FL_OVERRIDE { return 19; }
  virtual Fl_Widget *enter_live_mode(int top=0) FL_OVERRIDE;
  virtual void copy_properties() FL_OVERRIDE;
};

extern const char tile_type_name[];

class Fl_Tile_Type : public Fl_Group_Type {
public:
  virtual const char *type_name() FL_OVERRIDE {return tile_type_name;}
  virtual const char *alt_type_name() FL_OVERRIDE {return "fltk::TileGroup";}
  virtual Fl_Widget_Type *_make() FL_OVERRIDE {return new Fl_Tile_Type();}
  virtual int pixmapID() FL_OVERRIDE { return 20; }
  virtual void copy_properties() FL_OVERRIDE;
};

extern const char wizard_type_name[];

class Fl_Wizard_Type : public Fl_Group_Type {
public:
  virtual const char *type_name() FL_OVERRIDE {return wizard_type_name;}
  virtual const char *alt_type_name() FL_OVERRIDE {return "fltk::WizardGroup";}
  virtual Fl_Widget *widget(int X,int Y,int W,int H) FL_OVERRIDE {
    iwizard *g = new iwizard(X,Y,W,H); Fl_Group::current(0); return g;}
  virtual Fl_Widget_Type *_make() FL_OVERRIDE {return new Fl_Wizard_Type();}
  virtual int pixmapID() FL_OVERRIDE { return 21; }
};

#endif // _FLUID_FL_GROUP_TYPE_H
