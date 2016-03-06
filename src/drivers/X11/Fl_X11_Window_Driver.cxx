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
#include <FL/fl_draw.H>
#if USE_XDBE
#include <X11/extensions/Xdbe.h>
static int can_xdbe(); // forward

// class to be used only if Xdbe is used
class Fl_X11_Dbe_Window_Driver : public Fl_X11_Window_Driver {
public:
  Fl_X11_Dbe_Window_Driver(Fl_Window *w) : Fl_X11_Window_Driver(w) {}
  virtual int double_flush(int eraseoverlay);
  virtual void destroy_double_buffer();
};
#endif


Fl_Window_Driver *Fl_Window_Driver::newWindowDriver(Fl_Window *w)
{
#if USE_XDBE
  if (can_xdbe()) // strictly necessary only for Fl_Double_Window, but does no harm for Fl_Window
    return new Fl_X11_Dbe_Window_Driver(w);
  else
#endif
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

#if USE_XDBE

static int can_xdbe() { // whether the Xdbe extension is usable
  static int tried;
  static int use_xdbe = 0;
  if (!tried) {
    tried = 1;
    int event_base, error_base;
    if (!XdbeQueryExtension(fl_display, &event_base, &error_base)) return 0;
    Drawable root = RootWindow(fl_display,fl_screen);
    int numscreens = 1;
    XdbeScreenVisualInfo *a = XdbeGetVisualInfo(fl_display,&root,&numscreens);
    if (!a) return 0;
    for (int j = 0; j < a->count; j++) {
      if (a->visinfo[j].visual == fl_visual->visualid) {
        use_xdbe = 1; break;
      }
    }
    XdbeFreeVisualInfo(a);
  }
  return use_xdbe;
}

int Fl_X11_Dbe_Window_Driver::double_flush(int eraseoverlay) {
    if (!other_xid) {
      other_xid = XdbeAllocateBackBufferName(fl_display, xid, XdbeCopied);
      backbuffer_bad = 1;
      w->clear_damage(FL_DAMAGE_ALL);
    }
    if (backbuffer_bad || eraseoverlay) {
      // Make sure we do a complete redraw...
      if (region) {XDestroyRegion(region); region = 0;}
      w->clear_damage(FL_DAMAGE_ALL);
      backbuffer_bad = 0;
    }
    // Redraw as needed...
    if (w->damage()) {
      fl_clip_region(region); region = 0;
      fl_window = other_xid;
      draw();
      fl_window = xid;
    }
    // Copy contents of back buffer to window...
    XdbeSwapInfo s;
    s.swap_window = xid;
    s.swap_action = XdbeCopied;
    XdbeSwapBuffers(fl_display, &s, 1);
    return 1;
}

void Fl_X11_Dbe_Window_Driver::destroy_double_buffer() {
    XdbeDeallocateBackBufferName(fl_display, other_xid);
  other_xid = 0;
}

#endif // USE_XDBE

//
// End of "$Id$".
//
