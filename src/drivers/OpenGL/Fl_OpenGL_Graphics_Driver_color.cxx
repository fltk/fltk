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

void Fl_OpenGL_Graphics_Driver::color(Fl_Color i) {
  // FIXME: do we need the code below?
  /*
#if HAVE_GL_OVERLAY
#if defined(_WIN32)
  if (fl_overlay && fl_overlay_depth) {
    if (fl_overlay_depth < 8) {
      // only black & white produce the expected colors.  This could
      // be improved by fixing the colormap set in Fl_Gl_Overlay.cxx
      int size = 1<<fl_overlay_depth;
      if (!i) glIndexi(size-2);
      else if (i >= size-2) glIndexi(size-1);
      else glIndexi(i);
    } else {
      glIndexi(i ? i : FL_GRAY_RAMP);
    }
    return;
  }
#else
  if (fl_overlay) {glIndexi(int(fl_xpixel(i))); return;}
#endif
#endif
*/
  if (i & 0xffffff00) {
    unsigned rgb = (unsigned)i;
    color((uchar)(rgb >> 24), (uchar)(rgb >> 16), (uchar)(rgb >> 8));
  } else {
    Fl_Graphics_Driver::color(i);
    uchar red, green, blue;
    Fl::get_color(i, red, green, blue);
    glColor3ub(red, green, blue);
  }
}

void Fl_OpenGL_Graphics_Driver::color(uchar r,uchar g,uchar b) {
  Fl_Graphics_Driver::color( fl_rgb_color(r, g, b) );
  glColor3ub(r,g,b);
}
