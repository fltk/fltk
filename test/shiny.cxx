//
// "$Id: shiny.cxx,v 1.5.2.3 2001/01/22 15:13:41 easysw Exp $"
//
// OpenGL "shiny buttons" test program for the Fast Light Tool Kit (FLTK).
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

#include <config.h>
#include "shiny_panel.cxx"
#include <FL/fl_message.H>
#include <stdio.h>

static uchar color[8][3] = {
  {128,136,149},
  {63,54,21},
  {128,136,146},
  {223,223,223},
  {121,128,128},
  {192,206,220},
  {137,143,145},
  {99,81,64}
};
static int thickness = 3;

int which = 0;

static Fl_Color pcolor;

Fl_Window *window;

void color_slider_cb(Fl_Value_Slider *o, long i) {
  int v = int(o->value());
  if (!i) {
    color[which][0] = color[which][1] = color[which][2] = v;
    color_slider[1]->value(v);
    color_slider[2]->value(v);
    color_slider[3]->value(v);
  } else {
    color[which][i-1] = v;
  }
  for (int n=0; n<window->children(); n++) window->child(n)->redraw();
  pcolor = FL_BLACK; // make it recalculate actual colors
//   test_box[0]->redraw();
//   test_box[1]->redraw();
//   test_box[2]->redraw();
}

void set_sliders() {
  color_slider[0]->value(color[which][0]);
  color_slider[1]->value(color[which][0]);
  color_slider[2]->value(color[which][1]);
  color_slider[3]->value(color[which][2]);
}

void thickness_cb(Fl_Slider* s,void*) {
  thickness = int(s->value());
  for (int n=0; n<window->children(); n++) window->child(n)->redraw();
}

void which_cb(Fl_Button *, long i) {
  which = which&(~3) | i;
  set_sliders();
}

void inside_cb(Fl_Button *b, void*) {
  if (b->value()) which = which | 4;
  else which = which & (3|8);
  set_sliders();
}

void dump_cb(Fl_Button *, void*) {
  printf("static uchar color[8][3] = {\n");
  for (int i=0; i<8; i++) {
    printf("  {%d,%d,%d}",color[i][0],color[i][1],color[i][2]);
    if (i<7) printf(",\n");
  }
  printf("\n};\nstatic int thickness = %d;\n",thickness);
}

#include <FL/fl_draw.H>

#if HAVE_GL
#include <FL/gl.h>

static uchar C[8][3]; // actual colors for current button

static void calc_color(Fl_Color c) {
  uchar r[3];
  pcolor = c;
  Fl::get_color(c,r[0],r[1],r[2]);
  for (int x = 0; x<8; x++) for (int y=0; y<3; y++) {
    int i = r[y]-166+color[x][y];
    if (i<0) i = 0; else if (i>255) i = 255;
    C[x][y] = i;
  }
}
void shiny_up_box(int x1, int y1, int w1, int h1, Fl_Color c) {
  if (c != pcolor) calc_color(c);
  int x = x1+1;
  int y = Fl_Window::current()->h()-(y1+h1-1);
  int w = w1-2;
  int h = h1-2;
  gl_start();

  // left edge:
  glBegin(GL_POLYGON);
  glColor3ub(C[0][0],C[0][1],C[0][2]);
  glVertex2i(x,y);
  glVertex2i(x+thickness,y+thickness);
  glColor3ub(C[3][0],C[3][1],C[3][2]);
  glVertex2i(x+thickness,y+h-thickness);
  glVertex2i(x,y+h);
  glEnd();

  // top edge:
  glBegin(GL_POLYGON);
  glVertex2i(x,y+h);
  glVertex2i(x+thickness,y+h-thickness);
  glColor3ub(C[2][0],C[2][1],C[2][2]);
  glVertex2i(x+w-thickness,y+h-thickness);
  glVertex2i(x+w,y+h);
  glEnd();

  // right edge:
  glColor3ub(C[1][0],C[1][1],C[1][2]);
  glBegin(GL_POLYGON);
  glVertex2i(x+w-thickness,y+thickness);
  glVertex2i(x+w,y+thickness);
  glVertex2i(x+w,y+h);
  glVertex2i(x+w-thickness,y+h-thickness);
  glEnd();

  // bottom edge:
  glBegin(GL_POLYGON);
  glVertex2i(x,y);
  glVertex2i(x+w,y);
  glVertex2i(x+w,y+thickness);
  glVertex2i(x+thickness,y+thickness);
  glEnd();

  glBegin(GL_POLYGON);
  glColor3ub(C[4][0],C[4][1],C[4][2]);
  glVertex2i(x+thickness,y+thickness);
  glColor3ub(C[5][0],C[5][1],C[5][2]);
  glVertex2i(x+w-thickness,y+thickness);
  glColor3ub(C[6][0],C[6][1],C[6][2]);
  glVertex2i(x+w-thickness,y+h-thickness);
  glColor3ub(C[7][0],C[7][1],C[7][2]);
  glVertex2i(x+thickness,y+h-thickness);
  glEnd();

  gl_finish();
  fl_color(FL_BLACK);
  fl_rect(x1,y1,w1,h1);
}

