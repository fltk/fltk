//
// "$Id$"
//
// Arc (integer) drawing functions for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
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

/**
  \file fl_arci.cxx
  \brief Utility functions for drawing circles using integers
*/

// "integer" circle drawing functions.  These draw the limited
// circle types provided by X and NT graphics.  The advantage of
// these is that small ones draw quite nicely (probably due to stored
// hand-drawn bitmaps of small circles!) and may be implemented by
// hardware and thus are fast.

// Probably should add fl_chord.

// 3/10/98: created

#include <FL/fl_draw.H>
#include <config.h>
#include "config_lib.h"

// Remove #ifndef FL_LIBRARY_CMAKE and the entire block of #include
// statements when the new build system is ready:
#ifndef FL_LIBRARY_CMAKE

// -----------------------------------------------------------------------------

// Apple Quartz driver in "drivers/Quartz/Fl_Quartz_Graphics_Driver_arci.cxx"

// -----------------------------------------------------------------------------


#ifdef FL_CFG_GFX_GDI

# include "drivers/GDI/Fl_GDI_Graphics_Driver_arci.cxx"

#endif


// -----------------------------------------------------------------------------


#ifdef FL_CFG_GFX_XLIB

# include "drivers/Xlib/Fl_Xlib_Graphics_Driver_arci.cxx"

#endif


// -----------------------------------------------------------------------------

#endif // FL_LIBRARY_CMAKE

// -----------------------------------------------------------------------------

//
// End of "$Id$".
//
