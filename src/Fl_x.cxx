//
// "$Id: Fl_x.cxx,v 1.24.2.24 2001/04/27 14:39:27 easysw Exp $"
//
// X specific code for the Fast Light Tool Kit (FLTK).
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

#ifdef WIN32
#include "Fl_win32.cxx"
#else

#define CONSOLIDATE_MOTION 1
/**** Define this if your keyboard lacks a backspace key... ****/
/* #define BACKSPACE_HACK 1 */

#include <config.h>
#include <FL/Fl.H>
#include <FL/x.H>
#include <FL/Fl_Window.H>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

////////////////////////////////////////////////////////////////
// interface to poll/select call:

#if USE_POLL

#include <poll.h>
static pollfd *pollfds = 0;

#else

#if HAVE_SYS_SELECT_H
#  include <sys/select.h>
#endif /* HAVE_SYS_SELECT_H */

// The following #define is only needed for HP-UX 9.x and earlier:
//#define select(a,b,c,d,e) select((a),(int *)(b),(int *)(c),(int *)(d),(e))

static fd_set fdsets[3];
static int maxfd;
#define POLLIN 1
#define POLLOUT 4
#define POLLERR 8

#endif /* USE_POLL */

static int nfds = 0;
static int fd_array_size = 0;
struct FD {
#if !USE_POLL
  int fd;
  short events;
#endif
  void (*cb)(int, void*);
  void* arg;
};

static FD *fd = 0;

void Fl::add_fd(int n, int events, void (*cb)(int, void*), void *v) {
  remove_fd(n,events);
  int i = nfds++;
  if (i >= fd_array_size) {
    FD *temp;
    fd_array_size = 2*fd_array_size+1;

    if (!fd) temp = (FD*)malloc(fd_array_size*sizeof(FD));
    else temp = (FD*)realloc(fd, fd_array_size*sizeof(FD));

    if (!temp) return;
    fd = temp;

#if USE_POLL
    pollfd *tpoll;

    if (!pollfds) tpoll = (pollfd*)malloc(fd_array_size*sizeof(pollfd));
    else tpoll = (pollfd*)realloc(pollfds, fd_array_size*sizeof(pollfd));

    if (!tpoll) return;
    pollfds = tpoll;
#endif
  }
  fd[i].cb = cb;
  fd[i].arg = v;
#if USE_POLL
  pollfds[i].fd = n;
  pollfds[i].events = events;
#else
  fd[i].fd = n;
  fd[i].events = events;
  if (events & POLLIN) FD_SET(n, &fdsets[0]);
  if (events & POLLOUT) FD_SET(n, &fdsets[1]);
  if (events & POLLERR) FD_SET(n, &fdsets[2]);
  if (n > maxfd) maxfd = n;
#endif
}

void Fl::add_fd(int fd, void (*cb)(int, void*), void* v) {
  Fl::add_fd(fd, POLLIN, cb, v);
}

void Fl::remove_fd(int n, int events) {
  int i,j;
  for (i=j=0; i<nfds; i++) {
#if USE_POLL
    if (pollfds[i].fd == n) {
      int e = pollfds[i].events & ~events;
      if (!e) continue; // if no events left, delete this fd
      pollfds[j].events = e;
    }
#else
    if (fd[i].fd == n) {
      int e = fd[i].events & ~events;
      if (!e) continue; // if no events left, delete this fd
      fd[i].events = e;
    }
#endif
    // move it down in the array if necessary:
    if (j<i) {
      fd[j] = fd[i];
#if USE_POLL
      pollfds[j] = pollfds[i];
#endif
    }
    j++;
  }
  nfds = j;
#if !USE_POLL
  if (events & POLLIN) FD_CLR(n, &fdsets[0]);
  if (events & POLLOUT) FD_CLR(n, &fdsets[1]);
  if (events & POLLERR) FD_CLR(n, &fdsets[2]);
  if (n == maxfd) maxfd--;
#endif
}

