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
#include <FL/math.h>

// --- line and polygon drawing with integer coordinates

void Fl_OpenGL_Graphics_Driver::point(int x, int y) {
  if (line_width_ == 1.0f) {
    glBegin(GL_POINTS);
    glVertex2f(x+0.5f, y+0.5f);
    glEnd();
  } else {
    float offset = line_width_ / 2.0f;
    float xx = x+0.5f, yy = y+0.5f;
    glRectf(xx-offset, yy-offset, xx+offset, yy+offset);
  }
}

void Fl_OpenGL_Graphics_Driver::rect(int x, int y, int w, int h) {
  float offset = line_width_ / 2.0f;
  float xx = x+0.5f, yy = y+0.5f;
  float rr = x+w-0.5f, bb = y+h-0.5f;
  glRectf(xx-offset, yy-offset, rr+offset, yy+offset);
  glRectf(xx-offset, bb-offset, rr+offset, bb+offset);
  glRectf(xx-offset, yy-offset, xx+offset, bb+offset);
  glRectf(rr-offset, yy-offset, rr+offset, bb+offset);
}

void Fl_OpenGL_Graphics_Driver::rectf(int x, int y, int w, int h) {
  if (w<=0 || h<=0) return;
  glRectf((GLfloat)x, (GLfloat)y, (GLfloat)(x+w), (GLfloat)(y+h));
}

void Fl_OpenGL_Graphics_Driver::line(int x, int y, int x1, int y1) {
  if (x==x1 && y==y1) return;
  if (x==x1) {
    yxline(x, y, y1);
    return;
  }
  if (y==y1) {
    xyline(x, y, x1);
    return;
  }
  float xx = x+0.5f, xx1 = x1+0.5f;
  float yy = y+0.5f, yy1 = y1+0.5f;
  if (line_width_==1.0f) {
    glBegin(GL_LINE_STRIP);
    glVertex2f(xx, yy);
    glVertex2f(xx1, yy1);
    glEnd();
  } else {
    float dx = xx1-xx, dy = yy1-yy;
    float len = sqrtf(dx*dx+dy*dy);
    dx = dx/len*line_width_*0.5f;
    dy = dy/len*line_width_*0.5f;

    glBegin(GL_TRIANGLE_STRIP);
    glVertex2f(xx-dy, yy+dx);
    glVertex2f(xx+dy, yy-dx);
    glVertex2f(xx1-dy, yy1+dx);
    glVertex2f(xx1+dy, yy1-dx);
    glEnd();
  }
}

void Fl_OpenGL_Graphics_Driver::line(int x, int y, int x1, int y1, int x2, int y2) {
  // TODO: no corner types (miter) yet
  line(x, y, x1, y1);
  line(x1, y1, x2, y2);
}

void Fl_OpenGL_Graphics_Driver::xyline(int x, int y, int x1) {
  float offset = line_width_ / 2.0f;
  float xx = (float)x, yy = y+0.5f, rr = x1+1.0f;
  glRectf(xx, yy-offset, rr, yy+offset);
}

void Fl_OpenGL_Graphics_Driver::xyline(int x, int y, int x1, int y2) {
  float offset = line_width_ / 2.0f;
  float xx = (float)x, yy = y+0.5f, rr = x1+0.5f, bb = y2+1.0f;
  glRectf(xx, yy-offset, rr+offset, yy+offset);
  glRectf(rr-offset, yy+offset, rr+offset, bb);
}

void Fl_OpenGL_Graphics_Driver::xyline(int x, int y, int x1, int y2, int x3) {
  float offset = line_width_ / 2.0f;
  float xx = (float)x, yy = y+0.5f, xx1 = x1+0.5f, rr = x3+1.0f, bb = y2+0.5f;
  glRectf(xx, yy-offset, xx1+offset, yy+offset);
  glRectf(xx1-offset, yy+offset, xx1+offset, bb+offset);
  glRectf(xx1+offset, bb-offset, rr, bb+offset);
}

