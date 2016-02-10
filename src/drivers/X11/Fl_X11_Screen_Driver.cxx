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


/**
 Creates a driver that manages all screen and display related calls.
 
 This function must be implemented once for every platform. It is called
 when the static members of the class "Fl" are created.
 */
Fl_Screen_Driver *Fl_Screen_Driver::newScreenDriver()
{
  return new Fl_X11_Screen_Driver();
}


void Fl_X11_Screen_Driver::init()
{
  CGDirectDisplayID displays[MAX_SCREENS];
  CGDisplayCount count, i;
  CGRect r;
  CGGetActiveDisplayList(MAX_SCREENS, displays, &count);
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


void Fl_X11_Screen_Driver::screen_work_area(int &X, int &Y, int &W, int &H, int n)
{
  if (num_screens < 0) init();
  if (n < 0 || n >= num_screens) n = 0;
  Fl_X::screen_work_area(X, Y, W, H, n);
}


void Fl_X11_Screen_Driver::screen_xywh(int &X, int &Y, int &W, int &H, int n)
{
  if (num_screens < 0) init();

  if ((n < 0) || (n >= num_screens))
    n = 0;

  X = screens[n].x;
  Y = screens[n].y;
  W = screens[n].width;
  H = screens[n].height;
}


void Fl_X11_Screen_Driver::screen_dpi(float &h, float &v, int n)
{
  if (num_screens < 0) init();
  h = v = 0.0f;

  if (n >= 0 && n < num_screens) {
    h = dpi_h[n];
    v = dpi_v[n];
  }
}



//
// End of "$Id$".
//
