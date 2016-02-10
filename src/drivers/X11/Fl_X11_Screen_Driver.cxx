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
#include <FL/fl_ask.h>

#if HAVE_XINERAMA
#  include <X11/extensions/Xinerama.h>
#endif

extern Atom fl_NET_WORKAREA;


/**
 Creates a driver that manages all screen and display related calls.
 
 This function must be implemented once for every platform. It is called
 when the static members of the class "Fl" are created.
 */
Fl_Screen_Driver *Fl_Screen_Driver::newScreenDriver()
{
  return new Fl_X11_Screen_Driver();
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


void Fl_X11_Screen_Driver::beep(int type) {
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



//
// End of "$Id$".
//
