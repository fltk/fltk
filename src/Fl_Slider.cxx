// Fl_Slider.C

#include <FL/Fl.H>
#include <FL/Fl_Slider.H>
#include <FL/fl_draw.H>
#include <math.h>

void Fl_Slider::_Fl_Slider() {
  slider_size_ = 0;
  slider_ = 0; // FL_UP_BOX;
}

Fl_Slider::Fl_Slider(int x, int y, int w, int h, const char* l)
: Fl_Valuator(x, y, w, h, l) {
  box(FL_DOWN_BOX);
  _Fl_Slider();
}

Fl_Slider::Fl_Slider(uchar t, int x, int y, int w, int h, const char* l)
  : Fl_Valuator(x, y, w, h, l) {
  type(t);
  box(t==FL_HOR_NICE_SLIDER || t==FL_VERT_NICE_SLIDER ?
      FL_FLAT_BOX : FL_DOWN_BOX);
  _Fl_Slider();
}

void Fl_Slider::slider_size(double v) {
  if (v <  0) v = 0;
  if (v > 1) v = 1;
  if (slider_size_ != float(v)) {slider_size_ = float(v); damage(2);}
}

void Fl_Slider::bounds(double a, double b) {
  if (minimum() != a || maximum() != b) {Fl_Valuator::bounds(a, b); damage(2);}
}

int Fl_Slider::scrollvalue(int p, int w, int t, int l) {
//	p = position, first line displayed
//	w = window, number of lines displayed
//	t = top, number of first line
//	l = length, total number of lines
  step(1, 1);
  if (p+w > t+l) l = p+w-t;
  slider_size(w >= l ? 1.0 : double(w)/double(l));
  bounds(t, l-w+t);
  return value(p);
}

// All slider interaction is done as though the slider ranges from
// zero to one, and the left (bottom) edge of the slider is at the
// given position.  Since when the slider is all the way to the
// right (top) the left (bottom) edge is not all the way over, a
// position on the widget itself covers a wider range than 0-1,
// actually it ranges from 0 to 1/(1-size).

void Fl_Slider::draw_bg(int x, int y, int w, int h) {
  draw_box(box(), x, y, w, h, color());
  int BW = Fl::box_dx(box());
  if (type() == FL_VERT_NICE_SLIDER) {
    draw_box(FL_THIN_DOWN_BOX, x+w/2-2, y+BW, 4, h-2*BW, FL_BLACK);
  } else if (type() == FL_HOR_NICE_SLIDER) {
    draw_box(FL_THIN_DOWN_BOX, x+BW, y+h/2-2, w-2*BW, 4, FL_BLACK);
  }
}

