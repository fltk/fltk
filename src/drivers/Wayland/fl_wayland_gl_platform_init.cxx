//
// Wayland-specific code to initialize wayland support.
//
// Copyright 2022 by Bill Spitzak and others.
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


#include "Fl_Wayland_Gl_Window_Driver.H"
#include "Fl_Wayland_Screen_Driver.H"
#if FLTK_USE_X11
#include "../X11/Fl_X11_Gl_Window_Driver.H"
#endif

Fl_Gl_Window_Driver *Fl_Gl_Window_Driver::newGlWindowDriver(Fl_Gl_Window *w)
{
#if FLTK_USE_X11
  if (Fl_Wayland_Screen_Driver::wl_display) return new Fl_Wayland_Gl_Window_Driver(w);
  return new Fl_X11_Gl_Window_Driver(w);
#else
  return new Fl_Wayland_Gl_Window_Driver(w);
#endif
}