void shiny_down_box(int x1, int y1, int w1, int h1, Fl_Color c) {
  if (c != pcolor) calc_color(c);
  int x = x1+1;
  int y = Fl_Window::current()->h()-(y1+h1-1);
  int w = w1-2;
  int h = h1-2;
  gl_start();

  // left edge:
  glColor3ub(C[1][0],C[1][1],C[1][2]);
  glBegin(GL_POLYGON);
  glVertex2i(x,y);
  glVertex2i(x+thickness,y+thickness);
  glVertex2i(x+thickness,y+h-thickness);
  glVertex2i(x,y+h);
  glEnd();

  // top edge:
  glBegin(GL_POLYGON);
  glVertex2i(x,y+h);
  glVertex2i(x+thickness,y+h-thickness);
  glVertex2i(x+w-thickness,y+h-thickness);
  glVertex2i(x+w,y+h);
  glEnd();

  // bottom edge:
  glBegin(GL_POLYGON);
  glColor3ub(C[0][0],C[0][1],C[0][2]);
  glVertex2i(x+thickness,y+thickness);
  glVertex2i(x,y);
  glColor3ub(C[1][0],C[1][1],C[1][2]);
  glVertex2i(x+w,y);
  glVertex2i(x+w-thickness,y+thickness);
  glEnd();

  // right edge:
  glBegin(GL_POLYGON);
  glVertex2i(x+w-thickness,y+thickness);
  glVertex2i(x+w,y);
  glColor3ub(C[2][0],C[2][1],C[2][2]);
  glVertex2i(x+w,y+h);
  glVertex2i(x+w-thickness,y+h-thickness);
  glEnd();

  // inside:
  glBegin(GL_POLYGON);
  glColor3ub(C[4][0],C[4][1],C[4][2]);
  glVertex2i(x+thickness,y+thickness);
  glColor3ub(C[5][0],C[5][1],C[5][2]);
  glVertex2i(x+w-thickness,y+thickness);
  glColor3ub(C[6][0],C[6][1],C[6][2]);
  glVertex2i(x+w-thickness,y+h-thickness);
  glColor3ub(C[7][0],C[7][1],C[7][2]);
  glVertex2i(x+thickness,y+h-thickness);
  glEnd();

  gl_finish();
  fl_color(FL_BLACK);
  fl_rect(x1,y1,w1,h1);
}

// It looks interesting if you use this for the window's boxtype,
// but it is way too slow under MESA:
void shiny_flat_box(int x, int y1, int w, int h, Fl_Color c) {
  if (c != pcolor) calc_color(c);
  int y = Fl_Window::current()->h()-(y1+h);
  gl_start();
  glBegin(GL_POLYGON);
  glColor3ub(C[4][0],C[4][1],C[4][2]);
  glVertex2i(x,y);
  glColor3ub(C[5][0],C[5][1],C[5][2]);
  glVertex2i(x+w,y);
  glColor3ub(C[6][0],C[6][1],C[6][2]);
  glVertex2i(x+w,y+h);
  glColor3ub(C[7][0],C[7][1],C[7][2]);
  glVertex2i(x,y+h);
  glEnd();
  gl_finish();
}
#endif

// If you use a shiny box as a background, things like the sliders that
// expect to erase a flat area will not work, as you will see the edges
// of the area.  This "box type" clips to the area and then draws the
// parent's box.  Perhaps sliders should be fixed to do this automatically?
void invisible_box(int x, int y, int w, int h, Fl_Color c) {
  fl_clip(x,y,w,h);
  Fl_Window *W = Fl_Window::current();
  fl_draw_box(W->box(),0,0,W->w(),W->h(),c);
  fl_pop_clip();
}

#define SHINY_BOX (Fl_Boxtype)30
#define INVISIBLE_BOX (Fl_Boxtype)31

int main(int argc, char **argv) {
  window = make_panels();
#if HAVE_GL
  // This will cause all buttons to be shiny:
  Fl::set_boxtype(FL_UP_BOX, shiny_up_box,3,3,6,6);
  Fl::set_boxtype(FL_DOWN_BOX, shiny_down_box,3,3,6,6);
  // replacing FL_FLAT_BOX does not work!  Fl_Window makes assumptions
  // about what FL_FLAT_BOX does, and sets the X background pixel.
//Fl::set_boxtype(FL_FLAT_BOX, shiny_flat_box, 0,0,0,0);
  // Instead you must change box() on Fl_Window to a different value:
  Fl::set_boxtype(SHINY_BOX, shiny_flat_box, 0,0,0,0);
  window->box(SHINY_BOX);
  Fl::set_boxtype(INVISIBLE_BOX, invisible_box, 0,0,0,0);
#endif
  set_sliders();
//color_slider[0]->box(INVISIBLE_BOX);
//color_slider[1]->box(INVISIBLE_BOX);
//color_slider[2]->box(INVISIBLE_BOX);
//color_slider[3]->box(INVISIBLE_BOX);
  thickness_slider->value(thickness);
  thickness_slider->box(INVISIBLE_BOX);
  thickness_slider->slider(FL_UP_BOX);
  // we must eat the switches first so -display is done before trying
  // to set the visual:
  int i = 0;
  if (Fl::args(argc,argv,i) < argc) Fl::fatal(Fl::help);
#if HAVE_GL
  if (!Fl::gl_visual(FL_RGB)) Fl::fatal("Display does not do OpenGL");
#else
  fl_message("This demo does not work without OpenGL");
#endif
  window->show(argc,argv);
  return Fl::run();
}

//
// End of "$Id: shiny.cxx,v 1.5.2.3 2001/01/22 15:13:41 easysw Exp $".
//
