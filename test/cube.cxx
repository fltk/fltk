//
// OpenGL test program for the Fast Light Tool Kit (FLTK).
//
// Modified to have 2 cubes to test multiple OpenGL contexts
//
// Copyright 1998-2024 by Bill Spitzak and others.
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

// Define 'HAVE_GL=1' on the compiler commandline to build this program
// w/o 'config.h' (needs FLTK lib with GL), for instance like:
//   $ fltk-config --use-gl --compile cube.cxx -DHAVE_GL=1
// Use '-DHAVE_GL=0' to build and test w/o OpenGL support.

#ifndef HAVE_GL
#include <config.h> // needed only for 'HAVE_GL'
#endif

// ... or uncomment the next line to test w/o OpenGL (see also above)
// #undef HAVE_GL

#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Radio_Light_Button.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Sys_Menu_Bar.H>
#include <FL/Fl_Printer.H>      // demo printing
#include <FL/Fl_Grid.H>         // grid layout

// Glocal constants and variables

const double fps      = 25.0;     // desired frame rate (independent of speed slider)
const double delay    = 1.0/fps;  // calculated timer delay
int count             = -2;       // initialize loop (draw) counter
int done              =  0;       // set to 1 in exit button callback
Fl_Timestamp start;               // taken at start of main or after reset

#define DEBUG_TIMER (0)           // >0: detailed debugging, see code (should be 0)

#if (DEBUG_TIMER)
const double dx       = 0.001;    // max. allowed deviation from delta time
const double d_min    = delay - dx;
const double d_max    = delay + dx;
#endif // DEBUG_TIMER

// Global pointers to widgets

class cube_box;

Fl_Window *form;
Fl_Slider *speed, *size;
Fl_Button *exit_button, *wire, *flat;
cube_box  *lt_cube, *rt_cube;

#if !HAVE_GL
class cube_box : public Fl_Box {
public:
  double lasttime;
  int wire;
  double size;
  double speed;
  cube_box(int x,int y,int w,int h,const char *l=0) : Fl_Box(FL_DOWN_BOX,x,y,w,h,l) {
    label("This demo does\nnot work without GL");
  }
};
#else
#include <FL/Fl_Gl_Window.H>
#include <FL/gl.h>

