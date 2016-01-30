//
// "$Id$"
//
// Standard X11 font selection code for the Fast Light Tool Kit (FLTK).
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

/*
 This module implements a lowest-common-denominator font for OpenGL.
 It will always work, even if the main graphics library does not support
 rendering text into a texture buffer.
 
 The font is limited to a single face and ASCII characters. It is drawn using
 lines which makes it arbitrarily scalable. I am trying to keep font data really
 compact.
 */


#include <config.h>
#include "../../config_lib.h"
#include "Fl_OpenGL_Graphics_Driver.h"
#include <FL/gl.h>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/Fl.H>
#include <FL/fl_draw.H>


// FIXME: check out FreeGlut:
// FIXME: implement font-to-RGBA in the main graphics driver

#if 1

/*
  |01234567|
 -+--------+
 0|        |____
 1|++++++++|font
 2|++++++++|
 3|++++++++|
 4|++++++++|
 5|++++++++|____
 6|        |descent
 7|        |
 -+--------+
 */


static const char *font_data[128] = {
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,

  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,

  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, /*T*/"\11\71\100\41\45", 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,

  0, 0, 0, 0, 0, /*e*/"\55\25\14\13\22\52\63\64\14", 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, /*s*/"\62\22\13\64\55\15", /*t*/"\41\44\55\65\100\22\62", 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
};


double Fl_OpenGL_Graphics_Driver::width(const char *str, int n) {
  return size_*n*0.5;
}

int Fl_OpenGL_Graphics_Driver::descent() {
  return (int)(size_ - size_*0.8);
}

int Fl_OpenGL_Graphics_Driver::height() {
  return (int)(size_);
}

void Fl_OpenGL_Graphics_Driver::text_extents(const char *str, int n, int& dx, int& dy, int& w, int& h)
{
  dx = 0;
  dy = descent();
  w = width(str, n);
  h = size_;
}

void Fl_OpenGL_Graphics_Driver::draw(const char *str, int n, int x, int y)
{
  int i;
  for (i=0; i<n; i++) {
    char c = str[i] & 0x7f;
    const char *fd = font_data[(int)c];
    if (fd) {
      char rendering = 0;
      float px=0.0f, py=0.0f;
      for (;;) {
        char cmd = *fd++;
        if (cmd==0) {
          if (rendering) {
            glEnd();
            glBegin(GL_POINTS); glVertex2f(px, py); glEnd();
            rendering = 0;
          }
          break;
        } else if (cmd>63) {
          if (cmd=='\100' && rendering) {
            glEnd();
            glBegin(GL_POINTS); glVertex2f(px, py); glEnd();
            rendering = 0;
          }
        } else {
          if (!rendering) { glBegin(GL_LINE_STRIP); rendering = 1; }
          int vx = (cmd & '\70')>>3;
          int vy = (cmd & '\07');
          px = 0.5+x+vx*size_*0.5/8.0;
          py = 0.5+y+vy*size_/8.0-0.8*size_;
          glVertex2f(px, py);
        }
      }
    }
    x += size_*0.5;
  }
}

#elif 0

/*
extern FL_EXPORT Fl_Glut_StrokeFont glutStrokeRoman;
extern FL_EXPORT Fl_Glut_StrokeFont glutStrokeMonoRoman;
#  define GLUT_STROKE_ROMAN             (&glutStrokeRoman)
#  define GLUT_STROKE_MONO_ROMAN        (&glutStrokeMonoRoman)

FL_EXPORT void glutStrokeCharacter(void *font, int character);
FL_EXPORT GLfloat glutStrokeHeight(void *font);
FL_EXPORT int glutStrokeLength(void *font, const unsigned char *string);
FL_EXPORT void glutStrokeString(void *font, const unsigned char *string);
FL_EXPORT int glutStrokeWidth(void *font, int character);
*/

#else

void Fl_OpenGL_Graphics_Driver::draw(const char* str, int n, int x, int y) {
  gl_draw(str, n, x, y);
}

#endif


//
// End of "$Id$".
//
