//
// Fl_Grid object code for the Fast Light Tool Kit (FLTK).
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

#include "Fl_Grid_Type.h"

#include "fluid.h"
#include "file.h"
#include "code.h"
#include "widget_browser.h"
#include "undo.h"
#include "Fd_Snap_Action.h"
#include "custom_widgets.h"

#include <FL/Fl_Grid.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include "../src/flstring.h"

#include <stdio.h>
#include <stdlib.h>

// ---- Fl_Grid_Type --------------------------------------------------- MARK: -

const char grid_type_name[] = "Fl_Grid";

Fl_Grid_Type Fl_Grid_type;      // the "factory"

Fl_Grid_Type::Fl_Grid_Type() {
}

Fl_Widget *Fl_Grid_Type::widget(int X,int Y,int W,int H) {
  Fl_Grid *g = new Fl_Grid(X,Y,W,H);
  g->layout(3, 3);
  g->show_grid(1, FL_RED);
  Fl_Group::current(0);
  return g;
}

void Fl_Grid_Type::copy_properties()
{
  super::copy_properties();
  Fl_Grid *d = (Fl_Grid*)live_widget, *s =(Fl_Grid*)o;
  int lm, tm, rm, bm;
  s->margin(&lm, &tm, &rm, &bm);
  d->margin(lm, tm, rm, bm);
  int rg, cg;
  s->gap(&rg, &cg);
  d->gap(rg, cg);
  // TODO: lots to do!
}

void Fl_Grid_Type::write_properties(Fd_Project_Writer &f)
{
  super::write_properties(f);
  Fl_Grid* grid = (Fl_Grid*)o;
  int i, rows = grid->rows(), cols = grid->cols();
  f.write_indent(level+1);
  f.write_string("dimensions {%d %d}", rows, cols);
  int lm, tm, rm, bm;
  grid->margin(&lm, &tm, &rm, &bm);
  if (lm!=0 || tm!=0 || rm!=0 || bm!=0)
    f.write_string("margin {%d %d %d %d}", lm, tm, rm, bm);
  int rg, cg;
  grid->gap(&rg, &cg);
  if (rg!=0 || cg!=0)
    f.write_string("gap {%d %d}", rg, cg);
  // -- write all row heights if one of them is not the default 0
  for (i=0; i<rows; i++) if (grid->row_height(i)!=0) break;
  if (i<rows) {
    f.write_indent(level+1);
    f.write_string("rowheights {");
    for (i=0; i<rows; i++) f.write_string("%d", grid->row_height(i));
    f.write_string("}");
  }
  // -- write all row weights if one of them is not the default 50
  for (i=0; i<rows; i++) if (grid->row_weight(i)!=50) break;
  if (i<rows) {
    f.write_indent(level+1);
    f.write_string("rowweights {");
    for (i=0; i<rows; i++) f.write_string("%d", grid->row_weight(i));
    f.write_string("}");
  }
  // -- write all row gaps if one of them is not the default -1
  for (i=0; i<rows; i++) if (grid->row_gap(i)!=-1) break;
  if (i<rows) {
    f.write_indent(level+1);
    f.write_string("rowgaps {");
    for (i=0; i<rows; i++) f.write_string("%d", grid->row_gap(i));
    f.write_string("}");
  }
  // -- write all col widths if one of them is not the default 0
  for (i=0; i<cols; i++) if (grid->col_width(i)!=0) break;
  if (i<cols) {
    f.write_indent(level+1);
    f.write_string("colwidths {");
    for (i=0; i<cols; i++) f.write_string("%d", grid->col_width(i));
    f.write_string("}");
  }
  // -- write all col weights if one of them is not the default 50
  for (i=0; i<cols; i++) if (grid->col_weight(i)!=50) break;
  if (i<cols) {
    f.write_indent(level+1);
    f.write_string("colweights {");
    for (i=0; i<cols; i++) f.write_string("%d", grid->col_weight(i));
    f.write_string("}");
  }
  // -- write all col gaps if one of them is not the default -1
  for (i=0; i<cols; i++) if (grid->col_gap(i)!=-1) break;
  if (i<cols) {
    f.write_indent(level+1);
    f.write_string("colgaps {");
    for (i=0; i<cols; i++) f.write_string("%d", grid->col_gap(i));
    f.write_string("}");
  }
}

