//
// Menu Node code for the Fast Light Tool Kit (FLTK).
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

#include "nodes/Menu_Node.h"

#include "Fluid.h"
#include "Project.h"
#include "proj/Image_Asset.h"
#include "proj/mergeback.h"
#include "proj/undo.h"
#include "io/Project_Reader.h"
#include "io/Project_Writer.h"
#include "io/Code_Writer.h"
#include "nodes/Window_Node.h"
#include "nodes/Function_Node.h"
#include "widgets/Formula_Input.h"
#include "widgets/Node_Browser.h"

#include <FL/Fl.H>
#include <FL/fl_message.H>
#include <FL/Fl_Menu_.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Shortcut_Button.H>
#include <FL/Fl_Output.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Multi_Label.H>
#include "../src/flstring.h"

#include <stdio.h>
#include <stdlib.h>

using namespace fluid;
using namespace fluid::proj;

Fl_Menu_Item menu_item_type_menu[] = {
  {"Normal",0,nullptr,(void*)nullptr},
  {"Toggle",0,nullptr,(void*)FL_MENU_BOX},
  {"Radio",0,nullptr,(void*)FL_MENU_RADIO},
  {nullptr}};

static void delete_dependents(Fl_Menu_Item *m) {
  if (!m)
    return;
  int level = 0;
  for (;;m++) {
    if (m->label()==nullptr) {
      if (level==0) {
        break;
      } else {
        level--;
      }
    }
    if (m->flags&FL_SUBMENU)
      level++;
    if (m->labeltype()==FL_MULTI_LABEL)
      delete (Fl_Multi_Label*)m->label();
  }
}

static void delete_menu(Fl_Menu_Item *m) {
  if (!m)
    return;
  delete_dependents(m);
  delete[] m;
}

void Input_Choice_Node::build_menu() {
  Fl_Input_Choice* w = (Fl_Input_Choice*)o;
  // count how many Fl_Menu_Item structures needed:
  int n = 0;
  Node* q;
  for (q = next; q && q->level > level; q = q->next) {
    if (q->can_have_children()) n++; // space for null at end of submenu
    n++;
  }
  if (!n) {
    if (menusize) delete_menu((Fl_Menu_Item*)(w->menu()));
    w->menu(nullptr);
    menusize = 0;
  } else {
    n++; // space for null at end of menu
    if (menusize<n) {
      if (menusize) delete_menu((Fl_Menu_Item*)(w->menu()));
      menusize = n+10;
      w->menu(new Fl_Menu_Item[menusize]);
    } else {
      if (menusize) delete_dependents((Fl_Menu_Item*)(w->menu()));
    }
    // Menus are already built during the .fl file reading process, so if the
    // end of a menu list is not read yet, the end markers (label==nullptr) will
    // not be set, and deleting dependents will randomly free memory.
    // Clearing the array should avoid that.
    memset( (void*)w->menu(), 0, menusize * sizeof(Fl_Menu_Item) );
    // fill them all in:
    Fl_Menu_Item* m = (Fl_Menu_Item*)(w->menu());
    int lvl = level+1;
    for (q = next; q && q->level > level; q = q->next) {
      Menu_Item_Node* i = (Menu_Item_Node*)q;
      if (i->o->image()) {
        if (i->o->label() && i->o->label()[0]) {
          Fl_Multi_Label *ml = new Fl_Multi_Label;
          ml->labela = (char*)i->o->image();
          ml->labelb = i->o->label();
          ml->typea = FL_IMAGE_LABEL;
          ml->typeb = FL_NORMAL_LABEL;
          ml->label(m);
        } else {
          i->o->image()->label(m);
        }
      } else {
        m->label(i->o->label() ? i->o->label() : "(nolabel)");
        m->labeltype(i->o->labeltype());
      }
      m->shortcut(((Fl_Button*)(i->o))->shortcut());
      m->callback(nullptr,(void*)i);
      m->flags = i->flags() & ~FL_MENU_HEADLINE;
      m->labelfont(i->o->labelfont());
      m->labelsize(i->o->labelsize());
      m->labelcolor(i->o->labelcolor());
      if (q->can_have_children()) {lvl++; m->flags |= FL_SUBMENU;}
      m++;
      int l1 =
        (q->next && dynamic_cast<Menu_Item_Node*>(q->next)) ? q->next->level : level;
      while (lvl > l1) {m->label(nullptr); m++; lvl--;}
      lvl = l1;
    }
  }
  o->redraw();
}

