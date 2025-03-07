//
// X11-specific code to initialize wayland support.
//
// Copyright 2022-2025 by Bill Spitzak and others.
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
#include <FL/platform.H>
#include "../Xlib/Fl_Xlib_Copy_Surface_Driver.H"
#if FLTK_USE_CAIRO
#  include "../Cairo/Fl_X11_Cairo_Graphics_Driver.H"
#else
#  include "../Xlib/Fl_Xlib_Graphics_Driver.H"
#endif
#include "Fl_X11_Screen_Driver.H"
#include "../Unix/Fl_Unix_System_Driver.H"
#include "Fl_X11_Window_Driver.H"
#include "../Xlib/Fl_Xlib_Image_Surface_Driver.H"


Fl_Copy_Surface_Driver *Fl_Copy_Surface_Driver::newCopySurfaceDriver(int w, int h)
{
  return new Fl_Xlib_Copy_Surface_Driver(w, h);
}


Fl_Graphics_Driver *Fl_Graphics_Driver::newMainGraphicsDriver()
{
#if FLTK_USE_CAIRO
  return new Fl_X11_Cairo_Graphics_Driver();
#else
  return new Fl_Xlib_Graphics_Driver();
#endif
}


Fl_Screen_Driver *Fl_Screen_Driver::newScreenDriver()
{
  Fl_X11_Screen_Driver *d = new Fl_X11_Screen_Driver();
#if USE_XFT || FLTK_USE_CAIRO
  for (int i = 0;  i < MAX_SCREENS; i++) d->screens[i].scale = 1;
  d->current_xft_dpi = 0.; // means the value of the Xft.dpi resource is still unknown
#else
  secret_input_character = '*';
#endif
  return d;
}


Fl_System_Driver *Fl_System_Driver::newSystemDriver()
{
  return new Fl_Unix_System_Driver();
}


Fl_Window_Driver *Fl_Window_Driver::newWindowDriver(Fl_Window *w)
{
  return new Fl_X11_Window_Driver(w);
}


Fl_Image_Surface_Driver *Fl_Image_Surface_Driver::newImageSurfaceDriver(int w, int h, int high_res, Fl_Offscreen off)
{
  return new Fl_Xlib_Image_Surface_Driver(w, h, high_res, off);
}
