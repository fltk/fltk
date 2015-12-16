//
// "$Id$"
//
// implementation of Fl_Device class for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2012 by Bill Spitzak and others.
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
#include <FL/Fl_Device.H>
#include <FL/Fl_Image.H>

const char *Fl_Device::class_id = "Fl_Device";
const char *Fl_Surface_Device::class_id = "Fl_Surface_Device";
const char *Fl_Display_Device::class_id = "Fl_Display_Device";
const char *Fl_Graphics_Driver::class_id = "Fl_Graphics_Driver";
#if defined(__APPLE__) || defined(FL_DOXYGEN)
const char *Fl_Quartz_Graphics_Driver::class_id = "Fl_Quartz_Graphics_Driver";
#  ifndef FL_DOXYGEN
   bool Fl_Display_Device::high_res_window_ = false;
#  endif
#endif
#if defined(WIN32) || defined(FL_DOXYGEN)
const char *Fl_GDI_Graphics_Driver::class_id = "Fl_GDI_Graphics_Driver";
const char *Fl_GDI_Printer_Graphics_Driver::class_id = "Fl_GDI_Printer_Graphics_Driver";
#endif
#if !(defined(__APPLE__) || defined(WIN32))
const char *Fl_Xlib_Graphics_Driver::class_id = "Fl_Xlib_Graphics_Driver";
#endif


/** \brief Make this surface the current drawing surface.
 This surface will receive all future graphics requests. */
void Fl_Surface_Device::set_current(void)
{
  fl_graphics_driver = _driver;
  _surface = this;
}

FL_EXPORT Fl_Graphics_Driver *fl_graphics_driver; // the current target device of graphics operations
Fl_Surface_Device* Fl_Surface_Device::_surface; // the current target surface of graphics operations

const Fl_Graphics_Driver::matrix Fl_Graphics_Driver::m0 = {1, 0, 0, 1, 0, 0};

Fl_Graphics_Driver::Fl_Graphics_Driver() {
  font_ = 0;
  size_ = 0;
  sptr=0; rstackptr=0; 
  rstack[0] = NULL;
  fl_clip_state_number=0;
  m = m0; 
  fl_matrix = &m; 
  p = (XPOINT *)0;
  font_descriptor_ = NULL;
  p_size = 0;
};

void Fl_Graphics_Driver::text_extents(const char*t, int n, int& dx, int& dy, int& w, int& h)
{
  w = (int)width(t, n);
  h = - height();
  dx = 0;
  dy = descent();
}

/**  A constructor that sets the graphics driver used by the display */
Fl_Display_Device::Fl_Display_Device(Fl_Graphics_Driver *graphics_driver) : Fl_Surface_Device(graphics_driver) {
  this->set_current();
};


/** Returns the platform display device. */
Fl_Display_Device *Fl_Display_Device::display_device() {
  static Fl_Display_Device *display = new Fl_Display_Device(new
#if defined(__APPLE__)
                                                                  Fl_Quartz_Graphics_Driver
#elif defined(WIN32)
                                                                  Fl_GDI_Graphics_Driver
#else
                                                                  Fl_Xlib_Graphics_Driver
#endif
                                                                 );
  return display;
};


Fl_Surface_Device *Fl_Surface_Device::default_surface()
{
  return Fl_Display_Device::display_device();
}


Fl_Display_Device *Fl_Display_Device::_display = Fl_Display_Device::display_device();

//
// End of "$Id$".
//
