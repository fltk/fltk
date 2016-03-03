//
// "$Id$"
//
// implementation of Fl_Device class for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2016 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems to:
//
//     http://www.fltk.org/str.php
//

#include <FL/Fl.H>
#include "config_lib.h"
#include <FL/Fl_Device.H>
#include <FL/Fl_Graphics_Driver.H>

/* Attempt at an inheritance diagram.
 
 
  +- Fl_Surface_Device: any kind of surface that we can draw onto -> uses an Fl_Graphics_Driver
      |
      +- Fl_Display_Device: some kind of video device
      +- Fl_Copy_Surface: create an image for dnd or copy/paste
      +- Fl_Image_Surface: create an RGB Image
      +- Fl_Paged_Device: output to a printer or similar
          |
          +- Fl_..._Surface_: platform specific driver
          +- Fl_Printer: user can instantiate this to gain access to a printer
          +- Fl_System_Printer:
          +- Fl_PostScript_File_Device
              |
              +- Fl_PostScript_Printer
 
  +- Fl_Graphics_Driver
      |
      +- Fl_..._Graphics_Driver: platform specific graphics driver

TODO:
  Window Device to handle creation of surfaces and manage events
  System Device to handle file system acces, standard dialogs, etc.

*/

bool Fl_Display_Device::high_res_window_ = false;


/** \brief Make this surface the current drawing surface.
 This surface will receive all future graphics requests. */
void Fl_Surface_Device::set_current(void)
{
  fl_graphics_driver = _driver;
  _surface = this;
  _driver->global_gc();
}

Fl_Surface_Device* Fl_Surface_Device::_surface; // the current target surface of graphics operations


Fl_Surface_Device::~Fl_Surface_Device()
{
}


/**  A constructor that sets the graphics driver used by the display */
Fl_Display_Device::Fl_Display_Device(Fl_Graphics_Driver *graphics_driver) : Fl_Surface_Device(graphics_driver) {
  this->set_current();
};


/** Returns the platform display device. */
Fl_Display_Device *Fl_Display_Device::display_device() {
  static Fl_Display_Device *display = new Fl_Display_Device(Fl_Graphics_Driver::newMainGraphicsDriver());
  return display;
};


Fl_Surface_Device *Fl_Surface_Device::default_surface()
{
  return Fl_Display_Device::display_device();
}


Fl_Display_Device *Fl_Display_Device::_display;


//
// End of "$Id$".
//
