//
// "$Id$"
//
// Screen/monitor bounding box API for the Fast Light Tool Kit (FLTK).
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


#include <FL/Fl.H>
#include <FL/x.H>
#include <FL/Fl_Screen_Driver.H>
#include <config.h>

#define MAX_SCREENS 16


#ifdef WIN32
#  if !defined(HMONITOR_DECLARED) && (_WIN32_WINNT < 0x0500)
#    define COMPILE_MULTIMON_STUBS
#    include <multimon.h>
#  endif // !HMONITOR_DECLARED && _WIN32_WINNT < 0x0500

// Number of screens returned by multi monitor aware API; -1 before init
static int num_screens = -1;

#ifndef FL_DOXYGEN
void Fl::call_screen_init() {
  screen_init();
}
#endif

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

static RECT screens[MAX_SCREENS];
static RECT work_area[MAX_SCREENS];
static float dpi[MAX_SCREENS][2];

static BOOL CALLBACK screen_cb(HMONITOR mon, HDC, LPRECT r, LPARAM) {
  if (num_screens >= MAX_SCREENS) return TRUE;

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
      ReleaseDC(0L, screen);
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

#elif defined(FL_PORTING)

#else

#endif // WIN32



void Fl::call_screen_init()
{
  screen_driver()->init();
}


/** Returns the leftmost x coordinate of the main screen work area. */
int Fl::x()
{
  return screen_driver()->x();
}


/** Returns the topmost y coordinate of the main screen work area. */
int Fl::y()
{
  return screen_driver()->y();
}


/** Returns the width in pixels of the main screen work area. */
int Fl::w()
{
  return screen_driver()->w();
}


/** Returns the height in pixels of the main screen work area. */
int Fl::h()
{
  return screen_driver()->h();
}


/**
  Gets the number of available screens.
*/
int Fl::screen_count()
{
  return screen_driver()->screen_count();
}


/**
  Gets the bounding box of a screen
  that contains the specified screen position \p mx, \p my
  \param[out]  X,Y,W,H the corresponding screen bounding box
  \param[in] mx, my the absolute screen position
*/
void Fl::screen_xywh(int &X, int &Y, int &W, int &H, int mx, int my)
{
  screen_driver()->screen_xywh(X, Y, W, H, mx, my);
}


/**
 Gets the bounding box of the work area of a screen
 that contains the specified screen position \p mx, \p my
 \param[out]  X,Y,W,H the work area bounding box
 \param[in] mx, my the absolute screen position
 */
void Fl::screen_work_area(int &X, int &Y, int &W, int &H, int mx, int my)
{
  screen_driver()->screen_work_area(X, Y, W, H, mx, my);
}

/**
 Gets the bounding box of the work area of the given screen.
 \param[out]  X,Y,W,H the work area bounding box
 \param[in] n the screen number (0 to Fl::screen_count() - 1)
 \see void screen_xywh(int &x, int &y, int &w, int &h, int mx, int my)
*/
void Fl::screen_work_area(int &X, int &Y, int &W, int &H, int n)
{
  screen_driver()->screen_work_area(X, Y, W, H, n);
//  if (num_screens < 0) screen_init();
//  if (n < 0 || n >= num_screens) n = 0;
//#ifdef WIN32
//  X = work_area[n].left;
//  Y = work_area[n].top;
//  W = work_area[n].right - X;
//  H = work_area[n].bottom - Y;
//#elif defined(__APPLE__)
//  Fl_X::screen_work_area(X, Y, W, H, n);
//#else
//  if (n == 0) { // for the main screen, these return the work area
//    X = Fl::x();
//    Y = Fl::y();
//    W = Fl::w();
//    H = Fl::h();
//  } else { // for other screens, work area is full screen,
//    screen_xywh(X, Y, W, H, n);
//  }
//#endif
}

/**
  Gets the screen bounding rect for the given screen.
  Under MSWindows, Mac OS X, and the Gnome desktop, screen #0 contains the menubar/taskbar
  \param[out]  X,Y,W,H the corresponding screen bounding box
  \param[in] n the screen number (0 to Fl::screen_count() - 1)
  \see void screen_xywh(int &x, int &y, int &w, int &h, int mx, int my)
*/
void Fl::screen_xywh(int &X, int &Y, int &W, int &H, int n)
{
  screen_driver()->screen_xywh(X, Y, W, H, n);
//  
//  if (num_screens < 0) screen_init();
//
//  if ((n < 0) || (n >= num_screens))
//    n = 0;
//
//#ifdef WIN32
//  if (num_screens > 0) {
//    X = screens[n].left;
//    Y = screens[n].top;
//    W = screens[n].right - screens[n].left;
//    H = screens[n].bottom - screens[n].top;
//  } else {
//    /* Fallback if something is broken... */
//    X = 0;
//    Y = 0;
//    W = GetSystemMetrics(SM_CXSCREEN);
//    H = GetSystemMetrics(SM_CYSCREEN);
//  }
//#elif defined(__APPLE__)
//  X = screens[n].x;
//  Y = screens[n].y;
//  W = screens[n].width;
//  H = screens[n].height;
//#elif defined(FL_PORTING)
//#  pragma message "FL_PORTING: implement screen_xywh"
//  X = 0; Y = 0; W = 800; H = 600;
//#else
//  if (num_screens > 0) {
//    X = screens[n].x_org;
//    Y = screens[n].y_org;
//    W = screens[n].width;
//    H = screens[n].height;
//  }
//#endif // WIN32
}


/**
  Gets the screen bounding rect for the screen
  which intersects the most with the rectangle
  defined by \p mx, \p my, \p mw, \p mh.
  \param[out]  X,Y,W,H the corresponding screen bounding box
  \param[in] mx, my, mw, mh the rectangle to search for intersection with
  \see void screen_xywh(int &X, int &Y, int &W, int &H, int n)
  */
void Fl::screen_xywh(int &X, int &Y, int &W, int &H, int mx, int my, int mw, int mh)
{
  screen_driver()->screen_xywh(X, Y, W, H, mx, my, mw, mh);
}


/**
  Gets the screen number of a screen
  that contains the specified screen position \p x, \p y
  \param[in] x, y the absolute screen position
*/
int Fl::screen_num(int x, int y)
{
  return screen_driver()->screen_num(x, y);
}


/**
  Gets the screen number for the screen
  which intersects the most with the rectangle
  defined by \p x, \p y, \p w, \p h.
  \param[in] x, y, w, h the rectangle to search for intersection with
  */
int Fl::screen_num(int x, int y, int w, int h)
{
  return screen_driver()->screen_num(x, y, w, h);
}


/**
 Gets the screen resolution in dots-per-inch for the given screen.
 \param[out]  h, v  horizontal and vertical resolution
 \param[in]   n     the screen number (0 to Fl::screen_count() - 1)
 \see void screen_xywh(int &x, int &y, int &w, int &h, int mx, int my)
 */
void Fl::screen_dpi(float &h, float &v, int n)
{
  screen_driver()->screen_dpi(h, v, n);
//  if (num_screens < 0) screen_init();
//  h = v = 0.0f;
//
//#ifdef WIN32
//  if (n >= 0 && n < num_screens) {
//    h = float(dpi[n][0]);
//    v = float(dpi[n][1]);
//  }
//#elif defined(__APPLE__)
//  if (n >= 0 && n < num_screens) {
//    h = dpi_h[n];
//    v = dpi_v[n];
//  }
//#elif defined(FL_PORTING)
//#  pragma message "FL_PORTING: implement screen_dpi"
//#else
//  if (n >= 0 && n < num_screens) {
//    h = dpi[n][0];
//    v = dpi[n][1];
//  }
//#endif // WIN32
}


/**
 Gets the bounding box of a screen that contains the mouse pointer.
 \param[out]  X,Y,W,H the corresponding screen bounding box
 \see void screen_xywh(int &x, int &y, int &w, int &h, int mx, int my)
 */
void Fl::screen_xywh(int &X, int &Y, int &W, int &H)
{
  Fl::screen_driver()->screen_xywh(X, Y, W, H);
}


/**
 Gets the bounding box of the work area of the screen that contains the mouse pointer.
 \param[out]  X,Y,W,H the work area bounding box
 \see void screen_work_area(int &x, int &y, int &w, int &h, int mx, int my)
 */
void Fl::screen_work_area(int &X, int &Y, int &W, int &H)
{
  Fl::screen_driver()->screen_xywh(X, Y, W, H);
}



//
// End of "$Id$".
//
