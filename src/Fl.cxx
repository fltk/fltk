//
// "$Id: Fl.cxx,v 1.18 1999/02/19 14:26:30 mike Exp $"
//
// Main event handling code for the Fast Light Tool Kit (FLTK).
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

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/x.H>
#include <ctype.h>

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
char		*Fl::e_text = "";
int		Fl::e_length;

static double fl_elapsed();

//
// 'Fl:event_inside()' - Return whether or not the mouse event is inside
//                       the given rectangle.
//

int Fl::event_inside(int x,int y,int w,int h) /*const*/ {
  int mx = event_x() - x;
  int my = event_y() - y;
  return (mx >= 0 && mx < w && my >= 0 && my < h);
}

int Fl::event_inside(const Fl_Widget *o) /*const*/ {
  return event_inside(o->x(),o->y(),o->w(),o->h());
}

// Timeouts are insert-sorted into order.  This works good if there
// are only a small number:

#define MAXTIMEOUT 8

static struct {
  double time;
  void (*cb)(void*);
  void* arg;
} timeout[MAXTIMEOUT+1];
static int numtimeouts;

void Fl::add_timeout(double t, void (*cb)(void *), void *v) {
  int i;
  if (numtimeouts<MAXTIMEOUT) numtimeouts++;
  for (i=0; i<(numtimeouts-1); i++) {
    if (timeout[i].time > t) {
      for (int j=numtimeouts-1; j>i; j--) timeout[j] = timeout[j-1];
      break;
    }
  }
  timeout[i].time = t;
  timeout[i].cb = cb;
  timeout[i].arg = v;

  fl_elapsed();
}

void Fl::remove_timeout(void (*cb)(void *), void *v) {
  int i,j;
  for (i=j=0; i<numtimeouts; i++) {
    if (timeout[i].cb == cb && timeout[i].arg==v) ;
    else {if (j<i) timeout[j]=timeout[i]; j++;}
  }
  numtimeouts = j;
}

static void call_timeouts() {
  if (timeout[0].time > 0) return;
  struct {
    void (*cb)(void *);
    void *arg;
  } temp[MAXTIMEOUT];
  int i,j,k;
  // copy all expired timeouts to temp array:
  for (i=j=0; j<numtimeouts && timeout[j].time <= 0; i++,j++) {
    temp[i].cb = timeout[j].cb;
    temp[i].arg= timeout[j].arg;
  }
  // remove them from source array:
  for (k=0; j<numtimeouts;) timeout[k++] = timeout[j++];
  numtimeouts = k;
  // and then call them:
  for (k=0; k<i; k++) temp[k].cb(temp[k].arg);
}

void Fl::flush() {
  if (damage()) {
    damage_ = 0;
    for (Fl_X* x = Fl_X::first; x; x = x->next) {
      if (!x->wait_for_expose && x->w->damage() && x->w->visible()) {
	x->flush();
	x->w->clear_damage();
      }
    }
  }
#ifndef WIN32
  if (fl_display) XFlush(fl_display);
#endif
}

extern double fl_wait(int timeout_flag, double timeout);
extern int fl_ready();

static int initclock; // if false we didn't call fl_elapsed() last time

#ifndef WIN32
#include <sys/time.h>
#endif

// fl_elapsed must return the amount of time since the last time it was
// called.  To reduce the number of system calls the to get the
// current time, the "initclock" symbol is turned on by an indefinite
// wait.  This should then reset the measured-from time and return zero
static double fl_elapsed() {

#ifdef WIN32

  unsigned long newclock = fl_msg.time; // NOT YET IMPLEMENTED!
  const int TICKS_PER_SECOND = 1000; // divisor of the value to get seconds
  static unsigned long prevclock;
  if (!initclock) {prevclock = newclock; initclock = 1; return 0.0;}
  double t = double(newclock-prevclock)/TICKS_PER_SECOND;
  prevclock = newclock;

#else

  static struct timeval prevclock;
  struct timeval newclock;
  gettimeofday(&newclock, NULL);
  if (!initclock) {
    prevclock.tv_sec = newclock.tv_sec;
    prevclock.tv_usec = newclock.tv_usec;
    initclock = 1;
    return 0.0;
  }
  double t = newclock.tv_sec - prevclock.tv_sec +
    (newclock.tv_usec - prevclock.tv_usec)/1000000.0;
  prevclock.tv_sec = newclock.tv_sec;
  prevclock.tv_usec = newclock.tv_usec;

#endif

  // expire any timeouts:
  if (t > 0.0) for (int i=0; i<numtimeouts; i++) timeout[i].time -= t;
  return t;
}

