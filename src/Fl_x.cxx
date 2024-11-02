//
// X specific code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2024 by Bill Spitzak and others.
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

// Note: this file contains platform specific code and will therefore
// not be processed by doxygen (see Doxyfile.in).

#  ifndef FLTK_CONSOLIDATE_MOTION
#  define FLTK_CONSOLIDATE_MOTION 0
#  endif

/**** Define this if your keyboard lacks a backspace key... ****/
/* #define BACKSPACE_HACK 1 */

#  include <config.h>
#  include <FL/Fl.H>
#  include <FL/platform.H>
#  include "Fl_Window_Driver.H"
#  include <FL/Fl_Window.H>
#  include <FL/fl_utf8.h>
#  include <FL/Fl_Tooltip.H>
#  include <FL/fl_draw.H>
#  include <FL/Fl_Paged_Device.H>
#  include <FL/Fl_Shared_Image.H>
#  include <FL/fl_ask.H>
#  include <FL/filename.H>
#  include <stdio.h>
#  include <stdlib.h>
#  include "flstring.h"
#  include "drivers/X11/Fl_X11_Screen_Driver.H"
#  include "drivers/X11/Fl_X11_Window_Driver.H"
#  include "drivers/Unix/Fl_Unix_System_Driver.H"
#if FLTK_USE_CAIRO
#  include "drivers/Cairo/Fl_X11_Cairo_Graphics_Driver.H"
#else
#  include "drivers/Xlib/Fl_Xlib_Graphics_Driver.H"
#endif
#  include "print_button.h"
#  include <unistd.h>
#  include <time.h>
#  include <sys/time.h>
#  include <math.h>
#  include <X11/Xmd.h>
#  include <X11/Xlocale.h>
#  include <X11/Xlib.h>
#  include <X11/keysym.h>
#  include "Xutf8.h"

#if FLTK_USE_CAIRO
#  include <cairo-xlib.h>
#  include <cairo/cairo.h>
#endif // FLTK_USE_CAIRO

#define USE_XRANDR (HAVE_DLSYM && HAVE_DLFCN_H) // means attempt to dynamically load libXrandr.so
#if USE_XRANDR
#include <dlfcn.h>
#define RRScreenChangeNotifyMask  (1L << 0) // from X11/extensions/Xrandr.h
#define RRScreenChangeNotify    0           // from X11/extensions/Xrandr.h
typedef int (*XRRUpdateConfiguration_type)(XEvent *event);
static XRRUpdateConfiguration_type XRRUpdateConfiguration_f;
static int randrEventBase;                  // base of RandR-defined events
#endif

#  if HAVE_XFIXES
#  include <X11/extensions/Xfixes.h>
static int xfixes_event_base = 0;
static bool have_xfixes = false;
#  endif

#  include <X11/cursorfont.h>

#  if HAVE_XCURSOR
#    include <X11/Xcursor/Xcursor.h>
#  endif
#  if HAVE_XRENDER
#    include <X11/extensions/Xrender.h>
#  endif

#  if USE_POLL
#    include <poll.h>
#  else
#    define POLLIN 1
#  endif /* USE_POLL */

extern Fl_Widget *fl_selection_requestor;

static void open_display_i(Display *d); // open display (internal)


extern int fl_send_system_handlers(void *e);

#if FLTK_CONSOLIDATE_MOTION
static Fl_Window* send_motion;
extern Fl_Window* fl_xmousewin;
#endif

static bool in_a_window; // true if in any of our windows, even destroyed ones
static void do_queued_events() {
  in_a_window = true;
  while (XEventsQueued(fl_display,QueuedAfterReading)) {
    XEvent xevent;
    XNextEvent(fl_display, &xevent);
    if (fl_send_system_handlers(&xevent))
      continue;
    fl_handle(xevent);
  }
  // we send FL_LEAVE only if the mouse did not enter some other window:
  if (!in_a_window) Fl::handle(FL_LEAVE, 0);
#if FLTK_CONSOLIDATE_MOTION
  else if (send_motion && send_motion == fl_xmousewin) {
    send_motion = 0;
    Fl::handle(FL_MOVE, fl_xmousewin);
  }
#endif
}


// This is never called with time_to_wait < 0.0:
// It should return negative on error, 0 if nothing happens before
// timeout, and >0 if any callbacks were done.
int Fl_X11_Screen_Driver::poll_or_select_with_delay(double time_to_wait) {
  // OpenGL and other broken libraries call XEventsQueued
  // unnecessarily and thus cause the file descriptor to not be ready,
  // so we must check for already-read events:
  if (fl_display && XQLength(fl_display)) {do_queued_events(); return 1;}
  return Fl_Unix_Screen_Driver::poll_or_select_with_delay(time_to_wait);
}

// just like Fl_X11_Screen_Driver::poll_or_select_with_delay(0.0) except no callbacks are done:
int Fl_X11_Screen_Driver::poll_or_select() {
  if (XQLength(fl_display)) return 1;
  return Fl_Unix_Screen_Driver::poll_or_select();
}

// replace \r\n by \n
static void convert_crlf(unsigned char *string, long& len) {
  unsigned char *a, *b;
  a = b = string;
  while (*a) {
    if (*a == '\r' && a[1] == '\n') { a++; len--; }
    else *b++ = *a++;
  }
  *b = 0;
}

////////////////////////////////////////////////////////////////

Display *fl_display;
Display *fl_x11_display() { return fl_display; }
Window fl_message_window = 0;
int fl_screen;
XVisualInfo *fl_visual;
Colormap fl_colormap;
static XRectangle status_area;

static Atom WM_DELETE_WINDOW;
static Atom WM_PROTOCOLS;
static Atom fl_MOTIF_WM_HINTS;
static Atom TARGETS;
static Atom CLIPBOARD;
static Atom TIMESTAMP;
static Atom PRIMARY_TIMESTAMP;
static Atom CLIPBOARD_TIMESTAMP;
Atom fl_XdndAware;
Atom fl_XdndSelection;
Atom fl_XdndEnter;
Atom fl_XdndTypeList;
Atom fl_XdndPosition;
Atom fl_XdndLeave;
Atom fl_XdndDrop;
Atom fl_XdndStatus;
Atom fl_XdndActionCopy;
Atom fl_XdndFinished;
Atom fl_XdndURIList;
static Atom fl_Xatextplainutf;
static Atom fl_Xatextplainutf2;         // STR#2930
static Atom fl_Xatextplain;
static Atom fl_XaText;
static Atom fl_XaCompoundText;
Atom fl_XaUtf8String;
static Atom fl_XaTextUriList;
static Atom fl_XaImageBmp;
static Atom fl_XaImagePNG;
static Atom fl_INCR;
static Atom fl_NET_WM_PID;
static Atom fl_NET_WM_NAME;                     // utf8 aware window label
static Atom fl_NET_WM_ICON_NAME;                // utf8 aware window icon name
static Atom fl_NET_SUPPORTING_WM_CHECK;
static Atom fl_NET_WM_STATE;
static Atom fl_NET_WM_STATE_FULLSCREEN;
static Atom fl_NET_WM_STATE_MAXIMIZED_VERT;
static Atom fl_NET_WM_STATE_MAXIMIZED_HORZ;
static Atom fl_NET_WM_FULLSCREEN_MONITORS;
Atom fl_NET_WORKAREA;
static Atom fl_NET_WM_ICON;
static Atom fl_NET_ACTIVE_WINDOW;

/*
  Debug: translate Atom (number) to name: enable (1) if used below
*/
#if (0)
static void debug_atom_name(const char *fun, int line, Atom atom) {
  char *name = XGetAtomName(fl_display, atom);
  fprintf(stderr, "[%s:%d] debug_atom_name (%4lu) = %s\n", fun, line, atom, name);
  XFree(name);
}
#endif

/*
  Find the best target in a list of available copy/paste or dnd targets.

  This function searches all available targets [avail, na] by comparing
  these with an ordered list of suitable targets [targets, nt].
  The size of the list of available targets is determined by 'na'.

  The list of suitable targets must be ordered by preference with the
  best target first. This ensures that we always find our preferred
  target, no matter how the list of available targets is ordered.
  The list size is max. 'nt' entries but can also be terminated
  by a zero (0) entry.

  The list of suitable targets is provided by FLTK (see below); the
  list of available targets is provided by the data source program.

  Returns: (Atom) the "best target" or 0 (zero) if no target matches.
*/

static Atom find_target(Atom *targets, int nt, Atom *avail, int na) {
  Atom retval = 0;
  Atom mt, at;
  int i = 0, m = 0;
#if (0) // Debug
  fprintf(stderr, "find_target: looking for:\n");
  for (i = 0; i < nt; i++)
    debug_atom_name("  find_target", i, targets[i]);
  for (i = 0; i < na; i++)
    debug_atom_name("  available  ", i, avail[i]);
#endif // Debug
  int n = nt;
  for (i = 0; i < na; i++) { // search all available targets
    at = avail[i];
    // search only *better* targets: m < n !
    for (m = 0; m < n; m++) { // try to match usable targets
      mt = targets[m];
      if (!mt) break; // end of list
      if (mt == at) {
        n = m;
        retval = mt;
        break;
      }
    }
    if (n == 0) break; // found the *best* target (0)
  }
  // debug_atom_name("find_target: FOUND", n, retval);
  return retval;
}

/*
  Find a suitable target for text insertion in a list of available targets.
*/
static Atom find_target_text(Atom *avail, int na) {
  static Atom text_targets[] = {
    fl_XaUtf8String,      // "UTF8_STRING"
    fl_Xatextplainutf2,   // "text/plain;charset=utf-8" (correct: lowercase)
    fl_Xatextplainutf,    // "text/plain;charset=UTF-8" (non-standard: compat.)
    fl_Xatextplain,       // "text/plain"
    XA_STRING,            // "STRING"
    fl_XaText,            // "TEXT"
    fl_XaCompoundText,    // "COMPOUND_TEXT"
    fl_XaTextUriList,     // "text/uri-list" (e.g. file manager)
    (Atom)0               // list terminator (optional, but don't remove)
  };
  static int nt = sizeof(text_targets) / sizeof(Atom) - 1; // list size w/o terminator
  return find_target(text_targets, nt, avail, na);
}

/*
  Find a suitable target for image insertion in a list of available targets.
*/
static Atom find_target_image(Atom *avail, int na) {
  static Atom image_targets[] = {
    fl_XaImageBmp,        // "image/bmp"
    fl_XaImagePNG,        // "image/png"
    (Atom)0               // list terminator (optional, but don't remove)
  };
  static int ni = sizeof(image_targets) / sizeof(Atom) - 1; // list size w/o terminator
  return find_target(image_targets, ni, avail, na);
}

/*
  X defines 32-bit-entities to have a format value of max. 32,
  although sizeof(atom) can be 8 (64 bits) on a 64-bit OS.
  See also fl_open_display() for sizeof(atom) < 4.
  Used for XChangeProperty (see STR #2419).
*/
static int atom_bits = 32;

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
    snprintf(buf1, 128, "XRequest.%d", e->request_code);
    XGetErrorDatabaseText(d,"",buf1,buf1,buf2,128);
    XGetErrorText(d, e->error_code, buf1, 128);
    Fl::warning("%s: %s 0x%lx", buf2, buf1, e->resourceid);
    return 0;
  }
}

extern char *fl_get_font_xfld(int fnum, int size);

void Fl_X11_Screen_Driver::new_ic()
{
  XVaNestedList preedit_attr = NULL;
  XVaNestedList status_attr = NULL;
  static XFontSet fs = NULL;
  char *fnt;
  char **missing_list = 0;
  int missing_count = 0;
  char *def_string;
  static XRectangle spot;
  int predit = 0;
  int sarea = 0;
  XIMStyles* xim_styles = NULL;

#if USE_XFT

#if defined(__GNUC__)
// FIXME: warning XFT support here
#endif /*__GNUC__*/

  if (!fs) {
    fnt = (char*)"-misc-fixed-*";
    fs = XCreateFontSet(fl_display, fnt, &missing_list,
                        &missing_count, &def_string);
  }
#else
  if (!fs) {
    bool must_free_fnt = true;
    fnt = fl_get_font_xfld(0, 14);
    if (!fnt) {fnt = (char*)"-misc-fixed-*";must_free_fnt=false;}
    fs = XCreateFontSet(fl_display, fnt, &missing_list,
                        &missing_count, &def_string);
    if (must_free_fnt) free(fnt);
  }
#endif

  if (missing_list) XFreeStringList(missing_list);

  preedit_attr = XVaCreateNestedList(0,
                                     XNSpotLocation, &spot,
                                     XNFontSet, fs, NULL);
  status_attr = XVaCreateNestedList(0,
                                    XNAreaNeeded, &status_area,
                                    XNFontSet, fs, NULL);

  if (!XGetIMValues(xim_im, XNQueryInputStyle,
                    &xim_styles, NULL, NULL)) {
    int i;
    XIMStyle *style;
    for (i = 0, style = xim_styles->supported_styles;
         i < xim_styles->count_styles; i++, style++) {
      if (*style == (XIMPreeditPosition | XIMStatusArea)) {
        sarea = 1;
        predit = 1;
      } else if (*style == (XIMPreeditPosition | XIMStatusNothing)) {
        predit = 1;
      }
    }
  }
  XFree(xim_styles);

  if (sarea) {
    xim_ic = XCreateIC(xim_im,
                          XNInputStyle, (XIMPreeditPosition | XIMStatusArea),
                          XNPreeditAttributes, preedit_attr,
                          XNStatusAttributes, status_attr,
                          NULL);
  }

  if (!xim_ic && predit) {
    xim_ic = XCreateIC(xim_im,
                          XNInputStyle, (XIMPreeditPosition | XIMStatusNothing),
                          XNPreeditAttributes, preedit_attr,
                          NULL);
  }
  XFree(preedit_attr);
  XFree(status_attr);
  if (!xim_ic) {
    Fl_X11_Screen_Driver::fl_is_over_the_spot = 0;
    xim_ic = XCreateIC(xim_im,
                          XNInputStyle, (XIMPreeditNothing | XIMStatusNothing),
                          NULL);
  } else {
    Fl_X11_Screen_Driver::fl_is_over_the_spot = 1;
    status_attr = XVaCreateNestedList(0, XNAreaNeeded, &status_area, NULL);

    XGetICValues(xim_ic, XNStatusAttributes, status_attr, NULL);
    XFree(status_attr);
  }
}


void Fl_X11_Screen_Driver::set_status(int x, int y, int w, int h)
{
  XVaNestedList status_attr;
  status_area.x = x;
  status_area.y = y;
  status_area.width = w;
  status_area.height = h;
  if (!xim_ic) return;
  status_attr = XVaCreateNestedList(0, XNArea, &status_area, NULL);

  XSetICValues(xim_ic, XNStatusAttributes, status_attr, NULL);
  XFree(status_attr);
}


void Fl_X11_Screen_Driver::init_xim() {
  static int xim_warning = 2;
  if (xim_warning > 0) xim_warning--;

  //XIMStyle *style;
  XIMStyles *xim_styles;
  if (!fl_display) return;
  if (xim_im) return;

  xim_im = XOpenIM(fl_display, NULL, NULL, NULL);
  xim_styles = NULL;
  xim_ic = NULL;

  if (xim_im) {
    XGetIMValues (xim_im, XNQueryInputStyle,
                  &xim_styles, NULL, NULL);
  } else {
    if (xim_warning)
      Fl::warning("XOpenIM() failed");
    // if xim_styles is allocated, free it now
    if (xim_styles) XFree(xim_styles);
    return;
  }

  if (xim_styles && xim_styles->count_styles) {
    Fl_X11_Screen_Driver::new_ic();
   } else {
     if (xim_warning)
       Fl::warning("No XIM style found");
     XCloseIM(xim_im);
     xim_im = NULL;
     // if xim_styles is allocated, free it now
     if (xim_styles) XFree(xim_styles);
     return;
  }
  if (!xim_ic) {
    if (xim_warning)
      Fl::warning("XCreateIC() failed");
    XCloseIM(xim_im);
    xim_im = NULL;
  }
  // if xim_styles is still allocated, free it now
  if(xim_styles) XFree(xim_styles);
}


