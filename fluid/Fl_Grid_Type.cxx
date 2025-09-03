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
#include <assert.h>

// ---- Fl_Grid_Proxy --------------------------------------------------- MARK: -

/**
 An implementation of the Fl_Grid widget with additional functionality.

 Fl_Grid_Proxy add a list of transient children, i.e. children that are
 temporarily assigned to a cell that is already taken by another child.
 */
Fl_Grid_Proxy::Fl_Grid_Proxy(int X,int Y,int W,int H)
: Fl_Grid(X,Y,W,H),
  transient_(NULL),
  num_transient_(0),
  cap_transient_(0)
{
}

Fl_Grid_Proxy::~Fl_Grid_Proxy() {
  int i;
  if (transient_) {
    for (i=0; i<num_transient_; i++) {
      if (transient_[i].cell) ::free(transient_[i].cell);
    }
    ::free(transient_);
  }
}

// Override group's resize behavior to do nothing to children:
void Fl_Grid_Proxy::resize(int X, int Y, int W, int H) {
  if (Fl_Type::allow_layout > 0) {
    Fl_Grid::resize(X, Y, W, H);
  } else {
    Fl_Widget::resize(X, Y, W, H);
  }
  redraw();
}

/**
 Override draw() to make groups with no box or flat box background visible.
 */
void Fl_Grid_Proxy::draw() {
  if (show_ghosted_outline && (box() == FL_NO_BOX)) {
    fl_rect(x(), y(), w(), h(), Fl::box_color(fl_color_average(FL_FOREGROUND_COLOR, color(), .1f)));
  }
  Fl_Grid::draw();
}

/**
 Draw additional markings in the overlay plane when a grid is selected.
 */
void Fl_Grid_Proxy::draw_overlay() {
  fl_line_style(FL_DOT);
  grid_color = fl_color();
  draw_grid();
  fl_color(grid_color);
}

/**
 Move a cell into the grid or within the grid.

 If the target cell is already taken, \p how will determine what to do:

 If \p how is 0, the existing cell at \p to_row, \p to_col will be deleted,
 unlinking the occupant from the grid. \p in_child will the be inserted at the
 given location.

 If \p how is 1, the old cell will remain intact, however \p in_child will be
 unlinked from the grid.

 If \p how is 2, the old cell will remain intact, and \p in_child will be
 removed from the grid, but it will be stored in the transient list and
 resized to the target cell position and size. If \p in_child is later
 moved to an unoccupied cell, it will be removed from the transient list and
 relinked to the grid. Rowspan and colspan are ignored here.

 \param[in] in_child must already be a child of grid
 \param[in] to_row, to_col move the child into this cell
 \param[in] how 0: replace occupant, 1: don't replace, 2: make transient
    if occupied
 */
void Fl_Grid_Proxy::move_cell(Fl_Widget *in_child, int to_row, int to_col, int how) {
  // the child must already be a true child of grid
  assert(find(in_child)<children());

  short rowspan = 1, colspan = 1;
  Fl_Grid_Align align = FL_GRID_FILL;
  int w = 20, h = 20;
  const Fl_Grid::Cell *old_cell = cell(in_child);
  if (old_cell) {
    if (old_cell->row() == to_row && old_cell->col() == to_col) return;
    rowspan = old_cell->rowspan();
    colspan = old_cell->colspan();
    align = old_cell->align();
    old_cell->minimum_size(&w, &h);
  }
  if ((to_row < 0) || (to_row+rowspan > rows())) return;
  if ((to_col < 0) || (to_col+colspan > cols())) return;
  Fl_Grid::Cell *new_cell = NULL;
  if (how == 0) { // replace old occupant in cell, making that one homeless
    new_cell = widget(in_child, to_row, to_col, rowspan, colspan, align);
  } else if (how == 1) { // don't replace an old occupant, making ourselves homeless
    // todo: colspan, rowspan?
    if (cell(to_row, to_col) == NULL) {
      new_cell = widget(in_child, to_row, to_col, rowspan, colspan, align);
    } else {
      if (old_cell) remove_cell(old_cell->row(), old_cell->col());
    }
  } else if (how == 2) {
    Cell *current = cell(to_row, to_col);
    if (current == NULL) {
      new_cell = widget(in_child, to_row, to_col, rowspan, colspan, align);
    } else {
      if (old_cell) remove_cell(old_cell->row(), old_cell->col());
      new_cell = transient_widget(in_child, to_row, to_col, rowspan, colspan, align);
      Fl_Widget *w = current->widget();
      Fl_Type::allow_layout++;
      in_child->resize(w->x(), w->y(), w->w(), w->h());
      Fl_Type::allow_layout--;
    }
  }
  if (new_cell) new_cell->minimum_size(w, h);
}

