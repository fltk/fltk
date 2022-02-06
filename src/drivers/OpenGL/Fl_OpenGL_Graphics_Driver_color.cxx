//
// Color functions for the Fast Light Tool Kit (FLTK).
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
  \file fl_color.cxx
  \brief Color handling
*/

#include <config.h>
#include "Fl_OpenGL_Graphics_Driver.H"
#include <FL/gl.h>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/Fl.H>
#include <FL/fl_draw.H>


// Implementation of fl_color(i), fl_color(r,g,b).

extern unsigned fl_cmap[256]; // defined in fl_color.cxx

void Fl_OpenGL_Graphics_Driver::color(Fl_Color i) {
  if (i & 0xffffff00) {
    unsigned rgba = ((unsigned)i)^0x000000ff;
    Fl_Graphics_Driver::color(i);
    glColor4ub(rgba>>24, rgba>>16, rgba>>8, rgba);
  } else {
    unsigned rgba = ((unsigned)fl_cmap[i])^0x000000ff;
    Fl_Graphics_Driver::color(fl_cmap[i]);
    glColor4ub(rgba>>24, rgba>>16, rgba>>8, rgba);
  }
}

void Fl_OpenGL_Graphics_Driver::color(uchar r, uchar g, uchar b) {
  Fl_Graphics_Driver::color( fl_rgb_color(r, g, b) );
  glColor3ub(r,g,b);
}