void Fl_Grid_Type::read_property(Fd_Project_Reader &f, const char *c)
{
  Fl_Grid* grid = (Fl_Grid*)o;
  if (!strcmp(c,"dimensions")) {
    int rows = 3, cols = 3;
    if (sscanf(f.read_word(),"%d %d", &rows, &cols) == 2)
      grid->layout(rows, cols);
  } else if (!strcmp(c,"margin")) {
    int lm, tm, rm, bm;
    if (sscanf(f.read_word(),"%d %d %d %d", &lm, &tm, &rm, &bm) == 4)
      grid->margin(lm, tm, rm, bm);
  } else if (!strcmp(c,"gap")) {
    int rg, cg;
    if (sscanf(f.read_word(),"%d %d", &rg, &cg) == 2)
      grid->gap(rg, cg);
  } else if (!strcmp(c,"rowheights")) {
    int rows = grid->rows();
    f.read_word(1); // "{"
    for (int i=0; i<rows; i++) grid->row_height(i, f.read_int());
    f.read_word(1); // "}"
  } else if (!strcmp(c,"rowweights")) {
    int rows = grid->rows();
    f.read_word(1); // "{"
    for (int i=0; i<rows; i++) grid->row_weight(i, f.read_int());
    f.read_word(1); // "}"
  } else if (!strcmp(c,"rowgaps")) {
    int rows = grid->rows();
    f.read_word(1); // "{"
    for (int i=0; i<rows; i++) grid->row_gap(i, f.read_int());
    f.read_word(1); // "}"
  } else if (!strcmp(c,"colwidths")) {
    int cols = grid->cols();
    f.read_word(1); // "{"
    for (int i=0; i<cols; i++) grid->col_width(i, f.read_int());
    f.read_word(1); // "}"
  } else if (!strcmp(c,"colweights")) {
    int cols = grid->cols();
    f.read_word(1); // "{"
    for (int i=0; i<cols; i++) grid->col_weight(i, f.read_int());
    f.read_word(1); // "}"
  } else if (!strcmp(c,"colgaps")) {
    int cols = grid->cols();
    f.read_word(1); // "{"
    for (int i=0; i<cols; i++) grid->col_gap(i, f.read_int());
    f.read_word(1); // "}"
  } else {
    super::read_property(f, c);
  }
}

void Fl_Grid_Type::write_parent_properties(Fd_Project_Writer &f, Fl_Type *child, bool encapsulate) {
  Fl_Grid *grid;
  Fl_Widget *child_widget;
  Fl_Grid::Cell *cell;
  if (!child->is_true_widget()) return super::write_parent_properties(f, child, true);
  grid = (Fl_Grid*)o;
  child_widget = ((Fl_Widget_Type*)child)->o;
  cell = grid->cell(child_widget);
  if (!cell) return super::write_parent_properties(f, child, true);
  if (encapsulate) {
    f.write_indent(level+2);
    f.write_string("parent_properties {");
  }
  f.write_indent(level+3);
  f.write_string("location {%d %d}", cell->row(), cell->col());
  int v = cell->colspan();
  if (v>1) {
    f.write_indent(level+3);
    f.write_string("colspan %d", v);
  }
  v = cell->rowspan();
  if (v>1) {
    f.write_indent(level+3);
    f.write_string("rowspan %d", v);
  }
  v = (int)cell->align();
  if (v!=FL_GRID_FILL) {
    f.write_indent(level+3);
    f.write_string("align %d", v);
  }
  int min_w = 0, min_h = 0;
  cell->minimum_size(&min_w, &min_h);
  if (min_w!=20 || min_h!=20) {
    f.write_indent(level+3);
    f.write_string("minsize {%d %d}", min_w, min_h);
  }
  super::write_parent_properties(f, child, false);
  if (encapsulate) {
    f.write_indent(level+2);
    f.write_string("}");
  }
  return;
}

