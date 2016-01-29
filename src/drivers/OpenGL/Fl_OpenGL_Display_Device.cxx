//
// "$Id$"
//
// implementation of class Fl_Gl_Device_Plugin for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2014 by Bill Spitzak and others.
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

#include <config.h>
#include "../../config_lib.h"
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Device.H>
#include <FL/gl.h>

#include "Fl_OpenGL_Graphics_Driver.h"
#include "Fl_OpenGL_Display_Device.h"

// TODO: much of Fl_Gl_Choice should probably go here

Fl_OpenGL_Display_Device *Fl_OpenGL_Display_Device::display_device() {
  static Fl_OpenGL_Display_Device *display = new Fl_OpenGL_Display_Device(new Fl_OpenGL_Graphics_Driver());
  return display;
};

Fl_OpenGL_Display_Device::Fl_OpenGL_Display_Device(Fl_OpenGL_Graphics_Driver *graphics_driver)
: Fl_Surface_Device(graphics_driver)
{
}

const char *Fl_OpenGL_Display_Device::class_id = "Fl_OpenGL_Display_Device";


//
// End of "$Id$".
//
