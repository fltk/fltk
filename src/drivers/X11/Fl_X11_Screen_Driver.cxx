//
// "$Id$"
//
// Definition of X11 Screen interface
//
// Copyright 1998-2018 by Bill Spitzak and others.
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


#include "../../config_lib.h"
#include "Fl_X11_Screen_Driver.H"
#include "../Xlib/Fl_Font.H"
#include "Fl_X11_Window_Driver.H"
#include "../Xlib/Fl_Xlib_Graphics_Driver.H"
#include <FL/Fl.H>
#include <FL/platform.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Image_Surface.H>
#include <FL/Fl_Tooltip.H>

#include <sys/time.h>

#if HAVE_XINERAMA
#  include <X11/extensions/Xinerama.h>
#endif

#if USE_XDBE
#include <X11/extensions/Xdbe.h>
#endif

#  include <X11/Xutil.h>
#  ifdef __sgi
#    include <X11/extensions/readdisplay.h>
#  else
#    include <stdlib.h>
#  endif // __sgi

#ifdef DEBUG
#  include <stdio.h>
#endif // DEBUG

extern Atom fl_NET_WORKAREA;
extern XIC fl_xim_ic; // in Fl_x.cxx

// these are set by Fl::args() and override any system colors: from Fl_get_system_colors.cxx
extern const char *fl_fg;
extern const char *fl_bg;
extern const char *fl_bg2;
// end of extern additions workaround

//
// X11 timers
//


////////////////////////////////////////////////////////////////////////
// Timeouts are stored in a sorted list (*first_timeout), so only the
// first one needs to be checked to see if any should be called.
// Allocated, but unused (free) Timeout structs are stored in another
// linked list (*free_timeout).

struct Timeout {
  double time;
  void (*cb)(void*);
  void* arg;
  Timeout* next;
};
static Timeout* first_timeout, *free_timeout;

// I avoid the overhead of getting the current time when we have no
// timeouts by setting this flag instead of getting the time.
// In this case calling elapse_timeouts() does nothing, but records
// the current time, and the next call will actually elapse time.
static char reset_clock = 1;

