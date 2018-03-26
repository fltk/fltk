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

static int sign(int v) { return (v<0) ? -1 : 1; }

/*
 * By linking this module, the following static method will instantiate the
 * Windows GDI Graphics driver as the main display driver.
 */
Fl_Graphics_Driver *Fl_Graphics_Driver::newMainGraphicsDriver()
{
  return new Fl_Android_Graphics_Driver();
}


Fl_Android_Graphics_Driver::Fl_Android_Graphics_Driver() :
        pStride(0), pBits(0)
{
}


Fl_Android_Graphics_Driver::~Fl_Android_Graphics_Driver()
{
}


void Fl_Android_Graphics_Driver::make_current(Fl_Window *win)
{
  // The Stride is the offset between lines in the graphics buffer
  pStride = Fl_Android_Application::graphics_buffer().stride;
  // Bits is the memory address of the top left corner of the window
  pBits = ((uint16_t*)(Fl_Android_Application::graphics_buffer().bits))
          + win->x_root() + pStride * win->y_root();

  // TODO: set the clipping area
  // set the clipping area to the physical screen size in window coordinates
  pWindowRegion.set(-win->x(), -win->y(), 600, 800);
  pWindowRegion.intersect_with(Fl_Rect_Region(0, 0, win->w(), win->h()));

  pDesktopWindowRegion.set(pWindowRegion);

  // remove all window rectangles that are positioned on top of this window
  // TODO: this region is expensive to calculate. Cache it for each window and recalculate when windows move, show, hide, or change order
  Fl_Window *wTop = Fl::first_window();
  while (wTop) {
    if (wTop==win) break;
    Fl_Rect_Region r(wTop->x()-win->x(), wTop->y()-win->y(), wTop->w(), wTop->h());
    pDesktopWindowRegion.subtract(r);
    wTop = Fl::next_window(wTop);
  }
  pClippingRegion.set(pDesktopWindowRegion);
}


static uint16_t  make565(int red, int green, int blue)
{
    return (uint16_t)( ((red   << 8) & 0xf800) |
                       ((green << 3) & 0x07e0) |
                       ((blue  >> 3) & 0x001f) );
}

extern unsigned fl_cmap[256];


uint16_t Fl_Android_Graphics_Driver::make565(Fl_Color crgba)
{
  if (crgba<0x00000100) crgba = fl_cmap[crgba];
    return (uint16_t)( ((crgba >>16) & 0xf800) |
                       ((crgba >>13) & 0x07e0) |
                       ((crgba >>11) & 0x001f) );
}


void Fl_Android_Graphics_Driver::rectf_unscaled(float x, float y, float w, float h)
{
  for (const auto &it: pClippingRegion.overlapping(Fl_Rect_Region(x, y, w, h))) {
    Fl_Rect_Region &s = it->clipped_rect();
    rectf_unclipped(s.x(), s.y(), s.w(), s.h());
  }
}


void Fl_Android_Graphics_Driver::rectf_unclipped(float x, float y, float w, float h)
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


void Fl_Android_Graphics_Driver::xyline_unscaled(float x, float y, float x1)
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


void Fl_Android_Graphics_Driver::xyline_unclipped(float x, float y, float x1)
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

void Fl_Android_Graphics_Driver::yxline_unscaled(float x, float y, float y1)
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

void Fl_Android_Graphics_Driver::yxline_unclipped(float x, float y, float y1)
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


void Fl_Android_Graphics_Driver::rect_unscaled(float x, float y, float w, float h)
{
  xyline(x, y, x+w-1);
  yxline(x, y, y+h-1);
  yxline(x+w-1, y, y+h-1);
  xyline(x, y+h-1, x+w-1);
}


void Fl_Android_Graphics_Driver::line_style_unscaled(int style, float width, char* dashes)
{
  pLineStyle = style;
  // TODO: finish this!
}


void Fl_Android_Graphics_Driver::point_unscaled(float x, float y)
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
void Fl_Android_Graphics_Driver::line_unscaled(float x, float y, float x1, float y1)
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
    point_unscaled(x, y);
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


void Fl_Android_Graphics_Driver::line_unscaled(float x, float y, float x1, float y1, float x2, float y2)
{
  begin_line();
  transformed_vertex0(x, y);
  transformed_vertex0(x1, y1);
  transformed_vertex0(x2, y2);
  end_line();
}


