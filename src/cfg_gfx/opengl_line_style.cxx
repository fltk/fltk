//
// "$Id$"
//
// Line style code for the Fast Light Tool Kit (FLTK).
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

#ifndef FL_CFG_GFX_OPENGL_LINE_STYLE_CXX
#define FL_CFG_GFX_OPENGL_LINE_STYLE_CXX

/**
  \file opengl_line_style.cxx
  \brief Line style drawing utility hiding different platforms.
*/

#include "opengl.H"
#include <FL/gl.H>

extern int fl_line_width_;

// OpenGL implementation does not support custom patterns
// OpenGL implementation does not support cap and join types

void Fl_OpenGL_Graphics_Driver::line_style(int style, int width, char* dashes) {

  // save line width in global variable for X11 clipping
  // FIXME: what does this code do?
  if (width == 0) fl_line_width_ = 1;
  else fl_line_width_ = width>0 ? width : -width;

  if (width<1) width = 1;

  if (style==FL_SOLID) {
    glLineStipple(1, 0xFFFF);
    glDisable(GL_LINE_STIPPLE);
  } else {
    switch (style) {
      case FL_DASH:
        glLineStipple(width, 0x0F0F); // ....****....****
        break;
      case FL_DOT:
        glLineStipple(width, 0x5555); // .*.*.*.*.*.*.*.*
        break;
      case FL_DASHDOT:
        glLineStipple(width, 0x2727); // ..*..***..*..***
        break;
      case FL_DASHDOTDOT:
        glLineStipple(width, 0x5757); // .*.*.***.*.*.***
        break;
    }
    glEnable(GL_LINE_STIPPLE);
  }
}

#endif // FL_CFG_GFX_OPENGL_LINE_STYLE_CXX

//
// End of "$Id$".
//
