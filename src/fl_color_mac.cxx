//
// "$Id$"
//
// MacOS color functions for the Fast Light Tool Kit (FLTK).
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

// The fltk "colormap".  This allows ui colors to be stored in 8-bit
// locations, and provides a level of indirection so that global color
// changes can be made.  Not to be confused with the X colormap, which
// I try to hide completely.

// matt: Neither Quartz nor Quickdraw support colormaps in this implementation
// matt: Quartz support done

#include <config.h>
#include <FL/Fl.H>
#include <FL/x.H>
#include <FL/fl_draw.H>

static unsigned fl_cmap[256] = {
#include "fl_cmap.h" // this is a file produced by "cmap.cxx":
};

void Fl_Quartz_Graphics_Driver::color(Fl_Color i) {
  Fl_Graphics_Driver::color(i);
  uchar r, g, b;
  if (i & 0xFFFFFF00) {
    // translate rgb colors into color index
    r = i>>24;
    g = i>>16;
    b = i>> 8;
  } else {
    // translate index into rgb:
    unsigned c = fl_cmap[i];
    r = c>>24;
    g = c>>16;
    b = c>> 8;
  }
  if (!fl_gc) return; // no context yet? We will assign the color later.
  float fr = r/255.0f;
  float fg = g/255.0f;
  float fb = b/255.0f;
  CGContextSetRGBFillColor(fl_gc, fr, fg, fb, 1.0f);
  CGContextSetRGBStrokeColor(fl_gc, fr, fg, fb, 1.0f);
}

void Fl_Quartz_Graphics_Driver::color(uchar r, uchar g, uchar b) {
  Fl_Graphics_Driver::color( fl_rgb_color(r, g, b) );
  float fr = r/255.0f;
  float fg = g/255.0f;
  float fb = b/255.0f;
  if (!fl_gc) return; // no context yet? We will assign the color later.
  CGContextSetRGBFillColor(fl_gc, fr, fg, fb, 1.0f);
  CGContextSetRGBStrokeColor(fl_gc, fr, fg, fb, 1.0f);
}

void Fl::set_color(Fl_Color i, unsigned c) {
  if (fl_cmap[i] != c) {
    fl_cmap[i] = c;
  }
}

//
// End of "$Id$".
//
