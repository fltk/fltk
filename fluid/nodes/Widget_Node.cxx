//
// Widget Node code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2025 by Bill Spitzak and others.
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

#include "nodes/Widget_Node.h"

#include "Fluid.h"
#include "Project.h"
#include "app/Image_Asset.h"
#include "proj/mergeback.h"
#include "proj/undo.h"
#include "io/Project_Reader.h"
#include "io/Project_Writer.h"
#include "io/Code_Writer.h"
#include "nodes/Menu_Node.h"
#include "nodes/Function_Node.h"
#include "nodes/Window_Node.h"
#include "panels/widget_panel.h"

#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Table.H>
#include <FL/Fl_Input.H>
#include <FL/fl_message.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Spinner.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Flex.H>
#include "../src/flstring.h"

#include <stdio.h>
#include <stdlib.h>
#undef min
#undef max
#include <algorithm>

using namespace fld;
using namespace fld::proj;

// Make an Widget_Node subclass instance.
// It figures out the automatic size and parent of the new widget,
// creates the Fl_Widget (by calling the virtual function _make),
// adds it to the Fl_Widget hierarchy, creates a new Node
// instance, sets the widget pointers, and makes all the display
// update correctly...

int Widget_Node::is_widget() const {return 1;}
int Widget_Node::is_public() const {return public_;}

const char* subclassname(Node* l) {
  if (l->is_a(Type::Menu_Bar)) {
    Menu_Bar_Node *mb = static_cast<Menu_Bar_Node*>(l);
    if (mb->is_sys_menu_bar())
      return mb->sys_menubar_name();
  }
  if (l->is_widget()) {
    Widget_Node* p = (Widget_Node*)l;
    const char* c = p->subclass();
    if (c) return c;
    if (l->is_class()) return "Fl_Group";
    if (p->o->type() == FL_DOUBLE_WINDOW) return "Fl_Double_Window";
    if (p->type() == Type::Input) {
      if (p->o->type() == FL_FLOAT_INPUT) return "Fl_Float_Input";
      if (p->o->type() == FL_INT_INPUT) return "Fl_Int_Input";
    }
  }
  return l->type_name();
}

// Return the ideal widget size...
void
Widget_Node::ideal_size(int &w, int &h) {
  w = 120;
  h = 100;
  fld::app::Snap_Action::better_size(w, h);
}

/**
 Make a new Widget node.
 \param[in] strategy is Strategy::AS_LAST_CHILD or Strategy::AFTER_CURRENT
 \return new node
 */
Node *Widget_Node::make(Strategy strategy) {
  Node *anchor = Fluid.proj.tree.current, *pp = anchor;
  if (pp && (strategy.placement() == Strategy::AFTER_CURRENT))
    pp = pp->parent;
  while (pp && !pp->is_a(Type::Group)) {
    anchor = pp;
    strategy.placement(Strategy::AFTER_CURRENT);
    pp = pp->parent;
  }
  if (!pp || !pp->is_true_widget() || !anchor->is_true_widget()) {
    fl_message("Please select a group widget or window");
    return nullptr;
  }

  Widget_Node* p = (Widget_Node*)pp;
  Widget_Node* q = (Widget_Node*)anchor;

  // Figure out a border between widget and window:
  int B = p->o->w()/2; if (p->o->h()/2 < B) B = p->o->h()/2; if (B>25) B = 25;

  int ULX,ULY; // parent's origin in window
  if (!p->is_a(Type::Window)) { // if it is a group, add corner
    ULX = p->o->x(); ULY = p->o->y();
  } else {
    ULX = ULY = 0;
  }

  // Figure out a position and size for the widget
  int X,Y,W,H;
  if (is_a(Type::Group)) {     // fill the parent with the widget
    X = ULX+B;
    W = p->o->w()-B;
    Y = ULY+B;
    H = p->o->h()-B;
  } else if (q != p) {  // copy position and size of current widget
    W = q->o->w();
    H = q->o->h();
    X = q->o->x()+W;
    Y = q->o->y();
    if (X+W > ULX+p->o->w()) {
      X = q->o->x();
      Y = q->o->y()+H;
      if (Y+H > ULY+p->o->h()) Y = ULY+B;
    }
  } else {      // just make it small and square...
    X = ULX+B;
    Y = ULY+B;
    W = H = B;
  }

  // Construct the Node:
  Widget_Node *t = _make();
  if (!o) o = widget(0,0,100,100); // create template widget
  t->factory = this;
  // Construct the Fl_Widget:
  t->o = widget(X,Y,W,H);
  if (strategy.source() == Strategy::FROM_FILE)
    t->o->label(nullptr);
  else if (t->o->label()) t->label(t->o->label()); // allow editing
  t->o->user_data((void*)t);
  // Put it in the parent:
  //  ((Fl_Group *)(p->o))->add(t->o); (done by Node::add())
  // add to browser:
  t->add(anchor, strategy);
  t->redraw();
  return t;
}

void Widget_Node::setimage(Image_Asset *i) {
  if (i == image || is_a(Type::Window)) return;
  if (image) image->dec_ref();
  if (i) i->inc_ref();
  image = i;
  if (i) {
    o->image(i->image());
    if (o->image() && (scale_image_w_ || scale_image_h_)) {
      int iw = scale_image_w_>0 ? scale_image_w_ : o->image()->data_w();
      int ih = scale_image_h_>0 ? scale_image_h_ : o->image()->data_h();
      o->image()->scale(iw, ih, 0, 1);
    }
  } else {
    o->image(nullptr);
    //scale_image_w_ = scale_image_h_ = 0;
  }
  redraw();
}

void Widget_Node::setinactive(Image_Asset *i) {
  if (i == inactive || is_a(Type::Window)) return;
  if (inactive) inactive->dec_ref();
  if (i) i->inc_ref();
  inactive = i;
  if (i) {
    o->deimage(i->image());
    if (o->deimage()) {
      int iw = scale_deimage_w_>0 ? scale_deimage_w_ : o->deimage()->data_w();
      int ih = scale_deimage_h_>0 ? scale_deimage_h_ : o->deimage()->data_h();
      o->deimage()->scale(iw, ih, 0, 1);
    }
  } else {
    o->deimage(nullptr);
    //scale_deimage_w_ = scale_deimage_h_ = 0;
  }
  redraw();
}

void Widget_Node::setlabel(const char *n) {
  o->label(n);
  redraw();
}

Widget_Node::Widget_Node()
: override_visible_(0)
{
  for (int n=0; n<NUM_EXTRA_CODE; n++) {extra_code_[n] = nullptr; }
  subclass_ = nullptr;
  hotspot_ = 0;
  tooltip_ = nullptr;
  image_name_ = nullptr;
  inactive_name_ = nullptr;
  image = nullptr;
  inactive = nullptr;
  o = nullptr;
  public_ = 1;
  bind_image_ = 0;
  compress_image_ = 1;
  bind_deimage_ = 0;
  compress_deimage_ = 1;
  scale_image_w_ = 0;
  scale_image_h_ = 0;
  scale_deimage_w_ = 0;
  scale_deimage_h_ = 0;
}

Widget_Node::~Widget_Node() {
  if (o) {
    Fl_Window *win = o->window();
    delete o;
    if (win)
      win->redraw();
  }
  if (subclass_) free((void*)subclass_);
  if (tooltip_) free((void*)tooltip_);
  if (image_name_) {
    free((void*)image_name_);
    if (image) image->dec_ref();
  }
  if (inactive_name_) {
    free((void*)inactive_name_);
    if (inactive) inactive->dec_ref();
  }
  for (int n=0; n<NUM_EXTRA_CODE; n++) {
    if (extra_code_[n]) free((void*) extra_code_[n]);
  }
}

void Widget_Node::extra_code(int m,const char *n) {
  storestring(n,extra_code_[m]);
}

extern void redraw_browser();
void Widget_Node::subclass(const char *n) {
  if (storestring(n,subclass_) && visible)
    redraw_browser();
}

void Widget_Node::tooltip(const char *n) {
  storestring(n,tooltip_);
  o->tooltip(n);
}

void Widget_Node::image_name(const char *n) {
  setimage(Image_Asset::find(n));
  storestring(n,image_name_);
}

void Widget_Node::inactive_name(const char *n) {
  setinactive(Image_Asset::find(n));
  storestring(n,inactive_name_);
}

void Widget_Node::redraw() {
  Node *t = this;
  if (is_a(Type::Menu_Item)) {
    // find the menu button that parents this menu:
    do t = t->parent; while (t && t->is_a(Type::Menu_Item));
    // kludge to cause build_menu to be called again:
    if (t)
      t->add_child(nullptr, nullptr);
  } else {
    while (t->parent && t->parent->is_widget()) t = t->parent;
    ((Widget_Node*)t)->o->redraw();
  }
}

// the recursive part sorts all children, returns pointer to next:
Node *sort(Node *parent) {
  Node *f,*n=nullptr;
  for (f = parent ? parent->next : Fluid.proj.tree.first; ; f = n) {
    if (!f || (parent && f->level <= parent->level)) break;
    n = sort(f);
    if (!f->selected || !f->is_true_widget()) continue;
    Fl_Widget* fw = ((Widget_Node*)f)->o;
    Node *g; // we will insert before this
    for (g = parent ? parent->next : Fluid.proj.tree.first; g != f; g = g->next) {
      if (!g->selected || g->level > f->level) continue;
      Fl_Widget* gw = ((Widget_Node*)g)->o;
      if (gw->y() > fw->y()) break;
      if (gw->y() == fw->y() && gw->x() > fw->x()) break;
    }
    if (g != f) f->move_before(g);
  }
  if (parent)
    parent->layout_widget();
  return f;
}

////////////////////////////////////////////////////////////////
// The control panels!

Fl_Window *the_panel;

// All the callbacks use the argument to indicate whether to load or store.
// This avoids the need for pointers to all the widgets, and keeps the
// code localized in the callbacks.
// A value of LOAD means to load.  The hope is that this will not collide
// with any actual useful values for the argument.  I also use this to
// initialized parts of the widget that are nyi by fluid.

Widget_Node *current_widget; // one of the selected ones
void* const LOAD = (void *)"LOAD"; // "magic" pointer to indicate that we need to load values into the dialog
int numselected; // number selected
int haderror;

void name_public_cb(Fl_Choice* i, void* v) {
  if (v == LOAD) {
    i->value(current_widget->public_>0);
    if (current_widget->is_in_class()) i->hide(); else i->show();
  } else {
    int mod = 0;
    for (Node *o = Fluid.proj.tree.first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
        ((Widget_Node*)o)->public_ = i->value();
        mod = 1;
      }
    }
    if (mod) {
      Fluid.proj.set_modflag(1);
      redraw_browser();
    }
  }
}

/* Treating UNDO for text widget.

 Goal: we want to continuously update the UI while the user is typing text
 (changing the label, in this case). Code View does deferred updates, and
 the widget browser and widget panel update on every keystroke. At the same
 time, we want to limit undo actions to few and logical units.

 Caveats:

 1: the text widget has its own undo handling for the text field, but we may want to do a global undo
 2: every o->label() call will create an undo entry, but we want only one single event for all selected widgets
 3: we want a single undo for the entire editing phase, but still propagate changes as they happen

 The edit process has these main states:

 1: starting to edit [first_change==1 && !unfocus]; we must create a single undo checkpoint before anything changes
 2: continue editing [first_change==0 && !unfocus]; we must suspend any undo checkpoints
 3: done editing, unfocus [first_change==0 && unfocus]; we must make sure that undo checkpoints are enabled again
 4: losing focus without editing [first_change==1 && unfocus]; don't create and checkpoints

 We must also check:
 1: changing focus without changing text (works)
 2: copy and paste, drag and drop operations (works)
 3: save operation without unfocus event (works)
 */
void label_cb(Fl_Input* i, void *v) {
  static int first_change = 1;
  if (v == LOAD) {
    i->value(current_widget->label());
    first_change = 1;
  } else {
    if (i->changed()) {
      Fluid.proj.undo.suspend();
      int mod = 0;
      for (Node *o = Fluid.proj.tree.first; o; o = o->next) {
        if (o->selected && o->is_widget()) {
          if (!mod) {
            if (first_change) {
              Fluid.proj.undo.resume();
              Fluid.proj.undo.checkpoint();
              Fluid.proj.undo.suspend();
              first_change = 0;
            }
            mod = 1;
          }
          o->label(i->value());
        }
      }
      Fluid.proj.undo.resume();
      if (mod) Fluid.proj.set_modflag(1);
    }
    int r = (int)Fl::callback_reason();
    if ( (r == FL_REASON_LOST_FOCUS) || (r == FL_REASON_ENTER_KEY) )
      first_change = 1;
  }
}






int widget_i = 0;

static int vars_i_cb(const fld::widget::Formula_Input*, void *v) {
  return widget_i;
}

static int vars_x_cb(const fld::widget::Formula_Input*, void *v) {
  Node *t = (Node*)v;
  if (t->is_widget())
    return ((Widget_Node*)t)->o->x();
  return 0;
}

static int vars_y_cb(const fld::widget::Formula_Input*, void *v) {
  Node *t = (Node*)v;
  if (t->is_widget())
    return ((Widget_Node*)t)->o->y();
  return 0;
}

