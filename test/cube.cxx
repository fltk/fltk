//
// OpenGL test program for the Fast Light Tool Kit (FLTK).
//
// Modified to have 2 cubes to test multiple OpenGL contexts
//
// Copyright 1998-2021 by Bill Spitzak and others.
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

#include <config.h>
#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Radio_Light_Button.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Sys_Menu_Bar.H>
#include <FL/Fl_Printer.H>      // demo printing

#include <stdlib.h>

#if !HAVE_GL
class cube_box : public Fl_Box {
public:
  double lasttime;
  int wire;
  double size;
  double speed;
  cube_box(int x,int y,int w,int h,const char *l=0) :Fl_Box(FL_DOWN_BOX,x,y,w,h,l) {
    label("This demo does\nnot work without GL");
  }
  void begin() {}
  void end() {}
};
#else
#include <FL/Fl_Gl_Window.H>
#include <FL/gl.h>

class cube_box : public Fl_Gl_Window {
  void draw();
  int handle(int);
public:
  double lasttime;
  int wire;
  double size;
  double speed;
  cube_box(int x,int y,int w,int h,const char *l=0) : Fl_Gl_Window(x,y,w,h,l) {
    lasttime = 0.0;
    box(FL_DOWN_FRAME);
  }
};

/* The cube definition */
float v0[3] = {0.0, 0.0, 0.0};
float v1[3] = {1.0, 0.0, 0.0};
float v2[3] = {1.0, 1.0, 0.0};
float v3[3] = {0.0, 1.0, 0.0};
float v4[3] = {0.0, 0.0, 1.0};
float v5[3] = {1.0, 0.0, 1.0};
float v6[3] = {1.0, 1.0, 1.0};
float v7[3] = {0.0, 1.0, 1.0};

#define v3f(x) glVertex3fv(x)

void drawcube(int wire) {
/* Draw a colored cube */
  glBegin(wire ? GL_LINE_LOOP : GL_POLYGON);
  glColor3ub(0,0,255);
  v3f(v0); v3f(v1); v3f(v2); v3f(v3);
  glEnd();
  glBegin(wire ? GL_LINE_LOOP : GL_POLYGON);
  glColor3ub(0,255,255); v3f(v4); v3f(v5); v3f(v6); v3f(v7);
  glEnd();
  glBegin(wire ? GL_LINE_LOOP : GL_POLYGON);
  glColor3ub(255,0,255); v3f(v0); v3f(v1); v3f(v5); v3f(v4);
  glEnd();
  glBegin(wire ? GL_LINE_LOOP : GL_POLYGON);
  glColor3ub(255,255,0); v3f(v2); v3f(v3); v3f(v7); v3f(v6);
  glEnd();
  glBegin(wire ? GL_LINE_LOOP : GL_POLYGON);
  glColor3ub(0,255,0); v3f(v0); v3f(v4); v3f(v7); v3f(v3);
  glEnd();
  glBegin(wire ? GL_LINE_LOOP : GL_POLYGON);
  glColor3ub(255,0,0); v3f(v1); v3f(v2); v3f(v6); v3f(v5);
  glEnd();
}

void cube_box::draw() {
  lasttime = lasttime+speed;
  if (!valid()) {
    glLoadIdentity();
    glViewport(0,0,pixel_w(),pixel_h());
    glEnable(GL_DEPTH_TEST);
    glFrustum(-1,1,-1,1,2,10000);
    glTranslatef(0,0,-10);
    glClearColor(0.4f, 0.4f, 0.4f, 0);
  }
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glPushMatrix();
  glRotatef(float(lasttime*1.6),0,0,1);
  glRotatef(float(lasttime*4.2),1,0,0);
  glRotatef(float(lasttime*2.3),0,1,0);
  glTranslatef(-1.0f, 1.2f, -1.5f);
  glScalef(float(size),float(size),float(size));
  drawcube(wire);
  glPopMatrix();
  gl_color(FL_GRAY);
  glDisable(GL_DEPTH_TEST);
  gl_font(FL_HELVETICA_BOLD, 16 );
  gl_draw(wire ? "Cube: wire" : "Cube: flat", -4.5f, -4.5f );
  glEnable(GL_DEPTH_TEST);

  // if an OpenGL graphics driver is installed, give it a chance
  // to draw additional graphics
#if HAVE_GL
  Fl_Gl_Window::draw();
#endif
}

