//
// "$Id: Fl_Pico_Window_Driver.cxx 11253 2016-03-01 00:54:21Z matt $"
//
// Definition of SDL Window interface based on Pico
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
#include "Fl_Pico_Window_Driver.H"

#include <FL/x.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.h>



Fl_Pico_Window_Driver::Fl_Pico_Window_Driver(Fl_Window *win)
: Fl_Window_Driver(win)
{
}


Fl_Pico_Window_Driver::~Fl_Pico_Window_Driver()
{
}


// --- window data

int Fl_Pico_Window_Driver::decorated_w()
{
  return pWindow->w();
}


int Fl_Pico_Window_Driver::decorated_h()
{
  return pWindow->h();
}


// --- window management
void Fl_Pico_Window_Driver::flush_single()
{
  Fl_X *i = Fl_X::i(pWindow);
  if (!i) return;
  fl_clip_region(i->region);
  i->region = 0;
  pWindow->draw();
}


void Fl_Pico_Window_Driver::flush_double()
{
  flush_single();
}


void Fl_Pico_Window_Driver::flush_overlay()
{
  flush_single();
}




void Fl_Pico_Window_Driver::draw_begin()
{
  // nothing to do
}


void Fl_Pico_Window_Driver::draw_end()
{
  // nothing to do
}


//
// End of "$Id: Fl_Pico_Window_Driver.cxx 11253 2016-03-01 00:54:21Z matt $".
//