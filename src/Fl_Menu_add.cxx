// Fl_Menu_add.C

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

int Fl_Menu_Item::add(
  const char *text,
  int shortcut,
  Fl_Callback *cb,	
  void *data,
  int flags)
{
  Fl_Menu_Item *m;
  const char *p;
  char *q;
  char buf[1024];

  int size = this->size();
  int flags1 = 0;
  char* item;

  m = this;

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
      memmove(m+2,m,sizeof(Fl_Menu_Item)*(this+size-m));
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
    memmove(m+1,m,sizeof(Fl_Menu_Item)*(this+size-m));
    size++;
    m->text = strdup(item);
  }

  /* fill it in */
  m->shortcut_ = shortcut;
  m->callback_ = cb;
  m->user_data_ = data;
  m->flags = flags|flags1;
  m->labeltype_ = m->labelfont_ = m->labelsize_ = m->labelcolor_ = 0;

  return m-this;
}

// this is really lame, it will crash if this many items are added:
#define FL_MENU_MAXITEMS	128

int Fl_Menu_::add(const char *t, int s, Fl_Callback *c,void *v,int f) {
  if (!menu_) {
    value_ = menu_ = new Fl_Menu_Item[FL_MENU_MAXITEMS+1];
    alloc = 1;
    menu_[0].text = 0;
  }
  return menu_->add(t,s,c,v,f);
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

// end of Fl_Menu_.C
