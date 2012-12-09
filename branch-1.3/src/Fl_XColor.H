//
// "$Id$"
//
// X-specific color definitions for the Fast Light Tool Kit (FLTK).
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

#include <config.h>
#include <FL/Enumerations.H>

// one of these for each color in fltk's "colormap":
// if overlays are enabled, another one for the overlay
struct Fl_XColor {
  unsigned char r,g,b;	// actual color used by X
  unsigned char mapped;	// true when XAllocColor done
  unsigned long pixel;	// the X pixel to use
};
extern Fl_XColor fl_xmap[/*overlay*/][256];

// mask & shifts to produce xcolor for truecolor visuals:
extern unsigned char fl_redmask, fl_greenmask, fl_bluemask;
extern int fl_redshift, fl_greenshift, fl_blueshift, fl_extrashift;

//
// End of "$Id$".
//
