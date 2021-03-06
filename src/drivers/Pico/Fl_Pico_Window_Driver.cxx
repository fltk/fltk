//
// Definition of SDL Window interface based on Pico
//
// Copyright 1998-2018 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//


#include <config.h>
#include "Fl_Pico_Window_Driver.H"

#include <FL/platform.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>


Fl_Pico_Window_Driver::Fl_Pico_Window_Driver(Fl_Window *win)
: Fl_Window_Driver(win)
{
}


Fl_Pico_Window_Driver::~Fl_Pico_Window_Driver()
{
}


int Fl_Pico_Window_Driver::decorated_w()
{
  return w();
}


int Fl_Pico_Window_Driver::decorated_h()
{
  return h();
}
