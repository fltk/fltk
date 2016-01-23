//
// "$Id$"
//
// Arc (integer) drawing functions for the Fast Light Tool Kit (FLTK).
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

#ifndef FL_CFG_GFX_OPENGL_ARCI_CXX
#define FL_CFG_GFX_OPENGL_ARCI_CXX

/**
  \file opengl_arci.cxx
  \brief Utility functions for drawing circles using integers
*/

#include <FL/gl.h>

void Fl_OpenGL_Graphics_Driver::arc(int x,int y,int w,int h,double a1,double a2) {
  if (w <= 0 || h <= 0) return;
  while (a2<a1) a2 += 360.0;  // TODO: write a sensible fmod angle alignment here
  a1 = a1/180.0f*M_PI; a2 = a2/180.0f*M_PI;
  double cx = x + 0.5f*w - 0.5f, cy = y + 0.5f*h - 0.5f;
  double rMax; if (w<h) rMax = h/2; else rMax = w/2;
  int nSeg = (int)(10 * sqrt(rMax))+1;
  double incr = (a2-a1)/(double)nSeg;

  glBegin(GL_LINE_STRIP);
  for (int i=0; i<nSeg; i++) {
    glVertex2d(cx+cos(a1)*rMax, cy-sin(a1)*rMax);
    a1 += incr;
  }
  glEnd();
}

void Fl_OpenGL_Graphics_Driver::pie(int x,int y,int w,int h,double a1,double a2) {
  if (w <= 0 || h <= 0) return;
  while (a2<a1) a2 += 360.0;  // TODO: write a sensible fmod angle alignment here
  a1 = a1/180.0f*M_PI; a2 = a2/180.0f*M_PI;
  double cx = x + 0.5f*w - 0.5f, cy = y + 0.5f*h - 0.5f;
  double rMax; if (w<h) rMax = h/2; else rMax = w/2;
  int nSeg = (int)(10 * sqrt(rMax))+1;
  double incr = (a2-a1)/(double)nSeg;

  glBegin(GL_TRIANGLE_FAN);
  glVertex2d(cx, cy);
  for (int i=0; i<nSeg+1; i++) {
    glVertex2d(cx+cos(a1)*rMax, cy-sin(a1)*rMax);
    a1 += incr;
  }
  glEnd();
}

#endif FL_CFG_GFX_OPENGL_ARCI_CXX

//
// End of "$Id$".
//
