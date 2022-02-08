//
// Adjuster widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2011 by Bill Spitzak and others.
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
#include <FL/Fl_Adjuster.H>
#include <FL/Fl_Bitmap.H>
#include <FL/fl_draw.H>

#include "fastarrow.h"
static Fl_Bitmap fastarrow(fastarrow_bits, fastarrow_width, fastarrow_height);
#include "mediumarrow.h"
static Fl_Bitmap mediumarrow(mediumarrow_bits, mediumarrow_width, mediumarrow_height);
#include "slowarrow.h"
static Fl_Bitmap slowarrow(slowarrow_bits, slowarrow_width, slowarrow_height);

// changing the value does not change the appearance:
void Fl_Adjuster::value_damage() {}

void Fl_Adjuster::draw() {
  int dx, dy, W, H;
  if (w()>=h()) {
    dx = W = w()/3;
    dy = 0; H = h();
  } else {
    dx = 0; W = w();
    dy = H = h()/3;
  }
  draw_box(drag==1?FL_DOWN_BOX:box(), x(),  y()+2*dy, W, H, color());
  draw_box(drag==2?FL_DOWN_BOX:box(), x()+dx, y()+dy, W, H, color());
  draw_box(drag==3?FL_DOWN_BOX:box(), x()+2*dx,  y(), W, H, color());
  if (active_r())
    fl_color(selection_color());
  else
    fl_color(fl_inactive(selection_color()));
  fastarrow.draw(x()+(W-fastarrow_width)/2,
                 y()+2*dy+(H-fastarrow_height)/2, W, H);
  mediumarrow.draw(x()+dx+(W-mediumarrow_width)/2,
                   y()+dy+(H-mediumarrow_height)/2, W, H);
  slowarrow.draw(x()+2*dx+(W-slowarrow_width)/2,
                 y()+(H-slowarrow_width)/2, W, H);
  if (Fl::focus() == this) draw_focus();
}

int Fl_Adjuster::handle(int event) {
  double v;
  int delta;
  int mx = Fl::event_x();
  // Fl_Widget_Tracker wp(this);
  switch (event) {
    case FL_PUSH:
      if (Fl::visible_focus()) Fl::focus(this);
      ix = mx;
      if (w()>=h())
        drag = 3*(mx-x())/w() + 1;
      else
        drag = 3-3*(Fl::event_y()-y()-1)/h();
      { Fl_Widget_Tracker wp(this);
        handle_push();
        if (wp.deleted()) return 1;
      }
      redraw();
      return 1;
    case FL_DRAG:
      if (w() >= h()) {
        delta = x()+(drag-1)*w()/3;     // left edge of button
        if (mx < delta)
          delta = mx-delta;
        else if (mx > (delta+w()/3)) // right edge of button
          delta = mx-delta-w()/3;
        else
          delta = 0;
      } else {
        if (mx < x())
          delta = mx-x();
        else if (mx > (x()+w()))
          delta = mx-x()-w();
        else
          delta = 0;
      }
      switch (drag) {
        case 3: v = increment(previous_value(), delta); break;
        case 2: v = increment(previous_value(), delta*10); break;
        default:v = increment(previous_value(), delta*100); break;
      }
      handle_drag(soft() ? softclamp(v) : clamp(v));
      return 1;
    case FL_RELEASE:
      if (Fl::event_is_click()) { // detect click but no drag
        if (Fl::event_state()&0xF0000) delta = -10;
        else delta = 10;
        switch (drag) {
          case 3: v = increment(previous_value(), delta); break;
          case 2: v = increment(previous_value(), delta*10); break;
          default:v = increment(previous_value(), delta*100); break;
        }
        Fl_Widget_Tracker wp(this);
        handle_drag(soft() ? softclamp(v) : clamp(v));
        if (wp.deleted()) return 1;
      }
      drag = 0;
      redraw();
      handle_release();
      return 1;
    case FL_KEYBOARD :
      switch (Fl::event_key()) {
        case FL_Up:
          if (w() > h()) return 0;
          handle_drag(clamp(increment(value(),-1)));
          return 1;
        case FL_Down:
          if (w() > h()) return 0;
          handle_drag(clamp(increment(value(),1)));
          return 1;
        case FL_Left:
          if (w() < h()) return 0;
          handle_drag(clamp(increment(value(),-1)));
          return 1;
        case FL_Right:
          if (w() < h()) return 0;
          handle_drag(clamp(increment(value(),1)));
          return 1;
        default:
          return 0;
      }
      // break not required because of switch...

    case FL_FOCUS:
    case FL_UNFOCUS:
      if (Fl::visible_focus()) {
        redraw();
        return 1;
      } else return 0;

    case FL_ENTER :
    case FL_LEAVE :
      return 1;
  }
  return 0;
}

/**
  Creates a new Fl_Adjuster widget using the given position,
  size, and label string. It looks best if one of the dimensions is 3
  times the other.
  <P> Inherited destructor destroys the Valuator.
*/
Fl_Adjuster::Fl_Adjuster(int X, int Y, int W, int H, const char* l)
  : Fl_Valuator(X, Y, W, H, l) {
  box(FL_UP_BOX);
  step(1, 10000);
  selection_color(FL_SELECTION_COLOR);
  drag = 0;
  soft_ = 1;
}