void Fl_Slider::draw(int x, int y, int w, int h) {
  double val;

  if (minimum() == maximum())
    val = 0.5;
  else {
    val = (value()-minimum())/(maximum()-minimum());
    if (val > 1.0) val = 1.0;
    else if (val < 0.0) val = 0.0;
  }

  int BW = Fl::box_dx(box());
  int W = (horizontal() ? w : h) - 2*BW;
  int X, S;
  if (type()==FL_HOR_FILL_SLIDER || type() == FL_VERT_FILL_SLIDER) {
    S = int(val*W+.5);
    if (minimum()>maximum()) {S = W-S; X = h-BW-S;}
    else X = BW;
  } else {
    S = int(slider_size_*W+.5);
    int T = (horizontal() ? h : w)/2-BW+1;
    if (type()==FL_VERT_NICE_SLIDER || type()==FL_HOR_NICE_SLIDER) T += 4;
    if (S < T) S = T;
    X = BW+int(val*(W-S)+.5);
  }
  int xsl, ysl, wsl, hsl;
  if (horizontal()) {
    xsl = x+X;
    wsl = S;
    ysl = y+BW;
    hsl = h-2*BW;
  } else {
    ysl = y+X;
    hsl = S;
    xsl = x+BW;
    wsl = w-2*BW;
  }

  if (damage()&128) { // complete redraw
    draw_bg(x, y, w, h);
  } else { // partial redraw, clip off new position of slider
    if (X > BW) {
      if (horizontal()) fl_clip(x, ysl, X, hsl);
      else fl_clip(xsl, y, wsl, X);
      draw_bg(x, y, w, h);
      fl_pop_clip();
    }
    if (X+S < W+BW) {
      if (horizontal()) fl_clip(xsl+wsl, ysl, x+w-BW-xsl-wsl, hsl);
      else fl_clip(xsl, ysl+hsl, wsl, y+h-BW-ysl-hsl);
      draw_bg(x, y, w, h);
      fl_pop_clip();
    }
  }

  Fl_Boxtype box1 = slider();
  if (!box1) {box1 = (Fl_Boxtype)(box()&-2); if (!box1) box1 = FL_UP_BOX;}
  if (type() == FL_VERT_NICE_SLIDER) {
    draw_box(box1, xsl, ysl, wsl, hsl, FL_GRAY);
    int d = (hsl-4)/2;
    draw_box(FL_THIN_DOWN_BOX, xsl+2, ysl+d, wsl-4, hsl-2*d,selection_color());
  } else if (type() == FL_HOR_NICE_SLIDER) {
    draw_box(box1, xsl, ysl, wsl, hsl, FL_GRAY);
    int d = (wsl-4)/2;
    draw_box(FL_THIN_DOWN_BOX, xsl+d, ysl+2, wsl-2*d, hsl-4,selection_color());
  } else {
    if (wsl>0 && hsl>0) draw_box(box1, xsl, ysl, wsl, hsl, selection_color());
  }

  draw_label(xsl, ysl, wsl, hsl);
}

void Fl_Slider::draw() {
  draw(x(), y(), w(), h());
}

int Fl_Slider::handle(int event, int x, int y, int w, int h) {
  switch (event) {
  case FL_PUSH:
    if (!Fl::event_inside(x, y, w, h)) return 0;
    handle_push();
  case FL_DRAG: {
    if (slider_size() >= 1 || minimum()==maximum()) return 1;
    int BW = Fl::box_dx(box());
    int W = (horizontal() ? w : h) - 2*BW;
    int X = (horizontal() ? Fl::event_x()-x : Fl::event_y()-y) - BW;
    int S = int(slider_size_*W+.5);
    int T = (horizontal() ? h : w)/2-BW+1;
    if (type()==FL_VERT_NICE_SLIDER || type()==FL_HOR_NICE_SLIDER) T += 4;
    if (S < T) S = T;
    double v = double(X)/(W-S);
    double sliderwidth = double(S)/(W-S);
    double val = (value()-minimum())/(maximum()-minimum());
    static double offcenter;
    if (event == FL_PUSH) {
      offcenter = v-val;
      if (offcenter < 0) offcenter = 0;
      else if (offcenter > sliderwidth) offcenter = sliderwidth;
      else return 1;
    }
  TRY_AGAIN:
    v -= offcenter;
    if (v < 0) {
      offcenter = v+offcenter;
      if (offcenter<0) offcenter=0;
      v = 0;
    } else if (v > 1) {
      offcenter =  v+offcenter-1;
      if (offcenter > sliderwidth) offcenter = sliderwidth;
      v = 1;
    }
    // if (Fl::event_state(FL_SHIFT)) v = val+(v-val)*.05;
    v = round(v*(maximum()-minimum())+minimum());
    // make sure a click outside the sliderbar moves it:
    if (event == FL_PUSH && v == value()) {
      offcenter = sliderwidth/2;
      v = double(X)/(W-S);
      event = FL_DRAG;
      goto TRY_AGAIN;
    }
    handle_drag(clamp(v));
    } return 1;
  case FL_RELEASE:
    handle_release();
    return 1;
  default:
    return 0;
  }
}

int Fl_Slider::handle(int event) {
  return handle(event, x(), y(), w(), h());
}

// End of Fl_Slider.C