/**
 Generate or replace a transient widget entry.

 If the widget is in the cell list, it will be removed there.
 If the widget is already transient, the cell will be replaced.

 \param[in] wi a child of this Fl_Grid_Proxy, that may be linked to a cell or transient cell
 \param[in] row, col, row_span, col_span, align cell parameters
 */
Fl_Grid::Cell* Fl_Grid_Proxy::transient_widget(Fl_Widget *wi, int row, int col, int row_span, int col_span, Fl_Grid_Align align) {
  int i = 0;
  bool remove_old_cell = false;
  Cell *old_cell = cell(wi);
  if (old_cell) {
    remove_old_cell = true;
  } else {
    for (i=0; i<num_transient_; i++) {
      if (transient_[i].widget == wi) {
        old_cell = transient_[i].cell;
        break;
      }
    }
  }
  Cell *new_cell = new Cell(wi, row, col);
  new_cell->rowspan(row_span);
  new_cell->colspan(col_span);
  new_cell->align(align);
  if (old_cell) {
    int mw, mh;
    old_cell->minimum_size(&mw, &mh);
    new_cell->minimum_size(mw, mh);
    if (remove_old_cell) {
      remove_cell(old_cell->row(), old_cell->col());
    } else {
      delete old_cell;
    }
  }
  if (i == num_transient_) {
    transient_make_room_(num_transient_ + 1);
    transient_[i].widget = wi;
    num_transient_++;
  }
  transient_[i].cell = new_cell;
  return new_cell;
}

/**
 Make room for at least n transient widgets in the array.
 \param[in] n minimum number of entries
 */
void Fl_Grid_Proxy::transient_make_room_(int n) {
  if (n > cap_transient_) {
    cap_transient_ = n + 10;
    transient_ = (Cell_Widget_Pair*)::realloc(transient_, cap_transient_ * sizeof(Cell_Widget_Pair));
  }
}

/**
 Remove a widget form the list and deallocate the transient cell.
 \param[in] w remove the transient cell for this widget
 */
void Fl_Grid_Proxy::transient_remove_(Fl_Widget *w) {
  for (int i=0; i<num_transient_; i++) {
    if (transient_[i].widget==w) {
      if (transient_[i].cell) {
        ::free(transient_[i].cell);
        ::memmove(transient_+i, transient_+i+1, sizeof(Cell_Widget_Pair)*(num_transient_-i-1));
        num_transient_--;
        return;
      }
    }
  }
}

/**
 Find a cell in the grid or in the transient cell list.
 \param[in] widget must be a child of the grid.
 \return the cell, the transient cell, or NULL if neither was found.
 */
Fl_Grid_Proxy::Cell *Fl_Grid_Proxy::any_cell(Fl_Widget *widget) const {
  Cell *c = cell(widget);
  if (c) return c;
  return transient_cell(widget);
}

/**
 Find a cell in the transient cell list.
 \param[in] widget must be a child of the grid.
 \return the transient cell, or NULL if it was not found.
 */
Fl_Grid_Proxy::Cell *Fl_Grid_Proxy::transient_cell(Fl_Widget *widget) const {
  for (int i=0; i<num_transient_; i++) {
    if (transient_[i].widget == widget)
      return transient_[i].cell;
  }
  return NULL;
}

/**
 Forwarding the call.
 \param[in] wi generate a cell for this widget
 \param[in] row, col, align cell parameters
 */
Fl_Grid::Cell *Fl_Grid_Proxy::widget(Fl_Widget *wi, int row, int col, Fl_Grid_Align align) {
  return widget(wi, row, col, 1, 1, align);
}

/**
 Just like the Fl_Grid original, but removes potential transient cell.
 \param[in] wi generate a cell for this widget
 \param[in] row, col, rowspan, colspan, align cell parameters
 */
Fl_Grid::Cell *Fl_Grid_Proxy::widget(Fl_Widget *wi, int row, int col, int rowspan, int colspan, Fl_Grid_Align align) {
  transient_remove_(wi);
  return Fl_Grid::widget(wi, row, col, rowspan, colspan, align);
}



// ---- Fl_Grid_Type --------------------------------------------------- MARK: -

const char grid_type_name[] = "Fl_Grid";

