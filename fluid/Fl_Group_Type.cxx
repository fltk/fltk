//
// "$Id$"
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
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Table.H>
#include <FL/fl_message.H>
#include "Fl_Widget_Type.h"
#include "../src/flstring.h"

// Override group's resize behavior to do nothing to children:
void igroup::resize(int X, int Y, int W, int H) {
  Fl_Widget::resize(X,Y,W,H);
  redraw();
}

Fl_Group_Type Fl_Group_type;	// the "factory"

Fl_Type *Fl_Group_Type::make() {
  return Fl_Widget_Type::make();
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
    int x = n->o->x();	if (x < X) X = x;
    int y = n->o->y();	if (y < Y) Y = y;
    int r = x+n->o->w();if (r > R) R = r;
    int b = y+n->o->h();if (b > B) B = b;
  }
  t->o->resize(X,Y,R-X,B-Y);
}

extern int force_parent;

void group_cb(Fl_Widget *, void *) {
  // Find the current widget:
  Fl_Type *qq = Fl_Type::current;
  while (qq && (!qq->is_widget() || qq->is_menu_item())) qq = qq->parent;
  if (!qq || qq->level < 1 || (qq->level == 1 && !strcmp(qq->type_name(), "widget_class"))) {
    fl_message("Please select widgets to group");
    return;
  }
  Fl_Widget_Type* q = (Fl_Widget_Type*)qq;
  force_parent = 1;
  Fl_Group_Type *n = (Fl_Group_Type*)(Fl_Group_type.make());
  n->move_before(q);
  n->o->resize(q->o->x(),q->o->y(),q->o->w(),q->o->h());
  for (Fl_Type *t = Fl_Type::first; t;) {
    if (t->level != n->level || t == n || !t->selected) {
      t = t->next; continue;}
    Fl_Type *nxt = t->remove();
    t->add(n);
    t = nxt;
  }
  fix_group_size(n);
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
  for (n = q->next; n && n->level > q->level;) {
    Fl_Type *nxt = n->remove();
    n->insert(q);
    n = nxt;
  }
  delete q;
}

////////////////////////////////////////////////////////////////

#include <stdio.h>

void Fl_Group_Type::write_code1() {
  Fl_Widget_Type::write_code1();
}

void Fl_Group_Type::write_code2() {
  const char *var = name() ? name() : "o";
  write_extra_code();
  write_c("%s%s->end();\n", indent(), var);
  if (resizable()) {
    write_c("%sFl_Group::current()->resizable(%s);\n", indent(), var);
  }
  write_block_close();
}

////////////////////////////////////////////////////////////////

const char pack_type_name[] = "Fl_Pack";

Fl_Menu_Item pack_type_menu[] = {
  {"HORIZONTAL", 0, 0, (void*)Fl_Pack::HORIZONTAL},
  {"VERTICAL", 0, 0, (void*)Fl_Pack::VERTICAL},
  {0}};

Fl_Pack_Type Fl_Pack_type;	// the "factory"

