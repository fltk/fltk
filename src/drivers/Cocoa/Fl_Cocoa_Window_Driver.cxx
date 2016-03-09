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
#include "Fl_Cocoa_Window_Driver.h"
#include <FL/Fl_Double_Window.H>
#include <FL/fl_draw.H>

// class used for Fl_Double_Window but not for Fl_Overlay_Window
class Fl_Cocoa_Double_Window_Driver : public Fl_Cocoa_Window_Driver {
public:
  Fl_Cocoa_Double_Window_Driver(Fl_Window *w) : Fl_Cocoa_Window_Driver(w) {}
  int double_flush(int eraseoverlay) {
    draw();
    return 0;
  }
};

Fl_Window_Driver *Fl_Window_Driver::newWindowDriver(Fl_Window *w)
{
  if (w->as_double_window() && !w->as_double_window()->as_overlay_window())
    return new Fl_Cocoa_Double_Window_Driver(w);
  else
    return new Fl_Cocoa_Window_Driver(w);
}


Fl_Cocoa_Window_Driver::Fl_Cocoa_Window_Driver(Fl_Window *win)
: Fl_Window_Driver(win)
{
}


extern Fl_Window *fl_xfocus;


void Fl_Cocoa_Window_Driver::take_focus()
{
  Fl_X *x = Fl_X::i(pWindow);
  if (x) x->set_key_window();
}

//
// End of "$Id$".
//
