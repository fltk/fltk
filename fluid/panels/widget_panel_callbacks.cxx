//
// Widget Panel callbacks for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2026 by Bill Spitzak and others.
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

// This file holds the free functions and globals that back the widget
// properties panel (panels/widget_panel.fl). They are not members of
// Widget_Node -- they live here (rather than in nodes/Widget_Node.cxx)
// because they are panel UI glue, not part of the Node/Widget_Node
// serialization or code-generation interface.

#include "nodes/Widget_Node.h"

#include "Fluid.h"
#include "Project.h"
#include "proj/undo.h"
#include "nodes/Menu_Node.h"
#include "nodes/Function_Node.h"
#include "nodes/Window_Node.h"
#include "nodes/Grid_Node.h"
#include "panels/widget_panel.h"
#include "widgets/Formula_Input.h"

#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Flex.H>
#include <FL/fl_message.H>

#include <algorithm>

extern void redraw_browser();

////////////////////////////////////////////////////////////////
// The control panels!

Fl_Window* the_panel = nullptr;

// All the callbacks use the argument to indicate whether to load or store.
// This avoids the need for pointers to all the widgets, and keeps the
// code localized in the callbacks.
// A value of LOAD means to load.  The hope is that this will not collide
// with any actual useful values for the argument.  I also use this to
// initialized parts of the widget that are nyi by fluid.

Node* current_node = nullptr;
Widget_Node* current_widget = nullptr; // one of the selected ones
void* const LOAD = (void *)"LOAD"; // "magic" pointer to indicate that we need to load values into the dialog
int numselected = 0; // number selected
int haderror = 0;

