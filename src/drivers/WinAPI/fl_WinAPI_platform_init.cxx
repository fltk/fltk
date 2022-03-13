//
// Windows-specific code to initialize Windows support.
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


#include "../GDI/Fl_GDI_Copy_Surface_Driver.H"
#include "../GDI/Fl_GDI_Graphics_Driver.H"
#include "Fl_WinAPI_Screen_Driver.H"
#include "Fl_WinAPI_System_Driver.H"
#include "Fl_WinAPI_Window_Driver.H"
#include "../GDI/Fl_GDI_Image_Surface_Driver.H"


Fl_Copy_Surface_Driver *Fl_Copy_Surface_Driver::newCopySurfaceDriver(int w, int h)
{
  return new Fl_GDI_Copy_Surface_Driver(w, h);
}


Fl_Graphics_Driver *Fl_Graphics_Driver::newMainGraphicsDriver()
{
#if USE_GDIPLUS
  // Initialize GDI+.
  static Gdiplus::GdiplusStartupInput gdiplusStartupInput;
  if (Fl_GDIplus_Graphics_Driver::gdiplusToken == 0) {
    GdiplusStartup(&Fl_GDIplus_Graphics_Driver::gdiplusToken, &gdiplusStartupInput, NULL);
  }
  Fl_Graphics_Driver *driver = new Fl_GDIplus_Graphics_Driver();
  return driver;
#else
  return new Fl_GDI_Graphics_Driver();
#endif
}


Fl_Screen_Driver *Fl_Screen_Driver::newScreenDriver()
{
  return new Fl_WinAPI_Screen_Driver();
}


Fl_System_Driver *Fl_System_Driver::newSystemDriver()
{
  return new Fl_WinAPI_System_Driver();
}


Fl_Window_Driver *Fl_Window_Driver::newWindowDriver(Fl_Window *w)
{
  return new Fl_WinAPI_Window_Driver(w);
}


Fl_Image_Surface_Driver *Fl_Image_Surface_Driver::newImageSurfaceDriver(int w, int h, int high_res, Fl_Offscreen off)
{
  return new Fl_GDI_Image_Surface_Driver(w, h, high_res, off);
}
