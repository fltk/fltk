//
// "$Id: Fl_Scrollbar.cxx,v 1.4 1998/10/21 14:20:21 mike Exp $"
//
// Scroll bar widget for the Fast Light Tool Kit (FLTK).
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
#include <FL/Fl_Scrollbar.H>
#include <FL/fl_draw.H>
#include <math.h>

#define INITIALREPEAT .5
#define REPEAT .05

void Fl_Scrollbar::increment_cb() {
  handle_drag(clamp(value() + (
	((pushed_>1) == (maximum()>=minimum())) ? linesize_ : -linesize_)));
}

void Fl_Scrollbar::timeout_cb(void* v) {
  Fl_Scrollbar* s = (Fl_Scrollbar*)v;
  s->increment_cb();
  Fl::add_timeout(REPEAT, timeout_cb, s);
}

int Fl_Scrollbar::handle(int event) {
  if (!pushed_) {
    if (horizontal()) {
      if (w() < 3*h()) return Fl_Slider::handle(event);
      if (Fl_Slider::handle(event, x()+h(), y(), w()-2*h(), h())) return 1;
    } else {
      if (h() < 3*w()) return Fl_Slider::handle(event);
      if (Fl_Slider::handle(event, x(), y()+w(), w(), h()-2*w())) return 1;
    }
  }
  switch (event) {
  case FL_RELEASE:
    if (pushed_) {
      Fl::remove_timeout(timeout_cb, this);
      pushed_ = 0;
      redraw();
    }
    handle_release();
    return 1;
  case FL_PUSH:
    if (horizontal()) {
      if (Fl::event_inside(x(), y(), h(), h())) pushed_ = 1;
      if (Fl::event_inside(x()+w()-h(), y(), h(), h())) pushed_ = 2;
    } else {
      if (Fl::event_inside(x(), y(), w(), w())) pushed_ = 1;
      if (Fl::event_inside(x(), y()+h()-w(), w(), w())) pushed_ = 2;
    }
    if (pushed_) {
      handle_push();
      Fl::add_timeout(INITIALREPEAT, timeout_cb, this);
      increment_cb();
      redraw();
    }
    return 1;
  case FL_DRAG:
    return pushed_;
  case FL_SHORTCUT: {
    int v = value();
    int ls = maximum()>=minimum() ? linesize_ : -linesize_;
    if (horizontal()) {
      switch (Fl::event_key()) {
      case FL_Left:
	v -= ls;
	break;
      case FL_Right:
	v += ls;
	break;
      default:
	return 0;
      }
    } else { // vertical
      switch (Fl::event_key()) {
      case FL_Up:
	v -= ls;
	break;
      case FL_Down:
	v += ls;
	break;
      case FL_Page_Up:
	if (slider_size() >= 1.0) return 0;
	v -= int((maximum()-minimum())*slider_size()/(1.0-slider_size()));
	v += ls;
	break;
      case FL_Page_Down:
	if (slider_size() >= 1.0) return 0;
	v += int((maximum()-minimum())*slider_size()/(1.0-slider_size()));
	v -= ls;
	break;
      case FL_Home:
	v = int(minimum());
	break;
      case FL_End:
	v = int(maximum());
	break;
      default:
	return 0;
      }
    }
    v = int(clamp(v));
    if (v != value()) {
      Fl_Slider::value(v);
      value_damage();
      do_callback();
    }
    return 1;}
  }
  return 0;
}

void Fl_Scrollbar::draw() {
  if (horizontal()) {
    if (w() < 3*h()) {Fl_Slider::draw(); return;}
    Fl_Slider::draw(x()+h(), y(), w()-2*h(), h());
    if (damage()&FL_DAMAGE_ALL) {
      draw_box((pushed_&1) ? down(slider()) : slider(),
	       x(), y(), h(), h(), selection_color());
      draw_box((pushed_&2) ? down(slider()) : slider(),
		  x()+w()-h(), y(), h(), h(), selection_color());
      fl_color(labelcolor());
      int w1 = (h()-1)|1; // use odd sizes only
      int Y = y()+w1/2;
      int W = w1/3;
      int X = x()+w1/2+W/2;
      fl_polygon(X-W, Y, X, Y-W, X, Y+W);
      X = x()+w()-(X-x())-1;
      fl_polygon(X+W, Y, X, Y+W, X, Y-W);
    }
  } else { // vertical
    if (h() < 3*w()) {Fl_Slider::draw(); return;}
    Fl_Slider::draw(x(), y()+w(), w(), h()-2*w());
    if (damage()&FL_DAMAGE_ALL) {
      draw_box((pushed_&1) ? down(slider()) : slider(),
	       x(), y(), w(), w(), selection_color());
      draw_box((pushed_&2) ? down(slider()) : slider(),
	       x(), y()+h()-w(), w(), w(), selection_color());
      fl_color(labelcolor());
      int w1 = (w()-1)|1; // use odd sizes only
      int X = x()+w1/2;
      int W = w1/3;
      int Y = y()+w1/2+W/2;
      fl_polygon(X, Y-W, X+W, Y, X-W, Y);
      Y = y()+h()-(Y-y())-1;
      fl_polygon(X, Y+W, X-W, Y, X+W, Y);
    }
  }
}

Fl_Scrollbar::Fl_Scrollbar(int X, int Y, int W, int H, const char* L)
  : Fl_Slider(X, Y, W, H, L)
{
  box(FL_FLAT_BOX);
  color(FL_DARK2);
  slider(FL_UP_BOX);
  linesize_ = 16;
  pushed_ = 0;
  step(1);
}

//
// End of "$Id: Fl_Scrollbar.cxx,v 1.4 1998/10/21 14:20:21 mike Exp $".
//