void name_public_cb(Fl_Choice* i, void* v) {
  if (v == LOAD) {
    i->value(current_widget->public_>0);
    if (current_widget->is_in_class())
      i->hide();
    else
      i->show();
  } else {
    int mod = 0;
    for (Node* o = Fluid.proj.tree.first; o; o = o->next) {
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
void label_cb(Fl_Input* i, void* v) {
  static int first_change = 1;
  if (v == LOAD) {
    i->value(current_widget->label());
    first_change = 1;
  } else {
    if (i->changed()) {
      Fluid.proj.undo.suspend();
      int mod = 0;
      for (Node* o = Fluid.proj.tree.first; o; o = o->next) {
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
      if (mod)
        Fluid.proj.set_modflag(1);
    }
    int r = (int)Fl::callback_reason();
    if ( (r == FL_REASON_LOST_FOCUS) || (r == FL_REASON_ENTER_KEY) )
      first_change = 1;
  }
}

// ---- Formula_Input widget callbacks:

int widget_i = 0;

static int vars_i_cb(const fluid::widget::Formula_Input*, void* v) {
  return widget_i;
}

static int vars_x_cb(const fluid::widget::Formula_Input*, void* v) {
  Node* t = (Node*)v;
  if (t->is_widget())
    return ((Widget_Node*)t)->o->x();
  return 0;
}

static int vars_y_cb(const fluid::widget::Formula_Input*, void* v) {
  Node* t = (Node*)v;
  if (t->is_widget())
    return ((Widget_Node*)t)->o->y();
  return 0;
}

static int vars_w_cb(const fluid::widget::Formula_Input*, void* v) {
  Node* t = (Node*)v;
  if (t->is_widget())
    return ((Widget_Node*)t)->o->w();
  return 0;
}

static int vars_h_cb(const fluid::widget::Formula_Input*, void* v) {
  Node* t = (Node*)v;
  if (t->is_widget())
    return ((Widget_Node*)t)->o->h();
  return 0;
}

static int vars_px_cb(const fluid::widget::Formula_Input*, void* v) {
  Node* t = ((Node*)v)->parent;
  if (t && t->is_widget())
    return ((Widget_Node*)t)->o->x();
  return 0;
}

static int vars_py_cb(const fluid::widget::Formula_Input*, void* v) {
  Node* t = ((Node*)v)->parent;
  if (t && t->is_widget())
    return ((Widget_Node*)t)->o->y();
  return 0;
}

static int vars_pw_cb(const fluid::widget::Formula_Input*, void* v) {
  Node* t = ((Node*)v)->parent;
  if (t && t->is_widget())
    return ((Widget_Node*)t)->o->w();
  return 0;
}

static int vars_ph_cb(const fluid::widget::Formula_Input*, void* v) {
  Node* t = ((Node*)v)->parent;
  if (t && t->is_widget())
    return ((Widget_Node*)t)->o->h();
  return 0;
}

static int vars_sx_cb(const fluid::widget::Formula_Input*, void* v) {
  Node* t = ((Node*)v)->prev_sibling();
  if (t && t->is_widget())
    return ((Widget_Node*)t)->o->x();
  return 0;
}

static int vars_sy_cb(const fluid::widget::Formula_Input*, void* v) {
  Node* t = ((Node*)v)->prev_sibling();
  if (t && t->is_widget())
    return ((Widget_Node*)t)->o->y();
  return 0;
}

static int vars_sw_cb(const fluid::widget::Formula_Input*, void* v) {
  Node* t = ((Node*)v)->prev_sibling();
  if (t && t->is_widget())
    return ((Widget_Node*)t)->o->w();
  return 0;
}

static int vars_sh_cb(const fluid::widget::Formula_Input*, void* v) {
  Node* t = ((Node*)v)->prev_sibling();
  if (t && t->is_widget())
    return ((Widget_Node*)t)->o->h();
  return 0;
}

static int bbox_x, bbox_y, bbox_r, bbox_b;

static void calculate_bbox(Node* p) {
  char first = 1;
  bbox_x = bbox_y = bbox_r = bbox_b = 0;
  for (p=p->first_child(); p; p=p->next_sibling()) {
    if (p->is_widget()) {
      Fl_Widget* o = ((Widget_Node*)p)->o;
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

static int vars_cx_cb(const fluid::widget::Formula_Input*, void* v) {
  calculate_bbox((Node*)v);
  return bbox_x;
}

static int vars_cy_cb(const fluid::widget::Formula_Input*, void* v) {
  calculate_bbox((Node*)v);
  return bbox_y;
}

static int vars_cw_cb(const fluid::widget::Formula_Input*, void* v) {
  calculate_bbox((Node*)v);
  return bbox_r - bbox_x;
}

static int vars_ch_cb(const fluid::widget::Formula_Input*, void* v) {
  calculate_bbox((Node*)v);
  return bbox_b - bbox_y;
}

fluid::widget::Formula_Input_Vars widget_vars[] = {
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

// whenmenu is defined (non-static) in nodes/Widget_Node.cxx, where it is
// also used by Widget_Node::write_widget_code() via when_symbol_name().
extern Fl_Menu_Item whenmenu[];

// Set the check marks in the "when()" menu according to the Fl_When value n
void set_whenmenu(int n) {
  if (n&FL_WHEN_CHANGED)      whenmenu[0].set(); else whenmenu[0].clear();
  if (n&FL_WHEN_NOT_CHANGED)  whenmenu[1].set(); else whenmenu[1].clear();
  if (n&FL_WHEN_RELEASE)      whenmenu[2].set(); else whenmenu[2].clear();
  if (n&FL_WHEN_ENTER_KEY)    whenmenu[3].set(); else whenmenu[3].clear();
  if (n&FL_WHEN_CLOSED)       whenmenu[4].set(); else whenmenu[4].clear();
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

// labeltypemenu is defined (non-static) in nodes/Widget_Node.cxx, where it
// is also used by Widget_Node::write_widget_code/write_properties/
// read_property for serialization. The array bound must be repeated here
// (matching the real definition) so that sizeof(labeltypemenu) below works.
extern Fl_Menu_Item labeltypemenu[6];

void labeltype_cb(Fl_Choice* i, void* v) {
  if (v == LOAD) {
    int n = current_widget->o->labeltype();
    i->when(FL_WHEN_RELEASE);
    for (int j = 0; j < int(sizeof(labeltypemenu)/sizeof(*labeltypemenu)); j++) {
      if (labeltypemenu[j].argument() == n) {
        i->value(j);
        break;
      }
    }
  } else {
    int mod = 0;
    int m = i->value();
    int n = int(labeltypemenu[m].argument());
    if (n<0)
      return; // should not happen
    for (Node* o = Fluid.proj.tree.first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
        Widget_Node* p = (Widget_Node*)o;
        p->o->labeltype((Fl_Labeltype)n);
        p->redraw();
        mod = 1;
      }
    }
    if (mod)
      Fluid.proj.set_modflag(1);
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
  for (Node* o = Fluid.proj.tree.first; o; o = o->next) {
    if (o->selected && o->is_widget()) {
      Widget_Node* q = (Widget_Node*)o;
      q->o->color(c); q->o->redraw();
      if (q->parent && dynamic_cast<Tabs_Node*>(q->parent)) {
        if (q->o->parent()) {
          q->o->parent()->redraw();
        }
      }
      mod = 1;
    }
  }
  if (mod)
    Fluid.proj.set_modflag(1);
}



void color2_common(Fl_Color c) {
  int mod = 0;
  for (Node* o = Fluid.proj.tree.first; o; o = o->next) {
    if (o->selected && o->is_widget()) {
      Widget_Node* q = (Widget_Node*)o;
      q->o->selection_color(c); q->o->redraw();
      mod = 1;
    }
  }
  if (mod)
    Fluid.proj.set_modflag(1);
}



void labelcolor_common(Fl_Color c) {
  int mod = 0;
  for (Node* o = Fluid.proj.tree.first; o; o = o->next) {
    if (o->selected && o->is_widget()) {
      Widget_Node* q = (Widget_Node*)o;
      q->o->labelcolor(c); q->redraw();
      mod = 1;
    }
  }
  if (mod)
    Fluid.proj.set_modflag(1);
}


static Fl_Button* relative(Fl_Widget* o, int i) {
  Fl_Group* g = (Fl_Group*)(o->parent());
  return (Fl_Button*)(g->child(g->find(*o)+i));
}

// alignmenu is defined (non-static) in nodes/Widget_Node.cxx, where it is
// also used by Widget_Node::write_widget_code() to serialize align() values.
extern Fl_Menu_Item alignmenu[];

void align_cb(Fl_Button* i, void* v) {
  Fl_Align b = Fl_Align(fl_uintptr_t(i->user_data()));
  if (v == LOAD) {
    if (dynamic_cast<Menu_Item_Node*>(current_widget)) {
      i->deactivate();
      return;
    } else {
      i->activate();
    }
    i->value(current_widget->o->align() & b);
  } else {
    int mod = 0;
    Fluid.proj.undo.checkpoint();
    for (Node* o = Fluid.proj.tree.first; o; o = o->next) {
      if (o->selected && o->is_widget()) {
        Widget_Node* q = (Widget_Node*)o;
        Fl_Align x = q->o->align();
        Fl_Align y;
        if (i->value()) {
          y = x | b;
          if (b == FL_ALIGN_LEFT || b == FL_ALIGN_TOP) {
            Fl_Button* b1 = relative(i,+1);
            b1->clear();
            y = y & ~(b1->argument());
          }
          if (b == FL_ALIGN_RIGHT || b == FL_ALIGN_BOTTOM) {
            Fl_Button* b1 = relative(i,-1);
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

void align_position_cb(Fl_Choice* i, void* v) {
  if (v == LOAD) {
    if (dynamic_cast<Menu_Item_Node*>(current_widget)) {
      i->deactivate();
      return;
    } else {
      i->activate();
    }
    Fl_Menu_Item* mi = (Fl_Menu_Item*)i->menu();
    Fl_Align b = current_widget->o->align() & FL_ALIGN_POSITION_MASK;
    for (;mi->text;mi++) {
      if ((Fl_Align)(mi->argument())==b)
        i->value(mi);
    }
  } else {
    const Fl_Menu_Item* mi = i->menu() + i->value();
    Fl_Align b = Fl_Align(fl_uintptr_t(mi->user_data()));
    int mod = 0;
    Fluid.proj.undo.checkpoint();
    for (Node* o = Fluid.proj.tree.first; o; o = o->next) {
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

void align_text_image_cb(Fl_Choice* i, void* v) {
  if (v == LOAD) {
    if (dynamic_cast<Menu_Item_Node*>(current_widget)) {
      i->deactivate();
      return;
    } else {
      i->activate();
    }
    Fl_Menu_Item* mi = (Fl_Menu_Item*)i->menu();
    Fl_Align b = current_widget->o->align() & FL_ALIGN_IMAGE_MASK;
    for (;mi->text;mi++) {
      if ((Fl_Align)(mi->argument())==b)
        i->value(mi);
    }
  } else {
    const Fl_Menu_Item* mi = i->menu() + i->value();
    Fl_Align b = Fl_Align(fl_uintptr_t(mi->user_data()));
    int mod = 0;
    Fluid.proj.undo.checkpoint();
    for (Node* o = Fluid.proj.tree.first; o; o = o->next) {
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

void textcolor_common(Fl_Color c) {
  Fl_Font n; int s;
  int mod = 0;
  for (Node* o = Fluid.proj.tree.first; o; o = o->next) {
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

static void load_gap(Fl_Flex* w, Fl_Value_Input* i)
{
  int v = w->gap();
  i->value((double)v);
}

static int update_gap(Fl_Flex* w, int new_value)
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

void toggle_overlays(Fl_Widget*, void*); // in Window_Node.cxx

void overlay_cb(Fl_Button* o,void* v) {
  toggle_overlays(o,v);
}

void leave_live_mode_cb(Fl_Widget*, void*);

void live_mode_cb(Fl_Button* o, void *) {
  /// \todo live mode should end gracefully when the application quits
  ///       or when the user closes the live widget
  static Node* live_type = nullptr;
  static Fl_Widget* live_widget = nullptr;
  static Fl_Window* live_window = nullptr;

  if (!current_widget) {
    wLiveMode->value(0);
    return;
  }

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
        Fl_Group* rsz = new Fl_Group(0, h+20, 130, 35);
        rsz->box(FL_NO_BOX);
        Fl_Box* rsz_dummy = new Fl_Box(110, h+20, 1, 25);
        rsz_dummy->box(FL_NO_BOX);
        rsz->resizable(rsz_dummy);
        Fl_Button* btn = new Fl_Button(10, h+20, 100, 25, "Exit Live Resize");
        btn->labelsize(12);
        btn->callback(leave_live_mode_cb);
        rsz->end();
        live_window->add(live_widget);
        live_widget->position(10, 10);
        live_window->resizable(live_widget);
        live_window->set_modal(); // block all other UI
        live_window->callback(leave_live_mode_cb);
        if (dynamic_cast<Window_Node*>(current_widget)) {
          Window_Node* w = (Window_Node*)current_widget;
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
    if (dynamic_cast<Data_Node*>(Fluid.proj.tree.current)) {
      current_node = Fluid.proj.tree.current;
      tabs_wizard->value(data_tabs);
      numselected = 1;
    } else if (dynamic_cast<Comment_Node*>(Fluid.proj.tree.current)) {
      current_node = Fluid.proj.tree.current;
      tabs_wizard->value(comment_tabs);
      numselected = 1;
    } else if (dynamic_cast<Class_Node*>(Fluid.proj.tree.current)) {
      current_node = Fluid.proj.tree.current;
      tabs_wizard->value(class_tabs);
      numselected = 1;
    } else if (dynamic_cast<DeclBlock_Node*>(Fluid.proj.tree.current)) {
      current_node = Fluid.proj.tree.current;
      tabs_wizard->value(declblock_tabs);
      numselected = 1;
    } else if (dynamic_cast<Decl_Node*>(Fluid.proj.tree.current)) {
      current_node = Fluid.proj.tree.current;
      tabs_wizard->value(decl_tabs);
      numselected = 1;
    } else if (dynamic_cast<CodeBlock_Node*>(Fluid.proj.tree.current)) {
      current_node = Fluid.proj.tree.current;
      tabs_wizard->value(codeblock_tabs);
      numselected = 1;
    } else if (dynamic_cast<Code_Node*>(Fluid.proj.tree.current)) {
      current_node = Fluid.proj.tree.current;
      tabs_wizard->value(code_tabs);
      numselected = 1;
    } else if (dynamic_cast<Function_Node*>(Fluid.proj.tree.current)) {
      current_node = Fluid.proj.tree.current;
      tabs_wizard->value(func_tabs);
      numselected = 1;
    } else {
      current_node = nullptr;
      if (Fluid.proj.tree.current->is_widget())
        current_widget=(Widget_Node*)Fluid.proj.tree.current;
      for (Node* o = Fluid.proj.tree.first; o; o = o->next) {
        if (o->is_widget() && o->selected) {
          numselected++;
          if (!current_widget) current_widget = (Widget_Node*)o;
        }
      }
    }
  }
  if (current_widget) {
    tabs_wizard->value(widget_tabs);
    if (current_widget && dynamic_cast<Grid_Node*>(current_widget)) {
      if (widget_tab_grid->parent()!=widget_tabs)
        widget_tabs->add(widget_tab_grid);
    } else {
      if (widget_tab_grid->parent()==widget_tabs) {
        widget_tabs_repo->add(widget_tab_grid);
      }
    }
    if (current_widget && current_widget->parent && dynamic_cast<Grid_Node*>(current_widget->parent)) {
      if (widget_tab_grid_child->parent()!=widget_tabs)
        widget_tabs->add(widget_tab_grid_child);
    } else {
      if (widget_tab_grid_child->parent()==widget_tabs) {
        widget_tabs_repo->add(widget_tab_grid_child);
      }
    }
  }
  if (numselected)
    propagate_load(the_panel, LOAD);
  else
    the_panel->hide();
}

extern Fl_Window* widgetbin_panel;

// This is called when user double-clicks an item, open or update the panel:
void open_panel() {
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
extern void update_codeview_position();

// Called when ui changes what objects are selected:
// p is selected object, null for all deletions (we must throw away
// old panel in that case, as the object may no longer exist)
void selection_changed(Node* p) {
  // store all changes to the current selected objects:
  if (p && the_panel && the_panel->visible()) {
    set_cb(nullptr,nullptr);
    // if there was an error, we try to leave the selected set unchanged:
    if (haderror) {
      Node* q = nullptr;
      for (Node* o = Fluid.proj.tree.first; o; o = o->next) {
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
  Node* q = nullptr;
  for (Node* o = Fluid.proj.tree.first; o; o = o->next) {
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

void leave_live_mode_cb(Fl_Widget*, void*) {
  live_mode_cb(nullptr, nullptr);
}
