//
// "$Id: Fl_Menu_add.cxx,v 1.7 1999/02/03 08:43:33 bill Exp $"
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

// always allocate this much initially:
#define INITIAL_MENU_SIZE 15

// if this local pointer is set, array is reallocated and put here:
static Fl_Menu_Item** alloc;

// Insert a single Fl_Menu_Item into an array at offset n.  If ::alloc
// is not zero, the array may be reallocated.  This is done each time
// it's size passes through a power of 2.  The new (or old) array is
// returned.
// Notice that size does not count the trailing null item, so one more
// item than you think must be copied.
static Fl_Menu_Item* insert(
  Fl_Menu_Item* array, int size,
  int n,
  const char *text,
  int flags) {
  // If new size is a power of two, we reallocate to the next power
  // of two:
  if (alloc && size >= INITIAL_MENU_SIZE && !((size+1)&size)) {
    Fl_Menu_Item* newarray = new Fl_Menu_Item[(size+1)*2];
    memcpy(newarray, array, (size+1)*sizeof(Fl_Menu_Item));
    delete[] array;
    *alloc = array = newarray;
  }
  // move all the later items:
  memmove(array+n+1, array+n, sizeof(Fl_Menu_Item)*(size-n+1));
  // create the new item:
  Fl_Menu_Item* m = array+n;
  m->text = text ? strdup(text) : 0;
  m->shortcut_ = 0;
  m->callback_ = 0;
  m->user_data_ = 0;
  m->flags = flags;
  m->labeltype_ = m->labelfont_ = m->labelsize_ = m->labelcolor_ = 0;
  return array;
}

// Add an item.  The text is split at '/' characters to automatically
// produce submenus (actually a totally unnecessary feature as you can
// now add submenu titles directly by setting SUBMENU in the flags):
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
      if (m->flags&FL_SUBMENU && !strcmp(item, m->text)) break;

    if (!m->text) { /* create a new menu */
      int n = m-array;
      array = insert(array, size, n, item, FL_SUBMENU|flags1);
      size++;
      array = insert(array, size, n+1, 0, 0);
      size++;
      m = array+n;
    }
    m++;	/* go into the submenu */
    flags1 = 0;
  }

  /* find a matching menu item: */
  for (; m->text; m = m->next())
    if (!strcmp(m->text,item)) break;

  if (!m->text) {	/* add a new menu item */
    int n = m-array;
    array = insert(array, size, n, item, flags|flags1);
    size++;
    if (flags & FL_SUBMENU) { // add submenu delimiter
      array = insert(array, size, n+1, 0, 0);
      size++;
    }
    m = array+n;
  }

  /* fill it in */
  m->shortcut_ = shortcut;
  m->callback_ = cb;
  m->user_data_ = data;

  return m-array;
}

int Fl_Menu_::add(const char *t, int s, Fl_Callback *c,void *v,int f) {
  int n = value_ ? value_ - menu_ : 0;
  if (!menu_) {
    alloc = 2; // indicates that the strings can be freed
    menu_ = new Fl_Menu_Item[INITIAL_MENU_SIZE+1];
    menu_[0].text = 0;
  }
  if (alloc) ::alloc = &menu_;
  int r = menu_->add(t,s,c,v,f);
  ::alloc = 0;
  if (value_) value_ = menu_+n;
  return r;
}

// This is a Forms (and SGI GL library) compatable add function, it
// adds strings of the form "name\tshortcut|name\tshortcut|..."
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
  if (alloc > 1) {
    free((void *)menu_[i].text);
    str = strdup(str);
  }
  menu_[i].text = str;
}

void Fl_Menu_::remove(int i) {
  int n = size();
  if (i<0 || i>=n) return;
  if (alloc > 1) free((void *)menu_[i].text);
  memmove(&menu_[i],&menu_[i+1],(n-i)*sizeof(Fl_Menu_Item));
}

//
// End of "$Id: Fl_Menu_add.cxx,v 1.7 1999/02/03 08:43:33 bill Exp $".
//
