//
// "$Id$"
//
// Screen/monitor bounding box API for the Fast Light Tool Kit (FLTK).
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


#include <FL/Fl.H>
#include <FL/x.H>
#include <config.h>

#define MAX_SCREENS 16

// Number of screens returned by multi monitor aware API; -1 before init
static int num_screens = -1;

#ifdef WIN32
#  if !defined(HMONITOR_DECLARED) && (_WIN32_WINNT < 0x0500)
#    define COMPILE_MULTIMON_STUBS
#    include <multimon.h>
#  endif // !HMONITOR_DECLARED && _WIN32_WINNT < 0x0500

// We go the much more difficult route of individually picking some multi-screen
// functions from the USER32.DLL . If these functions are not available, we
// will gracefully fall back to single monitor support.
//
// If we were to insist on the existence of "EnumDisplayMonitors" and
// "GetMonitorInfoA", it would be impossible to use FLTK on Windows 2000
// before SP2 or earlier.

// BOOL EnumDisplayMonitors(HDC, LPCRECT, MONITORENUMPROC, LPARAM)
typedef BOOL (WINAPI* fl_edm_func)(HDC, LPCRECT, MONITORENUMPROC, LPARAM);
// BOOL GetMonitorInfo(HMONITOR, LPMONITORINFO)
typedef BOOL (WINAPI* fl_gmi_func)(HMONITOR, LPMONITORINFO);

static fl_gmi_func fl_gmi = NULL; // used to get a proc pointer for GetMonitorInfoA

static RECT screens[16];
static RECT work_area[16];
static float dpi[16][2];

static BOOL CALLBACK screen_cb(HMONITOR mon, HDC, LPRECT r, LPARAM) {
  if (num_screens >= 16) return TRUE;

  MONITORINFOEX mi;
  mi.cbSize = sizeof(mi);

//  GetMonitorInfo(mon, &mi);
//  (but we use our self-acquired function pointer instead)
  if (fl_gmi(mon, &mi)) {
    screens[num_screens] = mi.rcMonitor;
// If we also want to record the work area, we would also store mi.rcWork at this point
    work_area[num_screens] = mi.rcWork;
/*fl_alert("screen %d %d,%d,%d,%d work %d,%d,%d,%d",num_screens,
	 screens[num_screens].left,screens[num_screens].right,screens[num_screens].top,screens[num_screens].bottom,
    work_area[num_screens].left,work_area[num_screens].right,work_area[num_screens].top,work_area[num_screens].bottom);
*/
    // find the pixel size
    if (mi.cbSize == sizeof(mi)) {
      HDC screen = CreateDC(mi.szDevice, NULL, NULL, NULL);
      if (screen) {
        dpi[num_screens][0] = (float)GetDeviceCaps(screen, LOGPIXELSX);
        dpi[num_screens][1] = (float)GetDeviceCaps(screen, LOGPIXELSY);
      }
      DeleteDC(screen);
    }

    num_screens ++;
  }
  return TRUE;
}

static void screen_init() {
  // Since not all versions of Windows include multiple monitor support,
  // we do a run-time check for the required functions...
  HMODULE hMod = GetModuleHandle("USER32.DLL");

  if (hMod) {
    // check that EnumDisplayMonitors is available
    fl_edm_func fl_edm = (fl_edm_func)GetProcAddress(hMod, "EnumDisplayMonitors");

    if (fl_edm) {
      // we have EnumDisplayMonitors - do we also have GetMonitorInfoA ?
      fl_gmi = (fl_gmi_func)GetProcAddress(hMod, "GetMonitorInfoA");
      if (fl_gmi) {
        // We have GetMonitorInfoA, enumerate all the screens...
//      EnumDisplayMonitors(0,0,screen_cb,0);
//      (but we use our self-acquired function pointer instead)
//      NOTE: num_screens is incremented in screen_cb so we must first reset it here...
        num_screens = 0;
        fl_edm(0, 0, screen_cb, 0);
        return;
      }
    }
  }

  // If we get here, assume we have 1 monitor...
  num_screens = 1;
  screens[0].top      = 0;
  screens[0].left      = 0;
  screens[0].right  = GetSystemMetrics(SM_CXSCREEN);
  screens[0].bottom = GetSystemMetrics(SM_CYSCREEN);
  work_area[0] = screens[0];
}
#elif defined(__APPLE__)
static XRectangle screens[16];
static float dpi_h[16];
static float dpi_v[16];