void Fl::remove_fd(int n) {
  remove_fd(n, -1);
}

#if CONSOLIDATE_MOTION
static Fl_Window* send_motion;
extern Fl_Window* fl_xmousewin;
#endif
static void do_queued_events() {
 while (XEventsQueued(fl_display,QueuedAfterReading)) {
    XEvent xevent;
    XNextEvent(fl_display, &xevent);
    fl_handle(xevent);
  }
#if CONSOLIDATE_MOTION
  if (send_motion && send_motion == fl_xmousewin) {
    send_motion = 0;
    Fl::handle(FL_MOVE, fl_xmousewin);
  }
#endif
}

// This is never called with time_to_wait < 0.0:
// It should return negative on error, 0 if nothing happens before
// timeout, and >0 if any callbacks were done.
int fl_wait(double time_to_wait) {

  // OpenGL and other broken libraries call XEventsQueued
  // unnecessarily and thus cause the file descriptor to not be ready,
  // so we must check for already-read events:
  if (fl_display && XQLength(fl_display)) {do_queued_events(); return 1;}

#if !USE_POLL
  fd_set fdt[3];
  fdt[0] = fdsets[0];
  fdt[1] = fdsets[1];
  fdt[2] = fdsets[2];
#endif
  int n;

  if (time_to_wait < 2147483.648) {
#if USE_POLL
    n = ::poll(pollfds, nfds, int(time_to_wait*1000 + .5));
#else
    timeval t;
    t.tv_sec = int(time_to_wait);
    t.tv_usec = int(1000000 * (time_to_wait-t.tv_sec));
    n = ::select(maxfd+1,&fdt[0],&fdt[1],&fdt[2],&t);
#endif
  } else {
#if USE_POLL
    n = ::poll(pollfds, nfds, -1);
#else
    n = ::select(maxfd+1,&fdt[0],&fdt[1],&fdt[2],0);
#endif
  }
  if (n > 0) {
    for (int i=0; i<nfds; i++) {
#if USE_POLL
      if (pollfds[i].revents) fd[i].cb(pollfds[i].fd, fd[i].arg);
#else
      int f = fd[i].fd;
      short revents = 0;
      if (FD_ISSET(f,&fdt[0])) revents |= POLLIN;
      if (FD_ISSET(f,&fdt[1])) revents |= POLLOUT;
      if (FD_ISSET(f,&fdt[2])) revents |= POLLERR;
      if (fd[i].events & revents) fd[i].cb(f, fd[i].arg);
#endif
    }
  }
  return n;
}

// fl_ready() is just like fl_wait(0.0) except no callbacks are done:
int fl_ready() {
  if (XQLength(fl_display)) return 1;
#if USE_POLL
  return ::poll(pollfds, nfds, 0);
#else
  timeval t;
  t.tv_sec = 0;
  t.tv_usec = 0;
  fd_set fdt[3];
  fdt[0] = fdsets[0];
  fdt[1] = fdsets[1];
  fdt[2] = fdsets[2];
  return ::select(maxfd+1,&fdt[0],&fdt[1],&fdt[2],&t);
#endif
}

////////////////////////////////////////////////////////////////

Display *fl_display;
int fl_screen;
XVisualInfo *fl_visual;
Colormap fl_colormap;

static Atom wm_delete_window;
static Atom wm_protocols;
static Atom _motif_wm_hints;

static void fd_callback(int,void *) {
  do_queued_events();
}

extern "C" {
  static int io_error_handler(Display*) {
    Fl::fatal("X I/O error");
    return 0;
  }

  static int xerror_handler(Display* d, XErrorEvent* e) {
    char buf1[128], buf2[128];
    sprintf(buf1, "XRequest.%d", e->request_code);
    XGetErrorDatabaseText(d,"",buf1,buf1,buf2,128);
    XGetErrorText(d, e->error_code, buf1, 128);
    Fl::warning("%s: %s 0x%lx", buf2, buf1, e->resourceid);
    return 0;
  }
}

