//
// "$Id$"
//
// Tiny OpenGL demo program for the Fast Light Tool Kit (FLTK).
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

#include <config.h>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Hor_Slider.H>
#include <FL/math.h>

#if HAVE_GL

#include <FL/gl.h>
#include <FL/Fl_Gl_Window.H>

class shape_window : public Fl_Gl_Window {
  void draw();
public:
  int sides;
  shape_window(int x,int y,int w,int h,const char *l=0);
};

shape_window::shape_window(int x,int y,int w,int h,const char *l) :
Fl_Gl_Window(x,y,w,h,l) {
  sides = 3;
}

void shape_window::draw() {
// the valid() property may be used to avoid reinitializing your
// GL transformation for each redraw:
  if (!valid()) {
    valid(1);
    glLoadIdentity();
    glViewport(0, 0, pixel_w(), pixel_h());
  }
// draw an amazing graphic:
  glClear(GL_COLOR_BUFFER_BIT);
  glColor3f(.5,.6,.7);
  glBegin(GL_POLYGON);
  for (int j=0; j<sides; j++) {
    double ang = j*2*M_PI/sides;
    glVertex3f(cos(ang),sin(ang),0);
  }
  glEnd();
}

#else

#include <FL/Fl_Box.H>
class shape_window : public Fl_Box {
public:	
  int sides;
  shape_window(int x,int y,int w,int h,const char *l=0)
    :Fl_Box(FL_DOWN_BOX,x,y,w,h,l){
      label("This demo does\nnot work without GL");
  }
};

#endif

// when you change the data, as in this callback, you must call redraw():
void sides_cb(Fl_Widget *o, void *p) {
  shape_window *sw = (shape_window *)p;
  sw->sides = int(((Fl_Slider *)o)->value());
  sw->redraw();
}

int main(int argc, char **argv) {

  Fl::use_high_res_GL(1);
  Fl_Window window(300, 330);

// the shape window could be it's own window, but here we make it
// a child window:
  shape_window sw(10, 10, 280, 280);
// make it resize:
  window.resizable(&sw);
  //  window.size_range(300,330,0,0,1,1,1);
// add a knob to control it:
  Fl_Hor_Slider slider(50, 295, window.w()-60, 30, "Sides:");
  slider.align(FL_ALIGN_LEFT);
  slider.callback(sides_cb,&sw);
  slider.value(sw.sides);
  slider.step(1);
  slider.bounds(3,40);

  window.end();
  window.show(argc,argv);
    
  return Fl::run();
}

//
// End of "$Id$".
//