Fl_Grid_Type Fl_Grid_type;      // the "factory"

Fl_Grid_Type::Fl_Grid_Type() {
}

Fl_Widget *Fl_Grid_Type::widget(int X,int Y,int W,int H) {
  Fl_Grid *g = new Fl_Grid_Proxy(X,Y,W,H);
  g->layout(3, 3);
  Fl_Group::current(0);
  return g;
}

Fl_Widget *Fl_Grid_Type::enter_live_mode(int top) {
  Fl_Grid *grid = new Fl_Grid(o->x(), o->y(), o->w(), o->h());
  return propagate_live_mode(grid);
}

void Fl_Grid_Type::leave_live_mode() {
}

void Fl_Grid_Type::copy_properties()
{
  super::copy_properties();
  Fl_Grid *d = (Fl_Grid*)live_widget, *s =(Fl_Grid*)o;
  d->layout(s->rows(), s->cols());
  int lm, tm, rm, bm;
  s->margin(&lm, &tm, &rm, &bm);
  d->margin(lm, tm, rm, bm);
  int rg, cg;
  s->gap(&rg, &cg);
  d->gap(rg, cg);
  // copy col widths, heights, and gaps
  for (int c=0; c<s->cols(); c++) {
    d->col_width(c, s->col_width(c));
    d->col_gap(c, s->col_gap(c));
    d->col_weight(c, s->col_weight(c));
  }
  // copy row widths, heights, and gaps
  for (int r=0; r<s->rows(); r++) {
    d->row_height(r, s->row_height(r));
    d->row_gap(r, s->row_gap(r));
    d->row_weight(r, s->row_weight(r));
  }
}

