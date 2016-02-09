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


#include <config.h>
#include "../../config_lib.h"
#include "Fl_GDI_Graphics_Driver.h"


const char *Fl_GDI_Graphics_Driver::class_id = "Fl_GDI_Graphics_Driver";

// FIXME: move to printer graphics driver
const char *Fl_GDI_Printer_Graphics_Driver::class_id = "Fl_GDI_Printer_Graphics_Driver";

/*
 * By linking this module, the following static method will instatiate the
 * MSWindows GDI Graphics driver as the main display driver.
 */
Fl_Graphics_Driver *Fl_Graphics_Driver::newMainGraphicsDriver()
{
  return new Fl_GDI_Graphics_Driver();
}


//
// End of "$Id$".
//