void Fl_X11_Screen_Driver::xim_activate(Window xid) {
  if (!xim_im)
    return;

  // If the focused window has changed, then use the brute force method
  // of completely recreating the input context.
  if (xim_win != xid) {
    xim_deactivate();

    Fl_X11_Screen_Driver::new_ic();
    xim_win = xid;

    XSetICValues(xim_ic,
                 XNFocusWindow, xim_win,
                 XNClientWindow, xim_win,
                 NULL);
  }

  Fl_X11_Screen_Driver *driver = (Fl_X11_Screen_Driver*)Fl::screen_driver();
  driver->set_spot(fl_spotf, fl_spots, fl_spot.x, fl_spot.y, fl_spot.width, fl_spot.height, NULL);
}

void Fl_X11_Screen_Driver::xim_deactivate(void) {
  if (!xim_ic)
    return;

  XDestroyIC(xim_ic);
  xim_ic = NULL;

  xim_win = 0;
}

void Fl_X11_Screen_Driver::enable_im() {
  Fl_Window *win;

  win = Fl::first_window();
  if (win && win->shown()) {
    xim_activate(fl_xid(win));
    XSetICFocus(xim_ic);
  } else {
    new_ic();
  }
}

void Fl_X11_Screen_Driver::disable_im() {
  xim_deactivate();
}

static void delayed_create_print_window(void *) {
  Fl::remove_check(delayed_create_print_window);
  fl_create_print_window();
}

void Fl_X11_Screen_Driver::open_display_platform() {
  static Display *d = NULL;
  if (d) return;

  setlocale(LC_CTYPE, "");
  XSetLocaleModifiers("");

  XSetIOErrorHandler(io_error_handler);
  XSetErrorHandler(xerror_handler);

  d = (fl_display ? fl_display : XOpenDisplay(0));
  if (!d) {
    Fl::fatal("Can't open display: %s", XDisplayName(0)); // does not return
    return; // silence static code analyzer
  }

  open_display_i(d);
  // the unique GC used by all X windows
  GC gc = XCreateGC(fl_display, RootWindow(fl_display, fl_screen), 0, 0);
  Fl_Graphics_Driver::default_driver().gc(gc);
  Fl::add_check(delayed_create_print_window);
}


void open_display_i(Display* d) {
  fl_display = d;

  WM_DELETE_WINDOW      = XInternAtom(d, "WM_DELETE_WINDOW",    0);
  WM_PROTOCOLS          = XInternAtom(d, "WM_PROTOCOLS",        0);
  fl_MOTIF_WM_HINTS     = XInternAtom(d, "_MOTIF_WM_HINTS",     0);
  TARGETS               = XInternAtom(d, "TARGETS",             0);
  CLIPBOARD             = XInternAtom(d, "CLIPBOARD",           0);
  TIMESTAMP             = XInternAtom(d, "TIMESTAMP",           0);
  PRIMARY_TIMESTAMP     = XInternAtom(d, "PRIMARY_TIMESTAMP",   0);
  CLIPBOARD_TIMESTAMP   = XInternAtom(d, "CLIPBOARD_TIMESTAMP", 0);
  fl_XdndAware          = XInternAtom(d, "XdndAware",           0);
  fl_XdndSelection      = XInternAtom(d, "XdndSelection",       0);
  fl_XdndEnter          = XInternAtom(d, "XdndEnter",           0);
  fl_XdndTypeList       = XInternAtom(d, "XdndTypeList",        0);
  fl_XdndPosition       = XInternAtom(d, "XdndPosition",        0);
  fl_XdndLeave          = XInternAtom(d, "XdndLeave",           0);
  fl_XdndDrop           = XInternAtom(d, "XdndDrop",            0);
  fl_XdndStatus         = XInternAtom(d, "XdndStatus",          0);
  fl_XdndActionCopy     = XInternAtom(d, "XdndActionCopy",      0);
  fl_XdndFinished       = XInternAtom(d, "XdndFinished",        0);
  fl_XdndURIList        = XInternAtom(d, "text/uri-list",       0);
  fl_Xatextplainutf     = XInternAtom(d, "text/plain;charset=UTF-8",0);
  fl_Xatextplainutf2    = XInternAtom(d, "text/plain;charset=utf-8",0); // Firefox/Thunderbird needs this - See STR#2930
  fl_Xatextplain        = XInternAtom(d, "text/plain",          0);
  fl_XaText             = XInternAtom(d, "TEXT",                0);
  fl_XaCompoundText     = XInternAtom(d, "COMPOUND_TEXT",       0);
  fl_XaUtf8String       = XInternAtom(d, "UTF8_STRING",         0);
  fl_XaTextUriList      = XInternAtom(d, "text/uri-list",       0);
  fl_XaImageBmp         = XInternAtom(d, "image/bmp",           0);
  fl_XaImagePNG         = XInternAtom(d, "image/png",           0);
  fl_INCR               = XInternAtom(d, "INCR",                0);
  fl_NET_WM_PID         = XInternAtom(d, "_NET_WM_PID",         0);
  fl_NET_WM_NAME        = XInternAtom(d, "_NET_WM_NAME",        0);
  fl_NET_WM_ICON_NAME   = XInternAtom(d, "_NET_WM_ICON_NAME",   0);
  fl_NET_SUPPORTING_WM_CHECK = XInternAtom(d, "_NET_SUPPORTING_WM_CHECK", 0);
  fl_NET_WM_STATE       = XInternAtom(d, "_NET_WM_STATE",       0);
  fl_NET_WM_STATE_FULLSCREEN = XInternAtom(d, "_NET_WM_STATE_FULLSCREEN", 0);
  fl_NET_WM_STATE_MAXIMIZED_VERT = XInternAtom(d, "_NET_WM_STATE_MAXIMIZED_VERT", 0);
  fl_NET_WM_STATE_MAXIMIZED_HORZ = XInternAtom(d, "_NET_WM_STATE_MAXIMIZED_HORZ", 0);
  fl_NET_WM_FULLSCREEN_MONITORS = XInternAtom(d, "_NET_WM_FULLSCREEN_MONITORS", 0);
  fl_NET_WORKAREA       = XInternAtom(d, "_NET_WORKAREA",       0);
  fl_NET_WM_ICON        = XInternAtom(d, "_NET_WM_ICON",        0);
  fl_NET_ACTIVE_WINDOW  = XInternAtom(d, "_NET_ACTIVE_WINDOW",  0);

  if (sizeof(Atom) < 4)
    atom_bits = sizeof(Atom) * 8;

  Fl::add_fd(ConnectionNumber(d), POLLIN, fd_callback);

  fl_screen = DefaultScreen(d);

  fl_message_window =
    XCreateSimpleWindow(d, RootWindow(d,fl_screen), 0,0,1,1,0, 0, 0);

// construct an XVisualInfo that matches the default Visual:
  XVisualInfo templt; int num;
  templt.visualid = XVisualIDFromVisual(DefaultVisual(d, fl_screen));
  fl_visual = XGetVisualInfo(d, VisualIDMask, &templt, &num);
  fl_colormap = DefaultColormap(d, fl_screen);
  Fl_X11_Screen_Driver::init_xim();

#if !USE_COLORMAP
  Fl::visual(FL_RGB);
#endif

#if HAVE_XFIXES
  int error_base;
  if (XFixesQueryExtension(fl_display, &xfixes_event_base, &error_base))
    have_xfixes = true;
  else
    have_xfixes = false;
#endif

#if USE_XRANDR
  void *libxrandr_addr = Fl_Posix_System_Driver::dlopen_or_dlsym("libXrandr");
  if (libxrandr_addr) {
    int error_base;
    typedef Bool (*XRRQueryExtension_type)(Display*, int*, int*);
    typedef void (*XRRSelectInput_type)(Display*, Window, int);
    XRRQueryExtension_type XRRQueryExtension_f = (XRRQueryExtension_type)dlsym(libxrandr_addr, "XRRQueryExtension");
    XRRSelectInput_type XRRSelectInput_f = (XRRSelectInput_type)dlsym(libxrandr_addr, "XRRSelectInput");
    XRRUpdateConfiguration_f = (XRRUpdateConfiguration_type)dlsym(libxrandr_addr, "XRRUpdateConfiguration");
    if (XRRQueryExtension_f && XRRSelectInput_f && XRRQueryExtension_f(d, &randrEventBase, &error_base))
      XRRSelectInput_f(d, RootWindow(d, fl_screen), RRScreenChangeNotifyMask);
    else XRRUpdateConfiguration_f = NULL;
    }
#endif

  // Listen for changes to _NET_WORKAREA
  XSelectInput(d, RootWindow(d, fl_screen), PropertyChangeMask);
}

void Fl_X11_Screen_Driver::close_display() {
  Fl::remove_fd(ConnectionNumber(fl_display));
  XCloseDisplay(fl_display);
}

int Fl_X11_Screen_Driver::get_mouse_unscaled(int &mx, int &my) {
  open_display();
  Window root = RootWindow(fl_display, fl_screen);
  Window c; int cx,cy; unsigned int mask;
  XQueryPointer(fl_display, root, &root, &c, &mx, &my, &cx, &cy, &mask);
#if USE_XFT
  int screen = screen_num_unscaled(mx, my);
  return screen >= 0 ? screen : 0;
#else
  return screen_num(mx, my);
#endif
}


int Fl_X11_Screen_Driver::get_mouse(int &xx, int &yy) {
  int snum = get_mouse_unscaled(xx, yy);
  float s = scale(snum);
  xx = xx/s;
  yy = yy/s;
  return snum;
}

////////////////////////////////////////////////////////////////
// Code used for paste and DnD into the program:

char *fl_selection_buffer[2];
int fl_selection_length[2];
const char * fl_selection_type[2];
int fl_selection_buffer_length[2];
char fl_i_own_selection[2] = {0,0};


// Call this when a "paste" operation happens:
void Fl_X11_Screen_Driver::paste(Fl_Widget &receiver, int clipboard, const char *type) {
  if (fl_i_own_selection[clipboard]) {
    // We already have it, do it quickly without window server.
    if (type == Fl::clipboard_plain_text && fl_selection_type[clipboard] == type) {
      // Notice that the text is clobbered if set_selection is
      // called in response to FL_PASTE!
      // However, for now, we only paste text in this function
      Fl::e_text = fl_selection_buffer[clipboard];
      Fl::e_length = fl_selection_length[clipboard];
      if (!Fl::e_text) Fl::e_text = (char *)"";
    } else if (clipboard == 1 && type == Fl::clipboard_image && fl_selection_type[1] == type) {
      Fl::e_clipboard_data = Fl_Unix_System_Driver::own_bmp_to_RGB(fl_selection_buffer[1]);
      Fl::e_clipboard_type = Fl::clipboard_image;
    } else return;
    int retval = receiver.handle(FL_PASTE);
    if (retval == 0 && type == Fl::clipboard_image) {
      delete (Fl_RGB_Image*)Fl::e_clipboard_data;
      Fl::e_clipboard_data = NULL;
    }
    return;
  }
  // otherwise get the window server to return it:
  fl_selection_requestor = &receiver;
  Atom property = clipboard ? CLIPBOARD : XA_PRIMARY;
  Fl::e_clipboard_type = type;
  XConvertSelection(fl_display, property, TARGETS, property,
                    fl_xid(Fl::first_window()), fl_event_time);
}

int Fl_X11_Screen_Driver::clipboard_contains(const char *type) {
  if (fl_i_own_selection[1]) {
    return fl_selection_type[1] == type;
  }
  XEvent event;
  Atom actual; int format; unsigned long count, remaining, i = 0;
  unsigned char* portion = NULL;
  Fl_Window *win = Fl::first_window();
  if (!win || !fl_xid(win)) return 0;
  win->wait_for_expose();
  XConvertSelection(fl_display, CLIPBOARD, TARGETS, CLIPBOARD, fl_xid(win), CurrentTime);
  XFlush(fl_display);
  // FIXME: The following loop may ignore up to 20 events! (AlbrechtS)
  do {
    XNextEvent(fl_display, &event);
    if (event.type == SelectionNotify && event.xselection.property == None) return 0;
    i++;
  } while (i < 20 && event.type != SelectionNotify);
  if (i >= 20) return 0;
  XGetWindowProperty(fl_display,
                     event.xselection.requestor,
                     event.xselection.property,
                     0, 4000, 0, 0,
                     &actual, &format, &count, &remaining, &portion);
  if (actual != XA_ATOM) return 0;
  Atom t = (Atom)0;
  if (strcmp(type, Fl::clipboard_plain_text) == 0)
    t = find_target_text((Atom*)portion, count);
  else if (strcmp(type, Fl::clipboard_image) == 0)
    t = find_target_image((Atom*)portion, count);
  XFree(portion);
  return (t ? 1 : 0);
}

static Window fl_dnd_source_window;
static Atom *fl_dnd_source_types; // null-terminated list of data types being supplied
static Atom fl_dnd_type;
static Atom fl_dnd_source_action;
static Atom fl_dnd_action;

void fl_sendClientMessage(Window window, Atom message,
                                 unsigned long d0,
                                 unsigned long d1=0,
                                 unsigned long d2=0,
                                 unsigned long d3=0,
                                 unsigned long d4=0)
{
  XEvent e;
  e.xany.type = ClientMessage;
  e.xany.window = window;
  e.xclient.message_type = message;
  e.xclient.format = 32;
  e.xclient.data.l[0] = (long)d0;
  e.xclient.data.l[1] = (long)d1;
  e.xclient.data.l[2] = (long)d2;
  e.xclient.data.l[3] = (long)d3;
  e.xclient.data.l[4] = (long)d4;
  XSendEvent(fl_display, window, 0, 0, &e);
}


/*
   Get window property value (32 bit format)
   Returns zero on success, -1 on error

   'data' should be freed with XFree() using this pattern:

        unsigned long *data = 0;
        if (0 == get_xwinprop(....., &nitems, &data) ) { ..success.. }
        else { ..fail.. }
        if ( data ) { XFree(data); data=0; }

    Note: 'data' can be non-zero, even if the return value is -1 (error) and
    should hence be XFree'd *after* the if/else statement, as described above.
*/
static int get_xwinprop(Window wnd, Atom prop, long max_length,
                        unsigned long *nitems, unsigned long **data) {
  Atom actual;
  int format;
  unsigned long bytes_after;

  if (Success != XGetWindowProperty(fl_display, wnd, prop, 0, max_length,
                                    False, AnyPropertyType, &actual, &format,
                                    nitems, &bytes_after, (unsigned char**)data)) {
    return -1;
  }

  if (actual == None || format != 32) {
    return -1;
  }

  return 0;
}


////////////////////////////////////////////////////////////////
// Code for copying to clipboard and DnD out of the program:

// See Fl::copy() for possible values of the destination (argument clipboard)
// See also Fl::selection_to_clipboard()
void Fl_X11_Screen_Driver::copy(const char *stuff, int len, int clipboard, const char *type) {
  if (!stuff || len<0) return;

  // if selection_to_clipboard is enabled *and* destination is 0 (selection buffer),
  // then copy to both (STR 3229)
  if (clipboard == 0 && Fl::selection_to_clipboard())
    clipboard = 2;

  if (clipboard >= 2) {
    copy(stuff, len, 1, type);  // copy to clipboard first (this is a recursion!)
    clipboard = 0;              // ... and then to selection buffer: fall through
  }

  if (len+1 > fl_selection_buffer_length[clipboard]) {
    delete[] fl_selection_buffer[clipboard];
    fl_selection_buffer[clipboard] = new char[len+100];
    fl_selection_buffer_length[clipboard] = len+100;
  }
  memcpy(fl_selection_buffer[clipboard], stuff, len);
  fl_selection_buffer[clipboard][len] = 0; // needed for direct paste
  fl_selection_length[clipboard] = len;
  fl_i_own_selection[clipboard] = 1;
  fl_selection_type[clipboard] = Fl::clipboard_plain_text;
  Atom property = clipboard ? CLIPBOARD : XA_PRIMARY;
  XSetSelectionOwner(fl_display, property, fl_message_window, fl_event_time);
}


