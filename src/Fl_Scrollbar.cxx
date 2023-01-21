//
// Scroll bar widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2022 by Bill Spitzak and others.
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
#include <FL/Fl_Scrollbar.H>
#include <FL/fl_draw.H>
#include <math.h>
#include "flstring.h"

#define INITIALREPEAT .5
#define REPEAT .05

void Fl_Scrollbar::increment_cb() {
  char inv = maximum()<minimum();
  int ls = inv ? -linesize_ : linesize_;
  int i;
  switch (pushed_) {
    case 1: // clicked on arrow left
      i = -ls;
      break;
    default: // clicked on arrow right
      i =  ls;
      break;
    case 5: // clicked into the box next to the slider on the left
      i = -(int((maximum()-minimum())*slider_size()/(1.0-slider_size())));
      if (inv) {
        if (i<-ls) i = -ls;
      } else {
        if (i>-ls) i = -ls; // err
      }
      break;
    case 6: // clicked into the box next to the slider on the right
      i = (int((maximum()-minimum())*slider_size()/(1.0-slider_size())));
      if (inv) {
        if (i>ls) i = ls;
      } else {
        if (i<ls) i = ls; // err
      }
      break;
  }
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

  // adjust slider area to be inside the arrow buttons:
  if (horizontal()) {
    if (W >= 3*H) {X += H; W -= 2*H;}
  } else {
    if (H >= 3*W) {Y += W; H -= 2*W;}
  }

  // which widget part is highlighted?
  int relx;
  int ww;
  if (horizontal()) {
    relx = Fl::event_x()-X;
    ww = W;
  } else {
    relx = Fl::event_y()-Y;
    ww = H;
  }
  if (relx < 0) area = 1;
  else if (relx >= ww) area = 2;
  else {
    int S = int(slider_size()*ww+.5);
    int T = (horizontal() ? H : W)/2+1;
    if (type()==FL_VERT_NICE_SLIDER || type()==FL_HOR_NICE_SLIDER) T += 4;
    if (S < T) S = T;
    double val =
      (maximum()-minimum()) ? (value()-minimum())/(maximum()-minimum()) : 0.5;
    int sliderx;
    if (val >= 1.0) sliderx = ww-S;
    else if (val <= 0.0) sliderx = 0;
    else sliderx = int(val*(ww-S)+.5);
    if (Fl::event_button() == FL_MIDDLE_MOUSE) area = 8;
    else if (relx < sliderx) area = 5;
    else if (relx >= sliderx+S) area = 6;
    else area = 8;
  }

  switch (event) {
  case FL_ENTER:
  case FL_LEAVE:
    return 1;
  case FL_RELEASE:
      damage(FL_DAMAGE_ALL);
    if (pushed_) {
      Fl::remove_timeout(timeout_cb, this);
      pushed_ = 0;
    }
    handle_release();
    return 1;
  case FL_PUSH:
    if (pushed_) return 1;
    if (area != 8) pushed_ = area;
    if (pushed_) {
      handle_push();
      Fl::add_timeout(INITIALREPEAT, timeout_cb, this);
      increment_cb();
      damage(FL_DAMAGE_ALL);
      return 1;
    }
    return Fl_Slider::handle(event, X,Y,W,H);
  case FL_DRAG:
    if (pushed_) return 1;
    return Fl_Slider::handle(event, X,Y,W,H);
  case FL_MOUSEWHEEL :
    if (horizontal()) {
      if (Fl::e_dx==0) return 0;
      int ls = maximum()>=minimum() ? linesize_ : -linesize_;
      handle_drag(clamp(value() + ls * Fl::e_dx));
      return 1;
    } else {
      if (Fl::e_dy==0) return 0;
      int ls = maximum()>=minimum() ? linesize_ : -linesize_;
      handle_drag(clamp(value() + ls * Fl::e_dy));
      return 1;
    }
  case FL_SHORTCUT:
  case FL_KEYBOARD: {
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
      set_changed();
      do_callback(FL_REASON_DRAGGED);
    }
    return 1;}
  }
  return 0;
}

void Fl_Scrollbar::draw() {
  if (damage() & FL_DAMAGE_ALL) draw_box();
  int X = x() + Fl::box_dx(box());
  int Y = y() + Fl::box_dy(box());
  int W = w() - Fl::box_dw(box());
  int H = h() - Fl::box_dh(box());
  Fl_Rect ab; // arrow box

  int inset = 2;
  if (W < 8 || H < 8)
    inset = 1;

  if (horizontal()) {
    if (W < 3*H) {
      Fl_Slider::draw(X, Y, W, H);
      return;
    }
    Fl_Slider::draw(X+H, Y, W-2*H, H);
    if (damage()&FL_DAMAGE_ALL) {
      draw_box((pushed_==1) ? fl_down(slider()) : slider(),
               X, Y, H, H, selection_color());
      draw_box((pushed_==2) ? fl_down(slider()) : slider(),
               X+W-H, Y, H, H, selection_color());

      Fl_Color arrowcolor = active_r() ? labelcolor() : fl_inactive(labelcolor());
      ab = Fl_Rect(X, Y, H, H);
      ab.inset(inset);
      fl_draw_arrow(ab, FL_ARROW_SINGLE, FL_ORIENT_LEFT, arrowcolor); // left arrow
      ab = Fl_Rect(X+W-H, Y, H, H);
      ab.inset(inset);
      fl_draw_arrow(ab, FL_ARROW_SINGLE, FL_ORIENT_RIGHT, arrowcolor); // right arrow
    }
  } else { // vertical
    if (H < 3*W) {
      Fl_Slider::draw(X, Y, W, H);
      return;
    }
    Fl_Slider::draw(X, Y+W, W, H-2*W);
    if (damage() & FL_DAMAGE_ALL) {
      draw_box((pushed_==1) ? fl_down(slider()) : slider(),
               X, Y, W, W, selection_color());
      draw_box((pushed_==2) ? fl_down(slider()) : slider(),
               X, Y+H-W, W, W, selection_color());

      Fl_Color arrowcolor = active_r() ? labelcolor() : fl_inactive(labelcolor());
      ab = Fl_Rect(X, Y, W, W);
      ab.inset(inset);
      fl_draw_arrow(ab, FL_ARROW_SINGLE, FL_ORIENT_UP, arrowcolor); // up arrow
      ab = Fl_Rect(X, Y+H-W, W, W);
      ab.inset(inset);
      fl_draw_arrow(ab, FL_ARROW_SINGLE, FL_ORIENT_DOWN, arrowcolor); // down arrow
    }
  }
}

/**
  Creates a new Fl_Scrollbar widget with given position, size, and label.
  You need to do type(FL_HORIZONTAL) if you want a horizontal scrollbar.
*/
Fl_Scrollbar::Fl_Scrollbar(int X, int Y, int W, int H, const char* L)
  : Fl_Slider(X, Y, W, H, L) {
  box(FL_FLAT_BOX);
  color(FL_DARK2);
  slider(FL_UP_BOX);
  linesize_ = 16;
  pushed_ = 0;
  step(1);
}

/**  Destroys the Scrollbar. */
Fl_Scrollbar::~Fl_Scrollbar() {
  if (pushed_)
    Fl::remove_timeout(timeout_cb, this);
}
