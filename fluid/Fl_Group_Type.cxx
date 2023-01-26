//
// Fl_Group object code for the Fast Light Tool Kit (FLTK).
//
// Object describing an Fl_Group and links to Fl_Window_Type.C and
// the Fl_Tabs widget, with special stuff to select tab items and
// insure that only one is visible.
//
// Copyright 1998-2010 by Bill Spitzak and others.
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

#include "Fl_Group_Type.h"

#include "fluid.h"
#include "file.h"
#include "code.h"
#include "widget_browser.h"
#include "undo.h"

#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Table.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/fl_message.H>
#include <FL/Fl_Scroll.H>
#include "../src/flstring.h"

#include <stdio.h>
#include <stdlib.h>

// ---- Fl_Group_Type -------------------------------------------------- MARK: -

Fl_Group_Type Fl_Group_type;    // the "factory"

// Override group's resize behavior to do nothing to children:
void igroup::resize(int X, int Y, int W, int H) {
  Fl_Widget::resize(X,Y,W,H);
  redraw();
}

/**
 Create and add a new Group node.
 \param[in] strategy add after current or as last child
 \return new Group node
 */
Fl_Type *Fl_Group_Type::make(Strategy strategy) {
  return Fl_Widget_Type::make(strategy);
}

void fix_group_size(Fl_Type *tt) {
  if (!tt || !tt->is_group()) return;
  Fl_Group_Type* t = (Fl_Group_Type*)tt;
  int X = t->o->x();
  int Y = t->o->y();
  int R = X+t->o->w();
  int B = Y+t->o->h();
  for (Fl_Type *nn = t->next; nn && nn->level > t->level; nn = nn->next) {
    if (!nn->is_widget() || nn->is_menu_item()) continue;
    Fl_Widget_Type* n = (Fl_Widget_Type*)nn;
    int x = n->o->x();  if (x < X) X = x;
    int y = n->o->y();  if (y < Y) Y = y;
    int r = x+n->o->w();if (r > R) R = r;
    int b = y+n->o->h();if (b > B) B = b;
  }
  t->o->resize(X,Y,R-X,B-Y);
}

void group_cb(Fl_Widget *, void *) {
  // Find the current widget:
  Fl_Type *qq = Fl_Type::current;
  while (qq && (!qq->is_widget() || qq->is_menu_item())) qq = qq->parent;
  if (!qq || qq->level < 1 || (qq->level == 1 && !strcmp(qq->type_name(), "widget_class"))) {
    fl_message("Please select widgets to group");
    return;
  }
  undo_checkpoint();
  undo_suspend();
  Fl_Widget_Type* q = (Fl_Widget_Type*)qq;
  force_parent = 1;
  Fl_Group_Type *n = (Fl_Group_Type*)(Fl_Group_type.make(kAddAsLastChild));
  n->move_before(q);
  n->o->resize(q->o->x(),q->o->y(),q->o->w(),q->o->h());
  for (Fl_Type *t = Fl_Type::first; t;) {
    if (t->level != n->level || t == n || !t->selected) {
      t = t->next; continue;}
    Fl_Type *nxt = t->remove();
    t->add(n, kAddAsLastChild);
    t = nxt;
  }
  fix_group_size(n);
  widget_browser->rebuild();
  undo_resume();
  set_modflag(1);
}

void ungroup_cb(Fl_Widget *, void *) {
  // Find the group:
  Fl_Type *q = Fl_Type::current;
  while (q && (!q->is_widget() || q->is_menu_item())) q = q->parent;
  if (q) q = q->parent;
  if (!q || q->level < 1 || (q->level == 1 && !strcmp(q->type_name(), "widget_class"))) {
    fl_message("Please select widgets in a group");
    return;
  }
  Fl_Type* n;
  for (n = q->next; n && n->level > q->level; n = n->next) {
    if (n->level == q->level+1 && !n->selected) {
      fl_message("Please select all widgets in group");
      return;
    }
  }
  undo_checkpoint();
  undo_suspend();
  for (n = q->next; n && n->level > q->level;) {
    Fl_Type *nxt = n->remove();
    n->insert(q);
    n = nxt;
  }
  delete q;
  widget_browser->rebuild();
  undo_resume();
  set_modflag(1);
}

