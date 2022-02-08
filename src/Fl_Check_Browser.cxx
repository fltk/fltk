//
// Fl_Check_Browser implementation for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2020 by Bill Spitzak and others.
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

#include <stdio.h>
#include <stdlib.h>
#include "flstring.h"
#include <FL/fl_draw.H>
#include <FL/fl_string_functions.h> // fl_strdup()
#include <FL/Fl_Check_Browser.H>

/* This uses a cache for faster access when you're scanning the list
either forwards or backwards. */

Fl_Check_Browser::cb_item *Fl_Check_Browser::find_item(int n) const {
  int i = n;
  cb_item *p = first;

  if (n <= 0 || n > nitems_ || p == 0) {
    return 0;
  }

  if (n == cached_item) {
    p = cache;
    n = 1;
  } else if (n == cached_item + 1) {
    p = cache->next;
    n = 1;
  } else if (n == cached_item - 1) {
    p = cache->prev;
    n = 1;
  }

  while (--n) {
    p = p->next;
  }

  /* Cast to not const and cache it. */
  ((Fl_Check_Browser *)this)->cache = p;
  ((Fl_Check_Browser *)this)->cached_item = i;

  return p;
}

int Fl_Check_Browser::lineno(cb_item *p0) const {
  cb_item *p = first;

  if (p == 0) {
    return 0;
  }

  int i = 1;
  while (p) {
    if (p == p0) {
      return i;
    }
    i++;
    p = p->next;
  }

  return 0;
}

/** The constructor makes an empty browser. */
Fl_Check_Browser::Fl_Check_Browser(int X, int Y, int W, int H, const char *l)
  : Fl_Browser_(X, Y, W, H, l) {
  type(FL_SELECT_BROWSER);
  when(FL_WHEN_NEVER);
  first = last = 0;
  nitems_ = nchecked_ = 0;
  cached_item = -1;
}

void *Fl_Check_Browser::item_first() const {
  return first;
}

void *Fl_Check_Browser::item_next(void *l) const {
  return ((cb_item *)l)->next;
}

void *Fl_Check_Browser::item_prev(void *l) const {
  return ((cb_item *)l)->prev;
}

int Fl_Check_Browser::item_height(void *) const {
  return textsize() + 2;
}

const char *Fl_Check_Browser::item_text(void *item) const {
  cb_item *i = (cb_item *)item;
  return i->text;
}

void *Fl_Check_Browser::item_at(int index) const { // note: index is 1-based
  if (index < 1 || index > nitems())
    return 0L;
  cb_item *item = (cb_item *)item_first();
  for (int i = 1; i < index; i++)
    item = (cb_item *)(item_next(item));
  return (void *)item;
}

void Fl_Check_Browser::item_swap(int ia, int ib) {
  item_swap(item_at(ia), item_at(ib));
}

void Fl_Check_Browser::item_swap(void *a, void *b) {
  cb_item *ia = (cb_item *)a;
  cb_item *ib = (cb_item *)b;

  cb_item *a_next = ia->next;
  cb_item *a_prev = ia->prev;

  cb_item *b_next = ib->next;
  cb_item *b_prev = ib->prev;

  if (a_next == ib) {        // p - a - b - n  => p - b - a - n
    if (a_prev)
      a_prev->next = ib;
    if (b_next)
      b_next->prev = ia;
    ib->prev = a_prev;
    ib->next = ia;
    ia->prev = ib;
    ia->next = b_next;
  } else if (a_prev == ib) {    // p - b - a - n  => p - a - b - n
    if (b_prev)
      b_prev->next = ia;
    if (a_next)
      a_next->prev = ib;
    ia->prev = b_prev;
    ia->next = ib;
    ib->prev = ia;
    ib->next = a_next;
  } else {            // x - a - y - b - z => x - b - y - a - z
    if (a_prev)
      a_prev->next = ib;
    if (a_next)
      a_next->prev = ib;
    ia->next = b_next;
    ia->prev = b_prev;

    if (b_prev)
      b_prev->next = ia;
    if (b_next)
      b_next->prev = ia;
    ib->next = a_next;
    ib->prev = a_prev;
  }
  if (first == ia)
    first = ib;
  if (last == ia)
    last = ib;
  // invalidate item cache
  cached_item = -1;
  cache = 0L;
}

#define CHECK_SIZE (textsize()-2)

int Fl_Check_Browser::item_width(void *v) const {
  fl_font(textfont(), textsize());
  return int(fl_width(((cb_item *)v)->text)) + CHECK_SIZE + 8;
}