int cube_box::handle(int e) {
  switch (e) {
    case FL_ENTER: cursor(FL_CURSOR_CROSS);   break;
    case FL_LEAVE: cursor(FL_CURSOR_DEFAULT); break;
  }
  return Fl_Gl_Window::handle(e);
}

#endif

Fl_Window *form;
Fl_Slider *speed, *size;
Fl_Button *exit_button, *wire, *flat;
cube_box  *lt_cube, *rt_cube;

int done = 0; // set to 1 in exit button callback

// exit button callback
void exit_cb(Fl_Widget *, void *) {
  done = 1;
}

// print screen demo
void print_cb(Fl_Widget *w, void *data)
{
  Fl_Printer printer;
  Fl_Window *win = Fl::first_window();
  if(!win) return;
  if( printer.start_job(1) ) return;
  if( printer.start_page() ) return;
  printer.scale(0.5,0.5);
  printer.print_widget( win );
  printer.end_page();
  printer.end_job();
}

// Create a form that allows resizing for A and C (GL windows) with B fixed size/centered:
//
//                  lt_grp                                rt_grp
//      |<--------------------------------------->|<---------------------->|
//      .          lt_cube            ct_grp      :       rt_cube          .
//      .            350                100       :         350            .
//      .  |<------------------->|  |<-------->|  |<------------------->|  .
//      ....................................................................
//      :  .......................  ............  .......................  :
//      :  :                     :  :          :  :                     :  :
//      :  :          A          :  :    B     :  :          C          :  :
//      :  :                     :  :          :  :                     :  :
//      :  :.....................:  :..........:  :.....................:  :  __
//      :..................................................................:  __ MARGIN
//
//      |  |
//     MARGIN
//

#define MENUBAR_H 25            // menubar height
#define MARGIN    20            // fixed margin around widgets
#define MARGIN2   (MARGIN*2)
#define MARGIN3   (MARGIN*3)

void show_info_cb(Fl_Widget*, void*) {
  fl_message("This is an example of using FLTK widgets inside OpenGL windows.\n"
             "Multiple widgets can be added to Fl_Gl_Windows. They will be\n"
             "rendered as overlays over the scene.");
}

