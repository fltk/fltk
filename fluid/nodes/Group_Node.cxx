//
// Group Node code for the Fast Light Tool Kit (FLTK).
//
// Object describing an Fl_Group and links to Window_Node.C and
// the Fl_Tabs widget, with special stuff to select tab items and
// insure that only one is visible.
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

#include "nodes/Group_Node.h"

#include "Fluid.h"
#include "proj/undo.h"
#include "app/Snap_Action.h"
#include "io/Project_Reader.h"
#include "io/Project_Writer.h"
#include "io/Code_Writer.h"
#include "widgets/Node_Browser.h"

#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Table.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/fl_message.H>
#include <FL/Fl_Scroll.H>
#include "../src/flstring.h"

#include <stdio.h>
#include <stdlib.h>


// ---- Group_Node -------------------------------------------------- MARK: -

Group_Node Group_Node::prototype;

/**
 Override group's resize behavior to do nothing to children by default.
 \param[in] X, Y, W, H new size
 */
void Fl_Group_Proxy::resize(int X, int Y, int W, int H) {
  if (Fluid.proj.tree.allow_layout > 0) {
    Fl_Group::resize(X, Y, W, H);
  } else {
    Fl_Widget::resize(X, Y, W, H);
  }
  redraw();
}

/**
 Override draw() to make groups with no box or flat box background visible.
 */
void Fl_Group_Proxy::draw() {
  if (Fluid.show_ghosted_outline && (box() == FL_NO_BOX)) {
    fl_rect(x(), y(), w(), h(), Fl::box_color(fl_color_average(FL_FOREGROUND_COLOR, color(), .1f)));
  }
  Fl_Group::draw();
}


/**
 \brief Enlarge the group size, so all children fit within.
 */
void fix_group_size(Node *tt) {
  if (!tt || !tt->is_a(Type::Group)) return;
  Group_Node* t = (Group_Node*)tt;
  int X = t->o->x();
  int Y = t->o->y();
  int R = X+t->o->w();
  int B = Y+t->o->h();
  for (Node *nn = t->next; nn && nn->level > t->level; nn = nn->next) {
    if (nn->is_true_widget()) {
      Widget_Node* n = (Widget_Node*)nn;
      int x = n->o->x();  if (x < X) X = x;
      int y = n->o->y();  if (y < Y) Y = y;
      int r = x+n->o->w();if (r > R) R = r;
      int b = y+n->o->h();if (b > B) B = b;
    }
  }
  t->o->resize(X,Y,R-X,B-Y);
}

extern void group_selected_menuitems();

void group_cb(Fl_Widget *, void *) {
  if (!Fluid.proj.tree.current) {
    fl_message("No widgets selected.");
    return;
  }
  if (!Fluid.proj.tree.current->is_widget()) {
    fl_message("Only widgets and menu items can be grouped.");
    return;
  }
  if (Fluid.proj.tree.current->is_a(Type::Menu_Item)) {
    group_selected_menuitems();
    return;
  }
  // The group will be created in the parent group of the current widget
  Node *qq = Fluid.proj.tree.current->parent;
  Widget_Node *q = static_cast<Widget_Node*>(Fluid.proj.tree.current);
  while (qq && !qq->is_a(Type::Group)) {
    qq = qq->parent;
  }
  if (!qq) {
    fl_message("Can't create a new group here.");
    return;
  }
  Fluid.proj.undo.checkpoint();
  Fluid.proj.undo.suspend();
  Fluid.proj.tree.current = qq;
  Group_Node *n = (Group_Node*)(Group_Node::prototype.make(Strategy::AS_LAST_CHILD));
  n->move_before(q);
  n->o->resize(q->o->x(),q->o->y(),q->o->w(),q->o->h());
  for (Node *t = qq->next; t && (t->level > qq->level);) {
    if (t->level != n->level || t == n || !t->selected) {
      t = t->next;
      continue;
    }
    Node *nxt = t->remove();
    t->add(n, Strategy::AS_LAST_CHILD);
    t = nxt;
  }
  fix_group_size(n);
  Fluid.proj.tree.current = q;
  n->layout_widget();
  widget_browser->rebuild();
  Fluid.proj.undo.resume();
  Fluid.proj.set_modflag(1);
}