void fl_open_display() {
  if (fl_display) return;

  XSetIOErrorHandler(io_error_handler);
  XSetErrorHandler(xerror_handler);

  Display *d = XOpenDisplay(0);
  if (!d) Fl::fatal("Can't open display: %s",XDisplayName(0));

  fl_open_display(d);
}

void fl_open_display(Display* d) {
  fl_display = d;

  wm_delete_window = XInternAtom(d,"WM_DELETE_WINDOW",0);
  wm_protocols = XInternAtom(d,"WM_PROTOCOLS",0);
  _motif_wm_hints = XInternAtom(d,"_MOTIF_WM_HINTS",0);
  Fl::add_fd(ConnectionNumber(d), POLLIN, fd_callback);

  fl_screen = DefaultScreen(fl_display);
// construct an XVisualInfo that matches the default Visual:
  XVisualInfo templt; int num;
  templt.visualid = XVisualIDFromVisual(DefaultVisual(fl_display,fl_screen));
  fl_visual = XGetVisualInfo(fl_display, VisualIDMask, &templt, &num);
  fl_colormap = DefaultColormap(fl_display,fl_screen);

#if !USE_COLORMAP
  Fl::visual(FL_RGB);
#endif
}

void fl_close_display() {
  Fl::remove_fd(ConnectionNumber(fl_display));
  XCloseDisplay(fl_display);
}

int Fl::h() {
  fl_open_display();
  return DisplayHeight(fl_display,fl_screen);
}

int Fl::w() {
  fl_open_display();
  return DisplayWidth(fl_display,fl_screen);
}

void Fl::get_mouse(int &x, int &y) {
  fl_open_display();
  Window root = RootWindow(fl_display, fl_screen);
  Window c; int mx,my,cx,cy; unsigned int mask;
  XQueryPointer(fl_display,root,&root,&c,&mx,&my,&cx,&cy,&mask);
  x = mx;
  y = my;
}

////////////////////////////////////////////////////////////////

const XEvent* fl_xevent; // the current x event
ulong fl_event_time; // the last timestamp from an x event

char fl_key_vector[32]; // used by Fl::get_key()

// Record event mouse position and state from an XEvent:

static int px, py;
static ulong ptime;

static void set_event_xy() {
#if CONSOLIDATE_MOTION
  send_motion = 0;
#endif
  Fl::e_x_root = fl_xevent->xbutton.x_root;
  Fl::e_x = fl_xevent->xbutton.x;
  Fl::e_y_root = fl_xevent->xbutton.y_root;
  Fl::e_y = fl_xevent->xbutton.y;
  Fl::e_state = fl_xevent->xbutton.state << 16;
  fl_event_time = fl_xevent->xbutton.time;
#ifdef __sgi
  // get the meta key off PC keyboards:
  if (fl_key_vector[18]&0x18) Fl::e_state |= FL_META;
#endif
  // turn off is_click if enough time or mouse movement has passed:
  if (abs(Fl::e_x_root-px)+abs(Fl::e_y_root-py) > 3 ||
      fl_event_time >= ptime+1000)
    Fl::e_is_click = 0;
}

// if this is same event as last && is_click, increment click count:
static inline void checkdouble() {
  if (Fl::e_is_click == Fl::e_keysym)
    Fl::e_clicks++;
  else {
    Fl::e_clicks = 0;
    Fl::e_is_click = Fl::e_keysym;
  }
  px = Fl::e_x_root;
  py = Fl::e_y_root;
  ptime = fl_event_time;
}

static Fl_Window* resize_bug_fix;

////////////////////////////////////////////////////////////////

