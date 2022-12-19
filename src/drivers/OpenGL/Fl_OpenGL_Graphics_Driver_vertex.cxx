//
// Portable drawing routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2016 by Bill Spitzak and others.
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
  \file Fl_OpenGL_Graphics_Driver_vertex.cxx
  \brief  Portable drawing code for drawing arbitrary shapes with
          simple 2D transformations, implemented for OpenGL.
*/

#include "Fl_OpenGL_Graphics_Driver.H"

#include <FL/fl_draw.H>
#include <FL/gl.h>
#include <FL/math.h>

#include <stdlib.h>

// OpenGL does not support rednering non-convex polygons. Calling
// glBegin(GL_POLYGON); witha  complex outline will create rather random
// errors, often overwrinting gaps and holes.
//
// Defining SLOW_COMPLEX_POLY will activate a line-by-line drawing method
// for complex polygons that is correct for FLTK, but also a lot slower.
//
// It's recommended that SLOW_COMPLEX_POLY is defined, but fl_begin_polygon()
// is used instead of fl_begin_complex_polygon() whenever possible.

//#undef SLOW_COMPLEX_POLY
#define SLOW_COMPLEX_POLY
#ifdef SLOW_COMPLEX_POLY
# define GAP (1e9f)
#endif

// Event though there are faster versions of the functions in OpenGL,
// we use the default FLTK implementation for compatibility in the
// following functions.

// void Fl_OpenGL_Graphics_Driver::push_matrix()
// void Fl_OpenGL_Graphics_Driver::pop_matrix()
// void Fl_OpenGL_Graphics_Driver::mult_matrix(double a, double b, double c, double d, double x, double y)
// void Fl_OpenGL_Graphics_Driver::rotate(double d)
// double Fl_OpenGL_Graphics_Driver::transform_x(double x, double y)
// double Fl_OpenGL_Graphics_Driver::transform_y(double x, double y)
// double Fl_OpenGL_Graphics_Driver::transform_dx(double x, double y)
// double Fl_OpenGL_Graphics_Driver::transform_dy(double x, double y)

void Fl_OpenGL_Graphics_Driver::begin_points() {
  n = 0; gap_ = 0;
  what = POINTS;
  glBegin(GL_POINTS);
}

void Fl_OpenGL_Graphics_Driver::end_points() {
  glEnd();
}

void Fl_OpenGL_Graphics_Driver::begin_line() {
  n = 0; gap_ = 0;
  what = LINE;
  glBegin(GL_LINE_STRIP);
}

void Fl_OpenGL_Graphics_Driver::end_line() {
  glEnd();
}

void Fl_OpenGL_Graphics_Driver::begin_loop() {
  n = 0; gap_ = 0;
  what = LOOP;
  glBegin(GL_LINE_LOOP);
}

void Fl_OpenGL_Graphics_Driver::end_loop() {
  glEnd();
}

void Fl_OpenGL_Graphics_Driver::begin_polygon() {
  n = 0; gap_ = 0;
  what = POLYGON;
  glBegin(GL_POLYGON);
}

void Fl_OpenGL_Graphics_Driver::end_polygon() {
  glEnd();
}

void Fl_OpenGL_Graphics_Driver::begin_complex_polygon() {
  n = 0;
  what = COMPLEX_POLYGON;
#ifndef SLOW_COMPLEX_POLY
  glBegin(GL_POLYGON);
#endif
}

void Fl_OpenGL_Graphics_Driver::gap() {
#ifdef SLOW_COMPLEX_POLY
  // drop gaps at the start or gap after gap
  if (n==0 || n==gap_) // || pnVertex==pVertexGapStart)
    return;
  // create a loop
  XPOINT& p = xpoint[gap_];
  transformed_vertex(p.x, p.y);
  transformed_vertex(GAP, 0.0);
  gap_ = n;
#else
  glEnd();
  glBegin(GL_POLYGON);
#endif
}

#ifdef SLOW_COMPLEX_POLY

