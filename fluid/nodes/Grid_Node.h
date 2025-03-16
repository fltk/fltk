//
// Grid Node header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 2023-2025 by Bill Spitzak and others.
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

#ifndef FLUID_NODES_GRID_NODE_H
#define FLUID_NODES_GRID_NODE_H

#include "nodes/Group_Node.h"
#include <FL/Fl_Grid.H>

// ---- Grid_Node --------------------------------------------------- MARK: -

class Fl_Grid_Proxy : public Fl_Grid {
protected:
  typedef struct { Fl_Widget *widget; Cell *cell; } Cell_Widget_Pair;
  Cell_Widget_Pair *transient_;
  int num_transient_;
  int cap_transient_;
  void transient_make_room_(int n);
  void transient_remove_(Fl_Widget *w);
public:
  Fl_Grid_Proxy(int X,int Y,int W,int H);
  ~Fl_Grid_Proxy();
  void resize(int,int,int,int) override;
  void draw() override;
  void draw_overlay();
  void move_cell(Fl_Widget *child, int to_row, int to_col, int how = 0);
  Cell* any_cell(Fl_Widget *widget) const;
  Cell* transient_cell(Fl_Widget *widget) const;
  Cell* transient_widget(Fl_Widget *wi, int row, int col, int row_span, int col_span, Fl_Grid_Align align = FL_GRID_FILL);
  Cell* widget(Fl_Widget *wi, int row, int col, Fl_Grid_Align align = FL_GRID_FILL);
  Cell* widget(Fl_Widget *wi, int row, int col, int rowspan, int colspan, Fl_Grid_Align align = FL_GRID_FILL);
};

class Grid_Node : public Group_Node
{
public:
  typedef Group_Node super;
  static Grid_Node prototype;
public:
  Grid_Node();
  const char *type_name() override {return "Fl_Grid";}
  const char *alt_type_name() override {return "fltk::GridGroup";}
  Widget_Node *_make() override { return new Grid_Node(); }
  Fl_Widget *widget(int X,int Y,int W,int H) override;
  Type type() const override { return Type::Grid; }
  bool is_a(Type inType) const override { return (inType==Type::Grid) ? true : super::is_a(inType); }
  void write_properties(fld::io::Project_Writer &f) override;
  void read_property(fld::io::Project_Reader &f, const char *) override;
  void write_parent_properties(fld::io::Project_Writer &f, Node *child, bool encapsulate) override;
  void read_parent_property(fld::io::Project_Reader &f, Node *child, const char *property) override;
  Fl_Widget *enter_live_mode(int top=0) override;
  void leave_live_mode() override;
  void copy_properties() override;
  void copy_properties_for_children() override;
  void write_code1(fld::io::Code_Writer& f) override;
  void write_code2(fld::io::Code_Writer& f) override;
  void add_child(Node*, Node*) override;
  void move_child(Node*, Node*) override;
  void remove_child(Node*) override;
  void layout_widget() override;
  void child_resized(Widget_Node *child);
  void insert_child_at(Fl_Widget *child, int x, int y);
  void insert_child_at_next_free_cell(Fl_Widget *child);
  void keyboard_move_child(Widget_Node*, int key);

  static class Fl_Grid *selected();
};

#endif // FLUID_NODES_GRID_NODE_H
