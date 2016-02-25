//
// "$Id$"
//
// implementation of Fl_Display_Device class for the Fast Light Tool Kit (FLTK).
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

// FIXME: implement this
#if 0 

/**  A constructor that sets the graphics driver used by the display */
Fl_Display_Device::Fl_Display_Device(Fl_Graphics_Driver *graphics_driver) : Fl_Surface_Device(graphics_driver) {
  this->set_current();
};

/** Returns the platform display device. */
Fl_Display_Device *Fl_Display_Device::display_device() {
  static Fl_Display_Device *display = new Fl_Display_Device(new
                                                                  Fl_XXX_Graphics_Driver
                                                                 );
  return display;
};


Fl_Display_Device *Fl_Display_Device::_display = Fl_Display_Device::display_device();

#endif

//
// End of "$Id$".
//
