//
// "$Id$"
//
// Rectangle drawing routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2016 by Bill Spitzak and others.
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

#include "Fl_Quartz_Printer_Graphics_Driver.h"

int Fl_Quartz_Printer_Graphics_Driver::has_feature(driver_feature mask)
{
  return mask & (NATIVE | PRINTER);
}

#endif // FL_CFG_GFX_QUARTZ

//
// End of "$Id$".
//