void makeform(const char *name) {
  // Widget's XYWH's
  int form_w = 800 + 4 * MARGIN;             // main window width
  int form_h = 350 + MENUBAR_H + 2 * MARGIN; // main window height
  int me_bar_x=0,                    me_bar_y=0,                me_bar_w=form_w,          me_bar_h=MENUBAR_H;                // menubar
  int lt_grp_x=0,                    lt_grp_y=MENUBAR_H+MARGIN, lt_grp_w=350+100+MARGIN3, lt_grp_h=form_h-MENUBAR_H-MARGIN2; // left group
  int lt_cub_x=lt_grp_x+MARGIN,      lt_cub_y=lt_grp_y,         lt_cub_w=350,             lt_cub_h=lt_grp_h;                 // left cube box (GL)
  int ct_grp_x=lt_grp_x+350+MARGIN2, ct_grp_y=lt_grp_y,         ct_grp_w=100,             ct_grp_h=lt_grp_h;                 // center group
  int rt_grp_x=lt_grp_x+lt_grp_w,    rt_grp_y=lt_grp_y,         rt_grp_w=350+MARGIN,      rt_grp_h=lt_grp_h;                 // right group
  int rt_cub_x=rt_grp_x,             rt_cub_y=lt_grp_y,         rt_cub_w=350,             rt_cub_h=lt_grp_h;                 // right cube box (GL)

  // main window
  form = new Fl_Window(form_w, form_h, name);
  form->begin();
    // menu bar
    Fl_Sys_Menu_Bar *menubar = new Fl_Sys_Menu_Bar(me_bar_x, me_bar_y, me_bar_w, me_bar_h);
    menubar->add("File/Print window", FL_COMMAND+'p', print_cb);
    menubar->add("File/Quit",         FL_COMMAND+'q', exit_cb);
    // left group
    Fl_Group *lt_grp = new Fl_Group(lt_grp_x, lt_grp_y, lt_grp_w, lt_grp_h);
    lt_grp->begin();
      // left GL window
      lt_cube = new cube_box(lt_cub_x, lt_cub_y, lt_cub_w, lt_cub_h, 0);

      lt_cube->begin();
      Fl_Widget *w = new Fl_Button(10, 10, 120, 30, "FLTK over GL");
      w->color(FL_FREE_COLOR);
      w->box(FL_BORDER_BOX );
      w->callback(show_info_cb);
      lt_cube->end();

      // center group
      Fl_Group *ct_grp = new Fl_Group(ct_grp_x, ct_grp_y, ct_grp_w, ct_grp_h);
      ct_grp->begin();
        wire  = new Fl_Radio_Light_Button(ct_grp_x, ct_grp_y,            100, 25, "Wire");
        flat  = new Fl_Radio_Light_Button(ct_grp_x, wire->y()+wire->h(), 100, 25, "Flat");
        speed = new Fl_Slider(FL_VERT_SLIDER, ct_grp_x,           flat->y()+flat->h()+MARGIN, 40, 200, "Speed");
        size  = new Fl_Slider(FL_VERT_SLIDER, ct_grp_x+40+MARGIN, flat->y()+flat->h()+MARGIN, 40, 200, "Size");
        exit_button = new Fl_Button(ct_grp_x, form_h-MARGIN-25, 100, 25, "Exit");
        exit_button->callback(exit_cb);
      ct_grp->end();
      ct_grp->resizable(speed);      // only sliders resize vertically, not buttons
    lt_grp->end();
    lt_grp->resizable(lt_cube);
    // right group
    Fl_Group *rt_grp = new Fl_Group(rt_grp_x, rt_grp_y, rt_grp_w, rt_grp_h);
    rt_grp->begin();
      // right GL window
      rt_cube = new cube_box(rt_cub_x, rt_cub_y, rt_cub_w, rt_cub_h, 0);
    rt_grp->end();
    rt_grp->resizable(rt_cube);
    // right resizer
    Fl_Box *rt_resizer = new Fl_Box(rt_grp_x-5, rt_grp_y, 10, rt_grp_h);
    rt_resizer->box(FL_NO_BOX);

  form->end();
  form->resizable(rt_resizer);
  form->size_range(form->w(), form->h()); // minimum window size
}

int main(int argc, char **argv) {
  Fl::use_high_res_GL(1);
  Fl::set_color(FL_FREE_COLOR, 255, 255, 0, 75);
  makeform(argv[0]);
  speed->bounds(4,0);
#if HAVE_GL
  speed->value(lt_cube->speed = rt_cube->speed = 1.0);
#else
  speed->value(lt_cube->speed = rt_cube->speed = 0.0);
#endif
  size->bounds(4,0.01);
  size->value(lt_cube->size = rt_cube->size = 3.0);
  flat->value(1); lt_cube->wire = 0; rt_cube->wire = 1;
  form->label("cube");
  form->show(argc,argv);
  lt_cube->show();
  rt_cube->show();
#if 0
  // This demonstrates how to manipulate OpenGL contexts.
  // In this case the same context is used by multiple windows (I'm not
  // sure if this is allowed on Win32, can somebody check?).
  // This fixes a bug on the XFree86 3.0 OpenGL where only one context
  // per program seems to work, but there are probably better uses for
  // this!
  lt_cube->make_current(); // causes context to be created
  rt_cube->context(lt_cube->context()); // share the contexts
#endif
  for (;;) {
    if (form->visible() && speed->value()) {
      if (!Fl::check()) break;   // returns immediately
    } else {
      if (!Fl::wait()) break;    // waits until something happens
    }
    lt_cube->wire  = wire->value();
    rt_cube->wire  = !wire->value();
    lt_cube->size  = rt_cube->size = size->value();
    lt_cube->speed = rt_cube->speed = speed->value();
    lt_cube->redraw();
    rt_cube->redraw();
    if (done) break; // exit button was clicked
  }
  return 0;
}
