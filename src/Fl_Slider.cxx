//
// Slider widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2015 by Bill Spitzak and others.
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
#include <FL/Fl_Slider.H>
#include <FL/Fl_Fill_Slider.H>
#include <FL/Fl_Hor_Slider.H>
#include <FL/Fl_Hor_Fill_Slider.H>
#include <FL/Fl_Hor_Nice_Slider.H>
#include <FL/Fl_Nice_Slider.H>
#include <FL/fl_draw.H>
#include <math.h>
#include "flstring.h"


void Fl_Slider::_Fl_Slider() {
  slider_size_ = 0;
  slider_ = 0; // FL_UP_BOX;
}

/**
  Creates a new Fl_Slider widget using the given position,
  size, and label string. The default boxtype is FL_DOWN_BOX.
*/
Fl_Slider::Fl_Slider(int X, int Y, int W, int H, const char* L)
: Fl_Valuator(X, Y, W, H, L) {
  box(FL_DOWN_BOX);
  _Fl_Slider();
}

/**
  Creates a new Fl_Slider widget using the given type, position,
  size, and label string.
*/
Fl_Slider::Fl_Slider(uchar t, int X, int Y, int W, int H, const char* L)
  : Fl_Valuator(X, Y, W, H, L) {
  type(t);
  box(t==FL_HOR_NICE_SLIDER || t==FL_VERT_NICE_SLIDER ?
      FL_FLAT_BOX : FL_DOWN_BOX);
  _Fl_Slider();
}

void Fl_Slider::slider_size(double v) {
  if (v <  0) v = 0;
  if (v > 1) v = 1;
  if (slider_size_ != float(v)) {
    slider_size_ = float(v);
    damage(FL_DAMAGE_EXPOSE);
  }
}

/**
  Sets the minimum (a) and maximum (b) values for the valuator widget.
  if at least one of the values is changed, a partial redraw is asked.
*/
void Fl_Slider::bounds(double a, double b) {
  if (minimum() != a || maximum() != b) {
    Fl_Valuator::bounds(a, b);
    damage(FL_DAMAGE_EXPOSE);
  }
}

/**
  Sets the size and position of the sliding knob in the box.
  \param[in] pos position of first line displayed
  \param[in] size size of window in lines
  \param[in] first number of first line
  \param[in] total total number of lines
  Returns Fl_Valuator::value(p)
 */
int Fl_Slider::scrollvalue(int pos, int size, int first, int total) {
  step(1, 1);
  if (pos+size > first+total) total = pos+size-first;
  slider_size(size >= total ? 1.0 : double(size)/double(total));
  bounds(first, total-size+first);
  return value(pos);
}

// All slider interaction is done as though the slider ranges from
// zero to one, and the left (bottom) edge of the slider is at the
// given position.  Since when the slider is all the way to the
// right (top) the left (bottom) edge is not all the way over, a
// position on the widget itself covers a wider range than 0-1,
// actually it ranges from 0 to 1/(1-size).
// S is the size of the slider knob
void Fl_Slider::draw_bg(int X, int Y, int W, int H, int S) {
  fl_push_clip(X, Y, W, H);
  draw_box();
  fl_pop_clip();

  Fl_Color black = active_r() ? FL_FOREGROUND_COLOR : FL_INACTIVE_COLOR;
  if (type() == FL_VERT_NICE_SLIDER) {
    if (ticks()) draw_ticks(Fl_Rect { X, Y+S/2, W, H-S}, S);
    draw_box(FL_THIN_DOWN_BOX, X+W/2-2, Y, 4, H, black);
  } else if (type() == FL_HOR_NICE_SLIDER) {
    if (ticks()) draw_ticks(Fl_Rect { X+S/2, Y, W-S, H}, S);
    draw_box(FL_THIN_DOWN_BOX, X, Y+H/2-2, W, 4, black);
  }
}