void Fl_Group_Type::write_code1(Fd_Code_Writer& f) {
  Fl_Widget_Type::write_code1(f);
}

void Fl_Group_Type::write_code2(Fd_Code_Writer& f) {
  const char *var = name() ? name() : "o";
  write_extra_code(f);
  f.write_c("%s%s->end();\n", f.indent(), var);
  if (resizable()) {
    f.write_c("%sFl_Group::current()->resizable(%s);\n", f.indent(), var);
  }
  write_block_close(f);
}

// This is called when o is created.  If it is in the tab group make
// sure it is visible:
void Fl_Group_Type::add_child(Fl_Type* cc, Fl_Type* before) {
  Fl_Widget_Type* c = (Fl_Widget_Type*)cc;
  Fl_Widget* b = before ? ((Fl_Widget_Type*)before)->o : 0;
  ((Fl_Group*)o)->insert(*(c->o), b);
  o->redraw();
}

// This is called when o is deleted.  If it is in the tab group make
// sure it is not visible:
void Fl_Group_Type::remove_child(Fl_Type* cc) {
  Fl_Widget_Type* c = (Fl_Widget_Type*)cc;
  ((Fl_Group*)o)->remove(c->o);
  o->redraw();
}

// move, don't change selected value:
void Fl_Group_Type::move_child(Fl_Type* cc, Fl_Type* before) {
  Fl_Widget_Type* c = (Fl_Widget_Type*)cc;
  Fl_Widget* b = before ? ((Fl_Widget_Type*)before)->o : 0;
  ((Fl_Group*)o)->insert(*(c->o), b);
  o->redraw();
}

// live mode support
Fl_Widget* Fl_Group_Type::enter_live_mode(int) {
  Fl_Group *grp = new Fl_Group(o->x(), o->y(), o->w(), o->h());
  return propagate_live_mode(grp);
}

void Fl_Group_Type::leave_live_mode() {
}

/**
 copy all properties from the edit widget to the live widget
 */
void Fl_Group_Type::copy_properties() {
  Fl_Widget_Type::copy_properties();
}

// ---- Fl_Pack_Type --------------------------------------------------- MARK: -

Fl_Pack_Type Fl_Pack_type;      // the "factory"

const char pack_type_name[] = "Fl_Pack";

Fl_Menu_Item pack_type_menu[] = {
  {"HORIZONTAL", 0, 0, (void*)Fl_Pack::HORIZONTAL},
  {"VERTICAL", 0, 0, (void*)Fl_Pack::VERTICAL},
  {0}
};

Fl_Widget *Fl_Pack_Type::enter_live_mode(int) {
  Fl_Group *grp = new Fl_Pack(o->x(), o->y(), o->w(), o->h());
  return propagate_live_mode(grp);
}

void Fl_Pack_Type::copy_properties()
{
  Fl_Group_Type::copy_properties();
  Fl_Pack *d = (Fl_Pack*)live_widget, *s =(Fl_Pack*)o;
  d->spacing(s->spacing());
}

// ---- Fl_Flex_Type --------------------------------------------------- MARK: -

const char flex_type_name[] = "Fl_Flex";

Fl_Menu_Item flex_type_menu[] = {
  {"HORIZONTAL", 0, 0, (void*)Fl_Flex::HORIZONTAL},
  {"VERTICAL", 0, 0, (void*)Fl_Flex::VERTICAL},
  {0}};

Fl_Flex_Type Fl_Flex_type;      // the "factory"

Fl_Widget *Fl_Flex_Type::enter_live_mode(int) {
  Fl_Flex *grp = new Fl_Flex(o->x(), o->y(), o->w(), o->h());
  propagate_live_mode(grp);
  suspend_auto_layout = 0;
  Fl_Flex *d = grp, *s =(Fl_Flex*)o;
  int nc = s->children(), nd = d->children();
  if (nc>nd) nc = nd;
  for (int i=0; i<nc; i++) {
    if (s->fixed(s->child(i))) {
      Fl_Widget *dc = d->child(i);
      d->fixed(d->child(i), s->horizontal() ? dc->w() : dc->h());
    }
  }
  return grp;
}