static int vars_w_cb(const fld::widget::Formula_Input*, void *v) {
  Node *t = (Node*)v;
  if (t->is_widget())
    return ((Widget_Node*)t)->o->w();
  return 0;
}

static int vars_h_cb(const fld::widget::Formula_Input*, void *v) {
  Node *t = (Node*)v;
  if (t->is_widget())
    return ((Widget_Node*)t)->o->h();
  return 0;
}

static int vars_px_cb(const fld::widget::Formula_Input*, void *v) {
  Node *t = ((Node*)v)->parent;
  if (t && t->is_widget())
    return ((Widget_Node*)t)->o->x();
  return 0;
}

static int vars_py_cb(const fld::widget::Formula_Input*, void *v) {
  Node *t = ((Node*)v)->parent;
  if (t && t->is_widget())
    return ((Widget_Node*)t)->o->y();
  return 0;
}

static int vars_pw_cb(const fld::widget::Formula_Input*, void *v) {
  Node *t = ((Node*)v)->parent;
  if (t && t->is_widget())
    return ((Widget_Node*)t)->o->w();
  return 0;
}

static int vars_ph_cb(const fld::widget::Formula_Input*, void *v) {
  Node *t = ((Node*)v)->parent;
  if (t && t->is_widget())
    return ((Widget_Node*)t)->o->h();
  return 0;
}

static int vars_sx_cb(const fld::widget::Formula_Input*, void *v) {
  Node *t = ((Node*)v)->prev_sibling();
  if (t && t->is_widget())
    return ((Widget_Node*)t)->o->x();
  return 0;
}

static int vars_sy_cb(const fld::widget::Formula_Input*, void *v) {
  Node *t = ((Node*)v)->prev_sibling();
  if (t && t->is_widget())
    return ((Widget_Node*)t)->o->y();
  return 0;
}

static int vars_sw_cb(const fld::widget::Formula_Input*, void *v) {
  Node *t = ((Node*)v)->prev_sibling();
  if (t && t->is_widget())
    return ((Widget_Node*)t)->o->w();
  return 0;
}

static int vars_sh_cb(const fld::widget::Formula_Input*, void *v) {
  Node *t = ((Node*)v)->prev_sibling();
  if (t && t->is_widget())
    return ((Widget_Node*)t)->o->h();
  return 0;
}

static int bbox_x, bbox_y, bbox_r, bbox_b;

static void calculate_bbox(Node *p) {
  char first = 1;
  bbox_x = bbox_y = bbox_r = bbox_b = 0;
  for (p=p->first_child(); p; p=p->next_sibling()) {
    if (p->is_widget()) {
      Fl_Widget *o = ((Widget_Node*)p)->o;
      if (first) {
        bbox_x = o->x(); bbox_y = o->y();
        bbox_r = o->x() + o->w(); bbox_b = o->y() + o->h();
        first = 0;
      } else {
        bbox_x = std::min(bbox_x, o->x());
        bbox_y = std::min(bbox_y, o->y());
        bbox_r = std::max(bbox_r, o->x() + o->w());
        bbox_b = std::max(bbox_b, o->y() + o->h());
      }
    }
  }
}

static int vars_cx_cb(const fld::widget::Formula_Input*, void *v) {
  calculate_bbox((Node*)v);
  return bbox_x;
}

static int vars_cy_cb(const fld::widget::Formula_Input*, void *v) {
  calculate_bbox((Node*)v);
  return bbox_y;
}

static int vars_cw_cb(const fld::widget::Formula_Input*, void *v) {
  calculate_bbox((Node*)v);
  return bbox_r - bbox_x;
}

static int vars_ch_cb(const fld::widget::Formula_Input*, void *v) {
  calculate_bbox((Node*)v);
  return bbox_b - bbox_y;
}

fld::widget::Formula_Input_Vars widget_vars[] = {
  { "i", vars_i_cb },   // zero based counter of selected widgets
  { "x", vars_x_cb },   // position and size of current widget
  { "y", vars_y_cb },
  { "w", vars_w_cb },
  { "h", vars_h_cb },
  { "px", vars_px_cb }, // position and size of parent widget
  { "py", vars_py_cb },
  { "pw", vars_pw_cb },
  { "ph", vars_ph_cb },
  { "sx", vars_sx_cb }, // position and size of previous sibling
  { "sy", vars_sy_cb },
  { "sw", vars_sw_cb },
  { "sh", vars_sh_cb },
  { "cx", vars_cx_cb }, // position and size of bounding box of all children
  { "cy", vars_cy_cb },
  { "cw", vars_cw_cb },
  { "ch", vars_ch_cb },
  { nullptr }
};





////////////////////////////////////////////////////////////////

// turn number to string or string to number for saving to file:
// does not work for hierarchical menus!

const char *item_name(Fl_Menu_Item* m, int i) {
  if (m) {
    while (m->label()) {
      if (m->argument() == i) return m->label();
      m++;
    }
  }
  static char buffer[20];
  sprintf(buffer, "%d", i);
  return buffer;
}
int item_number(Fl_Menu_Item* m, const char* i) {
  if (!i)
    return 0;
  if (m && i) {
    if (i[0]=='F' && i[1]=='L' && i[2]=='_') i += 3;
    while (m->label()) {
      if (!strcmp(m->label(), i)) return int(m->argument());
      m++;
    }
  }
  return atoi(i);
}

#define ZERO_ENTRY 1000

Fl_Menu_Item boxmenu[] = {
  {"NO_BOX",0,nullptr,(void *)ZERO_ENTRY},
  {"boxes",0,nullptr,nullptr,FL_SUBMENU},
  {"UP_BOX",0,nullptr,(void *)FL_UP_BOX},
  {"DOWN_BOX",0,nullptr,(void *)FL_DOWN_BOX},
  {"FLAT_BOX",0,nullptr,(void *)FL_FLAT_BOX},
  {"BORDER_BOX",0,nullptr,(void *)FL_BORDER_BOX},
  {"THIN_UP_BOX",0,nullptr,(void *)FL_THIN_UP_BOX},
  {"THIN_DOWN_BOX",0,nullptr,(void *)FL_THIN_DOWN_BOX},
  {"ENGRAVED_BOX",0,nullptr,(void *)FL_ENGRAVED_BOX},
  {"EMBOSSED_BOX",0,nullptr,(void *)FL_EMBOSSED_BOX},
  {"ROUND_UP_BOX",0,nullptr,(void *)FL_ROUND_UP_BOX},
  {"ROUND_DOWN_BOX",0,nullptr,(void *)FL_ROUND_DOWN_BOX},
  {"DIAMOND_UP_BOX",0,nullptr,(void *)FL_DIAMOND_UP_BOX},
  {"DIAMOND_DOWN_BOX",0,nullptr,(void *)FL_DIAMOND_DOWN_BOX},
  {"SHADOW_BOX",0,nullptr,(void *)FL_SHADOW_BOX},
  {"ROUNDED_BOX",0,nullptr,(void *)FL_ROUNDED_BOX},
  {"RSHADOW_BOX",0,nullptr,(void *)FL_RSHADOW_BOX},
  {"RFLAT_BOX",0,nullptr,(void *)FL_RFLAT_BOX},
  {"OVAL_BOX",0,nullptr,(void *)FL_OVAL_BOX},
  {"OSHADOW_BOX",0,nullptr,(void *)FL_OSHADOW_BOX},
  {"OFLAT_BOX",0,nullptr,(void *)FL_OFLAT_BOX},
  {"PLASTIC_UP_BOX",0,nullptr,(void *)FL_PLASTIC_UP_BOX},
  {"PLASTIC_DOWN_BOX",0,nullptr,(void *)FL_PLASTIC_DOWN_BOX},
  {"PLASTIC_THIN_UP_BOX",0,nullptr,(void *)FL_PLASTIC_THIN_UP_BOX},
  {"PLASTIC_THIN_DOWN_BOX",0,nullptr,(void *)FL_PLASTIC_THIN_DOWN_BOX},
  {"PLASTIC_ROUND_UP_BOX",0,nullptr,(void *)FL_PLASTIC_ROUND_UP_BOX},
  {"PLASTIC_ROUND_DOWN_BOX",0,nullptr,(void *)FL_PLASTIC_ROUND_DOWN_BOX},
  {"GTK_UP_BOX",0,nullptr,(void *)FL_GTK_UP_BOX},
  {"GTK_DOWN_BOX",0,nullptr,(void *)FL_GTK_DOWN_BOX},
  {"GTK_THIN_UP_BOX",0,nullptr,(void *)FL_GTK_THIN_UP_BOX},
  {"GTK_THIN_DOWN_BOX",0,nullptr,(void *)FL_GTK_THIN_DOWN_BOX},
  {"GTK_ROUND_UP_BOX",0,nullptr,(void *)FL_GTK_ROUND_UP_BOX},
  {"GTK_ROUND_DOWN_BOX",0,nullptr,(void *)FL_GTK_ROUND_DOWN_BOX},
  {"GLEAM_UP_BOX",0,nullptr,(void *)FL_GLEAM_UP_BOX},
  {"GLEAM_DOWN_BOX",0,nullptr,(void *)FL_GLEAM_DOWN_BOX},
  {"GLEAM_THIN_UP_BOX",0,nullptr,(void *)FL_GLEAM_THIN_UP_BOX},
  {"GLEAM_THIN_DOWN_BOX",0,nullptr,(void *)FL_GLEAM_THIN_DOWN_BOX},
  {"GLEAM_ROUND_UP_BOX",0,nullptr,(void *)FL_GLEAM_ROUND_UP_BOX},
  {"GLEAM_ROUND_DOWN_BOX",0,nullptr,(void *)FL_GLEAM_ROUND_DOWN_BOX},
  {"OXY_UP_BOX",0,nullptr,(void *)FL_OXY_UP_BOX},
  {"OXY_DOWN_BOX",0,nullptr,(void *)FL_OXY_DOWN_BOX},
  {"OXY_THIN_UP_BOX",0,nullptr,(void *)FL_OXY_THIN_UP_BOX},
  {"OXY_THIN_DOWN_BOX",0,nullptr,(void *)FL_OXY_THIN_DOWN_BOX},
  {"OXY_ROUND_UP_BOX",0,nullptr,(void *)FL_OXY_ROUND_UP_BOX},
  {"OXY_ROUND_DOWN_BOX",0,nullptr,(void *)FL_OXY_ROUND_DOWN_BOX},
  {"OXY_BUTTON_UP_BOX",0,nullptr,(void *)FL_OXY_BUTTON_UP_BOX},
  {"OXY_BUTTON_DOWN_BOX",0,nullptr,(void *)FL_OXY_BUTTON_DOWN_BOX},
  {nullptr},
  {"frames",0,nullptr,nullptr,FL_SUBMENU},
  {"UP_FRAME",0,nullptr,(void *)FL_UP_FRAME},
  {"DOWN_FRAME",0,nullptr,(void *)FL_DOWN_FRAME},
  {"THIN_UP_FRAME",0,nullptr,(void *)FL_THIN_UP_FRAME},
  {"THIN_DOWN_FRAME",0,nullptr,(void *)FL_THIN_DOWN_FRAME},
  {"ENGRAVED_FRAME",0,nullptr,(void *)FL_ENGRAVED_FRAME},
  {"EMBOSSED_FRAME",0,nullptr,(void *)FL_EMBOSSED_FRAME},
  {"BORDER_FRAME",0,nullptr,(void *)FL_BORDER_FRAME},
  {"SHADOW_FRAME",0,nullptr,(void *)FL_SHADOW_FRAME},
  {"ROUNDED_FRAME",0,nullptr,(void *)FL_ROUNDED_FRAME},
  {"OVAL_FRAME",0,nullptr,(void *)FL_OVAL_FRAME},
  {"PLASTIC_UP_FRAME",0,nullptr,(void *)FL_PLASTIC_UP_FRAME},
  {"PLASTIC_DOWN_FRAME",0,nullptr,(void *)FL_PLASTIC_DOWN_FRAME},
  {"GTK_UP_FRAME",0,nullptr,(void *)FL_GTK_UP_FRAME},
  {"GTK_DOWN_FRAME",0,nullptr,(void *)FL_GTK_DOWN_FRAME},
  {"GTK_THIN_UP_FRAME",0,nullptr,(void *)FL_GTK_THIN_UP_FRAME},
  {"GTK_THIN_DOWN_FRAME",0,nullptr,(void *)FL_GTK_THIN_DOWN_FRAME},
  {"GLEAM_UP_FRAME",0,nullptr,(void *)FL_GLEAM_UP_FRAME},
  {"GLEAM_DOWN_FRAME",0,nullptr,(void *)FL_GLEAM_DOWN_FRAME},
  {"OXY_UP_FRAME",0,nullptr,(void *)FL_OXY_UP_FRAME},
  {"OXY_DOWN_FRAME",0,nullptr,(void *)FL_OXY_DOWN_FRAME},
  {"OXY_THIN_UP_FRAME",0,nullptr,(void *)FL_OXY_THIN_UP_FRAME},
  {"OXY_THIN_DOWN_FRAME",0,nullptr,(void *)FL_OXY_THIN_DOWN_FRAME},
  {nullptr},
  {nullptr}};