extern void ungroup_selected_menuitems();

void ungroup_cb(Fl_Widget *, void *) {
  if (!Fluid.proj.tree.current) {
    fl_message("No widgets selected.");
    return;
  }
  if (!Fluid.proj.tree.current->is_widget()) {
    fl_message("Only widgets and menu items can be ungrouped.");
    return;
  }
  if (Fluid.proj.tree.current->is_a(Type::Menu_Item)) {
    ungroup_selected_menuitems();
    return;
  }

  Widget_Node *q = static_cast<Widget_Node*>(Fluid.proj.tree.current);
  int q_level = q->level;
  Node *qq = Fluid.proj.tree.current->parent;
  while (qq && !qq->is_true_widget()) qq = qq->parent;
  if (!qq || !qq->is_a(Type::Group)) {
    fl_message("Only menu widgets inside a group can be ungrouped.");
    return;
  }
  Fluid.proj.undo.checkpoint();
  Fluid.proj.undo.suspend();
  Fluid.proj.tree.current = qq;
  for (Node *t = qq->next; t && (t->level > qq->level);) {
    if (t->level != q_level || !t->selected) {
      t = t->next;
      continue;
    }
    Node *nxt = t->remove();
    t->insert(qq);
    t = nxt;
  }
  if (!qq->next || (qq->next->level <= qq->level)) {
    qq->remove();
    delete qq;   // qq has no children that need to be delete
  }
  Fluid.proj.tree.current = q;
  widget_browser->rebuild();
  Fluid.proj.undo.resume();
  Fluid.proj.set_modflag(1);
}

void Group_Node::ideal_size(int &w, int &h) {
  if (parent && parent->is_true_widget()) {
    Fl_Widget *p = ((Widget_Node*)parent)->o;
    w = p->w() / 2;
    h = p->h() / 2;
  } else {
    w = 140;
    h = 140;
  }
  fld::app::Snap_Action::better_size(w, h);
}

void Group_Node::write_code1(fld::io::Code_Writer& f) {
  Widget_Node::write_code1(f);
}