void Fl_Flex_Type::copy_properties()
{
  Fl_Group_Type::copy_properties();
  Fl_Flex *d = (Fl_Flex*)live_widget, *s =(Fl_Flex*)o;
  int lm, tm, rm, bm;
  s->margin(&lm, &tm, &rm, &bm);
  d->margin(lm, tm, rm, bm);
  d->gap( s->gap() );
}

void Fl_Flex_Type::write_properties(Fd_Project_Writer &f)
{
  Fl_Group_Type::write_properties(f);
  Fl_Flex* flex = (Fl_Flex*)o;
  int lm, tm, rm, bm;
  flex->margin(&lm, &tm, &rm, &bm);
  if (lm!=0 || tm!=0 || rm!=0 || bm!=0)
    f.write_string("margin {%d %d %d %d}", lm, tm, rm, bm);
  if (flex->gap())
    f.write_string("gap %d", flex->gap());
  int nSet = 0;
  for (int i=0; i<flex->children(); i++)
    if (flex->fixed(flex->child(i)))
      nSet++;
  if (nSet) {
    f.write_string("fixed_size_tuples {%d", nSet);
    for (int i=0; i<flex->children(); i++) {
      Fl_Widget *ci = flex->child(i);
      if (flex->fixed(ci))
        f.write_string(" %d %d", i, flex->horizontal() ? ci->w() : ci->h());
    }
    f.write_string("}");
  }
}

void Fl_Flex_Type::read_property(Fd_Project_Reader &f, const char *c)
{
  Fl_Flex* flex = (Fl_Flex*)o;
  suspend_auto_layout = 1;
  if (!strcmp(c,"margin")) {
    int lm, tm, rm, bm;
    if (sscanf(f.read_word(),"%d %d %d %d",&lm,&tm,&rm,&bm) == 4)
      flex->margin(lm, tm, rm, bm);
  } else if (!strcmp(c,"gap")) {
    int g;
    if (sscanf(f.read_word(),"%d",&g))
      flex->gap(g);
  } else if (!strcmp(c,"fixed_size_tuples")) {
    f.read_word(1); // must be '{'
    const char *nStr = f.read_word(1); // number of indices in table
    fixedSizeTupleSize = atoi(nStr);
    fixedSizeTuple = new int[fixedSizeTupleSize*2];
    for (int i=0; i<fixedSizeTupleSize; i++) {
      const char *ix = f.read_word(1); // child at that index is fixed in size
      fixedSizeTuple[i*2] = atoi(ix);
      const char *size = f.read_word(1); // fixed size of that child
      fixedSizeTuple[i*2+1] = atoi(size);
    }
    f.read_word(1); // must be '}'
  } else {
    Fl_Group_Type::read_property(f, c);
  }
}

void Fl_Flex_Type::postprocess_read()
{
  if (fixedSizeTupleSize==0) return;
  Fl_Flex* flex = (Fl_Flex*)o;
  for (int i=0; i<fixedSizeTupleSize; i++) {
    int ix = fixedSizeTuple[2*i];
    int size = fixedSizeTuple[2*i+1];
    if (ix>=0 && ix<flex->children()) {
      Fl_Widget *ci = flex->child(ix);
      flex->fixed(ci, size);
    }
  }
  fixedSizeTupleSize = 0;
  delete[] fixedSizeTuple;
  fixedSizeTuple = NULL;
  flex->layout();
  suspend_auto_layout = 0;
}

void Fl_Flex_Type::write_code2(Fd_Code_Writer& f) {
  const char *var = name() ? name() : "o";
  Fl_Flex* flex = (Fl_Flex*)o;
  int lm, tm, rm, bm;
  flex->margin(&lm, &tm, &rm, &bm);
  if (lm!=0 || tm!=0 || rm!=0 || bm!=0)
    f.write_c("%s%s->margin(%d, %d, %d, %d);\n", f.indent(), var, lm, tm, rm, bm);
  if (flex->gap())
    f.write_c("%s%s->gap(%d);\n", f.indent(), var, flex->gap());
  for (int i=0; i<flex->children(); ++i) {
    Fl_Widget *ci = flex->child(i);
    if (flex->fixed(ci))
      f.write_c("%s%s->fixed(%s->child(%d), %d);\n", f.indent(), var, var, i,
                flex->horizontal() ? ci->w() : ci->h());
  }
  Fl_Group_Type::write_code2(f);
}

