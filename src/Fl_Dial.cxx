//
// "$Id: Fl_Dial.cxx,v 1.4 1998/10/21 14:20:02 mike Exp $"
//
// Circular dial widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998 by Bill Spitzak and others.
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
// Please report all bugs and problems to "fltk-bugs@easysw.com".
//

#include <FL/Fl.H>
#include <FL/Fl_Dial.H>
#include <FL/fl_draw.H>
#include <stdlib.h>
#include <FL/math.h>

void Fl_Dial::draw(int x, int y, int w, int h) {
  if (damage()&FL_DAMAGE_ALL) draw_box(box(), x, y, w, h, color());
  x += Fl::box_dx(box());
  y += Fl::box_dy(box());
  w -= Fl::box_dw(box());
  h -= Fl::box_dh(box());
  double angle = 270.0*(value()-minimum())/(maximum()-minimum());
  if (type() == FL_FILL_DIAL) {
    double a = angle; if (a < 0) a = 0;
    // foo: draw this nicely in certain round box types
    int foo = (box() > _FL_ROUND_UP_BOX && Fl::box_dx(box()));
    if (foo) {x--; y--; w+=2; h+=2;}
    fl_color(color());
    fl_pie(x, y, w-1, h-1, 225, 225+360-a);
    fl_color(selection_color());
    fl_pie(x, y, w-1, h-1, 225-a, 225);
    if (foo) {
      fl_color(FL_BLACK);
      fl_arc(x, y, w, h, 0, 360);
    }
    return;
  }
  if (!(damage()&FL_DAMAGE_ALL)) {
    fl_color(color());
    fl_pie(x+1, y+1, w-2, h-2, 0, 360);
  }
  fl_push_matrix();
  fl_translate(x+w/2-.5, y+h/2-.5);
  fl_scale(w-1, h-1);
  if (type() == FL_FILL_DIAL) {
    fl_rotate(225);
    fl_begin_line(); fl_vertex(0, 0); fl_vertex(.5, 0); fl_end_line();
  }
  fl_rotate(-angle);
  fl_color(selection_color());
  if (type() == FL_LINE_DIAL) {
    fl_begin_polygon();
    fl_vertex(0.0,   0.0);
    fl_vertex(-0.04, 0.0);
    fl_vertex(-0.25, 0.25);
    fl_vertex(0.0,   0.04);
    fl_end_polygon();
    fl_color(FL_BLACK);
    fl_begin_loop();
    fl_vertex(0.0,   0.0);
    fl_vertex(-0.04, 0.0);
    fl_vertex(-0.25, 0.25);
    fl_vertex(0.0,   0.04);
    fl_end_loop();
  } else {
    fl_begin_polygon(); fl_circle(-0.20, 0.20, 0.07); fl_end_polygon();
    fl_color(FL_BLACK);
    fl_begin_loop(); fl_circle(-0.20, 0.20, 0.07); fl_end_loop();
  }
  fl_pop_matrix();
}

void Fl_Dial::draw() {
  draw(x(), y(), w(), h());
  draw_label();
}

int Fl_Dial::handle(int event, int x, int y, int w, int h) {
  switch (event) {
  case FL_PUSH:
    handle_push();
  case FL_DRAG: {
    double angle;
    double val = value();
    int mx = Fl::event_x()-x-w/2;
    int my = Fl::event_y()-y-h/2;
    if (!mx && !my) return 1;
    if (abs(mx) > abs(my)) {
      angle = atan(-(double)my/mx);
      if (mx>0) angle = 1.25*M_PI - angle;
      else angle = 0.25*M_PI - angle;
    } else {
      angle = atan(-(double)mx/my);
      if (my<0) angle = 0.75*M_PI + angle;
      else angle = -0.25*M_PI + angle;
    }
    if (angle<(-0.25*M_PI)) angle += 2.0*M_PI;
    val = minimum() + (maximum()-minimum())*angle/(1.5*M_PI);
    if (fabs(val-value()) < (maximum()-minimum())/2.0)
      handle_drag(clamp(round(val)));
    } return 1;
  case FL_RELEASE:
    handle_release();
    return 1;
  default:
    return 0;
  }
}

int Fl_Dial::handle(int e) {
  return handle(e, x(), y(), w(), h());
}

Fl_Dial::Fl_Dial(int x, int y, int w, int h, const char* l)
  : Fl_Valuator(x, y, w, h, l) {
  box(FL_OVAL_BOX);
  selection_color(FL_INACTIVE_COLOR); // was 37
}

//
// End of "$Id: Fl_Dial.cxx,v 1.4 1998/10/21 14:20:02 mike Exp $".
//
