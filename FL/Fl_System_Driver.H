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

#ifdef __APPLE__
#include <src/drivers/Darwin/Fl_Darwin_System_Driver.H>

#elif defined(WIN32)
#include <src/drivers/WinAPI/Fl_WinAPI_System_Driver.H>

#elif defined(FL_PORTING)
# pragma message "FL_PORTING: define OS-dependent types"
typedef void* Fl_Offscreen;
typedef void* Fl_Bitmask;
typedef void *Fl_Region;
#else

#include <src/drivers/Posix/Fl_Posix_System_Driver.H>
#endif // __APPLE__

#ifndef FL_SYSTEM_DRIVER_H
#define FL_SYSTEM_DRIVER_H

#include <FL/Fl_Export.H>

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
