//
// "$Id: Fl_Menu_.cxx,v 1.7.2.8 2001/01/22 15:13:40 easysw Exp $"
//
// Common menu code for the Fast Light Tool Kit (FLTK).
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

// This is a base class for all items that have a menu:
//	Fl_Menu_Bar, Fl_Menu_Button, Fl_Choice
// This provides storage for a menu item, functions to add/modify/delete
// items, and a call for when the user picks a menu item.

// More code in Fl_Menu_add.C

#include <FL/Fl.H>
#include <FL/Fl_Menu_.H>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int Fl_Menu_::value(const Fl_Menu_Item* m) {
  clear_changed();
  if (value_ != m) {value_ = m; return 1;}
  return 0;
}

// When user picks a menu item, call this.  It will do the callback.
// Unfortunatly this also casts away const for the checkboxes, but this
// was necessary so non-checkbox menus can really be declared const...
const Fl_Menu_Item* Fl_Menu_::picked(const Fl_Menu_Item* v) {
  if (v) {
    if (v->radio()) {
      if (!v->value()) { // they are turning on a radio item
	set_changed();
	((Fl_Menu_Item*)v)->setonly();
      }
      redraw();
    } else if (v->flags & FL_MENU_TOGGLE) {
      set_changed();
      ((Fl_Menu_Item*)v)->flags ^= FL_MENU_VALUE;
      redraw();
    } else if (v != value_) { // normal item
      set_changed();
    }
    value_ = v;
    if (when()&(FL_WHEN_CHANGED|FL_WHEN_RELEASE)) {
      if (changed() || when()&FL_WHEN_NOT_CHANGED) {
	clear_changed();
	if (value_ && value_->callback_) value_->do_callback((Fl_Widget*)this);
	else do_callback();
      }
    }
  }
  return v;
}

// turn on one of a set of radio buttons
void Fl_Menu_Item::setonly() {
  flags |= FL_MENU_RADIO | FL_MENU_VALUE;
  Fl_Menu_Item* j;
  for (j = this; ; ) {	// go down
    if (j->flags & FL_MENU_DIVIDER) break; // stop on divider lines
    j++;
    if (!j->text || !j->radio()) break; // stop after group
    j->clear();
  }
  for (j = this-1; ; j--) { // go up
    if (!j->text || (j->flags&FL_MENU_DIVIDER) || !j->radio()) break;
    j->clear();
  }
}

Fl_Menu_::Fl_Menu_(int X,int Y,int W,int H,const char* l)
: Fl_Widget(X,Y,W,H,l) {
  set_flag(SHORTCUT_LABEL);
  box(FL_UP_BOX);
  when(FL_WHEN_RELEASE_ALWAYS);
  value_ = menu_ = 0;
  alloc = 0;
  selection_color(FL_SELECTION_COLOR);
  textfont(FL_HELVETICA);
  textsize(FL_NORMAL_SIZE);
  textcolor(FL_BLACK);
  down_box(FL_NO_BOX);
}

int Fl_Menu_::size() const {
  if (!menu_) return 0;
  return menu_->size();
}

void Fl_Menu_::menu(const Fl_Menu_Item* m) {
  clear();
  value_ = menu_ = (Fl_Menu_Item*)m;
}

#if 1
// this version is ok with new Fl_Menu_add code with fl_menu_array_owner:

void Fl_Menu_::copy(const Fl_Menu_Item* m, void* user_data) {
  int n = m->size();
  Fl_Menu_Item* newMenu = new Fl_Menu_Item[n];
  memcpy(newMenu, m, n*sizeof(Fl_Menu_Item));
  menu(newMenu);
  alloc = 1; // make destructor free array, but not strings
  // for convienence, provide way to change all the user data pointers:
  if (user_data) for (; n--;) {
    if (newMenu->callback_) newMenu->user_data_ = user_data;
    newMenu++;
  }
}

#else
// This is Guillaume Nodet's fixed version for the older Fl_Menu_add
// that enlarged the array at powers of 2:

void Fl_Menu_::copy(const Fl_Menu_Item* m, void* user_data) {
  int i, s = m->size(), n=s;
  for (i=0; n; n>>=1, i++);
  n = 1 << i;
  Fl_Menu_Item* newMenu = new Fl_Menu_Item[n];
  memcpy(newMenu, m, s*sizeof(Fl_Menu_Item));
  memset(newMenu+s, 0, (n-s)*sizeof(Fl_Menu_Item));
  menu(newMenu);
  alloc = 1; // make destructor free it
  // for convienence, provide way to change all the user data pointers:
  if (user_data) for (; s--;) {
    if (newMenu->callback_) newMenu->user_data_ = user_data;
    newMenu++;
  }
}
#endif

Fl_Menu_::~Fl_Menu_() {
  clear();
}

// Fl_Menu::add() uses this to indicate the owner of the dynamically-
// expanding array.  We must not free this array:
Fl_Menu_* fl_menu_array_owner = 0;

void Fl_Menu_::clear() {
  if (alloc) {
    if (alloc>1) for (int i = size(); i--;)
      if (menu_[i].text) free((void*)menu_[i].text);
    if (this == fl_menu_array_owner)
      fl_menu_array_owner = 0;
    else
      delete[] menu_;
    menu_ = 0;
    value_ = 0;
    alloc = 0;
  }
}

//
// End of "$Id: Fl_Menu_.cxx,v 1.7.2.8 2001/01/22 15:13:40 easysw Exp $".
//
