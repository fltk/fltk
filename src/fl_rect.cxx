//
// "$Id$"
//
// Rectangle drawing routines for the Fast Light Tool Kit (FLTK).
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
  \file fl_rect.cxx
  \brief Drawing and clipping routines for rectangles.
*/

// These routines from fl_draw.H are used by the standard boxtypes
// and thus are always linked into an fltk program.
// Also all fl_clip routines, since they are always linked in so
// that minimal update works.

#include <config.h>
#include "config_lib.h"
#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Printer.H>
#include <FL/fl_draw.H>
#include <FL/x.H>

#if defined(WIN32) || defined(__APPLE__)
#elif defined(FL_PORTING)
#  pragma message "FL_PORTING: implement all the line drawing functions below"
#else
#endif

// fl_line_width_ must contain the absolute value of the current
// line width to be used for X11 clipping (see below).
// This is defined in src/fl_line_style.cxx
extern int fl_line_width_;


void Fl_Graphics_Driver::restore_clip() {
  fl_clip_state_number++;
}


void Fl_Graphics_Driver::clip_region(Fl_Region r) {
  Fl_Region oldr = rstack[rstackptr];
  if (oldr) XDestroyRegion(oldr);
  rstack[rstackptr] = r;
  fl_restore_clip();
}


Fl_Region Fl_Graphics_Driver::clip_region() {
  return rstack[rstackptr];
}



////////////////////////////////////////////////////////////////

/*
 Matt: I wrote individual methods for every class. They are virtual, so the
 correct function is called, depending on the active driver.

 By having individual methods, multiple drivers can co-exist, for example
 Quartz, OpenGL, and a printer driver.

 The individual implementations should eventually go into files that are
 included into this file, based on the configuration, for example:

 src/cfg_gfx/quartz_rect.cxx
 src/cfg_gfx/gdi_rect.cxx
 src/cfg_gfx/xlib_rect.cxx

 Porting the graphics system to a new platform then requires to copy one of
 these files and implement the virtual functions. point() is the only function
 that *must* be implemented when deriving from 'Fl_Minimal_Graphics_Driver"
 (which is still to be written)
 */

////////////////////////////////////////////////////////////////


#ifdef FL_CFG_GFX_QUARTZ

# include "cfg_gfx/quartz_rect.cxx"

#endif


// -----------------------------------------------------------------------------


#ifdef FL_CFG_GFX_GDI

# include "cfg_gfx/gdi_rect.cxx"

#endif


// -----------------------------------------------------------------------------


#ifdef FL_CFG_GFX_XLIB

# include "cfg_gfx/xlib_rect.cxx"

#endif


//
// End of "$Id$".
//
