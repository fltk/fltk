//
// "$Id: Fl_Menu_Type.cxx,v 1.16.2.12 2001/01/22 15:13:38 easysw Exp $"
//
// Menu item code for the Fast Light Tool Kit (FLTK).
//
// Menu items are kludged by making a phony Fl_Box widget so the normal
// widget panel can be used to control them.
//
// This file also contains code to make Fl_Menu_Button, Fl_Menu_Bar,
// etc widgets.
//
// Copyright 1998-2001 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to "fltk-bugs@fltk.org".
//

#include <FL/Fl.H>
#include "Fl_Widget_Type.h"
#include "alignment_panel.h"
#include <FL/fl_message.H>
#include <FL/Fl_Menu_.H>
#include <FL/Fl_Button.H>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

Fl_Menu_Item menu_item_type_menu[] = {
  {"Normal",0,0,(void*)0},
  {"Toggle",0,0,(void*)FL_MENU_BOX},
  {"Radio",0,0,(void*)FL_MENU_RADIO},
  {0}};

extern int reading_file;
extern int force_parent;
extern int i18n_type;
extern const char* i18n_include;
extern const char* i18n_function;
extern const char* i18n_file;
extern const char* i18n_set;

static char submenuflag;

Fl_Type *Fl_Menu_Item_Type::make() {
  // Find the current menu item:
  Fl_Type* q = Fl_Type::current;
  Fl_Type* p = q;
  if (p) {
    if (force_parent && q->is_menu_item() || !q->is_parent()) p = p->parent;
  }
  force_parent = 0;
  if (!p || !(p->is_menu_button() || p->is_menu_item() && p->is_parent())) {
    fl_message("Please select a menu to add to");
    return 0;
  }
  if (!o) {
    o = new Fl_Button(0,0,100,20); // create template widget
  }

  Fl_Menu_Item_Type* t = submenuflag ? new Fl_Submenu_Type() : new Fl_Menu_Item_Type();
  t->o = new Fl_Button(0,0,100,20);
  t->factory = this;
  t->add(p);
  if (!reading_file) t->label(submenuflag ? "submenu" : "item");
  return t;
}

Fl_Type *Fl_Submenu_Type::make() {
  submenuflag = 1;
  Fl_Type* t = Fl_Menu_Item_Type::make();
  submenuflag = 0;
  return t;
}

Fl_Menu_Item_Type Fl_Menu_Item_type;
Fl_Submenu_Type Fl_Submenu_type;

////////////////////////////////////////////////////////////////
// Writing the C code:

#include <ctype.h>

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

#include "Fluid_Image.h"

