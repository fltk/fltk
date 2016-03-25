//
// "$Id$"
//
// A base class for platform specific system calls
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

/** \file Fl_System_Driver.H
 \brief declaration of classe Fl_System_Driver.
*/

#ifndef FL_SYSTEM_DRIVER_H
#define FL_SYSTEM_DRIVER_H

#include <FL/Fl_Device.H>
#include <FL/Fl_Export.H>

// Pointers you can use to change FLTK to a foreign language.
// Note: Similar pointers are defined in FL/fl_ask.H and src/fl_ask.cxx
extern FL_EXPORT const char* fl_local_alt;
extern FL_EXPORT const char* fl_local_ctrl;
extern FL_EXPORT const char* fl_local_meta;
extern FL_EXPORT const char* fl_local_shift;

/**
 \brief A base class for platform specific window handling code.
  */
class FL_EXPORT Fl_System_Driver {
public:
  Fl_System_Driver();
  virtual ~Fl_System_Driver();
};


#endif // FL_SYSTEM_DRIVER_H

//
// End of "$Id$".
//
