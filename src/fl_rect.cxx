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

/**
  \file fl_rect.cxx
  \brief Drawing and clipping routines for rectangles.
*/

// These routines from fl_draw.H are used by the standard boxtypes
// and thus are always linked into an fltk program.
// Also all fl_clip routines, since they are always linked in so
// that minimal update works.

#include <FL/x.H>
#include <FL/Fl_Graphics_Driver.H>

// -----------------------------------------------------------------------------
// all driver code is now in drivers/XXX/Fl_XXX_Graphics_Driver_xyz.cxx
// -----------------------------------------------------------------------------

// fl_line_width_ must contain the absolute value of the current
// line width to be used for X11 clipping (see driver code).
// This is defined in src/fl_line_style.cxx
extern int fl_line_width_;


/** see fl_restore_clip() */
void Fl_Graphics_Driver::restore_clip() {
  fl_clip_state_number++;
}

/** see fl_clip_region(Fl_Region) */
void Fl_Graphics_Driver::clip_region(Fl_Region r) {
  Fl_Region oldr = rstack[rstackptr];
  if (oldr) XDestroyRegion(oldr);
  rstack[rstackptr] = r;
  restore_clip();
}


/** see fl_clip_region(void) */
Fl_Region Fl_Graphics_Driver::clip_region() {
  return rstack[rstackptr];
}


//
// End of "$Id$".
//
