//
// "$Id: Fl.cxx,v 1.24.2.41 2001/02/26 00:19:02 spitzak Exp $"
//
// Main event handling code for the Fast Light Tool Kit (FLTK).
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
#include <FL/Fl_Window.H>
#include <FL/x.H>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>


//
// Globals...
//

Fl_Widget	*Fl::belowmouse_,
		*Fl::pushed_,
		*Fl::focus_,
		*Fl::selection_owner_;
int		Fl::damage_,
		Fl::e_x,
		Fl::e_y,
		Fl::e_x_root,
		Fl::e_y_root,
		Fl::e_state,
		Fl::e_clicks,
		Fl::e_is_click,
		Fl::e_keysym;
char		*Fl::e_text = (char *)"";
int		Fl::e_length;

//
// 'Fl:event_inside()' - Return whether or not the mouse event is inside
//                       the given rectangle.
//

int Fl::event_inside(int x,int y,int w,int h) /*const*/ {
  int mx = e_x - x;
  int my = e_y - y;
  return (mx >= 0 && mx < w && my >= 0 && my < h);
}

int Fl::event_inside(const Fl_Widget *o) /*const*/ {
  int mx = e_x - o->x();
  int my = e_y - o->y();
  return (mx >= 0 && mx < o->w() && my >= 0 && my < o->h());
}

////////////////////////////////////////////////////////////////
// Timeouts are stored in a sorted list, so only the first one needs
// to be checked to see if any should be called.
  
struct Timeout {
  double time;
  void (*cb)(void*);
  void* arg;
  Timeout* next;
};
static Timeout* first_timeout, *free_timeout;

#ifndef WIN32
#include <sys/time.h>
#endif

// I avoid the overhead of getting the current time when we have no
// timeouts by setting this flag instead of getting the time.
// In this case calling elapse_timeouts() does nothing, but records
// the current time, and the next call will actualy elapse time.
static char reset_clock = 1;

static void elapse_timeouts() {
#ifdef WIN32
  unsigned long newclock = GetTickCount();
  static unsigned long prevclock;
  double elapsed = (newclock-prevclock)/1000.0;
  prevclock = newclock;
#else
  static struct timeval prevclock;
  struct timeval newclock;
  gettimeofday(&newclock, NULL);
  double elapsed = newclock.tv_sec - prevclock.tv_sec +
    (newclock.tv_usec - prevclock.tv_usec)/1000000.0;
  prevclock.tv_sec = newclock.tv_sec;
  prevclock.tv_usec = newclock.tv_usec;
#endif
  if (reset_clock) {
    reset_clock = 0;
  } else if (elapsed > 0) {
    for (Timeout* t = first_timeout; t; t = t->next) t->time -= elapsed;
  }
}

// Continuously-adjusted error value, this is a number <= 0 for how late
// we were at calling the last timeout. This appears to make repeat_timeout
// very accurate even when processing takes a significant portion of the
// time interval:
static double missed_timeout_by;

void Fl::add_timeout(double time, Fl_Timeout_Handler cb, void *arg) {
  elapse_timeouts();
  repeat_timeout(time, cb, arg);
}

void Fl::repeat_timeout(double time, Fl_Timeout_Handler cb, void *arg) {
  time += missed_timeout_by; if (time < -.05) time = 0;
  Timeout* t = free_timeout;
  if (t) free_timeout = t->next;
  else t = new Timeout;
  t->time = time;
  t->cb = cb;
  t->arg = arg;
  // insert-sort the new timeout:
  Timeout** p = &first_timeout; 
  while (*p && (*p)->time <= time) p = &((*p)->next);
  t->next = *p;
  *p = t;
}

int Fl::has_timeout(Fl_Timeout_Handler cb, void *arg) {
  for (Timeout* t = first_timeout; t; t = t->next)
    if (t->cb == cb && t->arg == arg) return 1;
  return 0;
}