void Fl_Slider::draw(int X, int Y, int W, int H) {

  double val;
  if (minimum() == maximum())
    val = 0.5;
  else
    val = value_to_position(value());

  int ww = (horizontal() ? W : H);
  int xx, S;
  if (type()==FL_HOR_FILL_SLIDER || type() == FL_VERT_FILL_SLIDER) {
    S = int(val*ww+.5);
    if (minimum()>maximum()) {S = ww-S; xx = ww-S;}
    else xx = 0;
  } else {
    S = int(slider_size_*ww+.5);
    int T = (horizontal() ? H : W)/2+1;
    if (type()==FL_VERT_NICE_SLIDER || type()==FL_HOR_NICE_SLIDER) T += 4;
    if (S < T) S = T;
    xx = int(val*(ww-S)+.5);
  }
  int xsl, ysl, wsl, hsl;
  if (horizontal()) {
    xsl = X+xx;
    wsl = S;
    ysl = Y;
    hsl = H;
  } else {
    ysl = Y+xx;
    hsl = S;
    xsl = X;
    wsl = W;
  }

  draw_bg(X, Y, W, H, S);

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

    if (type() != FL_HOR_FILL_SLIDER && type() != FL_VERT_FILL_SLIDER &&
        Fl::is_scheme("gtk+")) {
      if (W>H && wsl>(hsl+8)) {
        // Draw horizontal grippers
        int yy, hh;
        hh = hsl-8;
        xx = xsl+(wsl-hsl-4)/2;
        yy = ysl+3;

        fl_color(fl_darker(selection_color()));
        fl_line(xx, yy+hh, xx+hh, yy);
        fl_line(xx+6, yy+hh, xx+hh+6, yy);
        fl_line(xx+12, yy+hh, xx+hh+12, yy);

        xx++;
        fl_color(fl_lighter(selection_color()));
        fl_line(xx, yy+hh, xx+hh, yy);
        fl_line(xx+6, yy+hh, xx+hh+6, yy);
        fl_line(xx+12, yy+hh, xx+hh+12, yy);
      } else if (H>W && hsl>(wsl+8)) {
        // Draw vertical grippers
        int yy;
        xx = xsl+4;
        ww = wsl-8;
        yy = ysl+(hsl-wsl-4)/2;

        fl_color(fl_darker(selection_color()));
        fl_line(xx, yy+ww, xx+ww, yy);
        fl_line(xx, yy+ww+6, xx+ww, yy+6);
        fl_line(xx, yy+ww+12, xx+ww, yy+12);

        yy++;
        fl_color(fl_lighter(selection_color()));
        fl_line(xx, yy+ww, xx+ww, yy);
        fl_line(xx, yy+ww+6, xx+ww, yy+6);
        fl_line(xx, yy+ww+12, xx+ww, yy+12);
      }
    }
  }

  draw_label(xsl, ysl, wsl, hsl);
  if (Fl::focus() == this) {
    if (type() == FL_HOR_FILL_SLIDER || type() == FL_VERT_FILL_SLIDER) draw_focus();
    else draw_focus(box1, xsl, ysl, wsl, hsl);
  }
}

void Fl_Slider::draw() {
  if (damage()&FL_DAMAGE_ALL) draw_box();
  draw(x()+Fl::box_dx(box()),
       y()+Fl::box_dy(box()),
       w()-Fl::box_dw(box()),
       h()-Fl::box_dh(box()));
}

// Format a tick value as a short label: "%g" for |v|>=1, "%.3g" with the
// leading "0" stripped for |v|<1 (so ".5" instead of "0.5"). Ported from
// fltk2's Slider::draw_ticks() helper of the same purpose.
static char *fl_slider_printtick(char *buffer, size_t bufsize, double v) {
  if (fabs(v) >= 1) {
    snprintf(buffer, bufsize, "%g", v);
    return buffer;
  } else {
    snprintf(buffer, bufsize, "%.3g", v);
    char *p = buffer;
    if (v < 0) p++;
    while (p[0]=='0' && p[1]) p++;
    if (v < 0) *--p = '-';
    return p;
  }
}

/** Draw tick marks.

 Ported from fltk2's Slider::draw_ticks(): tick marks are placed at "nice"
 round-number intervals (multiples of 1, 2, or 5 times a power of ten) that
 are never closer together on screen than about half the slider size, and
 every few ticks is drawn longer and labeled with its value.

 \param[in] r motion range of the slider
 \param[in] S size of the slider, horizontal or vertical, in pixels
 */
