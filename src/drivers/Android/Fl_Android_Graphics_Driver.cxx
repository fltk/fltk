//
// "$Id$"
//
// Graphics routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2018 by Bill Spitzak and others.
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
#include "Fl_Android_Application.H"
#include "Fl_Android_Graphics_Driver.H"
#include "Fl_Android_Screen_Driver.H"
#include <FL/Fl.H>
#include <FL/platform.H>
#include <errno.h>
#include <math.h>


extern int fl_convert_pixmap(const char*const* cdata, uchar* out, Fl_Color bg);

static int sign(int v) { return (v<0) ? -1 : 1; }

/*
 * By linking this module, the following static method will instantiate the
 * Windows GDI Graphics driver as the main display driver.
 */
Fl_Graphics_Driver *Fl_Graphics_Driver::newMainGraphicsDriver()
{
  return new Fl_Android_Graphics_Driver();
}


/**
 * Private default constructor.
 */
Fl_Android_Graphics_Driver::Fl_Android_Graphics_Driver() :
        super()
{
}


Fl_Android_Graphics_Driver::~Fl_Android_Graphics_Driver()
{
}


void Fl_Android_Graphics_Driver::make_current(Fl_Window *win)
{
  // In the special case of win==0, we activate the desktop (the screen
  // background) for drawing and clip out all visible windows

  // The Stride is the offset between lines in the graphics buffer
  pStride = Fl_Android_Application::graphics_buffer().stride;
  // Bits is the memory address of the top left corner of the window
  pBits = ((uint16_t*)(Fl_Android_Application::graphics_buffer().bits));
  if (win) pBits += win->x_root() + pStride * win->y_root();

  // TODO: set the clipping area
  // set the clipping area to the physical screen size in window coordinates
  if (win) {
    pWindowRegion.set(-win->x(), -win->y(), 600, 800);
    pWindowRegion.intersect_with(Fl_Rect_Region(0, 0, win->w(), win->h()));
  } else {
    pWindowRegion.set(0, 0, 600, 800);
  }

  pDesktopWindowRegion.set(pWindowRegion);

  // remove all window rectangles that are positioned on top of this window
  // TODO: this region is expensive to calculate. Cache it for each window and recalculate when windows move, show, hide, or change order
  // TODO: this is where we also need to subtract any possible window decoration, like the window title and drag bar, resizing edges, etc.
  Fl_Window *wTop = Fl::first_window();
  int wx = win ? win->x() : 0;
  int wy = win ? win->y() : 0;
  while (wTop) {
    if (wTop==win) break;
    Fl_Rect_Region r(wTop->x()-wx, wTop->y()-wy, wTop->w(), wTop->h());
    pDesktopWindowRegion.subtract(r);
    wTop = Fl::next_window(wTop);
  }
  pClippingRegion.set(pDesktopWindowRegion);
}


uint16_t Fl_Android_Graphics_Driver::make565(uchar red,  uchar green, uchar blue)
{
    return (uint16_t)( ((((uint16_t)(red))   << 8) & 0xf800) |
                       ((((uint16_t)(green)) << 3) & 0x07e0) |
                       ((((uint16_t)(blue))  >> 3) & 0x001f) );
}

extern unsigned fl_cmap[256];


uint16_t Fl_Android_Graphics_Driver::make565(Fl_Color crgba)
{
  if (crgba<0x00000100) crgba = fl_cmap[crgba];
    return (uint16_t)( ((crgba >> 16) & 0xf800) |
                       ((crgba >> 13) & 0x07e0) |
                       ((crgba >> 11) & 0x001f) );
}


void Fl_Android_Graphics_Driver::rectf(int x, int y, int w, int h)
{
  for (const auto &it: pClippingRegion.overlapping(Fl_Rect_Region(x, y, w, h))) {
    Fl_Rect_Region &s = it->clipped_rect();
    rectf_unclipped(s.x(), s.y(), s.w(), s.h());
  }
}


void Fl_Android_Graphics_Driver::rectf_unclipped(int x, int y, int w, int h)
{
  if (w<=0 || h<=0) return;

  uint16_t cc = make565(color());
  int32_t ss = pStride;
  uint16_t *bits = pBits;
  uint32_t xx = (uint32_t)x;
  uint32_t yy = (uint32_t)y;
  uint32_t ww = (uint32_t)w;
  uint32_t hh = (uint32_t)h;
  for (uint32_t iy = 0; iy<hh; ++iy) {
    uint16_t *d = bits + (iy+yy)*ss + xx;
    for (uint32_t ix = ww; ix>0; --ix) {
      *d++ = cc;
    }
  }
}


void Fl_Android_Graphics_Driver::xyline(int x, int y, int x1)
{
  float w;
  if (x1>x) {
    w = x1-x;
  } else {
    w = x-x1;
    x = x1;
  }
  for (const auto &it: pClippingRegion.overlapping(Fl_Rect_Region(x, y, w, 1))) {
    Fl_Rect_Region &s = it->clipped_rect();
    xyline_unclipped(s.x(), s.y(), s.right());
  }
}


void Fl_Android_Graphics_Driver::xyline(int x, int y, int x1, int y2)
{
  xyline(x, y, x1);
  yxline(x1, y, y2);
}