void (*Fl::idle)();
static char in_idle;
static void callidle() {
  if (!Fl::idle || in_idle) return;
  in_idle = 1;
  Fl::idle();
  in_idle = 0;
}

int Fl::wait() {
  callidle();
  if (numtimeouts) {fl_elapsed(); call_timeouts();}
  flush();
  if (!Fl_X::first) return 0; // no windows
  if (idle && !in_idle)
    fl_wait(1,0.0);
  else if (numtimeouts)
    fl_wait(1, timeout[0].time);
  else {
    initclock = 0;
    fl_wait(0,0);
  }
  return 1;
}

double Fl::wait(double time) {
  callidle();
  if (numtimeouts) {time -= fl_elapsed(); call_timeouts();}
  flush();
  double wait_time = idle && !in_idle ? 0.0 : time;
  if (numtimeouts && timeout[0].time < wait_time) wait_time = timeout[0].time;
  fl_wait(1, wait_time);
  return time - fl_elapsed();
}

int Fl::check() {
  callidle();
  if (numtimeouts) {fl_elapsed(); call_timeouts();}
  flush();
  if (!Fl_X::first) return 0; // no windows
  fl_wait(1, 0.0);
  return 1;
}

int Fl::ready() {
  // if (idle && !in_idle) return 1; // should it do this?
  if (numtimeouts) {fl_elapsed(); if (timeout[0].time <= 0) return 1;}
  return fl_ready();
}

int Fl::run() {
  while (wait());
  return 0;
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

void Fl::redraw() {
  for (Fl_X* x = Fl_X::first; x; x = x->next) x->w->redraw();
}

Fl_Window* Fl::first_window() {Fl_X* x = Fl_X::first; return x ? x->w : 0;}

Fl_Window* Fl::next_window(const Fl_Window* w) {
  Fl_X* x = Fl_X::i(w)->next; return x ? x->w : 0;}

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
    event_is_click(0);
    belowmouse_ = o;
    for (; p && !p->contains(o); p = p->parent()) p->handle(FL_LEAVE);
  }
}

// Because mouse events are posted to the outermost window we need to
// adjust them for child windows if they are pushed().  This should also
// be done for the focus() but that is nyi.
static int mouse_dx;
static int mouse_dy;

void Fl::pushed(Fl_Widget *o) {
  pushed_ = o;
  mouse_dx = 0;
  mouse_dy = 0;
  if (o) for (Fl_Widget* w = o; w->parent(); w = w->parent()) {
    if (w->type()>=FL_WINDOW) {mouse_dx -= w->x(); mouse_dy -= w->y();}
  }
}

Fl_Window *fl_xfocus;	// which window X thinks has focus
Fl_Window *fl_xmousewin; // which window X thinks has FL_ENTER
Fl_Window *Fl::grab_;	// most recent Fl::grab()
Fl_Window *Fl::modal_;

// Update modal(), focus() and other state according to system state.
// This is called whenever a window is added or hidden, and whenever
// X says the focus or mouse window have changed, and when grab_ is
// changed.
void fl_fix_focus() {

  if (Fl::grab()) return; // don't do anything while grab is on.

  // set Fl::modal() based on any modal windows displayed:
  Fl_Window* w = Fl::first_window();
  while (w && w->parent()) w = Fl::next_window(w);
  Fl::modal_ = (w && w->modal()) ? w : 0;

  // set focus based on Fl::modal() and fl_xfocus
  w = fl_xfocus;
  while (w && w->parent()) w = w->window();
  if (w) {
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
	Fl::belowmouse(w); w->handle(FL_ENTER);}
    } else
      Fl::belowmouse(0);
  }
}

////////////////////////////////////////////////////////////////