int fl_handle(const XEvent& xevent)
{
  fl_xevent = &xevent;
  Window xid = xevent.xany.window;

  switch (xevent.type) {

  // events where we don't care about window:

  case KeymapNotify:
    memcpy(fl_key_vector, xevent.xkeymap.key_vector, 32);
    return 0;

  case MappingNotify:
    XRefreshKeyboardMapping((XMappingEvent*)&xevent.xmapping);
    return 0;

  // events where interesting window id is in a different place:
  case CirculateNotify:
  case CirculateRequest:
  case ConfigureNotify:
  case ConfigureRequest:
  case CreateNotify:
  case DestroyNotify:
  case GravityNotify:
  case MapNotify:
  case MapRequest:
  case ReparentNotify:
  case UnmapNotify:
    xid = xevent.xmaprequest.window;
    break;
  }

  int event = 0;
  Fl_Window* window = fl_find(xid);

  if (window) switch (xevent.type) {

  case ClientMessage:
    if ((Atom)(xevent.xclient.data.l[0]) == wm_delete_window) event = FL_CLOSE;
    break;

  case MapNotify:
    event = FL_SHOW;
    break;

  case UnmapNotify:
    event = FL_HIDE;
    break;

  case Expose:
    Fl_X::i(window)->wait_for_expose = 0;
#if 0
    // try to keep windows on top even if WM_TRANSIENT_FOR does not work:
    // opaque move/resize window managers do not like this, so I disabled it.
    if (Fl::first_window()->non_modal() && window != Fl::first_window())
      Fl::first_window()->show();
#endif

  case GraphicsExpose:
    window->damage(FL_DAMAGE_EXPOSE, xevent.xexpose.x, xevent.xexpose.y,
		   xevent.xexpose.width, xevent.xexpose.height);
    return 1;

  case ButtonPress:
    Fl::e_keysym = FL_Button + xevent.xbutton.button;
    set_event_xy(); checkdouble();
    Fl::e_state |= (FL_BUTTON1 << (xevent.xbutton.button-1));
    event = FL_PUSH;
    break;

  case MotionNotify:
    set_event_xy();
#if CONSOLIDATE_MOTION
    send_motion = fl_xmousewin = window;
    return 0;
#else
    event = FL_MOVE;
    break;
#endif

  case ButtonRelease:
    Fl::e_keysym = FL_Button + xevent.xbutton.button;
    set_event_xy();
    Fl::e_state &= ~(FL_BUTTON1 << (xevent.xbutton.button-1));
    event = FL_RELEASE;
    break;

  case FocusIn:
    event = FL_FOCUS;
    break;

  case FocusOut:
    event = FL_UNFOCUS;
    break;

  case KeyPress: {
    int keycode = xevent.xkey.keycode;
    fl_key_vector[keycode/8] |= (1 << (keycode%8));
    static char buffer[21];
    KeySym keysym;
    //static XComposeStatus compose;
    int len = XLookupString((XKeyEvent*)&(xevent.xkey),
			    buffer, 20, &keysym, 0/*&compose*/);
    if (keysym && keysym < 0x400) { // a character in latin-1,2,3,4 sets
      // force it to type a character (not sure if this ever is needed):
      if (!len) {buffer[0] = char(keysym); len = 1;}
      // ignore all effects of shift on the keysyms, which makes it a lot
      // easier to program shortcuts and is Windoze-compatable:
      keysym = XKeycodeToKeysym(fl_display, keycode, 0);
    }
#ifdef __sgi
    // You can plug a microsoft keyboard into an sgi but the extra shift
    // keys are not translated.  Make them translate like XFree86 does:
    if (!keysym) switch(keycode) {
    case 147: keysym = FL_Meta_L; break;
    case 148: keysym = FL_Meta_R; break;
    case 149: keysym = FL_Menu; break;
    }
#endif
#if BACKSPACE_HACK
    // Attempt to fix keyboards that send "delete" for the key in the
    // upper-right corner of the main keyboard.  But it appears that
    // very few of these remain?
    static int got_backspace;
    if (!got_backspace) {
      if (keysym == FL_Delete) keysym = FL_BackSpace;
      else if (keysym == FL_BackSpace) got_backspace = 1;
    }
#endif
    // We have to get rid of the XK_KP_function keys, because they are
    // not produced on Windoze and thus case statements tend not to check
    // for them.  There are 15 of these in the range 0xff91 ... 0xff9f
    if (keysym >= 0xff91 && keysym <= 0xff9f) {
      // Try to make them turn into FL_KP+'c' so that NumLock is
      // irrelevant, by looking at the shifted code.  This matches the
      // behavior of the translator in Fl_win32.C, and IMHO is the
      // user-friendly result:
      unsigned long keysym1 = XKeycodeToKeysym(fl_display, keycode, 1);
      if (keysym1 <= 0x7f || keysym1 > 0xff9f && keysym1 <= FL_KP_Last) {
	keysym = keysym1 | FL_KP;
	buffer[0] = char(keysym1) & 0x7F;
	len = 1;
      } else {
	// If that failed to work, just translate them to the matching
	// normal function keys:
	static const unsigned short table[15] = {
	  FL_F+1, FL_F+2, FL_F+3, FL_F+4,
	  FL_Home, FL_Left, FL_Up, FL_Right,
	  FL_Down, FL_Page_Up, FL_Page_Down, FL_End,
	  0xff0b/*XK_Clear*/, FL_Insert, FL_Delete};
	keysym = table[keysym-0xff91];
      }
    }
    buffer[len] = 0;
    Fl::e_keysym = int(keysym);
    Fl::e_text = buffer;
    Fl::e_length = len;
    set_event_xy();
    Fl::e_is_click = 0;
    if (Fl::event_state(FL_CTRL) && keysym == '-') buffer[0] = 0x1f; // ^_
    event = FL_KEYBOARD;
    break;}

  case KeyRelease: {
    int keycode = xevent.xkey.keycode;
    fl_key_vector[keycode/8] &= ~(1 << (keycode%8));
    set_event_xy();}
    break;

  case EnterNotify:
    if (xevent.xcrossing.detail == NotifyInferior) break;
    // XInstallColormap(fl_display, Fl_X::i(window)->colormap);
    set_event_xy();
    Fl::e_state = xevent.xcrossing.state << 16;
    event = FL_ENTER;
    break;

  case LeaveNotify:
    if (xevent.xcrossing.detail == NotifyInferior) break;
    set_event_xy();
    Fl::e_state = xevent.xcrossing.state << 16;
    event = FL_LEAVE;
    break;

  case ConfigureNotify: {
    // We cannot rely on the x,y position in the configure notify event.
    // I now think this is an unavoidable problem with X: it is impossible
    // for a window manager to prevent the "real" notify event from being
    // sent when it resizes the contents, even though it can send an
    // artificial event with the correct position afterwards (and some
    // window managers do not send this fake event anyway)
    // So anyway, do a round trip to find the correct x,y:
    Window r, c; int X, Y, wX, wY; unsigned int m;
    XQueryPointer(fl_display, fl_xid(window), &r, &c, &X, &Y, &wX, &wY, &m);
    resize_bug_fix = window;
    window->resize(X-wX, Y-wY,
		   xevent.xconfigure.width, xevent.xconfigure.height);
    return 1;}
  }

  return Fl::handle(event, window);
}