void Fl_Android_Graphics_Driver::xyline(int x, int y, int x1, int y2, int x3)
{
  xyline(x, y, x1);
  yxline(x1, y, y2);
  xyline(x1, y2, x3);
}


void Fl_Android_Graphics_Driver::xyline_unclipped(int x, int y, int x1)
{
  uint16_t cc = make565(color());
  float w;
  if (x1>x) {
    w = x1-x+1;
  } else {
    w = x-x1+1;
    x = x1;
  }
  int32_t sx = 1;
  int32_t ss = pStride;
  uint16_t *bits = pBits;
  uint32_t xx = (uint32_t)x;
  uint32_t yy = (uint32_t)y;
  uint32_t ww = (uint32_t)w;
  uint16_t *d = bits + yy*ss + xx;
  if ((pLineStyle&0xff)==FL_DOT) { ww = ww/2; sx = sx*2; }
  for (uint32_t ix = ww; ix>0; --ix) {
    *d = cc;
    d+=sx;
  }
}


void Fl_Android_Graphics_Driver::yxline(int x, int y, int y1)
{
  float h;
  if (y1>y) {
    h = y1-y+1;
  } else {
    h = y-y1+1;
    y = y1;
  }
  for (const auto &it: pClippingRegion.overlapping(Fl_Rect_Region(x, y, 1, h))) {
    Fl_Rect_Region &s = it->clipped_rect();
    yxline_unclipped(s.x(), s.y(), s.bottom());
  }
}


void Fl_Android_Graphics_Driver::yxline(int x, int y, int y1, int x2)
{
  yxline(x, y, y1);
  xyline(x, y1, x2);
}


void Fl_Android_Graphics_Driver::yxline(int x, int y, int y1, int x2, int y3)
{
  yxline(x, y, y1);
  xyline(x, y1, x2);
  yxline(x2, y1, y3);
}


void Fl_Android_Graphics_Driver::yxline_unclipped(int x, int y, int y1)
{
  uint16_t cc = make565(color());
  float h = y1-y;
  int32_t ss = pStride;
  uint16_t *bits = pBits;
  uint32_t xx = (uint32_t)x;
  uint32_t yy = (uint32_t)y;
  uint32_t hh = (uint32_t)h;
  uint16_t *d = bits + yy*ss + xx;
  if ((pLineStyle&0xff)==FL_DOT) { hh = hh/2; ss = ss*2; }
  for (uint32_t iy = hh; iy>0; --iy) {
    *d = cc;
    d += ss;
  }
}


void Fl_Android_Graphics_Driver::rect(int x, int y, int w, int h)
{
  xyline(x, y, x+w-1);
  yxline(x, y, y+h-1);
  yxline(x+w-1, y, y+h-1);
  xyline(x, y+h-1, x+w-1);
}


void Fl_Android_Graphics_Driver::line_style(int style, int width, char* dashes)
{
  pLineStyle = style;
  // TODO: finish this!
}

/**
 * Draw a single dot in the current color.
 * @param x, y position relative to window.
 */
void Fl_Android_Graphics_Driver::point(int x, int y)
{
  // drawing a single point is insanely inefficient because we need to walk the
  // entire clipping region every time to see if the point needs to be drawn.
  for (const auto &it: pClippingRegion.overlapping(Fl_Rect_Region(x, y, 1, 1))) {
    Fl_Rect_Region &s = it->clipped_rect();
    uint16_t cc = make565(color());
    int32_t ss = pStride;
    uint16_t *bits = pBits;
    uint32_t xx = (uint32_t)x;
    uint32_t yy = (uint32_t)y;
    uint16_t *d = bits + yy*ss + xx;
    *d = cc;
  }

}

/**
 * Draw a line.
 * FIXME: it is incredibly inefficient to call 'point', especially for long lines
 * FIXME: clipping maust be moved into this call and drawing to the screen should happen right here
 * FIXME: line width is not considered
 */
void Fl_Android_Graphics_Driver::line(int x, int y, int x1, int y1)
{
  if (x==x1) {
    return yxline(x, y, y1);
  }
  if (y==y1) {
    return xyline(x, y, x1);
  }
  // Bresenham
  int w = x1 - x, dx = abs(w);
  int h = y1 - y, dy = abs(h);
  int dx1 = sign(w), dy1 = sign(h), dx2, dy2;
  int min, max;
  if (dx < dy) {
    min = dx; max = dy;
    dx2 = 0;
    dy2 = dy1;
  } else {
    min = dy; max = dx;
    dx2 = dx1;
    dy2 = 0;
  }
  int num = max/2;
  for (int i=max+1; i>0; i--) {
    point(x, y);
    num += min;
    if (num>=max) {
      num -= max;
      x += dx1;
      y += dy1;
    } else {
      x += dx2;
      y += dy2;
    }
  }
}


void Fl_Android_Graphics_Driver::line(int x, int y, int x1, int y1, int x2, int y2)
{
  begin_line();
  transformed_vertex(x, y);
  transformed_vertex(x1, y1);
  transformed_vertex(x2, y2);
  end_line();
}


void Fl_Android_Graphics_Driver::loop(int x0, int y0, int x1, int y1, int x2, int y2)
{
  begin_loop();
  transformed_vertex(x0, y0);
  transformed_vertex(x1, y1);
  transformed_vertex(x2, y2);
  end_loop();
}


