//
// "$Id: Fl_Valuator.cxx,v 1.5.2.4 2001/01/22 15:13:40 easysw Exp $"
//
// Valuator widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2001 by Bill Spitzak and others.
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
// Please report all bugs and problems to "fltk-bugs@fltk.org".
//

// Base class for sliders and all other one-value "knobs"

#include <FL/Fl.H>
#include <FL/Fl_Valuator.H>
#include <FL/math.h>
#include <stdio.h>

Fl_Valuator::Fl_Valuator(int X, int Y, int W, int H, const char* L)
  : Fl_Widget(X,Y,W,H,L) {
  align(FL_ALIGN_BOTTOM);
  when(FL_WHEN_CHANGED);
  value_ = 0;
  min = 0;
  max = 1;
  A = 0.0;
  B = 1;
}

const double epsilon = 4.66e-10;

void Fl_Valuator::step(double s) {
  if (s < 0) s = -s;
  A = rint(s);
  B = 1;
  while (fabs(s-A/B) > epsilon && B<=(0x7fffffff/10)) {B *= 10; A = rint(s*B);}
}

void Fl_Valuator::precision(int p) {
  A = 1.0;
  for (B = 1; p--;) B *= 10;
}

void Fl_Valuator::value_damage() {damage(FL_DAMAGE_EXPOSE);} // by default do partial-redraw

int Fl_Valuator::value(double v) {
  clear_changed();
  if (v == value_) return 0;
  value_ = v;
  value_damage();
  return 1;
}

double Fl_Valuator::softclamp(double v) {
  int which = (min<=max);
  double p = previous_value_;
  if ((v<min)==which && p!=min && (p<min)!=which) return min;
  else if ((v>max)==which && p!=max && (p>max)!=which) return max;
  else return v;
}

// inline void Fl_Valuator::handle_push() {previous_value_ = value_;}

void Fl_Valuator::handle_drag(double v) {
  if (v != value_) {
    value_ = v;
    value_damage();
    if (when() & FL_WHEN_CHANGED) do_callback();
    else set_changed();
  }
}

void Fl_Valuator::handle_release() {
  if (when()&FL_WHEN_RELEASE) {
    // insure changed() is off even if no callback is done.  It may have
    // been turned on by the drag, and then the slider returned to it's
    // initial position:
    clear_changed();
    // now do the callback only if slider in new position or always is on:
    if (value_ != previous_value_ || when() & FL_WHEN_NOT_CHANGED)
      do_callback();
  }
}

double Fl_Valuator::round(double v) {
  if (A) return rint(v*B/A)*A/B;
  else return v;
}

double Fl_Valuator::clamp(double v) {
  if ((v<min)==(min<=max)) return min;
  else if ((v>max)==(min<=max)) return max;
  else return v;
}

double Fl_Valuator::increment(double v, int n) {
  if (!A) return v+n*(max-min)/100;
  if (min > max) n = -n;
  return (rint(v*B/A)+n)*A/B;
}

int Fl_Valuator::format(char* buffer) {
  double v = value();
  if (!A || B==1) return sprintf(buffer, "%g", v);
  int i, x;
  for (x = 10, i = 2; x < B; x *= 10) i++;
  if (x == B) i--;
  return sprintf(buffer, "%.*f", i, v);
}

//
// End of "$Id: Fl_Valuator.cxx,v 1.5.2.4 2001/01/22 15:13:40 easysw Exp $".
//