void Fl_Android_Graphics_Driver::loop_unscaled(float x0, float y0, float x1, float y1, float x2, float y2)
{
  begin_loop();
  transformed_vertex0(x0, y0);
  transformed_vertex0(x1, y1);
  transformed_vertex0(x2, y2);
  end_loop();
}


void Fl_Android_Graphics_Driver::loop_unscaled(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3)
{
  begin_loop();
  transformed_vertex0(x0, y0);
  transformed_vertex0(x1, y1);
  transformed_vertex0(x2, y2);
  transformed_vertex0(x3, y3);
  end_loop();
}


void Fl_Android_Graphics_Driver::polygon_unscaled(float x0, float y0, float x1, float y1, float x2, float y2)
{
  begin_polygon();
  transformed_vertex0(x0, y0);
  transformed_vertex0(x1, y1);
  transformed_vertex0(x2, y2);
  end_polygon();
}


void Fl_Android_Graphics_Driver::polygon_unscaled(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3)
{
  begin_polygon();
  transformed_vertex0(x0, y0);
  transformed_vertex0(x1, y1);
  transformed_vertex0(x2, y2);
  transformed_vertex0(x3, y3);
  end_polygon();
}


/**
 * Reset the vertex counter to zero.
 */
void Fl_Android_Graphics_Driver::begin_vertices()
{
  pnVertex = 0;
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
  pnVertex++;
}

/**
 * Start a list of vertices to draw multiple points.
 */
void Fl_Android_Graphics_Driver::begin_points()
{
  begin_vertices();
  Fl_Scalable_Graphics_Driver::begin_points();
}

/**
 * Start a list of vertices to draw a polyline.
 */
void Fl_Android_Graphics_Driver::begin_line()
{
  begin_vertices();
  Fl_Scalable_Graphics_Driver::begin_line();
}

/**
 * Start a list of vertices to draw a line loop.
 */
void Fl_Android_Graphics_Driver::begin_loop()
{
  begin_vertices();
  Fl_Scalable_Graphics_Driver::begin_loop();
}

/**
 * Start a list of vertices to draw a polygon.
 */
void Fl_Android_Graphics_Driver::begin_polygon()
{
  begin_vertices();
  Fl_Scalable_Graphics_Driver::begin_polygon();
}

/**
 * Start a list of vertices to draw a complex polygon.
 */
void Fl_Android_Graphics_Driver::begin_complex_polygon()
{
  begin_vertices();
  Fl_Scalable_Graphics_Driver::begin_complex_polygon();
}

/**
 * Draw all stored vertices as points.
 */