void Fl_Android_Graphics_Driver::loop(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3)
{
  begin_loop();
  transformed_vertex(x0, y0);
  transformed_vertex(x1, y1);
  transformed_vertex(x2, y2);
  transformed_vertex(x3, y3);
  end_loop();
}


void Fl_Android_Graphics_Driver::polygon(int x0, int y0, int x1, int y1, int x2, int y2)
{
  begin_polygon();
  transformed_vertex(x0, y0);
  transformed_vertex(x1, y1);
  transformed_vertex(x2, y2);
  end_polygon();
}


void Fl_Android_Graphics_Driver::polygon(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3)
{
  begin_polygon();
  transformed_vertex(x0, y0);
  transformed_vertex(x1, y1);
  transformed_vertex(x2, y2);
  transformed_vertex(x3, y3);
  end_polygon();
}


/**
 * Reset the vertex counter to zero.
 */
void Fl_Android_Graphics_Driver::begin_vertices()
{
  pnVertex = n = 0;
  pVertexGapStart = 0;
}

/**
 * Add a vertex to the vertex list. Dynamically allocates memory.
 * @param x, y position of the vertex after matrix transformation
 * @param gap line and loop call offer to leave a gap in the drawing
 */
void Fl_Android_Graphics_Driver::add_vertex(float x, float y, bool gap)
{
  if (pnVertex == pNVertex) {
    pNVertex += 16;
    pVertex = (Vertex*)::realloc(pVertex, pNVertex*sizeof(Vertex));
  }
  pVertex[pnVertex].set(x, y);
  pVertex[pnVertex].pIsGap = gap;
  pnVertex++; n = pnVertex;
}

/**
 * Start a list of vertices to draw multiple points.
 */
void Fl_Android_Graphics_Driver::begin_points()
{
  begin_vertices();
  super::begin_points();
}

/**
 * Start a list of vertices to draw a polyline.
 */
void Fl_Android_Graphics_Driver::begin_line()
{
  begin_vertices();
  super::begin_line();
}

/**
 * Start a list of vertices to draw a line loop.
 */
void Fl_Android_Graphics_Driver::begin_loop()
{
  begin_vertices();
  super::begin_loop();
}

/**
 * Start a list of vertices to draw a polygon.
 */
void Fl_Android_Graphics_Driver::begin_polygon()
{
  begin_vertices();
  super::begin_polygon();
}

/**
 * Start a list of vertices to draw a complex polygon.
 */
void Fl_Android_Graphics_Driver::begin_complex_polygon()
{
  begin_vertices();
  super::begin_complex_polygon();
}

/**
 * Draw all stored vertices as points.
 */
void Fl_Android_Graphics_Driver::end_points()
{
  for (int i=0; i<pnVertex; ++i) {
    Vertex &v = pVertex[i];
    if (!v.pIsGap)
      point(v.pX, v.pY);
  }
}

/**
 * Draw all stored vertices as a polyline.
 */
void Fl_Android_Graphics_Driver::end_line()
{
  Vertex &v1 = pVertex[0];
  for (int i=1; i<pnVertex; ++i) {
    Vertex &v2 = pVertex[i];
    if (!v1.pIsGap && !v2.pIsGap)
      line(v1.pX, v1.pY, v2.pX, v2.pY);
    v1 = v2;
  }
}

/**
 * Draw all stored vertices as a polyline loop.
 */
void Fl_Android_Graphics_Driver::end_loop()
{
  gap();
  Vertex &v1 = pVertex[0];
  for (int i=1; i<pnVertex; ++i) {
    Vertex &v2 = pVertex[i];
    if (!v1.pIsGap)
      line(v1.pX, v1.pY, v2.pX, v2.pY);
    v1 = v2;
  }
}

/**
 * Draw all stored vertices as a polygon.
 * FIXME: these calls are very ineffiecient. Avoid pointer lookup.
 * FIXME: use the current clipping rect to accelerate rendering
 * FIXME: unmix float and int
 */
void Fl_Android_Graphics_Driver::end_polygon(int begin, int end)
{
  if (end - begin < 2) return;

  Vertex *v = pVertex+0;
  int xMin = v->pX, xMax = xMin, yMin = v->pY, yMax = yMin;
  for (int i = begin+1; i < end; i++) {
    v = pVertex+i;
    if (v->pX < xMin) xMin = v->pX;
    if (v->pX > xMax) xMax = v->pX;
    if (v->pY < yMin) yMin = v->pY;
    if (v->pY > yMax) yMax = v->pY;
  }
  xMax++; yMax++;

  int nodes, nodeX[end - begin], pixelX, pixelY, i, j, swap;

  //  Loop through the rows of the image.
  for (pixelY = yMin; pixelY < yMax; pixelY++) {
    //  Build a list of nodes.
    nodes = 0;
    j = begin;
    for (i = begin+1; i < end; i++) {
      if (   (pVertex[i].pY < pixelY && pVertex[j].pY >= pixelY)
          || (pVertex[j].pY < pixelY && pVertex[i].pY >= pixelY))
      {
        float dy = pVertex[j].pY - pVertex[i].pY;
        if (fabsf(dy)>.0001) {
          nodeX[nodes++] = (int)(pVertex[i].pX +
                                 (pixelY - pVertex[i].pY) / dy
                                 * (pVertex[j].pX - pVertex[i].pX));
        } else {
          nodeX[nodes++] = pVertex[i].pX;
        }
      }
      j = i;
    }

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
      if (nodeX[i] >= xMax) break;
      if (nodeX[i + 1] > xMin) {
        if (nodeX[i] < xMin) nodeX[i] = xMin;
        if (nodeX[i + 1] > xMax) nodeX[i + 1] = xMax;
        xyline(nodeX[i], pixelY, nodeX[i + 1]);
      }
    }
  }

}