void fl_send_extra_move() {
  // send a FL_MOVE event so the enter/leave state is up to date
  if (fl_xmousewin && !Fl::grab()) {
    Fl::e_x = Fl::e_x_root-fl_xmousewin->x();
    Fl::e_y = Fl::e_y_root-fl_xmousewin->y();
    fl_xmousewin->handle(FL_MOVE);
  }
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
    pushed_ = w; mouse_dx = mouse_dy = 0;
    if (w->handle(event)) return 1;
    // raise windows that are clicked on:
    window->show();
    return 1;

  case FL_MOVE:
  case FL_DRAG:
    // this should not happen if enter/leave events were reported
    // correctly by the system, but just in case:
    fl_xmousewin = window;
    if (pushed()) {
      w = pushed();
      event = FL_DRAG;
      e_x += mouse_dx;
      e_y += mouse_dy;
    } else if (modal() && w != modal()) {
      w = 0;
    }
    if (grab()) w = grab();
    break;

  case FL_RELEASE: {
    if (pushed()) {
      w = pushed();
      pushed_ = 0; // must be zero before callback is done!
      e_x += mouse_dx;
      e_y += mouse_dy;
    }
    if (grab()) w = grab();
    int r = w->handle(event);
    fl_fix_focus();
    fl_send_extra_move();
    return r;}

  case FL_UNFOCUS:
    window = 0;
  case FL_FOCUS:
    fl_xfocus = window;
    e_keysym = 0; // make sure it is not confused with navigation key
    fl_fix_focus();
    return 1;

  case FL_KEYBOARD:

    // this should not happen if focus/unfocus events were reported
    // correctly by the system, but just in case:
    fl_xfocus = window;
    // Try it as keystroke, sending it to focus and all parents:
    for (w = grab() ? grab() : focus(); w; w = w->parent())
      if (w->handle(FL_KEYBOARD)) return 1;

    // recursive call to try shortcut:
    if (handle(FL_SHORTCUT, window)) return 1;

    // and then try a shortcut with the case of the text swapped:
    if (!isalpha(event_text()[0])) return 0;

    // swap the case and fall through to FL_SHORTCUT case:
    *(char*)(event_text()) ^= ('A'^'a');
    event = FL_SHORTCUT;

  case FL_SHORTCUT:

    if (grab()) {w = grab(); break;} // send it to grab window

    // Try it as shortcut, sending to mouse widget and all parents:
    w = belowmouse(); if (!w) {w = modal(); if (!w) w = window;}
    for (; w; w = w->parent()) if (w->handle(FL_SHORTCUT)) return 1;

    // try using add_handle() functions:
    if (send_handlers(FL_SHORTCUT)) return 1;

    // make Escape key close windows:
    if (event_key()==FL_Escape) {
      window->do_callback();
      return 1;
    }

    return 0;

  case FL_ENTER:
    fl_xmousewin = window; fl_fix_focus();
    return 1;

  case FL_LEAVE:
    if (window == fl_xmousewin) {fl_xmousewin = 0; fl_fix_focus();}
    return 1;

  default:
    break;
  }
  if (w && w->handle(event)) return 1;
  return send_handlers(event);
}

////////////////////////////////////////////////////////////////
// hide() destroys the X window, it does not do unmap!

void fl_throw_focus(Fl_Widget*); // in Fl_x.C

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

  // Make sure no events are sent to this window:
  if (this == fl_xmousewin) fl_xmousewin = 0;
  if (this == fl_xfocus) fl_xfocus = 0;
  fl_throw_focus(this);
  handle(FL_HIDE);

#ifdef WIN32
  if (x->private_dc) ReleaseDC(x->xid,x->private_dc);
  if (x->xid == fl_window) fl_GetDC(0); // releases dc belonging to window
#else
  if (x->region) XDestroyRegion(x->region);
#endif
  XDestroyWindow(fl_display, x->xid);

  delete x;
}

Fl_Window::~Fl_Window() {
  hide();
}

// Child windows must respond to FL_SHOW and FL_HIDE by actually
// doing unmap operations.  Outer windows assumme FL_SHOW & FL_HIDE
// are messages from X:

int Fl_Window::handle(int event) {
  if (parent()) switch (event) {
  case FL_SHOW:
    if (!shown()) show();
    else XMapWindow(fl_display, fl_xid(this));
    break;
  case FL_HIDE:
    if (shown()) XUnmapWindow(fl_display, fl_xid(this));
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
  selection_owner_ = owner;
}

#ifndef WIN32
Fl_Widget *fl_selection_requestor; // from Fl_cutpaste.C
#endif

void fl_throw_focus(Fl_Widget *o) {
  if (o->contains(Fl::pushed())) Fl::pushed(0);
  if (o->contains(Fl::selection_owner())) Fl::selection_owner(0);
#ifndef WIN32
  if (o->contains(fl_selection_requestor)) fl_selection_requestor = 0;
#endif
  int fix = 0;
  if (o->contains(Fl::belowmouse())) {Fl::belowmouse(0); fix = 1;}
  if (o->contains(Fl::focus())) {Fl::focus(0); fix = 1;}
  if (fix) fl_fix_focus();
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

int fl_old_shortcut(const char* s) {
  if (!s || !*s) return 0;
  int n = 0;
  if (*s == '#') {n |= FL_ALT; s++;}
  if (*s == '+') {n |= FL_SHIFT; s++;}
  if (*s == '^') {n |= FL_CTRL; s++;}
  return n | *s;
}

//
// End of "$Id: Fl.cxx,v 1.18 1999/02/19 14:26:30 mike Exp $".
//