void Fl_Slider::draw_ticks(const Fl_Rect& r, int S)
{
  if (ticks() == TICKS_NONE) return;

  double A = minimum();
  double B = maximum();
  if (A > B) { double t = A; A = B; B = t; }
  if (A == B) return;

  int w = horizontal() ? r.w() : r.h();
  if (w <= 0) return;

  int min_spacing = S > 0 ? (S+1)/2 : 10;

  // Find a "nice" value-spacing (mul/div) between minor tick marks so they
  // land on round numbers and are never closer than min_spacing pixels.
  // smallmod: minor ticks per longer tick. nummod: minor ticks per labeled
  // tick. powincr: how many major ticks before the spacing grows tenfold
  // (only relevant for the log-scale branch).
  double mul = 1;
  double div = 1;
  int smallmod = 5;
  int nummod = 10;
  int powincr = 10000;

  bool log_scale = (scale() == LOG_SCALE) && (A > 0);
  if (!log_scale) {
    // linear scale (also used as a fallback when a log scale was
    // requested but the range isn't strictly positive, matching
    // value_to_position()'s own fallback to linear in that case)
    double derivative = (B-A)*min_spacing/w;
    if (derivative < step()) derivative = step();
    if (derivative <= 0) return;
    while (mul*5 <= derivative) mul *= 10;
    while (mul > derivative*2*div) div *= 10;
    if (derivative*div > mul*2) { mul *= 5; smallmod = 2; }
    else if (derivative*div > mul) { mul *= 2; nummod = 5; }
  } else {
    // log scale
    while (mul*5 <= A) mul *= 10;
    while (mul > A*2*div) div *= 10;
    powincr = 10;
    double d = exp(min_spacing*::log(B/A)/w*3);
    if (d >= 5) { mul *= 10; smallmod = nummod = 1; powincr = 1; }
    else if (d >= 2) { mul *= 5; smallmod = powincr = nummod = 2; }
  }

  // maps a value to a pixel offset from r.x() (horizontal) or r.y()
  // (vertical), using the same value_to_position() mapping the slider
  // knob itself is positioned with, so ticks line up with the knob.
  auto tick_offset = [this, w](double v) -> int {
    return int(value_to_position(v)*w + .5);
  };

  // the long, labeled tick marks span major1..major2; minor ticks span
  // only the middle half of that range so they read as visibly shorter
  int major1, major2;
  if (horizontal()) {
    major1 = (ticks() & TICKS_ABOVE) ? r.y()   : r.y() + r.h()/2;
    major2 = (ticks() & TICKS_BELOW) ? r.b()-1 : r.y() + r.h()/2;
  } else {
    major1 = (ticks() & TICKS_LEFT)  ? r.x()   : r.x() + r.w()/2;
    major2 = (ticks() & TICKS_RIGHT) ? r.r()-1 : r.x() + r.w()/2;
  }
  int mid = (major1+major2)/2;
  int minor1 = (major1+mid)/2;
  int minor2 = (major2+mid)/2;

  Fl_Color full_color = active_r() ? FL_FOREGROUND_COLOR : FL_INACTIVE_COLOR;
  Fl_Color line_color = fl_color_average(full_color, color(), .667f);
  fl_font(labelfont(), labelsize());
  bool label_below = horizontal() ? (ticks() & TICKS_BELOW) != 0
                                   : (ticks() & TICKS_RIGHT) != 0;

  auto draw_tick = [&](double v, bool major) {
    int t = tick_offset(v);
    if (horizontal()) {
      int x = r.x()+t;
      fl_yxline(x, major ? major1 : minor1, major ? major2 : minor2);
    } else {
      int y = r.y()+t;
      fl_xyline(major ? major1 : minor1, y, major ? major2 : minor2);
    }
  };
  auto draw_label = [&](double v) {
    char buffer[24];
    char *p = fl_slider_printtick(buffer, sizeof(buffer), v);
    int t = tick_offset(v);
    float x, y;
    if (horizontal()) {
      x = r.x()+t+1;
      if (x+fl_width(p) > r.r()) x = r.x()+t-1-fl_width(p); // keep on screen
      y = label_below ? major2+fl_height()-fl_descent() : major1-fl_descent()-1;
    } else {
      x = label_below ? major2+2 : major1-2-fl_width(p);
      y = r.y()+t+fl_height()/2-fl_descent();
    }
    fl_color(full_color);
    fl_draw(p, int(x), int(y));
  };

  for (int n = 0; ; n++) {
    // every powincr major ticks, the spacing grows tenfold (log scale)
    if (n > powincr) { mul *= 10; n = (n-1)/10+1; }
    double v = mul*n/div;
    if (v >= fabs(A) && v >= fabs(B)) break;
    bool major = (n%smallmod) == 0;
    fl_color(major ? full_color : line_color);
    if (v > A && v < B) {
      draw_tick(v, major);
      if (major && n%nummod == 0) draw_label(v);
    }
    if (v && -v > A && -v < B) {
      draw_tick(-v, major);
      if (major && n%nummod == 0) draw_label(-v);
    }
  }

  // always mark and label the two ends of the range, even if they don't
  // fall on a "nice" tick value
  fl_color(full_color);
  draw_tick(A, true);
  draw_label(A);
  draw_tick(B, true);
  draw_label(B);
}