/**
 * Draw all stored vertices as a polygon.
 * Mind the gap!
 */
void Fl_Android_Graphics_Driver::end_polygon()
{
  if (pnVertex==0) return;
  gap();
  int start = 0, end = 0;
  for (int i=0; i<pnVertex; i++) {
    if (pVertex[i].pIsGap) {
      end = i+1;
      end_polygon(start, end);
      start = end;
      i++;
    }
  }
}

/**
 * Draw all stored vertices as a possibly self-intersecting polygon.
 * FIXME: these calls are very ineffiecient. Avoid pointer lookup.
 * FIXME: use the current clipping rect to accelerate rendering
 * FIXME: unmix float and int
 */
void Fl_Android_Graphics_Driver::end_complex_polygon()
{
  if (pnVertex < 2) return;

  gap(); // adds the first coordinate of this loop and marks it as a gap
  int begin = 0, end = pnVertex;

  Vertex *v = pVertex+0;
  int xMin = v->pX, xMax = xMin, yMin = v->pY, yMax = yMin;
  for (int i = begin+1; i < end; i++) {
    v = pVertex+i;
    if (v->pX < xMin) xMin = v->pX;
    if (v->pX > xMax) xMax = v->pX;
    if (v->pY < yMin) yMin = v->pY;
    if (v->pY > yMax) yMax = v->pY;
  }
  xMax++; yMax++;

  int nodes, nodeX[end - begin], pixelX, pixelY, i, j, swap;

  //  Loop through the rows of the image.
  for (pixelY = yMin; pixelY < yMax; pixelY++) {
    //  Build a list of nodes.
    nodes = 0;
    for (i = begin+1; i < end; i++) {
      j = i-1;
      if (pVertex[j].pIsGap)
        continue;
      if (   (pVertex[i].pY < pixelY && pVertex[j].pY >= pixelY)
          || (pVertex[j].pY < pixelY && pVertex[i].pY >= pixelY) )
      {
        float dy = pVertex[j].pY - pVertex[i].pY;
        if (fabsf(dy)>.0001) {
          nodeX[nodes++] = (int)(pVertex[i].pX +
                                 (pixelY - pVertex[i].pY) / dy
                                 * (pVertex[j].pX - pVertex[i].pX));
        } else {
          nodeX[nodes++] = pVertex[i].pX;
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
      if (nodeX[i] >= xMax) break;
      if (nodeX[i + 1] > xMin) {
        if (nodeX[i] < xMin) nodeX[i] = xMin;
        if (nodeX[i + 1] > xMax) nodeX[i + 1] = xMax;
        xyline(nodeX[i], pixelY, nodeX[i + 1]);
      }
    }
  }
}

/**
 * Add a gap to a polyline drawing
 */
void Fl_Android_Graphics_Driver::gap()
{
  // drop gaps at the start or gap after gap
  if (pnVertex==0 || pnVertex==pVertexGapStart)
    return;

  // create a loop
  Vertex &v = pVertex[pVertexGapStart];
  add_vertex(v.pX, v.pY, true);
  pVertexGapStart = pnVertex;
}

/**
 * Add a vertex to the list.
 * TODO: we should maintain a bounding box for faster clipping.
 */
void Fl_Android_Graphics_Driver::transformed_vertex(double x, double y)
{
  add_vertex(x, y);
}


void Fl_Android_Graphics_Driver::vertex(double x,double y)
{
  transformed_vertex(x*m.a + y*m.c + m.x, x*m.b + y*m.d + m.y);
}


/**
 * Draw an arc.
 * @param xi
 * @param yi
 * @param w
 * @param h
 * @param a1
 * @param a2
 * FIXME: float-to-int interpolation is horrible!
 */
void Fl_Android_Graphics_Driver::arc(int xi, int yi, int w, int h, double a1, double a2)
{
  if (a2<=a1) return;

  double rx = w/2.0;
  double ry = h/2.0;
  double x = xi + rx;
  double y = yi + ry;
  double circ = M_PI*0.5*(rx+ry);
  int i, segs = circ * (a2-a1) / 1000;  // every line is about three pixels long
  if (segs<3) segs = 3;

  int px, py;
  a1 = a1/180*M_PI;
  a2 = a2/180*M_PI;
  double step = (a2-a1)/segs;

  int nx = x + cos(a1)*rx;
  int ny = y - sin(a1)*ry;
  for (i=segs; i>0; i--) {
    a1+=step;
    px = nx; py = ny;
    nx = x + cos(a1)*rx;
    ny = y - sin(a1)*ry;
    line(px, py, nx, ny);
  }
}

/**
 * Draw a piece of a pie.
 * FIXME: this is not working very well at all.
 * @param xi
 * @param yi
 * @param w
 * @param h
 * @param b1
 * @param b2
 */
void Fl_Android_Graphics_Driver::pie(int xi, int yi, int w, int h, double b1, double b2)
{
  // quick access to bounding box size
  double rx = w / 2.0;
  double ry = h / 2.0;
  double x = xi + rx;
  double y = yi + ry;


  double a1 = b1 / 180 * M_PI;
  double a2 = b2 / 180 * M_PI;

  // invert to make b1 always the smaller value
  if (b1 > b2) {
    b1 -= 360.0;
  }
  if (b1 == b2) return;

  // make the top the zero degree origin, turning CCW
  b1 -= 90.0;
  b2 -= 90.0;

  // find the delta between angles
  double delta = b2 - b1;
  if (delta >= 360.0) {
    b1 = 0.0;
    b2 = 360.0;
    delta = 360.0;
  }

  // make sure that b2 is always in the range [0.0..360.0]
  if (b2 > 360.0) b2 -= 360.0; // FIXME: fmod(...)
  if (b2 < 0.0) b2 += 360.0;
  b1 = b2 - delta;
  // now b1 is [-360...360] and b2 is [0..360] and b1<b2;

  a1 = b1 / 180 * M_PI;
  a2 = b2 / 180 * M_PI;
  double b1o = b1;
  bool flipped = false;
  if (a1<0.0) { a1 += 2*M_PI; b1 += 360.0; flipped = true; }

//  Fl_Android_Application::log_e(" %g %g %d", b1, b2, flipped);

  double a1Slope = tan(a1);
  double a2Slope = tan(a2);

  // draw the pie line by line
  for (double iy = y - ry; iy <= y + ry; iy++) {
    double a = acos((iy - y) / ry);
    double aL = M_PI - a; // 0..PI
    double aR = a + M_PI; // 2PI..PI
    double sinALrx = sin(aL)*rx;

//    fl_color(FL_RED);

    if (aL<0.5*M_PI) {
      // rasterize top left quadrant
      bool loInside = false, hiInside = false;
      double loLeft = 0.0, loRight = 0.0;
      double hiLeft = 0.0, hiRight = 0.0;
      if (b1 >= 0 && b1 < 90) {
        loInside = true;
        loLeft = -sinALrx;
        loRight = a1Slope * (iy - y);
      }
      if (b2 >= 0 && b2 < 90) {
        hiInside = true;
        if (aL < a2)
          hiLeft = -sinALrx;
        else
          hiLeft = a2Slope * (iy - y);
      }
      if (loInside && hiInside && !flipped) {
//        fl_color(FL_GREEN);
        if (a1 < aL)
          xyline(x + hiLeft, iy, x + loRight);
      } else {
        if ((!loInside) && (!hiInside)) {
//          fl_color(FL_MAGENTA);
          if ( (b1o<=0.0 && b2>=90.0) || (b1o<=(0.0-360.0) && b2>=(90.0-360.0)) )
            xyline(x - sinALrx, iy, x);
        } else {
          if (loInside) {
//            fl_color(FL_BLUE);
            if (a1 < aL)
              xyline(x + loLeft, iy, x + loRight);
          }
          if (hiInside) {
//            fl_color(FL_YELLOW);
            xyline(x + hiLeft, iy, x);
          }
        }
      }
    } else {
      // rasterize bottom left quadrant
      bool loInside = false, hiInside = false;
      double loLeft = 0.0, loRight = 0.0;
      double hiLeft = 0.0, hiRight = 0.0;
      if (b1 >= 90 && b1 < 180) {
        loInside = true;
        if (aL>=a1)
          loLeft = -sinALrx;
        else
          loLeft = a1Slope * (iy - y);
      }
      if (b2 >= 90 && b2 < 180) {
        hiInside = true;
        hiLeft = -sinALrx;
        hiRight = a2Slope * (iy - y);
      }
      if (loInside && hiInside && !flipped) {
//        fl_color(FL_GREEN);
        if (a2 > aL)
          xyline(x + loLeft, iy, x + hiRight);
      } else {
        if ((!loInside) && (!hiInside)) {
//          fl_color(FL_MAGENTA);
          if ( (b1o<=90.0 && b2>=180.0) || (b1o<=(90.0-360.0) && b2>=(180.0-360.0)) )
            xyline(x - sinALrx, iy, x);
        } else {
          if (loInside) {
//            fl_color(FL_BLUE);
            xyline(x + loLeft, iy, x);
          }
          if (hiInside) {
//            fl_color(FL_YELLOW);
            if (a2 > aL)
              xyline(x + hiLeft, iy, x + hiRight);
          }
        }
      }
    }
    if (aR<1.5*M_PI) {
      // rasterize bottom right quadrant
      bool loInside = false, hiInside = false;
      double loLeft = 0.0, loRight = 0.0;
      double hiLeft = 0.0, hiRight = 0.0;
      if (b1 >= 180 && b1 < 270) {
        loInside = true;
        loLeft = sinALrx;
        loRight = a1Slope * (iy - y);
      }
      if (b2 >= 180 && b2 < 270) {
        hiInside = true;
        if (aR < a2)
          hiLeft = sinALrx;
        else
          hiLeft = a2Slope * (iy - y);
      }
      if (loInside && hiInside && !flipped) {
//        fl_color(FL_GREEN);
        if (a1 < aR)
          xyline(x + hiLeft, iy, x + loRight);
      } else {
        if ((!loInside) && (!hiInside)) {
//          fl_color(FL_MAGENTA);
          if ( (b1o<=180.0 && b2>=270.0) || (b1o<=(180.0-360.0) && b2>=(270.0-360.0)) )
            xyline(x + sinALrx, iy, x);
        } else {
          if (loInside) {
//            fl_color(FL_BLUE);
            if (a1 < aR)
              xyline(x + loLeft, iy, x + loRight);
          }
          if (hiInside) {
//            fl_color(FL_YELLOW);
            xyline(x + hiLeft, iy, x);
          }
        }
      }
    } else {
      // rasterize top right quadrant
      bool loInside = false, hiInside = false;
      double loLeft = 0.0, loRight = 0.0;
      double hiLeft = 0.0, hiRight = 0.0;
      if (b1 >= 270 && b1 < 360) {
        loInside = true;
        if (aR>=a1)
          loLeft = sinALrx;
        else
          loLeft = a1Slope * (iy - y);
      }
      if (b2 >= 270 && b2 < 360) {
        hiInside = true;
        hiLeft = sinALrx;
        hiRight = a2Slope * (iy - y);
      }
      if (loInside && hiInside && !flipped) {
//        fl_color(FL_GREEN);
        if (a2 > aR)
          xyline(x + loLeft, iy, x + hiRight);
      } else {
        if ((!loInside) && (!hiInside)) {
//          fl_color(FL_MAGENTA);
          if ( (b1o<=270.0 && b2>=360.0) || (b1o<=(270.0-360.0) && b2>=(360.0-360.0)) )
            xyline(x + sinALrx, iy, x);
        } else {
          if (loInside) {
//            fl_color(FL_BLUE);
            xyline(x + loLeft, iy, x);
          }
          if (hiInside) {
//            fl_color(FL_YELLOW);
            if (a2 > aR)
              xyline(x + hiLeft, iy, x + hiRight);
          }
        }
      }
    }
  }
}

/**
 * FIXME: these do not draw rotated ellipses correctly!
 * FIXME: use floating point version of arc and pie?!
 * */
void Fl_Android_Graphics_Driver::ellipse(double xt, double yt, double rx, double ry)
{
  int llx = xt-rx;
  int w = xt+rx-llx;
  int lly = yt-ry;
  int h = yt+ry-lly;

  if (what==POLYGON)
    pie(llx, lly, w, h, 0.0, 360.0);
  else
    arc(llx, lly, w, h, 0.0, 360.0);
}


void Fl_Android_Graphics_Driver::circle(double x, double y, double r)
{
  double xt = transform_x(x,y);
  double yt = transform_y(x,y);
  double rx = r * (m.c ? sqrt(m.a*m.a+m.c*m.c) : fabs(m.a));
  double ry = r * (m.b ? sqrt(m.b*m.b+m.d*m.d) : fabs(m.d));
  ellipse(xt, yt, rx, ry);
}


void Fl_Android_Graphics_Driver::draw_fixed(Fl_Pixmap * pxm, int X, int Y, int W, int H, int cx, int cy)
{
  if (*Fl_Graphics_Driver::id(pxm)) {
    Fl_Android_565A_Map *cache = (Fl_Android_565A_Map*)*Fl_Graphics_Driver::id(pxm);
    for (const auto &it: pClippingRegion.overlapping(Fl_Rect_Region(X, Y, W, H))) {
      draw(X-cx, Y-cy, cache, it->clipped_rect());
    }
  }
}


void Fl_Android_Graphics_Driver::draw_fixed(Fl_Bitmap *bm, int X, int Y, int W, int H, int cx, int cy)
{
  if (*Fl_Graphics_Driver::id(bm)) {
    Fl_Android_Bytemap *cache = (Fl_Android_Bytemap*)*Fl_Graphics_Driver::id(bm);
    for (const auto &it: pClippingRegion.overlapping(Fl_Rect_Region(X, Y, W, H))) {
      draw(X-cx, Y-cy, cache, it->clipped_rect());
    }
  }
}


void Fl_Android_Graphics_Driver::cache(Fl_Bitmap *bm)
{
  int w = bm->w(), h = bm->h();
  int rowBytes = (w+7)>>3;

  Fl_Android_Bytemap *cache = new Fl_Android_Bytemap(w, h);
  for (int yy=0; yy<w; yy++) {
    const uchar *src = bm->array + yy*rowBytes;
    uchar *dst = cache->pBytes + yy*cache->pStride;
    uchar d = 0;
    for (int xx=0; xx<w; xx++) {
      if ((xx&7)==0) d = *src++;
      if (d&1) *dst = 0xff; else *dst = 0;
      dst++;
      d >>= 1;
    }
  }

  *Fl_Graphics_Driver::id(bm) = (fl_uintptr_t)cache;
  int *pw, *ph;
  cache_w_h(bm, pw, ph);
  *pw = bm->data_w();
  *ph = bm->data_h();
}

void Fl_Android_Graphics_Driver::delete_bitmask(Fl_Bitmask bm)
{
  delete (Fl_Android_Bytemap*)bm;
}

void Fl_Android_Graphics_Driver::cache(Fl_Pixmap *img)
{
  int w = img->w(), h = img->h();
  int rowBytes = 4*w;
  uchar *rgba = (uchar*)calloc(w*h, 4);
  int ret = fl_convert_pixmap(img->data(), rgba, 0);
  if (ret==0) {
    ::free(rgba);
    *Fl_Graphics_Driver::id(img) = 0;
    return;
  }

  Fl_Android_565A_Map *cache = new Fl_Android_565A_Map(w, h);
  for (int yy=0; yy<w; yy++) {
    const uchar *src = rgba + yy*rowBytes;
    uint32_t *dst = cache->pWords + yy*cache->pStride;
    for (int xx=0; xx<w; xx++) {
//      uint32_t c = ((((src[0] << 8) & 0xf800) |
//                     ((src[1] << 3) & 0x07e0) |
//                     ((src[2] >> 3) & 0x001f) ) << 16) | src[3]; // FIXME: alpha
      *dst++ = Fl_Android_565A_Map::toRGBA(src[0],src[1], src[2], src[3]);
      src+=4;
    }
  }

  ::free(rgba);
  *Fl_Graphics_Driver::id(img) = (fl_uintptr_t)cache;
  int *pw, *ph;
  cache_w_h(img, pw, ph);
  *pw = img->data_w();
  *ph = img->data_h();
}


void Fl_Android_Graphics_Driver::uncache_pixmap(fl_uintptr_t p)
{
  Fl_Android_565A_Map *img = (Fl_Android_565A_Map*)p;
  delete img;
}

void Fl_Android_Graphics_Driver::cache(Fl_RGB_Image *img)
{
  int w = img->data_w(), h = img->data_h(), d = img->d(), stride = w*d + img->ld();
  Fl_Android_565A_Map *cgimg = new Fl_Android_565A_Map(w, h);
  *Fl_Graphics_Driver::id(img) = (fl_uintptr_t)cgimg;
  int *pw, *ph;
  cache_w_h(img, pw, ph);
  *pw = img->data_w();
  *ph = img->data_h();
  if (d==1) { // grayscale
    for (int iy=0; iy<h; iy++) {
      const uchar *src = img->array + iy*stride;
      uint32_t *dst = cgimg->pWords + iy*cgimg->pStride;
      for (int ix=0; ix<w; ix++) {
        uchar l = *src++;
        uint32_t rgba = Fl_Android_565A_Map::toRGBA(l, l, l, 255);
        *dst++ = rgba;
      }
    }
  } else if (d==2) { // gray + alpha
    for (int iy=0; iy<h; iy++) {
      const uchar *src = img->array + iy*stride;
      uint32_t *dst = cgimg->pWords + iy*cgimg->pStride;
      for (int ix=0; ix<w; ix++) {
        uchar l = *src++, a = *src++;
        uint32_t rgba = Fl_Android_565A_Map::toRGBA(l, l, l, a);
        *dst++ = rgba;
      }
    }
  } else if (d==3) { // rgb
    for (int iy=0; iy<h; iy++) {
      const uchar *src = img->array + iy*stride;
      uint32_t *dst = cgimg->pWords + iy*cgimg->pStride;
      for (int ix=0; ix<w; ix++) {
        uchar r = *src++, g = *src++, b = *src++;
        uint32_t rgba = Fl_Android_565A_Map::toRGBA(r, g, b, 255);
        *dst++ = rgba;
      }
    }
  } else if (d==4) { // rgb + alpha
    for (int iy=0; iy<h; iy++) {
      const uchar *src = img->array + iy*stride;
      uint32_t *dst = cgimg->pWords + iy*cgimg->pStride;
      for (int ix=0; ix<w; ix++) {
        uchar r = *src++, g = *src++, b = *src++, a = *src++;
        uint32_t rgba = Fl_Android_565A_Map::toRGBA(r, g, b, a);
        *dst++ = rgba;
      }
    }
  }
}

void Fl_Android_Graphics_Driver::draw_fixed(Fl_RGB_Image *img, int X, int Y, int W, int H, int cx, int cy)
{
  Fl_Android_565A_Map *cgimg = (Fl_Android_565A_Map*)*Fl_Graphics_Driver::id(img);
  if (cgimg) {
    for (const auto &it: pClippingRegion.overlapping(Fl_Rect_Region(X, Y, W, H))) {
      draw(X-cx, Y-cy, cgimg, it->clipped_rect());
    }
  }
}


/**
 * Copy RGB (or RGBA?) image data directly onto the surface.
 * TODO: I did not find documentation on the possible values of D. If D is four, does that
 * mean that the fourth value must be an alpha value, and should that be applied here?
 * What does a negative D indicate?
 */
void Fl_Android_Graphics_Driver::draw_image(const uchar* buf, int X,int Y,int W,int H, int D, int L)
{
  int srcDelta = abs(D);
  int srcStride = L ? L : W*srcDelta;
  for (const auto &it: pClippingRegion.overlapping(Fl_Rect_Region(X, Y, W, H))) {
    Fl_Rect_Region *r = &it->clipped_rect();
    int rBottom = r->bottom();
    int rRight = r->right();
    for (int iy=r->top(); iy<rBottom;iy++) {
      const uchar *src = buf + (iy-Y)*srcStride + (r->left()-X)*srcDelta;
      uint16_t *dst = pBits + iy*pStride + r->left();
      for (int ix=r->left();ix<rRight;ix++) {
        uint16_t c = make565(src[0], src[1], src[2]);
        src += srcDelta;
        *dst++ = c;
      }
    }
  }
}

/**
 * Copy RGB (or RGBA?) image data directly onto the surface.
 * TODO: I did not find documentation on the possible values of D. If D is four, does that
 * mean that the fourth value must be an alpha value, and should that be applied here?
 * What does a negative D indicate?
 */
void Fl_Android_Graphics_Driver::draw_image_mono(const uchar* buf, int X,int Y,int W,int H, int D, int L)
{
  int srcDelta = abs(D);
  int srcStride = W*srcDelta+L;
  for (const auto &it: pClippingRegion.overlapping(Fl_Rect_Region(X, Y, W, H))) {
    Fl_Rect_Region *r = &it->clipped_rect();
    int rBottom = r->bottom();
    int rRight = r->right();
    for (int iy=r->top(); iy<rBottom;iy++) {
      const uchar *src = buf + iy*srcStride;
      uint16_t *dst = pBits + iy*pStride + r->left();
      for (int ix=r->left();ix<rRight;ix++) {
        uchar l = src[0];
        uint16_t c = make565(l, l, l);
        src += srcDelta;
        *dst++ = c;
      }
    }
  }
}

/*
 * Draw some graphics line-by-line directly onto this surface
 * TODO: I did not find documentation on the possible values of D. If D is four, does that
 * mean that the fourth value must be an alpha value, and should that be applied here?
 */
void Fl_Android_Graphics_Driver::draw_image(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D)
{
  int srcDelta = abs(D);
  for (const auto &it: pClippingRegion.overlapping(Fl_Rect_Region(X, Y, W, H))) {
    Fl_Rect_Region *r = &it->clipped_rect();
    uchar *buf = (uchar*)malloc(size_t(srcDelta*r->w()));
    int rBottom = r->bottom();
    int rRight = r->right();
    for (int iy=r->top(); iy<rBottom;iy++) {
      cb(data, r->left()-X, iy-Y, r->w(), buf);
      uchar *src = buf;
      uint16_t *dst = pBits + iy*pStride + r->left();
      for (int ix=r->left();ix<rRight;ix++) {
        uint16_t c = make565(src[0], src[1], src[2]);
        src += srcDelta;
        *dst++ = c;
      }
    }
    free(buf);
  }
}

/*
 * Draw some graphics line-by-line directly onto this surface
 * TODO: I did not find documentation on the possible values of D. If D is two, does that
 * mean that the fourth value must be an alpha value, and should that be applied here?
 * If it is three, doe we need to convert RGB to grayscale?
 * What exactly does a negative value mean? Where is this all documented? Sigh.
 */
void Fl_Android_Graphics_Driver::draw_image_mono(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D)
{
  int srcDelta = abs(D);
  for (const auto &it: pClippingRegion.overlapping(Fl_Rect_Region(X, Y, W, H))) {
    Fl_Rect_Region *r = &it->clipped_rect();
    uchar *buf = (uchar*)malloc(size_t(srcDelta*r->w()));
    int rBottom = r->bottom();
    int rRight = r->right();
    for (int iy=r->top(); iy<rBottom;iy++) {
      cb(data, r->left()-X, iy-Y, r->w(), buf);
      uchar *src = buf;
      uint16_t *dst = pBits + iy*pStride + r->left();
      for (int ix=r->left();ix<rRight;ix++) {
        uchar l = src[0];
        uint16_t c = make565(l, l, l);
        src += srcDelta;
        *dst++ = c;
      }
    }
    free(buf);
  }
}


void Fl_Android_Graphics_Driver::uncache(Fl_RGB_Image*, fl_uintptr_t &id_, fl_uintptr_t&)
{
  Fl_Android_565A_Map *cgimg = (Fl_Android_565A_Map*)id_;
  delete cgimg;
  id_ = 0;
}


void Fl_Android_Graphics_Driver::set_color(Fl_Color i, unsigned int c)
{
  if (i>255) return;
  fl_cmap[i] = c;
}


void Fl_Android_Graphics_Driver::color(uchar r, uchar g, uchar b)
{
  color( (((Fl_Color)r)<<24)|(((Fl_Color)g)<<16)|(((Fl_Color)b)<<8) );
}

/**
 * Draw a rectangle that may be dithered if we are in colormap mode (which in
 * the year 2018 is as likely has a user with a berstein colored tube TV).
 * FIXME: This function should be virtual as well, or should not exist at all.
 */
void fl_rectf(int x, int y, int w, int h, uchar r, uchar g, uchar b) {
#if USE_COLORMAP
  // ...
#endif
  fl_color(r,g,b);
  fl_rectf(x,y,w,h);
}


//
// End of "$Id$".
//