void Fl::remove_timeout(Fl_Timeout_Handler cb, void *arg) {
  // This version removes all matching timeouts, not just the first one.
  // This may change in the future.
  for (Timeout** p = &first_timeout; *p;) {
    Timeout* t = *p;
    if (t->cb == cb && t->arg == arg) {
      *p = t->next;
      t->next = free_timeout;
      free_timeout = t;
    } else {
      p = &(t->next);
    }
  }
}

////////////////////////////////////////////////////////////////
// Checks are just stored in a list. They are called in the reverse
// order that they were added (this may change in the future).
// This is a bit messy because I want to allow checks to be added,
// removed, and have wait() called from inside them, to do this
// next_check points at the next unprocessed one for the outermost
// call to Fl::wait().

struct Check {
  void (*cb)(void*);
  void* arg;
  Check* next;
};
static Check* first_check, *next_check, *free_check;

void Fl::add_check(Fl_Timeout_Handler cb, void *arg) {
  Check* t = free_check;
  if (t) free_check = t->next;
  else t = new Check;
  t->cb = cb;
  t->arg = arg;
  t->next = first_check;
  if (next_check == first_check) next_check = t;
  first_check = t;
}

void Fl::remove_check(Fl_Timeout_Handler cb, void *arg) {
  for (Check** p = &first_check; *p;) {
    Check* t = *p;
    if (t->cb == cb && t->arg == arg) {
      if (next_check == t) next_check = t->next;
      *p = t->next;
      t->next = free_check;
      free_check = t;
    } else {
      p = &(t->next);
    }
  }
}
  
////////////////////////////////////////////////////////////////
// wait/run/check/ready:

void (*Fl::idle)(); // see Fl_add_idle.cxx for the add/remove functions

extern int fl_wait(double time); // in Fl_x.cxx or Fl_win32.cxx

static char in_idle;

double Fl::wait(double time_to_wait) {
  if (first_timeout) {
    elapse_timeouts();
    Timeout *t;
    while ((t = first_timeout)) {
      if (t->time > 0) break;
      // The first timeout in the array has expired.
      missed_timeout_by = t->time;
      // We must remove timeout from array before doing the callback:
      void (*cb)(void*) = t->cb;
      void *arg = t->arg;
      first_timeout = t->next;
      t->next = free_timeout;
      free_timeout = t;
      // Now it is safe for the callback to do add_timeout:
      cb(arg);
    }
  } else {
    reset_clock = 1; // we are not going to check the clock
  }
  // checks are a bit messy so that add/remove and wait may be called
  // from inside them without causing an infinite loop:
  if (next_check == first_check) {
    while (next_check) {
      Check* check = next_check;
      next_check = check->next;
      (check->cb)(check->arg);
    }
    next_check = first_check;
  }
  if (idle) {
    if (!in_idle) {in_idle = 1; idle(); in_idle = 0;}
    // the idle function may turn off idle, we can then wait:
    if (idle) time_to_wait = 0.0;
  }
  if (first_timeout && first_timeout->time < time_to_wait)
    time_to_wait = first_timeout->time;
  if (time_to_wait <= 0.0) {
    // do flush second so that the results of events are visible:
    int ret = fl_wait(0.0);
    flush();
    return ret;
  } else {
    // do flush first so that user sees the display:
    flush();
    return fl_wait(time_to_wait);
  }
}

#define FOREVER 1e20

int Fl::run() {
  while (Fl_X::first) wait(FOREVER);
  return 0;
}

int Fl::wait() {
  wait(FOREVER);
  return Fl_X::first != 0; // return true if there is a window
}

int Fl::check() {
  wait(0.0);
  return Fl_X::first != 0; // return true if there is a window
}

extern int fl_ready();

int Fl::ready() {
  if (first_timeout) {
    elapse_timeouts();
    if (first_timeout->time <= 0) return 1;
  } else {
    reset_clock = 1;
  }
  return fl_ready();
}

////////////////////////////////////////////////////////////////
// Window list management:

Fl_X* Fl_X::first;

