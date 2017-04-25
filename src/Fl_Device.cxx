//
// "$Id$"
//
// implementation of Fl_Device class for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2017 by Bill Spitzak and others.
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

*/

/** Make this surface the current drawing surface.
 This surface will receive all future graphics requests. 
 \p Starting from FLTK 1.4.0, another convenient API to set/unset the current drawing surface
 is Fl_Surface_Device::push_current( ) / Fl_Surface_Device::pop_current().*/
void Fl_Surface_Device::set_current(void)
{
  if (surface_) surface_->end_current_(this);
  fl_graphics_driver = pGraphicsDriver;
  surface_ = this;
  pGraphicsDriver->global_gc();
}

Fl_Surface_Device* Fl_Surface_Device::surface_; // the current target surface of graphics operations


Fl_Surface_Device::~Fl_Surface_Device()
{
  if (surface_ == this) surface_ = NULL;
}


/**  A constructor that sets the graphics driver used by the display */
Fl_Display_Device::Fl_Display_Device(Fl_Graphics_Driver *graphics_driver) : Fl_Surface_Device(graphics_driver) {
  this->set_current();
};


/** Returns a pointer to the unique display device */
Fl_Display_Device *Fl_Display_Device::display_device() {
  static Fl_Display_Device *display = new Fl_Display_Device(Fl_Graphics_Driver::newMainGraphicsDriver());
  return display;
};


Fl_Surface_Device *Fl_Surface_Device::default_surface()
{
  return Fl_Display_Device::display_device();
}

static unsigned int surface_stack_height = 0;
static Fl_Surface_Device *surface_stack[16];

/** Pushes \p new_current on top of the stack of current drawing surfaces, and makes it current.
 \p new_current will receive all future graphics requests.
 \version 1.4.0 */
void Fl_Surface_Device::push_current(Fl_Surface_Device *new_current)
{
  if (surface_stack_height < sizeof(surface_stack)/sizeof(void*)) {
    surface_stack[surface_stack_height++] = surface();
  } else {
    fprintf(stderr, "FLTK Fl_Surface_Device::push_current Stack overflow error\n");
  }
  new_current->set_current();
}

/** Removes the top element from the current drawing surface stack, and makes the new top element current.
 \return A pointer to the new current drawing surface. 
 \version 1.4.0 */
Fl_Surface_Device *Fl_Surface_Device::pop_current()
{
  if (surface_stack_height > 0) surface_stack[--surface_stack_height]->set_current();
  return surface_;
}

//
// End of "$Id$".
//