void Fl_Grid_Type::copy_properties_for_children() {
  Fl_Grid *d = (Fl_Grid*)live_widget, *s =(Fl_Grid*)o;
  for (int i=0; i<s->children(); i++) {
    Fl_Grid::Cell *cell = s->cell(s->child(i));
    if (cell && i<d->children()) {
      d->widget(d->child(i),
                cell->row(), cell->col(),
                cell->rowspan(), cell->colspan(),
                cell->align());
    }
  }
  d->layout();
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

// NOTE: we have to do this in a loop just as ::read_property() in case a new
//    property is added. In the current setup, all the remaining properties
//    will be skipped
void Fl_Grid_Type::read_parent_property(Fd_Project_Reader &f, Fl_Type *child, const char *property) {
  if (!child->is_true_widget()) {
    super::read_parent_property(f, child, property);
    return;
  }
  Fl_Grid *grid = (Fl_Grid*)o;
  Fl_Widget *child_widget = ((Fl_Widget_Type*)child)->o;
  if (!strcmp(property, "location")) {
    int row = -1, col = -1;
    const char *value = f.read_word();
    sscanf(value, "%d %d", &row, &col);
    Fl_Grid::Cell *cell = grid->widget(child_widget, row, col);
    if (cell) {
      int min_w = 20, min_h = 20;
      cell->minimum_size(min_w, min_h);
    }
  } else if (!strcmp(property, "colspan")) {
    int colspan = atoi(f.read_word());
    Fl_Grid::Cell *cell = grid->cell(child_widget);
    if (cell) cell->colspan(colspan);
  } else if (!strcmp(property, "rowspan")) {
    int rowspan = atoi(f.read_word());
    Fl_Grid::Cell *cell = grid->cell(child_widget);
    if (cell) cell->rowspan(rowspan);
  } else if (!strcmp(property, "align")) {
    int align = atoi(f.read_word());
    Fl_Grid::Cell *cell = grid->cell(child_widget);
    if (cell) cell->align((Fl_Grid_Align)align);
  } if (!strcmp(property, "minsize")) {
    int min_w = 20, min_h = 20;
    const char *value = f.read_word();
    sscanf(value, "%d %d", &min_w, &min_h);
    Fl_Grid::Cell *cell = grid->cell(child_widget);
    if (cell) cell->minimum_size(min_w, min_h);
  } else {
    super::read_parent_property(f, child, property);
  }
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
  if (cell && ((cell->align()&FL_GRID_VERTICAL)==0)) {
    int min_w = 0, min_h = 0;
    cell->minimum_size(&min_w, &min_h);
    cell->minimum_size(min_w, child->h());
  }
  if (cell && ((cell->align()&FL_GRID_HORIZONTAL)==0)) {
    int min_w = 0, min_h = 0;
    cell->minimum_size(&min_w, &min_h);
    cell->minimum_size(child->w(), min_h);
  }
  // TODO: if the user resizes an FL_GRID_FILL widget, should we change the alignment?
}

/** Return the currently selected Grid widget if is a Grid Type. */
Fl_Grid *Fl_Grid_Type::selected() {
  if (current_widget && current_widget->is_a(ID_Grid))
    return ((Fl_Grid*)((Fl_Grid_Type*)current_widget)->o);
  return NULL;
}

/**
 Insert a child widget into the cell at the x, y position inside the window.
 /param[in] child
 /param[in] x, y pixels from the top left of the window
 */
void Fl_Grid_Type::insert_child_at(Fl_Widget *child, int x, int y) {
  Fl_Grid_Proxy *grid = (Fl_Grid_Proxy*)o;
  int row = -1, col = -1, ml, mt, grg, gcg;
  grid->margin(&ml, &mt, NULL, NULL);
  grid->gap(&grg, &gcg);
  int x0 = grid->x() + Fl::box_dx(grid->box()) + ml;
  int y0 = grid->y() + Fl::box_dy(grid->box()) + mt;

  for (int r = 0; r < grid->rows(); r++) {
    if (y>y0) row = r;
    int gap = grid->row_gap(r)>=0 ? grid->row_gap(r) : grg;
    y0 += grid->computed_row_height(r);
    y0 += gap;
  }

  for (int c = 0; c < grid->cols(); c++) {
    if (x>x0) col = c;
    int gap = grid->col_gap(c)>=0 ? grid->col_gap(c) : gcg;
    x0 += grid->computed_col_width(c);
    x0 += gap;
  }

  grid->move_cell(child, row, col, 2);
}

/**
 Insert a child widget into the first new cell we can find .

 There are many other possible strategies. How about inserting to the right
 of the last added child. Also, what happens if the grid is full? Should
 we add a new row at the bottom?

 /param[in] child
 */
void Fl_Grid_Type::insert_child_at_next_free_cell(Fl_Widget *child) {
  Fl_Grid_Proxy *grid = (Fl_Grid_Proxy*)o;
  if (grid->cell(child)) return;
// The code below would insert the new widget after the last selected one, but
// unfortunately the current_widget is already invalid.
//  if (current_widget && (current_widget->parent == this)) {
//    Fl_Grid::Cell *current_cell = grid->any_cell(current_widget->o);
//    if (current_cell) {
//      r = current_cell->row();
//      c = current_cell->col();
//    }
//  }
  for (int r = 0; r < grid->rows(); r++) {
    for (int c = 0; c < grid->cols(); c++) {
      if (!grid->cell(r, c)) {
        grid->move_cell(child, r, c);
        return;
      }
    }
  }
  grid->layout(grid->rows() + 1, grid->cols());
  grid->move_cell(child, grid->rows() - 1, 0);
}

/** Move cells around using the keyboard.
 \note this fails if we have two children selected side by side and press 'right',
    which will move the left child first, removing the right child from the
    cell system. When trying to move the second child, it has no longer an
    assigned row or column.
 \param[in] child pointer to the child type
 \param[in] key code of the last keypress when handling a FL_KEYBOARD event.
 */
void Fl_Grid_Type::keyboard_move_child(Fl_Widget_Type *child, int key) {
  Fl_Grid_Proxy *grid = ((Fl_Grid_Proxy*)o);
  Fl_Grid::Cell *cell = grid->any_cell(child->o);
  if (!cell) return;
  if (key == FL_Right) {
    grid->move_cell(child->o, cell->row(), cell->col()+1, 2);
  } else if (key == FL_Left) {
    grid->move_cell(child->o, cell->row(), cell->col()-1, 2);
  } else if (key == FL_Up) {
    grid->move_cell(child->o, cell->row()-1, cell->col(), 2);
  } else if (key == FL_Down) {
    grid->move_cell(child->o, cell->row()+1, cell->col(), 2);
  }
}

void Fl_Grid_Type::layout_widget() {
  allow_layout++;
  ((Fl_Grid*)o)->layout();
  allow_layout--;
}


// ---- Widget Panel Callbacks  ---------------------------------------- MARK: -

// TODO: better grid overlay?
// TODO: grid_child_cb should move all selected cells, not just the current_selected.
// TODO: buttons to add and delete rows and columns in the widget dialog
// TODO: ways to resize rows and columns, add and delete them in the project window, pulldown menu?
// TODO: alignment can be FL_GRID_LEFT|FL_GRID_VERTICAL?

extern Fluid_Coord_Input *widget_grid_row_input, *widget_grid_col_input,
*widget_grid_rowspan_input, *widget_grid_colspan_input;
extern Fl_Group *widget_tab_grid_child;

void grid_child_cb(Fluid_Coord_Input* i, void* v, int what) {
  if (   !current_widget
      || !current_widget->parent
      || !current_widget->parent->is_a(ID_Grid))
  {
    return;
  }
  Fl_Widget *child = ((Fl_Widget_Type*)current_widget)->o;
  Fl_Grid_Proxy *g = ((Fl_Grid_Proxy*)((Fl_Widget_Type*)current_widget->parent)->o);
  Fl_Grid::Cell *cell = g->any_cell(child);
  if (v == LOAD) {
    int v = -1;
    if (cell) {
      switch (what & 0x00ff) {
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
    undo_checkpoint();
    int v2 = -2, old_v = -2, v = i->value();
    if (i==widget_grid_row_input) v2 = widget_grid_col_input->value();
    if (i==widget_grid_col_input) v2 = widget_grid_row_input->value();
    Fl_Grid::Cell *new_cell = NULL;
    if (cell) {
      switch (what & 0x00ff) {
        case 8: old_v = cell->row(); v2 = cell->col(); break;
        case 9: old_v = cell->col(); v2 = cell->row(); break;
        case 10: old_v = cell->rowspan(); break;
        case 11: old_v = cell->colspan(); break;
        case 12: cell->minimum_size(&old_v, &v2); break;
        case 13: cell->minimum_size(&v2, &old_v); break;
      }
    }
    switch (what & 0xff00) {
      case 0x0100: v--; break;
      case 0x0200: v++; break;
    }
    if (old_v != v) {
      switch (what & 0x00ff) {
        case 8:
          if (v2 == -1 && v >= 0) v2 = 0;
          g->move_cell(current_widget->o, v, v2, 2); i->value(v);
          break;
        case 9:
          if (v2 == -1 && v >= 0) v2 = 0;
          g->move_cell(current_widget->o, v2, v, 2); i->value(v);
          break;
        case 10: if (cell && cell->row()+v<=g->rows() && v>0) cell->rowspan(v);
          break;
        case 11: if (cell && cell->col()+v<=g->cols() && v>0) cell->colspan(v);
          break;
        case 12: if (cell && v>=0) cell->minimum_size(v, v2);
          break;
        case 13: if (cell && v>=0) cell->minimum_size(v2, v);
          break;
      }
      if (!cell && new_cell)
        new_cell->minimum_size(20, 20);
      g->need_layout(true);
      set_modflag(1);
    }
  }
}
void grid_set_row_cb(Fluid_Coord_Input* i, void* v) {
  grid_child_cb(i, v, 8);
  if (v!=LOAD) widget_tab_grid_child->do_callback(widget_tab_grid_child, LOAD);
}
void grid_dec_row_cb(Fl_Button* i, void* v) {
  if (v!=LOAD) {
    grid_child_cb(widget_grid_row_input, v, 0x0100 + 8);
    widget_tab_grid_child->do_callback(widget_tab_grid_child, LOAD);
  }
}
void grid_inc_row_cb(Fl_Button* i, void* v) {
  if (v!=LOAD) {
    grid_child_cb(widget_grid_row_input, v, 0x0200 + 8);
    widget_tab_grid_child->do_callback(widget_tab_grid_child, LOAD);
  }
}
void grid_set_col_cb(Fluid_Coord_Input* i, void* v) {
  grid_child_cb(i, v, 9);
  if (v!=LOAD) widget_tab_grid_child->do_callback(widget_tab_grid_child, LOAD);
}
void grid_dec_col_cb(Fl_Button* i, void* v) {
  if (v!=LOAD) {
    grid_child_cb(widget_grid_col_input, v, 0x0100 + 9);
    widget_tab_grid_child->do_callback(widget_tab_grid_child, LOAD);
  }
}
void grid_inc_col_cb(Fl_Button* i, void* v) {
  if (v!=LOAD) {
    grid_child_cb(widget_grid_col_input, v, 0x0200 + 9);
    widget_tab_grid_child->do_callback(widget_tab_grid_child, LOAD);
  }
}
void grid_set_rowspan_cb(Fluid_Coord_Input* i, void* v) {
  grid_child_cb(i, v, 10);
  if (v!=LOAD) widget_tab_grid_child->do_callback(widget_tab_grid_child, LOAD);
}
void grid_dec_rowspan_cb(Fl_Button* i, void* v) {
  if (v!=LOAD) {
    grid_child_cb(widget_grid_rowspan_input, v, 0x0100 + 10);
    widget_tab_grid_child->do_callback(widget_tab_grid_child, LOAD);
  }
}
void grid_inc_rowspan_cb(Fl_Button* i, void* v) {
  if (v!=LOAD) {
    grid_child_cb(widget_grid_rowspan_input, v, 0x0200 + 10);
    widget_tab_grid_child->do_callback(widget_tab_grid_child, LOAD);
  }
}
void grid_set_colspan_cb(Fluid_Coord_Input* i, void* v) {
  grid_child_cb(i, v, 11);
  if (v!=LOAD) widget_tab_grid_child->do_callback(widget_tab_grid_child, LOAD);
}
void grid_dec_colspan_cb(Fl_Button* i, void* v) {
  if (v!=LOAD) {
    grid_child_cb(widget_grid_colspan_input, v, 0x0100 + 11);
    widget_tab_grid_child->do_callback(widget_tab_grid_child, LOAD);
  }
}
void grid_inc_colspan_cb(Fl_Button* i, void* v) {
  if (v!=LOAD) {
    grid_child_cb(widget_grid_colspan_input, v, 0x0200 + 11);
    widget_tab_grid_child->do_callback(widget_tab_grid_child, LOAD);
  }
}
void grid_set_min_wdt_cb(Fluid_Coord_Input* i, void* v) {
  grid_child_cb(i, v, 12);
}
void grid_set_min_hgt_cb(Fluid_Coord_Input* i, void* v) {
  grid_child_cb(i, v, 13);
}

void grid_align_horizontal_cb(Fl_Choice* i, void* v) {
  if (   !current_widget
      || !current_widget->parent
      || !current_widget->parent->is_a(ID_Grid))
  {
    return;
  }
  int mask = (FL_GRID_LEFT | FL_GRID_RIGHT | FL_GRID_HORIZONTAL);
  Fl_Grid *g = ((Fl_Grid*)((Fl_Widget_Type*)current_widget->parent)->o);
  if (v == LOAD) {
    int a = FL_GRID_FILL & mask;
    Fl_Grid::Cell *cell = g->cell(current_widget->o);
    if (cell) {
      a = cell->align() & mask;
    }
    const Fl_Menu_Item *mi = i->find_item_with_argument(a);
    if (mi) i->value(mi);
  } else {
    undo_checkpoint();
    int v = FL_GRID_FILL & mask, old_v = FL_GRID_FILL & mask;
    const Fl_Menu_Item *mi = i->mvalue();
    if (mi) v = (int)mi->argument();
    Fl_Grid::Cell *cell = g->cell(current_widget->o);
    if (cell) {
      old_v = cell->align() & mask;
      if (old_v != v) {
        cell->align((Fl_Grid_Align)(v | (cell->align() & ~mask)));
        g->need_layout(true);
        g->redraw();
        set_modflag(1);
      }
    }
  }
}

void grid_align_vertical_cb(Fl_Choice* i, void* v) {
  if (   !current_widget
      || !current_widget->parent
      || !current_widget->parent->is_a(ID_Grid))
  {
    return;
  }
  int mask = (FL_GRID_TOP | FL_GRID_BOTTOM | FL_GRID_VERTICAL);
  Fl_Grid *g = ((Fl_Grid*)((Fl_Widget_Type*)current_widget->parent)->o);
  if (v == LOAD) {
    int a = FL_GRID_FILL & mask;
    Fl_Grid::Cell *cell = g->cell(current_widget->o);
    if (cell) {
      a = cell->align() & mask;
    }
    const Fl_Menu_Item *mi = i->find_item_with_argument(a);
    if (mi) i->value(mi);
  } else {
    undo_checkpoint();
    int v = FL_GRID_FILL & mask, old_v = FL_GRID_FILL & mask;
    const Fl_Menu_Item *mi = i->mvalue();
    if (mi) v = (int)mi->argument();
    Fl_Grid::Cell *cell = g->cell(current_widget->o);
    if (cell) {
      old_v = cell->align() & mask;
      if (old_v != v) {
        cell->align((Fl_Grid_Align)(v | (cell->align() & ~mask)));
        g->need_layout(true);
        g->redraw();
        set_modflag(1);
      }
    }
  }
}