////////////////////////////////////////////////////////////////

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
  void draw_cell(TableContext context, int ROW=0, int COL=0, int X=0, int Y=0, int W=0, int H=0) {
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

const char table_type_name[] = "Fl_Table";

Fl_Table_Type Fl_Table_type;	// the "factory"

Fl_Widget *Fl_Table_Type::widget(int X,int Y,int W,int H) {
  Fluid_Table *table = new Fluid_Table(X, Y, W, H);
  return table;
}

////////////////////////////////////////////////////////////////

const char tabs_type_name[] = "Fl_Tabs";

// Override group's resize behavior to do nothing to children:
void itabs::resize(int X, int Y, int W, int H) {
  Fl_Widget::resize(X,Y,W,H);
  redraw();
}

Fl_Tabs_Type Fl_Tabs_type;	// the "factory"

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

////////////////////////////////////////////////////////////////

const char wizard_type_name[] = "Fl_Wizard";

// Override group's resize behavior to do nothing to children:
void iwizard::resize(int X, int Y, int W, int H) {
  Fl_Widget::resize(X,Y,W,H);
  redraw();
}

Fl_Wizard_Type Fl_Wizard_type;	// the "factory"

// This is called when o is created.  If it is in the tab group make
// sure it is visible:

void Fl_Group_Type::add_child(Fl_Type* cc, Fl_Type* before) {
  Fl_Widget_Type* c = (Fl_Widget_Type*)cc;
  Fl_Widget* b = before ? ((Fl_Widget_Type*)before)->o : 0;
  ((Fl_Group*)o)->insert(*(c->o), b);
  o->redraw();
}

void Fl_Tabs_Type::add_child(Fl_Type* c, Fl_Type* before) {
  Fl_Group_Type::add_child(c, before);
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


// This is called when o is deleted.  If it is in the tab group make
// sure it is not visible:

void Fl_Group_Type::remove_child(Fl_Type* cc) {
  Fl_Widget_Type* c = (Fl_Widget_Type*)cc;
  ((Fl_Group*)o)->remove(c->o);
  o->redraw();
}

void Fl_Tabs_Type::remove_child(Fl_Type* cc) {
  Fl_Widget_Type* c = (Fl_Widget_Type*)cc;
  Fl_Tabs *t = (Fl_Tabs*)o;
  if (t->value() == c->o) t->value(0);
  Fl_Group_Type::remove_child(c);
}

void Fl_Table_Type::remove_child(Fl_Type* cc) {
  Fl_Widget_Type* c = (Fl_Widget_Type*)cc;
  ((Fl_Table*)o)->remove(*(c->o));
  o->redraw();
}

// move, don't change selected value:

void Fl_Group_Type::move_child(Fl_Type* cc, Fl_Type* before) {
  Fl_Widget_Type* c = (Fl_Widget_Type*)cc;
  Fl_Widget* b = before ? ((Fl_Widget_Type*)before)->o : 0;
  ((Fl_Group*)o)->remove(c->o);
  ((Fl_Group*)o)->insert(*(c->o), b);
  o->redraw();
}

void Fl_Table_Type::move_child(Fl_Type* cc, Fl_Type* before) {
  Fl_Widget_Type* c = (Fl_Widget_Type*)cc;
  Fl_Widget* b = before ? ((Fl_Widget_Type*)before)->o : 0;
  ((Fl_Table*)o)->remove(*(c->o));
  ((Fl_Table*)o)->insert(*(c->o), b);
  o->redraw();
}

////////////////////////////////////////////////////////////////
// live mode support

Fl_Widget *Fl_Group_Type::enter_live_mode(int) {
  Fl_Group *grp = new Fl_Group(o->x(), o->y(), o->w(), o->h());
  live_widget = grp;
  if (live_widget) {
    copy_properties();
    Fl_Type *n;
    for (n = next; n && n->level > level; n = n->next) {
      if (n->level == level+1)
        n->enter_live_mode();
    }
    grp->end();
  }
  return live_widget;
}

Fl_Widget *Fl_Tabs_Type::enter_live_mode(int) {
  Fl_Tabs *grp = new Fl_Tabs(o->x(), o->y(), o->w(), o->h());
  live_widget = grp;
  if (live_widget) {
    copy_properties();
    Fl_Type *n;
    for (n = next; n && n->level > level; n = n->next) {
      if (n->level == level+1)
        n->enter_live_mode();
    }
    grp->end();
  }
  grp->value(((Fl_Tabs*)o)->value());
  return live_widget;
}

Fl_Widget *Fl_Table_Type::enter_live_mode(int) {
  Fl_Group *grp = new Fluid_Table(o->x(), o->y(), o->w(), o->h());
  live_widget = grp;
  if (live_widget) {
    copy_properties();
    grp->end();
  }
  return live_widget;
}

void Fl_Group_Type::leave_live_mode() {
}

/**
 * copy all properties from the edit widget to the live widget
 */
void Fl_Group_Type::copy_properties() {
  Fl_Widget_Type::copy_properties();
}

////////////////////////////////////////////////////////////////
// some other group subclasses that fluid does not treat specially:

#include <FL/Fl_Scroll.H>

const char scroll_type_name[] = "Fl_Scroll";

Fl_Widget *Fl_Scroll_Type::enter_live_mode(int) {
  Fl_Group *grp = new Fl_Scroll(o->x(), o->y(), o->w(), o->h());
  grp->show();
  live_widget = grp;
  if (live_widget) {
    copy_properties();
    Fl_Type *n;
    for (n = next; n && n->level > level; n = n->next) {
      if (n->level == level+1)
        n->enter_live_mode();
    }
    grp->end();
  }
  return live_widget;
}

Fl_Menu_Item scroll_type_menu[] = {
  {"BOTH", 0, 0, 0/*(void*)Fl_Scroll::BOTH*/},
  {"HORIZONTAL", 0, 0, (void*)Fl_Scroll::HORIZONTAL},
  {"VERTICAL", 0, 0, (void*)Fl_Scroll::VERTICAL},
  {"HORIZONTAL_ALWAYS", 0, 0, (void*)Fl_Scroll::HORIZONTAL_ALWAYS},
  {"VERTICAL_ALWAYS", 0, 0, (void*)Fl_Scroll::VERTICAL_ALWAYS},
  {"BOTH_ALWAYS", 0, 0, (void*)Fl_Scroll::BOTH_ALWAYS},
  {0}};

Fl_Scroll_Type Fl_Scroll_type;	// the "factory"

void Fl_Scroll_Type::copy_properties() {
  Fl_Group_Type::copy_properties();
  Fl_Scroll *s = (Fl_Scroll*)o, *d = (Fl_Scroll*)live_widget;
  d->position(s->xposition(), s->yposition());
  d->type(s->type()); // TODO: get this flag from Fl_Scroll_Type!
  d->scrollbar.align(s->scrollbar.align());
  d->hscrollbar.align(s->hscrollbar.align());
}

////////////////////////////////////////////////////////////////

const char tile_type_name[] = "Fl_Tile";

Fl_Tile_Type Fl_Tile_type;	// the "factory"

void Fl_Tile_Type::copy_properties() {
  Fl_Group_Type::copy_properties();
  // no additional properties
}

//
// End of "$Id$".
//
