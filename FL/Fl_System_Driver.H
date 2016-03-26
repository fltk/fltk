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
 \brief declaration of class Fl_System_Driver.
*/

#ifndef FL_SYSTEM_DRIVER_H
#define FL_SYSTEM_DRIVER_H

#include <FL/Fl_Export.H>

#ifdef __APPLE__
typedef struct CGContext* Fl_Offscreen;
typedef struct CGImage* Fl_Bitmask;
typedef struct flCocoaRegion* Fl_Region;

#elif defined(WIN32)
typedef struct HBITMAP__ *HBITMAP;
typedef HBITMAP Fl_Offscreen;
typedef HBITMAP Fl_Bitmask;
typedef struct HRGN__ *Fl_Region;

#elif defined(FL_PORTING)
# pragma message "FL_PORTING: define OS-dependent types"
typedef void* Fl_Offscreen;
typedef void* Fl_Bitmask;
typedef void *Fl_Region;

#else
typedef unsigned long Fl_Offscreen;
typedef unsigned long Fl_Bitmask;
typedef struct _XRegion *Fl_Region;

#endif // __APPLE__

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