const char *boxname(int i) {
  if (!i) i = ZERO_ENTRY;
  for (int j = 0; j < int(sizeof(boxmenu)/sizeof(*boxmenu)); j++)
    if (boxmenu[j].argument() == i) return boxmenu[j].label();
  return nullptr;
}

int boxnumber(const char *i) {
  if (i[0]=='F' && i[1]=='L' && i[2]=='_') i += 3;
  for (int j = 0; j < int(sizeof(boxmenu)/sizeof(*boxmenu)); j++)
    if (boxmenu[j].label() && !strcmp(boxmenu[j].label(), i)) {
      return int(boxmenu[j].argument());
    }
  return 0;
}




////////////////////////////////////////////////////////////////

Fl_Menu_Item whenmenu[] = {
  // set individual bits
  {"FL_WHEN_CHANGED",0,nullptr,(void*)FL_WHEN_CHANGED, FL_MENU_TOGGLE},
  {"FL_WHEN_NOT_CHANGED",0,nullptr,(void*)FL_WHEN_NOT_CHANGED, FL_MENU_TOGGLE},
  {"FL_WHEN_RELEASE",0,nullptr,(void*)FL_WHEN_RELEASE, FL_MENU_TOGGLE},
  {"FL_WHEN_ENTER_KEY",0,nullptr,(void*)FL_WHEN_ENTER_KEY, FL_MENU_TOGGLE},
  {"FL_WHEN_CLOSED",0,nullptr,(void*)FL_WHEN_CLOSED, FL_MENU_TOGGLE|FL_MENU_DIVIDER},
  // set bit combinations
  {"FL_WHEN_NEVER",0,nullptr,(void*)FL_WHEN_NEVER},
  {"FL_WHEN_RELEASE_ALWAYS",0,nullptr,(void*)FL_WHEN_RELEASE_ALWAYS},
  {"FL_WHEN_ENTER_KEY_ALWAYS",0,nullptr,(void*)FL_WHEN_ENTER_KEY_ALWAYS},
  {"FL_WHEN_ENTER_KEY_CHANGED",0,nullptr,(void*)FL_WHEN_ENTER_KEY_CHANGED},
  {nullptr}};


static Fl_Menu_Item whensymbolmenu[] = {
  /*  0 */ {"FL_WHEN_NEVER",0,nullptr,(void*)FL_WHEN_NEVER},
  /*  1 */ {"FL_WHEN_CHANGED",0,nullptr,(void*)FL_WHEN_CHANGED},
  /*  2 */ {"FL_WHEN_NOT_CHANGED",0,nullptr,(void*)FL_WHEN_NOT_CHANGED},
  /*  3 */ {"FL_WHEN_CHANGED | FL_WHEN_NOT_CHANGED",0,nullptr,(void*)(FL_WHEN_CHANGED|FL_WHEN_NOT_CHANGED)},
  /*  4 */ {"FL_WHEN_RELEASE",0,nullptr,(void*)FL_WHEN_RELEASE},
  /*  5 */ {"FL_WHEN_CHANGED | FL_WHEN_RELEASE",0,nullptr,(void*)(FL_WHEN_CHANGED|FL_WHEN_RELEASE)},
  /*  6 */ {"FL_WHEN_RELEASE_ALWAYS",0,nullptr,(void*)FL_WHEN_RELEASE_ALWAYS},
  /*  7 */ {"FL_WHEN_CHANGED | FL_WHEN_RELEASE_ALWAYS",0,nullptr,(void*)(FL_WHEN_CHANGED|FL_WHEN_RELEASE_ALWAYS)},
  /*  8 */ {"FL_WHEN_ENTER_KEY",0,nullptr,(void*)FL_WHEN_ENTER_KEY},
  /*  9 */ {"FL_WHEN_CHANGED | FL_WHEN_ENTER_KEY",0,nullptr,(void*)(FL_WHEN_CHANGED|FL_WHEN_ENTER_KEY)},
  /* 10 */ {"FL_WHEN_ENTER_KEY_ALWAYS",0,nullptr,(void*)FL_WHEN_ENTER_KEY_ALWAYS},
  /* 11 */ {"FL_WHEN_ENTER_KEY_CHANGED",0,nullptr,(void*)FL_WHEN_ENTER_KEY_CHANGED},
  /* 12 */ {"FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY",0,nullptr,(void*)(FL_WHEN_RELEASE|FL_WHEN_ENTER_KEY)},
  /* 13 */ {"FL_WHEN_RELEASE | FL_WHEN_CHANGED | FL_WHEN_ENTER_KEY",0,nullptr,(void*)(FL_WHEN_RELEASE|FL_WHEN_CHANGED|FL_WHEN_ENTER_KEY)},
  /* 14 */ {"FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY_ALWAYS",0,nullptr,(void*)(FL_WHEN_RELEASE|FL_WHEN_ENTER_KEY_ALWAYS)},
  /* 15 */ {"FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY_CHANGED",0,nullptr,(void*)(FL_WHEN_RELEASE|FL_WHEN_ENTER_KEY_CHANGED)},
  {nullptr}
};

// Return a text string representing  the Fl_When value n
const char* when_symbol_name(int n) {
  static char sym[128];
  if (n == FL_WHEN_CLOSED) {
    strcpy(sym, "FL_WHEN_CLOSED");
  } else {
    strcpy(sym, whensymbolmenu[n&15].label());
    if (n & FL_WHEN_CLOSED)
      strcat(sym, " | FL_WHEN_CLOSED");
  }
  return sym;
}

// Set the check marks in the "when()" menu according to the Fl_When value n
void set_whenmenu(int n) {
  if (n&FL_WHEN_CHANGED)      whenmenu[0].set(); else whenmenu[0].clear();
  if (n&FL_WHEN_NOT_CHANGED)  whenmenu[1].set(); else whenmenu[1].clear();
  if (n&FL_WHEN_RELEASE)      whenmenu[2].set(); else whenmenu[2].clear();
  if (n&FL_WHEN_ENTER_KEY)    whenmenu[3].set(); else whenmenu[3].clear();
  if (n&FL_WHEN_CLOSED)       whenmenu[4].set(); else whenmenu[4].clear();
}


uchar Widget_Node::resizable() const {
  if (is_a(Type::Window)) return ((Fl_Window*)o)->resizable() != nullptr;
  Fl_Group* p = (Fl_Group*)o->parent();
  if (p) return p->resizable() == o;
  else return 0;
}

void Widget_Node::resizable(uchar v) {
  if (v) {
    if (resizable()) return;
    if (is_a(Type::Window)) ((Fl_Window*)o)->resizable(o);
    else {
      Fl_Group* p = (Fl_Group*)o->parent();
      if (p) p->resizable(o);
    }
  } else {
    if (!resizable()) return;
    if (is_a(Type::Window)) {
      ((Fl_Window*)o)->resizable(nullptr);
    } else {
      Fl_Group* p = (Fl_Group*)o->parent();
      if (p) p->resizable(nullptr);
    }
  }
}



////////////////////////////////////////////////////////////////

Fl_Menu_Item fontmenu[] = {
  {"Helvetica"},
  {"Helvetica bold"},
  {"Helvetica italic"},
  {"Helvetica bold italic"},
  {"Courier"},
  {"Courier bold"},
  {"Courier italic"},
  {"Courier bold italic"},
  {"Times"},
  {"Times bold"},
  {"Times italic"},
  {"Times bold italic"},
  {"Symbol"},
  {"Terminal"},
  {"Terminal Bold"},
  {"Zapf Dingbats"},
  {nullptr}
};

Fl_Menu_Item fontmenu_w_default[] = {
  {"<default>", 0, nullptr, nullptr, FL_MENU_DIVIDER},
  {"Helvetica"},
  {"Helvetica bold"},
  {"Helvetica italic"},
  {"Helvetica bold italic"},
  {"Courier"},
  {"Courier bold"},
  {"Courier italic"},
  {"Courier bold italic"},
  {"Times"},
  {"Times bold"},
  {"Times italic"},
  {"Times bold italic"},
  {"Symbol"},
  {"Terminal"},
  {"Terminal Bold"},
  {"Zapf Dingbats"},
  {nullptr}
};



extern const char *ui_find_image_name;

Fl_Menu_Item labeltypemenu[] = {
  {"NORMAL_LABEL",0,nullptr,(void*)nullptr},
  {"SHADOW_LABEL",0,nullptr,(void*)FL_SHADOW_LABEL},
  {"ENGRAVED_LABEL",0,nullptr,(void*)FL_ENGRAVED_LABEL},
  {"EMBOSSED_LABEL",0,nullptr,(void*)FL_EMBOSSED_LABEL},
  {"NO_LABEL",0,nullptr,(void*)(FL_NO_LABEL)},
  {nullptr}};

void labeltype_cb(Fl_Choice* i, void *v) {
  if (v == LOAD) {
    int n;
    n = current_widget->o->labeltype();
    i->when(FL_WHEN_RELEASE);
    for (int j = 0; j < int(sizeof(labeltypemenu)/sizeof(*labeltypemenu)); j++)
      if (labeltypemenu[j].argument() == n) {i->value(j); break;}
  } else {
    int mod = 0;
    int m = i->value();
    int n = int(labeltypemenu[m].argument());
    if (n<0) return; // should not happen
    for (Node *o = Fluid.proj.tree.first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
        Widget_Node* p = (Widget_Node*)o;
        p->o->labeltype((Fl_Labeltype)n);
        p->redraw();
        mod = 1;
      }
    }
    if (mod) Fluid.proj.set_modflag(1);
  }
}

////////////////////////////////////////////////////////////////

Fl_Menu_Item colormenu[] = {
  { "Foreground Color",   0, nullptr, (void*)(fl_intptr_t)FL_FOREGROUND_COLOR,  0, 0, FL_HELVETICA, 11},
  { "Background Color",   0, nullptr, (void*)(fl_intptr_t)FL_BACKGROUND_COLOR,  0, 0, FL_HELVETICA, 11},
  { "Background Color 2", 0, nullptr, (void*)(fl_intptr_t)FL_BACKGROUND2_COLOR, 0, 0, FL_HELVETICA, 11},
  { "Selection Color",    0, nullptr, (void*)(fl_intptr_t)FL_SELECTION_COLOR,   0, 0, FL_HELVETICA, 11},
  { "Inactive Color",     0, nullptr, (void*)(fl_intptr_t)FL_INACTIVE_COLOR,    FL_MENU_DIVIDER, 0, FL_HELVETICA, 11},
  { "Black",              0, nullptr, (void*)(fl_intptr_t)FL_BLACK,             0, 0, FL_HELVETICA, 11},
  { "White",              0, nullptr, (void*)(fl_intptr_t)FL_WHITE,             FL_MENU_DIVIDER, 0, FL_HELVETICA, 11},
  { "Gray 0",             0, nullptr, (void*)(fl_intptr_t)FL_GRAY0,             0, 0, FL_HELVETICA, 11},
  { "Dark 3",             0, nullptr, (void*)(fl_intptr_t)FL_DARK3,             0, 0, FL_HELVETICA, 11},
  { "Dark 2",             0, nullptr, (void*)(fl_intptr_t)FL_DARK2,             0, 0, FL_HELVETICA, 11},
  { "Dark 1",             0, nullptr, (void*)(fl_intptr_t)FL_DARK1,             0, 0, FL_HELVETICA, 11},
  { "Light 1",            0, nullptr, (void*)(fl_intptr_t)FL_LIGHT1,            0, 0, FL_HELVETICA, 11},
  { "Light 2",            0, nullptr, (void*)(fl_intptr_t)FL_LIGHT2,            0, 0, FL_HELVETICA, 11},
  { "Light 3",            0, nullptr, (void*)(fl_intptr_t)FL_LIGHT3,            0, 0, FL_HELVETICA, 11},
  { nullptr }
};

void color_common(Fl_Color c) {
  int mod = 0;
  for (Node *o = Fluid.proj.tree.first; o; o = o->next) {
    if (o->selected && o->is_widget()) {
      Widget_Node* q = (Widget_Node*)o;
      q->o->color(c); q->o->redraw();
      if (q->parent && q->parent->is_a(Type::Tabs)) {
        if (q->o->parent()) q->o->parent()->redraw();
      }
      mod = 1;
    }
  }
  if (mod) Fluid.proj.set_modflag(1);
}



void color2_common(Fl_Color c) {
  int mod = 0;
  for (Node *o = Fluid.proj.tree.first; o; o = o->next) {
    if (o->selected && o->is_widget()) {
      Widget_Node* q = (Widget_Node*)o;
      q->o->selection_color(c); q->o->redraw();
      mod = 1;
    }
  }
  if (mod) Fluid.proj.set_modflag(1);
}



void labelcolor_common(Fl_Color c) {
  int mod = 0;
  for (Node *o = Fluid.proj.tree.first; o; o = o->next) {
    if (o->selected && o->is_widget()) {
      Widget_Node* q = (Widget_Node*)o;
      q->o->labelcolor(c); q->redraw();
      mod = 1;
    }
  }
  if (mod) Fluid.proj.set_modflag(1);
}



static Fl_Button* relative(Fl_Widget* o, int i) {
  Fl_Group* g = (Fl_Group*)(o->parent());
  return (Fl_Button*)(g->child(g->find(*o)+i));
}