void Fl_Flex_Type::add_child(Fl_Type* a, Fl_Type* b) {
  Fl_Group_Type::add_child(a, b);
  if (!suspend_auto_layout)
    ((Fl_Flex*)o)->layout();
}

void Fl_Flex_Type::move_child(Fl_Type* a, Fl_Type* b) {
  Fl_Group_Type::move_child(a, b);
  if (!suspend_auto_layout)
    ((Fl_Flex*)o)->layout();
}

void Fl_Flex_Type::remove_child(Fl_Type* a) {
  if (a->is_widget())
    ((Fl_Flex*)o)->fixed(((Fl_Widget_Type*)a)->o, 0);
  Fl_Group_Type::remove_child(a);
  ((Fl_Flex*)o)->layout();
}

// Change from HORIZONTAL to VERTICAL or back.
// Children in a horizontal Flex have already the full vertical height. If we
// just change to vertical, the accumulated hight of all children is too big.
// We need to relayout existing children.
void Fl_Flex_Type::change_subtype_to(int n) {
  Fl_Flex* f = (Fl_Flex*)o;
  if (f->type()==n) return;

  int nc = f->children();
  if (nc > 0) {
    int dw = Fl::box_dw(f->box());
    int dh = Fl::box_dh(f->box());
    int lm, tm, rm, bm;
    f->margin(&lm, &tm, &rm, &bm);
    int gap = f->gap();
    int fw = f->w()-dw-lm-rm-(nc*gap);
    if (fw<=nc) fw = nc; // avoid division by zero
    int fh = f->h()-dh-tm-bm-(nc*gap);
    if (fh<=nc) fh = nc; // avoid division by zero

    if (f->type()==Fl_Flex::HORIZONTAL && n==Fl_Flex::VERTICAL) {
      float scl = (float)fh/(float)fw;
      for (int i=0; i<nc; i++) {
        Fl_Widget* c = f->child(i);
        c->size(f->w(), (int)(c->w()*scl));
      }
    } else if (f->type()==Fl_Flex::VERTICAL && n==Fl_Flex::HORIZONTAL) {
      float scl = (float)fw/(float)fh;
      for (int i=0; i<nc; i++) {
        Fl_Widget* c = f->child(i);
        c->size((int)(c->h()*scl), f->h());
      }
    }
  }
  f->type(n);
  f->layout();
}

int Fl_Flex_Type::parent_is_flex(Fl_Type *t) {
  return (t->is_widget()
          && t->parent
          && t->parent->is_flex());
}

int Fl_Flex_Type::size(Fl_Type *t, char fixed_only) {
  if (!t->is_widget()) return 0;
  if (!t->parent) return 0;
  if (!t->parent->is_flex()) return 0;
  Fl_Flex_Type* ft = (Fl_Flex_Type*)t->parent;
  Fl_Flex* f = (Fl_Flex*)ft->o;
  Fl_Widget *w = ((Fl_Widget_Type*)t)->o;
  if (fixed_only && !f->fixed(w)) return 0;
  return f->horizontal() ? w->w() : w->h();
}

int Fl_Flex_Type::is_fixed(Fl_Type *t) {
  if (!t->is_widget()) return 0;
  if (!t->parent) return 0;
  if (!t->parent->is_flex()) return 0;
  Fl_Flex_Type* ft = (Fl_Flex_Type*)t->parent;
  Fl_Flex* f = (Fl_Flex*)ft->o;
  Fl_Widget *w = ((Fl_Widget_Type*)t)->o;
  return f->fixed(w);
}

// ---- Fl_Table_Type -------------------------------------------------- MARK: -

Fl_Table_Type Fl_Table_type;    // the "factory"

const char table_type_name[] = "Fl_Table";

static const int MAX_ROWS = 14;
static const int MAX_COLS = 7;

// this is a minimal table widget used as an example when adding tables in Fluid
class Fluid_Table : public Fl_Table {
  int data[MAX_ROWS][MAX_COLS];         // data array for cells

