//
// "$Id$"
//
// Definition of X11 Screen interface
//
// Copyright 1998-2016 by Bill Spitzak and others.
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
#include "Fl_X11_Screen_Driver.h"
#include <FL/Fl.H>
#include <FL/x.H>
#include <FL/fl_ask.H>

#include <sys/time.h>

#if HAVE_XINERAMA
#  include <X11/extensions/Xinerama.h>
#endif

#if USE_XDBE
#include <X11/extensions/Xdbe.h>
#endif

extern Atom fl_NET_WORKAREA;

// Add these externs to allow X11 port to build - same as Fl_WinAPI_Screen_Driver.cxx.
// These should be in an internal header somewhere?
// AlbrechtS (Comment by Ian, modified...)
extern int fl_ready(); // in Fl_x.cxx
extern int fl_wait(double time); // in Fl_x.cxx

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
 
 This function must be implemented once for every platform. It is called
 when the static members of the class "Fl" are created.
 */
Fl_Screen_Driver *Fl_Screen_Driver::newScreenDriver()
{
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
  fl_open_display();
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
  fl_open_display();

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
    fl_workarea_xywh[0] = (int)xywh[0];
    fl_workarea_xywh[1] = (int)xywh[1];
    fl_workarea_xywh[2] = (int)xywh[2];
    fl_workarea_xywh[3] = (int)xywh[3];
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


void Fl_X11_Screen_Driver::init()
{
  if (!fl_display) fl_open_display();
  // FIXME: Rewrite using RandR instead
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

      int mm = DisplayWidthMM(fl_display, fl_screen);
      dpi[i][0] = mm ? screens[i].width*25.4f/mm : 0.0f;
      mm = DisplayHeightMM(fl_display, fl_screen);
      dpi[i][1] = mm ? screens[i].height*25.4f/mm : 0.0f;
    }
    if (xsi) XFree(xsi);
  } else
#endif
  { // ! XineramaIsActive()
    num_screens = ScreenCount(fl_display);
    if (num_screens > MAX_SCREENS) num_screens = MAX_SCREENS;
   
    for (int i=0; i<num_screens; i++) {
      screens[i].x_org = 0;
      screens[i].y_org = 0;
      screens[i].width = DisplayWidth(fl_display, i);
      screens[i].height = DisplayHeight(fl_display, i);
 
      int mm = DisplayWidthMM(fl_display, i);
      dpi[i][0] = mm ? DisplayWidth(fl_display, i)*25.4f/mm : 0.0f;
      mm = DisplayHeightMM(fl_display, i);
      dpi[i][1] = mm ? DisplayHeight(fl_display, i)*25.4f/mm : 0.0f;
    }
  }
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
    X = screens[n].x_org;
    Y = screens[n].y_org;
    W = screens[n].width;
    H = screens[n].height;
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
      if (!fl_display) fl_open_display();
      XBell(fl_display, 100);
      break;
    default :
      if (!fl_display) fl_open_display();
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
  //  if (idle && !fl_ready()) {
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
    int ret = fl_wait(0.0);
    flush();
    return ret;
  } else {
    // do flush first so that user sees the display:
    Fl::flush();
    if (Fl::idle && !in_idle) // 'idle' may have been set within flush()
      time_to_wait = 0.0;
    return fl_wait(time_to_wait);
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
  return fl_ready();
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
    Fl::grab(win);
  } else {
    if (Fl::grab()) {
      // We must keep the grab in the non-EWMH fullscreen case
      if (!fullscreen_win || Fl_X::ewmh_supported()) {
        XUngrabKeyboard(fl_display, fl_event_time);
      }
      XUngrabPointer(fl_display, fl_event_time);
      // this flush is done in case the picked menu item goes into
      // an infinite loop, so we don't leave the X server locked up:
      XFlush(fl_display);
      Fl::grab(0);
      fl_fix_focus();
    }
  }
}


// Wrapper around XParseColor...
int Fl_X11_Screen_Driver::parse_color(const char* p, uchar& r, uchar& g, uchar& b)
{
  XColor x;
  if (!fl_display) fl_open_display();
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
  fl_open_display();
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
  if ((s = getenv("FLTK_SCHEME")) == NULL) {
    const char* key = 0;
    if (Fl::first_window()) key = Fl::first_window()->xclass();
    if (!key) key = "fltk";
    fl_open_display();
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

void Fl::add_timeout(double time, Fl_Timeout_Handler cb, void *argp) {
  elapse_timeouts();
  repeat_timeout(time, cb, argp);
}

void Fl::repeat_timeout(double time, Fl_Timeout_Handler cb, void *argp) {
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
int Fl::has_timeout(Fl_Timeout_Handler cb, void *argp) {
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
void Fl::remove_timeout(Fl_Timeout_Handler cb, void *argp) {
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



//
// End of "$Id$".
//
