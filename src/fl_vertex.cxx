//
// "$Id: fl_vertex.cxx,v 1.5.2.3 2001/01/22 15:13:41 easysw Exp $"
//
// Portable drawing routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2001 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to "fltk-bugs@fltk.org".
//

// Portable drawing code for drawing arbitrary shapes with
// simple 2D transformations.  See also fl_arc.C

#include <FL/fl_draw.H>
#include <FL/x.H>
#include <FL/math.h>
#include <stdlib.h>

struct matrix {double a, b, c, d, x, y;};

static matrix m = {1, 0, 0, 1, 0, 0};

static matrix stack[10];
static int sptr = 0;

void fl_push_matrix() {stack[sptr++] = m;}

void fl_pop_matrix() {m = stack[--sptr];}

void fl_mult_matrix(double a, double b, double c, double d, double x, double y) {
  matrix o;
  o.a = a*m.a + b*m.c;
  o.b = a*m.b + b*m.d;
  o.c = c*m.a + d*m.c;
  o.d = c*m.b + d*m.d;
  o.x = x*m.a + y*m.c + m.x;
  o.y = x*m.b + y*m.d + m.y;
  m = o;
}

void fl_scale(double x,double y) {fl_mult_matrix(x,0,0,y,0,0);}

void fl_scale(double x) {fl_mult_matrix(x,0,0,x,0,0);}

void fl_translate(double x,double y) {fl_mult_matrix(1,0,0,1,x,y);}

void fl_rotate(double d) {
  if (d) {
    double s, c;
    if (d == 0) {s = 0; c = 1;}
    else if (d == 90) {s = 1; c = 0;}
    else if (d == 180) {s = 0; c = -1;}
    else if (d == 270 || d == -90) {s = -1; c = 0;}
    else {s = sin(d*M_PI/180); c = cos(d*M_PI/180);}
    fl_mult_matrix(c,-s,s,c,0,0);
  }
}

static XPoint *p = (XPoint *)0;
// typedef what the x,y fields in a point are:
#ifdef WIN32
typedef int COORD_T;
#else
typedef short COORD_T;
#endif

static int p_size;
static int n;
static int what;
enum {LINE, LOOP, POLYGON, POINT_};

void fl_begin_points() {n = 0; what = POINT_;}

void fl_begin_line() {n = 0; what = LINE;}

void fl_begin_loop() {n = 0; what = LOOP;}

void fl_begin_polygon() {n = 0; what = POLYGON;}

double fl_transform_x(double x, double y) {return x*m.a + y*m.c + m.x;}

double fl_transform_y(double x, double y) {return x*m.b + y*m.d + m.y;}

double fl_transform_dx(double x, double y) {return x*m.a + y*m.c;}

double fl_transform_dy(double x, double y) {return x*m.b + y*m.d;}

static void fl_transformed_vertex(COORD_T x, COORD_T y) {
  if (!n || x != p[n-1].x || y != p[n-1].y) {
    if (n >= p_size) {
      p_size = p ? 2*p_size : 16;
      p = (XPoint *)realloc((void*)p, p_size*sizeof(*p));
    }
    p[n].x = x;
    p[n].y = y;
    n++;
  }
}

void fl_transformed_vertex(double xf, double yf) {
  fl_transformed_vertex(COORD_T(xf+.5), COORD_T(yf+.5));
}

void fl_vertex(double x,double y) {
  fl_transformed_vertex(x*m.a + y*m.c + m.x, x*m.b + y*m.d + m.y);
}

void fl_end_points() {
#ifdef WIN32
  for (int i=0; i<n; i++) SetPixel(fl_gc, p[i].x, p[i].y, fl_RGB());
#else
  if (n>1) XDrawPoints(fl_display, fl_window, fl_gc, p, n, 0);
#endif
}

void fl_end_line() {
#ifdef WIN32
  if (n>1) Polyline(fl_gc, p, n);
#else
  if (n>1) XDrawLines(fl_display, fl_window, fl_gc, p, n, 0);
#endif
}

static void fixloop() {  // remove equal points from closed path
  while (n>2 && p[n-1].x == p[0].x && p[n-1].y == p[0].y) n--;
}

void fl_end_loop() {
  fixloop();
  if (n>2) fl_transformed_vertex((COORD_T)p[0].x, (COORD_T)p[0].y);
  fl_end_line();
}

void fl_end_polygon() {
  fixloop();
#ifdef WIN32
  if (n>2) {
    SelectObject(fl_gc, fl_brush());
    Polygon(fl_gc, p, n);
  }
#else
  if (n>2) XFillPolygon(fl_display, fl_window, fl_gc, p, n, Convex, 0);
#endif
}

static int gap;
#ifdef WIN32
static int counts[20];
static int numcount;
#endif

void fl_begin_complex_polygon() {
  fl_begin_polygon();
  gap = 0;
#ifdef WIN32
  numcount = 0;
#endif
}

void fl_gap() {
  while (n>gap+2 && p[n-1].x == p[gap].x && p[n-1].y == p[gap].y) n--;
  if (n > gap+2) {
    fl_transformed_vertex((COORD_T)p[gap].x, (COORD_T)p[gap].y);
#ifdef WIN32
    counts[numcount++] = n-gap;
#endif
    gap = n;
  } else {
    n = gap;
  }
}

void fl_end_complex_polygon() {
  fl_gap();
#ifdef WIN32
  if (n>2) {
    SelectObject(fl_gc, fl_brush());
    PolyPolygon(fl_gc, p, counts, numcount);
  }
#else
  if (n>2) XFillPolygon(fl_display, fl_window, fl_gc, p, n, 0, 0);
#endif
}

// shortcut the closed circles so they use XDrawArc:
// warning: these do not draw rotated ellipses correctly!
// See fl_arc.c for portable version.

void fl_circle(double x, double y,double r) {
  double xt = fl_transform_x(x,y);
  double yt = fl_transform_y(x,y);
  double rx = r * (m.c ? sqrt(m.a*m.a+m.c*m.c) : fabs(m.a));
  double ry = r * (m.b ? sqrt(m.b*m.b+m.d*m.d) : fabs(m.d));
  int llx = int(xt-rx+.5);
  int w = int(xt+rx+.5)-llx;
  int lly = int(yt-ry+.5);
  int h = int(yt+ry+.5)-lly;
#ifdef WIN32
  if (what==POLYGON) {
    SelectObject(fl_gc, fl_brush());
    Pie(fl_gc, llx, lly, llx+w, lly+h, 0,0, 0,0); 
  } else
    Arc(fl_gc, llx, lly, llx+w, lly+h, 0,0, 0,0); 
#else
  (what == POLYGON ? XFillArc : XDrawArc)
    (fl_display, fl_window, fl_gc, llx, lly, w, h, 0, 360*64);
#endif
}

//
// End of "$Id: fl_vertex.cxx,v 1.5.2.3 2001/01/22 15:13:41 easysw Exp $".
//