/**
 Create and add a new Menu Item node.
 \param[in] strategy add after current or as last child
 \return new Menu Item node
 */
Node *Menu_Item_Node::make(Strategy strategy) {
  return Menu_Item_Node::make(0, strategy);
}

/**
 Create an add a specific Menu Item node.
 \param[in] flags set to 0, FL_MENU_RADIO, FL_MENU_TOGGLE, or FL_SUBMENU
 \param[in] strategy add after current or as last child
 \return new Menu Item node
 */
Node* Menu_Item_Node::make(int flags, Strategy strategy) {
  // Find a good insert position based on the current marked node
  Node *anchor = Fluid.proj.tree.current, *p = anchor;
  if (p && (strategy.placement() == Strategy::AFTER_CURRENT))
    p = p->parent;
  while (p && !(dynamic_cast<Menu_Manager_Node*>(p) || dynamic_cast<Submenu_Node*>(p))) {
    anchor = p;
    strategy.placement(Strategy::AFTER_CURRENT);
    p = p->parent;
  }
  if (!p) {
    fl_message("Please select a menu widget or a menu item");
    return nullptr;
  }
  if (!o) {
    o = new Fl_Button(0,0,100,20); // create template widget
  }

  Menu_Item_Node* t = nullptr;
  if (flags==FL_SUBMENU) {
    t = new Submenu_Node();
  } else {
    t = new Menu_Item_Node();
  }
  t->o = new Fl_Button(0,0,100,20);
  t->o->type(flags);
  t->factory = this;
  t->add(anchor, strategy);
  if (strategy.source() == Strategy::FROM_USER) {
    if (flags==FL_SUBMENU) {
      t->label("submenu");
    } else {
      t->label("item");
    }
  }
  return t;
}

void group_selected_menuitems() {
  // The group will be created in the parent group of the current menuitem
  if (!dynamic_cast<Menu_Item_Node*>(Fluid.proj.tree.current)) {
    return;
  }
  Menu_Item_Node *q = static_cast<Menu_Item_Node*>(Fluid.proj.tree.current);
  Node *qq = Fluid.proj.tree.current->parent;
  if (!qq || !(dynamic_cast<Menu_Manager_Node*>(qq) || dynamic_cast<Submenu_Node*>(qq))) {
    fl_message("Can't create a new submenu here.");
    return;
  }
  Fluid.proj.undo.checkpoint();
  Fluid.proj.undo.suspend();
  Widget_Node *n = (Widget_Node*)(q->make(FL_SUBMENU, Strategy::AFTER_CURRENT));
  for (Node *t = qq->next; t && (t->level > qq->level);) {
    if (t->level != n->level || t == n || !t->selected) {
      t = t->next;
      continue;
    }
    Node *nxt = t->remove();
    t->add(n, Strategy::AS_LAST_CHILD);
    t = nxt;
  }
  widget_browser->rebuild();
  Fluid.proj.undo.resume();
  Fluid.proj.set_modflag(1);
}