Fl_Window* fl_find(Window xid) {
  Fl_X *window;
  for (Fl_X **pp = &Fl_X::first; (window = *pp); pp = &window->next)
    if (window->xid == xid) {
      if (window != Fl_X::first && !Fl::modal()) {
	// make this window be first to speed up searches
	// this is not done if modal is true to avoid messing up modal stack
	*pp = window->next;
	window->next = Fl_X::first;
	Fl_X::first = window;
      }
      return window->w;
    }
  return 0;
}

Fl_Window* Fl::first_window() {
  Fl_X* x = Fl_X::first;
  return x ? x->w : 0;
}

Fl_Window* Fl::next_window(const Fl_Window* w) {
  Fl_X* x = Fl_X::i(w)->next;
  return x ? x->w : 0;
}

void Fl::first_window(Fl_Window* window) {
  if (!window || !window->shown()) return;
  fl_find(fl_xid(window));
}

void Fl::redraw() {
  for (Fl_X* x = Fl_X::first; x; x = x->next) x->w->redraw();
}

void Fl::flush() {
  if (damage()) {
    damage_ = 0;
    for (Fl_X* x = Fl_X::first; x; x = x->next) {
      if (x->wait_for_expose) {damage_ = 1; continue;}
      Fl_Window* w = x->w;
      if (!w->visible_r()) continue;
      if (w->damage()) {x->flush(); w->clear_damage();}
      // destroy damage regions for windows that don't use them:
      if (x->region) {XDestroyRegion(x->region); x->region = 0;}
    }
  }
#ifdef WIN32
  GdiFlush();
#else
  if (fl_display) XFlush(fl_display);
#endif
}

////////////////////////////////////////////////////////////////
// Event handlers:

struct handler_link {
  int (*handle)(int);
  const handler_link *next;
};

static const handler_link *handlers = 0;

void Fl::add_handler(int (*h)(int)) {
  handler_link *l = new handler_link;
  l->handle = h;
  l->next = handlers;
  handlers = l;
}

static int send_handlers(int event) {
  for (const handler_link *h = handlers; h; h = h->next)
    if (h->handle(event)) return 1;
  return 0;
}

////////////////////////////////////////////////////////////////

Fl_Widget* fl_oldfocus; // kludge for Fl_Group...

void Fl::focus(Fl_Widget *o) {
  if (grab()) return; // don't do anything while grab is on
  Fl_Widget *p = focus_;
  if (o != p) {
    Fl::compose_reset();
    focus_ = o;
    fl_oldfocus = 0;
    for (; p && !p->contains(o); p = p->parent()) {
      p->handle(FL_UNFOCUS);
      fl_oldfocus = p;
    }
  }
}

void Fl::belowmouse(Fl_Widget *o) {
  if (grab()) return; // don't do anything while grab is on
  Fl_Widget *p = belowmouse_;
  if (o != p) {
    belowmouse_ = o;
    for (; p && !p->contains(o); p = p->parent()) p->handle(FL_LEAVE);
  }
}

void Fl::pushed(Fl_Widget *o) {
  pushed_ = o;
}

Fl_Window *fl_xfocus;	// which window X thinks has focus
Fl_Window *fl_xmousewin;// which window X thinks has FL_ENTER
Fl_Window *Fl::grab_;	// most recent Fl::grab()
Fl_Window *Fl::modal_;	// topmost modal() window

// Update modal(), focus() and other state according to system state,
// and send FL_ENTER, FL_LEAVE, FL_FOCUS, and/or FL_UNFOCUS events.
// This is the only function that produces these events in response
// to system activity.
// This is called whenever a window is added or hidden, and whenever
// X says the focus or mouse window have changed.

