// Fl_Adjuster.C

// Fltk widget for drag-adjusting a floating point value.

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
  fl_color(selection_color());
  fastarrow.draw(x()+(W-fastarrow_width)/2,
		 y()+2*dy+(H-fastarrow_height)/2, W, H);
  mediumarrow.draw(x()+dx+(W-mediumarrow_width)/2,
		   y()+dy+(H-mediumarrow_height)/2, W, H);
  slowarrow.draw(x()+2*dx+(W-slowarrow_width)/2,
		 y()+(H-slowarrow_width)/2, W, H);
}

int Fl_Adjuster::handle(int event) {
  double v;
  int delta;
  int mx = Fl::event_x();
  switch (event) {
  case FL_PUSH:
    ix = mx;
    if (w()>=h())
      drag = 3*(mx-x())/w() + 1;
    else
      drag = 3-3*(Fl::event_y()-y()-1)/h();
    handle_push();
    redraw();
    return 1;
  case FL_DRAG:
    if (w() >= h()) {
      delta = x()+(drag-1)*w()/3;	// left edge of button
      if (mx < delta)
	delta = mx-delta;
      else if (mx > delta+w()/3) // right edge of button
	delta = mx-delta-w()/3;
      else
	delta = 0;
    } else {
      if (mx < x())
	delta = mx-x();
      else if (mx > x()+w())
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
      handle_drag(soft() ? softclamp(v) : clamp(v));
    }
    drag = 0;
    redraw();
    handle_release();
    return 1;
  }
  return 0;
}

Fl_Adjuster::Fl_Adjuster(int x, int y, int w, int h, const char* l)
  : Fl_Valuator(x, y, w, h, l) {
  box(FL_UP_BOX);
  step(1, 10000);
  selection_color(FL_BLACK);
  drag = 0;
  soft_ = 1;
}