void Fl_Grid_Type::read_parent_properties(Fd_Project_Reader &f, Fl_Type *child, const char *property) {
  if (!child->is_true_widget()) {
    super::read_parent_properties(f, child, property);
    return;
  }
  Fl_Grid *grid = (Fl_Grid*)o;
  Fl_Widget *child_widget = ((Fl_Widget_Type*)child)->o;
  int row = -1, col = -1, rowspan = 1, colspan = 1;
  Fl_Grid_Align align = FL_GRID_FILL;
  if (!strcmp(property, "location")) {
    const char *value = f.read_word();
    sscanf(value, "%d %d", &row, &col);
    property = f.read_word();
  }
  if (!strcmp(property, "colspan")) {
    colspan = atoi(f.read_word());
    property = f.read_word();
  }
  if (!strcmp(property, "rowspan")) {
    rowspan = atoi(f.read_word());
    property = f.read_word();
  }
  if (!strcmp(property, "align")) {
    align = atoi(f.read_word());
    property = f.read_word();
  }
  if (row>=0 && col>=0) {
    Fl_Grid::Cell *cell = grid->widget(child_widget, row, col, rowspan, colspan, (Fl_Grid_Align)align);
    if (cell) {
      int min_w = 20, min_h = 20;
      if (!strcmp(property, "minsize")) {
        const char *value = f.read_word();
        sscanf(value, "%d %d", &min_w, &min_h);
        property = f.read_word();
      }
      cell->minimum_size(min_w, min_h);
    }
  }
  super::read_parent_properties(f, child, property);
}

void Fl_Grid_Type::write_code1(Fd_Code_Writer& f) {
  const char *var = name() ? name() : "o";
  Fl_Grid* grid = (Fl_Grid*)o;
  Fl_Widget_Type::write_code1(f);
  int i, rows = grid->rows(), cols = grid->cols();
  f.write_c("%s%s->layout(%d, %d);\n", f.indent(), var, rows, cols);
  int lm, tm, rm, bm;
  grid->margin(&lm, &tm, &rm, &bm);
  if (lm!=0 || tm!=0 || rm!=0 || bm!=0)
    f.write_c("%s%s->margin(%d, %d, %d, %d);\n", f.indent(), var, lm, tm, rm, bm);
  int rg, cg;
  grid->gap(&rg, &cg);
  if (rg!=0 || cg!=0)
    f.write_c("%s%s->gap(%d, %d);\n", f.indent(), var, rg, cg);
  // -- write all row heights if one of them is not the default 0
  for (i=0; i<rows; i++) if (grid->row_height(i)!=0) break;
  if (i<rows) {
    f.write_c("%sstatic const int rowheights[] = { %d", f.indent(), grid->row_height(0));
    for (i=1; i<rows; i++) f.write_c(", %d", grid->row_height(i));
    f.write_c(" };\n");
    f.write_c("%s%s->row_height(rowheights, %d);\n", f.indent(), var, rows);
  }
  // -- write all row weights if one of them is not the default 50
  for (i=0; i<rows; i++) if (grid->row_weight(i)!=50) break;
  if (i<rows) {
    f.write_c("%sstatic const int rowweights[] = { %d", f.indent(), grid->row_weight(0));
    for (i=1; i<rows; i++) f.write_c(", %d", grid->row_weight(i));
    f.write_c(" };\n");
    f.write_c("%s%s->row_weight(rowweights, %d);\n", f.indent(), var, rows);
  }
  // -- write all row gaps if one of them is not the default -1
  for (i=0; i<rows; i++) if (grid->row_gap(i)!=-1) break;
  if (i<rows) {
    f.write_c("%sstatic const int rowgaps[] = { %d", f.indent(), grid->row_gap(0));
    for (i=1; i<rows; i++) f.write_c(", %d", grid->row_gap(i));
    f.write_c(" };\n");
    f.write_c("%s%s->row_gap(rowgaps, %d);\n", f.indent(), var, rows);
  }
  // -- write all col widths if one of them is not the default 0
  for (i=0; i<cols; i++) if (grid->col_width(i)!=0) break;
  if (i<cols) {
    f.write_c("%sstatic const int colwidths[] = { %d", f.indent(), grid->col_width(0));
    for (i=1; i<cols; i++) f.write_c(", %d", grid->col_width(i));
    f.write_c(" };\n");
    f.write_c("%s%s->col_width(colwidths, %d);\n", f.indent(), var, cols);
  }
  // -- write all col weights if one of them is not the default 50
  for (i=0; i<cols; i++) if (grid->col_weight(i)!=50) break;
  if (i<cols) {
    f.write_c("%sstatic const int colweights[] = { %d", f.indent(), grid->col_weight(0));
    for (i=1; i<cols; i++) f.write_c(", %d", grid->col_weight(i));
    f.write_c(" };\n");
    f.write_c("%s%s->col_weight(colweights, %d);\n", f.indent(), var, cols);
  }
  // -- write all col gaps if one of them is not the default -1
  for (i=0; i<cols; i++) if (grid->col_gap(i)!=-1) break;
  if (i<cols) {
    f.write_c("%sstatic const int colgaps[] = { %d", f.indent(), grid->col_gap(0));
    for (i=1; i<cols; i++) f.write_c(", %d", grid->col_gap(i));
    f.write_c(" };\n");
    f.write_c("%s%s->col_gap(colgaps, %d);\n", f.indent(), var, cols);
  }
}