// takes a raw RGB image and puts it in the copy/paste buffer
void Fl_X11_Screen_Driver::copy_image(const unsigned char *data, int W, int H, int clipboard){
  if (!data || W <= 0 || H <= 0) return;
  delete[] fl_selection_buffer[clipboard];
  fl_selection_buffer[clipboard] = (char *) Fl_Unix_System_Driver::create_bmp(data,W,H,&fl_selection_length[clipboard]);
  fl_selection_buffer_length[clipboard] = fl_selection_length[clipboard];
  fl_i_own_selection[clipboard] = 1;
  fl_selection_type[clipboard] = Fl::clipboard_image;
  Atom property = clipboard ? CLIPBOARD : XA_PRIMARY;
  XSetSelectionOwner(fl_display, property, fl_message_window, fl_event_time);
}

////////////////////////////////////////////////////////////////
// Code for tracking clipboard changes:

static Time primary_timestamp = (Time)-1;
static Time clipboard_timestamp = (Time)-1;

extern bool fl_clipboard_notify_empty(void);
extern void fl_trigger_clipboard_notify(int source);

static void poll_clipboard_owner(void) {
  Window xid;

#if HAVE_XFIXES
  // No polling needed with Xfixes
  if (have_xfixes)
    return;
#endif

  // No one is interested, so no point polling
  if (fl_clipboard_notify_empty())
    return;

  // We need a window for this to work
  if (!Fl::first_window())
    return;
  xid = fl_xid(Fl::first_window());
  if (!xid)
    return;

  // Request an update of the selection time for both the primary and
  // clipboard selections. Magic continues when we get a SelectionNotify.
  if (!fl_i_own_selection[0])
    XConvertSelection(fl_display, XA_PRIMARY, TIMESTAMP, PRIMARY_TIMESTAMP,
                      xid, fl_event_time);
  if (!fl_i_own_selection[1])
    XConvertSelection(fl_display, CLIPBOARD, TIMESTAMP, CLIPBOARD_TIMESTAMP,
                      xid, fl_event_time);
}

static void clipboard_timeout(void *data)
{
  // No one is interested, so stop polling
  if (fl_clipboard_notify_empty())
    return;

  poll_clipboard_owner();

  Fl::repeat_timeout(0.5, clipboard_timeout);
}

static void handle_clipboard_timestamp(int clipboard, Time time)
{
  Time *timestamp;

  timestamp = clipboard ? &clipboard_timestamp : &primary_timestamp;

#if HAVE_XFIXES
  if (!have_xfixes)
#endif
  {
    // Initial scan, just store the value
    if (*timestamp == (Time)-1) {
      *timestamp = time;
      return;
    }
  }

  // Same selection
  if (time == *timestamp)
    return;

  *timestamp = time;

  // The clipboard change is the event that caused us to request
  // the clipboard data, so use that time as the latest event.
  if (time > fl_event_time)
    fl_event_time = time;

  // Something happened! Let's tell someone!
  fl_trigger_clipboard_notify(clipboard);
}

void Fl_X11_Screen_Driver::clipboard_notify_change() {
  // Reset the timestamps if we've going idle so that you don't
  // get a bogus immediate trigger next time they're activated.
  if (fl_clipboard_notify_empty()) {
    primary_timestamp = (Time)-1;
    clipboard_timestamp = (Time)-1;
  } else {
#if HAVE_XFIXES
    if (!have_xfixes)
#endif
    {
      poll_clipboard_owner();

      if (!Fl::has_timeout(clipboard_timeout))
        Fl::add_timeout(0.5, clipboard_timeout);
    }
  }
}

////////////////////////////////////////////////////////////////

const XEvent* fl_xevent; // the current x event
ulong fl_event_time; // the last timestamp from an x event

char fl_key_vector[32]; // used by Fl::get_key()

// Record event mouse position and state from an XEvent:

static int px, py;
static ulong ptime;

// Citation from XButtonEvent and XKeyEvent docs:
//  "The state member is set to indicate the logical state of the pointer buttons
//   and modifier keys just prior to the event, which is the bitwise inclusive OR
//   of one or more of the button or modifier key masks:
//   Button1Mask, Button2Mask, Button3Mask, Button4Mask, Button5Mask,
//   ShiftMask, LockMask, ControlMask,
//   Mod1Mask, Mod2Mask, Mod3Mask, Mod4Mask, and Mod5Mask."
//
// Actual values in Debian Bookworm as of July 2024 (pseudo code):
//   static int states[] = {
//     ShiftMask, LockMask, ControlMask,                                 // 1<<0 .. 1<<2
//     Mod1Mask, Mod2Mask, Mod3Mask, Mod4Mask, Mod5Mask,                 // 1<<3 .. 1<<7
//     Button1Mask, Button2Mask, Button3Mask, Button4Mask, Button5Mask   // 1<<8 .. 1<<12
//   };
//
// Note: some more (undefined?) state bits *may* be set if the user uses a keyboard
// other than the primary one (the top-most in keyboard settings). Therefore we must
// take care not to use these undefined bits (found by accident).
// These undefined bits are ignored and not set in Fl::event_state(), otherwise we
// might overwrite other valid bits (since FLTK 1.4.0, Sep 2024 or later).
// See definition of FL_BUTTONS in FL/Enumerations.H:
// there are only five "sticky" mouse buttons as of Sep 27, 2024.

static unsigned int xbutton_state = 0; // extended button state (back, forward)

// Define the state bits we're interested in for Fl::event_state().
// Note that we ignore Button4Mask and Button5Mask (vertical scroll wheel).
// X11 doesn't define masks for Button6 and Button7 (horizontal scroll wheel)
// and any higher button numbers.

static const unsigned int event_state_mask =
  ShiftMask | LockMask | ControlMask |
  Mod1Mask | Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask |
  Button1Mask | Button2Mask | Button3Mask;

static void set_event_xy(Fl_Window *win) {
#  if FLTK_CONSOLIDATE_MOTION
  send_motion = 0;
#  endif
  float s = 1;
#if USE_XFT
  s = Fl::screen_driver()->scale(Fl_Window_Driver::driver(win)->screen_num());
#endif
  Fl::e_x_root  = fl_xevent->xbutton.x_root/s;
  Fl::e_x       = fl_xevent->xbutton.x/s;
  Fl::e_y_root  = fl_xevent->xbutton.y_root/s;
  Fl::e_y       = fl_xevent->xbutton.y/s;
  Fl::e_state   = ((fl_xevent->xbutton.state & event_state_mask) << 16) | xbutton_state;
  fl_event_time = fl_xevent->xbutton.time;
#  ifdef __sgi
  // get the meta key off PC keyboards:
  if (fl_key_vector[18]&0x18) Fl::e_state |= FL_META;
#  endif
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

static char unknown[] = "<unknown>";
const int unknown_len = 10;

extern "C" {

static int xerror = 0;

static int ignoreXEvents(Display *display, XErrorEvent *event) {
  xerror = 1;
  return 0;
}

static XErrorHandler catchXExceptions() {
  xerror = 0;
  return ignoreXEvents;
}

static int wasXExceptionRaised() {
  return xerror;
}

} // extern "C"

static bool getNextEvent(XEvent *event_return) {
  time_t t = time(NULL);
  while (!XPending(fl_display)) {
    if (time(NULL) - t > 10.0) {
      // fprintf(stderr,"Error: The XNextEvent never came...\n");
      return false;
    }
  }
  XNextEvent(fl_display, event_return);
  return true;
}

static long getIncrData(uchar* &data, const XSelectionEvent& selevent, size_t lower_bound) {
  // fprintf(stderr,"Incremental transfer starting due to INCR property\n");
  // fprintf(stderr, "[getIncrData:%d] lower_bound [in ] =%10ld\n", __LINE__, lower_bound);
  const size_t alloc_min =   4 * 1024 * 1024; // min. initial allocation
  const size_t alloc_max = 200 * 1024 * 1024; // max. initial allocation
  const size_t alloc_inc =   4 * 1024 * 1024; // (min.) increase if necessary
  size_t total = 0;
  size_t data_size = lower_bound + 1;
  if (data_size < alloc_min) {
    data_size = alloc_min;
  } else if (data_size > alloc_max) {
    data_size = alloc_max;
  }
  // fprintf(stderr, "[getIncrData:%d] initial alloc.    =%10ld\n", __LINE__, data_size);

  XEvent event;
  XDeleteProperty(fl_display, selevent.requestor, selevent.property);
  data = (uchar*)realloc(data, data_size);
  if (!data) {
    // fprintf(stderr, "[getIncrData:%d] realloc() FAILED, size = %ld\n", __LINE__, data_size);
    Fl::fatal("Clipboard data transfer failed, size %ld is too large.", data_size);
  }
  for (;;) {
    if (!getNextEvent(&event)) {
      // This is unexpected but may happen if the sender (clipboard owner) no longer sends data
      // fprintf(stderr, "[getIncrData:%d] Failed to get next event (timeout) *** break! ***\n", __LINE__);
      break;
    }
    if (event.type == PropertyNotify) {
      if (event.xproperty.state != PropertyNewValue) continue; // ignore PropertyDelete
      Atom actual_type;
      int actual_format;
      unsigned long nitems;
      unsigned long bytes_after;
      unsigned char* prop = 0;
      long offset = 0;
      size_t num_bytes = 0;
      // size_t slice_size = 0;
      do {
        XGetWindowProperty(fl_display, selevent.requestor, selevent.property, offset, 70000, True,
                           AnyPropertyType, &actual_type, &actual_format, &nitems, &bytes_after, &prop);
        num_bytes = nitems * (actual_format / 8);
        offset += num_bytes/4;
        // slice_size += num_bytes;
        if (total + num_bytes + bytes_after + 1 > data_size) {
          data_size += alloc_inc;
          if (total + num_bytes + bytes_after + 1 > data_size)
            data_size = total + num_bytes + bytes_after + 1;
          // printf(" -- realloc(%9ld), total=%10ld, num_bytes=%7ld, bytes_after=%7ld (%7ld), required=%10ld\n",
          //        data_size, total, num_bytes, bytes_after, num_bytes + bytes_after, total + num_bytes + bytes_after + 1);
          data = (uchar*)realloc(data, data_size);
          if (!data) {
            // fprintf(stderr, "[getIncrData():%d] realloc() FAILED, size = %ld\n", __LINE__, data_size);
            Fl::fatal("Clipboard data transfer failed, size %ld is too large.", data_size);
          }
        }
        memcpy(data + total, prop, num_bytes);
        total += num_bytes;
        if (prop) XFree(prop);
      } while (bytes_after != 0);
      // fprintf(stderr,"INCR data size:%ld\n", slice_size);
      if (num_bytes == 0) break;
    }
    else {
      // Unexpected next event. At this point we're handling the INCR protocol and can't deal with
      // *some* other events due to potential recursions. We *could* call fl_handle(event) to handle
      // *selected* other events but for the time being we ignore all other events!
      // Handling the INCR protocol for very large data may take some time and multiple events.
      // Interleaving "other" events are possible, for instance the KeyRelease event of the
      // ctrl/v key pressed to insert the clipboard. This solution is not perfect but it can
      // handle the INCR protocol with very large selections in most cases, although with potential
      // side effects because other events may be ignored.
      // See GitHub Issue #451: "Segfault if using very large selections".
      // Note: the "fix" for Issue #451 is basically to use 'continue' rather than 'break'
      // Debug:
      // fprintf(stderr,
      //   "[getIncrData:%d] getNextEvent() returned %d, not PropertyNotify (%d). Event ignored.\n",
      //   __LINE__, event.type, PropertyNotify);

      continue;
    }
  }
  XDeleteProperty(fl_display, selevent.requestor, selevent.property);
  // fprintf(stderr, "[getIncrData:%d] total data  [out] =%10ld\n", __LINE__, (long)total);
  return (long)total;
}

/*
  Internal function to reduce "deprecated" warnings for XKeycodeToKeysym().
  This way we get at most one warning. The option to use XkbKeycodeToKeysym()
  instead would not help much - see STR #2913 for more information.

  Update (Jan 31, 2020): disable "deprecated declaration" warnings in
  this function for GCC >= 4.6 and clang (all versions) to get rid of
  these warnings at least for current GCC and clang compilers.

  Note: '#pragma GCC diagnostic push' needs at least GCC 4.6.
*/

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 5))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

static KeySym fl_KeycodeToKeysym(Display *d, KeyCode k, unsigned i) {
  return XKeycodeToKeysym(d, k, i);
}

#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 5))
#pragma GCC diagnostic pop
#endif

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#if USE_XRANDR
static void react_to_screen_reconfiguration() {
#if USE_XFT
  // memorize previous screen sizes and scales
  int old_count = Fl::screen_count();
  int (*sizes)[4] =  new int[old_count][4];
  float *scales = new float[old_count];
  for (int screen = 0; screen < old_count; screen++) {
    Fl::screen_xywh(sizes[screen][0], sizes[screen][1], sizes[screen][2], sizes[screen][3], screen);
    scales[screen] = Fl::screen_scale(screen);
  }
#endif // USE_XFT
  Fl::call_screen_init(); // compute new screen sizes
#if USE_XFT
  // detect whether screen sizes were unchanged
  bool nochange = (old_count == Fl::screen_count());
  if (nochange) {
    for (int screen = 0; screen < old_count; screen++) {
      int X,Y,W,H;
      Fl::screen_xywh(X,Y,W,H, screen);
      X /= scales[screen];
      Y /= scales[screen];
      W /= scales[screen];
      H /= scales[screen];
      if (X != sizes[screen][0] || Y != sizes[screen][1] || W != sizes[screen][2] || H != sizes[screen][3]) {
        nochange = false;
        break;
      }
    }
  }
  delete[] sizes;
  if (nochange || (old_count == 1 && Fl::screen_count() == 1)) {
    // screen sizes did not change or single screen: re-use previous screen scale values
    for (int screen = 0; screen < old_count; screen++)
      Fl::screen_driver()->scale(screen, scales[screen]);
  } else {
    Fl::screen_driver()->use_startup_scale_factor();
    float new_scale = Fl::screen_driver()->scale(0);
    for (int screen = 0; screen < Fl::screen_count(); screen++) {
      Fl::screen_driver()->scale(screen, 1);
      Fl::screen_driver()->rescale_all_windows_from_screen(screen, new_scale, 1);
    }
  }
  delete[] scales;
#endif // USE_XFT
}
#endif // USE_XRANDR

#if USE_XFT
static void after_display_rescale(float *p_current_xft_dpi) {
  Display *new_dpy = XOpenDisplay(XDisplayString(fl_display));
  if (!new_dpy) return;
  char *s = XGetDefault(new_dpy, "Xft", "dpi");
  float dpi;
  if (s && sscanf(s, "%f", &dpi) == 1) {
    //printf("%s previous=%g dpi=%g \n", s, *p_current_xft_dpi, dpi);
    if (fabs(dpi - *p_current_xft_dpi) > 0.1) {
      *p_current_xft_dpi = dpi;
      float f = dpi / 96.;
      for (int i = 0; i < Fl::screen_count(); i++)
        Fl::screen_driver()->rescale_all_windows_from_screen(i, f, f);
    }
  }
  XCloseDisplay(new_dpy);
}
#endif // USE_XFT


static Window *xid_vector = NULL; // list of FLTK-created xid's (see issue #935)
static int xid_vector_size = 0;
static int xid_vector_count = 0;

static void add_xid_vector(Window xid) {
  if (xid_vector_count >= xid_vector_size) {
    xid_vector_size += 10;
    xid_vector = (Window*)realloc(xid_vector, xid_vector_size * sizeof(Window));
  }
  xid_vector[xid_vector_count++] = xid;
}

static bool remove_xid_vector(Window xid) {
  for (int pos = xid_vector_count - 1; pos >= 0; pos--) {
    if (xid_vector[pos] == xid) {
      if (pos != --xid_vector_count) xid_vector[pos] = xid_vector[xid_vector_count];
      return true;
    }
  }
  return false;
}

