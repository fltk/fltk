//
// Line style code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2017 by Bill Spitzak and others.
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
  \file Fl_OpenGL_Graphics_Driver_line_style.cxx
  \brief Line style drawing utility hiding different platforms.
*/

#include <config.h>
#include "Fl_OpenGL_Graphics_Driver.H"
#include <FL/gl.h>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/Fl.H>
#include <FL/fl_draw.H>

// OpenGL implementation does not support custom patterns
// OpenGL implementation does not support cap and join types

void Fl_OpenGL_Graphics_Driver::line_style(int style, int width, char* dashes) {
  if (width<1) width = 1;
  line_width_ = (float)width;

  int stipple = style & 0x00ff;
  line_stipple_ = stipple;
//  int cap     = style & 0x0f00;
//  int join    = style & 0xf000;

  if (stipple==FL_SOLID) {
    glLineStipple(1, 0xFFFF);
    glDisable(GL_LINE_STIPPLE);
  } else {
    char enable = 1;
    switch (stipple & 0x00ff) {
      case FL_DASH:
        glLineStipple(GLint(pixels_per_unit_*line_width_), 0x0F0F); // ....****....****
        break;
      case FL_DOT:
        glLineStipple(GLint(pixels_per_unit_*line_width_), 0x5555); // .*.*.*.*.*.*.*.*
        break;
      case FL_DASHDOT:
        glLineStipple(GLint(pixels_per_unit_*line_width_), 0x2727); // ..*..***..*..***
        break;
      case FL_DASHDOTDOT:
        glLineStipple(GLint(pixels_per_unit_*line_width_), 0x5757); // .*.*.***.*.*.***
        break;
      default:
        glLineStipple(1, 0xFFFF);
        enable = 0;
    }
    if (enable)
      glEnable(GL_LINE_STIPPLE);
    else
      glDisable(GL_LINE_STIPPLE);
  }
  glLineWidth( (GLfloat)(pixels_per_unit_ * line_width_) );
  glPointSize( (GLfloat)(pixels_per_unit_) );
}
