//
// Rectangle drawing routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2018 by Bill Spitzak and others.
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

/**
  \file fl_rect.cxx
  \brief Drawing and clipping routines for rectangles.
*/

// These routines from fl_draw.H are used by the standard boxtypes
// and thus are always linked into an fltk program.
// Also all fl_clip routines, since they are always linked in so
// that minimal update works.

#include <FL/platform.H>
#include <FL/Fl_Graphics_Driver.H>

// -----------------------------------------------------------------------------
// all driver code is now in drivers/XXX/Fl_XXX_Graphics_Driver_xyz.cxx
// -----------------------------------------------------------------------------

/**
 \cond DriverDev
 \addtogroup DriverDeveloper
 \{
 */

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

/**
 \}
 \endcond
 */
