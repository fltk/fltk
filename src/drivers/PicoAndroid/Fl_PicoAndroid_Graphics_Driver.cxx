//
// "$Id: Fl_PicoAndroid_Graphics_Driver.cxx 11241 2016-02-27 13:52:27Z manolo $"
//
// Rectangle drawing routines for the Fast Light Tool Kit (FLTK).
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
#include "Fl_PicoAndroid_Graphics_Driver.h"

#include <FL/Fl.H>
//#define __APPLE__
//#include <SDL2/SDL.h>
//#undef __APPLE__

/*
 * By linking this module, the following static method will instatiate the
 * PicoSDL Graphics driver as the main display driver.
 */
Fl_Graphics_Driver *Fl_Graphics_Driver::newMainGraphicsDriver()
{
  return new Fl_PicoAndroid_Graphics_Driver();
}


void Fl_PicoAndroid_Graphics_Driver::rectf(int x, int y, int w, int h)
{
//  uchar r, g, b;
//  Fl::get_color(Fl_Graphics_Driver::color(), r, g, b);
//  SDL_SetRenderDrawColor((SDL_Renderer*)fl_window, r, g, b, SDL_ALPHA_OPAQUE);
//  SDL_Rect rect = {x, y, w, h};
//  SDL_RenderFillRect((SDL_Renderer*)fl_window, &rect);
}


void Fl_PicoAndroid_Graphics_Driver::line(int x, int y, int x1, int y1)
{
//  uchar r, g, b;
//  Fl::get_color(Fl_Graphics_Driver::color(), r, g, b);
//  SDL_SetRenderDrawColor((SDL_Renderer*)fl_window, r, g, b, SDL_ALPHA_OPAQUE);
//  SDL_RenderDrawLine((SDL_Renderer*)fl_window, x, y, x1, y1);
}


void Fl_PicoAndroid_Graphics_Driver::point(int x, int y)
{
//  uchar r, g, b;
//  Fl::get_color(Fl_Graphics_Driver::color(), r, g, b);
//  SDL_SetRenderDrawColor((SDL_Renderer*)fl_window, r, g, b, SDL_ALPHA_OPAQUE);
//  SDL_RenderDrawPoint((SDL_Renderer*)fl_window, x, y);
}



//
// End of "$Id: Fl_PicoAndroid_Graphics_Driver.cxx 11241 2016-02-27 13:52:27Z manolo $".
//
