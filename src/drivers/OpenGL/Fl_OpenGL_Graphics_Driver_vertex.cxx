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

//#undef SLOW_COMPLEX_POLY
#define SLOW_COMPLEX_POLY

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
  // TODO: implement this
  // drop gaps at the start or gap after gap
//  if (pnVertex==0 || pnVertex==pVertexGapStart)
//    return;
//
//  // create a loop
//  Vertex &v = pVertex[pVertexGapStart];
//  add_vertex(v.pX, v.pY, true);
//  pVertexGapStart = pnVertex;
  // drop gaps at the start or gap after gap
  if (n==0 || n==gap_) // || pnVertex==pVertexGapStart)
    return;

  // create a loop
  XPOINT& p = xpoint[gap_];
  transformed_vertex(p.x, p.y);
  transformed_vertex(999999.0, 0.0);
  gap_ = n;
#else
  glEnd();
  glBegin(GL_POLYGON);
#endif
}

#ifdef SLOW_COMPLEX_POLY

void Fl_OpenGL_Graphics_Driver::end_complex_polygon()
{
  if (n < 2) return;

  gap(); // adds the first coordinate of this loop and marks it as a gap
  int begin = 0, end = n;

  XPOINT *v = xpoint+0;
//  v->x = roundf(v->x);
  v->y = roundf(v->y);
  float xMin = v->x, xMax = xMin;
  int yMin = v->y, yMax = yMin;
  for (int i = begin+1; i < end; i++) {
    v = xpoint+i;
//    v->x = roundf(v->x);
    v->y = roundf(v->y);
    if (v->x < xMin) xMin = v->x;
    if (v->x > xMax) xMax = v->x;
    if (v->y < yMin) yMin = v->y;
    if (v->y > yMax) yMax = v->y;
  }
  xMax++; yMax++;

  int nodes, pixelY, i, j, swap;
  float nodeX[end - begin], pixelX;

  //  Loop through the rows of the image.
  for (pixelY = yMin; pixelY < yMax; pixelY++) {
//    printf("Y=%d\n", pixelY);
    //  Build a list of nodes.
    nodes = 0;
    for (i = begin+1; i < end; i++) {
      j = i-1;
//      if (xpoint[j].pIsGap)
      if (xpoint[i].x==999999.0) {
        i++;
        continue;
      }
      if (   (xpoint[i].y < pixelY && xpoint[j].y >= pixelY)
          || (xpoint[j].y < pixelY && xpoint[i].y >= pixelY) )
      {
        float dy = xpoint[j].y - xpoint[i].y;
        if (fabsf(dy)>.0001) {
          nodeX[nodes++] = (int)(xpoint[i].x +
                                 (pixelY - xpoint[i].y) / dy
                                 * (xpoint[j].x - xpoint[i].x));

        } else {
          nodeX[nodes++] = xpoint[i].x;
        }
      }
    }
    //Fl_Android_Application::log_e("%d nodes (must be even!)", nodes);

    //  Sort the nodes, via a simple “Bubble” sort.
    i = 0;
    while (i < nodes - 1) {
      if (nodeX[i] > nodeX[i + 1]) {
        swap = nodeX[i];
        nodeX[i] = nodeX[i + 1];
        nodeX[i + 1] = swap;
        if (i) i--;
      } else {
        i++;
      }
    }

    //  Fill the pixels between node pairs.
    for (i = 0; i < nodes; i += 2) {
//      printf("n[%d] = %d %d\n", i, nodeX[i], nodeX[i+1]);
      if (nodeX[i] >= xMax) break;
      if (nodeX[i + 1] > xMin) {
        if (nodeX[i] < xMin) nodeX[i] = xMin;
        if (nodeX[i + 1] > xMax) nodeX[i + 1] = xMax;
        //xyline(nodeX[i], pixelY, nodeX[i + 1]);
        glBegin(GL_LINE_STRIP);
        glVertex2f(nodeX[i], pixelY);
        glVertex2f(nodeX[i+1], pixelY);
        glEnd();
      }
    }
  }
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