/**
 Adds a sensible step value to the passed value.
 The step size is not useful in logarithmic scales.
 */
double Fl_Slider::increment_lin_log(double v, int n, int range) {
  if (scale() == Scale_Type::LOG_SCALE) {
    double pos = value_to_position(v);
    pos = pos + (n*4/(double)range);
    return position_to_value(pos);
  } else {
    return increment(v, n);
  }
}


int Fl_Slider::handle(int event, int X, int Y, int W, int H) {
  // Fl_Widget_Tracker wp(this);
  switch (event) {
  case FL_PUSH: {
    Fl_Widget_Tracker wp(this);
    if (!Fl::event_inside(X, Y, W, H)) return 0;
    handle_push();
    if (wp.deleted()) return 1; }
    // fall through ...
  case FL_DRAG: {

    double val;
    if (minimum() == maximum())
      val = 0.5;
    else
      val = value_to_position(value());

    int ww = (horizontal() ? W : H);
    int mx = (horizontal() ? Fl::event_x()-X : Fl::event_y()-Y);
    int S;
    static int offcenter;

    if (type() == FL_HOR_FILL_SLIDER || type() == FL_VERT_FILL_SLIDER) {

      S = 0;
      if (event == FL_PUSH) {
        int xx = int(val*ww+.5);
        offcenter = mx-xx;
        if (offcenter < -10 || offcenter > 10) offcenter = 0;
        else return 1;
      }

    } else {

      S = int(slider_size_*ww+.5); if (S >= ww) return 0;
      int T = (horizontal() ? H : W)/2+1;
      if (type()==FL_VERT_NICE_SLIDER || type()==FL_HOR_NICE_SLIDER) T += 4;
      if (S < T) S = T;
      if (event == FL_PUSH) {
        int xx = int(val*(ww-S)+.5);
        offcenter = mx-xx;
        if (offcenter < 0) offcenter = 0;
        else if (offcenter > S) offcenter = S;
        else return 1;
      }
    }

    int xx = mx-offcenter;
    double v = 0;
    char tryAgain = 1;
    while (tryAgain)
    {
      tryAgain = 0;
      if (xx < 0) {
        xx = 0;
        offcenter = mx; if (offcenter < 0) offcenter = 0;
      } else if (xx > (ww-S)) {
        xx = ww-S;
        offcenter = mx-xx; if (offcenter > S) offcenter = S;
      }
      // Convert position back to value using the appropriate scale
      double pos = (ww-S) > 0 ? double(xx) / double(ww-S) : 0.0;
      v = round(position_to_value(pos));
      // make sure a click outside the sliderbar moves it:
      if (event == FL_PUSH && v == value()) {
        offcenter = S/2;
        event = FL_DRAG;
        tryAgain = 1;
      }
    }
    handle_drag(clamp(v));
    } return 1;
  case FL_RELEASE:
    handle_release();
    return 1;
  case FL_KEYBOARD:
    { Fl_Widget_Tracker wp(this);
      switch (Fl::event_key()) {
        case FL_Up:
          if (horizontal()) return 0;
          handle_push();
          if (wp.deleted()) return 1;
          handle_drag(clamp(increment_lin_log(value(), -1, H)));
          if (wp.deleted()) return 1;
          handle_release();
          return 1;
        case FL_Down:
          if (horizontal()) return 0;
          handle_push();
          if (wp.deleted()) return 1;
          handle_drag(clamp(increment_lin_log(value(), 1, H)));
          if (wp.deleted()) return 1;
          handle_release();
          return 1;
        case FL_Left:
          if (!horizontal()) return 0;
          handle_push();
          if (wp.deleted()) return 1;
          handle_drag(clamp(increment_lin_log(value(), -1, W)));
          if (wp.deleted()) return 1;
          handle_release();
          return 1;
        case FL_Right:
          if (!horizontal()) return 0;
          handle_push();
          if (wp.deleted()) return 1;
          handle_drag(clamp(increment_lin_log(value(), 1, W)));
          if (wp.deleted()) return 1;
          handle_release();
          return 1;
        default:
          return 0;
      }
    }
    // break not required because of switch...
  case FL_FOCUS :
  case FL_UNFOCUS :
    if (Fl::visible_focus()) {
      redraw();
      return 1;
    } else return 0;
  case FL_ENTER :
  case FL_LEAVE :
    return 1;
  default:
    return 0;
  }
}