////////////////////////////////////////////////////////////////

void Fl_Window::resize(int X,int Y,int W,int H) {
  int is_a_resize = (W != w() || H != h());
  int resize_from_program = (this != resize_bug_fix);
  if (!resize_from_program) resize_bug_fix = 0;
  if (X != x() || Y != y()) set_flag(FL_FORCE_POSITION);
  else if (!is_a_resize) return;
  if (is_a_resize) {
    Fl_Group::resize(X,Y,W,H);
    if (shown()) {redraw(); i->wait_for_expose = 1;}
  } else {
    x(X); y(Y);
  }
  if (resize_from_program && shown()) {
    if (is_a_resize) {
      if (!resizable()) size_range(w(),h(),w(),h());
      XMoveResizeWindow(fl_display, i->xid, X, Y, W>0 ? W : 1, H>0 ? H : 1);
    } else
      XMoveWindow(fl_display, i->xid, X, Y);
  }
}

////////////////////////////////////////////////////////////////

// A subclass of Fl_Window may call this to associate an X window it
// creates with the Fl_Window:

void fl_fix_focus(); // in Fl.cxx

Fl_X* Fl_X::set_xid(Fl_Window* w, Window xid) {
  Fl_X* x = new Fl_X;
  x->xid = xid;
  x->other_xid = 0;
  x->setwindow(w);
  x->next = Fl_X::first;
  x->region = 0;
  x->wait_for_expose = 1;
  Fl_X::first = x;
  if (w->modal()) {Fl::modal_ = w; fl_fix_focus();}
  return x;
}