int fl_handle(const XEvent& thisevent)
{
  XEvent xevent = thisevent;
  fl_xevent = &thisevent;
  Window xid = xevent.xany.window;

  // For each DestroyNotify event, determine whether an FLTK-created window
  // is being destroyed (see issue #935).
  bool xid_is_from_fltk_win = false;
  if (xevent.type == DestroyNotify) {
    xid_is_from_fltk_win = remove_xid_vector(xid);
  }

  // The following if statement is limited to cases when event DestroyNotify
  // concerns a non-FLTK window. Thus, the possibly slow call to XOpenIM()
  // is not performed when an FLTK-created window is closed. This fixes issue #935.
  if (Fl_X11_Screen_Driver::xim_ic && xevent.type == DestroyNotify &&
        xid != Fl_X11_Screen_Driver::xim_win && !fl_find(xid) && !xid_is_from_fltk_win)
  {
// When using menus or tooltips: xid is a just hidden top-level FLTK win, xim_win is non-FLTK;
// after XIM crash: xid is non-FLTK.
// Trigger XIM crash under Debian: kill process containing "ibus-daemon"
// Restart XIM after triggered crash: "ibus-daemon --panel disable --xim &"
    XIM xim_im;
    xim_im = XOpenIM(fl_display, NULL, NULL, NULL);
    if (!xim_im) {
      /*  XIM server has crashed */
      XSetLocaleModifiers("");
      Fl_X11_Screen_Driver::xim_im = NULL;
      Fl_X11_Screen_Driver::init_xim();
    } else {
      XCloseIM(xim_im); // see STR 2185 for comment
    }
    return 0;
  }

  if (Fl_X11_Screen_Driver::xim_ic && (xevent.type == FocusIn))
    Fl_X11_Screen_Driver::xim_activate(xid);

  if (Fl_X11_Screen_Driver::xim_ic && XFilterEvent((XEvent *)&xevent, 0))
      return(1);

#if USE_XRANDR
  if( XRRUpdateConfiguration_f && xevent.type == randrEventBase + RRScreenChangeNotify) {
    XRRUpdateConfiguration_f(&xevent);
    react_to_screen_reconfiguration();
    Fl::handle(FL_SCREEN_CONFIGURATION_CHANGED, NULL);
  }
#endif // USE_XRANDR

  if (xevent.type == PropertyNotify && xevent.xproperty.atom == fl_NET_WORKAREA) {
    Fl_X11_Screen_Driver *d = (Fl_X11_Screen_Driver*)Fl::screen_driver();
    d->init_workarea();
#if USE_XFT
    after_display_rescale(&(d->current_xft_dpi));
#endif // USE_XFT
  }

  switch (xevent.type) {

  case KeymapNotify:
    memcpy(fl_key_vector, xevent.xkeymap.key_vector, 32);
    return 0;

  case MappingNotify:
    XRefreshKeyboardMapping((XMappingEvent*)&xevent.xmapping);
    return 0;

  case SelectionNotify: {
    static unsigned char* sn_buffer = 0;
    if (sn_buffer) {
      free(sn_buffer); sn_buffer = 0;
    }
    long bytesread = 0;
    if (fl_xevent->xselection.property) for (;;) {
      // The Xdnd code pastes 64K chunks together, possibly to avoid
      // bugs in X servers, or maybe to avoid an extra round-trip to
      // get the property length.  I copy this here:
      Atom actual; int format; unsigned long count, remaining;
      unsigned char* portion = NULL;
      if (XGetWindowProperty(fl_display,
                             fl_xevent->xselection.requestor,
                             fl_xevent->xselection.property,
                             bytesread/4, 65536, 1, AnyPropertyType,
                             &actual, &format, &count, &remaining,
                             &portion)) break; // quit on error

      if ((fl_xevent->xselection.property == PRIMARY_TIMESTAMP) ||
          (fl_xevent->xselection.property == CLIPBOARD_TIMESTAMP)) {
        if (portion && format == 32 && count == 1) {
          Time t = *(unsigned int*)portion;
          if (fl_xevent->xselection.property == CLIPBOARD_TIMESTAMP)
            handle_clipboard_timestamp(1, t);
          else
            handle_clipboard_timestamp(0, t);
        }
        XFree(portion);
        return true;
      }

      if (actual == TARGETS || actual == XA_ATOM) {
        Atom type;
        if (Fl::e_clipboard_type == Fl::clipboard_image) { // searching for image data
          type = find_target_image((Atom *)portion, count);
        }
        else { // searching for text data. *FIXME* - there may be other data types!
          type = find_target_text((Atom *)portion, count);
        }
        XFree(portion);

        if (!type) { // not found
          if (Fl::e_clipboard_type == Fl::clipboard_image)
            return true;
          else
            type = fl_XaUtf8String; // text: try this anyway (was: XA_STRING)
        } // not found

        Atom property = xevent.xselection.property;
        XConvertSelection(fl_display, property, type, property,
              fl_xid(Fl::first_window()),
              fl_event_time);
        if (type == fl_XaImageBmp) {
          Fl::e_clipboard_type = Fl::clipboard_image;
        }
        else if (type == fl_XaImagePNG) {
          Fl::e_clipboard_type = Fl::clipboard_image;
        }
        else {
          Fl::e_clipboard_type = Fl::clipboard_plain_text;
        }
        return true;
      }
      if (actual == fl_INCR) {
        // From ICCCM: "The contents of the INCR property will be an integer, which
        // represents a lower bound on the number of bytes of data in the selection."
        //
        // However, some X clients don't set the integer ("lower bound") in the INCR
        // property, hence 'count' below is zero and we must not access '*portion'.
        // Debug:
#if (0)
        fprintf(stderr,
                "[fl_handle(SelectionNotify/INCR):%d] actual=%ld (INCR), format=%d, count=%ld, remaining=%ld",
                __LINE__, actual, format, count, remaining);
        if (portion && count > 0) {
          fprintf(stderr,
                ", portion=%p (%ld)", portion, *(long*)portion);
        }
        fprintf(stderr, "\n");
#endif
        size_t lower_bound = 0;
        if (portion && count > 0) {
          lower_bound = *(unsigned long *)portion;
        }
        bytesread = getIncrData(sn_buffer, xevent.xselection, lower_bound);
        XFree(portion);
        break;
      }
      // Make sure we got something sane...
      if ((portion == NULL) || (format != 8) || (count == 0)) {
        if (portion) XFree(portion);
        return true;
      }
      sn_buffer = (unsigned char*)realloc(sn_buffer, bytesread+count+remaining+1);
      memcpy(sn_buffer + bytesread, portion, count);
      XFree(portion);
      bytesread += count;
      // Cannot trust data to be null terminated
      sn_buffer[bytesread] = '\0';
      if (!remaining) break;
    }
    if (sn_buffer && Fl::e_clipboard_type == Fl::clipboard_plain_text) {
      sn_buffer[bytesread] = 0;
      convert_crlf(sn_buffer, bytesread);
    }
    if (!fl_selection_requestor) return 0;
    if (Fl::e_clipboard_type == Fl::clipboard_image) {
      if (bytesread == 0) return 0;
      static char tmp_fname[21];
      static Fl_Shared_Image *shared = 0;
      strcpy(tmp_fname, "/tmp/clipboardXXXXXX");
      int fd = mkstemp(tmp_fname);
      if (fd == -1) return 0;
      uchar *p = sn_buffer; ssize_t towrite = bytesread, written;
      while (towrite) {
        written = write(fd, p, towrite);
        p += written; towrite -= written;
        }
      close(fd);
      free(sn_buffer); sn_buffer = 0;
      shared = Fl_Shared_Image::get(tmp_fname);
      fl_unlink(tmp_fname);
      if (!shared) return 0;
      uchar *rgb = new uchar[shared->w() * shared->h() * shared->d()];
      memcpy(rgb, shared->data()[0], shared->w() * shared->h() * shared->d());
      Fl_RGB_Image *image = new Fl_RGB_Image(rgb, shared->w(), shared->h(), shared->d());
      shared->release();
      image->alloc_array = 1;
      Fl::e_clipboard_data = (void*)image;
    }
    else if (Fl::e_clipboard_type == Fl::clipboard_plain_text) {
      Fl::e_text = sn_buffer ? (char*)sn_buffer : (char *)"";
      Fl::e_length = bytesread;
    }
    int old_event = Fl::e_number;
    int retval = fl_selection_requestor->handle(Fl::e_number = FL_PASTE);
    if (!retval && Fl::e_clipboard_type == Fl::clipboard_image) {
      delete (Fl_RGB_Image*)Fl::e_clipboard_data;
      Fl::e_clipboard_data = NULL;
    }
    Fl::e_number = old_event;
    // Detect if this paste is due to Xdnd by the property name (I use
    // XA_SECONDARY for that) and send an XdndFinished message.
    // This has to be delayed until now rather than sending it immediately
    // after calling XConvertSelection because we need to send the success
    // status (retval) and the performed action to the sender - at least
    // since XDND protocol version 5 (see docs).
    // [FIXME: is the condition below really correct?]

    if (fl_xevent->xselection.property == XA_SECONDARY && fl_dnd_source_window) {
      fl_sendClientMessage(fl_dnd_source_window,            // send to window
                           fl_XdndFinished,                 // XdndFinished message
                           fl_xevent->xselection.requestor, // data.l[0] target window
                           retval ? 1 : 0,                  // data.l[1] Bit 0: 1 = success
                           retval ? fl_dnd_action : None);  // data.l[2] action performed
      fl_dnd_source_window = 0; // don't send a second time
    }
    return 1;
  } // SelectionNotify

  case SelectionClear: {
    int clipboard = fl_xevent->xselectionclear.selection == CLIPBOARD;
    fl_i_own_selection[clipboard] = 0;
    poll_clipboard_owner();
    return 1;
  }

  case SelectionRequest: {
    XSelectionEvent e;
    e.type = SelectionNotify;
    e.requestor = fl_xevent->xselectionrequest.requestor;
    e.selection = fl_xevent->xselectionrequest.selection;
    int clipboard = e.selection == CLIPBOARD;
    e.target = fl_xevent->xselectionrequest.target;
    e.time = fl_xevent->xselectionrequest.time;
    e.property = fl_xevent->xselectionrequest.property;
    if (fl_selection_type[clipboard] == Fl::clipboard_plain_text) {
      if (e.target == TARGETS) {
        Atom a[3] = {fl_XaUtf8String, XA_STRING, fl_XaText};
        XChangeProperty(fl_display, e.requestor, e.property,
                        XA_ATOM, atom_bits, 0, (unsigned char*)a, 3);
      } else {
        if (fl_selection_length[clipboard]) { // data available
          if (e.target == fl_XaUtf8String ||
              e.target == XA_STRING ||
              e.target == fl_XaCompoundText ||
              e.target == fl_XaText ||
              e.target == fl_Xatextplain ||
              e.target == fl_Xatextplainutf ||
              e.target == fl_Xatextplainutf2) {
            // clobber the target type, this seems to make some applications
            // behave that insist on asking for XA_TEXT instead of UTF8_STRING
            // Does not change XA_STRING as that breaks xclipboard.
            if (e.target != XA_STRING) e.target = fl_XaUtf8String;
            XChangeProperty(fl_display, e.requestor, e.property,
                            e.target, 8, 0,
                            (unsigned char *)fl_selection_buffer[clipboard],
                            fl_selection_length[clipboard]);
          }
        } else { // no data available
          e.property = 0;
        }
      }
    } else { // image in clipboard
      if (e.target == TARGETS) {
        Atom a[1] = {fl_XaImageBmp};
        XChangeProperty(fl_display, e.requestor, e.property,
                        XA_ATOM, atom_bits, 0, (unsigned char*)a, 1);
      } else {
        if (e.target == fl_XaImageBmp && fl_selection_length[clipboard]) {
            XChangeProperty(fl_display, e.requestor, e.property,
                            e.target, 8, 0,
                            (unsigned char *)fl_selection_buffer[clipboard],
                            fl_selection_length[clipboard]);
        } else {
          e.property = 0;
        }
      }
    }
    XSendEvent(fl_display, e.requestor, 0, 0, (XEvent *)&e);
    return 1;
  } // SelectionRequest

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
  } // switch (xevent.type)

  int event = 0;
  Fl_Window* window = fl_find(xid);

  if (window) switch (xevent.type) {

    case DestroyNotify: { // an X11 window was closed externally from the program
      Fl::handle(FL_CLOSE, window);
      Fl_X* X = Fl_X::flx(window);
      if (X) { // indicates the FLTK window was not closed
        X->xid = (Window)0; // indicates the X11 window was already destroyed
        window->hide();
        int oldx = window->x(), oldy = window->y();
        window->position(0, 0);
        window->position(oldx, oldy);
        window->show(); // recreate the X11 window in support of the FLTK window
      }
      return 1;
    }
  case ClientMessage: {
    Atom message = fl_xevent->xclient.message_type;
    const long* data = fl_xevent->xclient.data.l;
    if ((Atom)(data[0]) == WM_DELETE_WINDOW) {
      event = FL_CLOSE;
    } else if (message == fl_XdndEnter) {
      /*
       Excerpt from the XDND protocol at https://www.freedesktop.org/wiki/Specifications/XDND/ :
       - data.l[0] contains the XID of the source window.
       - data.l[1]:
           Bit 0 is set if the source supports more than three data types.
           The high byte contains the protocol version to use (minimum of the source's and
           target's highest supported versions). The rest of the bits are reserved for future use.
       - data.l[2,3,4] contain the first three types that the source supports. Unused slots are set
           to None. The ordering is arbitrary.

       If the Source supports more than three data types, bit 0 of data.l[1] is set. This tells the
         Target to check the property XdndTypeList on the Source window for the list of available
         types. This property should contain all the available types.

       BUT wayland gnome apps (e.g., gnome-text-editor) set bit 0 of data.l[1]
       even though their source supports 2 data types (UTF8 text + a gnome-specific type)
       and put None (==0) in each of data.l[2,3,4].
       The same gnome apps run in X11 mode (GDK_BACKEND=x11) clear bit 0 of data.l[1]
       and support only UTF8 text announced in data.l[2].
       FLTK wayland apps set bit 0 of data.l[1] and support only UTF8 text.

       Overall, the correct procedure is
       if (bit 0 of data.l[1] is set) {
         get the XdndTypeList property
         use all the data types it returns which can be in any number ≥ 1
       } else {
         the source supports 1, 2 or 3 data types available at data.l[2,3,4]
       }
       */
#if FLTK_CONSOLIDATE_MOTION
      fl_xmousewin = window;
#endif // FLTK_CONSOLIDATE_MOTION
      in_a_window = true;
      fl_dnd_source_window = data[0];
      // version number is data[1]>>24
      // fprintf(stderr, "XdndEnter, version %ld\n", data[1] >> 24);
      if (data[1]&1) {
        // get list of data types:
        Atom actual; int format; unsigned long count, remaining;
        unsigned char *cm_buffer = 0;
        XGetWindowProperty(fl_display, fl_dnd_source_window, fl_XdndTypeList,
                           0, 0x8000000L, False, XA_ATOM, &actual, &format,
                           &count, &remaining, &cm_buffer);
        if (actual != XA_ATOM || format != 32 || count <= 0 || !cm_buffer) {
          if ( cm_buffer ) { XFree(cm_buffer); cm_buffer = 0; }
          goto FAILED;
        }
        delete [] fl_dnd_source_types;
        fl_dnd_source_types = new Atom[count+1];
        for (unsigned i = 0; i < count; i++) {
          fl_dnd_source_types[i] = ((Atom*)cm_buffer)[i];
        }
        fl_dnd_source_types[count] = 0;
        XFree(cm_buffer); cm_buffer = 0;
      } else {
      FAILED:
        // less than four data types, or if the above messes up:
        if (!fl_dnd_source_types) fl_dnd_source_types = new Atom[4];
        fl_dnd_source_types[0] = data[2];
        fl_dnd_source_types[1] = data[3];
        fl_dnd_source_types[2] = data[4];
        fl_dnd_source_types[3] = 0;
      }

      // Pick the "best" source (text) type...
      // *FIXME* what if we don't find a suitable type? (see below: first type?)
      // *FIXME* count (zero terminated) dnd sources (must be at least 1)
      int dnd_sources;
      for (dnd_sources = 0; fl_dnd_source_types[dnd_sources]; dnd_sources++) {
        // empty
      }
      fl_dnd_type = find_target_text(fl_dnd_source_types, dnd_sources);
      if (!fl_dnd_type) // not found: pick first type
        fl_dnd_type = fl_dnd_source_types[0];

      event = FL_DND_ENTER;
      Fl::e_text = unknown;
      Fl::e_length = unknown_len;
      break;

    } else if (message == fl_XdndPosition) {
#if FLTK_CONSOLIDATE_MOTION
      fl_xmousewin = window;
#endif // FLTK_CONSOLIDATE_MOTION
      in_a_window = true;
      fl_dnd_source_window = data[0];
      float s = 1;
#if USE_XFT
      if (window) s = Fl::screen_driver()->scale(Fl_Window_Driver::driver(window)->screen_num());
#endif
      Fl::e_x_root = (data[2]>>16)/s;
      Fl::e_y_root = (data[2]&0xFFFF)/s;
      if (window) {
        Fl::e_x = Fl::e_x_root-window->x();
        Fl::e_y = Fl::e_y_root-window->y();
      }
      fl_event_time = data[3];
      fl_dnd_source_action = data[4];
      fl_dnd_action = fl_XdndActionCopy;
      Fl::e_text = unknown;
      Fl::e_length = unknown_len;
      int accept = Fl::handle(FL_DND_DRAG, window);
      fl_sendClientMessage(data[0], fl_XdndStatus,
                           fl_xevent->xclient.window,
                           accept ? 1 : 0,
                           0, // used for xy rectangle to not send position inside
                           0, // used for width+height of rectangle
                           accept ? fl_dnd_action : None);
      return 1;

    } else if (message == fl_XdndLeave) {
      fl_dnd_source_window = 0; // don't send a finished message to it
      event = FL_DND_LEAVE;
      Fl::e_text = unknown;
      Fl::e_length = unknown_len;
      break;

    } else if (message == fl_XdndDrop) {
#if FLTK_CONSOLIDATE_MOTION
      fl_xmousewin = window;
#endif // FLTK_CONSOLIDATE_MOTION
      in_a_window = true;
      fl_dnd_source_window = data[0];
      fl_event_time = data[2];
      Window to_window = fl_xevent->xclient.window;
      Fl::e_text = unknown;
      Fl::e_length = unknown_len;
      if (Fl::handle(FL_DND_RELEASE, window)) {
        fl_selection_requestor = Fl::belowmouse();
        Fl::e_clipboard_type = Fl::clipboard_plain_text;
        XConvertSelection(fl_display, fl_XdndSelection,
                          fl_dnd_type, XA_SECONDARY,
                          to_window, fl_event_time);
      } else {
        // Send the finished message if I refuse the drop.
        // It is not clear whether I can just send finished always,
        // or if I have to wait for the SelectionNotify event as the
        // code is currently doing.
        fl_sendClientMessage(fl_dnd_source_window, fl_XdndFinished, to_window);
        fl_dnd_source_window = 0;
      }
      return 1;

    }
    break;
  } // case ClientMessage

  case UnmapNotify:
    event = FL_HIDE;
    break;

  case Expose:
    Fl_Window_Driver::driver(window)->wait_for_expose_value = 0;
#  if 0
    // try to keep windows on top even if WM_TRANSIENT_FOR does not work:
    // opaque move/resize window managers do not like this, so I disabled it.
    if (Fl::first_window()->non_modal() && window != Fl::first_window())
      Fl::first_window()->show();
#  endif

  case GraphicsExpose:
    {
#if USE_XFT
      int ns = Fl_Window_Driver::driver(window)->screen_num();
      float s = Fl::screen_driver()->scale(ns);
      window->damage(FL_DAMAGE_EXPOSE, xevent.xexpose.x/s, xevent.xexpose.y/s,
                     xevent.xexpose.width/s + 2, xevent.xexpose.height/s + 2);
#else
      window->damage(FL_DAMAGE_EXPOSE, xevent.xexpose.x, xevent.xexpose.y,
                     xevent.xexpose.width, xevent.xexpose.height);
#endif
     }
    return 1;

  case FocusIn:
    if (Fl_X11_Screen_Driver::xim_ic) XSetICFocus(Fl_X11_Screen_Driver::xim_ic);
    event = FL_FOCUS;
    // If the user has toggled from another application to this one,
    // then it's a good time to check for clipboard changes.
    poll_clipboard_owner();
    break;

  case FocusOut:
    if (Fl_X11_Screen_Driver::xim_ic) XUnsetICFocus(Fl_X11_Screen_Driver::xim_ic);
    event = FL_UNFOCUS;
    break;

  case KeyPress:
  case KeyRelease: {
  KEYPRESS:
    int keycode = xevent.xkey.keycode;
    fl_key_vector[keycode/8] |= (1 << (keycode%8));
    static char *kp_buffer = NULL;
    static int kp_buffer_len = 0;
    KeySym keysym;
    if (kp_buffer_len == 0) {
      kp_buffer_len = 4096;
      kp_buffer = (char*) malloc(kp_buffer_len);
    }
    if (xevent.type == KeyPress) {
      event = FL_KEYDOWN;

      int len;
      if (Fl_X11_Screen_Driver::xim_ic) {
        Status status;
        len = XUtf8LookupString(Fl_X11_Screen_Driver::xim_ic, (XKeyPressedEvent *)&xevent.xkey,
                             kp_buffer, kp_buffer_len, &keysym, &status);

        while (status == XBufferOverflow && kp_buffer_len < 50000) {
          kp_buffer_len = kp_buffer_len * 5 + 1;
          kp_buffer = (char*)realloc(kp_buffer, kp_buffer_len);
          len = XUtf8LookupString(Fl_X11_Screen_Driver::xim_ic, (XKeyPressedEvent *)&xevent.xkey,
                             kp_buffer, kp_buffer_len, &keysym, &status);
        }
        keysym = fl_KeycodeToKeysym(fl_display, keycode, 0);
      } else {
        //static XComposeStatus compose;
        len = XLookupString((XKeyEvent*)&(xevent.xkey),
                             kp_buffer, kp_buffer_len, &keysym, 0/*&compose*/);
        if (keysym && keysym < 0x400) { // a character in latin-1,2,3,4 sets
          // force it to type a character (not sure if this ever is needed):
          // if (!len) {kp_buffer[0] = char(keysym); len = 1;}
          len = fl_utf8encode(XKeysymToUcs(keysym), kp_buffer);
          if (len < 1) len = 1;
          // ignore all effects of shift on the keysyms, which makes it a lot
          // easier to program shortcuts and is Windoze-compatible:
          keysym = fl_KeycodeToKeysym(fl_display, keycode, 0);
        }
      }
      kp_buffer[len] = 0;
      Fl::e_text = kp_buffer;
      Fl::e_length = len;
    } else {
      // Stupid X sends fake key-up events when a repeating key is held
      // down, probably due to some back compatibility problem. Fortunately
      // we can detect this because the repeating KeyPress event is in
      // the queue, get it and execute it instead:

      // Bool XkbSetDetectableAutoRepeat ( display, detectable, supported_rtrn )
      // Display * display ;
      // Bool detectable ;
      // Bool * supported_rtrn ;
      // ...would be the easy way to correct this issue. Unfortunately, this call is also
      // broken on many Unix distros including Ubuntu and Solaris (as of Dec 2009)

      // Bogus KeyUp events are generated by repeated KeyDown events. One
      // necessary condition is an identical key event pending right after
      // the bogus KeyUp.
      // The new code introduced Dec 2009 differs in that it only checks the very
      // next event in the queue, not the entire queue of events.
      // This function wrongly detects a repeat key if a software keyboard
      // sends a burst of events containing two consecutive equal keys. However,
      // in every non-gaming situation, this is no problem because both KeyPress
      // events will cause the expected behavior.
      XEvent peekevent;
      if (XPending(fl_display)) {
        XPeekEvent(fl_display, &peekevent);
        if (   (peekevent.type == KeyPress) // must be a KeyPress event
            && (peekevent.xkey.keycode == xevent.xkey.keycode) // must be the same key
            && (peekevent.xkey.time == xevent.xkey.time) // must be sent at the exact same time
            ) {
          XNextEvent(fl_display, &xevent);
          goto KEYPRESS;
        }
      }

      event = FL_KEYUP;
      fl_key_vector[keycode/8] &= ~(1 << (keycode%8));
      // keyup events just get the unshifted keysym:
      keysym = fl_KeycodeToKeysym(fl_display, keycode, 0);
    }
#  ifdef __sgi
    // You can plug a microsoft keyboard into an sgi but the extra shift
    // keys are not translated.  Make them translate like XFree86 does:
    if (!keysym) switch(keycode) {
    case 147: keysym = FL_Meta_L; break;
    case 148: keysym = FL_Meta_R; break;
    case 149: keysym = FL_Menu; break;
    }
#  endif
#  ifdef BACKSPACE_HACK
    // Attempt to fix keyboards that send "delete" for the key in the
    // upper-right corner of the main keyboard.  But it appears that
    // very few of these remain?
    static int got_backspace = 0;
    if (!got_backspace) {
      if (keysym == FL_Delete) keysym = FL_BackSpace;
      else if (keysym == FL_BackSpace) got_backspace = 1;
    }
#  endif
    // For the first few years, there wasn't a good consensus on what the
    // Windows keys should be mapped to for X11. So we need to help out a
    // bit and map all variants to the same FLTK key...
    switch (keysym) {
        case XK_Meta_L:
        case XK_Hyper_L:
        case XK_Super_L:
          keysym = FL_Meta_L;
          break;
        case XK_Meta_R:
        case XK_Hyper_R:
        case XK_Super_R:
          keysym = FL_Meta_R;
          break;
      }
    // Convert the multimedia keys to safer, portable values
    switch (keysym) { // XF names come from X11/XF86keysym.h
      case 0x1008FF11: // XF86XK_AudioLowerVolume:
        keysym = FL_Volume_Down;
        break;
      case 0x1008FF12: // XF86XK_AudioMute:
        keysym = FL_Volume_Mute;
        break;
      case 0x1008FF13: // XF86XK_AudioRaiseVolume:
        keysym = FL_Volume_Up;
        break;
      case 0x1008FF14: // XF86XK_AudioPlay:
        keysym = FL_Media_Play;
        break;
      case 0x1008FF15: // XF86XK_AudioStop:
        keysym = FL_Media_Stop;
        break;
      case 0x1008FF16: // XF86XK_AudioPrev:
        keysym = FL_Media_Prev;
        break;
      case 0x1008FF17: // XF86XK_AudioNext:
        keysym = FL_Media_Next;
        break;
      case 0x1008FF18: // XF86XK_HomePage:
        keysym = FL_Home_Page;
        break;
      case 0x1008FF19: // XF86XK_Mail:
        keysym = FL_Mail;
        break;
      case 0x1008FF1B: // XF86XK_Search:
        keysym = FL_Search;
        break;
      case 0x1008FF26: // XF86XK_Back:
        keysym = FL_Back;
        break;
      case 0x1008FF27: // XF86XK_Forward:
        keysym = FL_Forward;
        break;
      case 0x1008FF28: // XF86XK_Stop:
        keysym = FL_Stop;
        break;
      case 0x1008FF29: // XF86XK_Refresh:
        keysym = FL_Refresh;
        break;
      case 0x1008FF2F: // XF86XK_Sleep:
        keysym = FL_Sleep;
        break;
      case 0x1008FF30: // XF86XK_Favorites:
        keysym = FL_Favorites;
        break;
    }

    // Special processing for number keys == keycodes 10-19
    // necessary to support keyboard layouts with digits in uppercase :
    if (keycode >= 10 && keycode <= 18) {
      keysym = '1' + (keycode - 10);
    } else if (keycode == 19) {
      keysym = '0';
    }

    // We have to get rid of the XK_KP_function keys, because they are
    // not produced on Windoze and thus case statements tend not to check
    // for them.  There are 15 of these in the range 0xff91 ... 0xff9f
    if (keysym >= 0xff91 && keysym <= 0xff9f) {
      // Map keypad keysym to character or keysym depending on
      // numlock state...
      unsigned long keysym1 = fl_KeycodeToKeysym(fl_display, keycode, 1);
      if (keysym1 <= 0x7f || (keysym1 > 0xff9f && keysym1 <= FL_KP_Last))
        Fl::e_original_keysym = (int)(keysym1 | FL_KP);
      if ((xevent.xkey.state & Mod2Mask) &&
          (keysym1 <= 0x7f || (keysym1 > 0xff9f && keysym1 <= FL_KP_Last))) {
        // Store ASCII numeric keypad value...
        keysym = keysym1 | FL_KP;
        kp_buffer[0] = char(keysym1) & 0x7F;
        // len = 1;
      } else {
        // Map keypad to special key...
        static const unsigned short table[15] = {
          FL_F+1, FL_F+2, FL_F+3, FL_F+4,
          FL_Home, FL_Left, FL_Up, FL_Right,
          FL_Down, FL_Page_Up, FL_Page_Down, FL_End,
          0xff0b/*XK_Clear*/, FL_Insert, FL_Delete};
        keysym = table[keysym-0xff91];
      }
    } else {
      // Store this so we can later know if the KP was used
      Fl::e_original_keysym = (int)keysym;
    }
    Fl::e_keysym = int(keysym);

    // replace XK_ISO_Left_Tab (Shift-TAB) with FL_Tab (modifier flags are set correctly by X11)
    if (Fl::e_keysym == 0xfe20) Fl::e_keysym = FL_Tab;

    set_event_xy(window);
    Fl::e_is_click = 0; }
    break;

  // Mouse button "press" event:
  // ---------------------------
  // X11 uses special conventions for mouse "button" numbers:
  //  1-3:  standard mouse buttons left, middle, right in this order
  //  4-5:  scroll wheel up, down    - not reflected in Fl::event_state()
  //  6-7:  scroll wheel left, right - not reflected in Fl::event_state()
  //  8-9:  side buttons back, forward - mapped to 4-5, see below
  // Since X11 pseudo button numbers 4-7 are useless for Fl::event_state() we map
  // real button numbers 8 and 9 to 4 and 5, respectively in FLTK's button numbers
  // and in the event state (Fl::event_state()).
  // Variable `xbutton_state` is used internally to store the status of the extra
  // mouse buttons 4 (back) and 5 (forward) since X11 doesn't keep their status.

  case ButtonPress: {
    int mb = xevent.xbutton.button; // mouse button
    if (mb < 1 || mb > 9) return 0; // unknown or unsupported button, ignore

    // FIXME(?): here we set some event related variables although we *might*
    // ignore an event sent by X because we don't know or want it. This may lead to
    // inconsistencies in Fl::event_key(), Fl::event_state() and more (see set_event_xy).
    // For now we ignore this fact though, it's likely that it never happens.
    // Albrecht, Sep 27, 2024

    Fl::e_keysym = 0;       // init: not used (zero) for scroll wheel events
    set_event_xy(window);
    Fl::e_dx = Fl::e_dy = 0;

    if (mb == Button4 && !Fl::event_shift()) {
      Fl::e_dy = -1;                            // up
      event = FL_MOUSEWHEEL;
    } else if (mb == Button5 && !Fl::event_shift()) {
      Fl::e_dy = +1;                            // down
      event = FL_MOUSEWHEEL;
    } else if (mb == 6 || (mb == Button4 && Fl::event_shift())) {
      Fl::e_dx = -1;                            // left
      event = FL_MOUSEWHEEL;
    } else if (mb == 7 || (mb == Button5 && Fl::event_shift())) {
      Fl::e_dx = +1;                            // right
      event = FL_MOUSEWHEEL;
    } else if (mb < 4 || mb > 7) {              // real mouse *buttons*, not scroll wheel
      if (mb > 7)                               // 8 = back, 9 = forward
        mb -= 4;                                // map to 4 and 5, resp.
      Fl::e_keysym = FL_Button + mb;
      Fl::e_state |= (FL_BUTTON1 << (mb-1));    // set button state
      if (mb == 4) xbutton_state |= FL_BUTTON4; // save extra button state internally
      if (mb == 5) xbutton_state |= FL_BUTTON5; // save extra button state internally
      event = FL_PUSH;
      checkdouble();
    } else { // unknown button or shift combination
      return 0;
    }

#if FLTK_CONSOLIDATE_MOTION
    fl_xmousewin = window;
#endif // FLTK_CONSOLIDATE_MOTION
    in_a_window = true;
    break;
  } // ButtonPress

  // Mouse button release event: for details see ButtonPress above

  case ButtonRelease: {
    int mb = xevent.xbutton.button; // mouse button
    switch (mb) {                   // figure out which real button this is
      case 1:                       // left
      case 2:                       // middle
      case 3:                       // right
        break;                      // continue
      case 8:                       // side button 1 (back)
      case 9:                       // side button 2 (forward)
        mb -= 4;                    // map to 4 and 5, respectively
        break;                      // continue
      default:                      // unknown button or scroll wheel:
        return 0;                   // don't send FL_RELEASE event
    }
    Fl::e_keysym = FL_Button + mb;  // == FL_BUTTON1 .. FL_BUTTON5
    set_event_xy(window);

    Fl::e_state &= ~(FL_BUTTON1 << (mb-1));
    if (mb == 4) xbutton_state &= ~FL_BUTTON4; // clear internal button state
    if (mb == 5) xbutton_state &= ~FL_BUTTON5; // clear internal button state
    event = FL_RELEASE;

#if FLTK_CONSOLIDATE_MOTION
    fl_xmousewin = window;
#endif // FLTK_CONSOLIDATE_MOTION
    in_a_window = true;
    break;
  } // ButtonRelease

  case PropertyNotify:
    if (xevent.xproperty.atom == fl_NET_WM_STATE) {
      int fullscreen_state = 0;
      int maximize_state = 0;
      if (xevent.xproperty.state != PropertyDelete) {
        unsigned long nitems;
        unsigned long *words = 0;
        if (0 == get_xwinprop(xid, fl_NET_WM_STATE, 64, &nitems, &words) ) {
          for (unsigned long item = 0; item < nitems; item++) {
            if (words[item] == fl_NET_WM_STATE_FULLSCREEN) {
              fullscreen_state = 1;
            }
            if (words[item] == fl_NET_WM_STATE_MAXIMIZED_HORZ) {
              maximize_state = 1;
            }
          }
        }
        if ( words ) { XFree(words); words = 0; }
      }
      Fl_Window_Driver::driver(window)->is_maximized(maximize_state);
      if (window->fullscreen_active() && !fullscreen_state) {
        window->_clear_fullscreen();
        event = FL_FULLSCREEN;
      }
      if (!window->fullscreen_active() && fullscreen_state) {
        window->_set_fullscreen();
        event = FL_FULLSCREEN;
      }
    }
    break;

  case MotionNotify:
    set_event_xy(window);
    in_a_window = true;
#  if FLTK_CONSOLIDATE_MOTION
    send_motion = fl_xmousewin = window;
    return 0;
#  else
    event = FL_MOVE;
    break;
#  endif

  case EnterNotify:
    if (xevent.xcrossing.detail == NotifyInferior) break;
    // XInstallColormap(fl_display, Fl_X::flx(window)->colormap);
    set_event_xy(window);
    Fl::e_state = xevent.xcrossing.state << 16;
    event = FL_ENTER;

#if FLTK_CONSOLIDATE_MOTION
    fl_xmousewin = window;
#endif // FLTK_CONSOLIDATE_MOTION
    in_a_window = true;
    { XIMStyles *xim_styles = NULL;
      if(!Fl_X11_Screen_Driver::xim_im || XGetIMValues(Fl_X11_Screen_Driver::xim_im, XNQueryInputStyle, &xim_styles, NULL, NULL)) {
        Fl_X11_Screen_Driver::init_xim();
      }
      if (xim_styles) XFree(xim_styles);
    }
    break;

  case LeaveNotify:
    if (xevent.xcrossing.detail == NotifyInferior) break;
    set_event_xy(window);
    Fl::e_state = xevent.xcrossing.state << 16;
#if FLTK_CONSOLIDATE_MOTION
    fl_xmousewin = 0;
#endif // FLTK_CONSOLIDATE_MOTION
    in_a_window = false; // make do_queued_events produce FL_LEAVE event
    return 0;

  // We cannot rely on the x,y position in the configure notify event.
  // I now think this is an unavoidable problem with X: it is impossible
  // for a window manager to prevent the "real" notify event from being
  // sent when it resizes the contents, even though it can send an
  // artificial event with the correct position afterwards (and some
  // window managers do not send this fake event anyway)
  // So anyway, do a round trip to find the correct x,y:
  case MapNotify:
    event = FL_SHOW;

  case ConfigureNotify: {
    if (window->parent()) break; // ignore child windows

    // figure out where OS really put window
    XWindowAttributes actual;
    XGetWindowAttributes(fl_display, fl_xid(window), &actual);
    Window cr; int X, Y, W = actual.width, H = actual.height;
    XTranslateCoordinates(fl_display, fl_xid(window), actual.root,
                          0, 0, &X, &Y, &cr);
#if USE_XFT // detect when window centre changes screen
    Fl_X11_Screen_Driver *d = (Fl_X11_Screen_Driver*)Fl::screen_driver();
    Fl_X11_Window_Driver *wd = Fl_X11_Window_Driver::driver(window);
    int olds = wd->screen_num();
    int num = d->screen_num_unscaled(X+ actual.width/2, Y +actual.height/2);
    if (num == -1) num = olds;
    float s = d->scale(num);
    if (num != olds) {
      if (s != d->scale(olds) &&
          !Fl_X11_Window_Driver::data_for_resize_window_between_screens_.busy &&
          window->user_data() != &Fl_X11_Screen_Driver::transient_scale_display) {
        Fl_X11_Window_Driver::data_for_resize_window_between_screens_.busy = true;
        Fl_X11_Window_Driver::data_for_resize_window_between_screens_.screen = num;
        // resize_after_screen_change() works also if called here, but calling it
        // a second later gives a more pleasant user experience when moving windows between distinct screens
        Fl::add_timeout(1, Fl_X11_Window_Driver::resize_after_screen_change, window);
      } else if (!Fl_X11_Window_Driver::data_for_resize_window_between_screens_.busy)
        wd->screen_num(num);
    } else if (Fl_X11_Window_Driver::data_for_resize_window_between_screens_.busy) {
      Fl::remove_timeout(Fl_X11_Window_Driver::resize_after_screen_change, window);
      Fl_X11_Window_Driver::data_for_resize_window_between_screens_.busy = false;
    }
#else // ! USE_XFT
    Fl_Window_Driver::driver(window)->screen_num( Fl::screen_num(X, Y, W, H) );
#endif // USE_XFT

    // tell Fl_Window about it and set flag to prevent echoing:
    resize_bug_fix = window;
#if USE_XFT
    if (!Fl_X11_Window_Driver::data_for_resize_window_between_screens_.busy &&
      ( ceil(W/s) != window->w() || ceil(H/s) != window->h() ) ) {
        window->resize(rint(X/s), rint(Y/s), ceil(W/s), ceil(H/s));
    } else {
      window->position(rint(X/s), rint(Y/s));
    }
#else
    window->resize(X, Y, W, H);
#endif
    break; // allow add_handler to do something too
    }

  case ReparentNotify: {
    int xpos, ypos;
    Window junk;

    // on some systems, the ReparentNotify event is not handled as we would expect.
    XErrorHandler oldHandler = XSetErrorHandler(catchXExceptions());

    //ReparentNotify gives the new position of the window relative to
    //the new parent. FLTK cares about the position on the root window.
    XTranslateCoordinates(fl_display, xevent.xreparent.parent,
                          XRootWindow(fl_display, fl_screen),
                          xevent.xreparent.x, xevent.xreparent.y,
                          &xpos, &ypos, &junk);
    XSetErrorHandler(oldHandler);

    // tell Fl_Window about it and set flag to prevent echoing:
    if ( !wasXExceptionRaised() ) {
      resize_bug_fix = window;
#if USE_XFT
      int ns = Fl_Window_Driver::driver(window)->screen_num();
      float s = Fl::screen_driver()->scale(ns);
#else
      float s = 1;
#endif
      window->position(rint(xpos/s), rint(ypos/s));
    }
    break;
  } // ReparentNotify
  } // if (window) switch (xevent.type)

#if HAVE_XFIXES
  switch (xevent.type - xfixes_event_base) {
  case XFixesSelectionNotify: {
    // Someone feeding us bogus events?
    if (!have_xfixes)
      return true;

    XFixesSelectionNotifyEvent *selection_notify = (XFixesSelectionNotifyEvent *)&xevent;

    if ((selection_notify->selection == XA_PRIMARY) && !fl_i_own_selection[0])
      handle_clipboard_timestamp(0, selection_notify->selection_timestamp);
    else if ((selection_notify->selection == CLIPBOARD) && !fl_i_own_selection[1])
      handle_clipboard_timestamp(1, selection_notify->selection_timestamp);

    return true;
    }
  }
#endif

  return Fl::handle(event, window);
}

