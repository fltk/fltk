//
// Widget Node code for the Fast Light Tool Kit (FLTK).
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

#include "nodes/Widget_Node.h"

#include "Fluid.h"
#include "Project.h"
#include "proj/Image_Asset.h"
#include "proj/mergeback.h"
#include "proj/undo.h"
#include "io/Project_Reader.h"
#include "io/Project_Writer.h"
#include "io/Code_Writer.h"
#include "nodes/Menu_Node.h"
#include "nodes/Function_Node.h"
#include "nodes/Window_Node.h"
#include "nodes/Grid_Node.h"
#include "nodes/factory.h"
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
#include "../../src/flstring.h"

#include <stdio.h>
#include <stdlib.h>
#undef min
#undef max
#include <algorithm>

using namespace fluid;
using namespace fluid::proj;

extern void redraw_browser();

// Make an Widget_Node subclass instance.
// It figures out the automatic size and parent of the new widget,
// creates the Fl_Widget (by calling the virtual function _make),
// adds it to the Fl_Widget hierarchy, creates a new Node
// instance, sets the widget pointers, and makes all the display
// update correctly...

int Widget_Node::is_widget() const {
  return 1;
}

int Widget_Node::is_public() const {
  return public_;
}

std::string subclassname(Node* l) {
  if (dynamic_cast<Menu_Bar_Node*>(l)) {
    Menu_Bar_Node* mb = static_cast<Menu_Bar_Node*>(l);
    if (mb->is_sys_menu_bar())
      return mb->sys_menubar_name();
  }
  if (l->is_widget()) {
    Widget_Node* p = (Widget_Node*)l;
    std::string c = p->subclass();
    if (!c.empty())
      return c;
    if (l->is_class())
      return "Fl_Group";
    if (p->o->type() == FL_DOUBLE_WINDOW)
      return "Fl_Double_Window";
    if (typeid(*p) == typeid(Input_Node)) {
      if (p->o->type() == FL_FLOAT_INPUT)
        return "Fl_Float_Input";
      if (p->o->type() == FL_INT_INPUT)
        return "Fl_Int_Input";
    }
  }
  return l->type_name();
}

// Return the ideal widget size...
void Widget_Node::ideal_size(int &w, int &h) {
  w = 120;
  h = 100;
  fluid::app::Snap_Action::better_size(w, h);
}

/**
 Make a new Widget node.
 \param[in] strategy is Strategy::AS_LAST_CHILD or Strategy::AFTER_CURRENT
 \return new node
 */