void Fl_Android_Graphics_Driver::end_points()
{
  for (int i=0; i<pnVertex; ++i) {
    Vertex &v = pVertex[i];
    if (!v.pIsGap)
      point_unscaled(v.pX, v.pY);
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
      line_unscaled(v1.pX, v1.pY, v2.pX, v2.pY);
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
      line_unscaled(v1.pX, v1.pY, v2.pX, v2.pY);
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
        xyline_unscaled(nodeX[i], pixelY, nodeX[i + 1]);
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
        xyline_unscaled(nodeX[i], pixelY, nodeX[i + 1]);
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
void Fl_Android_Graphics_Driver::transformed_vertex0(float x, float y)
{
  add_vertex(x, y);
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
void Fl_Android_Graphics_Driver::arc_unscaled(float xi, float yi, float w, float h, double a1, double a2)
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
    line_unscaled(px, py, nx, ny);
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
void Fl_Android_Graphics_Driver::pie_unscaled(float xi, float yi, float w, float h, double b1, double b2)
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
          xyline_unscaled(x + hiLeft, iy, x + loRight);
      } else {
        if ((!loInside) && (!hiInside)) {
//          fl_color(FL_MAGENTA);
          if ( (b1o<=0.0 && b2>=90.0) || (b1o<=(0.0-360.0) && b2>=(90.0-360.0)) )
            xyline_unscaled(x - sinALrx, iy, x);
        } else {
          if (loInside) {
//            fl_color(FL_BLUE);
            if (a1 < aL)
              xyline_unscaled(x + loLeft, iy, x + loRight);
          }
          if (hiInside) {
//            fl_color(FL_YELLOW);
            xyline_unscaled(x + hiLeft, iy, x);
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
          xyline_unscaled(x + loLeft, iy, x + hiRight);
      } else {
        if ((!loInside) && (!hiInside)) {
//          fl_color(FL_MAGENTA);
          if ( (b1o<=90.0 && b2>=180.0) || (b1o<=(90.0-360.0) && b2>=(180.0-360.0)) )
            xyline_unscaled(x - sinALrx, iy, x);
        } else {
          if (loInside) {
//            fl_color(FL_BLUE);
            xyline_unscaled(x + loLeft, iy, x);
          }
          if (hiInside) {
//            fl_color(FL_YELLOW);
            if (a2 > aL)
              xyline_unscaled(x + hiLeft, iy, x + hiRight);
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
          xyline_unscaled(x + hiLeft, iy, x + loRight);
      } else {
        if ((!loInside) && (!hiInside)) {
//          fl_color(FL_MAGENTA);
          if ( (b1o<=180.0 && b2>=270.0) || (b1o<=(180.0-360.0) && b2>=(270.0-360.0)) )
            xyline_unscaled(x + sinALrx, iy, x);
        } else {
          if (loInside) {
//            fl_color(FL_BLUE);
            if (a1 < aR)
              xyline_unscaled(x + loLeft, iy, x + loRight);
          }
          if (hiInside) {
//            fl_color(FL_YELLOW);
            xyline_unscaled(x + hiLeft, iy, x);
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
          xyline_unscaled(x + loLeft, iy, x + hiRight);
      } else {
        if ((!loInside) && (!hiInside)) {
//          fl_color(FL_MAGENTA);
          if ( (b1o<=270.0 && b2>=360.0) || (b1o<=(270.0-360.0) && b2>=(360.0-360.0)) )
            xyline_unscaled(x + sinALrx, iy, x);
        } else {
          if (loInside) {
//            fl_color(FL_BLUE);
            xyline_unscaled(x + loLeft, iy, x);
          }
          if (hiInside) {
//            fl_color(FL_YELLOW);
            if (a2 > aR)
              xyline_unscaled(x + hiLeft, iy, x + hiRight);
          }
        }
      }
    }
  }
}

/**
 * shortcut the closed circles so they use XDrawArc:
 * FIXME: these do not draw rotated ellipses correctly!
 * */
void Fl_Android_Graphics_Driver::ellipse_unscaled(double xt, double yt, double rx, double ry) {
  float llx = xt-rx;
  float w = xt+rx-llx;
  float lly = yt-ry;
  float h = yt+ry-lly;

  if (what==POLYGON)
    pie_unscaled(llx, lly, w, h, 0.0, 360.0);
  else
    arc_unscaled(llx, lly, w, h, 0.0, 360.0);
}


void Fl_Android_Graphics_Driver::draw_unscaled(Fl_Bitmap *bm, float s, int XP, int YP, int WP, int HP, int cx, int cy)
{
  int X, Y, W, H;
  if (Fl_Graphics_Driver::prepare(bm, XP, YP, WP, HP, cx, cy, X, Y, W, H)) {
    return;
  }
  if (*Fl_Graphics_Driver::id(bm)) {
    Fl_Android_Bytemap *cache = (Fl_Android_Bytemap*)*Fl_Graphics_Driver::id(bm);
    for (const auto &it: pClippingRegion.overlapping(Fl_Rect_Region(X, Y, W, H))) {
      render_bytemap(XP, YP, cache, it->clipped_rect());
    }
  }
}


fl_uintptr_t Fl_Android_Graphics_Driver::cache(Fl_Bitmap *bm)
{
  int w = bm->w(), h = bm->h();
  int rowBytes = (w+7)>>3;

  Fl_Android_Bytemap *cache = new Fl_Android_Bytemap(w, h);
  for (int yy=0; yy<w; yy++) {
    const uchar *src = bm->array + yy*rowBytes;
    uchar *dst = cache->pBytes + yy*cache->pStride;
    uchar d;
    for (int xx=0; xx<w; xx++) {
      if ((xx&7)==0) d = *src++;
      if (d&1) *dst = 0xff; else *dst = 0;
      dst++;
      d >>= 1;
    }
  }

  return (fl_uintptr_t)cache;
}

//
// End of "$Id$".
//