static void elapse_timeouts() {
  static struct timeval prevclock;
  struct timeval newclock;
  gettimeofday(&newclock, NULL);
  double elapsed = newclock.tv_sec - prevclock.tv_sec +
    (newclock.tv_usec - prevclock.tv_usec)/1000000.0;
  prevclock.tv_sec = newclock.tv_sec;
  prevclock.tv_usec = newclock.tv_usec;
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

/**
 Creates a driver that manages all screen and display related calls.
 
 This function must be implemented once for every platform.
 */
Fl_Screen_Driver *Fl_Screen_Driver::newScreenDriver()
{
#if !USE_XFT
  secret_input_character = '*';
#endif
  return new Fl_X11_Screen_Driver();
}


void Fl_X11_Screen_Driver::display(const char *d)
{
  if (!d) return;
  
  static const char *cmd = "DISPLAY=";
  static const char *ext = ":0.0";
  int nc = strlen(cmd);
  int ne = strlen(ext);
  int nd = strlen(d);

  char *buf = (char *)malloc(nc+ne+nd+1);
  strcpy(buf, cmd);
  strcat(buf, d);
  if (!strchr(d, ':')) {
    strcat(buf, ext);
  }
  putenv(buf);
  free(buf);
}


static int test_visual(XVisualInfo& v, int flags) {
  if (v.screen != fl_screen) return 0;
#if USE_COLORMAP
  if (!(flags & FL_INDEX)) {
    if (v.c_class != StaticColor && v.c_class != TrueColor) return 0;
    if (v.depth <= 8) return 0; // fltk will work better in colormap mode
  }
  if (flags & FL_RGB8) {
    if (v.depth < 24) return 0;
  }
  // for now, fltk does not like colormaps of more than 8 bits:
  if ((v.c_class&1) && v.depth > 8) return 0;
#else
  // simpler if we can't use colormapped visuals at all:
  if (v.c_class != StaticColor && v.c_class != TrueColor) return 0;
#endif
#if USE_XDBE
  if (flags & FL_DOUBLE) {
    static XdbeScreenVisualInfo *xdbejunk;
    if (!xdbejunk) {
      int event_base, error_base;
      if (!XdbeQueryExtension(fl_display, &event_base, &error_base)) return 0;
      Drawable root = RootWindow(fl_display,fl_screen);
      int numscreens = 1;
      xdbejunk = XdbeGetVisualInfo(fl_display,&root,&numscreens);
      if (!xdbejunk) return 0;
    }
    for (int j = 0; ; j++) {
      if (j >= xdbejunk->count) return 0;
      if (xdbejunk->visinfo[j].visual == v.visualid) break;
    }
  }
#endif
  return 1;
}


int Fl_X11_Screen_Driver::visual(int flags)
{
#if USE_XDBE == 0
  if (flags & FL_DOUBLE) return 0;
#endif
  open_display();
  // always use default if possible:
  if (test_visual(*fl_visual, flags)) return 1;
  // get all the visuals:
  XVisualInfo vTemplate;
  int num;
  XVisualInfo *visualList = XGetVisualInfo(fl_display, 0, &vTemplate, &num);
  // find all matches, use the one with greatest depth:
  XVisualInfo *found = 0;
  for (int i=0; i<num; i++) if (test_visual(visualList[i], flags)) {
    if (!found || found->depth < visualList[i].depth)
      found = &visualList[i];
  }
  if (!found) {XFree((void*)visualList); return 0;}
  fl_visual = found;
  fl_colormap = XCreateColormap(fl_display, RootWindow(fl_display,fl_screen),
                                fl_visual->visual, AllocNone);
  return 1;
}


static int fl_workarea_xywh[4] = { -1, -1, -1, -1 };


void Fl_X11_Screen_Driver::init_workarea() 
{
  open_display();

  Atom actual;
  unsigned long count, remaining;
  int format;
  long *xywh = 0;

  /* If there are several screens, the _NET_WORKAREA property
   does not give the work area of the main screen, but that of all screens together.
   Therefore, we use this property only when there is a single screen,
   and fall back to the main screen full area when there are several screens.
   */
  if (Fl::screen_count() > 1 || XGetWindowProperty(fl_display, RootWindow(fl_display, fl_screen),
                         fl_NET_WORKAREA, 0, 4, False,
                         XA_CARDINAL, &actual, &format, &count, &remaining,
                         (unsigned char **)&xywh) || !xywh || !xywh[2] ||
                         !xywh[3])
  {
    Fl::screen_xywh(fl_workarea_xywh[0],
                    fl_workarea_xywh[1],
                    fl_workarea_xywh[2],
                    fl_workarea_xywh[3], 0);
  }
  else
  {
#if USE_XFT
    float s = screens[0].scale;
#else
    float s = 1;
#endif
    fl_workarea_xywh[0] = xywh[0] / s;
    fl_workarea_xywh[1] = xywh[1] / s;
    fl_workarea_xywh[2] = xywh[2] / s;
    fl_workarea_xywh[3] = xywh[3] / s;
  }
  if ( xywh ) { XFree(xywh); xywh = 0; }
}


int Fl_X11_Screen_Driver::x() {
  if (fl_workarea_xywh[0] < 0) init_workarea();
  return fl_workarea_xywh[0];
}

int Fl_X11_Screen_Driver::y() {
  if (fl_workarea_xywh[0] < 0) init_workarea();
  return fl_workarea_xywh[1];
}

int Fl_X11_Screen_Driver::w() {
  if (fl_workarea_xywh[0] < 0) init_workarea();
  return fl_workarea_xywh[2];
}

int Fl_X11_Screen_Driver::h() {
  if (fl_workarea_xywh[0] < 0) init_workarea();
  return fl_workarea_xywh[3];
}

#define USE_XRANDR (HAVE_DLSYM && HAVE_DLFCN_H) // means attempt to dynamically load libXrandr.so
#if USE_XRANDR
#include <dlfcn.h>
typedef struct {
  int    width, height;
  int    mwidth, mheight;
} XRRScreenSize;
typedef XRRScreenSize* (*XRRSizes_type)(Display *dpy, int screen, int *nsizes);
#endif // USE_XRANDR

void Fl_X11_Screen_Driver::init() {
  if (!fl_display) open_display();

  int dpi_by_randr = 0;
  float dpih = 0.0f, dpiv = 0.0f;

#if USE_XRANDR

  static XRRSizes_type XRRSizes_f = NULL;
  if (!XRRSizes_f) {
    void *libxrandr_addr = dlopen("libXrandr.so.2", RTLD_LAZY);
    if (!libxrandr_addr) libxrandr_addr = dlopen("libXrandr.so", RTLD_LAZY);
#   ifdef __APPLE_CC__ // allows testing on Darwin + X11
    if (!libxrandr_addr) libxrandr_addr = dlopen("/opt/X11/lib/libXrandr.dylib", RTLD_LAZY);
#   endif
    if (libxrandr_addr) XRRSizes_f = (XRRSizes_type)dlsym(libxrandr_addr, "XRRSizes");
  }
  if (XRRSizes_f) {
    int nscreens;
    XRRScreenSize *ssize = XRRSizes_f(fl_display, fl_screen, &nscreens);

    //for (int i=0; i<nscreens; i++)
    //  printf("width=%d height=%d mwidth=%d mheight=%d\n",
    //         ssize[i].width,ssize[i].height,ssize[i].mwidth,ssize[i].mheight);

    if (nscreens > 0) { // Note: XRRSizes() *may* return nscreens == 0, see docs
      int mm = ssize[0].mwidth;
      dpih = mm ? ssize[0].width*25.4f/mm : 0.0f;
      mm = ssize[0].mheight;
      dpiv = mm ? ssize[0].height*25.4f/mm : 0.0f;
      dpi_by_randr = 1;
    }
  }

#endif // USE_XRANDR

#if HAVE_XINERAMA

  if (XineramaIsActive(fl_display)) {
    XineramaScreenInfo *xsi = XineramaQueryScreens(fl_display, &num_screens);
    if (num_screens > MAX_SCREENS) num_screens = MAX_SCREENS;

    /* There's no way to use different DPI for different Xinerama screens. */
    for (int i=0; i<num_screens; i++) {
      screens[i].x_org = xsi[i].x_org;
      screens[i].y_org = xsi[i].y_org;
      screens[i].width = xsi[i].width;
      screens[i].height = xsi[i].height;
#if USE_XFT
      screens[i].scale = 1;
#endif
      if (dpi_by_randr) {
	dpi[i][0] = dpih;
	dpi[i][1] = dpiv;
      } else {
        int mm = DisplayWidthMM(fl_display, fl_screen);
        dpi[i][0] = mm ? screens[i].width*25.4f/mm : 0.0f;
        mm = DisplayHeightMM(fl_display, fl_screen);
        dpi[i][1] = mm ? screens[i].height*25.4f/mm : 0.0f;
      }
    }
    if (xsi) XFree(xsi);
  } else

#endif // HAVE_XINERAMA

  { // ! HAVE_XINERAMA || ! XineramaIsActive()
    num_screens = ScreenCount(fl_display);
    if (num_screens > MAX_SCREENS) num_screens = MAX_SCREENS;

    for (int i=0; i<num_screens; i++) {
      screens[i].x_org = 0;
      screens[i].y_org = 0;
      screens[i].width = DisplayWidth(fl_display, i);
      screens[i].height = DisplayHeight(fl_display, i);
#if USE_XFT
      screens[i].scale = 1;
#endif
      if (dpi_by_randr) {
	dpi[i][0] = dpih;
	dpi[i][1] = dpiv;
      } else {
	int mm = DisplayWidthMM(fl_display, i);
	dpi[i][0] = mm ? screens[i].width*25.4f/mm : 0.0f;
	mm = DisplayHeightMM(fl_display, fl_screen);
	dpi[i][1] = mm ? screens[i].height*25.4f/mm : 0.0f;
      }
    }
  }
  init_workarea();
}


void Fl_X11_Screen_Driver::screen_work_area(int &X, int &Y, int &W, int &H, int n)
{
  if (num_screens < 0) init();
  if (n < 0 || n >= num_screens) n = 0;
  if (n == 0) { // for the main screen, these return the work area
    X = Fl::x();
    Y = Fl::y();
    W = Fl::w();
    H = Fl::h();
  } else { // for other screens, work area is full screen,
    screen_xywh(X, Y, W, H, n);
  }
}


void Fl_X11_Screen_Driver::screen_xywh(int &X, int &Y, int &W, int &H, int n)
{
  if (num_screens < 0) init();

  if ((n < 0) || (n >= num_screens))
    n = 0;

  if (num_screens > 0) {
#if USE_XFT
    float s = screens[n].scale;
#else
    float s = 1;
#endif
    X = screens[n].x_org / s;
    Y = screens[n].y_org / s;
    W = screens[n].width / s;
    H = screens[n].height / s;
  }
}


void Fl_X11_Screen_Driver::screen_dpi(float &h, float &v, int n)
{
  if (num_screens < 0) init();
  h = v = 0.0f;

  if (n >= 0 && n < num_screens) {
    h = dpi[n][0];
    v = dpi[n][1];
  }
}


void Fl_X11_Screen_Driver::beep(int type)
{
  switch (type) {
    case FL_BEEP_DEFAULT :
    case FL_BEEP_ERROR :
      if (!fl_display) open_display();
      XBell(fl_display, 100);
      break;
    default :
      if (!fl_display) open_display();
      XBell(fl_display, 50);
      break;
  }
}


void Fl_X11_Screen_Driver::flush()
{
  if (fl_display)
    XFlush(fl_display);
}


double Fl_X11_Screen_Driver::wait(double time_to_wait)
{
  static char in_idle;

  if (first_timeout) {
    elapse_timeouts();
    Timeout *t;
    while ((t = first_timeout)) {
      if (t->time > 0) break;
      // The first timeout in the array has expired.
      missed_timeout_by = t->time;
      // We must remove timeout from array before doing the callback:
      void (*cb)(void*) = t->cb;
      void *argp = t->arg;
      first_timeout = t->next;
      t->next = free_timeout;
      free_timeout = t;
      // Now it is safe for the callback to do add_timeout:
      cb(argp);
    }
  } else {
    reset_clock = 1; // we are not going to check the clock
  }
  Fl::run_checks();
  if (Fl::idle) {
    if (!in_idle) {
      in_idle = 1;
      Fl::idle();
      in_idle = 0;
    }
    // the idle function may turn off idle, we can then wait:
    if (Fl::idle) time_to_wait = 0.0;
  }
  if (first_timeout && first_timeout->time < time_to_wait)
    time_to_wait = first_timeout->time;
  if (time_to_wait <= 0.0) {
    // do flush second so that the results of events are visible:
    int ret = this->poll_or_select_with_delay(0.0);
    Fl::flush();
    return ret;
  } else {
    // do flush first so that user sees the display:
    Fl::flush();
    if (Fl::idle && !in_idle) // 'idle' may have been set within flush()
      time_to_wait = 0.0;
    return this->poll_or_select_with_delay(time_to_wait);
  }
}


int Fl_X11_Screen_Driver::ready()
{
  if (first_timeout) {
    elapse_timeouts();
    if (first_timeout->time <= 0) return 1;
  } else {
    reset_clock = 1;
  }
  return this->poll_or_select();
}


extern void fl_fix_focus(); // in Fl.cxx


void Fl_X11_Screen_Driver::grab(Fl_Window* win)
{
  Fl_Window *fullscreen_win = NULL;
  for (Fl_Window *W = Fl::first_window(); W; W = Fl::next_window(W)) {
    if (W->fullscreen_active()) {
      fullscreen_win = W;
      break;
    }
  }
  if (win) {
    if (!Fl::grab()) {
      Window xid = fullscreen_win ? fl_xid(fullscreen_win) : fl_xid(Fl::first_window());
      XGrabPointer(fl_display,
                   xid,
                   1,
                   ButtonPressMask|ButtonReleaseMask|
                   ButtonMotionMask|PointerMotionMask,
                   GrabModeAsync,
                   GrabModeAsync,
                   None,
                   0,
                   fl_event_time);
      XGrabKeyboard(fl_display,
                    xid,
                    1,
                    GrabModeAsync,
                    GrabModeAsync,
                    fl_event_time);
    }
    Fl::grab_ = win;	// FIXME: Fl::grab_ "should be private", but we need
			// a way to *set* the variable from the driver!
  } else {
    if (Fl::grab()) {
      // We must keep the grab in the non-EWMH fullscreen case
      if (!fullscreen_win || ewmh_supported()) {
        XUngrabKeyboard(fl_display, fl_event_time);
      }
      XUngrabPointer(fl_display, fl_event_time);
      // this flush is done in case the picked menu item goes into
      // an infinite loop, so we don't leave the X server locked up:
      XFlush(fl_display);
      Fl::grab_ = 0;	// FIXME: Fl::grab_ "should be private", but we need
			// a way to *set* the variable from the driver!
      fl_fix_focus();
    }
  }
}


// Wrapper around XParseColor...
int Fl_X11_Screen_Driver::parse_color(const char* p, uchar& r, uchar& g, uchar& b)
{
  XColor x;
  if (!fl_display) open_display();
  if (XParseColor(fl_display, fl_colormap, p, &x)) {
    r = (uchar)(x.red>>8);
    g = (uchar)(x.green>>8);
    b = (uchar)(x.blue>>8);
    return 1;
  } else return 0;
}


// Read colors that KDE writes to the xrdb database.

// XGetDefault does not do the expected thing: it does not like
// periods in either word. Therefore it cannot match class.Text.background.
// However *.Text.background is matched by pretending the program is "Text".
// But this will also match *.background if there is no *.Text.background
// entry, requiring users to put in both (unless they want the text fields
// the same color as the windows).

static void set_selection_color(uchar r, uchar g, uchar b)
{
  Fl::set_color(FL_SELECTION_COLOR,r,g,b);
}

static void getsyscolor(const char *key1, const char* key2, const char *arg, const char *defarg, void (*func)(uchar,uchar,uchar))
{
  if (!arg) {
    arg = XGetDefault(fl_display, key1, key2);
    if (!arg) arg = defarg;
  }
  XColor x;
  if (!XParseColor(fl_display, fl_colormap, arg, &x))
    Fl::error("Unknown color: %s", arg);
  else
    func(x.red>>8, x.green>>8, x.blue>>8);
}


void Fl_X11_Screen_Driver::get_system_colors()
{
  open_display();
  const char* key1 = 0;
  if (Fl::first_window()) key1 = Fl::first_window()->xclass();
  if (!key1) key1 = "fltk";
  if (!bg2_set)
    getsyscolor("Text","background",	fl_bg2,	"#ffffff", Fl::background2);
  if (!fg_set)
    getsyscolor(key1,  "foreground",	fl_fg,	"#000000", Fl::foreground);
  if (!bg_set)
    getsyscolor(key1,  "background",	fl_bg,	"#c0c0c0", Fl::background);
  getsyscolor("Text", "selectBackground", 0, "#000080", set_selection_color);
}


const char *Fl_X11_Screen_Driver::get_system_scheme()
{
  const char *s = 0L;
  if ((s = fl_getenv("FLTK_SCHEME")) == NULL) {
    const char* key = 0;
    if (Fl::first_window()) key = Fl::first_window()->xclass();
    if (!key) key = "fltk";
    open_display();
    s = XGetDefault(fl_display, key, "scheme");
  }
  return s;
}

// ######################   *FIXME*   ########################
// ######################   *FIXME*   ########################
// ######################   *FIXME*   ########################


//
// X11 timers
//

void Fl_X11_Screen_Driver::add_timeout(double time, Fl_Timeout_Handler cb, void *argp) {
  elapse_timeouts();
  repeat_timeout(time, cb, argp);
}

void Fl_X11_Screen_Driver::repeat_timeout(double time, Fl_Timeout_Handler cb, void *argp) {
  time += missed_timeout_by; if (time < -.05) time = 0;
  Timeout* t = free_timeout;
  if (t) {
      free_timeout = t->next;
  } else {
      t = new Timeout;
  }
  t->time = time;
  t->cb = cb;
  t->arg = argp;
  // insert-sort the new timeout:
  Timeout** p = &first_timeout;
  while (*p && (*p)->time <= time) p = &((*p)->next);
  t->next = *p;
  *p = t;
}

/**
  Returns true if the timeout exists and has not been called yet.
*/
int Fl_X11_Screen_Driver::has_timeout(Fl_Timeout_Handler cb, void *argp) {
  for (Timeout* t = first_timeout; t; t = t->next)
    if (t->cb == cb && t->arg == argp) return 1;
  return 0;
}

/**
  Removes a timeout callback. It is harmless to remove a timeout
  callback that no longer exists.

  \note	This version removes all matching timeouts, not just the first one.
	This may change in the future.
*/
void Fl_X11_Screen_Driver::remove_timeout(Fl_Timeout_Handler cb, void *argp) {
  for (Timeout** p = &first_timeout; *p;) {
    Timeout* t = *p;
    if (t->cb == cb && (t->arg == argp || !argp)) {
      *p = t->next;
      t->next = free_timeout;
      free_timeout = t;
    } else {
      p = &(t->next);
    }
  }
}

int Fl_X11_Screen_Driver::compose(int& del) {
  int condition;
  unsigned char ascii = (unsigned char)Fl::e_text[0];
  condition = (Fl::e_state & (FL_ALT | FL_META | FL_CTRL)) && !(ascii & 128) ;
  if (condition) { del = 0; return 0;} // this stuff is to be treated as a function key
  del = Fl::compose_state;
  Fl::compose_state = 0;
  // Only insert non-control characters:
  if ( (!Fl::compose_state) && ! (ascii & ~31 && ascii!=127)) { return 0; }
  return 1;
}

void Fl_X11_Screen_Driver::compose_reset()
{
  Fl::compose_state = 0;
  if (fl_xim_ic) XmbResetIC(fl_xim_ic);
}

int Fl_X11_Screen_Driver::text_display_can_leak() {
#if USE_XFT
  return 1;
#else
  return 0;
#endif
}


//
// 'fl_subimage_offsets()' - Calculate subimage offsets for an axis
static inline int
fl_subimage_offsets(int a, int aw, int b, int bw, int &obw)
{
  int off;
  int ob;
  
  if (b >= a) {
    ob = b;
    off = 0;
  } else {
    ob = a;
    off = a - b;
  }
  
  bw -= off;
  
  if (ob + bw <= a + aw) {
    obw = bw;
  } else {
    obw = (a + aw) - ob;
  }
  
  return off;
}

// this handler will catch and ignore exceptions during XGetImage
// to avoid an application crash
extern "C" {
  static int xgetimageerrhandler(Display *display, XErrorEvent *error) {
    return 0;
  }
}

Fl_RGB_Image *Fl_X11_Screen_Driver::read_win_rectangle(int X, int Y, int w, int h)
{
  XImage	*image;		// Captured image
  int		i, maxindex;	// Looping vars
  int           x, y;		// Current X & Y in image
  int		d;		// Depth of image
  unsigned char *line,		// Array to hold image row
		*line_ptr;	// Pointer to current line image
  unsigned char	*pixel;		// Current color value
  XColor	colors[4096];	// Colors from the colormap...
  unsigned char	cvals[4096][3];	// Color values from the colormap...
  unsigned	index_mask,
		index_shift,
		red_mask,
		red_shift,
		green_mask,
		green_shift,
		blue_mask,
		blue_shift;
  //
  // Under X11 we have the option of the XGetImage() interface or SGI's
  // ReadDisplay extension which does all of the really hard work for
  // us...
  //
  int allow_outside = w < 0;    // negative w allows negative X or Y, that is, window frame
  if (w < 0) w = - w;
  
#  ifdef __sgi
  if (XReadDisplayQueryExtension(fl_display, &i, &i)) {
    image = XReadDisplay(fl_display, fl_window, X, Y, w, h, 0, NULL);
  } else
#  else
    image = 0;
#  endif // __sgi
  
  float s = Fl_Surface_Device::surface()->driver()->scale();
  int ws = w * s, hs = h * s;
  
  if (!image) {
    // fetch absolute coordinates
    int dx, dy, dxs, dys, sx, sy, sw, sh;
    Window child_win;
    
    Fl_Window *win;
    if (allow_outside) win = Fl_Window::current();
    else win = fl_find(fl_window);
    if (win) {
      XTranslateCoordinates(fl_display, fl_window,
                            RootWindow(fl_display, fl_screen), X*s, Y*s, &dxs, &dys, &child_win);
      dx = dxs/s; dy = dys/s;
      // screen dimensions
      Fl::screen_xywh(sx, sy, sw, sh, win->driver()->screen_num());
    }
    if (win && !allow_outside && int(s) != s) {
      ws = (w+1)*s-1;
      hs = (h+1)*s-1;
      if (int(X*s) + ws >= win->w()*s) ws = win->w()*s - int(X*s) -1;
      if (int(Y*s) + hs >= win->h()*s) hs = win->h()*s - int(Y*s) -1;
     }
    if (ws < 1) ws = 1;
    if (hs < 1) hs = 1;
    if (!win || (dx >= sx && dy >= sy && dxs + ws <= (sx+sw)*s && dys + hs <= (sy+sh)*s) ) {
      // the image is fully contained, we can use the traditional method
      // however, if the window is obscured etc. the function will still fail. Make sure we
      // catch the error and continue, otherwise an exception will be thrown.
      XErrorHandler old_handler = XSetErrorHandler(xgetimageerrhandler);
      image = XGetImage(fl_display, fl_window, int(X*s), int(Y*s), ws, hs, AllPlanes, ZPixmap);
      XSetErrorHandler(old_handler);
    } else {
      // image is crossing borders, determine visible region
      int nw, nh, noffx, noffy;
      noffx = fl_subimage_offsets(sx, sw, dx, w, nw);
      noffy = fl_subimage_offsets(sy, sh, dy, h, nh);
      if (nw <= 0 || nh <= 0) return 0;
      
      // allocate the image
      int bpp = fl_visual->depth + ((fl_visual->depth / 8) % 2) * 8;
      char* buf = (char*)malloc((bpp / 8) * ws * hs);
      image = XCreateImage(fl_display, fl_visual->visual,
                           fl_visual->depth, ZPixmap, 0, buf, ws, hs, bpp, 0);
      if (!image) {
        if (buf) free(buf);
        return 0;
      }
      
      XErrorHandler old_handler = XSetErrorHandler(xgetimageerrhandler);
      XImage *subimg = XGetSubImage(fl_display, fl_window, (X + noffx)*s, (Y + noffy)*s,
                                    nw*s, nh*s, AllPlanes, ZPixmap, image, noffx*s, noffy*s);
      XSetErrorHandler(old_handler);
      if (!subimg) {
        XDestroyImage(image);
        return 0;
      }
    }
  }
  
  if (!image) return 0;
  if (s != 1) {
    w = ws;
    h = hs;
  }
  
#ifdef DEBUG
  printf("width            = %d\n", image->width);
  printf("height           = %d\n", image->height);
  printf("xoffset          = %d\n", image->xoffset);
  printf("format           = %d\n", image->format);
  printf("data             = %p\n", image->data);
  printf("byte_order       = %d\n", image->byte_order);
  printf("bitmap_unit      = %d\n", image->bitmap_unit);
  printf("bitmap_bit_order = %d\n", image->bitmap_bit_order);
  printf("bitmap_pad       = %d\n", image->bitmap_pad);
  printf("depth            = %d\n", image->depth);
  printf("bytes_per_line   = %d\n", image->bytes_per_line);
  printf("bits_per_pixel   = %d\n", image->bits_per_pixel);
  printf("red_mask         = %08x\n", image->red_mask);
  printf("green_mask       = %08x\n", image->green_mask);
  printf("blue_mask        = %08x\n", image->blue_mask);
  printf("map_entries      = %d\n", fl_visual->visual->map_entries);
#endif // DEBUG
  
  d = 3;
  uchar *p = NULL;
  // Allocate the image data array as needed...
  const uchar *oldp = p;
  if (!p) p = new uchar[w * h * d];
  
  // Initialize the default colors/alpha in the whole image...
  memset(p, 0, w * h * d);
  
  // Check that we have valid mask/shift values...
  if (!image->red_mask && image->bits_per_pixel > 12) {
    // Greater than 12 bits must be TrueColor...
    image->red_mask   = fl_visual->visual->red_mask;
    image->green_mask = fl_visual->visual->green_mask;
    image->blue_mask  = fl_visual->visual->blue_mask;
    
#ifdef DEBUG
    // Defined in Fl_Xlib_Graphics_Driver_color.cxx
    extern uchar fl_redmask, fl_greenmask, fl_bluemask;
    extern int fl_redshift, fl_greenshift, fl_blueshift;
    puts("\n---- UPDATED ----");
    printf("fl_redmask       = %08x\n", fl_redmask);
    printf("fl_redshift      = %d\n", fl_redshift);
    printf("fl_greenmask     = %08x\n", fl_greenmask);
    printf("fl_greenshift    = %d\n", fl_greenshift);
    printf("fl_bluemask      = %08x\n", fl_bluemask);
    printf("fl_blueshift     = %d\n", fl_blueshift);
    printf("red_mask         = %08x\n", image->red_mask);
    printf("green_mask       = %08x\n", image->green_mask);
    printf("blue_mask        = %08x\n", image->blue_mask);
#endif // DEBUG
  }
  
  // Check if we have colormap image...
  if (!image->red_mask) {
    // Get the colormap entries for this window...
    maxindex = fl_visual->visual->map_entries;
    
    for (i = 0; i < maxindex; i ++) colors[i].pixel = i;
    
    XQueryColors(fl_display, fl_colormap, colors, maxindex);
    
    for (i = 0; i < maxindex; i ++) {
      cvals[i][0] = colors[i].red >> 8;
      cvals[i][1] = colors[i].green >> 8;
      cvals[i][2] = colors[i].blue >> 8;
    }
    
    // Read the pixels and output an RGB image...
    for (y = 0; y < image->height; y ++) {
      pixel = (unsigned char *)(image->data + y * image->bytes_per_line);
      line  = p + y * w * d;
      
      switch (image->bits_per_pixel) {
        case 1 :
          for (x = image->width, line_ptr = line, index_mask = 128;
               x > 0;
               x --, line_ptr += d) {
            if (*pixel & index_mask) {
              line_ptr[0] = cvals[1][0];
              line_ptr[1] = cvals[1][1];
              line_ptr[2] = cvals[1][2];
            } else {
              line_ptr[0] = cvals[0][0];
              line_ptr[1] = cvals[0][1];
              line_ptr[2] = cvals[0][2];
            }
            
            if (index_mask > 1) {
              index_mask >>= 1;
            } else {
              index_mask = 128;
              pixel ++;
            }
          }
          break;
          
        case 2 :
          for (x = image->width, line_ptr = line, index_shift = 6;
               x > 0;
               x --, line_ptr += d) {
            i = (*pixel >> index_shift) & 3;
            
            line_ptr[0] = cvals[i][0];
            line_ptr[1] = cvals[i][1];
            line_ptr[2] = cvals[i][2];
            
            if (index_shift > 0) {
              index_mask >>= 2;
              index_shift -= 2;
            } else {
              index_mask  = 192;
              index_shift = 6;
              pixel ++;
            }
          }
          break;
          
        case 4 :
          for (x = image->width, line_ptr = line, index_shift = 4;
               x > 0;
               x --, line_ptr += d) {
            if (index_shift == 4) i = (*pixel >> 4) & 15;
            else i = *pixel & 15;
            
            line_ptr[0] = cvals[i][0];
            line_ptr[1] = cvals[i][1];
            line_ptr[2] = cvals[i][2];
            
            if (index_shift > 0) {
              index_shift = 0;
            } else {
              index_shift = 4;
              pixel ++;
            }
          }
          break;
          
        case 8 :
          for (x = image->width, line_ptr = line;
               x > 0;
               x --, line_ptr += d, pixel ++) {
            line_ptr[0] = cvals[*pixel][0];
            line_ptr[1] = cvals[*pixel][1];
            line_ptr[2] = cvals[*pixel][2];
          }
          break;
          
        case 12 :
          for (x = image->width, line_ptr = line, index_shift = 0;
               x > 0;
               x --, line_ptr += d) {
            if (index_shift == 0) {
              i = ((pixel[0] << 4) | (pixel[1] >> 4)) & 4095;
            } else {
              i = ((pixel[1] << 8) | pixel[2]) & 4095;
            }
            
            line_ptr[0] = cvals[i][0];
            line_ptr[1] = cvals[i][1];
            line_ptr[2] = cvals[i][2];
            
            if (index_shift == 0) {
              index_shift = 4;
            } else {
              index_shift = 0;
              pixel += 3;
            }
          }
          break;
      }
    }
  } else {
    // RGB(A) image, so figure out the shifts & masks...
    red_mask  = image->red_mask;
    red_shift = 0;
    
    while ((red_mask & 1) == 0) {
      red_mask >>= 1;
      red_shift ++;
    }
    
    green_mask  = image->green_mask;
    green_shift = 0;
    
    while ((green_mask & 1) == 0) {
      green_mask >>= 1;
      green_shift ++;
    }
    
    blue_mask  = image->blue_mask;
    blue_shift = 0;
    
    while ((blue_mask & 1) == 0) {
      blue_mask >>= 1;
      blue_shift ++;
    }
    
    // Read the pixels and output an RGB image...
    for (y = 0; y < image->height; y ++) {
      pixel = (unsigned char *)(image->data + y * image->bytes_per_line);
      line  = p + y * w * d;
      
      switch (image->bits_per_pixel) {
        case 8 :
          for (x = image->width, line_ptr = line;
               x > 0;
               x --, line_ptr += d, pixel ++) {
            i = *pixel;
            
            line_ptr[0] = 255 * ((i >> red_shift) & red_mask) / red_mask;
            line_ptr[1] = 255 * ((i >> green_shift) & green_mask) / green_mask;
            line_ptr[2] = 255 * ((i >> blue_shift) & blue_mask) / blue_mask;
          }
          break;
          
        case 12 :
          for (x = image->width, line_ptr = line, index_shift = 0;
               x > 0;
               x --, line_ptr += d) {
            if (index_shift == 0) {
              i = ((pixel[0] << 4) | (pixel[1] >> 4)) & 4095;
            } else {
              i = ((pixel[1] << 8) | pixel[2]) & 4095;
            }
            
            line_ptr[0] = 255 * ((i >> red_shift) & red_mask) / red_mask;
            line_ptr[1] = 255 * ((i >> green_shift) & green_mask) / green_mask;
            line_ptr[2] = 255 * ((i >> blue_shift) & blue_mask) / blue_mask;
            
            if (index_shift == 0) {
              index_shift = 4;
            } else {
              index_shift = 0;
              pixel += 3;
            }
          }
          break;
          
        case 16 :
          if (image->byte_order == LSBFirst) {
            // Little-endian...
            for (x = image->width, line_ptr = line;
                 x > 0;
                 x --, line_ptr += d, pixel += 2) {
              i = (pixel[1] << 8) | pixel[0];
              
              line_ptr[0] = 255 * ((i >> red_shift) & red_mask) / red_mask;
              line_ptr[1] = 255 * ((i >> green_shift) & green_mask) / green_mask;
              line_ptr[2] = 255 * ((i >> blue_shift) & blue_mask) / blue_mask;
            }
          } else {
            // Big-endian...
            for (x = image->width, line_ptr = line;
                 x > 0;
                 x --, line_ptr += d, pixel += 2) {
              i = (pixel[0] << 8) | pixel[1];
              
              line_ptr[0] = 255 * ((i >> red_shift) & red_mask) / red_mask;
              line_ptr[1] = 255 * ((i >> green_shift) & green_mask) / green_mask;
              line_ptr[2] = 255 * ((i >> blue_shift) & blue_mask) / blue_mask;
            }
          }
          break;
          
        case 24 :
          if (image->byte_order == LSBFirst) {
            // Little-endian...
            for (x = image->width, line_ptr = line;
                 x > 0;
                 x --, line_ptr += d, pixel += 3) {
              i = (((pixel[2] << 8) | pixel[1]) << 8) | pixel[0];
              
              line_ptr[0] = 255 * ((i >> red_shift) & red_mask) / red_mask;
              line_ptr[1] = 255 * ((i >> green_shift) & green_mask) / green_mask;
              line_ptr[2] = 255 * ((i >> blue_shift) & blue_mask) / blue_mask;
            }
          } else {
            // Big-endian...
            for (x = image->width, line_ptr = line;
                 x > 0;
                 x --, line_ptr += d, pixel += 3) {
              i = (((pixel[0] << 8) | pixel[1]) << 8) | pixel[2];
              
              line_ptr[0] = 255 * ((i >> red_shift) & red_mask) / red_mask;
              line_ptr[1] = 255 * ((i >> green_shift) & green_mask) / green_mask;
              line_ptr[2] = 255 * ((i >> blue_shift) & blue_mask) / blue_mask;
            }
          }
          break;
          
        case 32 :
          if (image->byte_order == LSBFirst) {
            // Little-endian...
            for (x = image->width, line_ptr = line;
                 x > 0;
                 x --, line_ptr += d, pixel += 4) {
              i = (((((pixel[3] << 8) | pixel[2]) << 8) | pixel[1]) << 8) | pixel[0];
              
              line_ptr[0] = 255 * ((i >> red_shift) & red_mask) / red_mask;
              line_ptr[1] = 255 * ((i >> green_shift) & green_mask) / green_mask;
              line_ptr[2] = 255 * ((i >> blue_shift) & blue_mask) / blue_mask;
            }
          } else {
            // Big-endian...
            for (x = image->width, line_ptr = line;
                 x > 0;
                 x --, line_ptr += d, pixel += 4) {
              i = (((((pixel[0] << 8) | pixel[1]) << 8) | pixel[2]) << 8) | pixel[3];
              
              line_ptr[0] = 255 * ((i >> red_shift) & red_mask) / red_mask;
              line_ptr[1] = 255 * ((i >> green_shift) & green_mask) / green_mask;
              line_ptr[2] = 255 * ((i >> blue_shift) & blue_mask) / blue_mask;
            }
          }
          break;
      }
    }
  }
  
  // Destroy the X image we've read and return the RGB(A) image...
  XDestroyImage(image);
  
  Fl_RGB_Image *rgb = new Fl_RGB_Image(p, w, h, d);
  if (!oldp) rgb->alloc_array = 1;
  return rgb;
}


void Fl_X11_Screen_Driver::offscreen_size(Fl_Offscreen off, int &width, int &height)
{
  int px, py;
  unsigned w, h, b, d;
  Window root;
  XGetGeometry(fl_display, off, &root, &px, &py, &w, &h, &b, &d);
  width = (int)w;
  height = (int)h;
}

#if USE_XFT
//NOTICE: returns -1 if x,y is not in any screen
int Fl_X11_Screen_Driver::screen_num_unscaled(int x, int y)
{
  int screen = -1;
  if (num_screens < 0) init();
  
  for (int i = 0; i < num_screens; i ++) {
    int sx = screens[i].x_org, sy = screens[i].y_org, sw = screens[i].width, sh = screens[i].height;
    if ((x >= sx) && (x < (sx+sw)) && (y >= sy) && (y < (sy+sh))) {
      screen = i;
      break;
    }
  }
  return screen;
}

#if HAVE_DLSYM && HAVE_DLFCN_H

// returns true when name is among the list of known names
static bool is_name_in_list(const char *name, const char **list) {
  int i = 0;
  while (list[i]) {
    if (strcmp(list[i++], name) == 0) return true;
  }
  return false;
}


// define types needed for dynamic lib functions
typedef const char** (*g_settings_list_schemas_ftype)(void);
typedef void (*g_variant_get_ftype)(void *value, const char *format_string, ...);
typedef bool (*g_variant_iter_loop_ftype)(void *iter, const char *format_string, ...);
typedef const char **(*g_settings_list_keys_ftype)(void *);
typedef void* (*g_settings_new_ftype)(const char *);
typedef void* (*g_settings_get_value_ftype)(void *settings, const char *key);
typedef void (*pter_ftype)(void*);

// define run-time pointers to functions from dynamic libs
static g_settings_new_ftype g_settings_new_f;
static g_settings_list_keys_ftype g_settings_list_keys_f;
static pter_ftype g_object_unref_f;
static pter_ftype g_strfreev_f;
static g_settings_get_value_ftype g_settings_get_value_f;

static void* value_of_key_in_schema(const char **known, const char *schema, const char *key) {
  void *retval = NULL;
  if (is_name_in_list(schema, known)) {
    void *gset = g_settings_new_f(schema);
    const char **list = g_settings_list_keys_f(gset);
    if (is_name_in_list(key, list)) retval = g_settings_get_value_f(gset, key);
    g_strfreev_f(list);
    g_object_unref_f(gset);
  }
  return retval;
}

/*
 returns true under Ubuntu or Debian or FreeBSD and when the gnome scaling value has been found
 
 Ubuntu:
 Change the gnome scaling factor with:
 System Settings ==> Displays ==> Scale for menu and title bars
 Read the current gnome scaling factor with:
 gsettings get com.ubuntu.user-interface scale-factor
 Example value: {'VGA-0': 10}
 Its type is "a{si}". This value should be divided by 8 to get the correct scaling factor.
 
 Debian or FreeBSD :
 Change the gnome scaling factor with:
 Tweak tools ==> Windows ==> Window scaling
 Read the current gnome scaling factor with:
 gsettings get org.gnome.settings-daemon.plugins.xsettings overrides
 Example value: {'Gdk/WindowScalingFactor': <2>}
 Its type is "a{sv}" and v itself is of type i
 
 It's also possible to use 'Tweak tools' under Ubuntu. With the standard Ubuntu desktop,
 the modified value goes to "org.gnome.settings-daemon.plugins.xsettings" as above.
 
 With Gnome session flashback under Ubuntu  'Tweak tools' puts the scaling value (1 or 2)
 in "org.gnome.desktop.interface scaling-factor".
 Read the current gnome scaling factor with:
 gsettings get org.gnome.desktop.interface scaling-factor
 Its type is "u"
 
 Thus, under Ubuntu, we read the 3 possible factor values and 
 return the first value different from 1 to get the scaling factor.
 
 =================================================================================================
 Ubuntu | default ubuntu desktop | System Settings => Displays => Scale for menu and title bars
                                              com.ubuntu.user-interface scale-factor
                                   -----------------------
                                   Tweak tools => Windows => Window scaling
                                              org.gnome.settings-daemon.plugins.xsettings overrides
                                   -----------------------
          Gnome session flashback | System Settings => Displays => Scale for menu and title bars
                                              no effect
                                   -----------------------
                                   Tweak tools => Windows => Window scaling
                                              org.gnome.desktop.interface scaling-factor
 =================================================================================================
 Debian or FreeBSD | gnome | Tweak tools => Windows => Window scaling
                                            org.gnome.settings-daemon.plugins.xsettings overrides
 =================================================================================================
 */
static bool gnome_scale_factor(float& factor) {
  // open dynamic libs
  void *glib = dlopen("libglib-2.0.so", RTLD_LAZY);
  void *gio = dlopen("libgio-2.0.so", RTLD_LAZY);
  void *gobj = dlopen("libgobject-2.0.so", RTLD_LAZY);
  //fprintf(stderr,"glib=%p gio=%p gobj=%p\n",glib,gio,gobj);
  if (!glib || !gio || !gobj) return false;
  
  // define pters to used functions and variables
  g_settings_list_schemas_ftype g_settings_list_schemas_f = (g_settings_list_schemas_ftype)dlsym(gio, "g_settings_list_schemas"); // 2.26
  g_settings_new_f = (g_settings_new_ftype)dlsym(gio, "g_settings_new"); // 2.26
  g_settings_get_value_f =
		(g_settings_get_value_ftype)dlsym(gio, "g_settings_get_value"); // 2.26
  if (!g_settings_list_schemas_f || !g_settings_new_f || !g_settings_get_value_f) return false;
  g_variant_get_ftype g_variant_get_f = (g_variant_get_ftype)dlsym(glib, "g_variant_get"); //2.24
  g_variant_iter_loop_ftype g_variant_iter_loop_f = (g_variant_iter_loop_ftype)dlsym(glib, "g_variant_iter_loop"); // 2.24
  pter_ftype g_variant_iter_free_f = (pter_ftype)dlsym(glib, "g_variant_iter_free"); // 2.24
  g_object_unref_f = (pter_ftype)dlsym(gobj, "g_object_unref");
  pter_ftype g_variant_unref_f = (pter_ftype)dlsym(glib, "g_variant_unref"); // 2.24
  g_settings_list_keys_f = (g_settings_list_keys_ftype)dlsym(gio, "g_settings_list_keys");
  g_strfreev_f = (pter_ftype)dlsym(glib, "g_strfreev");
  //g_variant_get_type_ftype g_variant_get_type_f = (g_variant_get_type_ftype)dlsym(glib, "g_variant_get_type"); // 2.24
  const unsigned *glib_major_version = (const unsigned *)dlsym(glib, "glib_major_version");
  const unsigned *glib_minor_version = (const unsigned *)dlsym(glib, "glib_minor_version");
  
  // call dynamic lib functions
  if (*glib_major_version * 1000 + *glib_minor_version < 2036) {
    typedef void (*init_ftype)(void);
    init_ftype g_type_init_f = (init_ftype)dlsym(gobj, "g_type_init");
    g_type_init_f(); // necessary only if GLib version < 2.36
  }
  const char **known = g_settings_list_schemas_f(); // list of available GSettings schemas
  float ubuntu_f = 1, ubuntu_desktop_f = 1, gnome_f = 1;
  bool found = false;
  void *gvar;
  
  bool ubuntu = false;
  // determine whether we run Ubuntu
  char line[400] = "";
  FILE *in = fopen("/proc/version", "r");
  if (in) {
    char *s = fgets(line, sizeof(line), in);
    fclose(in);
    if (s && strstr(line, "Ubuntu")) ubuntu = true;
  }
  
  if (ubuntu) {
    gvar = value_of_key_in_schema(known, "com.ubuntu.user-interface", "scale-factor");
    if (gvar) {
      found = true;
      void *iter;
      char str[10], *str2; int v=8, v2;
      g_variant_get_f(gvar, "a{si}", &iter);
      while (g_variant_iter_loop_f(iter, "{si}", &str2, &v2)) { // read the last couple of values
        strcpy(str, str2);  v = v2;
      }
      ubuntu_f = v/8.;
      // printf("com.ubuntu.user-interface  scale-factor name=%s value=%d factor=%g\n", str, v, ubuntu_f);
      g_variant_iter_free_f(iter);
      g_variant_unref_f(gvar);
      if (ubuntu_f != 1) {
        factor = ubuntu_f;
        return true;
      }
    }
    gvar = value_of_key_in_schema(known, "org.gnome.desktop.interface", "scaling-factor");
    if (gvar) {
      found = true;
      unsigned v;
      g_variant_get_f(gvar, "u", &v);
      ubuntu_desktop_f = v;
      // printf("org.gnome.desktop.interface  scaling-factor value=%u factor=%g\n", v, ubuntu_desktop_f);
      g_variant_unref_f(gvar);
      if (ubuntu_desktop_f != 1) {
        factor = ubuntu_desktop_f;
        return true;
      }
    }
  }
  gvar = value_of_key_in_schema(known, "org.gnome.settings-daemon.plugins.xsettings", "overrides");
  if (gvar) {
    void *iter;
    char *str; int v;
    //str = (char*)g_variant_get_type_f(gvar); // -> "a{sv}"
    g_variant_get_f(gvar, "a{sv}", &iter);
    g_variant_unref_f(gvar);
    gvar = NULL;
    while (g_variant_iter_loop_f(iter, "{sv}", &str, &gvar)) {
      if (strstr(str, "WindowScalingFactor") == NULL) continue;
      found = true;
      //str = (char*)g_variant_get_type_f(gvar); // -> "i"
      g_variant_get_f(gvar, "i", &v);
      gnome_f = v;
      // printf("org.gnome.settings-daemon.plugins.xsettings  overrides name=%s value=%d factor=%g\n", str, v, gnome_f);
      free(str);
      break;
    }
    g_variant_iter_free_f(iter);
    if (gvar) g_variant_unref_f(gvar);
  }
  if (!found) return false;
  factor = gnome_f;
  return true;
}
#endif // HAVE_DLSYM && HAVE_DLFCN_H


// return the desktop's default scaling value
float Fl_X11_Screen_Driver::desktop_scale_factor()
{
  float factor = 1;
#if HAVE_DLSYM && HAVE_DLFCN_H
  gnome_scale_factor(factor);
#endif
  return factor;
}


#endif // USE_XFT

//
// End of "$Id$".
//