void fl_fix_focus() {

  if (Fl::grab()) return; // don't do anything while grab is on.

  // set focus based on Fl::modal() and fl_xfocus
  Fl_Widget* w = fl_xfocus;
  if (w) {
    Fl::e_keysym = 0; // make sure widgets don't think a keystroke moved focus
    while (w->parent()) w = w->parent();
    if (Fl::modal()) w = Fl::modal();
    if (!w->contains(Fl::focus()))
      if (!w->take_focus()) Fl::focus(w);
  } else
    Fl::focus(0);

  if (!Fl::pushed()) {

    // set belowmouse based on Fl::modal() and fl_xmousewin:
    w = fl_xmousewin;
    if (w) {
      if (Fl::modal()) w = Fl::modal();
      if (!w->contains(Fl::belowmouse())) {
	Fl::belowmouse(w);
	w->handle(FL_ENTER);
      } else {
	// send a FL_MOVE event so the enter/leave state is up to date
	Fl::e_x = Fl::e_x_root-fl_xmousewin->x();
	Fl::e_y = Fl::e_y_root-fl_xmousewin->y();
	w->handle(FL_MOVE);
      }
    } else {
      Fl::belowmouse(0);
    }
  }
}

#ifndef WIN32
Fl_Widget *fl_selection_requestor; // from Fl_cutpaste.C
#endif

// This function is called by ~Fl_Widget() and by Fl_Widget::deactivate
// and by Fl_Widget::hide().  It indicates that the widget does not want
// to receive any more events, and also removes all global variables that
// point at the widget.
// I changed this from the 1.0.1 behavior, the older version could send
// FL_LEAVE or FL_UNFOCUS events to the widget.  This appears to not be
// desirable behavior and caused flwm to crash.

void fl_throw_focus(Fl_Widget *o) {
  if (o->contains(Fl::pushed())) Fl::pushed_ = 0;
  if (o->contains(Fl::selection_owner())) Fl::selection_owner_ = 0;
#ifndef WIN32
  if (o->contains(fl_selection_requestor)) fl_selection_requestor = 0;
#endif
  if (o->contains(Fl::belowmouse())) Fl::belowmouse_ = 0;
  if (o->contains(Fl::focus())) Fl::focus_ = 0;
  if (o == fl_xfocus) fl_xfocus = 0;
  if (o == fl_xmousewin) fl_xmousewin = 0;
  fl_fix_focus();
}

////////////////////////////////////////////////////////////////

// Call to->handle but first replace the mouse x/y with the correct
// values to account for nested X windows. 'window' is the outermost
// window the event was posted to by X:
static int send(int event, Fl_Widget* to, Fl_Window* window) {
  int dx = window->x();
  int dy = window->y();
  for (const Fl_Widget* w = to; w; w = w->parent())
    if (w->type()>=FL_WINDOW) {dx -= w->x(); dy -= w->y();}
  int save_x = Fl::e_x; Fl::e_x += dx;
  int save_y = Fl::e_y; Fl::e_y += dy;
  int ret = to->handle(event);
  Fl::e_y = save_y;
  Fl::e_x = save_x;
  return ret;
}