void Fl_Grid_Type::write_code2(Fd_Code_Writer& f) {
  const char *var = name() ? name() : "o";
  Fl_Grid* grid = (Fl_Grid*)o;
  bool first_cell = true;
  for (int i=0; i<grid->children(); i++) {
    Fl_Widget *c = grid->child(i);
    Fl_Grid::Cell *cell = grid->cell(c);
    if (cell) {
      if (first_cell) {
        f.write_c("%sFl_Grid::Cell *cell = NULL;\n", f.indent());
        first_cell = false;
      }
      f.write_c("%scell = %s->widget(%s->child(%d), %d, %d, %d, %d, %d);\n",
                f.indent(), var, var, i, cell->row(), cell->col(),
                cell->rowspan(), cell->colspan(), cell->align());
      int min_w = 20, min_h = 20;
      cell->minimum_size(&min_w, &min_h);
      f.write_c("%sif (cell) cell->minimum_size(%d, %d);\n", f.indent(), min_w, min_h);
    }
  }
  super::write_code2(f);
}

void Fl_Grid_Type::add_child(Fl_Type* a, Fl_Type* b) {
  super::add_child(a, b);
  Fl_Grid* grid = (Fl_Grid*)o;
  grid->need_layout(1);
  grid->redraw();
}

void Fl_Grid_Type::move_child(Fl_Type* a, Fl_Type* b) {
  super::move_child(a, b);
  Fl_Grid* grid = (Fl_Grid*)o;
  grid->need_layout(1);
  grid->redraw();
}

void Fl_Grid_Type::remove_child(Fl_Type* a) {
  super::remove_child(a);
  Fl_Grid* grid = (Fl_Grid*)o;
  grid->need_layout(1);
  grid->redraw();
}

/** Update the initial size of a child widget.
 Fl_Grid keeps track of the size of children when they are first added. In
 FLUID, users will want to resize children. So we need to trick Fl_Grid into
 taking the new size as the initial size.
 */