  // Draw the row/col headings
  //    Make this a dark thin upbox with the text inside.
  //
  void DrawHeader(const char *s, int X, int Y, int W, int H) {
    fl_push_clip(X,Y,W,H);
    fl_draw_box(FL_THIN_UP_BOX, X,Y,W,H, row_header_color());
    fl_color(FL_BLACK);
    fl_draw(s, X,Y,W,H, FL_ALIGN_CENTER);
    fl_pop_clip();
  }
  // Draw the cell data
  //    Dark gray text on white background with subtle border
  //
  void DrawData(const char *s, int X, int Y, int W, int H) {
    fl_push_clip(X,Y,W,H);
    // Draw cell bg
    fl_color(FL_WHITE); fl_rectf(X,Y,W,H);
    // Draw cell data
    fl_color(FL_GRAY0); fl_draw(s, X,Y,W,H, FL_ALIGN_CENTER);
    // Draw box border
    fl_color(color()); fl_rect(X,Y,W,H);
    fl_pop_clip();
  }
  // Handle drawing table's cells
  //     Fl_Table calls this function to draw each visible cell in the table.
  //     It's up to us to use FLTK's drawing functions to draw the cells the way we want.
  //
  void draw_cell(TableContext context, int ROW=0, int COL=0, int X=0, int Y=0, int W=0, int H=0) FL_OVERRIDE {
    static char s[40];
    switch ( context ) {
      case CONTEXT_STARTPAGE:                   // before page is drawn..
        fl_font(FL_HELVETICA, 16);              // set the font for our drawing operations
        return;
      case CONTEXT_COL_HEADER:                  // Draw column headers
        sprintf(s,"%c",'A'+COL);                // "A", "B", "C", etc.
        DrawHeader(s,X,Y,W,H);
        return;
      case CONTEXT_ROW_HEADER:                  // Draw row headers
        sprintf(s,"%03d:",ROW);                 // "001:", "002:", etc
        DrawHeader(s,X,Y,W,H);
        return;
      case CONTEXT_CELL:                        // Draw data in cells
        sprintf(s,"%d",data[ROW][COL]);
        DrawData(s,X,Y,W,H);
        return;
      default:
        return;
    }
  }
public:
  Fluid_Table(int x, int y, int w, int h, const char *l=0L)
  : Fl_Table(x, y, w, h, l) {
    end();
    for ( int r=0; r<MAX_ROWS; r++ )
      for ( int c=0; c<MAX_COLS; c++ )
        data[r][c] = 1000+(r*1000)+c;
    // Rows
    rows(MAX_ROWS);             // how many rows
    row_header(1);              // enable row headers (along left)
    row_height_all(20);         // default height of rows
    row_resize(0);              // disable row resizing
    // Cols
    cols(MAX_COLS);             // how many columns
    col_header(1);              // enable column headers (along top)
    col_width_all(80);          // default width of columns
    col_resize(1);              // enable column resizing
  }
};

Fl_Widget *Fl_Table_Type::widget(int X,int Y,int W,int H) {
  Fluid_Table *table = new Fluid_Table(X, Y, W, H);
  return table;
}

void Fl_Table_Type::add_child(Fl_Type* cc, Fl_Type* before) {
  Fl_Widget_Type* c = (Fl_Widget_Type*)cc;
  Fl_Widget* b = before ? ((Fl_Widget_Type*)before)->o : 0;
  if (((Fl_Table*)o)->children()==1) { // the FLuid_Table has one extra child
    fl_message("Inserting child widgets into an Fl_Table is not recommended.\n"
               "Please refer to the documentation on Fl_Table.");
  }
  ((Fl_Table*)o)->insert(*(c->o), b);
  o->redraw();
}

void Fl_Table_Type::remove_child(Fl_Type* cc) {
  Fl_Widget_Type* c = (Fl_Widget_Type*)cc;
  ((Fl_Table*)o)->remove(*(c->o));
  o->redraw();
}

void Fl_Table_Type::move_child(Fl_Type* cc, Fl_Type* before) {
  Fl_Widget_Type* c = (Fl_Widget_Type*)cc;
  Fl_Widget* b = before ? ((Fl_Widget_Type*)before)->o : 0;
  ((Fl_Table*)o)->insert(*(c->o), b);
  o->redraw();
}

Fl_Widget *Fl_Table_Type::enter_live_mode(int) {
  Fl_Group *grp = new Fluid_Table(o->x(), o->y(), o->w(), o->h());
  live_widget = grp;
  copy_properties();
  grp->end();
  return live_widget;
}