int Fl::handle(int event, Fl_Window* window)
{
  Fl_Widget* w = window;

  switch (event) {

  case FL_CLOSE:
    if (grab() || modal() && window != modal()) return 0;
    w->do_callback();
    return 1;

  case FL_SHOW:
    ((Fl_Widget*)w)->show();
    return 1;

  case FL_HIDE:
    ((Fl_Widget*)w)->hide();
    return 1;

  case FL_PUSH:
    if (grab()) w = grab();
    else if (modal() && w != modal()) return 0;
    pushed_ = w;
    if (send(event, w, window)) return 1;
    // raise windows that are clicked on:
    window->show();
    return 1;

  case FL_MOVE:
  case FL_DRAG:
    fl_xmousewin = window; // this should already be set, but just in case.
    if (pushed()) {
      w = pushed();
      event = FL_DRAG;
    } else if (modal() && w != modal()) {
      w = 0;
    }
    if (grab()) w = grab();
    break;

  case FL_RELEASE: {
    if (pushed()) {
      w = pushed();
      pushed_ = 0; // must be zero before callback is done!
    }
    if (grab()) w = grab();
    int r = send(event, w, window);
    fl_fix_focus();
    return r;}

  case FL_UNFOCUS:
    window = 0;
  case FL_FOCUS:
    fl_xfocus = window;
    fl_fix_focus();
    return 1;

  case FL_KEYBOARD:
    fl_xfocus = window; // this should not happen!  But maybe it does:

    // Try it as keystroke, sending it to focus and all parents:
    for (w = grab() ? grab() : focus(); w; w = w->parent())
      if (send(FL_KEYBOARD, w, window)) return 1;

    // recursive call to try shortcut:
    if (handle(FL_SHORTCUT, window)) return 1;

    // and then try a shortcut with the case of the text swapped, by
    // changing the text and falling through to FL_SHORTCUT case:
    {char* c = (char*)event_text(); // cast away const
    if (!isalpha(*c)) return 0;
    *c = isupper(*c) ? tolower(*c) : toupper(*c);}
    event = FL_SHORTCUT;

  case FL_SHORTCUT:

    if (grab()) {w = grab(); break;} // send it to grab window

    // Try it as shortcut, sending to mouse widget and all parents:
    w = belowmouse(); if (!w) {w = modal(); if (!w) w = window;}
    for (; w; w = w->parent()) if (send(FL_SHORTCUT, w, window)) return 1;

    // try using add_handle() functions:
    if (send_handlers(FL_SHORTCUT)) return 1;

    // make Escape key close windows:
    if (event_key()==FL_Escape) {
      w = modal(); if (!w) w = window;
      w->do_callback();
      return 1;
    }

    return 0;

  case FL_ENTER:
    fl_xmousewin = window;
    fl_fix_focus();
    return 1;

  case FL_LEAVE:
    if (window == fl_xmousewin) {fl_xmousewin = 0; fl_fix_focus();}
    return 1;

  default:
    break;
  }
  if (w && send(event, w, window)) return 1;
  return send_handlers(event);
}

////////////////////////////////////////////////////////////////
// hide() destroys the X window, it does not do unmap!

void Fl_Window::hide() {
  clear_visible();
  if (!shown()) return;

  // remove from the list of windows:
  Fl_X* x = i;
  Fl_X** pp = &Fl_X::first;
  for (; *pp != x; pp = &(*pp)->next) if (!*pp) return;
  *pp = x->next;
  i = 0;

  // recursively remove any subwindows:
  for (Fl_X *w = Fl_X::first; w;) {
    Fl_Window* W = w->w;
    if (W->window() == this) {
      W->hide();
      W->set_visible();
      w = Fl_X::first;
    } else w = w->next;
  }

  if (this == Fl::modal_) { // we are closing the modal window, find next one:
    Fl_Window* w;
    for (w = Fl::first_window(); w; w = Fl::next_window(w))
      if (w->modal()) break;
    Fl::modal_ = w;
  }

  // Make sure no events are sent to this window:
  fl_throw_focus(this);
  handle(FL_HIDE);

#ifdef WIN32
  if (x->private_dc) ReleaseDC(x->xid,x->private_dc);
  if (x->xid == fl_window && fl_gc) {
    ReleaseDC(fl_window, fl_gc);
    fl_window = (HWND)-1;
    fl_gc = 0;
  }
#else
  if (x->region) XDestroyRegion(x->region);
#endif
  XDestroyWindow(fl_display, x->xid);
#ifdef WIN32
  // Try to stop the annoying "raise another program" behavior
  if (non_modal() && Fl::first_window() && Fl::first_window()->shown())
    Fl::first_window()->show();
#endif
  delete x;
}

Fl_Window::~Fl_Window() {
  hide();
}

// FL_SHOW and FL_HIDE are called whenever the visibility of this widget
// or any parent changes.  We must correctly map/unmap the system's window.

// For top-level windows it is assummed the window has already been
// mapped or unmapped!!!  This is because this should only happen when
// Fl_Window::show() or Fl_Window::hide() is called, or in response to
// iconize/deiconize events from the system.