void Fl_Grid_Type::child_resized(Fl_Widget_Type *child_type) {
  Fl_Grid *grid = (Fl_Grid*)o;
  Fl_Widget *child = child_type->o;
  Fl_Grid::Cell *cell = grid->cell(child);
  if (cell && ((cell->align()&FL_GRID_HORIZONTAL)==0)) {
    int min_w = 0, min_h = 0;
    cell->minimum_size(&min_w, &min_h);
    cell->minimum_size(min_w, child->h());
  }
  if (cell && ((cell->align()&FL_GRID_VERTICAL)==0)) {
    int min_w = 0, min_h = 0;
    cell->minimum_size(&min_w, &min_h);
    cell->minimum_size(child->w(), min_h);
  }
  // TODO: if the user resizes an FL_GRID_FILL widget, should we change the alignment?
}

void grid_cb(Fl_Value_Input* i, void* v, int what) {
  if (v == LOAD) {
    if (current_widget->is_a(ID_Grid)) {
      int v; 
      Fl_Grid *g = ((Fl_Grid*)current_widget->o);
      switch (what) {
        case 0: g->margin(&v, NULL, NULL, NULL); break;
        case 1: g->margin(NULL, &v, NULL, NULL); break;
        case 2: g->margin(NULL, NULL, &v, NULL); break;
        case 3: g->margin(NULL, NULL, NULL, &v); break;
        case 4: g->gap(&v, NULL); break;
        case 5: g->gap(NULL, &v); break;
      }
      i->value(v);
    }
  } else {
    int mod = 0;
    int v = (int)i->value();
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_a(ID_Grid)) {
        Fl_Grid *g = ((Fl_Grid*)(((Fl_Widget_Type*)o)->o));
        switch (what) {
          case 0: g->margin(v, -1, -1, -1); break;
          case 1: g->margin(-1, v, -1, -1); break;
          case 2: g->margin(-1, -1, v, -1); break;
          case 3: g->margin(-1, -1, -1, v); break;
          case 4: g->gap(v, -1); break;
          case 5: g->gap(-1, v); break;
        }
        g->need_layout(true);
        g->redraw();
        mod = 1;
      }
    }
    if (mod) set_modflag(1);
  }
}
void grid_margin_left_cb(Fl_Value_Input* i, void* v) {
  grid_cb(i, v, 0);
}
void grid_margin_top_cb(Fl_Value_Input* i, void* v) {
  grid_cb(i, v, 1);
}
void grid_margin_right_cb(Fl_Value_Input* i, void* v) {
  grid_cb(i, v, 2);
}
void grid_margin_bottom_cb(Fl_Value_Input* i, void* v) {
  grid_cb(i, v, 3);
}
void grid_row_gap_cb(Fl_Value_Input* i, void* v) {
  grid_cb(i, v, 4);
}
void grid_col_gap_cb(Fl_Value_Input* i, void* v) {
  grid_cb(i, v, 5);
}

void grid_cb(Fluid_Coord_Input* i, void* v, int what) {
  if (v == LOAD) {
    if (current_widget->is_a(ID_Grid)) {
      int v = 0;
      Fl_Grid *g = ((Fl_Grid*)current_widget->o);
      switch (what) {
        case 6: v = g->rows(); break;
        case 7: v = g->cols(); break;
      }
      i->value(v);
    }
  } else {
    int mod = 0;
    int old_v = 0, v = (int)i->value();
    for (Fl_Type *o = Fl_Type::first; o; o = o->next) {
      if (o->selected && o->is_a(ID_Grid)) {
        Fl_Grid *g = ((Fl_Grid*)(((Fl_Widget_Type*)o)->o));
        switch (what) {
          case 6: old_v = g->rows(); break;
          case 7: old_v = g->cols(); break;
        }
        if (old_v != v) {
          switch (what) {
            case 6: g->layout(v, g->cols()); break;
            case 7: g->layout(g->rows(), v); break;
          }
          g->need_layout(true);
          g->redraw();
          mod = 1;
        }
      }
    }
    if (mod) set_modflag(1);
  }
}
void grid_rows_cb(Fluid_Coord_Input* i, void* v) {
  grid_cb(i, v, 6);
}
void grid_cols_cb(Fluid_Coord_Input* i, void* v) {
  grid_cb(i, v, 7);
}