int Fl_Slider::handle(int event) {
  if (event == FL_PUSH && Fl::visible_focus()) {
    Fl::focus(this);
    redraw();
  }

  return handle(event,
                x()+Fl::box_dx(box()),
                y()+Fl::box_dy(box()),
                w()-Fl::box_dw(box()),
                h()-Fl::box_dh(box()));
}


Fl_Fill_Slider::Fl_Fill_Slider(int X,int Y,int W,int H,const char *L)
: Fl_Slider(X,Y,W,H,L)
{
  type(FL_VERT_FILL_SLIDER);
}


Fl_Hor_Slider::Fl_Hor_Slider(int X,int Y,int W,int H,const char *l)
: Fl_Slider(X,Y,W,H,l) {
  type(FL_HOR_SLIDER);
}


Fl_Hor_Fill_Slider::Fl_Hor_Fill_Slider(int X,int Y,int W,int H,const char *L)
: Fl_Slider(X,Y,W,H,L)
{
  type(FL_HOR_FILL_SLIDER);
}


Fl_Hor_Nice_Slider::Fl_Hor_Nice_Slider(int X,int Y,int W,int H,const char *L)
: Fl_Slider(X,Y,W,H,L)
{
  type(FL_HOR_NICE_SLIDER);
  box(FL_FLAT_BOX);
}


Fl_Nice_Slider::Fl_Nice_Slider(int X,int Y,int W,int H,const char *L)
: Fl_Slider(X,Y,W,H,L) {
  type(FL_VERT_NICE_SLIDER);
  box(FL_FLAT_BOX);
}

/**
  Converts a value to a normalized position (0.0 to 1.0) based on the current scale type.
  For linear scale, this is a simple linear interpolation.
  For logarithmic scale, this uses logarithmic interpolation.
  \param[in] val The value to convert.
  \return A normalized position between 0.0 and 1.0.
*/
double Fl_Slider::value_to_position(double val) const {
  if (minimum() == maximum()) return 0.5;

  if (scale_type_ == LOG_SCALE) {
    // For logarithmic scale, both min and max must be positive
    if (minimum() <= 0 || maximum() <= 0 || val <= 0) {
      // Fall back to linear if values are not suitable for log scale
      double pos = (val - minimum()) / (maximum() - minimum());
      if (pos > 1.0) pos = 1.0;
      else if (pos < 0.0) pos = 0.0;
      return pos;
    }
    double log_min = log(minimum());
    double log_max = log(maximum());
    double log_val = log(val);
    double pos = (log_val - log_min) / (log_max - log_min);
    if (pos > 1.0) pos = 1.0;
    else if (pos < 0.0) pos = 0.0;
    return pos;
  } else {
    // Linear scale
    double pos = (val - minimum()) / (maximum() - minimum());
    if (pos > 1.0) pos = 1.0;
    else if (pos < 0.0) pos = 0.0;
    return pos;
  }
}

/**
  Converts a normalized position (0.0 to 1.0) to a value based on the current scale type.
  For linear scale, this is a simple linear interpolation.
  For logarithmic scale, this uses exponential interpolation.
  \param[in] pos The normalized position to convert.
  \return The corresponding value.
*/
double Fl_Slider::position_to_value(double pos) const {
  if (minimum() == maximum()) return minimum();

  if (scale_type_ == LOG_SCALE) {
    // For logarithmic scale, both min and max must be positive
    if (minimum() <= 0 || maximum() <= 0) {
      // Fall back to linear if values are not suitable for log scale
      return pos * (maximum() - minimum()) + minimum();
    }
    double log_min = log(minimum());
    double log_max = log(maximum());
    return exp(pos * (log_max - log_min) + log_min);
  } else {
    // Linear scale
    return pos * (maximum() - minimum()) + minimum();
  }
}
