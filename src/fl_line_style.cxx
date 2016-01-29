//
// "$Id$"
//
// Line style code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2012 by Bill Spitzak and others.
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
  \file fl_line_style.cxx
  \brief Line style drawing utility hiding different platforms.
*/

#include "config_lib.h"
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/x.H>
#include <FL/Fl_Printer.H>
#include "flstring.h"
#include <stdio.h>

// We save the current line width (absolute value) here.
// This is currently used only for X11 clipping, see src/fl_rect.cxx.
// FIXME: this would probably better be in class Fl::
int fl_line_width_ = 0;


// Remove #ifndef FL_LIBRARY_CMAKE and the entire block of #include
// statements when the new build system is ready:
#ifndef FL_LIBRARY_CMAKE

// -----------------------------------------------------------------------------

// Apple Quartz driver in "drivers/Quartz/Fl_Quartz_Graphics_Driver_line_style.cxx"

// -----------------------------------------------------------------------------


#ifdef FL_CFG_GFX_GDI

# include "drivers/GDI/Fl_GDI_Graphics_Driver_line_style.cxx"

#endif


// -----------------------------------------------------------------------------


#ifdef FL_CFG_GFX_XLIB

// # include "drivers/Xlib/Fl_Xlib_Graphics_Driver_line_style.cxx"

#endif


// -----------------------------------------------------------------------------

#endif // FL_LIBRARY_CMAKE

//
// End of "$Id$".
//
