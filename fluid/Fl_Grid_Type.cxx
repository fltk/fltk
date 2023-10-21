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
}

void Fl_Grid_Type::write_properties(Fd_Project_Writer &f)
{
  super::write_properties(f);
  Fl_Grid* grid = (Fl_Grid*)o;
  int lm, tm, rm, bm;
  grid->margin(&lm, &tm, &rm, &bm);
  if (lm!=0 || tm!=0 || rm!=0 || bm!=0)
    f.write_string("margin {%d %d %d %d}", lm, tm, rm, bm);
  int rg, cg;
  grid->gap(&rg, &cg);
  if (rg!=0 || cg!=0)
    f.write_string("gap {%d %d}", rg, cg);
}

void Fl_Grid_Type::read_property(Fd_Project_Reader &f, const char *c)
{
  Fl_Grid* grid = (Fl_Grid*)o;
  if (!strcmp(c,"margin")) {
    int lm, tm, rm, bm;
    if (sscanf(f.read_word(),"%d %d %d %d", &lm, &tm, &rm, &bm) == 4)
      grid->margin(lm, tm, rm, bm);
  } else if (!strcmp(c,"gap")) {
    int rg, cg;
    if (sscanf(f.read_word(),"%d %d", &rg, &cg))
      grid->gap(rg, cg);
  } else {
    super::read_property(f, c);
  }
}

void Fl_Grid_Type::write_parent_properties(Fd_Project_Writer &f, Fl_Type *child, bool encapsulate) {
  Fl_Grid *grid;
  Fl_Widget *child_widget;
  Fl_Grid::Cell *cell;
  if (!child->is_true_widget()) goto err;
  grid = (Fl_Grid*)o;
  child_widget = ((Fl_Widget_Type*)child)->o;
  cell = grid->cell(child_widget);
  if (!cell) goto err;
  if (encapsulate) {
    f.write_indent(level+2);
    f.write_string("parent_properties {");
  }
  f.write_indent(level+3);
  f.write_string("location {%d %d}", cell->row(), cell->col());
  super::write_parent_properties(f, child, false);
  if (encapsulate) {
    f.write_indent(level+2);
    f.write_string("}");
  }
  return;
err:
  super::write_parent_properties(f, child, true);
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
  if (row>=0 && col>=0) grid->widget(child_widget, row, col, rowspan, colspan, (Fl_Grid_Align)align);
  super::read_parent_properties(f, child, property);
}

void Fl_Grid_Type::write_code1(Fd_Code_Writer& f) {
  const char *var = name() ? name() : "o";
  Fl_Grid* grid = (Fl_Grid*)o;
  Fl_Widget_Type::write_code1(f);
  f.write_c("%s%s->layout(%d, %d);\n", f.indent(), var, grid->rows(), grid->cols());
  int lm, tm, rm, bm;
  grid->margin(&lm, &tm, &rm, &bm);
  if (lm!=0 || tm!=0 || rm!=0 || bm!=0)
    f.write_c("%s%s->margin(%d, %d, %d, %d);\n", f.indent(), var, lm, tm, rm, bm);
  int rg, cg;
  grid->gap(&rg, &cg);
  if (rg!=0 || cg!=0)
    f.write_c("%s%s->gap(%d, %d);\n", f.indent(), var, rg, cg);
}

void Fl_Grid_Type::write_code2(Fd_Code_Writer& f) {
  const char *var = name() ? name() : "o";
  Fl_Grid* grid = (Fl_Grid*)o;
  for (int i=0; i<grid->children(); i++) {
    Fl_Widget *c = grid->child(i);
    Fl_Grid::Cell *cell = grid->cell(c);
    if (cell) {
      f.write_c("%s%s->widget(%s->child(%d), %d, %d);\n", f.indent(), var, var, i, cell->row(), cell->col());
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
      }
    }
    i->value(v);
  } else {
    int mod = 0;
    int v2 = 0, old_v = -1, v = (int)i->value();
    Fl_Grid::Cell *cell = g->cell(current_widget->o);
    if (cell) {
      switch (what) {
        case 8: old_v = cell->row(); v2 = cell->col(); break;
        case 9: old_v = cell->col(); v2 = cell->row(); break;
      }
    }
    if (old_v != v) {
      switch (what) {
        case 8: g->widget(current_widget->o, v, v2); break;
        case 9: g->widget(current_widget->o, v2, v); break;
      }
      g->need_layout(true);
      g->redraw();
      mod = 1;
      if (mod) set_modflag(1);
    }
  }
}
void grid_set_row_cb(Fluid_Coord_Input* i, void* v) {
  grid_child_cb(i, v, 8);
}
void grid_set_col_cb(Fluid_Coord_Input* i, void* v) {
  grid_child_cb(i, v, 9);
}
void grid_set_colspan_cb(Fluid_Coord_Input* i, void* v) {
  grid_child_cb(i, v, 10);
}
void grid_set_rowspan_cb(Fluid_Coord_Input* i, void* v) {
  grid_child_cb(i, v, 11);
}
void grid_align_cb(Fl_Choice* i, void* v) {
}