// More commonly a subclass calls this, because it hides the really
// ugly parts of X and sets all the stuff for a window that is set
// normally.  The global variables like fl_show_iconic are so that
// subclasses of *that* class may change the behavior...

char fl_show_iconic;	// hack for iconize()
int fl_background_pixel = -1; // hack to speed up bg box drawing
int fl_disable_transient_for; // secret method of removing TRANSIENT_FOR

static const int childEventMask = ExposureMask;

static const int XEventMask =
ExposureMask|StructureNotifyMask
|KeyPressMask|KeyReleaseMask|KeymapStateMask|FocusChangeMask
|ButtonPressMask|ButtonReleaseMask
|EnterWindowMask|LeaveWindowMask
|PointerMotionMask;

void Fl_X::make_xid(Fl_Window* w, XVisualInfo *visual, Colormap colormap)
{
  Fl_Group::current(0); // get rid of very common user bug: forgot end()

  int X = w->x();
  int Y = w->y();
  int W = w->w();
  if (W <= 0) W = 1; // X don't like zero...
  int H = w->h();
  if (H <= 0) H = 1; // X don't like zero...
  if (!w->parent() && !Fl::grab()) {
    // center windows in case window manager does not do anything:
    if (!(w->flags() & Fl_Window::FL_FORCE_POSITION)) {
      w->x(X = (Fl::w()-W)/2);
      w->y(Y = (Fl::h()-H)/2);
    }
    // force the window to be on-screen.  Usually the X window manager
    // does this, but a few don't, so we do it here for consistency:
    if (w->border()) {
      // ensure border is on screen:
      // (assumme extremely minimal dimensions for this border)
      const int top = 20;
      const int left = 1;
      const int right = 1;
      const int bottom = 1;
      if (X+W+right > Fl::w()) X = Fl::w()-right-W;
      if (X-left < 0) X = left;
      if (Y+H+bottom > Fl::h()) Y = Fl::h()-bottom-H;
      if (Y-top < 0) Y = top;
    }
    // now insure contents are on-screen (more important than border):
    if (X+W > Fl::w()) X = Fl::w()-W;
    if (X < 0) X = 0;
    if (Y+H > Fl::h()) Y = Fl::h()-H;
    if (Y < 0) Y = 0;
  }

  ulong root = w->parent() ?
    fl_xid(w->window()) : RootWindow(fl_display, fl_screen);

  XSetWindowAttributes attr;
  int mask = CWBorderPixel|CWColormap|CWEventMask|CWBitGravity;
  attr.event_mask = w->parent() ? childEventMask : XEventMask;
  attr.colormap = colormap;
  attr.border_pixel = 0;
  attr.bit_gravity = 0; // StaticGravity;
  attr.override_redirect = 0;
  if (Fl::grab()) {
    attr.save_under = 1; mask |= CWSaveUnder;
    if (!w->border()) {attr.override_redirect = 1; mask |= CWOverrideRedirect;}
  }
  if (fl_background_pixel >= 0) {
    attr.background_pixel = fl_background_pixel;
    fl_background_pixel = -1;
    mask |= CWBackPixel;
  }

  Fl_X* x =
    set_xid(w, XCreateWindow(fl_display,
			     root,
			     X, Y, W, H,
			     0, // borderwidth
			     visual->depth,
			     InputOutput,
			     visual->visual,
			     mask, &attr));
  int showit = 1;

  if (!w->parent() && !attr.override_redirect) {
    // Communicate all kinds 'o junk to the X Window Manager:

    w->label(w->label(), w->iconlabel());

    XChangeProperty(fl_display, x->xid, wm_protocols,
 		    XA_ATOM, 32, 0, (uchar*)&wm_delete_window, 1);

    // send size limits and border:
    x->sendxjunk();

    // set the class property, which controls the icon used:
    if (w->xclass()) {
      char buffer[1024];
      char *p; const char *q;
      // truncate on any punctuation, because they break XResource lookup:
      for (p = buffer, q = w->xclass(); isalnum(*q)||(*q&128);) *p++ = *q++;
      *p++ = 0;
      // create the capitalized version:
      q = buffer;
      *p = toupper(*q++); if (*p++ == 'X') *p++ = toupper(*q++);
      while ((*p++ = *q++));
      XChangeProperty(fl_display, x->xid, XA_WM_CLASS, XA_STRING, 8, 0,
		      (unsigned char *)buffer, p-buffer-1);
    }

    if (w->non_modal() && x->next && !fl_disable_transient_for) {
      // find some other window to be "transient for":
      Fl_Window* w = x->next->w;
      while (w->parent()) w = w->window();
      XSetTransientForHint(fl_display, x->xid, fl_xid(w));
      if (!w->visible()) showit = 0; // guess that wm will not show it
    }

    XWMHints hints;
    hints.input = True;
    hints.flags = InputHint;
    if (fl_show_iconic) {
      hints.flags |= StateHint;
      hints.initial_state = IconicState;
      fl_show_iconic = 0;
      showit = 0;
    }
    if (w->icon()) {
      hints.icon_pixmap = (Pixmap)w->icon();
      hints.flags       |= IconPixmapHint;
    }
    XSetWMHints(fl_display, x->xid, &hints);
  }

  XMapWindow(fl_display, x->xid);
  if (showit) {
    w->set_visible();
    w->handle(FL_SHOW); // get child windows to appear
    w->redraw();
  }
}

