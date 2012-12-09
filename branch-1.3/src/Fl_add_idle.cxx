//
// "$Id$"
//
// Idle routine support for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

// Allows you to manage an arbitrary set of idle() callbacks.
// Replaces the older set_idle() call (which is used to implement this)

#include <FL/Fl.H>

struct idle_cb {
  void (*cb)(void*);
  void* data;
  idle_cb *next;
};

// the callbacks are stored linked in a ring.  last points at the one
// just called, first at the next to call.  last->next == first.

static idle_cb* first;
static idle_cb* last;
static idle_cb* freelist;

static void call_idle() {
  idle_cb* p = first;
  last = p; first = p->next;
  p->cb(p->data); // this may call add_idle() or remove_idle()!
}

/**
  Adds a callback function that is called every time by Fl::wait() and also
  makes it act as though the timeout is zero (this makes Fl::wait() return
  immediately, so if it is in a loop it is called repeatedly, and thus the
  idle fucntion is called repeatedly).  The idle function can be used to get
  background processing done.
    
  You can have multiple idle callbacks. To remove an idle callback use
  Fl::remove_idle().
    
  Fl::wait() and Fl::check() call idle callbacks, but Fl::ready() does not.
    
  The idle callback can call any FLTK functions, including Fl::wait(),
  Fl::check(), and Fl::ready().

  FLTK will not recursively call the idle callback.
*/
void Fl::add_idle(Fl_Idle_Handler cb, void* data) {
  idle_cb* p = freelist;
  if (p) freelist = p->next;
  else p = new idle_cb;
  p->cb = cb;
  p->data = data;
  if (first) {
    last->next = p;
    last = p;
    p->next = first;
  } else {
    first = last = p;
    p->next = p;
    set_idle(call_idle);
  }
}

/**
  Returns true if the specified idle callback is currently installed.
*/
int Fl::has_idle(Fl_Idle_Handler cb, void* data) {
  idle_cb* p = first;
  if (!p) return 0;
  for (;; p = p->next) {
    if (p->cb == cb && p->data == data) return 1;
    if (p==last) return 0;
  }
}

/**
  Removes the specified idle callback, if it is installed.
*/
void Fl::remove_idle(Fl_Idle_Handler cb, void* data) {
  idle_cb* p = first;
  if (!p) return;
  idle_cb* l = last;
  for (;; p = p->next) {
    if (p->cb == cb && p->data == data) break;
    if (p==last) return; // not found
    l = p;
  }
  if (l == p) { // only one
    first = last = 0;
    set_idle(0);
  } else {
    last = l;
    first = l->next = p->next;
  }
  p->next = freelist;
  freelist = p;
}

//
// End of "$Id$".
//