class cube_box : public Fl_Gl_Window {
  void draw() FL_OVERRIDE;
  int handle(int) FL_OVERRIDE;
public:
  double lasttime;
  int wire;
  double size;
  double speed;
  cube_box(int x,int y,int w,int h,const char *l=0) : Fl_Gl_Window(x,y,w,h,l) {
    end();
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
  lasttime = lasttime + speed;
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

  // draw additional FLTK widgets and graphics
  Fl_Gl_Window::draw();
}

int cube_box::handle(int e) {
  switch (e) {
    case FL_ENTER: cursor(FL_CURSOR_CROSS);   break;
    case FL_LEAVE: cursor(FL_CURSOR_DEFAULT); break;
  }
  return Fl_Gl_Window::handle(e);
}

// callback for overlay button (Fl_Button on OpenGL scene)
void show_info_cb(Fl_Widget*, void*) {
  fl_message("This is an example of using FLTK widgets inside OpenGL windows.\n"
             "Multiple widgets can be added to Fl_Gl_Windows. They will be\n"
             "rendered as overlays over the scene.");
}

// overlay a button onto an OpenGL window (cube_box)
// but don't change the current group Fl_Group::current()
void overlay_button(cube_box *cube) {
  Fl_Group *curr = Fl_Group::current();
  Fl_Group::current(0);
  Fl_Widget *w = new Fl_Button(10, 10, 120, 30, "FLTK over GL");
  w->color(FL_FREE_COLOR);
  w->box(FL_BORDER_BOX);
  w->callback(show_info_cb);
  cube->add(w);
  Fl_Group::current(curr);
}

#endif // HAVE_GL

void exit_cb(Fl_Widget *w = NULL, void *v = NULL) {

#if HAVE_GL

  // display performance data on stdout and (for Windows!) in a message window

  double runtime = Fl::seconds_since(start);
  char buffer[120];

  sprintf(buffer, "Count =%5d, time = %7.3f sec, fps = %5.2f, requested: %5.2f",
          count, runtime, count / runtime, fps);
  printf("%s\n", buffer);
  fflush(stdout);

  int choice = fl_choice("%s", "E&xit", "&Continue", "&Reset", buffer);
  switch(choice) {
    case 0:                     // exit program, close all windows
      done = 1;
      Fl::hide_all_windows();
      break;
    case 2:                     // reset
      count = -2;
      printf("*** RESET ***\n");
      fflush(stdout);
      break;
    default:                    // continue
      break;
  }

#else
  done = 1;
  Fl::hide_all_windows();
#endif
}

#if HAVE_GL

void timer_cb(void *data) {
  static Fl_Timestamp last = Fl::now();
  static Fl_Timestamp now;
  count++;
  if (count == 0) {
    start = Fl::now();
    last  = start;
  } else if (count > 0) {
    now = Fl::now();
#if (DEBUG_TIMER)
    double delta = Fl::seconds_since(last);
    if ((delta < d_min || delta > d_max) && count > 1) {
      printf("Count =%5d, delay should be %7.5f but %7.5f is outside [%7.5f .. %7.5f]\n",
             count, delay, delta, d_min, d_max);
      fflush(stdout);
    }
#endif // DEBUG_TIMER
  }

#if (DEBUG_TIMER > 1)
  if (count > 0 && count <= DEBUG_TIMER) { // log the first N iterations
    double runtime = Fl::seconds_since(start);
    printf("Count =%5d, time = %7.3f sec, fps = %5.2f, requested: %5.2f\n",
            count, runtime, count / runtime, fps);
    fflush(stdout);
  }
#endif // (DEBUG_TIMER > 1)

  lt_cube->redraw();
  rt_cube->redraw();
  Fl::repeat_timeout(delay, timer_cb);
  last = Fl::now();
}

#endif // HAVE_GL

void speed_cb(Fl_Widget *w, void *) {
  lt_cube->speed = rt_cube->speed = ((Fl_Slider *)w)->value();
}

void size_cb(Fl_Widget *w, void *) {
  lt_cube->size = rt_cube->size = ((Fl_Slider *)w)->value();
}

void flat_cb(Fl_Widget *w, void *) {
  int flat = ((Fl_Light_Button *)w)->value();
  lt_cube->wire = 1 - flat;
  rt_cube->wire = flat;
}

void wire_cb(Fl_Widget *w, void *) {
  int wire = ((Fl_Light_Button *)w)->value();
  lt_cube->wire = wire;
  rt_cube->wire = 1 - wire;
}

// print screen demo
void print_cb(Fl_Widget *w, void *data) {
  Fl_Printer printer;
  Fl_Window *win = Fl::first_window();
  if (!win) return;
  if (printer.start_job(1)) return;
  if (printer.start_page()) return;
  printer.scale(0.5, 0.5);
  printer.print_widget(win);
  printer.end_page();
  printer.end_job();
}

// Create a form that allows resizing for A and C (GL windows) with B fixed size/centered:
//
//      |<--------------------------------------->|<---------------------->|
//      .          lt_cube            center      :       rt_cube          .
//      .            350                100       :         350            .
//      .  |<------------------->|  |<-------->|  |<------------------->|  .
//      ....................................................................
//      :  .......................  ............  .......................  :  __
//      :  :                     :  :          :  :                     :  :
//      :  :          A          :  :    B     :  :          C          :  :     h = 350
//      :  :                     :  :          :  :                     :  :
//      :  :.....................:  :..........:  :.....................:  :  __
//      :..................................................................:  __ MARGIN
//
//      |  |                     |  |          |  |
//     MARGIN                    GAP           GAP

#define MENUBAR_H 25    // menubar height
#define MARGIN    20    // fixed margin around widgets
#define GAP       20    // fixed gap between widgets

void makeform(const char *name) {
  // Widget's XYWH's
  int form_w = 800 + 2 * MARGIN + 2 * GAP;   // main window width
  int form_h = 350 + MENUBAR_H + 2 * MARGIN; // main window height

  // main window
  form = new Fl_Window(form_w, form_h, name);
  form->callback(exit_cb);
  // menu bar
  Fl_Sys_Menu_Bar *menubar = new Fl_Sys_Menu_Bar(0, 0, form_w, MENUBAR_H);
  menubar->add("File/Print window", FL_COMMAND+'p', print_cb);
  menubar->add("File/Quit",         FL_COMMAND+'q', exit_cb);

  // Fl_Grid (layout)
  Fl_Grid *grid = new Fl_Grid(0, MENUBAR_H, form_w, 350 + 2 * MARGIN);
  grid->layout(4, 4, MARGIN, GAP);
  grid->box(FL_FLAT_BOX);

  // set column and row weights to control resizing behavior
  int cwe[] = {50,  0,  0, 50}; // column weights
  int rwe[] = { 0,  0, 50,  0}; // row weights
  grid->col_weight(cwe, 4);     // set weights for resizing
  grid->row_weight(rwe, 4);     // set weights for resizing

  // set non-default gaps for special layout purposes and labels
  grid->row_gap(0,  0);         // no gap below wire button
  grid->row_gap(2, 50);         // gap below sliders for labels

  // left GL window
  lt_cube = new cube_box(0, 0, 350, 350);

  // center group
  wire  = new Fl_Radio_Light_Button(    0, 0, 100, 25, "Wire");
  flat  = new Fl_Radio_Light_Button(    0, 0, 100, 25, "Flat");
  speed = new Fl_Slider(FL_VERT_SLIDER, 0, 0,  40, 90, "Speed");
  size  = new Fl_Slider(FL_VERT_SLIDER, 0, 0,  40, 90, "Size");
#if HAVE_GL
  exit_button = new Fl_Button(          0, 0, 100, 25, "Stats / E&xit");
#else
  exit_button = new Fl_Button(          0, 0, 100, 25, "E&xit");
#endif
  exit_button->callback(exit_cb);
  exit_button->tooltip("Display statistics (fps) and\nchoose to exit or continue\n");

  // right GL window
  rt_cube = new cube_box(0, 0, 350, 350);

  // assign widgets to grid positions (R=row, C=col) and sizes
  // RS=rowspan, CS=colspan: R, C, RS, CS, optional alignment
  grid->widget(lt_cube,      0, 0,  4,  1);
  grid->widget(wire,         0, 1,  1,  2);
  grid->widget(flat,         1, 1,  1,  2);
  grid->widget(speed,        2, 1,  1,  1, FL_GRID_VERTICAL);
  grid->widget(size,         2, 2,  1,  1, FL_GRID_VERTICAL);
  grid->widget(exit_button,  3, 1,  1,  2);
  grid->widget(rt_cube,      0, 3,  4,  1);

#if HAVE_GL
  overlay_button(lt_cube);  // overlay a button onto the OpenGL window
#endif // HAVE_GL

  form->end();
  form->resizable(grid);
  form->size_range(form->w(), form->h()); // minimum window size
}

int main(int argc, char **argv) {
  Fl::use_high_res_GL(1);
  Fl::set_color(FL_FREE_COLOR, 255, 255, 0, 75);
  makeform(argv[0]);

  speed->bounds(6, 0);
  speed->value(lt_cube->speed = rt_cube->speed = 2.0);
  speed->callback(speed_cb);

  size->bounds(4, 0.2);
  size->value(lt_cube->size = rt_cube->size = 2.0);
  size->callback(size_cb);

  flat->value(1);
  flat->callback(flat_cb);
  wire->value(0);
  wire->callback(wire_cb);

  form->label("Cube Demo");
  form->show(argc,argv);
  lt_cube->show();
  rt_cube->show();

  lt_cube->wire  = wire->value();
  rt_cube->wire  = !wire->value();
  lt_cube->size  = rt_cube->size = size->value();
  lt_cube->speed = rt_cube->speed = speed->value();
  lt_cube->redraw();
  rt_cube->redraw();

#if HAVE_GL

#if (0) // share OpenGL contexts

  // This demonstrates how to manipulate OpenGL contexts.
  // In this case the same context is used by multiple windows (I'm not
  // sure if this is allowed on Win32, can somebody check?).
  // This fixes a bug on the XFree86 3.0 OpenGL where only one context per
  // program seems to work, but there are probably better uses for this!

  lt_cube->make_current();              // causes context to be created
  rt_cube->context(lt_cube->context()); // share the contexts

#endif // share OpenGL contexts

  // with GL: use a timer for drawing and measure performance

  form->wait_for_expose();
  Fl::add_timeout(0.1, timer_cb);      // start timer
  int ret = Fl::run();
  return ret;

#else // w/o GL: no timer, no performance report

  return Fl::run();

#endif
}