int Fl_Window::handle(int event) {
  if (parent()) switch (event) {
  case FL_SHOW:
    if (!shown()) show();
    else XMapWindow(fl_display, fl_xid(this)); // extra map calls are harmless
    break;
  case FL_HIDE:
    if (shown()) {
      // Find what really turned invisible, if is was a parent window
      // we do nothing.  We need to avoid unnecessary unmap calls
      // because they cause the display to blink when the parent is
      // remapped.  However if this or any intermediate non-window
      // widget has really had hide() called directly on it, we must
      // unmap because when the parent window is remapped we don't
      // want to reappear.
      if (visible()) {
       Fl_Widget* p = parent(); for (;p->visible();p = p->parent()) {}
       if (p->type() >= FL_WINDOW) break; // don't do the unmap
      }
      XUnmapWindow(fl_display, fl_xid(this));
    }
    break;
  }
  return Fl_Group::handle(event);
}

////////////////////////////////////////////////////////////////
// ~Fl_Widget() calls this: this function must get rid of any
// global pointers to the widget.  This is also called by hide()
// and deactivate().

// call this to free a selection (or change the owner):
void Fl::selection_owner(Fl_Widget *owner) {
  if (selection_owner_ && owner != selection_owner_)
    selection_owner_->handle(FL_SELECTIONCLEAR);
  if (focus_ && owner != focus_ && focus_ != selection_owner_)
    focus_->handle(FL_SELECTIONCLEAR); // clear non-X-selection highlight
  selection_owner_ = owner;
}

#include <FL/fl_draw.H>

void Fl_Widget::redraw() {damage(FL_DAMAGE_ALL);}

void Fl_Widget::damage(uchar flags) {
  if (type() < FL_WINDOW) {
    // damage only the rectangle covered by a child widget:
    damage(flags, x(), y(), w(), h());
  } else {
    // damage entire window by deleting the region:
    Fl_X* i = Fl_X::i((Fl_Window*)this);
    if (!i) return; // window not mapped, so ignore it
    if (i->region) {XDestroyRegion(i->region); i->region = 0;}
    damage_ |= flags;
    Fl::damage(FL_DAMAGE_CHILD);
  }
}

void Fl_Widget::damage(uchar flags, int X, int Y, int W, int H) {
  Fl_Widget* window = this;
  // mark all parent widgets between this and window with FL_DAMAGE_CHILD:
  while (window->type() < FL_WINDOW) {
    window->damage_ |= flags;
    window = window->parent();
    if (!window) return;
    flags = FL_DAMAGE_CHILD;
  }
  Fl_X* i = Fl_X::i((Fl_Window*)window);
  if (!i) return; // window not mapped, so ignore it

  if (X<=0 && Y<=0 && W>=window->w() && H>=window->h()) {
    // if damage covers entire window delete region:
    window->damage(flags);
    return;
  }

  // clip the damage to the window and quit if none:
  if (X < 0) {W += X; X = 0;}
  if (Y < 0) {H += Y; Y = 0;}
  if (W > window->w()-X) W = window->w()-X;
  if (H > window->h()-Y) H = window->h()-Y;
  if (W <= 0 || H <= 0) return;

  if (window->damage()) {
    // if we already have damage we must merge with existing region:
    if (i->region) {
#ifndef WIN32
      XRectangle R;
      R.x = X; R.y = Y; R.width = W; R.height = H;
      XUnionRectWithRegion(&R, i->region, i->region);
#else
      Region R = XRectangleRegion(X, Y, W, H);
      CombineRgn(i->region, i->region, R, RGN_OR);
      XDestroyRegion(R);
#endif
    }
    window->damage_ |= flags;
  } else {
    // create a new region:
    if (i->region) XDestroyRegion(i->region);
    i->region = XRectangleRegion(X,Y,W,H);
    window->damage_ = flags;
  }
  Fl::damage(FL_DAMAGE_CHILD);
}

void Fl_Window::flush() {
  make_current();
//if (damage() == FL_DAMAGE_EXPOSE && can_boxcheat(box())) fl_boxcheat = this;
  fl_clip_region(i->region); i->region = 0;
  draw();
}

//
// End of "$Id: Fl.cxx,v 1.24.2.41 2001/02/26 00:19:02 spitzak Exp $".
//
