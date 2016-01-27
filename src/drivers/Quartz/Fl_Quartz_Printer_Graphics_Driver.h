//
// "$Id: quartz.H 11017 2016-01-20 21:40:12Z matt $"
//
// Definition of Apple Quartz graphics driver
// for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2016 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

#include "../../config_lib.h"
#ifdef FL_CFG_GFX_QUARTZ

#ifndef FL_QUARTZ_PRINTER_GRAPHICS_DRIVER_H
#define FL_QUARTZ_PRINTER_GRAPHICS_DRIVER_H

#include "Fl_Quartz_Graphics_Driver.h"


/** Graphics driver used for Mac OS X printing. */
class Fl_Quartz_Printer_Graphics_Driver : public Fl_Quartz_Graphics_Driver {
public:
  virtual int has_feature(driver_feature mask);
};


#endif // FL_QUARTZ_PRINTER_GRAPHICS_DRIVER_H

#endif // FL_CFG_GFX_QUARTZ

//
// End of "$Id: quartz.H 11017 2016-01-20 21:40:12Z matt $".
//
