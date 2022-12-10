//
// Menu item code for the Fast Light Tool Kit (FLTK).
//
// Menu items are kludged by making a phony Fl_Box widget so the normal
// widget panel can be used to control them.
//
// This file also contains code to make Fl_Menu_Button, Fl_Menu_Bar,
// etc widgets.
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

#include "Fl_Menu_Type.h"

#include "fluid.h"
#include "Fl_Window_Type.h"
#include "alignment_panel.h"
#include "file.h"
#include "code.h"
#include "Fluid_Image.h"
#include "Shortcut_Button.h"

#include <FL/Fl.H>
#include <FL/fl_message.H>
#include <FL/Fl_Menu_.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Output.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Multi_Label.H>
#include "../src/flstring.h"

#include <stdio.h>
#include <stdlib.h>

Fl_Menu_Item menu_item_type_menu[] = {
  {"Normal",0,0,(void*)0},
  {"Toggle",0,0,(void*)FL_MENU_BOX},
  {"Radio",0,0,(void*)FL_MENU_RADIO},
  {0}};

static char submenuflag;
static uchar menuitemtype = 0;

static void delete_dependents(Fl_Menu_Item *m) {
  if (!m)
    return;
  int level = 0;
  for (;;m++) {
    if (m->label()==NULL) {
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

void Fl_Input_Choice_Type::build_menu() {
  Fl_Input_Choice* w = (Fl_Input_Choice*)o;
  // count how many Fl_Menu_Item structures needed:
  int n = 0;
  Fl_Type* q;
  for (q = next; q && q->level > level; q = q->next) {
    if (q->is_parent()) n++; // space for null at end of submenu
    n++;
  }
  if (!n) {
    if (menusize) delete_menu((Fl_Menu_Item*)(w->menu()));
    w->menu(0);
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
    // end of a menu list is not read yet, the end markers (label==NULL) will
    // not be set, and deleting dependants will randomly free memory.
    // Clearing the array should avoid that.
    memset( (void*)w->menu(), 0, menusize * sizeof(Fl_Menu_Item) );
    // fill them all in:
    Fl_Menu_Item* m = (Fl_Menu_Item*)(w->menu());
    int lvl = level+1;
    for (q = next; q && q->level > level; q = q->next) {
      Fl_Menu_Item_Type* i = (Fl_Menu_Item_Type*)q;
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
      m->callback(0,(void*)i);
      m->flags = i->flags();
      m->labelfont(i->o->labelfont());
      m->labelsize(i->o->labelsize());
      m->labelcolor(i->o->labelcolor());
      if (q->is_parent()) {lvl++; m->flags |= FL_SUBMENU;}
      m++;
      int l1 =
        (q->next && q->next->is_menu_item()) ? q->next->level : level;
      while (lvl > l1) {m->label(0); m++; lvl--;}
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
Fl_Type *Fl_Menu_Item_Type::make(Strategy strategy) {
  // Find the current menu item:
  Fl_Type* q = Fl_Type::current;
  Fl_Type* p = q;
  if (p) {
    if ( (force_parent && q->is_menu_item()) || !q->is_parent()) p = p->parent;
  }
  force_parent = 0;
  if (!p || !(p->is_menu_button() || (p->is_menu_item() && p->is_parent()))) {
    fl_message("Please select a menu to add to");
    return 0;
  }
  if (!o) {
    o = new Fl_Button(0,0,100,20); // create template widget
    o->labelsize(Fl_Widget_Type::default_size);
  }

  Fl_Menu_Item_Type* t = submenuflag ? new Fl_Submenu_Type() : new Fl_Menu_Item_Type();
  t->o = new Fl_Button(0,0,100,20);
  t->o->type(menuitemtype);
  t->factory = this;
  t->add(p, strategy);
  if (!reading_file) t->label(submenuflag ? "submenu" : "item");
  return t;
}

/**
 Create and add a new Checkbox Menu Item node.
 \param[in] strategy add after current or as last child
 \return new node
 */
Fl_Type *Fl_Checkbox_Menu_Item_Type::make(Strategy strategy) {
    menuitemtype = FL_MENU_TOGGLE;
    Fl_Type* t = Fl_Menu_Item_Type::make(strategy);
    menuitemtype = 0;
    return t;
}

/**
 Create and add a new Radio ButtonMenu Item node.
 \param[in] strategy add after current or as last child
 \return new node
 */
Fl_Type *Fl_Radio_Menu_Item_Type::make(Strategy strategy) {
    menuitemtype = FL_MENU_RADIO;
    Fl_Type* t = Fl_Menu_Item_Type::make(strategy);
    menuitemtype = 0;
    return t;
}

/**
 Create and add a new Submenu Item node.
 \param[in] strategy add after current or as last child
 \return new node
 */
Fl_Type *Fl_Submenu_Type::make(Strategy strategy) {
  submenuflag = 1;
  Fl_Type* t = Fl_Menu_Item_Type::make(strategy);
  submenuflag = 0;
  return t;
}

Fl_Menu_Item_Type Fl_Menu_Item_type;
Fl_Checkbox_Menu_Item_Type Fl_Checkbox_Menu_Item_type;
Fl_Radio_Menu_Item_Type Fl_Radio_Menu_Item_type;
Fl_Submenu_Type Fl_Submenu_type;

////////////////////////////////////////////////////////////////
// Writing the C code:

// test functions in Fl_Widget_Type.C:
int is_name(const char *c);
const char *array_name(Fl_Widget_Type *o);
int isdeclare(const char *c);

// Search backwards to find the parent menu button and return it's name.
// Also put in i the index into the button's menu item array belonging
// to this menu item.
const char* Fl_Menu_Item_Type::menu_name(int& i) {
  i = 0;
  Fl_Type* t = prev;
  while (t && t->is_menu_item()) {
    // be sure to count the {0} that ends a submenu:
    if (t->level > t->next->level) i += (t->level - t->next->level);
    // detect empty submenu:
    else if (t->level == t->next->level && t->is_parent()) i++;
    t = t->prev;
    i++;
  }
  return unique_id(t, "menu", t->name(), t->label());
}

void Fl_Menu_Item_Type::write_static() {
  if (image && label() && label()[0]) {
    write_declare("#include <FL/Fl.H>");
    write_declare("#include <FL/Fl_Multi_Label.H>");
  }
  if (callback() && is_name(callback()) && !user_defined(callback()))
    write_declare("extern void %s(Fl_Menu_*, %s);", callback(),
                  user_data_type() ? user_data_type() : "void*");
  for (int n=0; n < NUM_EXTRA_CODE; n++) {
    if (extra_code(n) && isdeclare(extra_code(n)))
      write_declare("%s", extra_code(n));
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
    const char* cn = callback_name();
    const char* k = class_name(1);
    if (k) {
      write_c("\nvoid %s::%s_i(Fl_Menu_*", k, cn);
    } else {
      write_c("\nstatic void %s(Fl_Menu_*", cn);
    }
    if (use_o) write_c(" o");
    const char* ut = user_data_type() ? user_data_type() : "void*";
    write_c(", %s", ut);
    if (use_v) write_c(" v");
    write_c(") {\n");
    write_c_indented(callback(), 1, 0);
    if (*(d-1) != ';' && *(d-1) != '}') {
      const char *p = strrchr(callback(), '\n');
      if (p) p ++;
      else p = callback();
      // Only add trailing semicolon if the last line is not a preprocessor
      // statement...
      if (*p != '#' && *p) write_c(";");
    }
    write_c("\n}\n");
    if (k) {
      write_c("void %s::%s(Fl_Menu_* o, %s v) {\n", k, cn, ut);
      write_c("%s((%s*)(o", indent(1), k);
      Fl_Type* t = parent; while (t->is_menu_item()) t = t->parent;
      Fl_Type *q = 0;
      // Go up one more level for Fl_Input_Choice, as these are groups themselves
      if (t && !strcmp(t->type_name(), "Fl_Input_Choice"))
        write_c("->parent()");
      for (t = t->parent; t && t->is_widget() && !is_class(); q = t, t = t->parent)
        write_c("->parent()");
      if (!q || strcmp(q->type_name(), "widget_class"))
        write_c("->user_data()");
      write_c("))->%s_i(o,v);\n}\n", cn);
    }
  }
  if (image) {
    if (image->written != write_number) {
      image->write_static();
      image->written = write_number;
    }
  }
  if (next && next->is_menu_item()) return;
  // okay, when we hit last item in the menu we have to write the
  // entire array out:
  const char* k = class_name(1);
  if (k) {
    int i;
    write_c("\nFl_Menu_Item %s::%s[] = {\n", k, menu_name(i));
  } else {
    int i;
    write_c("\nFl_Menu_Item %s[] = {\n", menu_name(i));
  }
  Fl_Type* t = prev; while (t && t->is_menu_item()) t = t->prev;
  for (Fl_Type* q = t->next; q && q->is_menu_item(); q = q->next) {
    ((Fl_Menu_Item_Type*)q)->write_item();
    int thislevel = q->level; if (q->is_parent()) thislevel++;
    int nextlevel =
      (q->next && q->next->is_menu_item()) ? q->next->level : t->level+1;
    while (thislevel > nextlevel) {write_c(" {0,0,0,0,0,0,0,0,0},\n"); thislevel--;}
  }
  write_c(" {0,0,0,0,0,0,0,0,0}\n};\n");

  if (k) {
    // Write menu item variables...
    t = prev; while (t && t->is_menu_item()) t = t->prev;
    for (Fl_Type* q = t->next; q && q->is_menu_item(); q = q->next) {
      Fl_Menu_Item_Type *m = (Fl_Menu_Item_Type*)q;
      const char *c = array_name(m);
      if (c) {
        if (c==m->name()) {
          // assign a menu item address directly to a variable
          int i;
          const char* n = ((Fl_Menu_Item_Type *)q)->menu_name(i);
          write_c("Fl_Menu_Item* %s::%s = %s::%s + %d;\n", k, c, k, n, i);
        } else {
          // if the name is an array, only define the array.
          // The actual assignment is in write_code1()
          write_c("Fl_Menu_Item* %s::%s;\n", k, c);
        }
      }
    }
  }
}

int Fl_Menu_Item_Type::flags() {
  int i = o->type();
  if (((Fl_Button*)o)->value()) i |= FL_MENU_VALUE;
  if (!o->active()) i |= FL_MENU_INACTIVE;
  if (!o->visible()) i |= FL_MENU_INVISIBLE;
  if (is_parent()) {
    if (user_data() == NULL) i |= FL_SUBMENU;
    else i |= FL_SUBMENU_POINTER;
  }
  if (hotspot()) i |= FL_MENU_DIVIDER;
  return i;
}

void Fl_Menu_Item_Type::write_item() {
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

  write_comment_inline_c(" ");
  write_c(" {");
  if (label() && label()[0])
    switch (P.i18n_type) {
      case 1:
        // we will call i18n when the menu is instantiated for the first time
        write_c("%s(", P.i18n_static_function.value());
        write_cstring(label());
        write_c(")");
        break;
      case 2:
        // fall through: strings can't be translated before a catalog is choosen
      default:
        write_cstring(label());
    }
  else
    write_c("\"\"");
  if (((Fl_Button*)o)->shortcut()) {
                int s = ((Fl_Button*)o)->shortcut();
                if (P.use_FL_COMMAND && (s & (FL_CTRL|FL_META))) {
                        write_c(", FL_COMMAND|0x%x, ", s & ~(FL_CTRL|FL_META));
                } else {
                        write_c(", 0x%x, ", s);
                }
  } else
    write_c(", 0, ");
  if (callback()) {
    const char* k = is_name(callback()) ? 0 : class_name(1);
    if (k) {
      write_c(" (Fl_Callback*)%s::%s,", k, callback_name());
    } else {
      write_c(" (Fl_Callback*)%s,", callback_name());
    }
  } else
    write_c(" 0,");
  if (user_data())
    write_c(" (void*)(%s),", user_data());
  else
    write_c(" 0,");
  write_c(" %d, (uchar)%s, %d, %d, %d", flags(),
          labeltypes[o->labeltype()], o->labelfont(), o->labelsize(), o->labelcolor());
  write_c("},\n");
}

void start_menu_initialiser(int &initialized, const char *name, int index) {
  if (!initialized) {
    initialized = 1;
    write_c("%s{ Fl_Menu_Item* o = &%s[%d];\n", indent(), name, index);
    indentation++;
  }
}

void Fl_Menu_Item_Type::write_code1() {
  int i; const char* mname = menu_name(i);

  if (!prev->is_menu_item()) {
    // for first menu item, declare the array
    if (class_name(1)) {
      write_h("%sstatic Fl_Menu_Item %s[];\n", indent(1), mname);
    } else {
      write_h("extern Fl_Menu_Item %s[];\n", mname);
    }
  }

  const char *c = array_name(this);
  if (c) {
    if (class_name(1)) {
      write_public(public_);
      write_h("%sstatic Fl_Menu_Item *%s;\n", indent(1), c);
    } else {
      if (c==name())
        write_h("#define %s (%s+%d)\n", c, mname, i);
      else
        write_h("extern Fl_Menu_Item *%s;\n", c);
    }
  }

  if (callback()) {
    if (!is_name(callback()) && class_name(1)) {
      const char* cn = callback_name();
      const char* ut = user_data_type() ? user_data_type() : "void*";
      write_public(0);
      write_h("%sinline void %s_i(Fl_Menu_*, %s);\n", indent(1), cn, ut);
      write_h("%sstatic void %s(Fl_Menu_*, %s);\n", indent(1), cn, ut);
    }
  }

  int menuItemInitialized = 0;
  // if the name is an array variable, assign the value here
  if (name() && strchr(name(), '[')) {
    write_c("%s%s = &%s[%d];\n", indent_plus(1), name(), mname, i);
  }
  if (image) {
    start_menu_initialiser(menuItemInitialized, mname, i);
    if (label() && label()[0]) {
      write_c("%sFl_Multi_Label *ml = new Fl_Multi_Label;\n", indent());
      write_c("%sml->labela = (char*)", indent());
      image->write_inline();
      write_c(";\n");
      if (P.i18n_type==0) {
        write_c("%sml->labelb = o->label();\n", indent());
      } else if (P.i18n_type==1) {
        write_c("%sml->labelb = %s(o->label());\n",
                indent(), P.i18n_function.value());
      } else if (P.i18n_type==2) {
        write_c("%sml->labelb = catgets(%s,%s,i+%d,o->label());\n",
                indent(), P.i18n_file[0] ? P.i18n_file.value() : "_catalog",
                P.i18n_set.value(), msgnum());
      }
      write_c("%sml->typea = FL_IMAGE_LABEL;\n", indent());
      write_c("%sml->typeb = FL_NORMAL_LABEL;\n", indent());
      write_c("%sml->label(o);\n", indent());
    } else {
      image->write_code(0, "o");
    }
  }
  if (P.i18n_type && label() && label()[0]) {
    Fl_Labeltype t = o->labeltype();
    if (image) {
      // label was already copied a few lines up
    } else if (   t==FL_NORMAL_LABEL   || t==FL_SHADOW_LABEL
               || t==FL_ENGRAVED_LABEL || t==FL_EMBOSSED_LABEL) {
      start_menu_initialiser(menuItemInitialized, mname, i);
      if (P.i18n_type==1) {
        write_c("%so->label(%s(o->label()));\n",
                indent(), P.i18n_function.value());
      } else if (P.i18n_type==2) {
        write_c("%so->label(catgets(%s,%s,i+%d,o->label()));\n",
                indent(), P.i18n_file[0] ? P.i18n_file.value() : "_catalog",
                P.i18n_set.value(), msgnum());
      }
    }
  }
  for (int n=0; n < NUM_EXTRA_CODE; n++) {
    if (extra_code(n) && !isdeclare(extra_code(n))) {
      start_menu_initialiser(menuItemInitialized, mname, i);
      write_c("%s%s\n", indent(), extra_code(n));
    }
  }
  if (menuItemInitialized) {
    indentation--;
    write_c("%s}\n",indent());
  }
}

void Fl_Menu_Item_Type::write_code2() {}

////////////////////////////////////////////////////////////////
// This is the base class for widgets that contain a menu (ie
// subclasses of Fl_Menu_.
// This is a parent widget and menu items can be added as
// children.  An actual array of Fl_Menu_Items is kept parallel
// with the child objects and updated as they change.

void Fl_Menu_Type::build_menu() {
  Fl_Menu_* w = (Fl_Menu_*)o;
  // count how many Fl_Menu_Item structures needed:
  int n = 0;
  Fl_Type* q;
  for (q = next; q && q->level > level; q = q->next) {
    if (q->is_parent()) n++; // space for null at end of submenu
    n++;
  }
  if (!n) {
    if (menusize) delete_menu((Fl_Menu_Item*)(w->menu()));
    w->menu(0);
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
    // end of a menu list is not read yet, the end markers (label==NULL) will
    // not be set, and deleting dependants will randomly free memory.
    // Clearing the array should avoid that.
    memset( (void*)w->menu(), 0, menusize * sizeof(Fl_Menu_Item) );
    // fill them all in:
    Fl_Menu_Item* m = (Fl_Menu_Item*)(w->menu());
    int lvl = level+1;
    for (q = next; q && q->level > level; q = q->next) {
      Fl_Menu_Item_Type* i = (Fl_Menu_Item_Type*)q;
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
      m->callback(0,(void*)i);
      m->flags = i->flags() | i->o->type();
      m->labelfont(i->o->labelfont());
      m->labelsize(i->o->labelsize());
      m->labelcolor(i->o->labelcolor());
      if (q->is_parent()) {lvl++; m->flags |= FL_SUBMENU;}
      m++;
      int l1 =
        (q->next && q->next->is_menu_item()) ? q->next->level : level;
      while (lvl > l1) {m->label(0); m++; lvl--;}
      lvl = l1;
    }
  }
  o->redraw();
}

Fl_Type* Fl_Menu_Type::click_test(int, int) {
  if (selected) return 0; // let user move the widget
  Fl_Menu_* w = (Fl_Menu_*)o;
  if (!menusize) return 0;
  const Fl_Menu_Item* save = w->mvalue();
  w->value((Fl_Menu_Item*)0);
  Fl::pushed(w);
  w->handle(FL_PUSH);
  Fl::focus(NULL);
  const Fl_Menu_Item* m = w->mvalue();
  if (m) {
    // restore the settings of toggles & radio items:
    if (m->flags & (FL_MENU_RADIO | FL_MENU_TOGGLE)) build_menu();
    return (Fl_Type*)(m->user_data());
  }
  w->value(save);
  return this;
}

void Fl_Menu_Type::write_code2() {
  if (next && next->is_menu_item()) {
    write_c("%s%s->menu(%s);\n", indent(), name() ? name() : "o",
            unique_id(this, "menu", name(), label()));
  }
  Fl_Widget_Type::write_code2();
}

void Fl_Menu_Type::copy_properties() {
  Fl_Widget_Type::copy_properties();
  Fl_Menu_ *s = (Fl_Menu_*)o, *d = (Fl_Menu_*)live_widget;
  d->menu(s->menu());
  d->down_box(s->down_box());
  d->textcolor(s->textcolor());
  d->textfont(s->textfont());
  d->textsize(s->textsize());
}

////////////////////////////////////////////////////////////////

Fl_Menu_Item button_type_menu[] = {
  {"normal",0,0,(void*)0},
  {"popup1",0,0,(void*)Fl_Menu_Button::POPUP1},
  {"popup2",0,0,(void*)Fl_Menu_Button::POPUP2},
  {"popup3",0,0,(void*)Fl_Menu_Button::POPUP3},
  {"popup12",0,0,(void*)Fl_Menu_Button::POPUP12},
  {"popup23",0,0,(void*)Fl_Menu_Button::POPUP23},
  {"popup13",0,0,(void*)Fl_Menu_Button::POPUP13},
  {"popup123",0,0,(void*)Fl_Menu_Button::POPUP123},
  {0}};

Fl_Menu_Button_Type Fl_Menu_Button_type;

////////////////////////////////////////////////////////////////

Fl_Menu_Item dummymenu[] = {{"CHOICE"},{0}};

Fl_Choice_Type Fl_Choice_type;

Fl_Input_Choice_Type Fl_Input_Choice_type;

void Fl_Input_Choice_Type::copy_properties() {
  Fl_Widget_Type::copy_properties();
  Fl_Input_Choice *s = (Fl_Input_Choice*)o, *d = (Fl_Input_Choice*)live_widget;
  d->menu(s->menu());
  d->down_box(s->down_box());
  d->textcolor(s->textcolor());
  d->textfont(s->textfont());
  d->textsize(s->textsize());
}

Fl_Type* Fl_Input_Choice_Type::click_test(int, int) {
  if (selected) return 0; // let user move the widget
  Fl_Menu_* w = ((Fl_Input_Choice*)o)->menubutton();
  if (!menusize) return 0;
  const Fl_Menu_Item* save = w->mvalue();
  w->value((Fl_Menu_Item*)0);
  Fl::pushed(w);
  w->handle(FL_PUSH);
  Fl::focus(NULL);
  const Fl_Menu_Item* m = w->mvalue();
  if (m) {
    // restore the settings of toggles & radio items:
    if (m->flags & (FL_MENU_RADIO | FL_MENU_TOGGLE)) build_menu();
    return (Fl_Type*)(m->user_data());
  }
  w->value(save);
  return this;
}

////////////////////////////////////////////////////////////////

Fl_Menu_Bar_Type Fl_Menu_Bar_type;

////////////////////////////////////////////////////////////////
// Shortcut entry item in panel:


void shortcut_in_cb(Shortcut_Button* i, void* v) {
  if (v == LOAD) {
    if (current_widget->is_button())
      i->svalue = ((Fl_Button*)(current_widget->o))->shortcut();
    else if (current_widget->is_input())
      i->svalue = ((Fl_Input_*)(current_widget->o))->shortcut();
    else if (current_widget->is_value_input())
      i->svalue = ((Fl_Value_Input*)(current_widget->o))->shortcut();
    else if (current_widget->is_text_display())
      i->svalue = ((Fl_Text_Display*)(current_widget->o))->shortcut();
    else {
      i->hide();
      return;
    }
    i->show();
    i->redraw();
  } else {
    int mod = 0;
    for (Fl_Type *o = Fl_Type::first; o; o = o->next)
      if (o->selected && o->is_button()) {
        Fl_Button* b = (Fl_Button*)(((Fl_Widget_Type*)o)->o);
        if (b->shortcut()!=i->svalue) mod = 1;
        b->shortcut(i->svalue);
        if (o->is_menu_item()) ((Fl_Widget_Type*)o)->redraw();
      } else if (o->selected && o->is_input()) {
        Fl_Input_* b = (Fl_Input_*)(((Fl_Widget_Type*)o)->o);
        if (b->shortcut()!=i->svalue) mod = 1;
        b->shortcut(i->svalue);
      } else if (o->selected && o->is_value_input()) {
        Fl_Value_Input* b = (Fl_Value_Input*)(((Fl_Widget_Type*)o)->o);
        if (b->shortcut()!=i->svalue) mod = 1;
        b->shortcut(i->svalue);
      } else if (o->selected && o->is_text_display()) {
        Fl_Text_Display* b = (Fl_Text_Display*)(((Fl_Widget_Type*)o)->o);
        if (b->shortcut()!=i->svalue) mod = 1;
        b->shortcut(i->svalue);
      }
    if (mod) set_modflag(1);
  }
}
