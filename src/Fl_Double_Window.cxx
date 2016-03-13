//
// "$Id$"
//
// Double-buffered window code for the Fast Light Tool Kit (FLTK).
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
/** \file
 Fl_Double_Window implementation.
 */

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Printer.H>
#include <FL/x.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Window_Driver.H>

// On systems that support double buffering "naturally" the base
// Fl_Window class will probably do double-buffer and this subclass
// does nothing.


Fl_Double_Window::Fl_Double_Window(int W, int H, const char *l)
: Fl_Window(0, 0, W, H, l)
{
  type(FL_DOUBLE_WINDOW);
  clear_flag(FORCE_POSITION);
}


Fl_Double_Window::Fl_Double_Window(int X, int Y, int W, int H, const char *l)
: Fl_Window(X,Y,W,H,l)
{
  type(FL_DOUBLE_WINDOW);
}


void Fl_Double_Window::show() {
  Fl_Window::show();
}


/*char fl_can_do_alpha_blending() {
  return Fl_Display_Device::display_device()->driver()->can_do_alpha_blending();
}*/


void Fl_Double_Window::resize(int X,int Y,int W,int H) {
  int ow = w();
  int oh = h();
  Fl_Window::resize(X,Y,W,H);
  Fl_X *myi = Fl_X::i(this);
  if (myi && myi->other_xid && (ow < w() || oh < h()))
    driver()->destroy_double_buffer();
}

void Fl_Double_Window::hide() {
  Fl_X *myi = Fl_X::i(this);
  if (myi && myi->other_xid) {
    driver()->destroy_double_buffer();
  }
  Fl_Window::hide();
}


void Fl_Double_Window::flush()
{
  driver()->flush_double();
}


/**
  The destructor <I>also deletes all the children</I>. This allows a
  whole tree to be deleted at once, without having to keep a pointer to
  all the children in the user code.
*/
Fl_Double_Window::~Fl_Double_Window() {
  hide();
}




//
// End of "$Id$".
//
