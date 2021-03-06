//
// Rectangle drawing routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2020 by Bill Spitzak and others.
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
 \file Fl_OpenGL_Graphics_Driver_rect.cxx
 \brief OpenGL specific line and polygon drawing with integer coordinates.
 */

#include <config.h>
#include "Fl_OpenGL_Graphics_Driver.H"
#include <FL/gl.h>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/Fl.H>

// --- line and polygon drawing with integer coordinates

void Fl_OpenGL_Graphics_Driver::point(int x, int y) {
  glBegin(GL_POINTS);
  glVertex2i(x, y);
  glEnd();
}

void Fl_OpenGL_Graphics_Driver::rect(int x, int y, int w, int h) {
  glBegin(GL_LINE_LOOP);
  glVertex2i(x, y);
  glVertex2i(x+w, y);
  glVertex2i(x+w, y+h);
  glVertex2i(x, y+h);
  glEnd();
}

void Fl_OpenGL_Graphics_Driver::rectf(int x, int y, int w, int h) {
  if (w<=0 || h<=0) return;
  // OpenGL has the natural origin at the bottom left. Drawing in FLTK
  // coordinates requires that we shift the rectangle one pixel up.
  glBegin(GL_POLYGON);
  glVertex2i(x, y-1);
  glVertex2i(x+w, y-1);
  glVertex2i(x+w, y+h-1);
  glVertex2i(x, y+h-1);
  glEnd();
}

void Fl_OpenGL_Graphics_Driver::line(int x, int y, int x1, int y1) {
  glBegin(GL_LINE_STRIP);
  glVertex2i(x, y);
  glVertex2i(x1, y1);
  glEnd();
  point(x1, y1);
}

void Fl_OpenGL_Graphics_Driver::line(int x, int y, int x1, int y1, int x2, int y2) {
  glBegin(GL_LINE_STRIP);
  glVertex2i(x, y);
  glVertex2i(x1, y1);
  glVertex2i(x2, y2);
  glEnd();
  point(x2, y2);
}

void Fl_OpenGL_Graphics_Driver::xyline(int x, int y, int x1) {
  glBegin(GL_LINE_STRIP);
  glVertex2i(x, y);
  glVertex2i(x1, y);
  glEnd();
  point(x1, y);
}

void Fl_OpenGL_Graphics_Driver::xyline(int x, int y, int x1, int y2) {
  glBegin(GL_LINE_STRIP);
  glVertex2i(x, y);
  glVertex2i(x1, y);
  glVertex2i(x1, y2);
  glEnd();
  point(x1, y2);
}

void Fl_OpenGL_Graphics_Driver::xyline(int x, int y, int x1, int y2, int x3) {
  glBegin(GL_LINE_STRIP);
  glVertex2i(x, y);
  glVertex2i(x1, y);
  glVertex2i(x1, y2);
  glVertex2i(x3, y2);
  glEnd();
  point(x3, y2);
}

void Fl_OpenGL_Graphics_Driver::yxline(int x, int y, int y1) {
  glBegin(GL_LINE_STRIP);
  glVertex2i(x, y);
  glVertex2i(x, y1);
  glEnd();
  point(x, y1);
}

void Fl_OpenGL_Graphics_Driver::yxline(int x, int y, int y1, int x2) {
  glBegin(GL_LINE_STRIP);
  glVertex2i(x, y);
  glVertex2i(x, y1);
  glVertex2i(x2, y1);
  glEnd();
  point(x2, y1);
}

void Fl_OpenGL_Graphics_Driver::yxline(int x, int y, int y1, int x2, int y3) {
  glBegin(GL_LINE_STRIP);
  glVertex2i(x, y);
  glVertex2i(x, y1);
  glVertex2i(x2, y1);
  glVertex2i(x2, y3);
  glEnd();
  point(x2, y3);
}

void Fl_OpenGL_Graphics_Driver::loop(int x0, int y0, int x1, int y1, int x2, int y2) {
  glBegin(GL_LINE_LOOP);
  glVertex2i(x0, y0);
  glVertex2i(x1, y1);
  glVertex2i(x2, y2);
  glEnd();
}

void Fl_OpenGL_Graphics_Driver::loop(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) {
  glBegin(GL_LINE_LOOP);
  glVertex2i(x0, y0);
  glVertex2i(x1, y1);
  glVertex2i(x2, y2);
  glVertex2i(x3, y3);
  glEnd();
}

void Fl_OpenGL_Graphics_Driver::polygon(int x0, int y0, int x1, int y1, int x2, int y2) {
  glBegin(GL_POLYGON);
  glVertex2i(x0, y0);
  glVertex2i(x1, y1);
  glVertex2i(x2, y2);
  glEnd();
}

void Fl_OpenGL_Graphics_Driver::polygon(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) {
  glBegin(GL_POLYGON);
  glVertex2i(x0, y0);
  glVertex2i(x1, y1);
  glVertex2i(x2, y2);
  glVertex2i(x3, y3);
  glEnd();
}

void Fl_OpenGL_Graphics_Driver::push_clip(int x, int y, int w, int h) {
  // TODO: implement OpenGL clipping
  if (rstackptr < region_stack_max) rstack[++rstackptr] = 0L;
  else Fl::warning("Fl_OpenGL_Graphics_Driver::push_clip: clip stack overflow!\n");
}

int Fl_OpenGL_Graphics_Driver::clip_box(int x, int y, int w, int h, int &X, int &Y, int &W, int &H) {
  // TODO: implement OpenGL clipping
  X = x; Y = y; W = w; H = h;
  return 0;
}

int Fl_OpenGL_Graphics_Driver::not_clipped(int x, int y, int w, int h) {
  // TODO: implement OpenGL clipping
  return 1;
}

void Fl_OpenGL_Graphics_Driver::restore_clip() {
  // TODO: implement OpenGL clipping
  fl_clip_state_number++;
}