void Fl_Check_Browser::item_draw(void *v, int X, int Y, int, int) const {
  cb_item *i = (cb_item *)v;
  char *s = i->text;
  int tsize = textsize();
  Fl_Color col = active_r() ? textcolor() : fl_inactive(textcolor());
  int cy = Y + (tsize + 1 - CHECK_SIZE) / 2;
  X += 2;

  // draw the check mark box (always)
  fl_color(active_r() ? FL_FOREGROUND_COLOR : fl_inactive(FL_FOREGROUND_COLOR));
  fl_loop(X, cy, X, cy + CHECK_SIZE,
          X + CHECK_SIZE, cy + CHECK_SIZE, X + CHECK_SIZE, cy);

  // draw the check mark
  if (i->checked) {
    fl_draw_check(Fl_Rect(X + 1, cy + 1, CHECK_SIZE - 1, CHECK_SIZE - 1), fl_color());
  }

  // draw the item text
  fl_font(textfont(), tsize);
  if (i->selected) {
    col = fl_contrast(col, selection_color());
  }
  fl_color(col);
  fl_draw(s, X + CHECK_SIZE + 8, Y + tsize - 1);
}

void Fl_Check_Browser::item_select(void *v, int state) {
  cb_item *i = (cb_item *)v;

  if (state) {
    if (i->checked) {
      i->checked = 0;
      nchecked_--;
    } else {
      i->checked = 1;
      nchecked_++;
    }
  }
}

int Fl_Check_Browser::item_selected(void *v) const {
  cb_item *i = (cb_item *)v;
  return i->selected;
}
/**
 Add a new unchecked line to the end of the browser.
 \see add(char *s, int b)
*/
int Fl_Check_Browser::add(char *s) {
  return (add(s, 0));
}

/**
 Add a new line to the end of the browser.  The text is copied
 using the strdup() function.  It may also be NULL to make
 a blank line.  It can set the item checked if \p b is not 0.
 */
int Fl_Check_Browser::add(char *s, int b) {
  cb_item *p = (cb_item *)malloc(sizeof(cb_item));
  p->next = 0;
  p->prev = 0;
  p->checked = b;
  p->selected = 0;
  p->text = fl_strdup(s ? s : "");

  if (b) {
    nchecked_++;
  }

  if (last == 0) {
    first = last = p;
  } else {
    last->next = p;
    p->prev = last;
    last = p;
  }
  nitems_++;

  return (nitems_);
}

/**
  Remove line n and make the browser one line shorter. Returns the
  number of lines left in the browser.
*/
int Fl_Check_Browser::remove(int item) {
  cb_item *p = find_item(item);

  // line at item exists
  if (p) {
    // tell the Browser_ what we will do
    deleting(p);

    // fix checked count
    if (p->checked)
      --nchecked_;

    // remove the node
    if (p->prev)
      p->prev->next = p->next;
    else
      first = p->next;
    if (p->next)
      p->next->prev = p->prev;
    else
      last = p->prev;

    free(p->text);
    free(p);

    --nitems_;
    cached_item = -1;
  }

  return (nitems_);
}

/**  Remove every item from the browser.*/
void Fl_Check_Browser::clear() {
  cb_item *p = first;
  cb_item *next;

  if (!p) return;

  new_list();
  do {
    next = p->next;
    free(p->text);
    free(p);
    p = next;
  } while (p);

  first = last = 0;
  nitems_ = nchecked_ = 0;
  cached_item = -1;
}

/** Gets the current status of item item. */
int Fl_Check_Browser::checked(int i) const {
  cb_item *p = find_item(i);

  if (p) return p->checked;
  return 0;
}

/** Sets the check status of item item to b. */
void Fl_Check_Browser::checked(int i, int b) {
  cb_item *p = find_item(i);

  if (p && (p->checked ^ b)) {
    p->checked = b;
    if (b) {
      nchecked_++;
    } else {
      nchecked_--;
    }
    redraw();
  }
}

/**  Returns the index of the currently selected item.*/
int Fl_Check_Browser::value() const {
  return lineno((cb_item *)selection());
}

/**  Return a pointer to an internal buffer holding item item's text.*/
char *Fl_Check_Browser::text(int i) const {
  cb_item *p = find_item(i);

  if (p) return p->text;
  return 0;
}

/**  Sets all the items checked.*/
void Fl_Check_Browser::check_all() {
  cb_item *p;

  nchecked_ = nitems_;
  for (p = first; p; p = p->next) {
    p->checked = 1;
  }
  redraw();
}

/**  Sets all the items unchecked.*/
void Fl_Check_Browser::check_none() {
  cb_item *p;

  nchecked_ = 0;
  for (p = first; p; p = p->next) {
    p->checked = 0;
  }
  redraw();
}

int Fl_Check_Browser::handle(int event) {
  if (event == FL_PUSH) {
    int X, Y, W, H;
    bbox(X, Y, W, H);
    if (Fl::event_inside(X, Y, W, H)) {
      deselect();
    }
  }
  return Fl_Browser_::handle(event);
}