static Fl_Menu_Item alignmenu[] = {
  {"FL_ALIGN_CENTER",0,nullptr,(void*)(fl_intptr_t)(FL_ALIGN_CENTER)},
  {"FL_ALIGN_TOP",0,nullptr,(void*)(fl_intptr_t)(FL_ALIGN_TOP)},
  {"FL_ALIGN_BOTTOM",0,nullptr,(void*)(fl_intptr_t)(FL_ALIGN_BOTTOM)},
  {"FL_ALIGN_LEFT",0,nullptr,(void*)(fl_intptr_t)(FL_ALIGN_LEFT)},
  {"FL_ALIGN_RIGHT",0,nullptr,(void*)(fl_intptr_t)(FL_ALIGN_RIGHT)},
  {"FL_ALIGN_INSIDE",0,nullptr,(void*)(fl_intptr_t)(FL_ALIGN_INSIDE)},
  {"FL_ALIGN_CLIP",0,nullptr,(void*)(fl_intptr_t)(FL_ALIGN_CLIP)},
  {"FL_ALIGN_WRAP",0,nullptr,(void*)(fl_intptr_t)(FL_ALIGN_WRAP)},
  {"FL_ALIGN_TEXT_OVER_IMAGE",0,nullptr,(void*)(fl_intptr_t)(FL_ALIGN_TEXT_OVER_IMAGE)},
  {"FL_ALIGN_TOP_LEFT",0,nullptr,(void*)(fl_intptr_t)(FL_ALIGN_TOP_LEFT)},
  {"FL_ALIGN_TOP_RIGHT",0,nullptr,(void*)(fl_intptr_t)(FL_ALIGN_TOP_RIGHT)},
  {"FL_ALIGN_BOTTOM_LEFT",0,nullptr,(void*)(fl_intptr_t)(FL_ALIGN_BOTTOM_LEFT)},
  {"FL_ALIGN_BOTTOM_RIGHT",0,nullptr,(void*)(fl_intptr_t)(FL_ALIGN_BOTTOM_RIGHT)},
  {"FL_ALIGN_LEFT_TOP",0,nullptr,(void*)(fl_intptr_t)(FL_ALIGN_LEFT_TOP)},
  {"FL_ALIGN_RIGHT_TOP",0,nullptr,(void*)(fl_intptr_t)(FL_ALIGN_RIGHT_TOP)},
  {"FL_ALIGN_LEFT_BOTTOM",0,nullptr,(void*)(fl_intptr_t)(FL_ALIGN_LEFT_BOTTOM)},
  {"FL_ALIGN_RIGHT_BOTTOM",0,nullptr,(void*)(fl_intptr_t)(FL_ALIGN_RIGHT_BOTTOM)},
  {nullptr}};

void align_cb(Fl_Button* i, void *v) {
  Fl_Align b = Fl_Align(fl_uintptr_t(i->user_data()));
  if (v == LOAD) {
    if (current_widget->is_a(Type::Menu_Item)) {i->deactivate(); return;} else i->activate();
    i->value(current_widget->o->align() & b);
  } else {
    int mod = 0;
    Fluid.proj.undo.checkpoint();
    for (Node *o = Fluid.proj.tree.first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
        Widget_Node* q = (Widget_Node*)o;
        Fl_Align x = q->o->align();
        Fl_Align y;
        if (i->value()) {
          y = x | b;
          if (b == FL_ALIGN_LEFT || b == FL_ALIGN_TOP) {
            Fl_Button *b1 = relative(i,+1);
            b1->clear();
            y = y & ~(b1->argument());
          }
          if (b == FL_ALIGN_RIGHT || b == FL_ALIGN_BOTTOM) {
            Fl_Button *b1 = relative(i,-1);
            b1->clear();
            y = y & ~(b1->argument());
          }
        } else {
          y = x & ~b;
        }
        if (x != y) {
          q->o->align(y);
          q->redraw();
          mod = 1;
        }
      }
    }
    if (mod) Fluid.proj.set_modflag(1);
  }
}

void align_position_cb(Fl_Choice *i, void *v) {
  if (v == LOAD) {
    if (current_widget->is_a(Type::Menu_Item)) {i->deactivate(); return;} else i->activate();
    Fl_Menu_Item *mi = (Fl_Menu_Item*)i->menu();
    Fl_Align b = current_widget->o->align() & FL_ALIGN_POSITION_MASK;
    for (;mi->text;mi++) {
      if ((Fl_Align)(mi->argument())==b)
        i->value(mi);
    }
  } else {
    const Fl_Menu_Item *mi = i->menu() + i->value();
    Fl_Align b = Fl_Align(fl_uintptr_t(mi->user_data()));
    int mod = 0;
    Fluid.proj.undo.checkpoint();
    for (Node *o = Fluid.proj.tree.first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
        Widget_Node* q = (Widget_Node*)o;
        Fl_Align x = q->o->align();
        Fl_Align y = (x & ~FL_ALIGN_POSITION_MASK) | b;
        if (x != y) {
          q->o->align(y);
          q->redraw();
          mod = 1;
        }
      }
    }
    if (mod) Fluid.proj.set_modflag(1);
  }
}

void align_text_image_cb(Fl_Choice *i, void *v) {
  if (v == LOAD) {
    if (current_widget->is_a(Type::Menu_Item)) {i->deactivate(); return;} else i->activate();
    Fl_Menu_Item *mi = (Fl_Menu_Item*)i->menu();
    Fl_Align b = current_widget->o->align() & FL_ALIGN_IMAGE_MASK;
    for (;mi->text;mi++) {
      if ((Fl_Align)(mi->argument())==b)
        i->value(mi);
    }
  } else {
    const Fl_Menu_Item *mi = i->menu() + i->value();
    Fl_Align b = Fl_Align(fl_uintptr_t(mi->user_data()));
    int mod = 0;
    Fluid.proj.undo.checkpoint();
    for (Node *o = Fluid.proj.tree.first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
        Widget_Node* q = (Widget_Node*)o;
        Fl_Align x = q->o->align();
        Fl_Align y = (x & ~FL_ALIGN_IMAGE_MASK) | b;
        if (x != y) {
          q->o->align(y);
          q->redraw();
          mod = 1;
        }
      }
    }
    if (mod) Fluid.proj.set_modflag(1);
  }
}

////////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////////

// textstuff: set textfont, textsize, textcolor attributes:

// default widget returns 0 to indicate not-implemented:
// The first parameter specifies the operation:
// 0: get all values
// 1: set the text font
// 2: set the text size
// 3: set the text color
// 4: get all default values for this type
int Widget_Node::textstuff(int, Fl_Font&, int&, Fl_Color&) {
  return 0;
}



void textcolor_common(Fl_Color c) {
  Fl_Font n; int s;
  int mod = 0;
  for (Node *o = Fluid.proj.tree.first; o; o = o->next) {
    if (o->selected && o->is_widget()) {
      Widget_Node* q = (Widget_Node*)o;
      q->textstuff(3,n,s,c); q->o->redraw();
      mod = 1;
    }
  }
  if (mod) Fluid.proj.set_modflag(1);
}

////////////////////////////////////////////////////////////////
// Kludges to the panel for subclasses:




//static void flex_margin_cb(Fl_Value_Input* i, void* v,
//                           void (*load_margin)(Fl_Flex*,Fl_Value_Input*),
//                           int (*update_margin)(Fl_Flex*,int)) {
//  if (v == LOAD) {
//    if (current_widget->is_a(Type::Flex)) {
//      load_margin((Fl_Flex*)current_widget->o, i);
//    }
//  } else {
//    int mod = 0;
//    int new_value = (int)i->value();
//    for (Node *o = Fluid.proj.tree.first; o; o = o->next) {
//      if (o->selected && o->is_a(Type::Flex)) {
//        Flex_Node* q = (Flex_Node*)o;
//        Fl_Flex* w = (Fl_Flex*)q->o;
//        if (update_margin(w, new_value)) {
//          w->layout();
//          mod = 1;
//        }
//      }
//    }
//    if (mod) Fluid.proj.set_modflag(1);
//  }
//}




static void load_gap(Fl_Flex *w, Fl_Value_Input* i)
{
  int v = w->gap();
  i->value((double)v);
}

static int update_gap(Fl_Flex *w, int new_value)
{
  int g = w->gap();
  if (new_value!=g) {
    w->gap(new_value);
    return 1;
  } else {
    return 0;
  }
}

void flex_margin_gap_cb(Fl_Value_Input* i, void* v) {
  flex_margin_cb(i, v, load_gap, update_gap);
}

void position_group_cb(Fl_Group* g, void* v) {
  if (v == LOAD) {
    if (Flex_Node::parent_is_flex(current_widget)) {
      g->hide();
    } else {
      g->show();
    }
  }
  propagate_load(g, v);
}



////////////////////////////////////////////////////////////////

// subtypes:

Fl_Menu_Item *Widget_Node::subtypes() {return nullptr;}


////////////////////////////////////////////////////////////////

void propagate_load(Fl_Group* g, void* v) {
  if (v == LOAD) {
    Fl_Widget*const* a = g->array();
    for (int i=g->children(); i--;) {
      Fl_Widget* o = *a++;
      o->do_callback(o, LOAD, FL_REASON_USER);
    }
  }
}

void set_cb(Fl_Button*, void*) {
  haderror = 0;
  Fl_Widget*const* a = the_panel->array();
  for (int i=the_panel->children(); i--;) {
    Fl_Widget* o = *a++;
    if (o->changed()) {
      o->do_callback();
      if (haderror) return;
      o->clear_changed();
    }
  }
}

void ok_cb(Fl_Return_Button* o, void* v) {
  set_cb(o,v);
  if (!haderror) the_panel->hide();
}

void toggle_overlays(Fl_Widget *,void *); // in Window_Node.cxx
void overlay_cb(Fl_Button*o,void *v) {
  toggle_overlays(o,v);
}

void leave_live_mode_cb(Fl_Widget*, void*);

void live_mode_cb(Fl_Button*o,void *) {
  /// \todo live mode should end gracefully when the application quits
  ///       or when the user closes the live widget
  static Node *live_type = nullptr;
  static Fl_Widget *live_widget = nullptr;
  static Fl_Window *live_window = nullptr;
  // if 'o' is 0, we must quit live mode
  if (!o) {
    o = wLiveMode;
    o->value(0);
  }
  if (o->value()) {
    if (numselected == 1) {
      Fl_Group::current(nullptr);
      live_widget = current_widget->enter_live_mode(1);
      if (live_widget) {
        live_type = current_widget;
        Fl_Group::current(nullptr);
        int w = live_widget->w();
        int h = live_widget->h();
        live_window = new Fl_Double_Window(w+20, h+55, "Fluid Live Resize");
        live_window->box(FL_FLAT_BOX);
        live_window->color(FL_GREEN);
        Fl_Group *rsz = new Fl_Group(0, h+20, 130, 35);
        rsz->box(FL_NO_BOX);
        Fl_Box *rsz_dummy = new Fl_Box(110, h+20, 1, 25);
        rsz_dummy->box(FL_NO_BOX);
        rsz->resizable(rsz_dummy);
        Fl_Button *btn = new Fl_Button(10, h+20, 100, 25, "Exit Live Resize");
        btn->labelsize(12);
        btn->callback(leave_live_mode_cb);
        rsz->end();
        live_window->add(live_widget);
        live_widget->position(10, 10);
        live_window->resizable(live_widget);
        live_window->set_modal(); // block all other UI
        live_window->callback(leave_live_mode_cb);
        if (current_widget->is_a(Type::Window)) {
          Window_Node *w = (Window_Node*)current_widget;
          int mw = w->sr_min_w; if (mw>0) mw += 20;
          int mh = w->sr_min_h; if (mh>0) mh += 55;
          int MW = w->sr_max_w; if (MW>0) MW += 20;
          int MH = w->sr_max_h; if (MH>2) MH += 55;
          if (mw || mh || MW || MH)
            live_window->size_range(mw, mh, MW, MH);
        }
        live_window->show();
        live_widget->show();
      } else o->value(0);
    } else o->value(0);
  } else {
    if (live_type)
      live_type->leave_live_mode();
    if (live_window) {
      live_window->hide();
      Fl::delete_widget(live_window);
    }
    live_type = nullptr;
    live_widget = nullptr;
    live_window = nullptr;
  }
}

// update the panel according to current widget set:
void load_panel() {
  if (!the_panel) return;

  // find all the Fl_Widget subclasses currently selected:
  numselected = 0;
  current_widget = nullptr;
  if (Fluid.proj.tree.current) {
    if (Fluid.proj.tree.current->is_widget())
      current_widget=(Widget_Node*)Fluid.proj.tree.current;
    for (Node *o = Fluid.proj.tree.first; o; o = o->next) {
      if (o->is_widget() && o->selected) {
        numselected++;
        if (!current_widget) current_widget = (Widget_Node*)o;
      }
    }
  }
  if (current_widget && current_widget->is_a(Type::Grid)) {
    if (widget_tab_grid->parent()!=widget_tabs)
      widget_tabs->add(widget_tab_grid);
  } else {
    if (widget_tab_grid->parent()==widget_tabs) {
      widget_tabs_repo->add(widget_tab_grid);
    }
  }
  if (current_widget && current_widget->parent && current_widget->parent->is_a(Type::Grid)) {
    if (widget_tab_grid_child->parent()!=widget_tabs)
      widget_tabs->add(widget_tab_grid_child);
  } else {
    if (widget_tab_grid_child->parent()==widget_tabs) {
      widget_tabs_repo->add(widget_tab_grid_child);
    }
  }
  if (numselected)
    propagate_load(the_panel, LOAD);
  else
    the_panel->hide();
}