void Group_Node::write_code2(fld::io::Code_Writer& f) {
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
void Group_Node::add_child(Node* cc, Node* before) {
  Widget_Node* c = (Widget_Node*)cc;
  Fl_Widget* b = before ? ((Widget_Node*)before)->o : nullptr;
  ((Fl_Group*)o)->insert(*(c->o), b);
  o->redraw();
}

// This is called when o is deleted.  If it is in the tab group make
// sure it is not visible:
void Group_Node::remove_child(Node* cc) {
  Widget_Node* c = (Widget_Node*)cc;
  ((Fl_Group*)o)->remove(c->o);
  o->redraw();
}

// move, don't change selected value:
void Group_Node::move_child(Node* cc, Node* before) {
  Widget_Node* c = (Widget_Node*)cc;
  Fl_Widget* b = before ? ((Widget_Node*)before)->o : nullptr;
  ((Fl_Group*)o)->insert(*(c->o), b);
  o->redraw();
}

// live mode support
Fl_Widget* Group_Node::enter_live_mode(int) {
  Fl_Group *grp = new Fl_Group(o->x(), o->y(), o->w(), o->h());
  return propagate_live_mode(grp);
}

void Group_Node::leave_live_mode() {
}

/**
 copy all properties from the edit widget to the live widget
 */
void Group_Node::copy_properties() {
  Widget_Node::copy_properties();
}

// ---- Pack_Node --------------------------------------------------- MARK: -

Pack_Node Pack_Node::prototype;      // the "factory"

const char pack_type_name[] = "Fl_Pack";

Fl_Menu_Item pack_type_menu[] = {
  {"HORIZONTAL", 0, nullptr, (void*)Fl_Pack::HORIZONTAL},
  {"VERTICAL", 0, nullptr, (void*)Fl_Pack::VERTICAL},
  {nullptr}
};

Fl_Widget *Pack_Node::enter_live_mode(int) {
  Fl_Group *grp = new Fl_Pack(o->x(), o->y(), o->w(), o->h());
  return propagate_live_mode(grp);
}

void Pack_Node::copy_properties()
{
  Group_Node::copy_properties();
  Fl_Pack *d = (Fl_Pack*)live_widget, *s =(Fl_Pack*)o;
  d->spacing(s->spacing());
}

// ---- Flex_Node --------------------------------------------------- MARK: -

const char flex_type_name[] = "Fl_Flex";

Fl_Menu_Item flex_type_menu[] = {
  {"HORIZONTAL", 0, nullptr, (void*)Fl_Flex::HORIZONTAL},
  {"VERTICAL", 0, nullptr, (void*)Fl_Flex::VERTICAL},
  {nullptr}};

Flex_Node Flex_Node::prototype;      // the "factory"

/**
 Override flex's resize behavior to do nothing to children by default.

 \param[in] X, Y, W, H new size
 */
void Fl_Flex_Proxy::resize(int X, int Y, int W, int H) {
  if (Fluid.proj.tree.allow_layout > 0) {
    Fl_Flex::resize(X, Y, W, H);
  } else {
    Fl_Widget::resize(X, Y, W, H);
  }
  redraw();
}

/**
 Override draw() to make groups with no box or flat box background visible.
 */
void Fl_Flex_Proxy::draw() {
  if (Fluid.show_ghosted_outline && (box() == FL_NO_BOX)) {
    fl_rect(x(), y(), w(), h(), Fl::box_color(fl_color_average(FL_FOREGROUND_COLOR, color(), .1f)));
  }
  Fl_Flex::draw();
}

Fl_Widget *Flex_Node::enter_live_mode(int) {
  Fl_Flex *grp = new Fl_Flex(o->x(), o->y(), o->w(), o->h());
  propagate_live_mode(grp);
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

void Flex_Node::copy_properties()
{
  Group_Node::copy_properties();
  Fl_Flex *d = (Fl_Flex*)live_widget, *s =(Fl_Flex*)o;
  int lm, tm, rm, bm;
  s->margin(&lm, &tm, &rm, &bm);
  d->margin(lm, tm, rm, bm);
  d->gap( s->gap() );
}

void Flex_Node::copy_properties_for_children() {
  Fl_Flex *d = (Fl_Flex*)live_widget, *s =(Fl_Flex*)o;
  for (int i=0; i<s->children(); i++) {
    if (s->fixed(s->child(i)) && i<d->children()) {
      if (s->horizontal()) {
        d->fixed(d->child(i), d->child(i)->w());
      } else {
        d->fixed(d->child(i), d->child(i)->h());
      }
    }
  }
  d->layout();
}

void Flex_Node::write_properties(fld::io::Project_Writer &f)
{
  Group_Node::write_properties(f);
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

void Flex_Node::read_property(fld::io::Project_Reader &f, const char *c)
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
    Group_Node::read_property(f, c);
  }
}

void Flex_Node::postprocess_read()
{
  Fl_Flex* flex = (Fl_Flex*)o;
  if (fixedSizeTupleSize>0) {
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
    fixedSizeTuple = nullptr;
  }
  suspend_auto_layout = 0;
}

void Flex_Node::write_code2(fld::io::Code_Writer& f) {
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
  Group_Node::write_code2(f);
}

//void Flex_Node::add_child(Node* a, Node* b) {
//  Group_Node::add_child(a, b);
//  if (!suspend_auto_layout)
//    ((Fl_Flex*)o)->layout();
//}
//
//void Flex_Node::move_child(Node* a, Node* b) {
//  Group_Node::move_child(a, b);
//  if (!suspend_auto_layout)
//    ((Fl_Flex*)o)->layout();
//}

void Flex_Node::remove_child(Node* a) {
  if (a->is_widget())
    ((Fl_Flex*)o)->fixed(((Widget_Node*)a)->o, 0);
  Group_Node::remove_child(a);
//  ((Fl_Flex*)o)->layout();
  layout_widget();
}

void Flex_Node::layout_widget() {
  Fluid.proj.tree.allow_layout++;
  ((Fl_Flex*)o)->layout();
  Fluid.proj.tree.allow_layout--;
}

// Change from HORIZONTAL to VERTICAL or back.
// Children in a horizontal Flex have already the full vertical height. If we
// just change to vertical, the accumulated hight of all children is too big.
// We need to relayout existing children.
void Flex_Node::change_subtype_to(int n) {
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

int Flex_Node::parent_is_flex(Node *t) {
  return (t->is_widget()
          && t->parent
          && t->parent->is_a(Type::Flex));
}

/**
 Insert a widget in the child list so that it moves as close as possible the position.

 \param[in] child any widget in the tree but this, may already be a child of
      this and will be relocated if so
 \param[in] x, y pixel coordinates relative to the top left of the window
 */
void Flex_Node::insert_child_at(Fl_Widget *child, int x, int y) {
  Fl_Flex *flex = (Fl_Flex*)o;
  // find the insertion point closest to x, y
  int d = flex->w() + flex->h(), di = -1;
  if (flex->horizontal()) {
    int i, dx;
    for (i=0; i<flex->children(); i++) {
      dx = x - flex->child(i)->x();
      if (dx < 0) dx = -dx;
      if (dx < d) { d = dx; di = i; }
    }
    dx = x - (flex->x()+flex->w());
    if (dx < 0) dx = -dx;
    if (dx < d) { d = dx; di = i; }
  } else {
    int i, dy;
    for (i=0; i<flex->children(); i++) {
      dy = y - flex->child(i)->y();
      if (dy < 0) dy = -dy;
      if (dy < d) { d = dy; di = i; }
    }
    dy = y - (flex->y()+flex->h());
    if (dy < 0) dy = -dy;
    if (dy < d) { d = dy; di = i; }
  }
  if (di > -1) {
    flex->insert(*child, di);
  }
}

/** Move children around using the keyboard.
 \param[in] child pointer to the child type
 \param[in] key code of the last keypress when handling a FL_KEYBOARD event.
 */
void Flex_Node::keyboard_move_child(Widget_Node *child, int key) {
  Fl_Flex *flex = ((Fl_Flex*)o);
  int ix = flex->find(child->o);
  if (ix == flex->children()) return;
  if (flex->horizontal()) {
    if (key==FL_Right) {
      flex->insert(*child->o, ix+2);
    } else if (key==FL_Left) {
      if (ix > 0) flex->insert(*child->o, ix-1);
    }
  } else {
    if (key==FL_Down) {
      flex->insert(*child->o, ix+2);
    } else if (key==FL_Up) {
      if (ix > 0) flex->insert(*child->o, ix-1);
    }
  }
}

int Flex_Node::size(Node *t, char fixed_only) {
  if (!t->is_widget()) return 0;
  if (!t->parent) return 0;
  if (!t->parent->is_a(Type::Flex)) return 0;
  Flex_Node* ft = (Flex_Node*)t->parent;
  Fl_Flex* f = (Fl_Flex*)ft->o;
  Fl_Widget *w = ((Widget_Node*)t)->o;
  if (fixed_only && !f->fixed(w)) return 0;
  return f->horizontal() ? w->w() : w->h();
}

int Flex_Node::is_fixed(Node *t) {
  if (!t->is_widget()) return 0;
  if (!t->parent) return 0;
  if (!t->parent->is_a(Type::Flex)) return 0;
  Flex_Node* ft = (Flex_Node*)t->parent;
  Fl_Flex* f = (Fl_Flex*)ft->o;
  Fl_Widget *w = ((Widget_Node*)t)->o;
  return f->fixed(w);
}

// ---- Table_Node -------------------------------------------------- MARK: -

Table_Node Table_Node::prototype;    // the "factory"

static const int MAX_ROWS = 14;
static const int MAX_COLS = 7;

// this is a minimal table widget used as an example when adding tables in Fluid
class Fl_Table_Proxy : public Fl_Table {
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
  void draw_cell(TableContext context, int ROW=0, int COL=0, int X=0, int Y=0, int W=0, int H=0) override {
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
  Fl_Table_Proxy(int x, int y, int w, int h, const char *l=nullptr)
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

Fl_Widget *Table_Node::widget(int X,int Y,int W,int H) {
  Fl_Table_Proxy *table = new Fl_Table_Proxy(X, Y, W, H);
  return table;
}

void Table_Node::add_child(Node* cc, Node* before) {
  Widget_Node* c = (Widget_Node*)cc;
  Fl_Widget* b = before ? ((Widget_Node*)before)->o : nullptr;
  if (((Fl_Table*)o)->children()==1) { // the FLuid_Table has one extra child
    fl_message("Inserting child widgets into an Fl_Table is not recommended.\n"
               "Please refer to the documentation on Fl_Table.");
  }
  ((Fl_Table*)o)->insert(*(c->o), b);
  o->redraw();
}

void Table_Node::remove_child(Node* cc) {
  Widget_Node* c = (Widget_Node*)cc;
  ((Fl_Table*)o)->remove(*(c->o));
  o->redraw();
}

void Table_Node::move_child(Node* cc, Node* before) {
  Widget_Node* c = (Widget_Node*)cc;
  Fl_Widget* b = before ? ((Widget_Node*)before)->o : nullptr;
  ((Fl_Table*)o)->insert(*(c->o), b);
  o->redraw();
}

Fl_Widget *Table_Node::enter_live_mode(int) {
  Fl_Group *grp = new Fl_Table_Proxy(o->x(), o->y(), o->w(), o->h());
  live_widget = grp;
  copy_properties();
  grp->end();
  return live_widget;
}

void Table_Node::ideal_size(int &w, int &h) {
  w = 160;
  h = 120;
  fld::app::Snap_Action::better_size(w, h);
}

// ---- Tabs_Node --------------------------------------------------- MARK: -

Tabs_Node Tabs_Node::prototype;

const char tabs_type_name[] = "Fl_Tabs";

// Override group's resize behavior to do nothing to children:
void Fl_Tabs_Proxy::resize(int X, int Y, int W, int H) {
  if (Fluid.proj.tree.allow_layout > 0) {
    Fl_Tabs::resize(X, Y, W, H);
  } else {
    Fl_Widget::resize(X, Y, W, H);
  }
  redraw();
}

/**
 Override draw() to make groups with no box or flat box background visible.
 */
void Fl_Tabs_Proxy::draw() {
  if (Fluid.show_ghosted_outline && (box() == FL_NO_BOX)) {
    fl_rect(x(), y(), w(), h(), Fl::box_color(fl_color_average(FL_FOREGROUND_COLOR, color(), .1f)));
  }
  Fl_Tabs::draw();
}

// This is called when user clicks on a widget in the window.  See
// if it is a tab title, and adjust visibility and return new selection:
// If none, return o unchanged:

Node* Tabs_Node::click_test(int x, int y) {
  Fl_Tabs *t = (Fl_Tabs*)o;
  Fl_Widget *a = t->which(x,y);
  if (!a) return nullptr; // didn't click on tab
  // changing the visible tab has an impact on the generated
  // source code, so mark this project as changed.
  int changed = (a!=t->value());
  // okay, run the tabs ui until they let go of mouse:
  t->handle(FL_PUSH);
  Fl::pushed(t);
  while (Fl::pushed()==t) Fl::wait();
  if (changed) Fluid.proj.set_modflag(1);
  return (Node*)(t->value()->user_data());
}

void Tabs_Node::add_child(Node* c, Node* before) {
  Group_Node::add_child(c, before);
}

void Tabs_Node::remove_child(Node* cc) {
  Widget_Node* c = (Widget_Node*)cc;
  Fl_Tabs *t = (Fl_Tabs*)o;
  if (t->value() == c->o) t->value(nullptr);
  Group_Node::remove_child(c);
}

Fl_Widget *Tabs_Node::enter_live_mode(int) {
  Fl_Tabs *original = static_cast<Fl_Tabs*>(o);
  Fl_Tabs *clone = new Fl_Tabs(o->x(), o->y(), o->w(), o->h());
  propagate_live_mode(clone);
  int tab_index = original->find(original->value());
  if ((tab_index>=0) && (tab_index<clone->children()))
    clone->value(clone->child(tab_index));
  return clone;
}

// ---- Scroll_Node ------------------------------------------------- MARK: -

Scroll_Node Scroll_Node::prototype;  // the "factory"

const char scroll_type_name[] = "Fl_Scroll";

Fl_Menu_Item scroll_type_menu[] = {
  {"BOTH", 0, nullptr, nullptr/*(void*)Fl_Scroll::BOTH*/},
  {"HORIZONTAL", 0, nullptr, (void*)Fl_Scroll::HORIZONTAL},
  {"VERTICAL", 0, nullptr, (void*)Fl_Scroll::VERTICAL},
  {"HORIZONTAL_ALWAYS", 0, nullptr, (void*)Fl_Scroll::HORIZONTAL_ALWAYS},
  {"VERTICAL_ALWAYS", 0, nullptr, (void*)Fl_Scroll::VERTICAL_ALWAYS},
  {"BOTH_ALWAYS", 0, nullptr, (void*)Fl_Scroll::BOTH_ALWAYS},
  {nullptr}};

Fl_Widget *Scroll_Node::enter_live_mode(int) {
  Fl_Group *grp = new Fl_Scroll(o->x(), o->y(), o->w(), o->h());
  grp->show();
  return propagate_live_mode(grp);
}

void Scroll_Node::copy_properties() {
  Group_Node::copy_properties();
  Fl_Scroll *s = (Fl_Scroll*)o, *d = (Fl_Scroll*)live_widget;
  d->scroll_to(s->xposition(), s->yposition());
  d->type(s->type());
  d->scrollbar.align(s->scrollbar.align());
  d->hscrollbar.align(s->hscrollbar.align());
}

// ---- Tile_Node --------------------------------------------------- MARK: -

Tile_Node Tile_Node::prototype;      // the "factory"

const char tile_type_name[] = "Fl_Tile";

void Tile_Node::copy_properties() {
  Group_Node::copy_properties();
  // no additional properties
}

// ---- Wizard_Node ------------------------------------------------ MARK: -

Wizard_Node Wizard_Node::prototype;  // the "factory"

const char wizard_type_name[] = "Fl_Wizard";

// Override group's resize behavior to do nothing to children:
void Fl_Wizard_Proxy::resize(int X, int Y, int W, int H) {
  if (Fluid.proj.tree.allow_layout > 0) {
    Fl_Wizard::resize(X, Y, W, H);
  } else {
    Fl_Widget::resize(X, Y, W, H);
  }
  redraw();
}

/**
 Override draw() to make groups with no box or flat box background visible.
 */
void Fl_Wizard_Proxy::draw() {
  if (Fluid.show_ghosted_outline && (box() == FL_NO_BOX)) {
    fl_rect(x(), y(), w(), h(), Fl::box_color(fl_color_average(FL_FOREGROUND_COLOR, color(), .1f)));
  }
  Fl_Wizard::draw();
}