static void screen_init() {
  CGDirectDisplayID displays[16];
  CGDisplayCount count, i;
  CGRect r;
  CGGetActiveDisplayList(16, displays, &count);
  for( i = 0; i < count; i++) {
    r = CGDisplayBounds(displays[i]);
    screens[i].x      = int(r.origin.x);
    screens[i].y      = int(r.origin.y);
    screens[i].width  = int(r.size.width);
    screens[i].height = int(r.size.height);
//fprintf(stderr,"screen %d %dx%dx%dx%d\n",i,screens[i].x,screens[i].y,screens[i].width,screens[i].height);
    if (&CGDisplayScreenSize != NULL) {
      CGSize s = CGDisplayScreenSize(displays[i]); // from 10.3
      dpi_h[i] = screens[i].width / (s.width/25.4);
      dpi_v[i] = screens[i].height / (s.height/25.4);
    } else {
      dpi_h[i] = dpi_v[i] = 75.;
    }
  }
  num_screens = count;
}

#else // X11

#if HAVE_XINERAMA
#  include <X11/extensions/Xinerama.h>
#endif
typedef struct {
  short x_org;
  short y_org;
  short width;
  short height;
} FLScreenInfo;
static FLScreenInfo screens[MAX_SCREENS];
static float dpi[MAX_SCREENS][2];

#define USE_XRANDR (HAVE_DLSYM && HAVE_DLFCN_H) // means attempt to dynamically load libXrandr.so
#if USE_XRANDR
#include <dlfcn.h>
typedef struct {
  int    width, height;
  int    mwidth, mheight;
} XRRScreenSize;
typedef XRRScreenSize* (*XRRSizes_type)(Display *dpy, int screen, int *nsizes);
#endif // USE_XRANDR

static void screen_init() {
  if (!fl_display) fl_open_display();

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
}

#endif // ( WIN32 || __APPLE__ || ) X11

#ifndef FL_DOXYGEN
void Fl::call_screen_init() {
  screen_init();
}
#endif

/**
  Gets the number of available screens.
*/
int Fl::screen_count() {
  if (num_screens < 0) screen_init();

  return num_screens ? num_screens : 1;
}

/**
  Gets the bounding box of a screen
  that contains the specified screen position \p mx, \p my
  \param[out]  X,Y,W,H the corresponding screen bounding box
  \param[in] mx, my the absolute screen position
*/
void Fl::screen_xywh(int &X, int &Y, int &W, int &H, int mx, int my) {
  screen_xywh(X, Y, W, H, screen_num(mx, my));
}


/**
 Gets the bounding box of the work area of a screen
 that contains the specified screen position \p mx, \p my
 \param[out]  X,Y,W,H the work area bounding box
 \param[in] mx, my the absolute screen position
 */
void Fl::screen_work_area(int &X, int &Y, int &W, int &H, int mx, int my) {
  screen_work_area(X, Y, W, H, screen_num(mx, my));
}

/**
 Gets the bounding box of the work area of the given screen.
 \param[out]  X,Y,W,H the work area bounding box
 \param[in] n the screen number (0 to Fl::screen_count() - 1)
 \see void screen_xywh(int &x, int &y, int &w, int &h, int mx, int my)
*/
void Fl::screen_work_area(int &X, int &Y, int &W, int &H, int n) {
  if (num_screens < 0) screen_init();
  if (n < 0 || n >= num_screens) n = 0;
#ifdef WIN32
  X = work_area[n].left;
  Y = work_area[n].top;
  W = work_area[n].right - X;
  H = work_area[n].bottom - Y;
#elif defined(__APPLE__)
  Fl_X::screen_work_area(X, Y, W, H, n);
#else
  if (n == 0) { // for the main screen, these return the work area
    X = Fl::x();
    Y = Fl::y();
    W = Fl::w();
    H = Fl::h();
  } else { // for other screens, work area is full screen,
    screen_xywh(X, Y, W, H, n);
  }
#endif
}