extern Fl_Window *widgetbin_panel;

// This is called when user double-clicks an item, open or update the panel:
void Widget_Node::open() {
  bool adjust_position = false;
  if (!the_panel) {
    the_panel = make_widget_panel();
    adjust_position = true;
  }
  load_panel();
  if (numselected) {
    the_panel->show();
    if (adjust_position) {
      if (widgetbin_panel && widgetbin_panel->visible()) {
        if (   (the_panel->x()+the_panel->w() > widgetbin_panel->x())
            && (the_panel->x() < widgetbin_panel->x()+widgetbin_panel->w())
            && (the_panel->y()+the_panel->h() > widgetbin_panel->y())
            && (the_panel->y() < widgetbin_panel->y()+widgetbin_panel->h()) )
        {
          if (widgetbin_panel->y()+widgetbin_panel->h()+the_panel->h() > Fl::h())
            the_panel->position(the_panel->x(), widgetbin_panel->y()-the_panel->h()-30);
          else
            the_panel->position(the_panel->x(), widgetbin_panel->y()+widgetbin_panel->h()+30);
        }
      }
    }
  }
}

extern void redraw_overlays();
extern void check_redraw_corresponding_parent(Node*);
extern void redraw_browser();
extern void update_codeview_position();

// Called when ui changes what objects are selected:
// p is selected object, null for all deletions (we must throw away
// old panel in that case, as the object may no longer exist)
void selection_changed(Node *p) {
  // store all changes to the current selected objects:
  if (p && the_panel && the_panel->visible()) {
    set_cb(nullptr,nullptr);
    // if there was an error, we try to leave the selected set unchanged:
    if (haderror) {
      Node *q = nullptr;
      for (Node *o = Fluid.proj.tree.first; o; o = o->next) {
        o->new_selected = o->selected;
        if (!q && o->selected) q = o;
      }
      if (!p || !p->selected) p = q;
      Fluid.proj.tree.current = p;
      redraw_browser();
      return;
    }
  }
  // update the selected flags to new set:
  Node *q = nullptr;
  for (Node *o = Fluid.proj.tree.first; o; o = o->next) {
    o->selected = o->new_selected;
    if (!q && o->selected) q = o;
  }
  if (!p || !p->selected) p = q;
  Fluid.proj.tree.current = p;
  check_redraw_corresponding_parent(p);
  redraw_overlays();
  // load the panel with the new settings:
  load_panel();
  // update the code viewer to show the code for the selected object
  update_codeview_position();
}

////////////////////////////////////////////////////////////////
// Writing the C code:

// test to see if user named a function, or typed in code:
int is_name(const char *c) {
  for (; *c; c++)
    if ((ispunct(*c)||*c=='\n') && *c!='_' && *c!=':') return 0;
  return 1;
}

// Test to see if name() is an array entry.  If so, and this is the
// highest number, return name[num+1].  Return null if not the highest
// number or a field or function.  Return name() if not an array entry.
const char *array_name(Widget_Node *o) {
  const char *c = o->name();
  if (!c) return nullptr;
  const char *d;
  for (d = c; *d != '['; d++) {
    if (!*d) return c;
    if (ispunct(*d) && *d!='_') return nullptr;
  }
  int num = atoi(d+1);
  int sawthis = 0;
  Node *t = o->prev;
  Node *tp = o;
  const char *cn = o->class_name(1);
  for (; t && t->class_name(1) == cn; tp = t, t = t->prev) {/*empty*/}
  for (t = tp; t && t->class_name(1) == cn; t = t->next) {
    if (t == o) {sawthis=1; continue;}
    const char *e = t->name();
    if (!e) continue;
    if (strncmp(c,e,d-c)) continue;
    int n1 = atoi(e+(d-c)+1);
    if (n1 > num || (n1==num && sawthis)) return nullptr;
  }
  static char buffer[128];
  // MRS: we want strncpy() here...
  strncpy(buffer,c,d-c+1);
  snprintf(buffer+(d-c+1),sizeof(buffer) - (d-c+1), "%d]",num+1);
  return buffer;
}

// Test to see if extra code is a declaration:
int isdeclare(const char *c) {
  while (isspace(*c)) c++;
  if (*c == '#') return 1;
  if (!strncmp(c,"extern",6)) return 1;
  if (!strncmp(c,"typedef",7)) return 1;
  if (!strncmp(c,"using",5)) return 1;
  return 0;
}

void Widget_Node::write_static(fld::io::Code_Writer& f) {
  const char* t = subclassname(this);
  if (!subclass() || (is_class() && !strncmp(t, "Fl_", 3))) {
    f.write_h_once("#include <FL/Fl.H>");
    f.write_h_once("#include <FL/%s.H>", t);
  }
  for (int n=0; n < NUM_EXTRA_CODE; n++) {
    if (extra_code(n) && isdeclare(extra_code(n)))
      f.write_h_once("%s", extra_code(n));
  }
  if (callback() && is_name(callback())) {
    int write_extern_declaration = 1;
    char buf[1024]; snprintf(buf, 1023, "%s(*)",  callback());
    if (is_in_class()) {
      if (has_function("static void", buf))
        write_extern_declaration = 0;
    } else {
      if (has_toplevel_function(nullptr, buf))
        write_extern_declaration = 0;
    }
    if (write_extern_declaration)
      f.write_h_once("extern void %s(%s*, %s);", callback(), t,
                    user_data_type() ? user_data_type() : "void*");
  }
  const char* k = class_name(1);
  const char* c = array_name(this);
  if (c && !k && !is_class()) {
    f.write_c("\n");
    if (!public_) f.write_c("static ");
    else f.write_h("extern %s *%s;\n", t, c);
    if (strchr(c, '[') == nullptr) f.write_c("%s *%s=(%s *)0;\n", t, c, t);
    else f.write_c("%s *%s={(%s *)0};\n", t, c, t);
  }
  if (callback() && !is_name(callback())) {
    // see if 'o' or 'v' used, to prevent unused argument warnings:
    int use_o = 0;
    int use_v = 0;
    const char *d;
    for (d = callback(); *d;) {
      if (*d == 'o' && !is_id(d[1])) use_o = 1;
      if (*d == 'v' && !is_id(d[1])) use_v = 1;
      do d++; while (is_id(*d));
      while (*d && !is_id(*d)) d++;
    }
    const char* cn = callback_name(f);
    if (k) {
      f.write_c("\nvoid %s::%s_i(%s*", k, cn, t);
    } else {
      f.write_c("\nstatic void %s(%s*", cn, t);
    }
    if (use_o) f.write_c(" o");
    const char* ut = user_data_type() ? user_data_type() : "void*";
    f.write_c(", %s", ut);
    if (use_v) f.write_c(" v");
    f.write_c(") {\n");
    f.tag(Mergeback::Tag::GENERIC, Mergeback::Tag::WIDGET_CALLBACK, 0);
    f.write_c_indented(callback(), 1, 0);
    if (*(d-1) != ';' && *(d-1) != '}') {
      const char *p = strrchr(callback(), '\n');
      if (p) p ++;
      else p = callback();
      // Only add trailing semicolon if the last line is not a preprocessor
      // statement...
      if (*p != '#' && *p) f.write_c(";");
    }
    f.write_c("\n");
    f.tag(Mergeback::Tag::WIDGET_CALLBACK, Mergeback::Tag::GENERIC, get_uid());
    f.write_c("}\n");
    if (k) {
      f.write_c("void %s::%s(%s* o, %s v) {\n", k, cn, t, ut);
      f.write_c("%s((%s*)(o", f.indent(1), k);
      Node *q = nullptr;
      for (Node* p = parent; p && p->is_widget(); q = p, p = p->parent)
        f.write_c("->parent()");
      if (!q || !q->is_a(Type::Widget_Class))
        f.write_c("->user_data()");
      f.write_c("))->%s_i(o,v);\n}\n", cn);
    }
  }
  if (image) {
    if (!f.c_contains(image))
      image->write_static(f, compress_image_);
  }
  if (inactive) {
    if (!f.c_contains(inactive))
      inactive->write_static(f, compress_deimage_);
  }
}