extern Fluid_Coord_Input *widget_grid_row_input, *widget_grid_col_input;

// FIXME: when changing row or col or moving a cell in any other way, we need
//        to copy all attributes from the old cell, or they will be lost
// FIXME: changing a single row or col value should not move the cell, or we
//        may overwrite other cells
// FIXME: when changing the cell location, activate a button [move] to actually
//        move the cell there. If it would replace another cell, show a
//        [replace] button instead that my be bright red
// TODO: update the row and col widgets when the cell changes location
// TODO: move cells by using the arrow keys?
// TODO: move cells via drag'n'drop
// TODO: insert cells when adding them from the menu or toolbar
// TODO: better grid overlay?
// TODO: handle illegal values in all cells
// TODO: maybe increment, decrement buttons for cell locations as they only change in increments?
// TODO: we could even use formulas for moving multiple cells in one go
// TODO: buttons to add and delete rows and columns in the widget dialog
// TODO: ways to resize rows and columns, add and delete them in the project window, pulldown menu?
void grid_child_cb(Fluid_Coord_Input* i, void* v, int what) {
  if (   !current_widget
      || !current_widget->parent
      || !current_widget->parent->is_a(ID_Grid))
  {
    return;
  }
  Fl_Grid *g = ((Fl_Grid*)((Fl_Widget_Type*)current_widget->parent)->o);
  if (v == LOAD) {
    int v = -1;
    Fl_Grid::Cell *cell = g->cell(current_widget->o);
    if (cell) {
      switch (what) {
        case 8: v = cell->row(); break;
        case 9: v = cell->col(); break;
        case 10: v = cell->rowspan(); break;
        case 11: v = cell->colspan(); break;
        case 12: cell->minimum_size(&v, NULL); break;
        case 13: cell->minimum_size(NULL, &v); break;
      }
    }
    i->value(v);
  } else {
    int v2 = -1, old_v = -1, v = i->value();
    if (i==widget_grid_row_input) v2 = widget_grid_col_input->value();
    if (i==widget_grid_col_input) v2 = widget_grid_row_input->value();
    Fl_Grid::Cell *cell = g->cell(current_widget->o);
    Fl_Grid::Cell *new_cell = NULL;
    if (cell) {
      switch (what) {
        case 8: old_v = cell->row(); v2 = cell->col(); break;
        case 9: old_v = cell->col(); v2 = cell->row(); break;
        case 10: old_v = cell->rowspan(); break;
        case 11: old_v = cell->colspan(); break;
        case 12: cell->minimum_size(&old_v, &v2); break;
        case 13: cell->minimum_size(&v2, &old_v); break;
      }
    }
    if (old_v != v) {
      switch (what) {
        case 8: if (v>=0 && v2>=0) new_cell = g->widget(current_widget->o, v, v2); break;
        case 9: if (v>=0 && v2>=0) new_cell = g->widget(current_widget->o, v2, v); break;
        case 10: if (cell) cell->rowspan(v); break;
        case 11: if (cell) cell->colspan(v); break;
        case 12: if (cell) cell->minimum_size(v, v2); break;
        case 13: if (cell) cell->minimum_size(v2, v); break;
      }
      if (!cell && new_cell)
        new_cell->minimum_size(20, 20);
      g->need_layout(true);
      g->redraw();
      set_modflag(1);
    }
  }
}
void grid_set_row_cb(Fluid_Coord_Input* i, void* v) {
  grid_child_cb(i, v, 8);
}
void grid_set_col_cb(Fluid_Coord_Input* i, void* v) {
  grid_child_cb(i, v, 9);
}
void grid_set_rowspan_cb(Fluid_Coord_Input* i, void* v) {
  grid_child_cb(i, v, 10);
}
void grid_set_colspan_cb(Fluid_Coord_Input* i, void* v) {
  grid_child_cb(i, v, 11);
}
void grid_set_min_wdt_cb(Fluid_Coord_Input* i, void* v) {
  grid_child_cb(i, v, 12);
}
void grid_set_min_hgt_cb(Fluid_Coord_Input* i, void* v) {
  grid_child_cb(i, v, 13);
}

