// x.H

// These are internal fltk symbols that are necessary or useful for
// calling Xlib.  You should include this file if (and ONLY if) you
// need to call Xlib directly.  These symbols may not exist on non-X
// systems.

#ifndef Fl_X_H
#define Fl_X_H

#include "Enumerations.H"

#ifdef WIN32

#include "win32.H"

#else

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include "Fl_Window.H"

void fl_open_display();
void fl_close_display();

// constant info about the X server connection:
extern Display *fl_display;
extern int fl_screen;
extern XVisualInfo *fl_visual;
extern Colormap fl_colormap;

// drawing functions:
extern GC fl_gc;
extern Window fl_window;
ulong fl_xpixel(Fl_Color i);
ulong fl_xpixel(uchar r, uchar g, uchar b);
void fl_clip_region(Region);

// feed events into fltk:
int fl_handle(const XEvent&);

// you can use these in Fl::add_handler() to look at events:
extern const XEvent* fl_xevent;
extern ulong fl_event_time;

// off-screen pixmaps: create, destroy, draw into, copy to window:
#define Fl_Offscreen ulong
#define fl_create_offscreen(w,h) \
  XCreatePixmap(fl_display, fl_window, w, h, fl_visual->depth)
// begin/end are macros that save the old state in local variables:
#define fl_begin_offscreen(pixmap) \
  Window _sw=fl_window; fl_window=pixmap; fl_push_no_clip()
#define fl_end_offscreen() \
  fl_pop_clip(); fl_window = _sw

#define fl_copy_offscreen(x,y,w,h,pixmap,srcx,srcy) \
  XCopyArea(fl_display, pixmap, fl_window, fl_gc, srcx, srcy, w, h, x, y)
#define fl_delete_offscreen(pixmap) XFreePixmap(fl_display, pixmap)

// this object contains all X-specific stuff about a window:
// Warning: this object is highly subject to change!  It's definition
// is only here so that fl_xid can be declared inline:
class Fl_X {
public:
  Window xid;
  Window other_xid;
  Fl_Window *w;
  Region region;
  Fl_X *next;
  static Fl_X* first;
  static Fl_X* i(const Fl_Window* w) {return w->i;}
  void setwindow(Fl_Window* wi) {w=wi; wi->i=this;}
  void sendxjunk();
  static void make_xid(Fl_Window*,XVisualInfo* =fl_visual, Colormap=fl_colormap);
  static Fl_X* set_xid(Fl_Window*, Window);
  // kludges to get around protection:
  void flush() {w->flush();}
  static void x(Fl_Window* w, int X) {w->x(X);}
  static void y(Fl_Window* w, int Y) {w->y(Y);}
};

// convert xid <-> Fl_Window:
inline Window fl_xid(const Fl_Window*w) {return Fl_X::i(w)->xid;}
Fl_Window* fl_find(Window xid);

extern char fl_override_redirect; // hack into Fl_X::make_xid()
extern int fl_background_pixel;  // hack into Fl_X::make_xid()

#endif
#endif