void ungroup_selected_menuitems() {
  // Find the submenu
  Node *qq = Fluid.proj.tree.current->parent;
  Widget_Node *q = static_cast<Widget_Node*>(Fluid.proj.tree.current);
  int q_level = q->level;
  if (!qq || !dynamic_cast<Submenu_Node*>(qq)) {
    fl_message("Only menu items inside a submenu can be ungrouped.");
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


/**
 Create and add a new Checkbox Menu Item node.
 \param[in] strategy add after current or as last child
 \return new node
 */
Node *Checkbox_Menu_Item_Node::make(Strategy strategy) {
  return Menu_Item_Node::make(FL_MENU_TOGGLE, strategy);
}

/**
 Create and add a new Radio ButtonMenu Item node.
 \param[in] strategy add after current or as last child
 \return new node
 */
Node *Radio_Menu_Item_Node::make(Strategy strategy) {
  return Menu_Item_Node::make(FL_MENU_RADIO, strategy);
}

/**
 Create and add a new Submenu Item node.
 \param[in] strategy add after current or as last child
 \return new node
 */
Node *Submenu_Node::make(Strategy strategy) {
  return Menu_Item_Node::make(FL_SUBMENU, strategy);
}

Menu_Item_Node Menu_Item_Node::prototype;

Checkbox_Menu_Item_Node Checkbox_Menu_Item_Node::prototype;

Radio_Menu_Item_Node Radio_Menu_Item_Node::prototype;

Submenu_Node Submenu_Node::prototype;

////////////////////////////////////////////////////////////////
// Writing the C code:

// test functions in Widget_Node.C:
bool is_function_name(const std::string& name);
bool is_lambda(const std::string& name);
const char *array_name(Widget_Node *o);

// Search backwards to find the parent menu button and return it's name.
// Also put in i the index into the button's menu item array belonging
// to this menu item.
std::string Menu_Item_Node::menu_name(fluid::io::Code_Writer& f, int& i) {
  i = 0;
  Node* t = prev;
  while (t && dynamic_cast<Menu_Item_Node*>(t)) {
    // be sure to count the {0} that ends a submenu:
    if (t->level > t->next->level) i += (t->level - t->next->level);
    // detect empty submenu:
    else if (t->level == t->next->level && t->can_have_children()) i++;
    t = t->prev;
    i++;
  }
  if (!t) return "\n#error Menu_Item_Node::menu_name, invalid f\n";
  return f.unique_id(t, "menu", (t->name()?t->name():""), (t->label()?t->label():""));
}

void Menu_Item_Node::write_static(fluid::io::Code_Writer& f) {
  if (active_image.asset && label() && label()[0]) {
    f.write_h_once("#include <FL/Fl.H>");
    f.write_h_once("#include <FL/Fl_Multi_Label.H>");
  }
  if (callback() && is_function_name(callback())) {
    std::string callback_name_pattern = std::string(callback()) + "(*)";
    Node* pClass = find_parent_class_node();
    if (pClass && pClass->has_function("static void", callback_name_pattern)) {
      // nothing to do, method already exists
    } else if (has_toplevel_function("*void", callback_name_pattern)) {
      // nothing to do, function already exists
    } else {
      f.write_h_once("extern void " + std::string(callback()) + "(Fl_Menu_*, " + user_data_type_or_voidp() + ");");
    }
  }
  if (!extra_code(0).empty()) {
    f.write_block_h_once(extra_code(0)); // TODO: line by line
  }
  if (!extra_code(1).empty()) {
    f.write_h(extra_code(1));
    f.write_h("\n");
  }
  if (callback() && !is_function_name(callback()) && !is_lambda(callback())) {
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
    std::string cn = callback_name(f);
    const char* k = class_name(1);
    if (k) {
      f.write_c("\nvoid " + std::string(k) + "::" + cn + "_i(Fl_Menu_*");
    } else {
      f.write_c("\nstatic void " + cn + "(Fl_Menu_*");
    }
    if (use_o) f.write_c(" o");
    std::string ut = user_data_type_or_voidp();
    f.write_c(", " + ut);
    if (use_v) f.write_c(" v");
    f.write_c(") {\n");
    f.tag(Mergeback::Tag::GENERIC, Mergeback::Tag::MENU_CALLBACK, 0);
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
    f.tag(Mergeback::Tag::MENU_CALLBACK, Mergeback::Tag::GENERIC, get_uid());
    f.write_c("}\n");

    // If the menu item is part of a Class or Widget Class, FLUID generates
    // a dummy static callback which retrieves a pointer to the class and then
    // calls the original callback from within the class context.
    // k is the name of the enclosing class (or classes)
    if (k) {
      // Implement the callback as a static member function
      f.write_c("void " + std::string(k) + "::" + std::string(cn) + "(Fl_Menu_* o, " + ut + " v) {\n");
      // Find the Fl_Menu_ container for this menu item
      Node* t = parent; while (dynamic_cast<Menu_Item_Node*>(t)) t = t->parent;
      if (t) {
        Widget_Node *tw = (t->is_widget()) ? static_cast<Widget_Node*>(t) : nullptr;
        Node *q = nullptr;
        // Generate code to call the callback
        if (dynamic_cast<Menu_Bar_Node*>(tw) && ((Menu_Bar_Node*)tw)->is_sys_menu_bar()) {
          // Fl_Sys_Menu_Bar removes itself from any parent on macOS, so we
          // wrapped it in a class and remeber the parent class in a new
          // class memeber variable.
          Menu_Bar_Node *tmb = (Menu_Bar_Node*)tw;
          f.write_c(f.indent(1) + tmb->sys_menubar_proxy_name() + "* sys_menu_bar = ((" + tmb->sys_menubar_proxy_name() + "*)o);\n");
          f.write_c(f.indent(1) + k + "* parent_class = ((" + k + "*)sys_menu_bar->_parent_class);\n");
          f.write_c(f.indent(1) + "parent_class->" + cn + "_i(o,v);\n}\n");
        } else {
          f.write_c(f.indent(1) + "((" + k + "*)(o");
          // The class pointer is in the user_data field of the top widget
          if (t && dynamic_cast<Input_Choice_Node*>(t)) {
            // Go up one more level for Fl_Input_Choice, as these are groups themselves
            f.write_c("->parent()");
          }
          // Now generate code to find the topmost widget in this class
          for (t = t->parent; t && t->is_widget() && !is_class(); q = t, t = t->parent)
            f.write_c("->parent()");
          // user_data is cast into a pointer to the
          if (!q || !dynamic_cast<Widget_Class_Node*>(q))
            f.write_c("->user_data()");
          f.write_c("))->" + std::string(cn) + "_i(o,v);\n}\n");
        }
      } else {
        f.write_c("#error Enclosing Fl_Menu_* not found\n");
      }
    }
  }
  active_image.write_static(f);
  if (next && dynamic_cast<Menu_Item_Node*>(next)) return;
  // okay, when we hit last item in the menu we have to write the
  // entire array out:
  const char* k = class_name(1);
  if (k) {
    int i;
    f.write_c("\nFl_Menu_Item " + std::string(k) + "::" + menu_name(f, i) + "[] = {\n");
  } else {
    int i;
    f.write_c("\nFl_Menu_Item " + menu_name(f, i) + "[] = {\n");
  }
  Node* t = prev; while (t && dynamic_cast<Menu_Item_Node*>(t)) t = t->prev;
  for (Node* q = t->next; q && dynamic_cast<Menu_Item_Node*>(q); q = q->next) {
    ((Menu_Item_Node*)q)->write_item(f);
    int thislevel = q->level; if (q->can_have_children()) thislevel++;
    int nextlevel =
      (q->next && dynamic_cast<Menu_Item_Node*>(q->next)) ? q->next->level : t->level+1;
    while (thislevel > nextlevel) {
      // text, shortcut, callback, user_data, flags, labeltype, labelfont, labelsize, labelcolor
      f.write_c(" { nullptr, 0, nullptr, nullptr, 0, 0, 0, 0, 0 },\n");
      thislevel--;
    }
  }
  f.write_c(" { nullptr, 0, nullptr, nullptr, 0, 0, 0, 0, 0 }\n};\n");

  if (k) {
    // Write menu item variables...
    t = prev; while (t && dynamic_cast<Menu_Item_Node*>(t)) t = t->prev;
    for (Node* q = t->next; q && dynamic_cast<Menu_Item_Node*>(q); q = q->next) {
      Menu_Item_Node *m = (Menu_Item_Node*)q;
      const char *c = array_name(m);
      if (c) {
        if (c==m->name()) {
          // assign a menu item address directly to a variable
          int i;
          std::string n = ((Menu_Item_Node *)q)->menu_name(f, i);
          f.write_c("Fl_Menu_Item* " + std::string(k) + "::" + std::string(c) + " = " + std::string(k) + "::" + n + " + " + std::to_string(i) + ";\n");
        } else {
          // if the name is an array, only define the array.
          // The actual assignment is in write_code1(fluid::io::Code_Writer& f)
          f.write_c("Fl_Menu_Item* " + std::string(k) + "::" + std::string(c) + ";\n");
        }
      }
    }
  }
}

int Menu_Item_Node::flags() {
  int i = o->type();
  if (((Fl_Button*)o)->value()) i |= FL_MENU_VALUE;
  if (!o->active()) i |= FL_MENU_INACTIVE;
  if (!o->visible()) i |= FL_MENU_INVISIBLE;
  if (can_have_children()) {
    if (user_data().empty())
      i |= FL_SUBMENU;
    else
      i |= FL_SUBMENU_POINTER;
  }
  if (hotspot()) i |= FL_MENU_DIVIDER;
  if (headline()) i |= FL_MENU_HEADLINE;
  return i;
}

void Menu_Item_Node::write_item(fluid::io::Code_Writer& f) {
  static const char * const labeltypes[] = {
    "FL_NORMAL_LABEL",
    "FL_NO_LABEL",
    "FL_SHADOW_LABEL",
    "FL_ENGRAVED_LABEL",
    "FL_EMBOSSED_LABEL",
    "FL_MULTI_LABEL",
    "FL_ICON_LABEL",
    "FL_IMAGE_LABEL"
  };

  // Start of Fl_Menu_Item array member
  write_comment_inline_c(f, " ");
  f.write_c(" {");

  // Label, can not be nullptr which has a special meaning here
  if (label() && label()[0])
    switch (Fluid.proj.i18n.type) {
      case fluid::I18n_Type::GNU:
        // we will call i18n when the menu is instantiated for the first time
        f.write_c(Fluid.proj.i18n.gnu_static_function + "(");
        f.write_cstring(label());
        f.write_c(")");
        break;
      case fluid::I18n_Type::POSIX:
        // fall through: strings can't be translated before a catalog is chosen
      default:
        f.write_cstring(label());
    }
  else
    f.write_c("\"\"");

  // Shortcut, try to write it in a human readable form, else write it as hex
  if (((Fl_Button*)o)->shortcut()) {
    int s = ((Fl_Button*)o)->shortcut();
    f.write_c(", ");
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
      f.write_c("'" + std::string(1, (char)s) + "'");
    else
      f.write_c("0x" + fluid::io::to_string_8x(s) + ", ");
  } else {
    f.write_c(", 0, ");
  }

  // Write callback or nullptr
  if (callback()) {
    if (is_lambda(callback())) {
      // Write lambda expressions inline, allow mergeback
      f.write_c("\n");
      f.tag(Mergeback::Tag::GENERIC, Mergeback::Tag::WIDGET_CALLBACK, 0);
      f.write_c_indented(callback(), 1, 0);
      f.write_c("\n");
      f.tag(Mergeback::Tag::WIDGET_CALLBACK, Mergeback::Tag::GENERIC, get_uid());
      f.write_c(f.indent_plus(1) + ", ");
    } else {
      // Write named callback, try to qualify it with the class name if possible
      const char* k = is_function_name(callback()) ? nullptr : class_name(1);
      if (k) {
        f.write_c(" (Fl_Callback*)" + std::string(k) + "::" + std::string(callback_name(f)) + ",");
      } else {
        f.write_c(" (Fl_Callback*)" + std::string(callback_name(f)) + ",");
      }
    }
  } else
    f.write_c(" nullptr,");

  // Write user_data or nullptr
  if (!user_data().empty())
    f.write_c(" (void*)(" + user_data() + "),");
  else
    f.write_c(" nullptr,");

  // Write flags, labeltype, labelfont, labelsize, and labelcolor
  f.write_c(" " + std::to_string(flags()) + ", (uchar)" + labeltypes[o->labeltype()] + ", "
           + std::to_string(o->labelfont()) + ", " + std::to_string(o->labelsize()) + ", " + std::to_string(o->labelcolor()) + " ");
  f.write_c("},\n");
}

void start_menu_initialiser(fluid::io::Code_Writer& f, int &initialized, const char *name, int index) {
  if (!initialized) {
    initialized = 1;
    f.write_c(f.indent() + "{ Fl_Menu_Item* o = &" + std::string(name) + "[" + std::to_string(index) + "];\n");
    f.indent_more();
  }
}

void Menu_Item_Node::write_code1(fluid::io::Code_Writer& f) {
  int i;
  std::string mname = menu_name(f, i);

  if (!dynamic_cast<Menu_Item_Node*>(prev)) {
    // for first menu item, declare the array
    if (class_name(1)) {
      f.write_h(f.indent(1) + "static Fl_Menu_Item " + mname + "[];\n");
    } else {
      f.write_h("extern Fl_Menu_Item " + mname + "[];\n");
    }
  }

  const char *c = array_name(this);
  if (c) {
    if (class_name(1)) {
      f.write_public(public_);
      f.write_h(f.indent(1) + "static Fl_Menu_Item* " + std::string(c) + ";\n");
    } else {
      if (c==name())
        f.write_h("#define " + std::string(c) + " (" + mname + "+" + std::to_string(i) + ")\n");
      else
        f.write_h("extern Fl_Menu_Item* " + std::string(c) + ";\n");
    }
  }

  if (callback()) {
    if (!is_function_name(callback()) && !is_lambda(callback()) && class_name(1)) {
      std::string cn = callback_name(f);
      std::string ut = user_data_type_or_voidp();
      f.write_public(0);
      f.write_h(f.indent(1) + "inline void " + cn + "_i(Fl_Menu_*, " + ut + ");\n");
      f.write_h(f.indent(1) + "static void " + cn + "(Fl_Menu_*, " + ut + ");\n");
    }
  }

  int menuItemInitialized = 0;
  // if the name is an array variable, assign the value here
  if (name() && strchr(name(), '[')) {
    f.write_c(f.indent_plus(1) + std::string(name()) + " = &" + mname + "[" + std::to_string(i) + "];\n");
  }
  if (active_image.asset) {
    start_menu_initialiser(f, menuItemInitialized, mname.c_str(), i);
    if (label() && label()[0]) {
      f.write_c(f.indent() + "Fl_Multi_Label* ml = new Fl_Multi_Label;\n");
      f.write_c(f.indent() + "ml->labela = (char*)");
      active_image.asset->write_inline(f);
      f.write_c(";\n");
      if (Fluid.proj.i18n.type==fluid::I18n_Type::NONE) {
        f.write_c(f.indent() + "ml->labelb = o->label();\n");
      } else if (Fluid.proj.i18n.type==fluid::I18n_Type::GNU) {
        f.write_c(f.indent() + "ml->labelb = " + Fluid.proj.i18n.gnu_function + "(o->label());\n");
      } else if (Fluid.proj.i18n.type==fluid::I18n_Type::POSIX) {
        f.write_c(f.indent() + "ml->labelb = catgets(" +
                  (Fluid.proj.i18n.posix_file.empty() ? "_catalog" : Fluid.proj.i18n.posix_file) + "," +
                  Fluid.proj.i18n.posix_set + "," + std::to_string(msgnum()) + ",o->label());\n");
      }
      f.write_c(f.indent() + "ml->typea = FL_IMAGE_LABEL;\n");
      f.write_c(f.indent() + "ml->typeb = FL_NORMAL_LABEL;\n");
      f.write_c(f.indent() + "ml->label(o);\n");
    } else {
      active_image.asset->write_code(f, 0, "o");
    }
  }
  if ((Fluid.proj.i18n.type != fluid::I18n_Type::NONE) && label() && label()[0]) {
    Fl_Labeltype t = o->labeltype();
    if (active_image.asset) {
      // label was already copied a few lines up
    } else if (   t==FL_NORMAL_LABEL   || t==FL_SHADOW_LABEL
               || t==FL_ENGRAVED_LABEL || t==FL_EMBOSSED_LABEL) {
      start_menu_initialiser(f, menuItemInitialized, mname.c_str(), i);
      if (Fluid.proj.i18n.type==fluid::I18n_Type::GNU) {
        f.write_c(f.indent() + "o->label(" + Fluid.proj.i18n.gnu_function + "(o->label()));\n");
      } else if (Fluid.proj.i18n.type==fluid::I18n_Type::POSIX) {
        f.write_c(f.indent() + "o->label(catgets(" +
                  (Fluid.proj.i18n.posix_file.empty() ? "_catalog" : Fluid.proj.i18n.posix_file) + "," +
                  Fluid.proj.i18n.posix_set + "," + std::to_string(msgnum()) + ",o->label()));\n");
      }
    }
  }
  if (!extra_code(2).empty()) {
    start_menu_initialiser(f, menuItemInitialized, mname.c_str(), i);
    f.write_c_indented(extra_code(2), 0, '\n');
  }
  if (!extra_code(3).empty()) {
    start_menu_initialiser(f, menuItemInitialized, mname.c_str(), i);
    f.write_c_indented(extra_code(3), 0, '\n');
  }
  if (menuItemInitialized) {
    f.indent_less();
    f.write_c(f.indent() + "}\n");
  }
}

void Menu_Item_Node::write_code2(fluid::io::Code_Writer&) {}

////////////////////////////////////////////////////////////////
// This is the base class for widgets that contain a menu (ie
// subclasses of Fl_Menu_.
// This is a parent widget and menu items can be added as
// children.  An actual array of Fl_Menu_Items is kept parallel
// with the child objects and updated as they change.

void Menu_Base_Node::build_menu() {
  Fl_Menu_* w = (Fl_Menu_*)o;
  // count how many Fl_Menu_Item structures needed:
  int n = 0;
  Node* q;
  for (q = next; q && q->level > level; q = q->next) {
    if (q->can_have_children()) n++; // space for null at end of submenu
    n++;
  }
  if (!n) {
    if (menusize) delete_menu((Fl_Menu_Item*)(w->menu()));
    w->menu(nullptr);
    menusize = 0;
  } else {
    n++; // space for null at end of menu
    if (menusize<n) {
      if (menusize) delete_menu((Fl_Menu_Item*)(w->menu()));
      menusize = n+10;
      w->menu(new Fl_Menu_Item[menusize]);
    } else {
      if (menusize) delete_dependents((Fl_Menu_Item*)(w->menu()));
    }
    // Menus are already built during the .fl file reading process, so if the
    // end of a menu list is not read yet, the end markers (label==nullptr) will
    // not be set, and deleting dependents will randomly free memory.
    // Clearing the array should avoid that.
    memset( (void*)w->menu(), 0, menusize * sizeof(Fl_Menu_Item) );
    // fill them all in:
    Fl_Menu_Item* m = (Fl_Menu_Item*)(w->menu());
    int lvl = level+1;
    for (q = next; q && q->level > level; q = q->next) {
      Menu_Item_Node* i = (Menu_Item_Node*)q;
      if (i->o->image()) {
        if (i->o->label() && i->o->label()[0]) {
          Fl_Multi_Label *ml = new Fl_Multi_Label;
          ml->labela = (char*)i->o->image();
          ml->labelb = i->o->label();
          ml->typea = FL_IMAGE_LABEL;
          ml->typeb = FL_NORMAL_LABEL;
          ml->label(m);
        } else {
          i->o->image()->label(m);
        }
      } else {
        m->label(i->o->label() ? i->o->label() : "(nolabel)");
        m->labeltype(i->o->labeltype());
      }
      m->shortcut(((Fl_Button*)(i->o))->shortcut());
      m->callback(nullptr,(void*)i);
      m->flags = (i->flags() | i->o->type())  & ~FL_MENU_HEADLINE;
      m->labelfont(i->o->labelfont());
      m->labelsize(i->o->labelsize());
      m->labelcolor(i->o->labelcolor());
      if (q->can_have_children()) {lvl++; m->flags |= FL_SUBMENU;}
      m++;
      int l1 =
        (q->next && dynamic_cast<Menu_Item_Node*>(q->next)) ? q->next->level : level;
      while (lvl > l1) {m->label(nullptr); m++; lvl--;}
      lvl = l1;
    }
  }
  o->redraw();
}

Node* Menu_Base_Node::click_test(int, int) {
  if (selected) return nullptr; // let user move the widget
  Fl_Menu_* w = (Fl_Menu_*)o;
  if (!menusize) return nullptr;
  const Fl_Menu_Item* save = w->mvalue();
  w->value((Fl_Menu_Item*)nullptr);
  Fl::pushed(w);
  w->handle(FL_PUSH);
  Fl::focus(nullptr);
  const Fl_Menu_Item* m = w->mvalue();
  if (m) {
    // restore the settings of toggles & radio items:
    if (m->flags & (FL_MENU_RADIO | FL_MENU_TOGGLE)) build_menu();
    return (Node*)(m->user_data());
  }
  w->value(save);
  return this;
}

void Menu_Manager_Node::write_code2(fluid::io::Code_Writer& f) {
  if (next && dynamic_cast<Menu_Item_Node*>(next)) {
    f.write_c(f.indent() + (name() ? name() : "o") + "->menu(" +
            f.unique_id(this, "menu", (name()?name():""), (label()?label():"")) + ");\n");
  }
  Widget_Node::write_code2(f);
}

void Menu_Base_Node::copy_properties() {
  Widget_Node::copy_properties();
  Fl_Menu_ *s = (Fl_Menu_*)o, *d = (Fl_Menu_*)live_widget;
  d->menu(s->menu());
  d->down_box(s->down_box());
  d->textcolor(s->textcolor());
  d->textfont(s->textfont());
  d->textsize(s->textsize());
}

////////////////////////////////////////////////////////////////

Fl_Menu_Item button_type_menu[] = {
  {"normal",0,nullptr,(void*)nullptr},
  {"popup1",0,nullptr,(void*)Fl_Menu_Button::POPUP1},
  {"popup2",0,nullptr,(void*)Fl_Menu_Button::POPUP2},
  {"popup3",0,nullptr,(void*)Fl_Menu_Button::POPUP3},
  {"popup12",0,nullptr,(void*)Fl_Menu_Button::POPUP12},
  {"popup23",0,nullptr,(void*)Fl_Menu_Button::POPUP23},
  {"popup13",0,nullptr,(void*)Fl_Menu_Button::POPUP13},
  {"popup123",0,nullptr,(void*)Fl_Menu_Button::POPUP123},
  {nullptr}};

Menu_Button_Node Menu_Button_Node::prototype;

////////////////////////////////////////////////////////////////

Fl_Menu_Item dummymenu[] = {{"CHOICE"},{nullptr}};

Choice_Node Choice_Node::prototype;

Input_Choice_Node Input_Choice_Node::prototype;

void Input_Choice_Node::copy_properties() {
  Widget_Node::copy_properties();
  Fl_Input_Choice *s = (Fl_Input_Choice*)o, *d = (Fl_Input_Choice*)live_widget;
  d->menu(s->menu());
  d->down_box(s->down_box());
  d->textcolor(s->textcolor());
  d->textfont(s->textfont());
  d->textsize(s->textsize());
}

Node* Input_Choice_Node::click_test(int, int) {
  if (selected) return nullptr; // let user move the widget
  Fl_Menu_* w = ((Fl_Input_Choice*)o)->menubutton();
  if (!menusize) return nullptr;
  const Fl_Menu_Item* save = w->mvalue();
  w->value((Fl_Menu_Item*)nullptr);
  Fl::pushed(w);
  w->handle(FL_PUSH);
  Fl::focus(nullptr);
  const Fl_Menu_Item* m = w->mvalue();
  if (m) {
    // restore the settings of toggles & radio items:
    if (m->flags & (FL_MENU_RADIO | FL_MENU_TOGGLE)) build_menu();
    return (Node*)(m->user_data());
  }
  w->value(save);
  return this;
}

////////////////////////////////////////////////////////////////

Menu_Bar_Node Menu_Bar_Node::prototype;

Fl_Menu_Item menu_bar_type_menu[] = {
  {"Fl_Menu_Bar",0,nullptr,(void*)nullptr},
  {"Fl_Sys_Menu_Bar",0,nullptr,(void*)1},
  {nullptr}};

Menu_Bar_Node::Menu_Bar_Node()
{
}

Menu_Bar_Node::~Menu_Bar_Node()
{
}

/**
 \brief Return true if this is an Fl_Sys_Menu_Bar.
 This test fails if subclass() is the name of a class that the user may have
 derived from Fl_Sys_Menu_Bar.
 */
bool Menu_Bar_Node::is_sys_menu_bar() {
  if (o->type()==1) return true;
  return (subclass() == "Fl_Sys_Menu_Bar");
}

std::string Menu_Bar_Node::sys_menubar_name() const {
  if (!subclass().empty())
    return subclass();
  else
    return "Fl_Sys_Menu_Bar";
}

std::string Menu_Bar_Node::sys_menubar_proxy_name() const {
  return sys_menubar_name() + "_Proxy";
}


void Menu_Bar_Node::write_static(fluid::io::Code_Writer& f) {
  super::write_static(f);
  if (is_sys_menu_bar()) {
    f.write_h_once("#include <FL/Fl_Sys_Menu_Bar.H>");
    if (is_in_class()) {
      // Make room for a pointer to the enclosing class.
      f.write_c_once( // must be less than 1024 bytes!
                     "\nclass " + sys_menubar_proxy_name() + ": public " + sys_menubar_name() + " {\n"
                     "public:\n"
                     "  " + sys_menubar_proxy_name() + "(int x, int y, int w, int h, const char* l=nullptr)\n"
                     "  : " + sys_menubar_name() + "(x, y, w, h, l) { }\n"
                     "  void* _parent_class;\n"
                     "};\n");
    }
  }
}

void Menu_Bar_Node::write_code1(fluid::io::Code_Writer& f) {
  super::write_code1(f);
  if (is_sys_menu_bar() && is_in_class()) {
    f.write_c(f.indent() + "((" + sys_menubar_proxy_name() + "*)" + (name() ? name() : "o") + ")->_parent_class = (void*)this;\n");
  }
}

//void Menu_Bar_Node::write_code2(fluid::io::Code_Writer& f) {
//  super::write_code2(f);
//}