void grid_align_cb(Fl_Choice* i, void* v) {
  if (   !current_widget
      || !current_widget->parent
      || !current_widget->parent->is_a(ID_Grid))
  {
    return;
  }
  Fl_Grid *g = ((Fl_Grid*)((Fl_Widget_Type*)current_widget->parent)->o);
  if (v == LOAD) {
    int a = FL_GRID_FILL;
    Fl_Grid::Cell *cell = g->cell(current_widget->o);
    if (cell) {
      a = cell->align();
    }
    const Fl_Menu_Item *mi = i->find_item_with_argument(a);
    if (mi) i->value(mi);
  } else {
    int v = FL_GRID_FILL, old_v = FL_GRID_FILL;
    const Fl_Menu_Item *mi = i->mvalue();
    if (mi) v = (int)mi->argument();
    Fl_Grid::Cell *cell = g->cell(current_widget->o);
    if (cell) {
      old_v = cell->align();
    }
    if (old_v != v) {
      cell->align((Fl_Grid_Align)v);
      g->need_layout(true);
      g->redraw();
      set_modflag(1);
    }
  }
}

void grid_row_col_cb(Fluid_Coord_Input* i, void* v, int what) {
  if (   !current_widget
      || !current_widget->parent
      || !current_widget->parent->is_a(ID_Grid))
  {
    return;
  }
  Fl_Grid *g = ((Fl_Grid*)((Fl_Widget_Type*)current_widget->parent)->o);
  Fl_Grid::Cell *cell = g->cell(current_widget->o);
  if (v == LOAD) {
    if (cell) {
      int v = 0;
      switch (what) {
        case 0: v = g->row_height(cell->row()); break;
        case 1: v = g->row_weight(cell->row()); break;
        case 2: v = g->row_gap(cell->row()); break;
        case 3: v = g->col_width(cell->col()); break;
        case 4: v = g->col_weight(cell->col()); break;
        case 5: v = g->col_gap(cell->col()); break;
      }
      i->value(v);
      i->activate();
    } else {
      i->deactivate();
    }
  } else {
    if (cell) {
      int v = i->value(), old_v = 0;
      switch (what) {
        case 0: old_v = g->row_height(cell->row()); break;
        case 1: old_v = g->row_weight(cell->row()); break;
        case 2: old_v = g->row_gap(cell->row()); break;
        case 3: old_v = g->col_width(cell->col()); break;
        case 4: old_v = g->col_weight(cell->col()); break;
        case 5: old_v = g->col_gap(cell->col()); break;
      }
      if (old_v != v) {
        switch (what) {
          case 0: g->row_height(cell->row(), v); break;
          case 1: g->row_weight(cell->row(), v); break;
          case 2: g->row_gap(cell->row(), v); break;
          case 3: g->col_width(cell->col(), v); break;
          case 4: g->col_weight(cell->col(), v); break;
          case 5: g->col_gap(cell->col(), v); break;
        }
        g->need_layout(true);
        g->redraw();
        set_modflag(1);
      }
    }
  }
}
void grid_row_height(Fluid_Coord_Input* i, void* v) {
  grid_row_col_cb(i, v, 0);
}
void grid_row_weight(Fluid_Coord_Input* i, void* v) {
  grid_row_col_cb(i, v, 1);
}
void grid_row_gap(Fluid_Coord_Input* i, void* v) {
  grid_row_col_cb(i, v, 2);
}

void grid_col_width(Fluid_Coord_Input* i, void* v) {
  grid_row_col_cb(i, v, 3);
}
void grid_col_weight(Fluid_Coord_Input* i, void* v) {
  grid_row_col_cb(i, v, 4);
}
void grid_col_gap(Fluid_Coord_Input* i, void* v) {
  grid_row_col_cb(i, v, 5);
}