////////////////////////////////////////////////////////////////

void Fl_X11_Window_Driver::resize(int X,int Y,int W,int H) {
  int is_a_rescale = Fl_Window::is_a_rescale();
  int is_a_move = (X != x() || Y != y() || is_a_rescale);
  int is_a_resize = (W != w() || H != h() || is_a_rescale);
  int resize_from_program = (pWindow != resize_bug_fix);
  if (!resize_from_program) resize_bug_fix = 0;
  if (is_a_move && resize_from_program) force_position(1);
  else if (!is_a_resize && !is_a_move) return;
  if (is_a_resize) {
    if (pWindow->as_double_window() && pWindow->parent()) {
      if (W < 1) W = 1;
      if (H < 1) H = 1;
    }
    pWindow->Fl_Group::resize(X,Y,W,H);
    if (shown()) {
#if FLTK_USE_CAIRO
      if (!pWindow->as_gl_window() && cairo_) {
        float s = Fl::screen_driver()->scale(screen_num());
        cairo_xlib_surface_set_size(cairo_get_target(cairo_), (W>0 ? int(W*s) : 1), (H>0 ? int(H*s) : 1));
      }
#endif
      pWindow->redraw();
    }
  } else {
    x(X); y(Y);
    if (Fl_X11_Screen_Driver::xim_win && Fl::focus()) {
      // Force the Input Method auxiliary window to move too.
      Fl::focus()->handle(FL_FOCUS);
      fl_set_spot(fl_font(), fl_size(), Fl::focus()->x(), Fl::focus()->y() + fl_size(), Fl::focus()->w(), Fl::focus()->h(), NULL);
    }
  }

  if (is_a_rescale) size_range();

  if (resize_from_program && shown()) {
    float s = Fl::screen_driver()->scale(screen_num());
    if (is_a_resize) {
      if (!is_resizable()) pWindow->size_range(w(),h(),w(),h());
      if (is_a_move) {
        XMoveResizeWindow(fl_display, fl_xid(pWindow), rint(X*s), rint(Y*s), W>0 ? W*s : 1, H>0 ? H*s : 1);
      } else {
        XResizeWindow(fl_display, fl_xid(pWindow), W>0 ? W*s : 1, H>0 ? H*s : 1);
      }
    } else
      XMoveWindow(fl_display, fl_xid(pWindow), rint(X*s), rint(Y*s));
  }
}

