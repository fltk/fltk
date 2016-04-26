//
// "$Id$"
//
// Definition of SDL Screen interface based on Pico
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
#include "Fl_Pico_Screen_Driver.H"



Fl_Pico_Screen_Driver::Fl_Pico_Screen_Driver()
{
}


Fl_Pico_Screen_Driver::~Fl_Pico_Screen_Driver()
{
}


void Fl_Pico_Screen_Driver::init()
{
  // nothing to do yet
}


int Fl_Pico_Screen_Driver::x()
{
  return 0;
}


int Fl_Pico_Screen_Driver::y()
{
  return 0;
}


int Fl_Pico_Screen_Driver::w()
{
  return 800;
}


int Fl_Pico_Screen_Driver::h()
{
  return 600;
}


void Fl_Pico_Screen_Driver::screen_xywh(int &X, int &Y, int &W, int &H, int n)
{
  X = x();
  Y = y();
  W = w();
  H = h();
}


void Fl_Pico_Screen_Driver::screen_dpi(float &h, float &v, int n)
{
  h = 75.0;
  v = 75.0;
}


void Fl_Pico_Screen_Driver::screen_work_area(int &X, int &Y, int &W, int &H, int n)
{
  X = x();
  Y = y();
  W = w();
  H = h();
}


void Fl_Pico_Screen_Driver::beep(int type)
{
}


void Fl_Pico_Screen_Driver::flush()
{
}


int Fl_Pico_Screen_Driver::ready()
{
  return 1;
}


void Fl_Pico_Screen_Driver::grab(Fl_Window* win)
{
}


int Fl_Pico_Screen_Driver::parse_color(const char* p, uchar& r, uchar& g, uchar& b)
{
  return 0;
}


void Fl_Pico_Screen_Driver::get_system_colors()
{
}


void Fl_Pico_Screen_Driver::add_timeout(double time, Fl_Timeout_Handler cb, void *argp)
{
}


void Fl_Pico_Screen_Driver::repeat_timeout(double time, Fl_Timeout_Handler cb, void *argp)
{
}


int Fl_Pico_Screen_Driver::has_timeout(Fl_Timeout_Handler cb, void *argp)
{
  return 0;
}


void Fl_Pico_Screen_Driver::remove_timeout(Fl_Timeout_Handler cb, void *argp)
{
}


//
// End of "$Id$".
//