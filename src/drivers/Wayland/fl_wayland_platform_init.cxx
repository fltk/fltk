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


#include "Fl_Wayland_Copy_Surface_Driver.H"
#include "Fl_Wayland_Graphics_Driver.H"
#include "Fl_Wayland_Screen_Driver.H"
#include "Fl_Wayland_System_Driver.H"
#include "Fl_Wayland_Window_Driver.H"
#include "Fl_Wayland_Image_Surface_Driver.H"


Fl_Copy_Surface_Driver *Fl_Copy_Surface_Driver::newCopySurfaceDriver(int w, int h)
{
  return new Fl_Wayland_Copy_Surface_Driver(w, h);
}


static Fl_Fontdesc built_in_table[] = {  // Pango font names
  {"Sans"},
  {"Sans Bold"},
  {"Sans Italic"},
  {"Sans Bold Italic"},
  {"Monospace"},
  {"Monospace Bold"},
  {"Monospace Italic"},
  {"Monospace Bold Italic"},
  {"Serif"},
  {"Serif Bold"},
  {"Serif Italic"},
  {"Serif Bold Italic"},
  {"Standard Symbols PS"}, // FL_SYMBOL
  {"Monospace"},           // FL_SCREEN
  {"Monospace Bold"},      // FL_SCREEN_BOLD
  {"D050000L"},            // FL_ZAPF_DINGBATS
};


FL_EXPORT Fl_Fontdesc *fl_fonts = built_in_table;


Fl_Graphics_Driver *Fl_Graphics_Driver::newMainGraphicsDriver()
{
  fl_graphics_driver = new Fl_Wayland_Graphics_Driver();
  return fl_graphics_driver;
}


Fl_Screen_Driver *Fl_Screen_Driver::newScreenDriver()
{
  return new Fl_Wayland_Screen_Driver();
}


Fl_System_Driver *Fl_System_Driver::newSystemDriver()
{
  return new Fl_Wayland_System_Driver();
}


Fl_Window_Driver *Fl_Window_Driver::newWindowDriver(Fl_Window *w)
{
  return new Fl_Wayland_Window_Driver(w);
}


Fl_Image_Surface_Driver *Fl_Image_Surface_Driver::newImageSurfaceDriver(int w, int h, int high_res, Fl_Offscreen off)
{
  return new Fl_Wayland_Image_Surface_Driver(w, h, high_res, off);
}