// ---- Fl_Tabs_Type --------------------------------------------------- MARK: -

Fl_Tabs_Type Fl_Tabs_type;      // the "factory"

const char tabs_type_name[] = "Fl_Tabs";

// Override group's resize behavior to do nothing to children:
void itabs::resize(int X, int Y, int W, int H) {
  Fl_Widget::resize(X,Y,W,H);
  redraw();
}

// This is called when user clicks on a widget in the window.  See
// if it is a tab title, and adjust visibility and return new selection:
// If none, return o unchanged:

Fl_Type* Fl_Tabs_Type::click_test(int x, int y) {
  Fl_Tabs *t = (Fl_Tabs*)o;
  Fl_Widget *a = t->which(x,y);
  if (!a) return 0; // didn't click on tab
  // changing the visible tab has an impact on the generated
  // source code, so mark this project as changed.
  int changed = (a!=t->value());
  // okay, run the tabs ui until they let go of mouse:
  t->handle(FL_PUSH);
  Fl::pushed(t);
  while (Fl::pushed()==t) Fl::wait();
  if (changed) set_modflag(1);
  return (Fl_Type*)(t->value()->user_data());
}

void Fl_Tabs_Type::add_child(Fl_Type* c, Fl_Type* before) {
  Fl_Group_Type::add_child(c, before);
}

void Fl_Tabs_Type::remove_child(Fl_Type* cc) {
  Fl_Widget_Type* c = (Fl_Widget_Type*)cc;
  Fl_Tabs *t = (Fl_Tabs*)o;
  if (t->value() == c->o) t->value(0);
  Fl_Group_Type::remove_child(c);
}

Fl_Widget *Fl_Tabs_Type::enter_live_mode(int) {
  Fl_Tabs *grp = new Fl_Tabs(o->x(), o->y(), o->w(), o->h());
  propagate_live_mode(grp);
  grp->value(((Fl_Tabs*)o)->value());
  return grp;
}

// ---- Fl_Scroll_Type ------------------------------------------------- MARK: -

Fl_Scroll_Type Fl_Scroll_type;  // the "factory"

const char scroll_type_name[] = "Fl_Scroll";

Fl_Menu_Item scroll_type_menu[] = {
  {"BOTH", 0, 0, 0/*(void*)Fl_Scroll::BOTH*/},
  {"HORIZONTAL", 0, 0, (void*)Fl_Scroll::HORIZONTAL},
  {"VERTICAL", 0, 0, (void*)Fl_Scroll::VERTICAL},
  {"HORIZONTAL_ALWAYS", 0, 0, (void*)Fl_Scroll::HORIZONTAL_ALWAYS},
  {"VERTICAL_ALWAYS", 0, 0, (void*)Fl_Scroll::VERTICAL_ALWAYS},
  {"BOTH_ALWAYS", 0, 0, (void*)Fl_Scroll::BOTH_ALWAYS},
  {0}};

Fl_Widget *Fl_Scroll_Type::enter_live_mode(int) {
  Fl_Group *grp = new Fl_Scroll(o->x(), o->y(), o->w(), o->h());
  grp->show();
  return propagate_live_mode(grp);
}

void Fl_Scroll_Type::copy_properties() {
  Fl_Group_Type::copy_properties();
  Fl_Scroll *s = (Fl_Scroll*)o, *d = (Fl_Scroll*)live_widget;
  d->scroll_to(s->xposition(), s->yposition());
  d->type(s->type());
  d->scrollbar.align(s->scrollbar.align());
  d->hscrollbar.align(s->hscrollbar.align());
}

// ---- Fl_Tile_Type --------------------------------------------------- MARK: -

Fl_Tile_Type Fl_Tile_type;      // the "factory"

const char tile_type_name[] = "Fl_Tile";

void Fl_Tile_Type::copy_properties() {
  Fl_Group_Type::copy_properties();
  // no additional properties
}

// ---- Fl_Wizard_Type ------------------------------------------------ MARK: -

Fl_Wizard_Type Fl_Wizard_type;  // the "factory"

const char wizard_type_name[] = "Fl_Wizard";

// Override group's resize behavior to do nothing to children:
void iwizard::resize(int X, int Y, int W, int H) {
  Fl_Widget::resize(X,Y,W,H);
  redraw();
}