////////////////////////////////////////////////////////////////
// Send X window stuff that can be changed over time:

void Fl_X::sendxjunk() {
  if (w->parent()) return; // it's not a window manager window!

  if (!w->size_range_set) { // default size_range based on resizable():
    if (w->resizable()) {
      Fl_Widget *o = w->resizable();
      int minw = o->w(); if (minw > 100) minw = 100;
      int minh = o->h(); if (minh > 100) minh = 100;
      w->size_range(w->w() - o->w() + minw, w->h() - o->h() + minh, 0, 0);
    } else {
      w->size_range(w->w(), w->h(), w->w(), w->h());
    }
    return; // because this recursively called here
  }

  XSizeHints hints;
  // memset(&hints, 0, sizeof(hints)); jreiser suggestion to fix purify?
  hints.min_width = w->minw;
  hints.min_height = w->minh;
  hints.max_width = w->maxw;
  hints.max_height = w->maxh;
  hints.width_inc = w->dw;
  hints.height_inc = w->dh;
  hints.win_gravity = StaticGravity;

  // see the file /usr/include/X11/Xm/MwmUtil.h:
  // fill all fields to avoid bugs in kwm and perhaps other window managers:
  // 0, MWM_FUNC_ALL, MWM_DECOR_ALL
  long prop[5] = {0, 1, 1, 0, 0};

  if (hints.min_width != hints.max_width ||
      hints.min_height != hints.max_height) { // resizable
    hints.flags = PMinSize|PWinGravity;
    if (hints.max_width >= hints.min_width ||
	hints.max_height >= hints.min_height) {
      hints.flags = PMinSize|PMaxSize|PWinGravity;
      // unfortunately we can't set just one maximum size.  Guess a
      // value for the other one.  Some window managers will make the
      // window fit on screen when maximized, others will put it off screen:
      if (hints.max_width < hints.min_width) hints.max_width = Fl::w();
      if (hints.max_height < hints.min_height) hints.max_height = Fl::h();
    }
    if (hints.width_inc && hints.height_inc) hints.flags |= PResizeInc;
    if (w->aspect) {
      // stupid X!  It could insist that the corner go on the
      // straight line between min and max...
      hints.min_aspect.x = hints.max_aspect.x = hints.min_width;
      hints.min_aspect.y = hints.max_aspect.y = hints.min_height;
      hints.flags |= PAspect;
    }
  } else { // not resizable:
    hints.flags = PMinSize|PMaxSize|PWinGravity;
    prop[0] = 1; // MWM_HINTS_FUNCTIONS
    prop[1] = 1|2|16; // MWM_FUNC_ALL | MWM_FUNC_RESIZE | MWM_FUNC_MAXIMIZE
  }

  if (w->flags() & Fl_Window::FL_FORCE_POSITION) {
    hints.flags |= USPosition;
    hints.x = w->x();
    hints.y = w->y();
  }

  if (!w->border()) {
    prop[0] |= 2; // MWM_HINTS_DECORATIONS
    prop[2] = 0; // no decorations
  }

  XSetWMNormalHints(fl_display, xid, &hints);
  XChangeProperty(fl_display, xid,
		  _motif_wm_hints, _motif_wm_hints,
		  32, 0, (unsigned char *)prop, 5);
}

