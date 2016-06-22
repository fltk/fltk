//
// "$Id$"
//

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

//
// Fullscreen test program for the Fast Light Tool Kit (FLTK).
//
// This demo shows how to do many of the window manipulations that
// are popular on SGI programs, even though X does not really like
// them.  You can toggle the border on/off, change the visual to
// switch between single/double buffer, and make the window take
// over the screen.
//
// Normally the program makes a single window with a child GL window.
// This simulates a program where the 3D display is surrounded by
// control knobs.  Running the program with an argument will
// make it make a seperate GL window from the controls window.  This
// simulates a (older?) style program where the graphics display is
// a different window than the controls.
//
// This program reports how many times it redraws the window to
// stdout, so you can see how much time it is wasting.  It appears
// to be impossible to prevent X from sending redundant resize
// events, so there are extra redraws.  But the way I have the
// code arranged here seems to be keeping that to a minimu.
//
// Apparently unavoidable bugs:
//
// Turning the border on causes an unnecessary redraw.
//
// Turning off full screen when the border is on causes an unnecessary
// resize and redraw when the program turns the border on.
//
// If it is a separate window, turning double buffering on and off
// will cause the window to raise, deiconize, and possibly move.  You
// can avoid this by making the Fl_Gl_Window a child of a normal
// window.

#include <config.h>
#include <FL/Fl.H>
#include <FL/Fl_Single_Window.H>
#include <FL/Fl_Hor_Slider.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Toggle_Light_Button.H>
#include <FL/math.h>
#include <FL/fl_ask.H>
#include <FL/Fl_Browser.H>
#include <stdio.h>

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
  printf("drawing size %d %d\n",w(),h());
  if (!valid()) {
    valid(1);
//  printf("init\n");
    glLoadIdentity();
    glViewport(0,0,pixel_w(),pixel_h());
  }
  glClear(GL_COLOR_BUFFER_BIT);
  glColor3f(.5,.6,.7);
  glBegin(GL_POLYGON);
  for (int j = 0; j < sides; j ++) {
    double ang = j*2*M_PI/sides;
    glVertex3f(cos(ang),sin(ang),0);
  }
  glEnd();
}

#else

#include <FL/fl_draw.H>

class shape_window : public Fl_Window {
  void draw();
public:
  int sides;
  shape_window(int x,int y,int w,int h,const char *l=0);
};

shape_window::shape_window(int x,int y,int w,int h,const char *l) :
Fl_Window(x,y,w,h,l) {
  sides = 3;
}

void shape_window::draw() {
  fl_color(0);
  fl_rectf(0,0,w(),h());
  fl_font(0,20);
  fl_color(7);
  fl_draw("This requires GL",0,0,w(),h(),FL_ALIGN_CENTER);
}

#endif

class fullscreen_window : public Fl_Single_Window {
  public:
  fullscreen_window(int W, int H, const char *t=0);
  int handle (int e);
  Fl_Toggle_Light_Button *b3;
  Fl_Toggle_Light_Button *b4;
};

fullscreen_window::fullscreen_window(int W, int H, const char *t) : Fl_Single_Window(W, H, t) { 

}

int fullscreen_window::handle(int e) {
  if (e == FL_FULLSCREEN) {
    printf("Received FL_FULLSCREEN event\n");
    b3->value(fullscreen_active());
  }
  if (Fl_Single_Window::handle(e)) return 1;
  return 0;
}

void sides_cb(Fl_Widget *o, void *p) {
  shape_window *sw = (shape_window *)p;
  sw->sides = int(((Fl_Slider *)o)->value());
  sw->redraw();
}

#if HAVE_GL
void double_cb(Fl_Widget *o, void *p) {
  shape_window *sw = (shape_window *)p;
  int d = ((Fl_Button *)o)->value();
  sw->mode(d ? Fl_Mode(FL_DOUBLE|FL_RGB) : FL_RGB);
}
#else
void double_cb(Fl_Widget *, void *) {}
#endif

void border_cb(Fl_Widget *o, void *p) {
  Fl_Window *w = (Fl_Window *)p;
  int d = ((Fl_Button *)o)->value();
  w->border(d);
#if defined(WIN32) || defined(__APPLE__)
  int wx = w->x(), wy = w->y();
  w->hide(); w->show();
  w->position(wx, wy);
#endif
}

Fl_Button *border_button;
void fullscreen_cb(Fl_Widget *o, void *p) {
  Fl_Window *w = (Fl_Window *)p;
  int d = ((Fl_Button *)o)->value();
  if (d) {
    w->fullscreen();
#ifndef WIN32 // update our border state in case border was turned off
    border_button->value(w->border());
#endif
  } else {
    w->fullscreen_off();
  }
}