Node* Widget_Node::make(Strategy strategy) {
  Node* anchor = Fluid.proj.tree.current, *pp = anchor;
  if (pp && (strategy.placement() == Strategy::AFTER_CURRENT))
    pp = pp->parent;
  while (pp && !dynamic_cast<Group_Node*>(pp)) {
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
  int B = p->o->w()/2;
  if (p->o->h()/2 < B)
    B = p->o->h()/2;
  if (B>25)
    B = 25;

  int ULX,ULY; // parent's origin in window
  if (!dynamic_cast<Window_Node*>(p)) { // if it is a group, add corner
    ULX = p->o->x(); ULY = p->o->y();
  } else {
    ULX = ULY = 0;
  }

  // Figure out a position and size for the widget
  int X,Y,W,H;
  if (dynamic_cast<Group_Node*>(this)) {     // fill the parent with the widget
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
  Widget_Node* t = _make();
  if (!o)
    o = widget(0,0,100,100); // create template widget
  t->factory = this;

  // Construct the Fl_Widget:
  t->o = widget(X,Y,W,H);
  if (strategy.source() == Strategy::FROM_FILE)
    t->o->label(nullptr);
  else if (t->o->label())
    t->label(t->o->label()); // allow editing
  t->o->user_data((void*)t);

  // Put it in the parent:
  //  ((Fl_Group *)(p->o))->add(t->o); (done by Node::add())
  // add to browser:
  t->add(anchor, strategy);
  t->redraw();
  return t;
}

void Widget_Node::setlabel(const char* n) {
  o->label(n);
  redraw();
}

Widget_Node::~Widget_Node() {
  if (o) {
    Fl_Window* win = o->window();
    delete o;
    if (win)
      win->redraw();
  }
}

void Widget_Node::extra_code(int m, const std::string& n) {
  storestring(n, extra_code_[m]);
}

void Widget_Node::subclass(const std::string& n) {
  if (storestring(n, subclass_) && visible)
    redraw_browser();
}

void Widget_Node::tooltip(const std::string& text) {
  storestring(text, tooltip_);
  if (text.empty()) {
    o->tooltip(nullptr);
  } else {
    o->copy_tooltip(text.c_str());
  }
}

void Widget_Node::redraw() {
  Node* t = this;
  if (dynamic_cast<Menu_Item_Node*>(this)) {
    // find the menu button that parents this menu:
    do {
      t = t->parent;
    } while (t && dynamic_cast<Menu_Item_Node*>(t));
    // kludge to cause build_menu to be called again:
    if (t)
      t->add_child(nullptr, nullptr);
  } else {
    while (t->parent && t->parent->is_widget())
      t = t->parent;
    ((Widget_Node*)t)->o->redraw();
  }
}

// the recursive part sorts all children, returns pointer to next:
Node* sort(Node* parent) {
  Node* f;
  Node* n = nullptr;
  for (f = parent ? parent->next : Fluid.proj.tree.first; ; f = n) {
    if (!f || (parent && f->level <= parent->level))
      break;
    n = sort(f);
    if (!f->selected || !f->is_true_widget())
      continue;
    Fl_Widget* fw = ((Widget_Node*)f)->o;
    Node* g; // we will insert before this
    for (g = parent ? parent->next : Fluid.proj.tree.first; g != f; g = g->next) {
      if (!g->selected || g->level > f->level)
        continue;
      Fl_Widget* gw = ((Widget_Node*)g)->o;
      if (gw->y() > fw->y())
        break;
      if (gw->y() == fw->y() && gw->x() > fw->x())
        break;
    }
    if (g != f)
      f->move_before(g);
  }
  if (parent)
    parent->layout_widget();
  return f;
}


////////////////////////////////////////////////////////////////

// turn number to string or string to number for saving to file:
// does not work for hierarchical menus!

const char* item_name(Fl_Menu_Item* m, int i) {
  if (m) {
    while (m->label()) {
      if (m->argument() == i)
        return m->label();
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
    if (i[0]=='F' && i[1]=='L' && i[2]=='_')
      i += 3;
    while (m->label()) {
      if (!strcmp(m->label(), i)) return
        int(m->argument());
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

const char* boxname(int i) {
  if (!i) i = ZERO_ENTRY;
  for (int j = 0; j < int(sizeof(boxmenu)/sizeof(*boxmenu)); j++)
    if (boxmenu[j].argument() == i)
      return boxmenu[j].label();
  return nullptr;
}

int boxnumber(const char* i) {
  if (i[0]=='F' && i[1]=='L' && i[2]=='_')
    i += 3;
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

uchar Widget_Node::resizable() const {
  if (dynamic_cast<const Window_Node*>(this))
    return ((Fl_Window*)o)->resizable() != nullptr;
  Fl_Group* p = (Fl_Group*)o->parent();
  if (p)
    return p->resizable() == o;
  else
    return 0;
}

void Widget_Node::resizable(uchar v) {
  if (v) {
    if (resizable())
      return;
    if (dynamic_cast<Window_Node*>(this)) {
      ((Fl_Window*)o)->resizable(o);
    } else {
      Fl_Group* p = (Fl_Group*)o->parent();
      if (p) p->resizable(o);
    }
  } else {
    if (!resizable())
      return;
    if (dynamic_cast<Window_Node*>(this)) {
      ((Fl_Window*)o)->resizable(nullptr);
    } else {
      Fl_Group* p = (Fl_Group*)o->parent();
      if (p) p->resizable(nullptr);
    }
  }
}



Fl_Menu_Item labeltypemenu[] = {
  {"NORMAL_LABEL",0,nullptr,(void*)nullptr},
  {"SHADOW_LABEL",0,nullptr,(void*)FL_SHADOW_LABEL},
  {"ENGRAVED_LABEL",0,nullptr,(void*)FL_ENGRAVED_LABEL},
  {"EMBOSSED_LABEL",0,nullptr,(void*)FL_EMBOSSED_LABEL},
  {"NO_LABEL",0,nullptr,(void*)(FL_NO_LABEL)},
  {nullptr}};



// Not static: also used by align_cb/align_position_cb/align_text_image_cb
// in panels/widget_panel_callbacks.cxx.
Fl_Menu_Item alignmenu[] = {
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
  {nullptr}
};


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






////////////////////////////////////////////////////////////////

// subtypes:

Fl_Menu_Item* Widget_Node::subtypes() { return nullptr; }


////////////////////////////////////////////////////////////////


extern void open_panel(); // in panels/widget_panel_callbacks.cxx

// This is called when user double-clicks an item, open or update the panel:
void Widget_Node::open() {
  open_panel();
}

int is_name(const char* c) {
  for (; *c; c++)
    if ((ispunct(*c)||*c=='\n') && *c!='_' && *c!=':') return 0;
  return 1;
}

// Test to see if name() is an array entry.  If so, and this is the
// highest number, return name[num+1].  Return null if not the highest
// number or a field or function.  Return name() if not an array entry.
const char* array_name(Widget_Node* o) {
  const char* c = o->name();
  if (!c) return nullptr;
  const char* d;
  for (d = c; *d != '['; d++) {
    if (!*d) return c;
    if (ispunct(*d) && *d!='_') return nullptr;
  }
  int num = atoi(d+1);
  int sawthis = 0;
  Node* t = o->prev;
  Node* tp = o;
  const char* cn = o->class_name(1);
  for (; t && t->class_name(1) == cn; tp = t, t = t->prev) {/*empty*/}
  for (t = tp; t && t->class_name(1) == cn; t = t->next) {
    if (t == o) {sawthis=1; continue;}
    const char* e = t->name();
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
int isdeclare(const char* c) {
  while (fl_ascii_isspace(*c)) c++;
  if (*c == '#') return 1;
  if (!strncmp(c,"extern",6)) return 1;
  if (!strncmp(c,"typedef",7)) return 1;
  if (!strncmp(c,"using",5)) return 1;
  return 0;
}

void Widget_Node::write_static(fluid::io::Code_Writer& f) {
  std::string t = subclassname(this);
  if (subclass().empty() || (is_class() && (t.compare(0, 3, "Fl_")==0))) {
    f.write_h_once("#include <FL/Fl.H>");
    f.write_h_once("#include <FL/" + t + ".H>");
  }
  for (int n=0; n < NUM_EXTRA_CODE; n++) {
    if (!extra_code(n).empty() && isdeclare(extra_code(n).c_str()))
      f.write_h_once(extra_code(n));
  }
  if (callback() && is_name(callback())) {
    std::string callback_name_pattern = std::string(callback()) + "(*)";
    Node* pClass = find_parent_class_node();
    if (pClass && pClass->has_function("static void", callback_name_pattern)) {
      // nothing to do, method already exists
    } else if (has_toplevel_function("*void", callback_name_pattern)) {
      // nothing to do, function already exists
    } else {
      f.write_h_once("extern void " + std::string(callback()) + "(" + t + "*, " + user_data_type_or_voidp() + ");");
    }
  }
  const char* k = class_name(1);
  const char* c = array_name(this);
  if (c && !k && !is_class()) {
    f.write_c("\n");
    if (!public_) f.write_c("static ");
    else f.write_h("extern " + t + "* " + c + ";\n");
    if (strchr(c, '[') == nullptr)
      f.write_c(t + "* " + c + " = (" + t + "*)nullptr;\n");
    else
      f.write_c(t + "* " + c + " = {(" + t + "*)nullptr};\n");
  }
  if (callback() && !is_name(callback()) && (callback()[0] != '[')) {
    // see if 'o' or 'v' used, to prevent unused argument warnings:
    int use_o = 0;
    int use_v = 0;
    const char* d;
    for (d = callback(); *d;) {
      if (*d == 'o' && !is_id(d[1])) use_o = 1;
      if (*d == 'v' && !is_id(d[1])) use_v = 1;
      do d++; while (is_id(*d));
      while (*d && !is_id(*d)) d++;
    }
    std::string cn = callback_name(f);
    if (k) {
      f.write_c("\nvoid " + std::string(k) + "::" + cn + "_i(" + t + "*");
    } else {
      f.write_c("\nstatic void " + cn + "(" + t + "*");
    }
    if (use_o) f.write_c(" o");
    std::string ut = user_data_type_or_voidp();
    f.write_c(", " + ut);
    if (use_v) f.write_c(" v");
    f.write_c(") {\n");
    f.tag(Mergeback::Tag::GENERIC, Mergeback::Tag::WIDGET_CALLBACK, 0);
    f.write_c_indented(callback(), 1, 0);
    if (*(d-1) != ';' && *(d-1) != '}') {
      const char* p = strrchr(callback(), '\n');
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
      f.write_c("void " + std::string(k) + "::" + std::string(cn) + "(" + t + "* o, " + ut + " v) {\n");
      f.write_c(f.indent(1) + "((" + std::string(k)+ "*)(o");
      Node* q = nullptr;
      for (Node* p = parent; p && p->is_widget(); q = p, p = p->parent)
        f.write_c("->parent()");
      if (!q || !dynamic_cast<Widget_Class_Node*>(q))
        f.write_c("->user_data()");
      f.write_c("))->" + std::string(cn) + "_i(o,v);\n}\n");
    }
  }
  active_image.write_static(f);
  inactive_image.write_static(f);
}

void Widget_Node::write_code1(fluid::io::Code_Writer& f) {
  std::string t = subclassname(this);
  const char* c = array_name(this);
  if (c) {
    if (class_name(1)) {
      f.write_public(public_);
      f.write_h(f.indent(1) + t + "* " + c + ";\n");
    }
  }
  if (class_name(1) && callback() && !is_name(callback())) {
    std::string cn = callback_name(f);
    std::string ut = user_data_type_or_voidp();
    f.write_public(0);
    f.write_h(f.indent(1) + "inline void " + cn + "_i(" + t + "*, " + ut + ");\n");
    f.write_h(f.indent(1) + "static void " + cn + "(" + t + "*, " + ut + ");\n");
  }
  // figure out if local variable will be used (prevent compiler warnings):
  int wused = !name() && dynamic_cast<Window_Node*>(this);
  const char* ptr;

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
      if (!extra_code(n).empty() && !isdeclare(extra_code(n).c_str()))
      {
        int instring = 0;
        int inname = 0;
        int incomment = 0;
        int incppcomment = 0;
        std::string code = extra_code(n);
        for (ptr = code.c_str(); *ptr; ptr ++) {
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

  f.write_c(f.indent() + "{ ");
  write_comment_inline_c(f);
  if (f.varused) f.write_c(t + "* o = ");
  if (name()) f.write_c(std::string(name()) + " = ");
  if (dynamic_cast<Window_Node*>(this)) {
    // Handle special case where user is faking a Fl_Group type as a window,
    // there is no 2-argument constructor in that case:
    if (t.find("Window")==t.npos)
      f.write_c("new " + t + "(0, 0, " + std::to_string(o->w()) + ", " + std::to_string(o->h()));
    else
      f.write_c("new " + t + "(" + std::to_string(o->w()) + ", " + std::to_string(o->h()));
  } else if (dynamic_cast<Menu_Bar_Node*>(this)
             && ((Menu_Bar_Node*)this)->is_sys_menu_bar()
             && is_in_class()) {
    f.write_c("(" + t + "*)new " + ((Menu_Bar_Node*)this)->sys_menubar_proxy_name()
              + "(" + std::to_string(o->x()) + ", " + std::to_string(o->y())
              + ", " + std::to_string(o->w()) + ", " + std::to_string(o->h()) );
  } else {
    f.write_c("new " + t
              + "(" + std::to_string(o->x()) + ", " + std::to_string(o->y())
              + ", " + std::to_string(o->w()) + ", " + std::to_string(o->h()) );
  }
  if (label() && *label()) {
    f.write_c(", ");
    switch (Fluid.proj.i18n.type) {
    case fluid::I18n_Type::NONE : /* None */
        f.write_cstring(label());
        break;
    case fluid::I18n_Type::GNU : /* GNU gettext */
        f.write_c(Fluid.proj.i18n.gnu_function + "(");
        f.write_cstring(label());
        f.write_c(")");
        break;
    case fluid::I18n_Type::POSIX : /* POSIX catgets */
        f.write_c("catgets("
                  + (Fluid.proj.i18n.posix_file.empty() ? std::string("_catalog") : Fluid.proj.i18n.posix_file)
                  + ", " + Fluid.proj.i18n.posix_set + ", " + std::to_string(msgnum()) + ",");
        f.write_cstring(label());
        f.write_c(")");
        break;
    }
  }
  f.write_c(");\n");

  f.indent_more();

  // Avoid compiler warning for unused variable.
  // Also avoid quality control warnings about incorrect allocation error handling.
  if (wused) f.write_c(f.indent() + "w = o; (void)w;\n");

  write_widget_code(f);
}

void Widget_Node::write_color(fluid::io::Code_Writer& f, const char* field, Fl_Color color) {
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
  const char* var = is_class() ? "this" : name() ? name() : "o";
  if (color_name) {
    f.write_c(f.indent() + var + "->" + field + "(" + color_name + ");\n");
  } else {
    f.write_c(f.indent() + var + "->" + field + "((Fl_Color)" + std::to_string(color) + ");\n");
  }
}

// this is split from write_code1(fluid::io::Code_Writer& f) for Window_Node:
void Widget_Node::write_widget_code(fluid::io::Code_Writer& f) {
  Fl_Widget* tplate = ((Widget_Node*)factory)->o;
  const char* var = is_class() ? "this" : name() ? name() : "o";

  if (!tooltip().empty()) {
    f.write_c(f.indent() + var + "->tooltip(");
    switch (Fluid.proj.i18n.type) {
    case fluid::I18n_Type::NONE : /* None */
        f.write_cstring(tooltip().c_str());
        break;
    case fluid::I18n_Type::GNU : /* GNU gettext */
        f.write_c(Fluid.proj.i18n.gnu_function + "(");
        f.write_cstring(tooltip().c_str());
        f.write_c(")");
        break;
    case fluid::I18n_Type::POSIX : /* POSIX catgets */
        f.write_c("catgets("
                  + (Fluid.proj.i18n.posix_file.empty() ? std::string("_catalog") : Fluid.proj.i18n.posix_file)
                  + ", " + Fluid.proj.i18n.posix_set
                  + ", " + std::to_string(msgnum() + 1) + ", ");
        f.write_cstring(tooltip().c_str());
        f.write_c(")");
        break;
    }
    f.write_c(");\n");
  }

  if (dynamic_cast<Spinner_Node*>(this) && ((Fl_Spinner*)o)->type() != ((Fl_Spinner*)tplate)->type())
    f.write_c(f.indent() + var + "->type(" + std::to_string(((Fl_Spinner*)o)->type()) + ");\n");
  else if (o->type() != tplate->type() && !dynamic_cast<Window_Node*>(this))
    f.write_c(f.indent() + var + "->type(" + std::to_string(o->type()) + ");\n");
  if (o->box() != tplate->box() || !subclass().empty())
    f.write_c(f.indent() + var + "->box(FL_" + boxname(o->box()) + ");\n");

  // write shortcut command if needed
  int shortcut = 0;
  if (is_button()) shortcut = ((Fl_Button*)o)->shortcut();
  else if (dynamic_cast<Input_Node*>(this)) shortcut = ((Fl_Input_*)o)->shortcut();
  else if (dynamic_cast<Value_Input_Node*>(this)) shortcut = ((Fl_Value_Input*)o)->shortcut();
  else if (dynamic_cast<Text_Display_Node*>(this)) shortcut = ((Fl_Text_Display*)o)->shortcut();
  if (shortcut) {
    int s = shortcut;
    f.write_c(f.indent() + var + "->shortcut(");
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
      f.write_c("'" + std::string(1, s) + "');\n");
    else {
      f.write_c("0x" + fluid::io::to_string_8x(s) + ");\n");
    }
  }

  if (dynamic_cast<Button_Node*>(this)) {
    Fl_Button* b = (Fl_Button*)o;
    if (b->down_box())
      f.write_c(f.indent() + var + "->down_box(FL_" + boxname(b->down_box()) + ");\n");
    if (b->value())
      f.write_c(f.indent() + var + "->value(1);\n");
    if (b->compact())
      f.write_c(f.indent() + var + "->compact(" + std::to_string(b->compact()) + ");\n");
  } else if (dynamic_cast<Input_Choice_Node*>(this)) {
    Fl_Input_Choice* b = (Fl_Input_Choice*)o;
    if (b->down_box())
      f.write_c(f.indent() + var + "->down_box(FL_" + boxname(b->down_box()) + ");\n");
  } else if (dynamic_cast<Menu_Manager_Node*>(this)) {
    Fl_Menu_* b = (Fl_Menu_*)o;
    if (b->down_box())
      f.write_c(f.indent() + var + "->down_box(FL_" + boxname(b->down_box()) + ");\n");
  }
  if (o->color() != tplate->color() || !subclass().empty())
    write_color(f, "color", o->color());
  if (o->selection_color() != tplate->selection_color() || !subclass().empty())
    write_color(f, "selection_color", o->selection_color());
  active_image.write_code(f, var, false);
  inactive_image.write_code(f, var, true);
  if (o->labeltype() != tplate->labeltype() || !subclass().empty())
    f.write_c(f.indent() + var + "->labeltype(FL_" + item_name(labeltypemenu, o->labeltype()) + ");\n");
  if (o->labelfont() != tplate->labelfont() || !subclass().empty())
    f.write_c(f.indent() + var + "->labelfont(" + std::to_string(o->labelfont()) + ");\n");
  if (o->labelsize() != tplate->labelsize() || !subclass().empty())
    f.write_c(f.indent() + var + "->labelsize(" + std::to_string(o->labelsize()) + ");\n");
  if (o->labelcolor() != tplate->labelcolor() || !subclass().empty())
    write_color(f, "labelcolor", o->labelcolor());
  if (o->horizontal_label_margin() != tplate->horizontal_label_margin())
    f.write_c(f.indent() + var + "->horizontal_label_margin(" + std::to_string(o->horizontal_label_margin()) + ");\n");
  if (o->vertical_label_margin() != tplate->vertical_label_margin())
    f.write_c(f.indent() + var + "->vertical_label_margin(" + std::to_string(o->vertical_label_margin()) + ");\n");
  if (o->label_image_spacing() != tplate->label_image_spacing())
    f.write_c(f.indent() + var + "->label_image_spacing(" + std::to_string(o->label_image_spacing()) + ");\n");
  if (dynamic_cast<Valuator_Node*>(this)) {
    Fl_Valuator* v = (Fl_Valuator*)o;
    Fl_Valuator* t = (Fl_Valuator*)(tplate);
    if (v->minimum()!=t->minimum())
      f.write_c(f.indent() + var + "->minimum(" + fluid::io::to_string_g(v->minimum()) + ");\n");
    if (v->maximum()!=t->maximum())
      f.write_c(f.indent() + var + "->maximum(" + fluid::io::to_string_g(v->maximum()) + ");\n");
    if (v->step()!=t->step())
      f.write_c(f.indent() + var + "->step(" + fluid::io::to_string_g(v->step()) + ");\n");
    if (v->value()) {
      if (dynamic_cast<Scrollbar_Node*>(this)) { // Fl_Scrollbar::value(double) is not available
        f.write_c(f.indent() + var + "->Fl_Slider::value(" + fluid::io::to_string_g(v->value()) + ");\n");
      } else {
        f.write_c(f.indent() + var + "->value(" + fluid::io::to_string_g(v->value()) + ");\n");
      }
    }
    if (dynamic_cast<Slider_Node*>(this)) {
      double x = ((Fl_Slider*)v)->slider_size();
      double y = ((Fl_Slider*)t)->slider_size();
      if (x != y) f.write_c(f.indent() + var + "->slider_size(" + fluid::io::to_string_g(x) + ");\n");
    }
  }
  if (dynamic_cast<Spinner_Node*>(this)) {
    Fl_Spinner* v = (Fl_Spinner*)o;
    Fl_Spinner* t = (Fl_Spinner*)(tplate);
    if (v->minimum()!=t->minimum())
      f.write_c(f.indent() + var + "->minimum(" + fluid::io::to_string_g(v->minimum()) + ");\n");
    if (v->maximum()!=t->maximum())
      f.write_c(f.indent() + var + "->maximum(" + fluid::io::to_string_g(v->maximum()) + ");\n");
    if (v->step()!=t->step())
      f.write_c(f.indent() + var + "->step(" + fluid::io::to_string_g(v->step()) + ");\n");
    if (v->value()!=1.0f)
      f.write_c(f.indent() + var + "->value(" + fluid::io::to_string_g(v->value()) + ");\n");
  }

  {Fl_Font ff; int fs; Fl_Color fc; if (textstuff(4,ff,fs,fc)) {
    Fl_Font g; int s; Fl_Color c; textstuff(0,g,s,c);
    if (g != ff) f.write_c(f.indent() + var + "->textfont(" + std::to_string(g) + ");\n");
    if (s != fs) f.write_c(f.indent() + var + "->textsize(" + std::to_string(s) + ");\n");
    if (c != fc) write_color(f, "textcolor", c);
  }}
  std::string ud = user_data();
  if (class_name(1) && !parent->is_widget()) ud = "this";
  if (callback()) {
    if (callback()[0] == '[') { // lambda callback function
      f.write_c(f.indent() + var + "->callback(\n");
      f.tag(Mergeback::Tag::GENERIC, Mergeback::Tag::WIDGET_CALLBACK, 0);
      f.write_c_indented(callback(), 1, 0);
      f.write_c("\n");
      f.tag(Mergeback::Tag::WIDGET_CALLBACK, Mergeback::Tag::GENERIC, get_uid());
      f.write_c(f.indent());
    } else {
      f.write_c(f.indent() + var + "->callback((Fl_Callback*)" + callback_name(f));
    }
    if (!ud.empty())
      f.write_c(", (void*)(" + ud + "));\n");
    else
      f.write_c(");\n");
  } else if (!ud.empty()) {
    f.write_c(f.indent() + var + "->user_data((void*)(" + ud + "));\n");
  }
  if (o->align() != tplate->align() || !subclass().empty()) {
    int i = o->align();
    f.write_c(f.indent() + var + "->align(Fl_Align("
            + item_name(alignmenu, i & ~FL_ALIGN_INSIDE));
    if (i & FL_ALIGN_INSIDE) f.write_c("|FL_ALIGN_INSIDE");
    f.write_c("));\n");
  }
  Fl_When ww = o->when();
  if (ww != tplate->when() || !subclass().empty())
    f.write_c(f.indent() + var + "->when(" + when_symbol_name(ww) + ");\n");
  if (!o->visible() && o->parent())
    f.write_c(f.indent() + var + "->hide();\n");
  if (!o->active())
    f.write_c(f.indent() + var + "->deactivate();\n");
  if (!dynamic_cast<Group_Node*>(this) && resizable())
    f.write_c(f.indent() + "Fl_Group::current()->resizable(" + var + ");\n");
  if (hotspot()) {
    if (is_class())
      f.write_c(f.indent() + "hotspot(" + var + ");\n");
    else if (dynamic_cast<Window_Node*>(this))
      f.write_c(f.indent() + var + "->hotspot(" + var + ");\n");
    else
      f.write_c(f.indent() + var + "->window()->hotspot(" + var + ");\n");
  }
}

void Widget_Node::write_extra_code(fluid::io::Code_Writer& f) {
  for (int n=0; n < NUM_EXTRA_CODE; n++)
    if (!extra_code(n).empty() && !isdeclare(extra_code(n).c_str()))
      f.write_c(f.indent() + extra_code(n) + "\n");
}

void Widget_Node::write_block_close(fluid::io::Code_Writer& f) {
  f.indent_less();
  f.write_c(f.indent() + "} // " + subclassname(this) + "* "
          + (name() ? name() : "o") + "\n");
}

void Widget_Node::write_code2(fluid::io::Code_Writer& f) {
  write_extra_code(f);
  write_block_close(f);
}

////////////////////////////////////////////////////////////////

void Widget_Node::write_properties(fluid::io::Project_Writer &f) {
  Node::write_properties(f);
  f.write_indent(level+1);
  switch (public_) {
    case 0: f.write_string("private"); break;
    case 1: break;
    case 2: f.write_string("protected"); break;
  }
  if (!tooltip().empty()) {
    f.write_string("tooltip");
    f.write_word(tooltip());
  }
  active_image.write_properties(f, false);
  inactive_image.write_properties(f, true);
  f.write_string("xywh {%d %d %d %d}", o->x(), o->y(), o->w(), o->h());
  Fl_Widget* tplate = ((Widget_Node*)factory)->o;
  if (dynamic_cast<Spinner_Node*>(this) && ((Fl_Spinner*)o)->type() != ((Fl_Spinner*)tplate)->type()) {
    f.write_string("type");
    f.write_word(item_name(subtypes(), ((Fl_Spinner*)o)->type()));
  } else if (subtypes() && (o->type() != tplate->type() || dynamic_cast<Window_Node*>(this))) {
    f.write_string("type");
    f.write_word(item_name(subtypes(), o->type()));
  }
  if (o->box() != tplate->box()) {
    f.write_string("box"); f.write_word(boxname(o->box()));}
  if (dynamic_cast<Input_Node*>(this)) {
    Fl_Input_* b = (Fl_Input_*)o;
    if (b->shortcut()) f.write_string("shortcut 0x%x", b->shortcut());
  }
  if (dynamic_cast<Value_Input_Node*>(this)) {
    Fl_Value_Input* b = (Fl_Value_Input*)o;
    if (b->shortcut()) f.write_string("shortcut 0x%x", b->shortcut());
  }
  if (dynamic_cast<Text_Display_Node*>(this)) {
    Fl_Text_Display* b = (Fl_Text_Display*)o;
    if (b->shortcut()) f.write_string("shortcut 0x%x", b->shortcut());
  }
  if (dynamic_cast<Button_Node*>(this)) {
    Fl_Button* b = (Fl_Button*)o;
    if (b->down_box()) {
      f.write_string("down_box"); f.write_word(boxname(b->down_box()));}
    if (b->shortcut()) f.write_string("shortcut 0x%x", b->shortcut());
    if (b->value()) f.write_string("value 1");
  } else if (dynamic_cast<Input_Choice_Node*>(this)) {
    Fl_Input_Choice* b = (Fl_Input_Choice*)o;
    if (b->down_box()) {
      f.write_string("down_box"); f.write_word(boxname(b->down_box()));}
  } else if (dynamic_cast<Menu_Base_Node*>(this)) {
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
  if (dynamic_cast<Valuator_Node*>(this)) {
    Fl_Valuator* v = (Fl_Valuator*)o;
    Fl_Valuator* t = (Fl_Valuator*)(tplate);
    if (v->minimum()!=t->minimum()) f.write_string("minimum %g",v->minimum());
    if (v->maximum()!=t->maximum()) f.write_string("maximum %g",v->maximum());
    if (v->step()!=t->step()) f.write_string("step %g",v->step());
    if (v->value()!=0.0) f.write_string("value %g",v->value());
    if (dynamic_cast<Slider_Node*>(this)) {
      double x = ((Fl_Slider*)v)->slider_size();
      double y = ((Fl_Slider*)t)->slider_size();
      if (x != y) f.write_string("slider_size %g", x);
    }
  }
  if (dynamic_cast<Spinner_Node*>(this)) {
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
  if (hotspot()) f.write_string(dynamic_cast<Menu_Item_Node*>(this) ? "divider" : "hotspot");
  if (dynamic_cast<Menu_Item_Node*>(this)) {
    auto nd = dynamic_cast<Menu_Item_Node*>(this);
    if (nd && nd->headline()) f.write_string("headline");
  }
  for (int n=0; n < NUM_EXTRA_CODE; n++) if (!extra_code(n).empty()) {
    f.write_indent(level+1);
    f.write_string("code%d",n);
    f.write_word(extra_code(n).c_str());
  }
  if (!subclass().empty()) {
    f.write_indent(level+1);
    f.write_string("class");
    f.write_word(subclass());
  }
}

void Widget_Node::read_property(fluid::io::Project_Reader &f, const char* c) {
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
      active_image.scale_w = w;
      active_image.scale_h = h;
    }
  } else if (!strcmp(c,"image")) {
    active_image.set(f.read_word(), dynamic_cast<Window_Node*>(this) ? nullptr : o, false);
    if (!dynamic_cast<Window_Node*>(this)) redraw();
    // starting in 2023, `image` is always followed by `compress_image`
    // the code below is for compatibility with older .fl files
    std::string ext = fl_filename_ext_str(active_image.name);
    if (   (ext != ".jpg")
        && (ext != ".png")
        && (ext != ".svg")
        && (ext != ".svgz"))
      active_image.compress = 0; // if it is neither of those, default to uncompressed
  } else if (!strcmp(c,"bind_image")) {
    active_image.bind = (int)atol(f.read_word());
  } else if (!strcmp(c,"compress_image")) {
    active_image.compress = (int)atol(f.read_word());
  } else if (!strcmp(c,"scale_deimage")) {
    if (sscanf(f.read_word(),"%d %d",&w,&h) == 2) {
      inactive_image.scale_w = w;
      inactive_image.scale_h = h;
    }
  } else if (!strcmp(c,"deimage")) {
    inactive_image.set(f.read_word(), dynamic_cast<Window_Node*>(this) ? nullptr : o, true);
    if (!dynamic_cast<Window_Node*>(this)) redraw();
    // starting in 2023, `deimage` is always followed by `compress_deimage`
    // the code below is for compatibility with older .fl files
    std::string ext = fl_filename_ext_str(inactive_image.name);
    if (   (ext != ".jpg")
        && (ext != ".png")
        && (ext != ".svg")
        && (ext != ".svgz"))
      inactive_image.compress = 0; // if it is neither of those, default to uncompressed
  } else if (!strcmp(c,"bind_deimage")) {
    inactive_image.bind = (int)atol(f.read_word());
  } else if (!strcmp(c,"compress_deimage")) {
    inactive_image.compress = (int)atol(f.read_word());
  } else if (!strcmp(c,"type")) {
    if (dynamic_cast<Spinner_Node*>(this))
      ((Fl_Spinner*)o)->type(item_number(subtypes(), f.read_word()));
    else
      o->type(item_number(subtypes(), f.read_word()));
  } else if (!strcmp(c,"box")) {
    const char* value = f.read_word();
    if ((x = boxnumber(value))) {
      if (x == ZERO_ENTRY) x = 0;
      o->box((Fl_Boxtype)x);
    } else if (sscanf(value,"%d",&x) == 1) o->box((Fl_Boxtype)x);
  } else if (dynamic_cast<Button_Node*>(this) && !strcmp(c,"down_box")) {
    const char* value = f.read_word();
    if ((x = boxnumber(value))) {
      if (x == ZERO_ENTRY) x = 0;
      ((Fl_Button*)o)->down_box((Fl_Boxtype)x);
    }
  } else if (dynamic_cast<Input_Choice_Node*>(this) && !strcmp(c,"down_box")) {
    const char* value = f.read_word();
    if ((x = boxnumber(value))) {
      if (x == ZERO_ENTRY) x = 0;
      ((Fl_Input_Choice*)o)->down_box((Fl_Boxtype)x);
    }
  } else if (dynamic_cast<Menu_Base_Node*>(this) && !strcmp(c,"down_box")) {
    const char* value = f.read_word();
    if ((x = boxnumber(value))) {
      if (x == ZERO_ENTRY) x = 0;
      ((Fl_Menu_*)o)->down_box((Fl_Boxtype)x);
    }
  } else if (is_button() && !strcmp(c,"value")) {
    const char* value = f.read_word();
    ((Fl_Button*)o)->value(atoi(value));
  } else if (!strcmp(c,"color")) {
    const char* cw = f.read_word();
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
      if (!Fluid.proj.image_assets.find_or_create(label()))
        f.read_error("Image file '%s' not found", label());
      active_image.set(label(), dynamic_cast<Window_Node*>(this) ? nullptr : o, false);
      if (!dynamic_cast<Window_Node*>(this)) redraw();
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
    if (dynamic_cast<Valuator_Node*>(this)) ((Fl_Valuator*)o)->minimum(strtod(f.read_word(),nullptr));
    if (dynamic_cast<Spinner_Node*>(this)) ((Fl_Spinner*)o)->minimum(strtod(f.read_word(),nullptr));
  } else if (!strcmp(c,"maximum")) {
    if (dynamic_cast<Valuator_Node*>(this)) ((Fl_Valuator*)o)->maximum(strtod(f.read_word(),nullptr));
    if (dynamic_cast<Spinner_Node*>(this)) ((Fl_Spinner*)o)->maximum(strtod(f.read_word(),nullptr));
  } else if (!strcmp(c,"step")) {
    if (dynamic_cast<Valuator_Node*>(this)) ((Fl_Valuator*)o)->step(strtod(f.read_word(),nullptr));
    if (dynamic_cast<Spinner_Node*>(this)) ((Fl_Spinner*)o)->step(strtod(f.read_word(),nullptr));
  } else if (!strcmp(c,"value")) {
    if (dynamic_cast<Valuator_Node*>(this)) ((Fl_Valuator*)o)->value(strtod(f.read_word(),nullptr));
    if (dynamic_cast<Spinner_Node*>(this)) ((Fl_Spinner*)o)->value(strtod(f.read_word(),nullptr));
  } else if ( (!strcmp(c,"slider_size") || !strcmp(c,"size")) && dynamic_cast<Slider_Node*>(this)) {
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
  } else if (!strcmp(c,"headline")) {
    if (dynamic_cast<Menu_Item_Node*>(this)) {
      auto nd = dynamic_cast<Menu_Item_Node*>(this);
      if (nd) nd->headline(true);
    }
  } else if (!strcmp(c,"class")) {
    subclass(f.read_word());
  } else if (!strcmp(c,"shortcut")) {
    int shortcut = (int)strtol(f.read_word(),nullptr,0);
    if (is_button()) ((Fl_Button*)o)->shortcut(shortcut);
    else if (dynamic_cast<Input_Node*>(this)) ((Fl_Input_*)o)->shortcut(shortcut);
    else if (dynamic_cast<Value_Input_Node*>(this)) ((Fl_Value_Input*)o)->shortcut(shortcut);
    else if (dynamic_cast<Text_Display_Node*>(this)) ((Fl_Text_Display*)o)->shortcut(shortcut);
  } else {
    if (!strncmp(c,"code",4)) {
      int n = atoi(c+4);
      if (n >= 0 && n <= NUM_EXTRA_CODE) {
        extra_code(n, f.read_word());
        return;
      }
    } else if (!strcmp(c,"extra_code")) {
      extra_code(0, f.read_word());
      return;
    }
    Node::read_property(f, c);
  }
}

Fl_Menu_Item boxmenu1[] = {
  // these extra ones are for looking up fdesign saved strings:
  {"NO_FRAME",          0,nullptr,(void *)FL_NO_BOX},
  {"ROUNDED3D_UPBOX",   0,nullptr,(void *)FL_ROUND_UP_BOX},
  {"ROUNDED3D_DOWNBOX", 0,nullptr,(void *)FL_ROUND_DOWN_BOX},
  {"OVAL3D_UPBOX",      0,nullptr,(void *)FL_ROUND_UP_BOX},
  {"OVAL3D_DOWNBOX",    0,nullptr,(void *)FL_ROUND_DOWN_BOX},
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
      if (fluid::io::fdesign_flip) {
        Node* p;
        for (p = parent; p && !dynamic_cast<Window_Node*>(p); p = p->parent) {/*empty*/}
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
      char buf[128]; sprintf(buf,"o->shortcut(\"%s\");", value);
      extra_code(0, buf);
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

Fl_Widget* Widget_Node::enter_live_mode(int) {
  live_widget = widget(o->x(), o->y(), o->w(), o->h());
  if (live_widget)
    copy_properties();
  return live_widget;
}

Fl_Widget* Widget_Node::propagate_live_mode(Fl_Group* grp) {
  live_widget = grp;
  copy_properties();
  Node* n;
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
  Fl_Widget* w = live_widget;
  w->label(o->label());
  if (tooltip().empty())
    w->tooltip(nullptr);
  else
    w->copy_tooltip(tooltip().c_str());
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
  if (dynamic_cast<Input_Node*>(this)) {
    Fl_Input_* d = (Fl_Input_*)live_widget, *s = (Fl_Input_*)o;
    d->shortcut(s->shortcut());
    d->textfont(ff);
    d->textsize(fs);
    d->textcolor(fc);
  }

  // copy all attributes specific to widgets derived from Fl_Value_Input
  if (dynamic_cast<Value_Input_Node*>(this)) {
    Fl_Value_Input* d = (Fl_Value_Input*)live_widget, *s = (Fl_Value_Input*)o;
    d->shortcut(s->shortcut());
    d->textfont(ff);
    d->textsize(fs);
    d->textcolor(fc);
  }

  // copy all attributes specific to widgets derived from Fl_Text_Display
  if (dynamic_cast<Text_Display_Node*>(this)) {
    Fl_Text_Display* d = (Fl_Text_Display*)live_widget, *s = (Fl_Text_Display*)o;
    d->shortcut(s->shortcut());
    d->textfont(ff);
    d->textsize(fs);
    d->textcolor(fc);
  }

  // copy all attributes specific to Fl_Valuator and derived classes
  if (dynamic_cast<Valuator_Node*>(this)) {
    Fl_Valuator* d = (Fl_Valuator*)live_widget, *s = (Fl_Valuator*)o;
    d->minimum(s->minimum());
    d->maximum(s->maximum());
    d->step(s->step());
    d->value(s->value());
    if (dynamic_cast<Slider_Node*>(this)) {
      Fl_Slider* d = (Fl_Slider*)live_widget, *s = (Fl_Slider*)o;
      d->slider_size(s->slider_size());
    }
  }

  // copy all attributes specific to Fl_Spinner and derived classes
  if (dynamic_cast<Spinner_Node*>(this)) {
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

