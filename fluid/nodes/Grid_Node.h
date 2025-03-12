//
// Fl_Grid type header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 2023 by Bill Spitzak and others.
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
  void resize(int,int,int,int) FL_OVERRIDE;
  void draw() FL_OVERRIDE;
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
  const char *type_name() FL_OVERRIDE {return "Fl_Grid";}
  const char *alt_type_name() FL_OVERRIDE {return "fltk::GridGroup";}
  Widget_Node *_make() FL_OVERRIDE { return new Grid_Node(); }
  Fl_Widget *widget(int X,int Y,int W,int H) FL_OVERRIDE;
  ID id() const FL_OVERRIDE { return ID_Grid; }
  bool is_a(ID inID) const FL_OVERRIDE { return (inID==ID_Grid) ? true : super::is_a(inID); }
  void write_properties(fld::io::Project_Writer &f) FL_OVERRIDE;
  void read_property(fld::io::Project_Reader &f, const char *) FL_OVERRIDE;
  void write_parent_properties(fld::io::Project_Writer &f, Node *child, bool encapsulate) FL_OVERRIDE;
  void read_parent_property(fld::io::Project_Reader &f, Node *child, const char *property) FL_OVERRIDE;
  Fl_Widget *enter_live_mode(int top=0) FL_OVERRIDE;
  void leave_live_mode() FL_OVERRIDE;
  void copy_properties() FL_OVERRIDE;
  void copy_properties_for_children() FL_OVERRIDE;
  void write_code1(fld::io::Code_Writer& f) FL_OVERRIDE;
  void write_code2(fld::io::Code_Writer& f) FL_OVERRIDE;
  void add_child(Node*, Node*) FL_OVERRIDE;
  void move_child(Node*, Node*) FL_OVERRIDE;
  void remove_child(Node*) FL_OVERRIDE;
  void layout_widget() FL_OVERRIDE;
  void child_resized(Widget_Node *child);
  void insert_child_at(Fl_Widget *child, int x, int y);
  void insert_child_at_next_free_cell(Fl_Widget *child);
  void keyboard_move_child(Widget_Node*, int key);

  static class Fl_Grid *selected();
};

#endif // FLUID_NODES_GRID_NODE_H