////////////////////////////////////////////////////////////////

#define _NET_WM_STATE_REMOVE        0  /* remove/unset property */
#define _NET_WM_STATE_ADD           1  /* add/set property */
#define _NET_WM_STATE_TOGGLE        2  /* toggle property  */

static void send_wm_event(Window wnd, Atom message,
                          unsigned long d0, unsigned long d1=0,
                          unsigned long d2=0, unsigned long d3=0,
                          unsigned long d4=0) {
  XEvent e;
  e.xany.type = ClientMessage;
  e.xany.window = wnd;
  e.xclient.message_type = message;
  e.xclient.format = 32;
  e.xclient.data.l[0] = d0;
  e.xclient.data.l[1] = d1;
  e.xclient.data.l[2] = d2;
  e.xclient.data.l[3] = d3;
  e.xclient.data.l[4] = d4;
  XSendEvent(fl_display, RootWindow(fl_display, fl_screen),
             0, SubstructureNotifyMask | SubstructureRedirectMask,
             &e);
}

static void send_wm_state_event(Window wnd, int add, Atom prop) {
  send_wm_event(wnd, fl_NET_WM_STATE,
                add ? _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE, prop);
}

int Fl_X11_Screen_Driver::ewmh_supported() {
  static int result = -1;

  if (result == -1) {
    fl_open_display();
    result = 0;
    unsigned long nitems;
    unsigned long *words = 0;
    if (0 == get_xwinprop(XRootWindow(fl_display, fl_screen), fl_NET_SUPPORTING_WM_CHECK, 64,
                          &nitems, &words) && nitems == 1) {
      Window child = words[0];
      if ( words ) { XFree(words); words = 0; }
      if (0 == get_xwinprop(child, fl_NET_SUPPORTING_WM_CHECK, 64,
                           &nitems, &words) ) {
        if ( nitems == 1) result = (child == words[0]);
      }
    }
    if ( words ) { XFree(words); words = 0; }
  }

  return result;
}

#if HAVE_XRENDER && (!FLTK_USE_CAIRO)
static int xrender_supported() {
  int nop1, nop2;
  fl_open_display();
  return XRenderQueryExtension(fl_display, &nop1, &nop2);
}
#endif

#if ! FLTK_USE_CAIRO
char Fl_Xlib_Graphics_Driver::can_do_alpha_blending() {
#if HAVE_XRENDER
  static char result = (char)xrender_supported();
  return result;
#else
  return 0;
#endif
}
#endif

extern Fl_Window *fl_xfocus;

void Fl_X11_Window_Driver::activate_window() {
  Window w = fl_xid(pWindow);
  if (!Fl_X11_Screen_Driver::ewmh_supported())
    return;

  Window prev = 0;

  if (fl_xfocus) {
    Fl_X *x = Fl_X::flx(fl_xfocus);
    if (!x)
      return;
    prev = x->xid;
  }

  send_wm_event(w, fl_NET_ACTIVE_WINDOW,
                1,              // source: 1 = application
                fl_event_time,  // time of client's last user activity (STR 3396)
                prev);          // previously active window
}

/* Change an existing window to fullscreen */
void Fl_X11_Window_Driver::fullscreen_on() {
  pWindow->_set_fullscreen();
  if (Fl_X11_Screen_Driver::ewmh_supported()) {
    int top, bottom, left, right;

    top = fullscreen_screen_top();
    bottom = fullscreen_screen_bottom();
    left = fullscreen_screen_left();
    right = fullscreen_screen_right();

    if ((top < 0) || (bottom < 0) || (left < 0) || (right < 0)) {
      top = screen_num();
      bottom = top;
      left = top;
      right = top;
    }

    send_wm_event(fl_xid(pWindow), fl_NET_WM_FULLSCREEN_MONITORS,
                  top, bottom, left, right);
    send_wm_state_event(fl_xid(pWindow), 1, fl_NET_WM_STATE_FULLSCREEN);
  } else {
    hide();
    show();
    /* We want to grab the window, not a widget, so we cannot use Fl::grab */
    XGrabKeyboard(fl_display, fl_xid(pWindow), 1, GrabModeAsync, GrabModeAsync, fl_event_time);
    Fl::handle(FL_FULLSCREEN, pWindow);
  }
}