void allscreens_cb(Fl_Widget *o, void *p) {
  Fl_Window *w = (Fl_Window *)p;
  int d = ((Fl_Button *)o)->value();
  if (d) {
    int top, bottom, left, right;
    int top_y, bottom_y, left_x, right_x;

    int sx, sy, sw, sh;

    top = bottom = left = right = 0;

    Fl::screen_xywh(sx, sy, sw, sh, 0);
    top_y = sy;
    bottom_y = sy + sh;
    left_x = sx;
    right_x = sx + sw;

    for (int i = 1;i < Fl::screen_count();i++) {
      Fl::screen_xywh(sx, sy, sw, sh, i);
      if (sy < top_y) {
        top = i;
        top_y = sy;
      }
      if ((sy + sh) > bottom_y) {
        bottom = i;
        bottom_y = sy + sh;
      }
      if (sx < left_x) {
        left = i;
        left_x = sx;
      }
      if ((sx + sw) > right_x) {
        right = i;
        right_x = sx + sw;
      }
    }

    w->fullscreen_screens(top, bottom, left, right);
  } else {
    w->fullscreen_screens(-1, -1, -1, -1);
  }
}

void update_screeninfo(Fl_Widget *b, void *p) {
    Fl_Browser *browser = (Fl_Browser *)p;
    int x, y, w, h;
    char line[128];
    browser->clear();

    sprintf(line, "Main screen work area: %dx%d@%d,%d", Fl::w(), Fl::h(), Fl::x(), Fl::y());
    browser->add(line);
    Fl::screen_work_area(x, y, w, h);
    sprintf(line, "Mouse screen work area: %dx%d@%d,%d", w, h, x, y);
    browser->add(line);
    for (int n = 0; n < Fl::screen_count(); n++) {
	int x, y, w, h;
        float dpih, dpiv;
	Fl::screen_xywh(x, y, w, h, n);
        Fl::screen_dpi(dpih, dpiv, n);
        sprintf(line, "Screen %d: %dx%d@%d,%d DPI:%.1fx%.1f", n, w, h, x, y, dpih, dpiv);
	browser->add(line);
	Fl::screen_work_area(x, y, w, h, n);
	sprintf(line, "Work area %d: %dx%d@%d,%d", n, w, h, x, y);
	browser->add(line);
    }
}

#include <stdlib.h>

void exit_cb(Fl_Widget *, void *) {
  exit(0);
}

#define NUMB 8

int twowindow = 0;
int initfull = 0;
int arg(int, char **argv, int &i) {
  if (argv[i][1] == '2') {twowindow = 1; i++; return 1;}
  if (argv[i][1] == 'f') {initfull = 1; i++; return 1;}
  return 0;
}

int main(int argc, char **argv) {

  Fl::use_high_res_GL(1);
  int i=0;
  if (Fl::args(argc,argv,i,arg) < argc)
    Fl::fatal("Options are:\n -2 = 2 windows\n -f = startup fullscreen\n%s",Fl::help);

  fullscreen_window window(400,400+30*NUMB); window.end();

  shape_window sw(10,10,window.w()-20,window.h()-30*NUMB-120);

#if HAVE_GL
  sw.mode(FL_RGB);
#endif

  Fl_Window *w;
  if (twowindow) {	// make it's own window
    sw.resizable(&sw);
    w = &sw;
    window.set_modal();	// makes controls stay on top when fullscreen pushed
    argc--;
    sw.show();
  } else {		// otherwise make a subwindow
    window.add(sw);
    window.resizable(&sw);
    w = &window;
  }

  window.begin();

  int y = window.h()-30*NUMB-105;
  Fl_Hor_Slider slider(50,y,window.w()-60,30,"Sides:");
  slider.align(FL_ALIGN_LEFT);
  slider.callback(sides_cb,&sw);
  slider.value(sw.sides);
  slider.step(1);
  slider.bounds(3,40);
  y+=30;

  Fl_Toggle_Light_Button b1(50,y,window.w()-60,30,"Double Buffered");
  b1.callback(double_cb,&sw);
  y+=30;

  Fl_Input i1(50,y,window.w()-60,30, "Input");
  y+=30;

  Fl_Toggle_Light_Button b2(50,y,window.w()-60,30,"Border");
  b2.callback(border_cb,w);
  b2.set();
  border_button = &b2;
  y+=30;

  window.b3 = new Fl_Toggle_Light_Button(50,y,window.w()-60,30,"FullScreen");
  window.b3->callback(fullscreen_cb,w);
  y+=30;

  window.b4 = new Fl_Toggle_Light_Button(50,y,window.w()-60,30,"All Screens");
  window.b4->callback(allscreens_cb,w);
  y+=30;

  Fl_Button eb(50,y,window.w()-60,30,"Exit");
  eb.callback(exit_cb);
  y+=30;

  Fl_Browser *browser = new Fl_Browser(50,y,window.w()-60,100);
  update_screeninfo(0, browser);
  y+=100;
  
  Fl_Button update(50,y,window.w()-60,30,"Update");
  update.callback(update_screeninfo, browser);
  y+=30;

  if (initfull) {window.b3->set(); window.b3->do_callback();}

  window.end();
  window.show(argc,argv);
  
  return Fl::run();
}

//
// End of "$Id$".
//
