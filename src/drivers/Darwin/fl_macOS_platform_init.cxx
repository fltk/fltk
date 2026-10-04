//
// macOS-specific code to initialize macOS support.
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


#include "../Quartz/Fl_Quartz_Copy_Surface_Driver.H"
#include "../Quartz/Fl_Quartz_Graphics_Driver.H"
#include "../Cocoa/Fl_Cocoa_Screen_Driver.H"
#include "../Darwin/Fl_Darwin_System_Driver.H"
#include "../Cocoa/Fl_Cocoa_Window_Driver.H"
#include "../Quartz/Fl_Quartz_Image_Surface_Driver.H"


Fl_Copy_Surface_Driver *Fl_Copy_Surface_Driver::newCopySurfaceDriver(int w, int h)
{
  return new Fl_Quartz_Copy_Surface_Driver(w, h);
}


#include <FL/Fl_Plugin.H>

Fl_Graphics_Driver *Fl_Graphics_Driver::newMainGraphicsDriver()
{
  static Fl_Graphics_Driver_Plugin* best = nullptr;
  if (best)
    return best->create();

  Fl_Plugin_Manager pm(Fl_Graphics_Driver_Plugin::kGrapicsDriverClass);
  for (int i = 0; i < pm.plugins(); i++) {
    Fl_Graphics_Driver_Plugin* p = static_cast<Fl_Graphics_Driver_Plugin*>(pm.plugin(i));
    fprintf(stderr, "Found graphics driver plugin: %s with priority %d\n", pm.name(i), p->priority());
    if (!best || p->priority() > best->priority()) {
      best = p;
    }
  }
  return best ? best->create() : nullptr;
}


Fl_Screen_Driver *Fl_Screen_Driver::newScreenDriver()
{
  return new Fl_Cocoa_Screen_Driver();
}


Fl_System_Driver *Fl_System_Driver::newSystemDriver()
{
  return new Fl_Darwin_System_Driver();
}


Fl_Window_Driver *Fl_Window_Driver::newWindowDriver(Fl_Window *w)
{
  return new Fl_Cocoa_Window_Driver(w);
}


Fl_Image_Surface_Driver *Fl_Image_Surface_Driver::newImageSurfaceDriver(int w, int h, int high_res, Fl_Offscreen off)
{
  return new Fl_Quartz_Image_Surface_Driver(w, h, high_res, off);
}