/**
  Gets the screen bounding rect for the given screen.
  Under MSWindows, Mac OS X, and the Gnome desktop, screen #0 contains the menubar/taskbar
  \param[out]  X,Y,W,H the corresponding screen bounding box
  \param[in] n the screen number (0 to Fl::screen_count() - 1)
  \see void screen_xywh(int &x, int &y, int &w, int &h, int mx, int my)
*/
void Fl::screen_xywh(int &X, int &Y, int &W, int &H, int n) {
  if (num_screens < 0) screen_init();

  if ((n < 0) || (n >= num_screens))
    n = 0;

#ifdef WIN32
  if (num_screens > 0) {
    X = screens[n].left;
    Y = screens[n].top;
    W = screens[n].right - screens[n].left;
    H = screens[n].bottom - screens[n].top;
  } else {
    /* Fallback if something is broken... */
    X = 0;
    Y = 0;
    W = GetSystemMetrics(SM_CXSCREEN);
    H = GetSystemMetrics(SM_CYSCREEN);
  }
#elif defined(__APPLE__)
  X = screens[n].x;
  Y = screens[n].y;
  W = screens[n].width;
  H = screens[n].height;
#else
  if (num_screens > 0) {
    X = screens[n].x_org;
    Y = screens[n].y_org;
    W = screens[n].width;
    H = screens[n].height;
  }
#endif // WIN32
}

/**
  Gets the screen bounding rect for the screen
  which intersects the most with the rectangle
  defined by \p mx, \p my, \p mw, \p mh.
  \param[out]  X,Y,W,H the corresponding screen bounding box
  \param[in] mx, my, mw, mh the rectangle to search for intersection with
  \see void screen_xywh(int &X, int &Y, int &W, int &H, int n)
  */
void Fl::screen_xywh(int &X, int &Y, int &W, int &H, int mx, int my, int mw, int mh) {
  screen_xywh(X, Y, W, H, screen_num(mx, my, mw, mh));
}

/**
  Gets the screen number of a screen
  that contains the specified screen position \p x, \p y
  \param[in] x, y the absolute screen position
*/
int Fl::screen_num(int x, int y) {
  int screen = 0;
  if (num_screens < 0) screen_init();
  
  for (int i = 0; i < num_screens; i ++) {
    int sx = 0, sy = 0, sw = 0, sh = 0;
    Fl::screen_xywh(sx, sy, sw, sh, i);
    if ((x >= sx) && (x < (sx+sw)) && (y >= sy) && (y < (sy+sh))) {
      screen = i;
      break;
    }
  }
  return screen;
}

// Return the number of pixels common to the two rectangular areas
static inline float fl_intersection(int x1, int y1, int w1, int h1,
                                    int x2, int y2, int w2, int h2) {
  if(x1+w1 < x2 || x2+w2 < x1 || y1+h1 < y2 || y2+h2 < y1)
    return 0.;
  int int_left = x1 > x2 ? x1 : x2;
  int int_right = x1+w1 > x2+w2 ? x2+w2 : x1+w1;
  int int_top = y1 > y2 ? y1 : y2;
  int int_bottom = y1+h1 > y2+h2 ? y2+h2 : y1+h1;
  return (float)(int_right - int_left) * (int_bottom - int_top);
}

/**
  Gets the screen number for the screen
  which intersects the most with the rectangle
  defined by \p x, \p y, \p w, \p h.
  \param[in] x, y, w, h the rectangle to search for intersection with
  */
int Fl::screen_num(int x, int y, int w, int h) {
  int best_screen = 0;
  float best_intersection = 0.;
  for (int i = 0; i < Fl::screen_count(); i++) {
    int sx = 0, sy = 0, sw = 0, sh = 0;
    Fl::screen_xywh(sx, sy, sw, sh, i);
    float sintersection = fl_intersection(x, y, w, h, sx, sy, sw, sh);
    if (sintersection > best_intersection) {
      best_screen = i;
      best_intersection = sintersection;
    }
  }
  return best_screen;
}

/**
 Gets the screen resolution in dots-per-inch for the given screen.
 \param[out]  h, v  horizontal and vertical resolution
 \param[in]   n     the screen number (0 to Fl::screen_count() - 1)
 \see void screen_xywh(int &x, int &y, int &w, int &h, int mx, int my)
 */
void Fl::screen_dpi(float &h, float &v, int n)
{
  if (num_screens < 0) screen_init();
  h = v = 0.0f;

#ifdef WIN32
  if (n >= 0 && n < num_screens) {
    h = float(dpi[n][0]);
    v = float(dpi[n][1]);
  }
#elif defined(__APPLE__)
  if (n >= 0 && n < num_screens) {
    h = dpi_h[n];
    v = dpi_v[n];
  }
#else
  if (n >= 0 && n < num_screens) {
    h = dpi[n][0];
    v = dpi[n][1];
  }
#endif // WIN32
}



//
// End of "$Id$".
//