void Fl_X11_Window_Driver::fullscreen_off(int X, int Y, int W, int H) {
  pWindow->_clear_fullscreen();
  if (Fl_X11_Screen_Driver::ewmh_supported()) {
    send_wm_state_event(fl_xid(pWindow), 0, fl_NET_WM_STATE_FULLSCREEN);
  } else {
    /* The grab will be lost when the window is destroyed */
    hide();
    resize(X,Y,W,H);
    show();
    Fl::handle(FL_FULLSCREEN, pWindow);
  }
}


void Fl_X11_Window_Driver::maximize() {
  if (Fl_X11_Screen_Driver::ewmh_supported()) {
    send_wm_event(fl_xid(pWindow), fl_NET_WM_STATE, _NET_WM_STATE_ADD,
                  fl_NET_WM_STATE_MAXIMIZED_VERT, fl_NET_WM_STATE_MAXIMIZED_HORZ);
  } else {
    *no_fullscreen_x() = x();
    *no_fullscreen_y() = y();
    *no_fullscreen_w() = w();
    *no_fullscreen_h() = h();
    int X,Y,W,H;
    Fl::screen_work_area(X, Y, W, H, screen_num());
    int width, height;
    decorated_win_size(width, height);
    int dw = (width - w());
    int dh = (height - h() - dw);
    resize(X + dw/2, Y + dh + dw/2, W - dw, H - dh - dw);
  }
}

void Fl_X11_Window_Driver::un_maximize() {
  if (Fl_X11_Screen_Driver::ewmh_supported()) {
    send_wm_event(fl_xid(pWindow), fl_NET_WM_STATE, _NET_WM_STATE_REMOVE,
                  fl_NET_WM_STATE_MAXIMIZED_VERT, fl_NET_WM_STATE_MAXIMIZED_HORZ);
  } else {
    resize(*no_fullscreen_x(), *no_fullscreen_y(),
           *no_fullscreen_w(), *no_fullscreen_h());
    *no_fullscreen_x() = 0;
    *no_fullscreen_y() = 0;
    *no_fullscreen_w() = 0;
    *no_fullscreen_h() = 0;
  }
}


////////////////////////////////////////////////////////////////

// A subclass of Fl_Window may call this to associate an X window it
// creates with the Fl_Window:

void fl_fix_focus(); // in Fl.cxx

Fl_X* Fl_X::set_xid(Fl_Window* win, Window winxid) {
  if (!win->parent()) add_xid_vector(winxid); // store xid's of top-level FLTK windows
  Fl_X *xp = new Fl_X;
  xp->xid = winxid;
  Fl_Window_Driver::driver(win)->other_xid = 0;
  xp->w = win; win->flx_ = xp;
  xp->next = Fl_X::first;
  xp->region = 0;
  Fl_Window_Driver::driver(win)->wait_for_expose_value = 1;
  Fl_X::first = xp;
  if (win->modal()) {Fl::modal_ = win; fl_fix_focus();}
  return xp;
}

// More commonly a subclass calls this, because it hides the really
// ugly parts of X and sets all the stuff for a window that is set
// normally.  The global variables like fl_show_iconic are so that
// subclasses of *that* class may change the behavior...

int fl_disable_transient_for; // secret method of removing TRANSIENT_FOR

static const int childEventMask = ExposureMask;

static const int XEventMask =
ExposureMask|StructureNotifyMask
|KeyPressMask|KeyReleaseMask|KeymapStateMask|FocusChangeMask
|ButtonPressMask|ButtonReleaseMask
|EnterWindowMask|LeaveWindowMask
|PropertyChangeMask
|PointerMotionMask;

void Fl_X::make_xid(Fl_Window* win, XVisualInfo *visual, Colormap colormap)
{
  Fl_Group::current(0); // get rid of very common user bug: forgot end()

  int X = win->x();
  int Y = win->y();
  int W = win->w();
  if (W <= 0) W = 1; // X don't like zero...
  int H = win->h();
  if (H <= 0) H = 1; // X don't like zero...
  if (!win->parent() && !Fl::grab()) {
    // center windows in case window manager does not do anything:
#ifdef FL_CENTER_WINDOWS
    if (!win->force_position()) {
      win->x(X = scr_x+(scr_w-W)/2);
      win->y(Y = scr_y+(scr_h-H)/2);
    }
#endif // FL_CENTER_WINDOWS

    // force the window to be on-screen.  Usually the X window manager
    // does this, but a few don't, so we do it here for consistency:
    int scr_x, scr_y, scr_w, scr_h;
    Fl::screen_xywh(scr_x, scr_y, scr_w, scr_h, X, Y, W, H);

    if (win->border()) {
      // ensure border is on screen:
      // (assume extremely minimal dimensions for this border)
      const int top = 20;
      const int left = 1;
      const int right = 1;
      const int bottom = 1;
      if (X+W+right > scr_x+scr_w) X = scr_x+scr_w-right-W;
      if (X-left < scr_x) X = scr_x+left;
      if (Y+H+bottom > scr_y+scr_h) Y = scr_y+scr_h-bottom-H;
      if (Y-top < scr_y) Y = scr_y+top;
    }
    // now insure contents are on-screen (more important than border):
    if (X+W > scr_x+scr_w) X = scr_x+scr_w-W;
    if (X < scr_x) X = scr_x;
    if (Y+H > scr_y+scr_h) Y = scr_y+scr_h-H;
    if (Y < scr_y) Y = scr_y;
  }

  // if the window is a subwindow and our parent is not mapped yet, we
  // mark this window visible, so that mapping the parent at a later
  // point in time will call this function again to finally map the subwindow.
  if (win->parent() && !Fl_X::flx(win->window())) {
    win->set_visible();
    return;
  }

  // Compute which screen(s) we should be on if we want to go fullscreen
  int fullscreen_top, fullscreen_bottom, fullscreen_left, fullscreen_right;

  fullscreen_top = win->fullscreen_screen_top;
  fullscreen_bottom = win->fullscreen_screen_bottom;
  fullscreen_left = win->fullscreen_screen_left;
  fullscreen_right = win->fullscreen_screen_right;

  if ((fullscreen_top < 0) || (fullscreen_bottom < 0) ||
      (fullscreen_left < 0) || (fullscreen_right < 0)) {
    fullscreen_top = Fl::screen_num(X, Y, W, H);
    fullscreen_bottom = fullscreen_top;
    fullscreen_left = fullscreen_top;
    fullscreen_right = fullscreen_top;
  }


  ulong root = win->parent() ?
    fl_xid(win->window()) : RootWindow(fl_display, fl_screen);

  XSetWindowAttributes attr;
  int mask = CWBorderPixel|CWColormap|CWEventMask|CWBitGravity;
  attr.event_mask = win->parent() ? childEventMask : XEventMask;
  attr.colormap = colormap;
  attr.border_pixel = 0;
  attr.bit_gravity = 0; // StaticGravity;
  if (win->override()) {
    attr.override_redirect = 1;
    attr.save_under = 1;
    mask |= CWOverrideRedirect | CWSaveUnder;
  } else attr.override_redirect = 0;
  if (Fl::grab()) {
    attr.save_under = 1; mask |= CWSaveUnder;
    if (!win->border()) {attr.override_redirect = 1; mask |= CWOverrideRedirect;}
  }
  // For the non-EWMH fullscreen case, we cannot use the code above,
  // since we do not want save_under, do not want to turn off the
  // border, and cannot grab without an existing window. Besides,
  // there is no clear_override().
  if (win->fullscreen_active() && !Fl_X11_Screen_Driver::ewmh_supported()) {
    int sx, sy, sw, sh;
    attr.override_redirect = 1;
    mask |= CWOverrideRedirect;
    Fl::screen_xywh(sx, sy, sw, sh, fullscreen_left);
    X = sx;
    Fl::screen_xywh(sx, sy, sw, sh, fullscreen_right);
    W = sx + sw - X;
    Fl::screen_xywh(sx, sy, sw, sh, fullscreen_top);
    Y = sy;
    Fl::screen_xywh(sx, sy, sw, sh, fullscreen_bottom);
    H = sy + sh - Y;
  }

#ifdef ENABLE_BOXCHEAT
  if (fl_background_pixel >= 0) {
    attr.background_pixel = fl_background_pixel;
    fl_background_pixel = -1;
    mask |= CWBackPixel;
  }
#endif // (ENABLE_BOXCHEAT)

  float s = 1;
#if USE_XFT
  //compute adequate screen where to put the window
  int nscreen = 0;
  if (win->parent()) {
    nscreen = Fl_Window_Driver::driver(win->top_window())->screen_num();
  } else if (win->force_position() && Fl_X11_Window_Driver::driver(win)->screen_num_ >= 0) {
    nscreen = Fl_Window_Driver::driver(win)->screen_num();
  } else {
    Fl_Window *hint = Fl::first_window();
    if (hint) {
      nscreen = Fl_Window_Driver::driver(hint->top_window())->screen_num();
    }
  }
  Fl_X11_Window_Driver::driver(win)->screen_num(nscreen);
  s = Fl::screen_driver()->scale(nscreen);
  // if (!win->parent()) printf("win creation on screen #%d\n", nscreen);
#endif
  Fl_X* xp =
    set_xid(win, XCreateWindow(fl_display,
                               root,
                               rint(X*s), rint(Y*s), W*s, H*s,
                               0, // borderwidth
                               visual->depth,
                               InputOutput,
                               visual->visual,
                               mask, &attr));
  int showit = 1;

  // Set WM_CLIENT_MACHINE and WM_LOCALE_NAME
  XSetWMProperties(fl_display, xp->xid, NULL, NULL, NULL, 0, NULL, NULL, NULL);

  // Set _NET_WM_PID
  long pid;
  pid = getpid();
  XChangeProperty(fl_display, xp->xid, fl_NET_WM_PID,
                  XA_CARDINAL, 32, 0, (unsigned char *)&pid, 1);

  if (!win->parent() && !attr.override_redirect) {
    // Communicate all kinds 'o junk to the X Window Manager:

    win->label(win->label(), win->iconlabel());

    XChangeProperty(fl_display, xp->xid, WM_PROTOCOLS,
                    XA_ATOM, 32, 0, (uchar*)&WM_DELETE_WINDOW, 1);

    // send size limits and border:
    Fl_X11_Window_Driver::driver(win)->sendxjunk();

    // set the class property, which controls the icon used:
    if (win->xclass()) {
      char buffer[1024];
      const char *xclass = win->xclass();
      const int len = strlen(xclass);
      // duplicate the xclass string for use as XA_WM_CLASS
      strcpy(buffer, xclass);
      strcpy(buffer + len + 1, xclass);
      // create the capitalized version:
      buffer[len + 1] = toupper(buffer[len + 1]);
      if (buffer[len + 1] == 'X')
        buffer[len + 2] = toupper(buffer[len + 2]);
      XChangeProperty(fl_display, xp->xid, XA_WM_CLASS, XA_STRING, 8, 0,
                      (unsigned char *)buffer, len * 2 + 2);
    }

    if (win->non_modal() && xp->next && !fl_disable_transient_for) {
      // find some other window to be "transient for":
      Fl_Window* wp = xp->next->w;
      while (wp->parent()) wp = wp->window();
      XSetTransientForHint(fl_display, xp->xid, fl_xid(wp));
      if (!wp->visible()) showit = 0; // guess that wm will not show it
      if (win->modal()) {
        Atom net_wm_state = XInternAtom (fl_display, "_NET_WM_STATE", 0);
        Atom net_wm_state_skip_taskbar = XInternAtom (fl_display, "_NET_WM_STATE_MODAL", 0);
        XChangeProperty (fl_display, xp->xid, net_wm_state, XA_ATOM, 32,
            PropModeAppend, (unsigned char*) &net_wm_state_skip_taskbar, 1);
      }
    }

    // Make sure that borderless windows do not show in the task bar
    if (!win->border()) {
      Atom net_wm_state = XInternAtom (fl_display, "_NET_WM_STATE", 0);
      Atom net_wm_state_skip_taskbar = XInternAtom (fl_display, "_NET_WM_STATE_SKIP_TASKBAR", 0);
      XChangeProperty (fl_display, xp->xid, net_wm_state, XA_ATOM, 32,
          PropModeAppend, (unsigned char*) &net_wm_state_skip_taskbar, 1);
    }

    // If asked for, create fullscreen
    if (win->fullscreen_active() && Fl_X11_Screen_Driver::ewmh_supported()) {
      unsigned long data[4];
      data[0] = fullscreen_top;
      data[1] = fullscreen_bottom;
      data[2] = fullscreen_left;
      data[3] = fullscreen_right;
      XChangeProperty (fl_display, xp->xid, fl_NET_WM_FULLSCREEN_MONITORS, XA_ATOM, 32,
                       PropModeReplace, (unsigned char*) data, 4);
      XChangeProperty (fl_display, xp->xid, fl_NET_WM_STATE, XA_ATOM, 32,
                       PropModeAppend, (unsigned char*) &fl_NET_WM_STATE_FULLSCREEN, 1);
    }

    // Make it receptive to DnD:
    Atom version = 5; // max. XDND protocol version we understand
    XChangeProperty(fl_display, xp->xid, fl_XdndAware,
                    XA_ATOM, sizeof(int)*8, 0, (unsigned char*)&version, 1);

    XWMHints *hints = XAllocWMHints();
    hints->input = True;
    hints->flags = InputHint;
    if (Fl_Window::show_next_window_iconic()) {
      hints->flags |= StateHint;
      hints->initial_state = IconicState;
      Fl_Window::show_next_window_iconic(0);
      showit = 0;
    }
    if (Fl_X11_Window_Driver::driver(win)->icon_ &&
        Fl_X11_Window_Driver::driver(win)->icon_->legacy_icon) {
      hints->icon_pixmap = (Pixmap)Fl_X11_Window_Driver::driver(win)->icon_->legacy_icon;
      hints->flags       |= IconPixmapHint;
    }
    XSetWMHints(fl_display, xp->xid, hints);
    XFree(hints);

    Fl_X11_Window_Driver::driver(win)->set_icons();
  }

  // set the window type for menu and tooltip windows to avoid animations (compiz)
  if (win->menu_window() || win->tooltip_window()) {
    Atom net_wm_type = XInternAtom(fl_display, "_NET_WM_WINDOW_TYPE", False);
    Atom net_wm_type_kind = XInternAtom(fl_display, "_NET_WM_WINDOW_TYPE_MENU", False);
    XChangeProperty(fl_display, xp->xid, net_wm_type, XA_ATOM, 32, PropModeReplace, (unsigned char*)&net_wm_type_kind, 1);
  }

#if HAVE_XFIXES
  // register for clipboard change notifications
  if (have_xfixes && !win->parent()) {
    XFixesSelectSelectionInput(fl_display, xp->xid, XA_PRIMARY,
                               XFixesSetSelectionOwnerNotifyMask);
    XFixesSelectSelectionInput(fl_display, xp->xid, CLIPBOARD,
                               XFixesSetSelectionOwnerNotifyMask);
  }
#endif

  if (win->shape()) {
    Fl_X11_Window_Driver::driver(win)->combine_mask();
    }
  XMapWindow(fl_display, xp->xid);
  if (showit) {
    win->set_visible();
    int old_event = Fl::e_number;
    win->handle(Fl::e_number = FL_SHOW); // get child windows to appear
    Fl::e_number = old_event;
    win->redraw();
  }

  // non-EWMH fullscreen case, need grab
  if (win->fullscreen_active() && !Fl_X11_Screen_Driver::ewmh_supported()) {
    XGrabKeyboard(fl_display, xp->xid, 1, GrabModeAsync, GrabModeAsync, fl_event_time);
  }

}

////////////////////////////////////////////////////////////////
// Send X window stuff that can be changed over time:

