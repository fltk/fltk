//
// "$Id: Fl_Menu_add.cxx,v 1.5 1999/01/07 19:17:23 mike Exp $"
//
// Menu utilities for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-1999 by Bill Spitzak and others.
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
// Please report all bugs and problems to "fltk-bugs@easysw.com".
//

// Methods to alter the menu in an Fl_Menu_ widget.
// This code is seperated so that it is not linked in if not used.

// These functions are for emulation of Forms and for dynamically
// changing the menus.  They are in this source file so they are
// not linked in if not used, which is what will happen if the
// the program only uses constant menu tables.

// Not at all guaranteed to be Forms compatable, especially with any
// string with a % sign in it!

#include <FL/Fl_Menu_.H>
#include <string.h>
#include <stdlib.h>

int fl_old_shortcut(const char* s) {
  if (!s || !*s) return 0;
  int n = 0;
  if (*s == '#') {n |= FL_ALT; s++;}
  if (*s == '+') {n |= FL_SHIFT; s++;}
  if (*s == '^') {n |= FL_CTRL; s++;}
  return n | *s;
}

// always allocate this much initially:
#define INITIAL_MENU_SIZE 15

// as menu array size passes through each power of two, the memory
// array allocated is doubled in size:
static Fl_Menu_Item* incr_array(Fl_Menu_Item* array, int size) {
  if (size < INITIAL_MENU_SIZE) return array;
  if ((size+1) & size) return array; // not a power of 2
  Fl_Menu_Item* newarray = new Fl_Menu_Item[size*2+1];
  for (int i = 0; i <= size; i++) newarray[i] = array[i];
  delete[] array;
  return newarray;
}

// if this local pointer is set, array is reallocated and put here:
static Fl_Menu_Item** alloc;

int Fl_Menu_Item::add(
  const char *text,
  int shortcut,
  Fl_Callback *cb,	
  void *data,
  int flags)
{
  Fl_Menu_Item *array = this;
  Fl_Menu_Item *m = this;
  const char *p;
  char *q;
  char buf[1024];

  int size = array->size();
  int flags1 = 0;
  char* item;
  for (;;) {    /* do all the supermenus: */

    /* fill in the buf with name, changing \x to x: */
    q = buf;
    for (p=text; *p && *p != '/'; *q++ = *p++) if (*p=='\\') p++;
    *q = 0;

    item = buf;
    if (*item == '_') {item++; flags1 = FL_MENU_DIVIDER;}
    if (*p != '/') break; /* not a menu title */
    text = p+1;	/* point at item title */

    /* find a matching menu title: */
    for (; m->text; m = m->next())
      if (m->flags&FL_SUBMENU && !strcmp(item,m->text)) break;

    if (!m->text) { /* create a new menu */
      if (alloc) {
	int n = m-array;
	array = incr_array(array,size);
	array = incr_array(array,size+1);
	*alloc = array;
	m = array+n;
      }
      memmove(m+2,m,sizeof(Fl_Menu_Item)*(array+size-m));
      m->text = strdup(item);
      m->shortcut_ = 0;
      m->callback_ = 0;
      m->user_data_ = 0;
      m->flags = FL_SUBMENU|flags1;
      m->labeltype_ = m->labelfont_ = m->labelsize_ = m->labelcolor_ = 0;
      (m+1)->text = 0;
      size += 2;
    }
    m++;	/* go into the menu */
    flags1 = 0;
  }

  /* find a matching menu item: */
  for (; m->text; m = m->next())
    if (!strcmp(m->text,item)) break;

  if (!m->text) {	/* add a new menu item */
    if (alloc) {
      int n = m-array;
      *alloc = array = incr_array(array,size);
      m = array+n;
    }
    memmove(m+1,m,sizeof(Fl_Menu_Item)*(array+size-m));
    size++;
    m->text = strdup(item);
  }

  /* fill it in */
  m->shortcut_ = shortcut;
  m->callback_ = cb;
  m->user_data_ = data;
  m->flags = flags|flags1;
  m->labeltype_ = m->labelfont_ = m->labelsize_ = m->labelcolor_ = 0;

  return m-array;
}

int Fl_Menu_::add(const char *t, int s, Fl_Callback *c,void *v,int f) {
  int n = value_ ? value_ - menu_ : 0;
  if (!menu_) {
    alloc = 1;
    menu_ = new Fl_Menu_Item[INITIAL_MENU_SIZE];
    menu_[0].text = 0;
  }
  if (alloc) ::alloc = &menu_;
  int r = menu_->add(t,s,c,v,f);
  ::alloc = 0;
  if (value_) value_ = menu_+n;
  return r;
}

int Fl_Menu_::add(const char *str) {
  char buf[128];
  int r = 0;
  while (*str) {
    int shortcut = 0;
    char *c;
    for (c = buf; *str && *str != '|'; str++) {
      if (*str == '\t') {*c++ = 0; shortcut = fl_old_shortcut(str);}
      else *c++ = *str;
    }
    *c = 0;
    r = add(buf, shortcut, 0, 0, 0);
    if (*str) str++;
  }
  return r;
}

void Fl_Menu_::replace(int i, const char *str) {
  if (i<0 || i>=size()) return;
  if (alloc) free((void *)menu_[i].text);
  menu_[i].text = strdup(str);
}

void Fl_Menu_::remove(int i) {
  int n = size();
  if (i<0 || i>=n) return;
  if (alloc) free((void *)menu_[i].text);
  memmove(&menu_[i],&menu_[i+1],(n-i)*sizeof(Fl_Menu_Item));
}

void Fl_Menu_::clear() {
  for (int i = size(); i--;)
    if (menu_[i].text) free((void*)menu_[i].text);
  if (alloc) {
    delete[] menu_;
    menu_ = 0;
    alloc = 0;
  } else if (menu_) {
    menu_[0].text = 0;
    value_ = menu_;
  }
}

//
// End of "$Id: Fl_Menu_add.cxx,v 1.5 1999/01/07 19:17:23 mike Exp $".
//
