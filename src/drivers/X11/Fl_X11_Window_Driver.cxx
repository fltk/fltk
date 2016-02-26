//
// "$Id$"
//
// Definition of Apple Cocoa window driver.
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
#include "Fl_X11_Window_Driver.H"


Fl_Window_Driver *Fl_Window_Driver::newWindowDriver(Fl_Window *w)
{
  return new Fl_X11_Window_Driver(w);
}


Fl_X11_Window_Driver::Fl_X11_Window_Driver(Fl_Window *win)
: Fl_Window_Driver(win)
{
}


void Fl_X11_Window_Driver::take_focus()
{
  if (!Fl_X::ewmh_supported())
      w->show(); // Old WMs, XMapRaised
    else if (x) // New WMs use the NETWM attribute:
      Fl_X::activate_window(xid);
}


//
// End of "$Id$".
//