void Fl_X11_Window_Driver::sendxjunk() {
  Fl_Window *w = pWindow;
  if (w->parent() || w->override()) return; // it's not a window manager window!

  XSizeHints *hints = XAllocSizeHints();
  // memset(&hints, 0, sizeof(hints)); jreiser suggestion to fix purify?
  float s = Fl::screen_driver()->scale(screen_num());

  int minw, minh, maxw, maxh, dw, dh, aspect;
  pWindow->get_size_range(&minw, &minh, &maxw, &maxh, &dw, &dh, &aspect);
  hints->min_width  = s * minw;
  hints->min_height = s * minh;
  hints->max_width  = s * maxw;
  hints->max_height = s * maxh;
  if (int(s) == s) { // use win size increment value only if scale is an integer. Is it possible to do better?
    hints->width_inc  = s * dw;
    hints->height_inc = s * dh;
  } else {
    hints->width_inc  = 0;
    hints->height_inc = 0;
  }

  hints->win_gravity = StaticGravity;

  // see the file /usr/include/X11/Xm/MwmUtil.h:
  // fill all fields to avoid bugs in kwm and perhaps other window managers:
  // 0, MWM_FUNC_ALL, MWM_DECOR_ALL
  long prop[5] = {0, 1, 1, 0, 0};

  if (hints->min_width != hints->max_width ||
      hints->min_height != hints->max_height) { // resizable
    hints->flags = PMinSize|PWinGravity;
    if (hints->max_width >= hints->min_width ||
        hints->max_height >= hints->min_height) {
      hints->flags = PMinSize|PMaxSize|PWinGravity;
      // unfortunately we can't set just one maximum size.  Guess a
      // value for the other one.  Some window managers will make the
      // window fit on screen when maximized, others will put it off screen:
      if (hints->max_width < hints->min_width) hints->max_width = Fl::w()*s;
      if (hints->max_height < hints->min_height) hints->max_height = Fl::h()*s;
    }
    if (hints->width_inc && hints->height_inc) hints->flags |= PResizeInc;
    if (aspect) {
      // stupid X!  It could insist that the corner go on the
      // straight line between min and max...
      hints->min_aspect.x = hints->max_aspect.x = hints->min_width;
      hints->min_aspect.y = hints->max_aspect.y = hints->min_height;
      hints->flags |= PAspect;
    }
  } else { // not resizable:
    hints->flags = PMinSize|PMaxSize|PWinGravity;
    prop[0] = 1; // MWM_HINTS_FUNCTIONS
    prop[1] = 1|2|16; // MWM_FUNC_ALL | MWM_FUNC_RESIZE | MWM_FUNC_MAXIMIZE
  }

  if (force_position()) {
    hints->flags |= USPosition;
    hints->x = s*w->x();
    hints->y = s*w->y();
  }

  if (!w->border()) {
    prop[0] |= 2; // MWM_HINTS_DECORATIONS
    prop[2] = 0; // no decorations
  }

  XSetWMNormalHints(fl_display, fl_xid(w), hints);
  XChangeProperty(fl_display, fl_xid(w),
                  fl_MOTIF_WM_HINTS, fl_MOTIF_WM_HINTS,
                  32, 0, (unsigned char *)prop, 5);
  XFree(hints);
}

////////////////////////////////////////////////////////////////

static unsigned long *default_net_wm_icons = 0L;
static size_t default_net_wm_icons_size = 0;

// Note: icons[] *must* contain at least <count> valid image pointers (!NULL),
//  but: <count> *may* be 0
static void icons_to_property(const Fl_RGB_Image *icons[], int count,
                              unsigned long **property, size_t *len) {
  size_t sz;
  unsigned long *data;

  sz = 0;
  for (int i = 0;i < count;i++)
    sz += 2 + icons[i]->data_w() * icons[i]->data_h();

  // FIXME: Might want to sort the icons

  *property = data = new unsigned long[sz];
  *len = sz;

  for (int i = 0;i < count;i++) {
    const Fl_RGB_Image *image;

    image = icons[i];
    bool need_delete = false;
    if (image->w() != image->data_w() || image->h() != image->data_h()) {
      image = (Fl_RGB_Image*)image->copy();
      need_delete = true;
    }

    data[0] = image->data_w();
    data[1] = image->data_h();
    data += 2;

    const int extra_data = image->ld() ? (image->ld() - image->data_w() * image->d()) : 0;

    const uchar *in = (const uchar*)*image->data();
    for (int y = 0; y < image->data_h(); y++) {
      for (int x = 0; x < image->data_w(); x++) {
        switch (image->d()) {
        case 1:
          *data = ( 0xff<<24) | (in[0]<<16) | (in[0]<<8) | in[0];
          break;
        case 2:
          *data = (in[1]<<24) | (in[0]<<16) | (in[0]<<8) | in[0];
          break;
        case 3:
          *data = ( 0xff<<24) | (in[0]<<16) | (in[1]<<8) | in[2];
          break;
        case 4:
          *data = (in[3]<<24) | (in[0]<<16) | (in[1]<<8) | in[2];
          break;
        }
        in += image->d();
        data++;
      }
      in += extra_data;
    }
    if (need_delete) delete image;
  }
}

void Fl_X11_Screen_Driver::default_icons(const Fl_RGB_Image *icons[], int count) {
  if (default_net_wm_icons) {
    delete [] default_net_wm_icons;
    default_net_wm_icons = 0L;
    default_net_wm_icons_size = 0;
  }

  if (count > 0)
    icons_to_property(icons, count,
                      &default_net_wm_icons, &default_net_wm_icons_size);
}

void Fl_X11_Window_Driver::set_icons() {
  unsigned long *net_wm_icons;
  size_t net_wm_icons_size;

  if (icon_ && icon_->count) {
    icons_to_property((const Fl_RGB_Image **)icon_->icons, icon_->count,
                      &net_wm_icons, &net_wm_icons_size);
  } else {
    net_wm_icons = default_net_wm_icons;
    net_wm_icons_size = default_net_wm_icons_size;
  }

  XChangeProperty (fl_display, fl_xid(pWindow), fl_NET_WM_ICON, XA_CARDINAL, 32,
      PropModeReplace, (unsigned char*) net_wm_icons, net_wm_icons_size);

  if (icon_ && icon_->count) {
    delete [] net_wm_icons;
    net_wm_icons = 0L;
    net_wm_icons_size = 0;
  }
}

////////////////////////////////////////////////////////////////

#if ! HAVE_XCURSOR
static void cache_pixmap_cursor(Fl_Cursor c, Cursor& cursor, Fl_Window *pWindow, Cursor& result) {
  if (cursor != None) { // already cached?
    result = cursor;
    return;
  }
#define CURSORSIZE 16
#define HOTXY 7
  static struct TableEntry {
    uchar bits[CURSORSIZE*CURSORSIZE/8];
    uchar mask[CURSORSIZE*CURSORSIZE/8];
    Cursor cursor;
  } table[] = {
    {{  // FL_CURSOR_NWSE
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x38, 0x00, 0x78, 0x00,
     0xe8, 0x00, 0xc0, 0x01, 0x80, 0x03, 0x00, 0x17, 0x00, 0x1e, 0x00, 0x1c,
     0x00, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
     {
     0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0xfc, 0x00, 0x7c, 0x00, 0xfc, 0x00,
     0xfc, 0x01, 0xec, 0x03, 0xc0, 0x37, 0x80, 0x3f, 0x00, 0x3f, 0x00, 0x3e,
     0x00, 0x3f, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00}},
    {{  // FL_CURSOR_NESW
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x1c, 0x00, 0x1e,
     0x00, 0x17, 0x80, 0x03, 0xc0, 0x01, 0xe8, 0x00, 0x78, 0x00, 0x38, 0x00,
     0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
     {
     0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x3f, 0x00, 0x3e, 0x00, 0x3f,
     0x80, 0x3f, 0xc0, 0x37, 0xec, 0x03, 0xfc, 0x01, 0xfc, 0x00, 0x7c, 0x00,
     0xfc, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00}},
    {{0}, {0}} // FL_CURSOR_NONE & unknown
  };

  Cursor xc = None;
  if (c >= FL_CURSOR_NWSE) {
     TableEntry *q = (c > FL_CURSOR_NESW) ? table+2 : table+(c-FL_CURSOR_NWSE);
   if (!(q->cursor)) {
     XColor dummy = { 0 };
     Pixmap p = XCreateBitmapFromData(fl_display,
     RootWindow(fl_display, fl_screen), (const char*)(q->bits), CURSORSIZE, CURSORSIZE);
     Pixmap m = XCreateBitmapFromData(fl_display,
     RootWindow(fl_display, fl_screen), (const char*)(q->mask), CURSORSIZE, CURSORSIZE);
     q->cursor = XCreatePixmapCursor(fl_display, p,m,&dummy, &dummy, HOTXY, HOTXY);
     XFreePixmap(fl_display, m);
     XFreePixmap(fl_display, p);
    }
    xc = q->cursor;
  }
  XColor fgc;
  uchar r,g,b;
  // hardcoded colors (legacy)
  Fl_Color fg = FL_WHITE;
  Fl_Color bg = FL_BLACK;
  Fl::get_color(fg,r,g,b);
  fgc.red = r<<8; fgc.green = g<<8; fgc.blue = b<<8;
  XColor bgc;
  Fl::get_color(bg,r,g,b);
  bgc.red = r<<8; bgc.green = g<<8; bgc.blue = b<<8;
  XRecolorCursor(fl_display, xc, &fgc, &bgc);
  result = xc;
}
#endif // ! HAVE_XCURSOR

int Fl_X11_Window_Driver::set_cursor(Fl_Cursor c) {

  /* The cursors are cached, because creating one takes 0.5ms including
     opening, reading, and closing theme files. They are kept until program
     exit by design, which valgrind will note as reachable. */
  static Cursor xc_arrow = None;
  static Cursor xc_cross = None;
  static Cursor xc_wait = None;
  static Cursor xc_insert = None;
  static Cursor xc_hand = None;
  static Cursor xc_help = None;
  static Cursor xc_move = None;
  static Cursor xc_ns = None;
  static Cursor xc_we = None;
  static Cursor xc_ne = None;
  static Cursor xc_n = None;
  static Cursor xc_nw = None;
  static Cursor xc_e = None;
  static Cursor xc_w = None;
  static Cursor xc_se = None;
  static Cursor xc_s = None;
  static Cursor xc_sw = None;
#if ! HAVE_XCURSOR
  static Cursor xc_nwse = None;
  static Cursor xc_nesw = None;
  static Cursor xc_none = None;
#endif // ! HAVE_XCURSOR

  Cursor xc;

#define cache_cursor(name, var) if (var == None) { \
                                  var = XCreateFontCursor(fl_display, name); \
                                } \
                                xc = var

  switch (c) {
  case FL_CURSOR_ARROW:   cache_cursor(XC_left_ptr, xc_arrow); break;
  case FL_CURSOR_CROSS:   cache_cursor(XC_tcross, xc_cross); break;
  case FL_CURSOR_WAIT:    cache_cursor(XC_watch, xc_wait); break;
  case FL_CURSOR_INSERT:  cache_cursor(XC_xterm, xc_insert); break;
  case FL_CURSOR_HAND:    cache_cursor(XC_hand2, xc_hand); break;
  case FL_CURSOR_HELP:    cache_cursor(XC_question_arrow, xc_help); break;
  case FL_CURSOR_MOVE:    cache_cursor(XC_fleur, xc_move); break;
  case FL_CURSOR_NS:      cache_cursor(XC_sb_v_double_arrow, xc_ns); break;
  case FL_CURSOR_WE:      cache_cursor(XC_sb_h_double_arrow, xc_we); break;
  case FL_CURSOR_NE:      cache_cursor(XC_top_right_corner, xc_ne); break;
  case FL_CURSOR_N:       cache_cursor(XC_top_side, xc_n); break;
  case FL_CURSOR_NW:      cache_cursor(XC_top_left_corner, xc_nw); break;
  case FL_CURSOR_E:       cache_cursor(XC_right_side, xc_e); break;
  case FL_CURSOR_W:       cache_cursor(XC_left_side, xc_w); break;
  case FL_CURSOR_SE:      cache_cursor(XC_bottom_right_corner, xc_se); break;
  case FL_CURSOR_S:       cache_cursor(XC_bottom_side, xc_s); break;
  case FL_CURSOR_SW:      cache_cursor(XC_bottom_left_corner, xc_sw); break;
#if ! HAVE_XCURSOR
  case FL_CURSOR_NWSE:    cache_pixmap_cursor(c, xc_nwse, pWindow, xc); break;
  case FL_CURSOR_NESW:    cache_pixmap_cursor(c, xc_nesw, pWindow, xc); break;
  case FL_CURSOR_NONE:    cache_pixmap_cursor(c, xc_none, pWindow, xc); break;
#endif // ! HAVE_XCURSOR
  default:
    return 0;
  }

#undef cache_cursor

  XDefineCursor(fl_display, fl_xid(pWindow), xc);

  return 1;
}

int Fl_X11_Window_Driver::set_cursor(const Fl_RGB_Image *image, int hotx, int hoty) {
#if ! HAVE_XCURSOR
  return 0;
#else
  XcursorImage *cursor;
  Cursor xc;

  if ((hotx < 0) || (hotx >= image->w()))
    return 0;
  if ((hoty < 0) || (hoty >= image->h()))
    return 0;

  cursor = XcursorImageCreate(image->w(), image->h());
  if (!cursor)
    return 0;

  image = (Fl_RGB_Image*)image->copy();
  const int extra_data = image->ld() ? (image->ld()-image->w()*image->d()) : 0;
  const uchar *i = (const uchar*)*image->data();
  XcursorPixel *o = cursor->pixels;
  for (int y = 0;y < image->h();y++) {
    for (int x = 0;x < image->w();x++) {
      uchar r, g, b, a;
      switch (image->d()) {
      case 1:
        r = g = b = i[0];
        a = 0xff;
        break;
      case 2:
        r = g = b = i[0];
        a = i[1];
        break;
      case 3:
        r = i[0];
        g = i[1];
        b = i[2];
        a = 0xff;
        break;
      case 4:
        r = i[0];
        g = i[1];
        b = i[2];
        a = i[3];
        break;
      default:
        return 0;
      }
      // Alpha needs to be pre-multiplied for X11
      r = (uchar)((unsigned)r * a / 255);
      g = (uchar)((unsigned)g * a / 255);
      b = (uchar)((unsigned)b * a / 255);

      *o = (a<<24) | (r<<16) | (g<<8) | b;
      i += image->d();
      o++;
    }
    i += extra_data;
  }

  cursor->xhot = hotx;
  cursor->yhot = hoty;

  xc = XcursorImageLoadCursor(fl_display, cursor);
  XDefineCursor(fl_display, fl_xid(pWindow), xc);
  XFreeCursor(fl_display, xc);

  XcursorImageDestroy(cursor);
  delete image;
  return 1;
#endif
}

////////////////////////////////////////////////////////////////


void Fl_X11_Window_Driver::label(const char *name, const char *iname) {
  if (shown() && !parent()) {
    if (!name) name = "";
    int namelen = strlen(name);
    if (!iname) iname = fl_filename_name(name);
    int inamelen = strlen(iname);
    Window win = fl_xid(pWindow);
    XChangeProperty(fl_display, win, fl_NET_WM_NAME,      fl_XaUtf8String, 8, 0, (uchar*)name,  namelen);       // utf8
    XChangeProperty(fl_display, win, XA_WM_NAME,          XA_STRING,       8, 0, (uchar*)name,  namelen);       // non-utf8
    XChangeProperty(fl_display, win, fl_NET_WM_ICON_NAME, fl_XaUtf8String, 8, 0, (uchar*)iname, inamelen);      // utf8
    XChangeProperty(fl_display, win, XA_WM_ICON_NAME,     XA_STRING,       8, 0, (uchar*)iname, inamelen);      // non-utf8
  }
}

////////////////////////////////////////////////////////////////
// Implement the virtual functions for the base Fl_Window class:

void Fl_X11_Window_Driver::show() {
  if (!shown()) {
    fl_open_display();

#ifdef ENABLE_BOXCHEAT
    // Don't set background pixel for double-buffered windows...
    if (pWindow->type() != FL_DOUBLE_WINDOW && can_boxcheat(pWindow->box())) {
      fl_background_pixel = int(fl_xpixel(pWindow->color()));
    }
#endif // (ENABLE_BOXCHEAT)

    makeWindow();
  } else {
    XMapRaised(fl_display, fl_xid(pWindow));
  }
}
