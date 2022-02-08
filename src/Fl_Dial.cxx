//
// Circular dial widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
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

#include <FL/Fl.H>
#include <FL/Fl_Dial.H>
#include <FL/Fl_Fill_Dial.H>
#include <FL/Fl_Line_Dial.H>
#include <FL/fl_draw.H>
#include <stdlib.h>
#include <FL/math.h>


// All angles are measured with 0 to the right and counter-clockwise
/**
  Draws dial at given position and size.
  \param[in] X, Y, W, H position and size
*/
void Fl_Dial::draw(int X, int Y, int W, int H) {
  if (damage()&FL_DAMAGE_ALL) draw_box(box(), X, Y, W, H, color());
  X += Fl::box_dx(box());
  Y += Fl::box_dy(box());
  W -= Fl::box_dw(box());
  H -= Fl::box_dh(box());
  double angle = (a2-a1)*(value()-minimum())/(maximum()-minimum()) + a1;
  if (type() == FL_FILL_DIAL) {
    // foo: draw this nicely in certain round box types
    int foo = (box() > _FL_ROUND_UP_BOX && Fl::box_dx(box()));
    if (foo) {X--; Y--; W+=2; H+=2;}
    if (active_r()) fl_color(color());
    else fl_color(fl_inactive(color()));
    fl_pie(X, Y, W, H, 270-a1, angle > a1 ? 360+270-angle : 270-360-angle);
    if (active_r()) fl_color(selection_color());
    else fl_color(fl_inactive(selection_color()));
    fl_pie(X, Y, W, H, 270-angle, 270-a1);
    if (foo) {
      if (active_r()) fl_color(FL_FOREGROUND_COLOR);
      else fl_color(fl_inactive(FL_FOREGROUND_COLOR));
      fl_arc(X, Y, W, H, 0, 360);
    }
    return;
  }
  if (!(damage()&FL_DAMAGE_ALL)) {
    if (active_r()) fl_color(color());
    else fl_color(fl_inactive(color()));
    fl_pie(X+1, Y+1, W-2, H-2, 0, 360);
  }
  fl_push_matrix();
  fl_translate(X+W/2-.5, Y+H/2-.5);
  fl_scale(W-1, H-1);
  fl_rotate(45-angle);
  if (active_r()) fl_color(selection_color());
  else fl_color(fl_inactive(selection_color()));
  if (type()) { // FL_LINE_DIAL
    fl_begin_polygon();
    fl_vertex(0.0,   0.0);
    fl_vertex(-0.04, 0.0);
    fl_vertex(-0.25, 0.25);
    fl_vertex(0.0,   0.04);
    fl_end_polygon();
    if (active_r()) fl_color(FL_FOREGROUND_COLOR);
    else fl_color(fl_inactive(FL_FOREGROUND_COLOR));
    fl_begin_loop();
    fl_vertex(0.0,   0.0);
    fl_vertex(-0.04, 0.0);
    fl_vertex(-0.25, 0.25);
    fl_vertex(0.0,   0.04);
    fl_end_loop();
  } else {
    fl_begin_polygon(); fl_circle(-0.20, 0.20, 0.07); fl_end_polygon();
    if (active_r()) fl_color(FL_FOREGROUND_COLOR);
    else fl_color(fl_inactive(FL_FOREGROUND_COLOR));
    fl_begin_loop(); fl_circle(-0.20, 0.20, 0.07); fl_end_loop();
  }
  fl_pop_matrix();
}

/**
  Draws dial at current position and size.
*/
void Fl_Dial::draw() {
  draw(x(), y(), w(), h());
  draw_label();
}

/**
  Allows subclasses to handle event based on given position and size.
  \param[in] event, X, Y, W, H event to handle, related position and size.
*/
int Fl_Dial::handle(int event, int X, int Y, int W, int H) {
  switch (event) {
  case FL_PUSH:
    handle_push();
    /* FALLTHROUGH */
  case FL_DRAG: {
    int mx = (Fl::event_x()-X-W/2)*H;
    int my = (Fl::event_y()-Y-H/2)*W;
    if (!mx && !my) return 1;
    double angle = 270-atan2((float)-my, (float)mx)*180/M_PI;
    double oldangle = (a2-a1)*(value()-minimum())/(maximum()-minimum()) + a1;
    while (angle < oldangle-180) angle += 360;
    while (angle > oldangle+180) angle -= 360;
    double val;
    if ((a1<a2) ? (angle <= a1) : (angle >= a1)) {
      val = minimum();
    } else if ((a1<a2) ? (angle >= a2) : (angle <= a2)) {
      val = maximum();
    } else {
      val = minimum() + (maximum()-minimum())*(angle-a1)/(a2-a1);
    }
    handle_drag(clamp(round(val)));
    return 1;
  } /* NOTREACHED */
  case FL_RELEASE:
    handle_release();
    return 1;
  case FL_ENTER: /* FALLTHROUGH */
  case FL_LEAVE:
    return 1;
  default:
    return 0;
  }
}

/**
  Allow subclasses to handle event based on current position and size.
*/
int Fl_Dial::handle(int e) {
  return handle(e, x(), y(), w(), h());
}

Fl_Dial::Fl_Dial(int X, int Y, int W, int H, const char* l)
/**
  Creates a new Fl_Dial widget using the given position, size,
  and label string. The default type is FL_NORMAL_DIAL.
*/
: Fl_Valuator(X, Y, W, H, l) {
  box(FL_OVAL_BOX);
  selection_color(FL_INACTIVE_COLOR); // was 37
  a1 = 45;
  a2 = 315;
}


Fl_Fill_Dial::Fl_Fill_Dial(int X,int Y,int W,int H, const char *L)
: Fl_Dial(X,Y,W,H,L) {
  type(FL_FILL_DIAL);
}


Fl_Line_Dial::Fl_Line_Dial(int X,int Y,int W,int H, const char *L)
: Fl_Dial(X,Y,W,H,L) {
  type(FL_LINE_DIAL);
}
