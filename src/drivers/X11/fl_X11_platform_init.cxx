//
// X11-specific code to initialize wayland support.
//
// Copyright 2022-2023 by Bill Spitzak and others.
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
#include "../Xlib/Fl_Font.H"


Fl_Copy_Surface_Driver *Fl_Copy_Surface_Driver::newCopySurfaceDriver(int w, int h)
{
  return new Fl_Xlib_Copy_Surface_Driver(w, h);
}


#if !USE_XFT

// WARNING: if you add to this table, you must redefine FL_FREE_FONT
// in Enumerations.H & recompile!!
static Fl_Xlib_Fontdesc built_in_table[] = {
{"-*-helvetica-medium-r-normal--*"},
{"-*-helvetica-bold-r-normal--*"},
{"-*-helvetica-medium-o-normal--*"},
{"-*-helvetica-bold-o-normal--*"},
{"-*-courier-medium-r-normal--*"},
{"-*-courier-bold-r-normal--*"},
{"-*-courier-medium-o-normal--*"},
{"-*-courier-bold-o-normal--*"},
{"-*-times-medium-r-normal--*"},
{"-*-times-bold-r-normal--*"},
{"-*-times-medium-i-normal--*"},
{"-*-times-bold-i-normal--*"},
{"-*-symbol-*"},
{"-*-lucidatypewriter-medium-r-normal-sans-*"},
{"-*-lucidatypewriter-bold-r-normal-sans-*"},
{"-*-*zapf dingbats-*"}
};

#else // USE_XFT

#if ! USE_PANGO

// The predefined fonts that FLTK has:
static Fl_Fontdesc built_in_table[] = {
#if 1
  {" sans"},
  {"Bsans"},
  {"Isans"},
  {"Psans"},
  {" mono"},
  {"Bmono"},
  {"Imono"},
  {"Pmono"},
  {" serif"},
  {"Bserif"},
  {"Iserif"},
  {"Pserif"},
  {" symbol"},
  {" screen"},
  {"Bscreen"},
  {" zapf dingbats"},
#else
  {" helvetica"},
  {"Bhelvetica"},
  {"Ihelvetica"},
  {"Phelvetica"},
  {" courier"},
  {"Bcourier"},
  {"Icourier"},
  {"Pcourier"},
  {" times"},
  {"Btimes"},
  {"Itimes"},
  {"Ptimes"},
  {" symbol"},
  {" lucidatypewriter"},
  {"Blucidatypewriter"},
  {" zapf dingbats"},
#endif
};

#else

// The predefined fonts that FLTK has with Pango:
static Fl_Fontdesc built_in_table[] = {
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
  {"Sans"},
  {"Monospace"},
  {"Monospace Bold"},
  {"Sans"},
};

#endif // USE_PANGO

#endif // USE_XFT

FL_EXPORT Fl_Fontdesc* fl_fonts = (Fl_Fontdesc*)built_in_table;


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
#if USE_XFT
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
