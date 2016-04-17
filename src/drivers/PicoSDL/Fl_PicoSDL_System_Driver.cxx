//
// "$Id: Fl_PicoSDL_System_Driver.cxx 11241 2016-02-27 13:52:27Z manolo $"
//
// System routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 2016 by Bill Spitzak and others.
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
#include "../../FL/Fl_System_Driver.H"
//#include "Fl_PicoSDL_System_Driver.h"


/*
 * By linking this module, the following static method will instantiate the
 * PicoSDL Graphics driver as the main display driver.
 */
Fl_System_Driver *Fl_System_Driver::newSystemDriver()
{
  return new Fl_System_Driver();
}


//
// End of "$Id: Fl_PicoSDL_System_Driver.cxx 11241 2016-02-27 13:52:27Z manolo $".
//