void Fl_OpenGL_Graphics_Driver::yxline(int x, int y, int y1) {
  float offset = line_width_ / 2.0f;
  float xx = x+0.5f, yy = (float)y, bb = y1+1.0f;
  glRectf(xx-offset, yy, xx+offset, bb);
}

void Fl_OpenGL_Graphics_Driver::yxline(int x, int y, int y1, int x2) {
  float offset = line_width_ / 2.0f;
  float xx = x+0.5f, yy = (float)y, rr = x2+1.0f, bb = y1+0.5f;
  glRectf(xx-offset, yy, xx+offset, bb+offset);
  glRectf(xx+offset, bb-offset, rr, bb+offset);
}

void Fl_OpenGL_Graphics_Driver::yxline(int x, int y, int y1, int x2, int y3) {
  float offset = line_width_ / 2.0f;
  float xx = x+0.5f, yy = (float)y, yy1 = y1+0.5f, rr = x2+0.5f, bb = y3+1.0f;
  glRectf(xx-offset, yy, xx+offset, yy1+offset);
  glRectf(xx+offset, yy1-offset, rr+offset, yy1+offset);
  glRectf(rr-offset, yy1+offset, rr+offset, bb);
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

void Fl_OpenGL_Graphics_Driver::focus_rect(int x, int y, int w, int h) {
  float width = line_width_;
  int stipple = line_stipple_;
  line_style(FL_DOT, 1);
  glBegin(GL_LINE_LOOP);
  glVertex2f(x+0.5f, y+0.5f);
  glVertex2f(x+w+0.5f, y+0.5f);
  glVertex2f(x+w+0.5f, y+h+0.5f);
  glVertex2f(x+0.5f, y+h+0.5f);
  glEnd();
  line_style(stipple, (int)width);
}


// -----------------------------------------------------------------------------

static int gl_min(int a, int b) { return (a<b) ? a : b; }
static int gl_max(int a, int b) { return (a>b) ? a : b; }

enum {
  kStateFull, // Region is the full window
  kStateRect, // Region is a rectangle
  kStateEmpty // Region is an empty space
};

typedef struct Fl_Gl_Region {
  int x, y, w, h;
  int gl_x, gl_y, gl_w, gl_h;
  char state;
  void set(int inX, int inY, int inW, int inH) {
    if (inW<=0 || inH<=0) {
      state = kStateEmpty;
      x = inX; y = inY; w = 1; h = 1; // or 0?
    } else {
      x = inX; y = inY; w = inW; h = inH;
      state = kStateRect;
    }
    Fl_Gl_Window *win = Fl_Gl_Window::current()->as_gl_window();
    if (win) {
      float scale = win->pixels_per_unit();
      gl_x = int(x*scale);
      gl_y = int((win->h()-h-y+1)*scale);
      gl_w = int((w-1)*scale);
      gl_h = int((h-1)*scale);
      if (inX<=0 && inY<=0 && inX+inW>win->w() && inY+inH>=win->h()) {
        state = kStateFull;
      }
    } else {
      state = kStateFull;
    }
  }
  void set_full() { state = kStateFull; }
  void set_empty() { state = kStateEmpty; }
  void set_intersect(int inX, int inY, int inW, int inH, Fl_Gl_Region &g) {
    if (g.state==kStateFull) {
      set(inX, inY, inW, inH);
    } else if (g.state==kStateEmpty) {
      set_empty();
    } else {
      int rx = gl_max(inX, g.x);
      int ry = gl_max(inY, g.y);
      int rr = gl_min(inX+inW, g.x+g.w);
      int rb = gl_max(inY+inH, g.y+g.h);
      set(rx, ry, rr-rx, rb-ry);
    }
  }
  void apply() {
    if (state==kStateFull) {
      glDisable(GL_SCISSOR_TEST);
    } else {
      glScissor(gl_x, gl_y, gl_w, gl_h);
      glEnable(GL_SCISSOR_TEST);
    }
  }
} Fl_Gl_Region;

static int gl_rstackptr = 0;
static const int gl_region_stack_max = FL_REGION_STACK_SIZE - 1;
static Fl_Gl_Region gl_rstack[FL_REGION_STACK_SIZE];

/*
 Intersect the given rect with the current rect, push the result on the stack,
 and apply the new clipping area.
 */
void Fl_OpenGL_Graphics_Driver::push_clip(int x, int y, int w, int h) {
  if (gl_rstackptr==gl_region_stack_max) {
    Fl::warning("Fl_OpenGL_Graphics_Driver::push_clip: clip stack overflow!\n");
    return;
  }
  if (gl_rstackptr==0) {
    gl_rstack[gl_rstackptr].set(x, y, w, h);
  } else {
    gl_rstack[gl_rstackptr].set_intersect(x, y, w, h, gl_rstack[gl_rstackptr-1]);
  }
  gl_rstack[gl_rstackptr].apply();
  gl_rstackptr++;
}

/*
 Remove the current clipping area and apply the previous one on the stack.
 */
void Fl_OpenGL_Graphics_Driver::pop_clip() {
  if (gl_rstackptr==0) {
    glDisable(GL_SCISSOR_TEST);
    Fl::warning("Fl_OpenGL_Graphics_Driver::pop_clip: clip stack underflow!\n");
    return;
  }
  gl_rstackptr--;
  restore_clip();
}

/*
 Push a full area onton the stack, so no clipping will take place.
 */
void Fl_OpenGL_Graphics_Driver::push_no_clip() {
  if (gl_rstackptr==gl_region_stack_max) {
    Fl::warning("Fl_OpenGL_Graphics_Driver::push_no_clip: clip stack overflow!\n");
    return;
  }
  gl_rstack[gl_rstackptr].set_full();
  gl_rstack[gl_rstackptr].apply();
  gl_rstackptr++;
}

/*
 We don't know the format of clip regions of the default driver, so return NULL.
 */
Fl_Region Fl_OpenGL_Graphics_Driver::clip_region() {
  return NULL;
}

/*
 We don't know the format of clip regions of the default driver, so do the best
 we can.
 */
void Fl_OpenGL_Graphics_Driver::clip_region(Fl_Region r) {
  if (r==NULL) {
    glDisable(GL_SCISSOR_TEST);
  } else {
    restore_clip();
  }
}

/*
 Apply the current clipping rect.
 */
void Fl_OpenGL_Graphics_Driver::restore_clip() {
  if (gl_rstackptr==0) {
    glDisable(GL_SCISSOR_TEST);
  } else {
    gl_rstack[gl_rstackptr-1].apply();
  }
}

/*
 Does the rectangle intersect the current clip region?
 0 = regions don't intersect, nothing to draw
 1 = region is fully inside current clipping region
 2 = region is partially inside current clipping region
 */
int Fl_OpenGL_Graphics_Driver::not_clipped(int x, int y, int w, int h) {
  if (gl_rstackptr==0)
    return 1;
  Fl_Gl_Region &g = gl_rstack[gl_rstackptr-1];
  if (g.state==kStateFull)
    return 1;
  if (g.state==kStateEmpty)
    return 0;
  int r = x+w, b = y + h;
  int gr = g.x+g.w, gb = g.y+g.h;
  if (r<=g.x || x>=gr || b<=g.y || y>=gb) return 0;
  if (x>=g.x && y>=g.y && r<=gr && b<=gb) return 1;
  return 2;
}

/*
 Calculate the intersection of the given rect and the clipping area.
 Return 0 if the result did not change.
 */
int Fl_OpenGL_Graphics_Driver::clip_box(int x, int y, int w, int h, int &X, int &Y, int &W, int &H) {
  X = x; Y = y; W = w; H = h;
  if (gl_rstackptr==0)
    return 0;
  Fl_Gl_Region &g = gl_rstack[gl_rstackptr-1];
  if (g.state==kStateFull)
    return 0;
  int r = x+w, b = y + h;
  int gr = g.x+g.w, gb = g.y+g.h;
  X = gl_max(x, g.x);
  Y = gl_max(y, g.y);
  W = gl_min(r, gr) - X;
  H = gl_min(b, gb) - Y;
  return (x!=X || y!=Y || w!=W || h!=H);
}