void Fl_Window::size_range_() {
  size_range_set = 1;
  if (shown()) i->sendxjunk();
}

////////////////////////////////////////////////////////////////

// returns pointer to the filename, or null if name ends with '/'
const char *filename_name(const char *name) {
  const char *p,*q;
  for (p=q=name; *p;) if (*p++ == '/') q = p;
  return q;
}

void Fl_Window::label(const char *name,const char *iname) {
  Fl_Widget::label(name);
  iconlabel_ = iname;
  if (shown() && !parent()) {
    if (!name) name = "";
    XChangeProperty(fl_display, i->xid, XA_WM_NAME,
		    XA_STRING, 8, 0, (uchar*)name, strlen(name));
    if (!iname) iname = filename_name(name);
    XChangeProperty(fl_display, i->xid, XA_WM_ICON_NAME, 
		    XA_STRING, 8, 0, (uchar*)iname, strlen(iname));
  }
}

////////////////////////////////////////////////////////////////
// Implement the virtual functions for the base Fl_Window class:

// If the box is a filled rectangle, we can make the redisplay *look*
// faster by using X's background pixel erasing.  We can make it
// actually *be* faster by drawing the frame only, this is done by
// setting fl_boxcheat, which is seen by code in fl_drawbox.C:
//
// On XFree86 (and prehaps all X's) this has a problem if the window
// is resized while a save-behind window is atop it.  The previous
// contents are restored to the area, but this assummes the area
// is cleared to background color.  So this is disabled in this version.
// Fl_Window *fl_boxcheat;
static inline int can_boxcheat(uchar b) {return (b==1 || (b&2) && b<=15);}

void Fl_Window::show() {
  if (!shown()) {
    fl_open_display();
    if (can_boxcheat(box())) fl_background_pixel = int(fl_xpixel(color()));
    Fl_X::make_xid(this);
  } else {
    XMapRaised(fl_display, i->xid);
  }
}

Window fl_window;
Fl_Window *Fl_Window::current_;
GC fl_gc;

// make X drawing go into this window (called by subclass flush() impl.)
void Fl_Window::make_current() {
  static GC gc;	// the GC used by all X windows
  if (!gc) gc = XCreateGC(fl_display, i->xid, 0, 0);
  fl_window = i->xid;
  fl_gc = gc;
  current_ = this;
  fl_clip_region(0);
}

#endif

//
// End of "$Id: Fl_x.cxx,v 1.24.2.24 2001/04/27 14:39:27 easysw Exp $".
//