void Widget_Node::write_code1(fld::io::Code_Writer& f) {
  const char* t = subclassname(this);
  const char *c = array_name(this);
  if (c) {
    if (class_name(1)) {
      f.write_public(public_);
      f.write_h("%s%s *%s;\n", f.indent(1), t, c);
    }
  }
  if (class_name(1) && callback() && !is_name(callback())) {
    const char* cn = callback_name(f);
    const char* ut = user_data_type() ? user_data_type() : "void*";
    f.write_public(0);
    f.write_h("%sinline void %s_i(%s*, %s);\n", f.indent(1), cn, t, ut);
    f.write_h("%sstatic void %s(%s*, %s);\n", f.indent(1), cn, t, ut);
  }
  // figure out if local variable will be used (prevent compiler warnings):
  int wused = !name() && is_a(Type::Window);
  const char *ptr;

  f.varused = wused;

  if (!name() && !f.varused) {
    f.varused |= can_have_children();

    if (!f.varused) {
      f.varused_test = 1;
      write_widget_code(f);
      f.varused_test = 0;
    }
  }

  if (!f.varused) {
    for (int n=0; n < NUM_EXTRA_CODE; n++)
      if (extra_code(n) && !isdeclare(extra_code(n)))
      {
        int instring = 0;
        int inname = 0;
        int incomment = 0;
        int incppcomment = 0;
        for (ptr = extra_code(n); *ptr; ptr ++) {
          if (instring) {
            if (*ptr == '\\') ptr++;
            else if (*ptr == '\"') instring = 0;
          } else if (inname && !isalnum(*ptr & 255)) {
            inname = 0;
          } else if (*ptr == '/' && ptr[1]=='*') {
            incomment = 1; ptr++;
          } else if (incomment) {
            if (*ptr == '*' && ptr[1]=='/') {
              incomment = 0; ptr++;
            }
          } else if (*ptr == '/' && ptr[1]=='/') {
            incppcomment = 1; ptr++;
          } else if (incppcomment) {
            if (*ptr == '\n')
              incppcomment = 0;
          } else if (*ptr == '\"') {
            instring = 1;
          } else if (isalnum(*ptr & 255) || *ptr == '_') {
            size_t len = strspn(ptr, "0123456789_"
                                     "abcdefghijklmnopqrstuvwxyz"
                                     "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
            if (!strncmp(ptr, "o", len)) {
              f.varused = 1;
              break;
            } else {
              ptr += len - 1;
            }
          }
        }
      }
  }

  f.write_c("%s{ ", f.indent());
  write_comment_inline_c(f);
  if (f.varused) f.write_c("%s* o = ", t);
  if (name()) f.write_c("%s = ", name());
  if (is_a(Type::Window)) {
    // Handle special case where user is faking a Fl_Group type as a window,
    // there is no 2-argument constructor in that case:
    if (!strstr(t, "Window"))
      f.write_c("new %s(0, 0, %d, %d", t, o->w(), o->h());
    else
      f.write_c("new %s(%d, %d", t, o->w(), o->h());
  } else if (is_a(Type::Menu_Bar)
             && ((Menu_Bar_Node*)this)->is_sys_menu_bar()
             && is_in_class()) {
    f.write_c("(%s*)new %s(%d, %d, %d, %d",
              t, ((Menu_Bar_Node*)this)->sys_menubar_proxy_name(),
              o->x(), o->y(), o->w(), o->h());
  } else {
    f.write_c("new %s(%d, %d, %d, %d", t, o->x(), o->y(), o->w(), o->h());
  }
  if (label() && *label()) {
    f.write_c(", ");
    switch (Fluid.proj.i18n.type) {
    case fld::I18n_Type::NONE : /* None */
        f.write_cstring(label());
        break;
    case fld::I18n_Type::GNU : /* GNU gettext */
        f.write_c("%s(", Fluid.proj.i18n.gnu_function.c_str());
        f.write_cstring(label());
        f.write_c(")");
        break;
    case fld::I18n_Type::POSIX : /* POSIX catgets */
        f.write_c("catgets(%s,%s,%d,",
                  Fluid.proj.i18n.posix_file.empty() ? "_catalog" : Fluid.proj.i18n.posix_file.c_str(),
                  Fluid.proj.i18n.posix_set.c_str(), msgnum());
        f.write_cstring(label());
        f.write_c(")");
        break;
    }
  }
  f.write_c(");\n");

  f.indentation++;

  // Avoid compiler warning for unused variable.
  // Also avoid quality control warnings about incorrect allocation error handling.
  if (wused) f.write_c("%sw = o; (void)w;\n", f.indent());

  write_widget_code(f);
}

void Widget_Node::write_color(fld::io::Code_Writer& f, const char* field, Fl_Color color) {
  const char* color_name = nullptr;
  switch (color) {
  case FL_FOREGROUND_COLOR:     color_name = "FL_FOREGROUND_COLOR";     break;
  case FL_BACKGROUND2_COLOR:    color_name = "FL_BACKGROUND2_COLOR";    break;
  case FL_INACTIVE_COLOR:       color_name = "FL_INACTIVE_COLOR";       break;
  case FL_SELECTION_COLOR:      color_name = "FL_SELECTION_COLOR";      break;
  case FL_GRAY0:                color_name = "FL_GRAY0";                break;
  case FL_DARK3:                color_name = "FL_DARK3";                break;
  case FL_DARK2:                color_name = "FL_DARK2";                break;
  case FL_DARK1:                color_name = "FL_DARK1";                break;
  case FL_BACKGROUND_COLOR:     color_name = "FL_BACKGROUND_COLOR";     break;
  case FL_LIGHT1:               color_name = "FL_LIGHT1";               break;
  case FL_LIGHT2:               color_name = "FL_LIGHT2";               break;
  case FL_LIGHT3:               color_name = "FL_LIGHT3";               break;
  case FL_BLACK:                color_name = "FL_BLACK";                break;
  case FL_RED:                  color_name = "FL_RED";                  break;
  case FL_GREEN:                color_name = "FL_GREEN";                break;
  case FL_YELLOW:               color_name = "FL_YELLOW";               break;
  case FL_BLUE:                 color_name = "FL_BLUE";                 break;
  case FL_MAGENTA:              color_name = "FL_MAGENTA";              break;
  case FL_CYAN:                 color_name = "FL_CYAN";                 break;
  case FL_DARK_RED:             color_name = "FL_DARK_RED";             break;
  case FL_DARK_GREEN:           color_name = "FL_DARK_GREEN";           break;
  case FL_DARK_YELLOW:          color_name = "FL_DARK_YELLOW";          break;
  case FL_DARK_BLUE:            color_name = "FL_DARK_BLUE";            break;
  case FL_DARK_MAGENTA:         color_name = "FL_DARK_MAGENTA";         break;
  case FL_DARK_CYAN:            color_name = "FL_DARK_CYAN";            break;
  case FL_WHITE:                color_name = "FL_WHITE";                break;
  }
  const char *var = is_class() ? "this" : name() ? name() : "o";
  if (color_name) {
    f.write_c("%s%s->%s(%s);\n", f.indent(), var, field, color_name);
  } else {
    f.write_c("%s%s->%s((Fl_Color)%d);\n", f.indent(), var, field, color);
  }
}

// this is split from write_code1(fld::io::Code_Writer& f) for Window_Node:
void Widget_Node::write_widget_code(fld::io::Code_Writer& f) {
  Fl_Widget* tplate = ((Widget_Node*)factory)->o;
  const char *var = is_class() ? "this" : name() ? name() : "o";

  if (tooltip() && *tooltip()) {
    f.write_c("%s%s->tooltip(",f.indent(), var);
    switch (Fluid.proj.i18n.type) {
    case fld::I18n_Type::NONE : /* None */
        f.write_cstring(tooltip());
        break;
    case fld::I18n_Type::GNU : /* GNU gettext */
        f.write_c("%s(", Fluid.proj.i18n.gnu_function.c_str());
        f.write_cstring(tooltip());
        f.write_c(")");
        break;
    case fld::I18n_Type::POSIX : /* POSIX catgets */
        f.write_c("catgets(%s,%s,%d,",
                  Fluid.proj.i18n.posix_file.empty() ? "_catalog" : Fluid.proj.i18n.posix_file.c_str(),
                  Fluid.proj.i18n.posix_set.c_str(),
                  msgnum() + 1);
        f.write_cstring(tooltip());
        f.write_c(")");
        break;
    }
    f.write_c(");\n");
  }

  if (is_a(Type::Spinner) && ((Fl_Spinner*)o)->type() != ((Fl_Spinner*)tplate)->type())
    f.write_c("%s%s->type(%d);\n", f.indent(), var, ((Fl_Spinner*)o)->type());
  else if (o->type() != tplate->type() && !is_a(Type::Window))
    f.write_c("%s%s->type(%d);\n", f.indent(), var, o->type());
  if (o->box() != tplate->box() || subclass())
    f.write_c("%s%s->box(FL_%s);\n", f.indent(), var, boxname(o->box()));

  // write shortcut command if needed
  int shortcut = 0;
  if (is_button()) shortcut = ((Fl_Button*)o)->shortcut();
  else if (is_a(Type::Input)) shortcut = ((Fl_Input_*)o)->shortcut();
  else if (is_a(Type::Value_Input)) shortcut = ((Fl_Value_Input*)o)->shortcut();
  else if (is_a(Type::Text_Display)) shortcut = ((Fl_Text_Display*)o)->shortcut();
  if (shortcut) {
    int s = shortcut;
    f.write_c("%s%s->shortcut(", f.indent(), var);
    if (Fluid.proj.use_FL_COMMAND) {
      if (s & FL_CTRL) { f.write_c("FL_CONTROL|"); s &= ~FL_CTRL; }
      if (s & FL_META) { f.write_c("FL_COMMAND|"); s &= ~FL_META; }
    } else {
      if (s & FL_CTRL) { f.write_c("FL_CTRL|"); s &= ~FL_CTRL; }
      if (s & FL_META) { f.write_c("FL_META|"); s &= ~FL_META; }
    }
    if (s & FL_SHIFT) { f.write_c("FL_SHIFT|"); s &= ~FL_SHIFT; }
    if (s & FL_ALT) { f.write_c("FL_ALT|"); s &= ~FL_ALT; }
    if ((s < 127) && isprint(s))
      f.write_c("'%c');\n", s);
    else
      f.write_c("0x%x);\n", s);
  }

  if (is_a(Type::Button)) {
    Fl_Button* b = (Fl_Button*)o;
    if (b->down_box()) f.write_c("%s%s->down_box(FL_%s);\n", f.indent(), var,
                               boxname(b->down_box()));
    if (b->value()) f.write_c("%s%s->value(1);\n", f.indent(), var);
    if (b->compact()) f.write_c("%s%s->compact(%d);\n", f.indent(), var, b->compact());
  } else if (is_a(Type::Input_Choice)) {
    Fl_Input_Choice* b = (Fl_Input_Choice*)o;
    if (b->down_box()) f.write_c("%s%s->down_box(FL_%s);\n", f.indent(), var,
                               boxname(b->down_box()));
  } else if (is_a(Type::Menu_Manager_)) {
    Fl_Menu_* b = (Fl_Menu_*)o;
    if (b->down_box()) f.write_c("%s%s->down_box(FL_%s);\n", f.indent(), var,
                               boxname(b->down_box()));
  }
  if (o->color() != tplate->color() || subclass())
    write_color(f, "color", o->color());
  if (o->selection_color() != tplate->selection_color() || subclass())
    write_color(f, "selection_color", o->selection_color());
  if (image) {
    image->write_code(f, bind_image_, var);
    if (scale_image_w_ || scale_image_h_) {
      f.write_c("%s%s->image()->scale(", f.indent(), var);
      if (scale_image_w_>0)
        f.write_c("%d, ", scale_image_w_);
      else
        f.write_c("%s->image()->data_w(), ", var);
      if (scale_image_h_>0)
        f.write_c("%d, 0, 1);\n", scale_image_h_);
      else
        f.write_c("%s->image()->data_h(), 0, 1);\n", var);
    }
  }
  if (inactive) {
    inactive->write_code(f, bind_deimage_, var, 1);
    if (scale_deimage_w_ || scale_deimage_h_) {
      f.write_c("%s%s->deimage()->scale(", f.indent(), var);
      if (scale_deimage_w_>0)
        f.write_c("%d, ", scale_deimage_w_);
      else
        f.write_c("%s->deimage()->data_w(), ", var);
      if (scale_deimage_h_>0)
        f.write_c("%d, 0, 1);\n", scale_deimage_h_);
      else
        f.write_c("%s->deimage()->data_h(), 0, 1);\n", var);
    }
  }
  if (o->labeltype() != tplate->labeltype() || subclass())
    f.write_c("%s%s->labeltype(FL_%s);\n", f.indent(), var,
            item_name(labeltypemenu, o->labeltype()));
  if (o->labelfont() != tplate->labelfont() || subclass())
    f.write_c("%s%s->labelfont(%d);\n", f.indent(), var, o->labelfont());
  if (o->labelsize() != tplate->labelsize() || subclass())
    f.write_c("%s%s->labelsize(%d);\n", f.indent(), var, o->labelsize());
  if (o->labelcolor() != tplate->labelcolor() || subclass())
    write_color(f, "labelcolor", o->labelcolor());
  if (o->horizontal_label_margin() != tplate->horizontal_label_margin())
    f.write_c("%s%s->horizontal_label_margin(%d);\n", f.indent(), var, o->horizontal_label_margin());
  if (o->vertical_label_margin() != tplate->vertical_label_margin())
    f.write_c("%s%s->vertical_label_margin(%d);\n", f.indent(), var, o->vertical_label_margin());
  if (o->label_image_spacing() != tplate->label_image_spacing())
    f.write_c("%s%s->label_image_spacing(%d);\n", f.indent(), var, o->label_image_spacing());
  if (is_a(Type::Valuator_)) {
    Fl_Valuator* v = (Fl_Valuator*)o;
    Fl_Valuator* t = (Fl_Valuator*)(tplate);
    if (v->minimum()!=t->minimum())
      f.write_c("%s%s->minimum(%g);\n", f.indent(), var, v->minimum());
    if (v->maximum()!=t->maximum())
      f.write_c("%s%s->maximum(%g);\n", f.indent(), var, v->maximum());
    if (v->step()!=t->step())
      f.write_c("%s%s->step(%g);\n", f.indent(), var, v->step());
    if (v->value()) {
      if (is_a(Type::Scrollbar)) { // Fl_Scrollbar::value(double) is not available
        f.write_c("%s%s->Fl_Slider::value(%g);\n", f.indent(), var, v->value());
      } else {
        f.write_c("%s%s->value(%g);\n", f.indent(), var, v->value());
      }
    }
    if (is_a(Type::Slider)) {
      double x = ((Fl_Slider*)v)->slider_size();
      double y = ((Fl_Slider*)t)->slider_size();
      if (x != y) f.write_c("%s%s->slider_size(%g);\n", f.indent(), var, x);
    }
  }
  if (is_a(Type::Spinner)) {
    Fl_Spinner* v = (Fl_Spinner*)o;
    Fl_Spinner* t = (Fl_Spinner*)(tplate);
    if (v->minimum()!=t->minimum())
      f.write_c("%s%s->minimum(%g);\n", f.indent(), var, v->minimum());
    if (v->maximum()!=t->maximum())
      f.write_c("%s%s->maximum(%g);\n", f.indent(), var, v->maximum());
    if (v->step()!=t->step())
      f.write_c("%s%s->step(%g);\n", f.indent(), var, v->step());
    if (v->value()!=1.0f)
      f.write_c("%s%s->value(%g);\n", f.indent(), var, v->value());
  }

  {Fl_Font ff; int fs; Fl_Color fc; if (textstuff(4,ff,fs,fc)) {
    Fl_Font g; int s; Fl_Color c; textstuff(0,g,s,c);
    if (g != ff) f.write_c("%s%s->textfont(%d);\n", f.indent(), var, g);
    if (s != fs) f.write_c("%s%s->textsize(%d);\n", f.indent(), var, s);
    if (c != fc) write_color(f, "textcolor", c);
  }}
  const char* ud = user_data();
  if (class_name(1) && !parent->is_widget()) ud = "this";
  if (callback()) {
    f.write_c("%s%s->callback((Fl_Callback*)%s", f.indent(), var, callback_name(f));
    if (ud)
      f.write_c(", (void*)(%s));\n", ud);
    else
      f.write_c(");\n");
  } else if (ud) {
    f.write_c("%s%s->user_data((void*)(%s));\n", f.indent(), var, ud);
  }
  if (o->align() != tplate->align() || subclass()) {
    int i = o->align();
    f.write_c("%s%s->align(Fl_Align(%s", f.indent(), var,
            item_name(alignmenu, i & ~FL_ALIGN_INSIDE));
    if (i & FL_ALIGN_INSIDE) f.write_c("|FL_ALIGN_INSIDE");
    f.write_c("));\n");
  }
  Fl_When ww = o->when();
  if (ww != tplate->when() || subclass())
    f.write_c("%s%s->when(%s);\n", f.indent(), var, when_symbol_name(ww));
  if (!o->visible() && o->parent())
    f.write_c("%s%s->hide();\n", f.indent(), var);
  if (!o->active())
    f.write_c("%s%s->deactivate();\n", f.indent(), var);
  if (!is_a(Type::Group) && resizable())
    f.write_c("%sFl_Group::current()->resizable(%s);\n", f.indent(), var);
  if (hotspot()) {
    if (is_class())
      f.write_c("%shotspot(%s);\n", f.indent(), var);
    else if (is_a(Type::Window))
      f.write_c("%s%s->hotspot(%s);\n", f.indent(), var, var);
    else
      f.write_c("%s%s->window()->hotspot(%s);\n", f.indent(), var, var);
  }
}

void Widget_Node::write_extra_code(fld::io::Code_Writer& f) {
  for (int n=0; n < NUM_EXTRA_CODE; n++)
    if (extra_code(n) && !isdeclare(extra_code(n)))
      f.write_c("%s%s\n", f.indent(), extra_code(n));
}

void Widget_Node::write_block_close(fld::io::Code_Writer& f) {
  f.indentation--;
  f.write_c("%s} // %s* %s\n", f.indent(), subclassname(this),
          name() ? name() : "o");
}

void Widget_Node::write_code2(fld::io::Code_Writer& f) {
  write_extra_code(f);
  write_block_close(f);
}

////////////////////////////////////////////////////////////////

void Widget_Node::write_properties(fld::io::Project_Writer &f) {
  Node::write_properties(f);
  f.write_indent(level+1);
  switch (public_) {
    case 0: f.write_string("private"); break;
    case 1: break;
    case 2: f.write_string("protected"); break;
  }
  if (tooltip() && *tooltip()) {
    f.write_string("tooltip");
    f.write_word(tooltip());
  }
  if (image_name() && *image_name()) {
    if (scale_image_w_ || scale_image_h_)
      f.write_string("scale_image {%d %d}", scale_image_w_, scale_image_h_);
    f.write_string("image");
    f.write_word(image_name());
    f.write_string("compress_image %d", compress_image_);
  }
  if (bind_image_) f.write_string("bind_image 1");
  if (inactive_name() && *inactive_name()) {
    if (scale_deimage_w_ || scale_deimage_h_)
      f.write_string("scale_deimage {%d %d}", scale_deimage_w_, scale_deimage_h_);
    f.write_string("deimage");
    f.write_word(inactive_name());
    f.write_string("compress_deimage %d", compress_deimage_);
  }
  if (bind_deimage_) f.write_string("bind_deimage 1");
  f.write_string("xywh {%d %d %d %d}", o->x(), o->y(), o->w(), o->h());
  Fl_Widget* tplate = ((Widget_Node*)factory)->o;
  if (is_a(Type::Spinner) && ((Fl_Spinner*)o)->type() != ((Fl_Spinner*)tplate)->type()) {
    f.write_string("type");
    f.write_word(item_name(subtypes(), ((Fl_Spinner*)o)->type()));
  } else if (subtypes() && (o->type() != tplate->type() || is_a(Type::Window))) {
    f.write_string("type");
    f.write_word(item_name(subtypes(), o->type()));
  }
  if (o->box() != tplate->box()) {
    f.write_string("box"); f.write_word(boxname(o->box()));}
  if (is_a(Type::Input)) {
    Fl_Input_* b = (Fl_Input_*)o;
    if (b->shortcut()) f.write_string("shortcut 0x%x", b->shortcut());
  }
  if (is_a(Type::Value_Input)) {
    Fl_Value_Input* b = (Fl_Value_Input*)o;
    if (b->shortcut()) f.write_string("shortcut 0x%x", b->shortcut());
  }
  if (is_a(Type::Text_Display)) {
    Fl_Text_Display* b = (Fl_Text_Display*)o;
    if (b->shortcut()) f.write_string("shortcut 0x%x", b->shortcut());
  }
  if (is_a(Type::Button)) {
    Fl_Button* b = (Fl_Button*)o;
    if (b->down_box()) {
      f.write_string("down_box"); f.write_word(boxname(b->down_box()));}
    if (b->shortcut()) f.write_string("shortcut 0x%x", b->shortcut());
    if (b->value()) f.write_string("value 1");
  } else if (is_a(Type::Input_Choice)) {
    Fl_Input_Choice* b = (Fl_Input_Choice*)o;
    if (b->down_box()) {
      f.write_string("down_box"); f.write_word(boxname(b->down_box()));}
  } else if (is_a(Type::Menu_)) {
    Fl_Menu_* b = (Fl_Menu_*)o;
    if (b->down_box()) {
      f.write_string("down_box"); f.write_word(boxname(b->down_box()));}
  }
  if (o->color()!=tplate->color())
    f.write_string("color %d", o->color());
  if (o->selection_color()!=tplate->selection_color())
    f.write_string("selection_color %d", o->selection_color());
  if (o->labeltype()!=tplate->labeltype()) {
    f.write_string("labeltype");
    f.write_word(item_name(labeltypemenu, o->labeltype()));
  }
  if (o->labelfont()!=tplate->labelfont())
    f.write_string("labelfont %d", o->labelfont());
  if (o->labelsize()!=tplate->labelsize())
    f.write_string("labelsize %d", o->labelsize());
  if (o->labelcolor()!=tplate->labelcolor())
    f.write_string("labelcolor %d", o->labelcolor());
  if (o->align()!=tplate->align())
    f.write_string("align %d", o->align());
  if (o->horizontal_label_margin()!=tplate->horizontal_label_margin())
    f.write_string("h_label_margin %d", o->horizontal_label_margin());
  if (o->vertical_label_margin()!=tplate->vertical_label_margin())
    f.write_string("v_label_margin %d", o->vertical_label_margin());
  if (o->label_image_spacing()!=tplate->label_image_spacing())
    f.write_string("image_spacing %d", o->label_image_spacing());
  if (o->when() != tplate->when())
    f.write_string("when %d", o->when());
  if (is_a(Type::Valuator_)) {
    Fl_Valuator* v = (Fl_Valuator*)o;
    Fl_Valuator* t = (Fl_Valuator*)(tplate);
    if (v->minimum()!=t->minimum()) f.write_string("minimum %g",v->minimum());
    if (v->maximum()!=t->maximum()) f.write_string("maximum %g",v->maximum());
    if (v->step()!=t->step()) f.write_string("step %g",v->step());
    if (v->value()!=0.0) f.write_string("value %g",v->value());
    if (is_a(Type::Slider)) {
      double x = ((Fl_Slider*)v)->slider_size();
      double y = ((Fl_Slider*)t)->slider_size();
      if (x != y) f.write_string("slider_size %g", x);
    }
  }
  if (is_a(Type::Spinner)) {
    Fl_Spinner* v = (Fl_Spinner*)o;
    Fl_Spinner* t = (Fl_Spinner*)(tplate);
    if (v->minimum()!=t->minimum()) f.write_string("minimum %g",v->minimum());
    if (v->maximum()!=t->maximum()) f.write_string("maximum %g",v->maximum());
    if (v->step()!=t->step()) f.write_string("step %g",v->step());
    if (v->value()!=1.0) f.write_string("value %g",v->value());
  }
  {Fl_Font ff; int fs; Fl_Color fc; if (textstuff(4,ff,fs,fc)) {
    Fl_Font ft; int s; Fl_Color c; textstuff(0,ft,s,c);
    if (ft != ff) f.write_string("textfont %d", ft);
    if (s != fs) f.write_string("textsize %d", s);
    if (c != fc) f.write_string("textcolor %d", c);
  }}
  if (!o->visible() && !override_visible_) f.write_string("hide");
  if (!o->active()) f.write_string("deactivate");
  if (resizable()) f.write_string("resizable");
  if (hotspot()) f.write_string(is_a(Type::Menu_Item) ? "divider" : "hotspot");
  for (int n=0; n < NUM_EXTRA_CODE; n++) if (extra_code(n)) {
    f.write_indent(level+1);
    f.write_string("code%d",n);
    f.write_word(extra_code(n));
  }
  if (subclass()) {
    f.write_indent(level+1);
    f.write_string("class");
    f.write_word(subclass());
  }
}

void Widget_Node::read_property(fld::io::Project_Reader &f, const char *c) {
  int x,y,w,h; Fl_Font ft; int s; Fl_Color cc;
  if (!strcmp(c,"private")) {
    public_ = 0;
  } else if (!strcmp(c,"protected")) {
    public_ = 2;
  } else if (!strcmp(c,"xywh")) {
    if (sscanf(f.read_word(),"%d %d %d %d",&x,&y,&w,&h) == 4) {
      x += Fluid.pasteoffset;
      y += Fluid.pasteoffset;
      // FIXME temporary change!
      if (f.read_version>=2.0 && o->parent() && o->parent()!=o->window()) {
        x += o->parent()->x();
        y += o->parent()->y();
      }
      o->resize(x,y,w,h);
    }
  } else if (!strcmp(c,"tooltip")) {
    tooltip(f.read_word());
  } else if (!strcmp(c,"scale_image")) {
    if (sscanf(f.read_word(),"%d %d",&w,&h) == 2) {
      scale_image_w_ = w;
      scale_image_h_ = h;
    }
  } else if (!strcmp(c,"image")) {
    image_name(f.read_word());
    // starting in 2023, `image` is always followed by `compress_image`
    // the code below is for compatibility with older .fl files
    const char *ext = fl_filename_ext(image_name_);
    if (   strcmp(ext, ".jpg")
        && strcmp(ext, ".png")
        && strcmp(ext, ".svg")
        && strcmp(ext, ".svgz"))
      compress_image_ = 0; // if it is neither of those, default to uncompressed
  } else if (!strcmp(c,"bind_image")) {
    bind_image_ = (int)atol(f.read_word());
  } else if (!strcmp(c,"compress_image")) {
    compress_image_ = (int)atol(f.read_word());
  } else if (!strcmp(c,"scale_deimage")) {
    if (sscanf(f.read_word(),"%d %d",&w,&h) == 2) {
      scale_deimage_w_ = w;
      scale_deimage_h_ = h;
    }
  } else if (!strcmp(c,"deimage")) {
    inactive_name(f.read_word());
    // starting in 2023, `deimage` is always followed by `compress_deimage`
    // the code below is for compatibility with older .fl files
    const char *ext = fl_filename_ext(inactive_name_);
    if (   strcmp(ext, ".jpg")
        && strcmp(ext, ".png")
        && strcmp(ext, ".svg")
        && strcmp(ext, ".svgz"))
      compress_deimage_ = 0; // if it is neither of those, default to uncompressed
  } else if (!strcmp(c,"bind_deimage")) {
    bind_deimage_ = (int)atol(f.read_word());
  } else if (!strcmp(c,"compress_deimage")) {
    compress_deimage_ = (int)atol(f.read_word());
  } else if (!strcmp(c,"type")) {
    if (is_a(Type::Spinner))
      ((Fl_Spinner*)o)->type(item_number(subtypes(), f.read_word()));
    else
      o->type(item_number(subtypes(), f.read_word()));
  } else if (!strcmp(c,"box")) {
    const char* value = f.read_word();
    if ((x = boxnumber(value))) {
      if (x == ZERO_ENTRY) x = 0;
      o->box((Fl_Boxtype)x);
    } else if (sscanf(value,"%d",&x) == 1) o->box((Fl_Boxtype)x);
  } else if (is_a(Type::Button) && !strcmp(c,"down_box")) {
    const char* value = f.read_word();
    if ((x = boxnumber(value))) {
      if (x == ZERO_ENTRY) x = 0;
      ((Fl_Button*)o)->down_box((Fl_Boxtype)x);
    }
  } else if (is_a(Type::Input_Choice) && !strcmp(c,"down_box")) {
    const char* value = f.read_word();
    if ((x = boxnumber(value))) {
      if (x == ZERO_ENTRY) x = 0;
      ((Fl_Input_Choice*)o)->down_box((Fl_Boxtype)x);
    }
  } else if (is_a(Type::Menu_) && !strcmp(c,"down_box")) {
    const char* value = f.read_word();
    if ((x = boxnumber(value))) {
      if (x == ZERO_ENTRY) x = 0;
      ((Fl_Menu_*)o)->down_box((Fl_Boxtype)x);
    }
  } else if (is_button() && !strcmp(c,"value")) {
    const char* value = f.read_word();
    ((Fl_Button*)o)->value(atoi(value));
  } else if (!strcmp(c,"color")) {
    const char *cw = f.read_word();
    if (cw[0]=='0' && cw[1]=='x') {
      sscanf(cw,"0x%x",&x);
      o->color(x);
    } else {
      int n = sscanf(cw,"%d %d",&x,&y);
      if (n == 2) { // back compatibility...
        if (x != 47) o->color(x);
        o->selection_color(y);
      } else {
        o->color(x);
      }
    }
  } else if (!strcmp(c,"selection_color")) {
    if (sscanf(f.read_word(),"%d",&x)) o->selection_color(x);
  } else if (!strcmp(c,"labeltype")) {
    c = f.read_word();
    if (!strcmp(c,"image")) {
      Image_Asset *i = Image_Asset::find(label());
      if (!i) f.read_error("Image file '%s' not found", label());
      else setimage(i);
      image_name(label());
      label("");
    } else {
      o->labeltype((Fl_Labeltype)item_number(labeltypemenu,c));
    }
  } else if (!strcmp(c,"labelfont")) {
    if (sscanf(f.read_word(),"%d",&x) == 1) o->labelfont(x);
  } else if (!strcmp(c,"labelsize")) {
    if (sscanf(f.read_word(),"%d",&x) == 1) o->labelsize(x);
  } else if (!strcmp(c,"labelcolor")) {
    if (sscanf(f.read_word(),"%d",&x) == 1) o->labelcolor(x);
  } else if (!strcmp(c,"align")) {
    if (sscanf(f.read_word(),"%d",&x) == 1) o->align(x);
  } else if (!strcmp(c,"h_label_margin")) {
    if (sscanf(f.read_word(),"%d",&x) == 1) o->horizontal_label_margin(x);
  } else if (!strcmp(c,"v_label_margin")) {
    if (sscanf(f.read_word(),"%d",&x) == 1) o->vertical_label_margin(x);
  } else if (!strcmp(c,"image_spacing")) {
    if (sscanf(f.read_word(),"%d",&x) == 1) o->label_image_spacing(x);
  } else if (!strcmp(c,"when")) {
    if (sscanf(f.read_word(),"%d",&x) == 1) o->when(x);
  } else if (!strcmp(c,"minimum")) {
    if (is_a(Type::Valuator_)) ((Fl_Valuator*)o)->minimum(strtod(f.read_word(),nullptr));
    if (is_a(Type::Spinner)) ((Fl_Spinner*)o)->minimum(strtod(f.read_word(),nullptr));
  } else if (!strcmp(c,"maximum")) {
    if (is_a(Type::Valuator_)) ((Fl_Valuator*)o)->maximum(strtod(f.read_word(),nullptr));
    if (is_a(Type::Spinner)) ((Fl_Spinner*)o)->maximum(strtod(f.read_word(),nullptr));
  } else if (!strcmp(c,"step")) {
    if (is_a(Type::Valuator_)) ((Fl_Valuator*)o)->step(strtod(f.read_word(),nullptr));
    if (is_a(Type::Spinner)) ((Fl_Spinner*)o)->step(strtod(f.read_word(),nullptr));
  } else if (!strcmp(c,"value")) {
    if (is_a(Type::Valuator_)) ((Fl_Valuator*)o)->value(strtod(f.read_word(),nullptr));
    if (is_a(Type::Spinner)) ((Fl_Spinner*)o)->value(strtod(f.read_word(),nullptr));
  } else if ( (!strcmp(c,"slider_size") || !strcmp(c,"size")) && is_a(Type::Slider)) {
    ((Fl_Slider*)o)->slider_size(strtod(f.read_word(),nullptr));
  } else if (!strcmp(c,"textfont")) {
    if (sscanf(f.read_word(),"%d",&x) == 1) {ft=(Fl_Font)x; textstuff(1,ft,s,cc);}
  } else if (!strcmp(c,"textsize")) {
    if (sscanf(f.read_word(),"%d",&x) == 1) {s=x; textstuff(2,ft,s,cc);}
  } else if (!strcmp(c,"textcolor")) {
    if (sscanf(f.read_word(),"%d",&x) == 1) {cc=(Fl_Color)x;textstuff(3,ft,s,cc);}
  } else if (!strcmp(c,"hide")) {
    o->hide();
  } else if (!strcmp(c,"deactivate")) {
    o->deactivate();
  } else if (!strcmp(c,"resizable")) {
    resizable(1);
  } else if (!strcmp(c,"hotspot") || !strcmp(c, "divider")) {
    hotspot(1);
  } else if (!strcmp(c,"class")) {
    subclass(f.read_word());
  } else if (!strcmp(c,"shortcut")) {
    int shortcut = (int)strtol(f.read_word(),nullptr,0);
    if (is_button()) ((Fl_Button*)o)->shortcut(shortcut);
    else if (is_a(Type::Input)) ((Fl_Input_*)o)->shortcut(shortcut);
    else if (is_a(Type::Value_Input)) ((Fl_Value_Input*)o)->shortcut(shortcut);
    else if (is_a(Type::Text_Display)) ((Fl_Text_Display*)o)->shortcut(shortcut);
  } else {
    if (!strncmp(c,"code",4)) {
      int n = atoi(c+4);
      if (n >= 0 && n <= NUM_EXTRA_CODE) {
        extra_code(n,f.read_word());
        return;
      }
    } else if (!strcmp(c,"extra_code")) {
      extra_code(0,f.read_word());
      return;
    }
    Node::read_property(f, c);
  }
}

Fl_Menu_Item boxmenu1[] = {
  // these extra ones are for looking up fdesign saved strings:
  {"NO_FRAME",          0,nullptr,(void *)FL_NO_BOX},
  {"ROUNDED3D_UPBOX",   0,nullptr,(void *)_FL_ROUND_UP_BOX},
  {"ROUNDED3D_DOWNBOX", 0,nullptr,(void *)_FL_ROUND_DOWN_BOX},
  {"OVAL3D_UPBOX",      0,nullptr,(void *)_FL_ROUND_UP_BOX},
  {"OVAL3D_DOWNBOX",    0,nullptr,(void *)_FL_ROUND_DOWN_BOX},
  {"0",                 0,nullptr,(void *)ZERO_ENTRY},
  {"1",                 0,nullptr,(void *)FL_UP_BOX},
  {"2",                 0,nullptr,(void *)FL_DOWN_BOX},
  {"3",                 0,nullptr,(void *)FL_FLAT_BOX},
  {"4",                 0,nullptr,(void *)FL_BORDER_BOX},
  {"5",                 0,nullptr,(void *)FL_SHADOW_BOX},
  {"6",                 0,nullptr,(void *)FL_FRAME_BOX},
  {"7",                 0,nullptr,(void *)FL_ROUNDED_BOX},
  {"8",                 0,nullptr,(void *)FL_RFLAT_BOX},
  {"9",                 0,nullptr,(void *)FL_RSHADOW_BOX},
  {"10",                0,nullptr,(void *)FL_UP_FRAME},
  {"11",                0,nullptr,(void *)FL_DOWN_FRAME},
  {nullptr}};

int lookup_symbol(const char *, int &, int numberok = 0);

int Widget_Node::read_fdesign(const char* propname, const char* value) {
  int v;
  if (!strcmp(propname,"box")) {
    float x,y,w,h;
    if (sscanf(value,"%f %f %f %f",&x,&y,&w,&h) == 4) {
      if (fld::io::fdesign_flip) {
        Node *p;
        for (p = parent; p && !p->is_a(Type::Window); p = p->parent) {/*empty*/}
        if (p && p->is_widget()) y = ((Widget_Node*)p)->o->h()-(y+h);
      }
      x += Fluid.pasteoffset;
      y += Fluid.pasteoffset;
      o->resize(int(x),int(y),int(w),int(h));
    }
  } else if (!strcmp(propname,"label")) {
    label(value);
  } else if (!strcmp(propname,"name")) {
    this->name(value);
  } else if (!strcmp(propname,"callback")) {
    callback(value); user_data_type("long");
  } else if (!strcmp(propname,"argument")) {
    user_data(value);
  } else if (!strcmp(propname,"shortcut")) {
    if (value[0]) {
      char buf[128]; sprintf(buf,"o->shortcut(\"%s\");",value);
      extra_code(0,buf);
    }
  } else if (!strcmp(propname,"style")) {
    if (!strncmp(value,"FL_NORMAL",9)) return 1;
    if (!lookup_symbol(value,v,1)) return 0;
    o->labelfont(v); o->labeltype((Fl_Labeltype)(v>>8));
  } else if (!strcmp(propname,"size")) {
    if (!lookup_symbol(value,v,1)) return 0;
    o->labelsize(v);
  } else if (!strcmp(propname,"type")) {
    if (!strncmp(value,"NORMAL",6)) return 1;
    if (lookup_symbol(value,v,1)) {o->type(v); return 1;}
    if (!strcmp(value+strlen(value)-5,"FRAME")) goto TRY_BOXTYPE;
    if (!strcmp(value+strlen(value)-3,"BOX")) goto TRY_BOXTYPE;
    return 0;
  } else if (!strcmp(propname,"lcol")) {
    if (!lookup_symbol(value,v,1)) return 0;
    o->labelcolor(v);
  } else if (!strcmp(propname,"return")) {
    if (!lookup_symbol(value,v,0)) return 0;
    o->when(v|FL_WHEN_RELEASE);
  } else if (!strcmp(propname,"alignment")) {
    if (!lookup_symbol(value,v)) {
      // convert old numeric values:
      int v1 = atoi(value); if (v1 <= 0 && strcmp(value,"0")) return 0;
      v = 0;
      if (v1 >= 5) {v = FL_ALIGN_INSIDE; v1 -= 5;}
      switch (v1) {
      case 0: v += FL_ALIGN_TOP; break;
      case 1: v += FL_ALIGN_BOTTOM; break;
      case 2: v += FL_ALIGN_LEFT; break;
      case 3: v += FL_ALIGN_RIGHT; break;
      case 4: v += FL_ALIGN_CENTER; break;
      default: return 0;
      }
    }
    o->align(v);
  } else if (!strcmp(propname,"resizebox")) {
    resizable(1);
  } else if (!strcmp(propname,"colors")) {
    char* p = (char*)value;
    while (*p != ' ') {if (!*p) return 0; p++;}
    *p = 0;
    int v1;
    if (!lookup_symbol(value,v,1) || !lookup_symbol(p+1,v1,1)) {
      *p=' '; return 0;}
    o->color(v,v1);
  } else if (!strcmp(propname,"resize")) {
    return !strcmp(value,"FL_RESIZE_ALL");
  } else if (!strcmp(propname,"gravity")) {
    return !strcmp(value,"FL_NoGravity FL_NoGravity");
  } else if (!strcmp(propname,"boxtype")) {
  TRY_BOXTYPE:
    int x = boxnumber(value);
    if (!x) {x = item_number(boxmenu1, value); if (x < 0) return 0;}
    if (x == ZERO_ENTRY) {
      x = 0;
      if (o->box() != ((Widget_Node*)factory)->o->box()) return 1; // kludge for frame
    }
    o->box((Fl_Boxtype)x);
  } else {
    return 0;
  }
  return 1;
}

void leave_live_mode_cb(Fl_Widget*, void*) {
  live_mode_cb(nullptr, nullptr);
}

Fl_Widget *Widget_Node::enter_live_mode(int) {
  live_widget = widget(o->x(), o->y(), o->w(), o->h());
  if (live_widget)
    copy_properties();
  return live_widget;
}

Fl_Widget* Widget_Node::propagate_live_mode(Fl_Group* grp) {
  live_widget = grp;
  copy_properties();
  Node *n;
  for (n = next; n && n->level > level; n = n->next) {
    if (n->level == level+1) {
      Fl_Widget* proxy_child = n->enter_live_mode();
      if (proxy_child && n->is_widget() && ((Widget_Node*)n)->resizable()) {
        grp->resizable(proxy_child);
      }
    }
  }
  grp->end();
  live_widget = grp;
  copy_properties_for_children();
  return live_widget;
}


void Widget_Node::leave_live_mode() {
}

/**
 copy all properties from the edit widget to the live widget
 */
void Widget_Node::copy_properties() {
  if (!live_widget)
    return;

  Fl_Font ff = 0;
  int fs = 0;
  Fl_Color fc = 0;
  textstuff(0, ff, fs, fc);

  // copy all attributes common to all widget types
  Fl_Widget *w = live_widget;
  w->label(o->label());
  w->tooltip(tooltip());
  w->type(o->type());
  w->box(o->box());
  w->color(o->color());
  w->selection_color(o->selection_color());
  w->labeltype(o->labeltype());
  w->labelfont(o->labelfont());
  w->labelsize(o->labelsize());
  w->labelcolor(o->labelcolor());
  w->align(o->align());
  w->when(o->when());

  // copy all attributes specific to widgets derived from Fl_Button
  if (is_button()) {
    Fl_Button* d = (Fl_Button*)live_widget, *s = (Fl_Button*)o;
    d->down_box(s->down_box());
    d->shortcut(s->shortcut());
    d->value(s->value());
  }

  // copy all attributes specific to widgets derived from Fl_Input_
  if (is_a(Type::Input)) {
    Fl_Input_* d = (Fl_Input_*)live_widget, *s = (Fl_Input_*)o;
    d->shortcut(s->shortcut());
    d->textfont(ff);
    d->textsize(fs);
    d->textcolor(fc);
  }

  // copy all attributes specific to widgets derived from Fl_Value_Input
  if (is_a(Type::Value_Input)) {
    Fl_Value_Input* d = (Fl_Value_Input*)live_widget, *s = (Fl_Value_Input*)o;
    d->shortcut(s->shortcut());
    d->textfont(ff);
    d->textsize(fs);
    d->textcolor(fc);
  }

  // copy all attributes specific to widgets derived from Fl_Text_Display
  if (is_a(Type::Text_Display)) {
    Fl_Text_Display* d = (Fl_Text_Display*)live_widget, *s = (Fl_Text_Display*)o;
    d->shortcut(s->shortcut());
    d->textfont(ff);
    d->textsize(fs);
    d->textcolor(fc);
  }

  // copy all attributes specific to Fl_Valuator and derived classes
  if (is_a(Type::Valuator_)) {
    Fl_Valuator* d = (Fl_Valuator*)live_widget, *s = (Fl_Valuator*)o;
    d->minimum(s->minimum());
    d->maximum(s->maximum());
    d->step(s->step());
    d->value(s->value());
    if (is_a(Type::Slider)) {
      Fl_Slider *d = (Fl_Slider*)live_widget, *s = (Fl_Slider*)o;
      d->slider_size(s->slider_size());
    }
  }

  // copy all attributes specific to Fl_Spinner and derived classes
  if (is_a(Type::Spinner)) {
    Fl_Spinner* d = (Fl_Spinner*)live_widget, *s = (Fl_Spinner*)o;
    d->minimum(s->minimum());
    d->maximum(s->maximum());
    d->step(s->step());
    d->value(s->value());
  }

  if (!o->visible())
    w->hide();
  if (!o->active())
    w->deactivate();
}