void Fl_Menu_Item_Type::write_static() {
  if (callback() && is_name(callback()))
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
      write_c("\ninline void %s::%s_i(Fl_Menu_*", k, cn);
    } else {
      write_c("\nstatic void %s(Fl_Menu_*", cn);
    }
    if (use_o) write_c(" o");
    const char* ut = user_data_type() ? user_data_type() : "void*";
    write_c(", %s", ut);
    if (use_v) write_c(" v");
    write_c(") {\n  %s", callback());
    if (*(d-1) != ';') write_c(";");
    write_c("\n}\n");
    if (k) {
      write_c("void %s::%s(Fl_Menu_* o, %s v) {\n", k, cn, ut);
      write_c("  ((%s*)(o->", k);
      Fl_Type* t = parent; while (t->is_menu_item()) t = t->parent;
      for (t = t->parent; t->is_widget(); t = t->parent) write_c("parent()->");
      write_c("user_data()))->%s_i(o,v);\n}\n", cn);
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
    int i; write_c("\nFl_Menu_Item %s::%s[] = {\n", k, menu_name(i));
  } else {
    int i; write_c("\nFl_Menu_Item %s[] = {\n", menu_name(i));
  }
  Fl_Type* t = prev; while (t && t->is_menu_item()) t = t->prev;
  for (Fl_Type* q = t->next; q && q->is_menu_item(); q = q->next) {
    ((Fl_Menu_Item_Type*)q)->write_item();
    int thislevel = q->level; if (q->is_parent()) thislevel++;
    int nextlevel =
      (q->next && q->next->is_menu_item()) ? q->next->level : t->level+1;
    while (thislevel > nextlevel) {write_c(" {0},\n"); thislevel--;}
  }
  write_c(" {0}\n};\n");

  if (k) {
    // Write menu item variables...
    t = prev; while (t && t->is_menu_item()) t = t->prev;
    for (Fl_Type* q = t->next; q && q->is_menu_item(); q = q->next) {
      const char *c = array_name((Fl_Menu_Item_Type *)q);
      if (c) {
      int i; const char* n = ((Fl_Menu_Item_Type *)q)->menu_name(i);
      write_c("Fl_Menu_Item* %s::%s = %s::%s + %d;\n", k, c, k, n, i);
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
  write_c(" {");
  if (image) write_c("0");
  else if (label()) {
    switch (i18n_type) {
    case 0 : /* None */
        write_cstring(label());
        break;
    case 1 : /* GNU gettext */
        write_c("%s(", i18n_function);
        write_cstring(label());
	write_c(")");
        break;
    case 2 : /* POSIX catgets */
        write_c("catgets(%s,%s,%d,", i18n_file[0] ? i18n_file : "_catalog",
	        i18n_set, msgnum());
        write_cstring(label());
	write_c(")");
        break;
    }
  }
  else write_c("\"\"");
  if (((Fl_Button*)o)->shortcut())
    write_c(", 0x%x, ", ((Fl_Button*)o)->shortcut());
  else
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
  write_c(" %d, %d, %d, %d, %d", flags(),
	  o->labeltype(), o->labelfont(), o->labelsize(), o->labelcolor());
  write_c("},\n");
}

void Fl_Menu_Item_Type::write_code1() {
  int i; const char* name = menu_name(i);
  if (!prev->is_menu_item()) {
    // for first menu item, declare the array
    if (class_name(1))
      write_h("  static Fl_Menu_Item %s[];\n", name);
    else
      write_h("extern Fl_Menu_Item %s[];\n", name);
  }

  const char *c = array_name(this);
  if (c) {
    if (class_name(1)) {
      write_public(public_);
      write_h("  static Fl_Menu_Item *%s;\n", c);
    } else
      write_h("#define %s (%s+%d)\n", c, name, i);
  }

  if (callback()) {
    if (!is_name(callback()) && class_name(1)) {
      const char* cn = callback_name();
      const char* ut = user_data_type() ? user_data_type() : "void*";
      write_public(0);
      write_h("  inline void %s_i(Fl_Menu_*, %s);\n", cn, ut);
      write_h("  static void %s(Fl_Menu_*, %s);\n", cn, ut);
    }
  }

  int init = 0;
  if (image) {
    write_c(" {Fl_Menu_Item* o = &%s[%d];\n", name, i);
    init = 1;
    image->write_code();
  }
  for (int n=0; n < NUM_EXTRA_CODE; n++)
    if (extra_code(n) && !isdeclare(extra_code(n))) {
      if (!init) {
	init = 1;
	write_c("%s{ Fl_Menu_Item* o = &%s[%d];\n", indent(), name, i);
      }
      write_c("%s  %s\n", indent(), extra_code(n));
    }
  if (init) write_c("%s}\n",indent());
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
    if (menusize) delete[] (Fl_Menu_Item*)(w->menu());
    w->menu(0);
    menusize = 0;
  } else {
    n++; // space for null at end of menu
    if (menusize<n) {
      if (menusize) delete[] (Fl_Menu_Item*)(w->menu());
      menusize = n+10;
      w->menu(new Fl_Menu_Item[menusize]);
    }
    // fill them all in:
    Fl_Menu_Item* m = (Fl_Menu_Item*)(w->menu());
    int lvl = level+1;
    for (q = next; q && q->level > level; q = q->next) {
      Fl_Menu_Item_Type* i = (Fl_Menu_Item_Type*)q;
      m->label(i->o->label());
      m->shortcut(((Fl_Button*)(i->o))->shortcut());
      m->callback(0,(void*)i);
      m->flags = i->flags();
      m->labeltype(i->o->labeltype());
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
  if (next && next->is_menu_item())
    write_c("%so->menu(%s);\n", indent(),
	    unique_id(this, "menu", name(), label()));
  Fl_Widget_Type::write_code2();
}

////////////////////////////////////////////////////////////////

#include <FL/Fl_Menu_Button.H>
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

////////////////////////////////////////////////////////////////

Fl_Menu_Bar_Type Fl_Menu_Bar_type;

////////////////////////////////////////////////////////////////
// Shortcut entry item in panel:

#include <FL/Fl_Output.H>
#include "Shortcut_Button.h"
#include <FL/fl_draw.H>

void Shortcut_Button::draw() {
  if (value()) draw_box(FL_THIN_DOWN_BOX, (Fl_Color)9);
  else draw_box(FL_THIN_UP_BOX, FL_WHITE);
  fl_font(FL_HELVETICA,14); fl_color(FL_BLACK);
  fl_draw(fl_shortcut_label(svalue),x()+6,y(),w(),h(),FL_ALIGN_LEFT);
}

int Shortcut_Button::handle(int e) {
  when(0); type(FL_TOGGLE_BUTTON);
  if (e == FL_KEYBOARD) {
    if (!value()) return 0;
    int v = Fl::event_text()[0];
    if (v > 32 && v < 0x7f || v > 0xa0 && v <= 0xff) {
      v = v | Fl::event_state()&(FL_META|FL_ALT|FL_CTRL);
    } else {
      v = Fl::event_state()&(FL_META|FL_ALT|FL_CTRL|FL_SHIFT) | Fl::event_key();
      if (v == FL_BackSpace && svalue) v = 0;
    }
    if (v != svalue) {svalue = v; set_changed(); redraw();}
    return 1;
  } else if (e == FL_UNFOCUS) {
    int c = changed(); value(0); if (c) set_changed();
    return 1;
  } else if (e == FL_FOCUS) {
    return value();
  } else {
    int r = Fl_Button::handle(e);
    if (e == FL_RELEASE && value() && Fl::focus() != this) take_focus();
    return r;
  }
}
  
void shortcut_in_cb(Shortcut_Button* i, void* v) {
  if (v == LOAD) {
    if (!current_widget->is_button()) {i->hide(); return;}
    i->show();
    i->svalue = ((Fl_Button*)(current_widget->o))->shortcut();
    i->redraw();
  } else {
    for (Fl_Type *o = Fl_Type::first; o; o = o->next)
      if (o->selected && o->is_button()) {
	Fl_Button* b = (Fl_Button*)(((Fl_Widget_Type*)o)->o);
	b->shortcut(i->svalue);
	if (o->is_menu_item()) ((Fl_Widget_Type*)o)->redraw();
      }
  }
}

//
// End of "$Id: Fl_Menu_Type.cxx,v 1.16.2.12 2001/01/22 15:13:38 easysw Exp $".
//
