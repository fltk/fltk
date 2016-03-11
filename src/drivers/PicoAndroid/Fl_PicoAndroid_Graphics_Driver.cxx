//
// "$Id: Fl_PicoAndroid_Graphics_Driver.cxx 11241 2016-02-27 13:52:27Z manolo $"
//
// Rectangle drawing routines for the Fast Light Tool Kit (FLTK).
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


#include "../../config_lib.h"
#include "Fl_PicoAndroid_Graphics_Driver.h"

#include <jni.h>
#include <errno.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/log.h>
#include <android_native_app_glue.h>

#include <FL/Fl.H>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))


/*
 * By linking this module, the following static method will instatiate the
 * PicoSDL Graphics driver as the main display driver.
 */
Fl_Graphics_Driver *Fl_Graphics_Driver::newMainGraphicsDriver()
{
  return new Fl_PicoAndroid_Graphics_Driver();
}




static GLint vertices[][3] = {
  { -0x10000, -0x10000, -0x10000 },
  {  0x10000, -0x10000, -0x10000 },
  {  0x10000,  0x10000, -0x10000 },
  { -0x10000,  0x10000, -0x10000 },
  { -0x10000, -0x10000,  0x10000 },
  {  0x10000, -0x10000,  0x10000 },
  {  0x10000,  0x10000,  0x10000 },
  { -0x10000,  0x10000,  0x10000 }
};

static GLint colors[][4] = {
  { 0x00000, 0x00000, 0x00000, 0x10000 },
  { 0x10000, 0x00000, 0x00000, 0x10000 },
  { 0x10000, 0x10000, 0x00000, 0x10000 },
  { 0x00000, 0x10000, 0x00000, 0x10000 },
  { 0x00000, 0x00000, 0x10000, 0x10000 },
  { 0x10000, 0x00000, 0x10000, 0x10000 },
  { 0x10000, 0x10000, 0x10000, 0x10000 },
  { 0x00000, 0x10000, 0x10000, 0x10000 }
};

GLubyte indices[] = {
  0, 4, 5,    0, 5, 1,
  1, 5, 6,    1, 6, 2,
  2, 6, 7,    2, 7, 3,
  3, 7, 4,    3, 4, 0,
  4, 7, 6,    4, 6, 5,
  3, 0, 1,    3, 1, 2
};

static void drawSomething()
{
  /*
  static float _angle = 0.0f;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(0, 0, -3.0f);
  glRotatef(_angle, 0, 1, 0);
  glRotatef(_angle*0.25f, 1, 0, 0);

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);

  glFrontFace(GL_CW);
  glVertexPointer(3, GL_FIXED, 0, vertices);
  glColorPointer(4, GL_FIXED, 0, colors);
  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, indices);

  _angle += 1.2f;
   */

  GLfloat q3[] = {
    -10,-10,
    10,-10,
    10,10,
    -10,10
  };

  uchar r, g, b;
  Fl::get_color(FL_RED, r, g, b);
//  Fl::get_color(Fl_Graphics_Driver::color(), r, g, b);
  glColor4ub(r, g, b, 255);

  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(2, GL_FLOAT, 0, q3);
  glDrawArrays(GL_TRIANGLE_FAN,0,4);
  glDisableClientState(GL_VERTEX_ARRAY);
}



void Fl_PicoAndroid_Graphics_Driver::rectf(int x, int y, int w, int h)
{
  GLfloat q3[] = {
    x,     y,
    x,     y+h-3,
    x+w-3, y+h-3,
    x+w-3, y
  };

  uchar r, g, b;
  Fl::get_color(Fl_Graphics_Driver::color(), r, g, b);
  glColor4ub(r, g, b, 255);

  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(2, GL_FLOAT, 0, q3);
  glDrawArrays(GL_TRIANGLE_FAN,0,4);
  glDisableClientState(GL_VERTEX_ARRAY);

  LOGI("Rect: %d %d %d %d", x, y, w, h);
}


void Fl_PicoAndroid_Graphics_Driver::line(int x, int y, int x1, int y1)
{
  GLfloat q3[] = {
    x,   y,
    x1, y1
  };

  uchar r, g, b;
  Fl::get_color(Fl_Graphics_Driver::color(), r, g, b);
  glColor4ub(r, g, b, 255);

  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(2, GL_FLOAT, 0, q3);
  glDrawArrays(GL_LINES,0,2);
  glDisableClientState(GL_VERTEX_ARRAY);
}


void Fl_PicoAndroid_Graphics_Driver::point(int x, int y)
{
  GLfloat q3[] = {
    x, y
  };

  uchar r, g, b;
  Fl::get_color(Fl_Graphics_Driver::color(), r, g, b);
  glColor4ub(r, g, b, 255);

  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(2, GL_POINTS, 0, q3);
  glDrawArrays(GL_LINES,0,1);
  glDisableClientState(GL_VERTEX_ARRAY);
}



//
// End of "$Id: Fl_PicoAndroid_Graphics_Driver.cxx 11241 2016-02-27 13:52:27Z manolo $".
//
