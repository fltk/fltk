//
// "$Id: Fl_Scrollbar.cxx,v 1.7.2.3 1999/12/07 17:53:08 bill Exp $"
//
// Scroll bar widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-1999 by Bill Spitzak and others.
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
  int i;
  int W = horizontal() ? w() : h();
  int S = int(slider_size()*W+.5);

  switch (pushed_) {
  case 1: i = -linesize_; break;
  default:i =  linesize_; break;
  case 3: i = -int(S * (maximum() - minimum()) / W); break;
  case 4: i =  int(S * (maximum() - minimum()) / W); break;
  }
  if (maximum() < minimum() && pushed_ < 3) i = -i;
  handle_drag(clamp(value() + i));
}

void Fl_Scrollbar::timeout_cb(void* v) {
  Fl_Scrollbar* s = (Fl_Scrollbar*)v;
  s->increment_cb();
  Fl::add_timeout(REPEAT, timeout_cb, s);
}

int Fl_Scrollbar::handle(int event) {
  // area of scrollbar:
  int area;
  int X=x(); int Y=y(); int W=w(); int H=h();
  int SX = X; int SY = Y; int SW = W; int SH = H;

  // adjust slider area to be inside the arrow buttons:
  if (horizontal()) {
    if (W >= 3*H) {X += H; W -= 2*H;}
  } else {
    if (H >= 3*W) {Y += W; H -= 2*W;}
  }

  // which widget part is highlighted?
  int mx = Fl::event_x();
  int my = Fl::event_y();
  if (!Fl::event_inside(SX, SY, SW, SH)) area = 0;
  else if (horizontal()) {
    if (mx < X) area = 1;
    else if (mx >= X+W) area = 2;
    else {
      int sliderx;
      int S = int(slider_size()*W+.5);
      double val = (value()-minimum())/(maximum()-minimum());
      if (val >= 1.0) sliderx = W-S;
      else if (val <= 0.0) sliderx = 0;
      else sliderx = int(val*(W-S)+.5);

      if (mx < X+sliderx) area = 3;
      else if (mx >= X+sliderx+S) area = 4;
      else area = 5;
    }
  } else {
    if (mx < X || mx >= X+W) area = 0;
    else if (my < Y) area = 1;
    else if (my >= Y+H) area = 2;
    else {
      int slidery;
      int S = int(slider_size()*H+.5);
      double val = (value()-minimum())/(maximum()-minimum());
      if (val >= 1.0) slidery = H-S;
      else if (val <= 0.0) slidery = 0;
      else slidery = int(val*(H-S)+.5);

      if (my < Y+slidery) area = 3;
      else if (my >= Y+slidery+S) area = 4;
      else area = 5;
    }
  }
  switch (event) {
  case FL_ENTER:
  case FL_LEAVE:
    return 1;
  case FL_RELEASE:
      damage(FL_DAMAGE_EXPOSE);
    if (pushed_) {
      Fl::remove_timeout(timeout_cb, this);
      pushed_ = 0;
    }
    handle_release();
    return 1;
  case FL_PUSH:
    if (pushed_) return 1;
    if (area != 5) pushed_ = area;
    if (pushed_) {
      handle_push();
      Fl::add_timeout(INITIALREPEAT, timeout_cb, this);
      increment_cb();
      damage(FL_DAMAGE_EXPOSE);
      return 1;
    }
    return Fl_Slider::handle(event, X,Y,W,H);
  case FL_DRAG:
    if (pushed_) return 1;
    return Fl_Slider::handle(event, X,Y,W,H);
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
  if (damage()&FL_DAMAGE_ALL) draw_box();
  int X = x()+Fl::box_dx(box());
  int Y = y()+Fl::box_dy(box());
  int W = w()-Fl::box_dw(box());
  int H = h()-Fl::box_dh(box());
  if (horizontal()) {
    if (W < 3*H) {Fl_Slider::draw(X,Y,W,H); return;}
    Fl_Slider::draw(X+H,Y,W-2*H,H);
    if (damage()&FL_DAMAGE_ALL) {
      draw_box((pushed_&1) ? down(slider()) : slider(),
	       X, Y, H, H, selection_color());
      draw_box((pushed_&2) ? down(slider()) : slider(),
		  X+W-H, Y, H, H, selection_color());
      if (active_r())
        fl_color(labelcolor());
      else
        fl_color(inactive(labelcolor()));
      int w1 = (H-1)|1; // use odd sizes only
      int Y1 = Y+w1/2;
      int W1 = w1/3;
      int X1 = X+w1/2+W1/2;
      fl_polygon(X1-W1, Y1, X1, Y1-W1, X1, Y1+W1);
      X1 = X+W-(X1-X)-1;
      fl_polygon(X1+W1, Y1, X1, Y1+W1, X1, Y1-W1);
    }
  } else { // vertical
    if (H < 3*W) {Fl_Slider::draw(X,Y,W,H); return;}
    Fl_Slider::draw(X,Y+W,W,H-2*W);
    if (damage()&FL_DAMAGE_ALL) {
      draw_box((pushed_&1) ? down(slider()) : slider(),
	       X, Y, W, W, selection_color());
      draw_box((pushed_&2) ? down(slider()) : slider(),
	       X, Y+H-W, W, W, selection_color());
      if (active_r())
        fl_color(labelcolor());
      else
        fl_color(labelcolor() | 8);
      int w1 = (W-1)|1; // use odd sizes only
      int X1 = X+w1/2;
      int W1 = w1/3;
      int Y1 = Y+w1/2+W1/2;
      fl_polygon(X1, Y1-W1, X1+W1, Y1, X1-W1, Y1);
      Y1 = Y+H-(Y1-Y)-1;
      fl_polygon(X1, Y1+W1, X1-W1, Y1, X1+W1, Y1);
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
// End of "$Id: Fl_Scrollbar.cxx,v 1.7.2.3 1999/12/07 17:53:08 bill Exp $".
//
