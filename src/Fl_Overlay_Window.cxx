//
// "$Id$"
//
// Overlay window code for the Fast Light Tool Kit (FLTK).
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


// A window using double-buffering and able to draw an overlay
// on top of that.  Uses the hardware to draw the overlay if
// possible, otherwise it just draws in the front buffer.

#include <FL/Fl.H>
#include <FL/Fl_Overlay_Window.H>
#include <FL/Fl_Window_Driver.H>


Fl_Overlay_Window::Fl_Overlay_Window(int W, int H, const char *l)
: Fl_Double_Window(W,H,l)
{
  overlay_ = 0;
  image(0);
}

Fl_Overlay_Window::Fl_Overlay_Window(int X, int Y, int W, int H, const char *l)
: Fl_Double_Window(X,Y,W,H,l)
{
  overlay_ = 0;
  image(0);
}

void Fl_Overlay_Window::show() {
  Fl_Double_Window::show();
  if (overlay_ && overlay_ != this) overlay_->show();
}

void Fl_Overlay_Window::hide() {
  Fl_Double_Window::hide();
}

void Fl_Overlay_Window::flush()
{
  Fl_Window_Driver::driver(this)->flush_overlay();
}

void Fl_Overlay_Window::resize(int X, int Y, int W, int H) {
  Fl_Double_Window::resize(X,Y,W,H);
  if (overlay_ && overlay_!=this) overlay_->resize(0,0,w(),h());
}

/**
  Destroys the window and all child widgets.
*/
Fl_Overlay_Window::~Fl_Overlay_Window() {
  hide();
//  delete overlay; this is done by ~Fl_Group
}

int Fl_Overlay_Window::can_do_overlay() {
  return Fl_Window_Driver::driver(this)->can_do_overlay();
}

/**
 Call this to indicate that the overlay data has changed and needs to
 be redrawn.  The overlay will be clear until the first time this is
 called, so if you want an initial display you must call this after
 calling show().
 */
void Fl_Overlay_Window::redraw_overlay() {
  Fl_Window_Driver::driver(this)->redraw_overlay();
}

//
// End of "$Id$".
//
