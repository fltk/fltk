//
// System routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 2016 by Bill Spitzak and others.
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


#include <config.h>
#include "../../Fl_System_Driver.H"
//#include "Fl_PicoSDL_System_Driver.h"


/*
 By linking this module, the following static method will instantiate the
 PicoSDL Graphics driver as the main display driver.
 */
Fl_System_Driver *Fl_System_Driver::newSystemDriver()
{
  return new Fl_System_Driver();
}