// Draw a complex polygon line by line from the top to the bottom.
void Fl_OpenGL_Graphics_Driver::end_complex_polygon()
{
  int i, y;
  XPOINT *v0, *v1;

  // don't bother if no polygon is defined
  if (n < 2) return;

  // make sure that we always have a closed loop by appending the first
  // coordinate again as the alst coordinate
  gap();

  // find the bounding box for this polygon
  v0 = xpoint;
  v0->y -= 0.1f;
  float xMin = v0->x, xMax = xMin;
  int yMin = (int)v0->y, yMax = yMin;
  for (i = 1; i < n; i++) {
    v0++;
    v0->y -= 0.1f;
    float v0x = v0->x;
    int v0y = (int)v0->y;
    if (v0x == GAP) continue;
    if (v0x <= xMin) xMin = v0x;
    if (v0x >= xMax) xMax = v0x;
    if (v0y <= yMin) yMin = v0y;
    if (v0y >= yMax) yMax = v0y;
  }

  int nNodes;
  float *nodeX = (float*)malloc((n-1)*sizeof(float)), swap;
  if (!nodeX)
    return;

  // loop through the rows of the image
  for (y = yMin; y <= yMax; y++) {
    //  Build a list of all crossing points with this y axis
    v0 = xpoint + 0;
    v1 = xpoint + 1;
    nNodes = 0;
    for (i = 1; i < n; i++) {
      if (v1->x==GAP) { // skip the gap
        i++; v0++; v1++; v0++; v1++;
        continue;
      }
      if (   (v1->y < y && v0->y >= y)
          || (v0->y < y && v1->y >= y) )
      {
        float dy = v0->y - v1->y;
        if (fabsf(dy)>.0001f) {
          nodeX[nNodes++] = v1->x + ((y - v1->y) / dy) * (v0->x - v1->x);
        } else {
          nodeX[nNodes++] = v1->x;
        }
      }
      v0++; v1++;
    }

    // sort the nodes, via a simple Bubble sort
    i = 0;
    while (i < nNodes-1) {
      if (nodeX[i] > nodeX[i+1]) {
        swap = nodeX[i];
        nodeX[i] = nodeX[i+1];
        nodeX[i+1] = swap;
        if (i) i--;
      } else {
        i++;
      }
    }

    //  fill the pixels between node pairs
//    Using lines requires additional attention to the current line width and pattern
//    We are using glRectf instead
//    glBegin(GL_LINES);
    for (i = 0; i < nNodes; i += 2) {
      float x0 = nodeX[i];
      if (x0 >= xMax)
        break;
      float x1 = nodeX[i+1];
      if (x1 > xMin) {
        if (x0 < xMin)
          x0 = xMin;
        if (x1 > xMax)
          x1 = xMax;
        glRectf((GLfloat)(x0-0.25f), (GLfloat)(y), (GLfloat)(x1+0.25f), (GLfloat)(y+1.0f));
//        glVertex2f((GLfloat)x0, (GLfloat)y);
//        glVertex2f((GLfloat)x1, (GLfloat)y);
      }
    }
//    glEnd();
  }

  ::free(nodeX);
}

#else

// FXIME: non-convex polygons are not supported yet
// use gluTess* functions to do this; search for gluBeginPolygon
void Fl_OpenGL_Graphics_Driver::end_complex_polygon() {
  glEnd();
}

#endif


// remove equal points from closed path
void Fl_OpenGL_Graphics_Driver::fixloop() { }

void Fl_OpenGL_Graphics_Driver::transformed_vertex(double xf, double yf) {
#ifdef SLOW_COMPLEX_POLY
  if (what==COMPLEX_POLYGON) {
    Fl_Graphics_Driver::transformed_vertex(xf, yf);
  } else {
    glVertex2d(xf, yf);
  }
#else
  glVertex2d(xf, yf);
#endif
}

void Fl_OpenGL_Graphics_Driver::circle(double cx, double cy, double r) {
  double rx = r * (m.c ? sqrt(m.a*m.a+m.c*m.c) : fabs(m.a));
  double ry = r * (m.b ? sqrt(m.b*m.b+m.d*m.d) : fabs(m.d));
  double rMax;
  if (ry>rx) rMax = ry; else rMax = rx;

  // from http://slabode.exofire.net/circle_draw.shtml and many other places
  int num_segments = (int)(10 * sqrt(rMax))+1;
  double theta = 2 * M_PI / float(num_segments);
  double tangetial_factor = tan(theta);
  double radial_factor = cos(theta);//calculate the radial factor
  double x = r; //we start at angle = 0
  double y = 0;

  glBegin(GL_LINE_LOOP);
  for(int ii = 0; ii < num_segments; ii++) {
    vertex(x + cx, y + cy); // output vertex
    double tx = -y;
    double ty = x;
    x += tx * tangetial_factor;
    y += ty * tangetial_factor;
    x *= radial_factor;
    y *= radial_factor;
  }
  glEnd();

}
