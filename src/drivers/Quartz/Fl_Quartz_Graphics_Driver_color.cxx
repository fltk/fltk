//
// MacOS color functions for the Fast Light Tool Kit (FLTK).
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

// The fltk "colormap".  This allows ui colors to be stored in 8-bit
// locations, and provides a level of indirection so that global color
// changes can be made.  Not to be confused with the X colormap, which
// I try to hide completely.

// matt: Neither Quartz nor Quickdraw support colormaps in this implementation
// matt: Quartz support done

#include "Fl_Quartz_Graphics_Driver.H"

#include <config.h>
#include <FL/Fl.H>
#include <FL/platform.H>
#include <FL/fl_draw.H>

extern unsigned fl_cmap[256]; // defined in fl_color.cxx

void Fl_Quartz_Graphics_Driver::color(Fl_Color i) {
  Fl_Graphics_Driver::color(i);
  uchar r, g, b;
  float fa = 1.0f;
  if (i & 0xFFFFFF00) {
    // translate rgb colors into color index
    r = i>>24;
    g = i>>16;
    b = i>> 8;
  } else {
    // translate index into rgb:
    unsigned c = fl_cmap[i];
    c = c ^ 0x000000ff; // trick to restore the color's correct alpha value
    r = c>>24;
    g = c>>16;
    b = c>> 8;
    uchar a = c & 0xff;
    //printf("i=%d rgb=%u,%u,%u a=%u\n",i,r,g,b,a);
    fa = a/255.0f;
  }
  if (!gc_) return; // no context yet? We will assign the color later.
  float fr = r/255.0f;
  float fg = g/255.0f;
  float fb = b/255.0f;
  CGContextSetRGBFillColor(gc_, fr, fg, fb, fa);
  CGContextSetRGBStrokeColor(gc_, fr, fg, fb, fa);
}

void Fl_Quartz_Graphics_Driver::color(uchar r, uchar g, uchar b) {
  Fl_Graphics_Driver::color( fl_rgb_color(r, g, b) );
  float fr = r/255.0f;
  float fg = g/255.0f;
  float fb = b/255.0f;
  if (!gc_) return; // no context yet? We will assign the color later.
  CGContextSetRGBFillColor(gc_, fr, fg, fb, 1.0f);
  CGContextSetRGBStrokeColor(gc_, fr, fg, fb, 1.0f);
}
