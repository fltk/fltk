//
// "$Id$"
//
// Spinner widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2005 by Bill Spitzak and others.
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
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

#ifndef Fl_Spinner_H
#  define Fl_Spinner_H

//
// Include necessary headers...
//

#  include <FL/Fl_Group.H>
#  include <FL/Fl_Input.H>
#  include <FL/Fl_Repeat_Button.H>
#  include <stdio.h>
#  include <stdlib.h>


//
// Fl_Spinner widget class...
//

class Fl_Spinner : public Fl_Group
{
  double	value_;			// Current value
  double	minimum_;		// Minimum value
  double	maximum_;		// Maximum value
  double	step_;			// Amount to add/subtract for up/down
  const char	*format_;		// Format string

  Fl_Input	input_;			// Input field for the value
  Fl_Repeat_Button
		up_button_,		// Up button
		down_button_;		// Down button

  static void	sb_cb(Fl_Widget *w, Fl_Spinner *sb) {
		  double v;		// New value

		  if (w == &(sb->input_)) {
		    // Something changed in the input field...
		    v = atof(sb->input_.value());

		    if (v < sb->minimum_) {
		      sb->value_ = sb->minimum_;
		      sb->update();
		    } else if (v > sb->maximum_) {
		      sb->value_ = sb->maximum_;
		      sb->update();
		    } else sb->value_ = v;
		  } else if (w == &(sb->up_button_)) {
		    // Up button pressed...
		    v = sb->value_ + sb->step_;

		    if (v > sb->maximum_) sb->value_ = sb->minimum_;
		    else sb->value_ = v;

		    sb->update();
		  } else if (w == &(sb->down_button_)) {
		    // Down button pressed...
		    v = sb->value_ - sb->step_;

		    if (v < sb->minimum_) sb->value_ = sb->maximum_;
		    else sb->value_ = v;

		    sb->update();
		  }

		  sb->do_callback();
		}
  void		update() {
		  char s[255];		// Value string

		  sprintf(s, format_, value_);
		  input_.value(s);
		}

  public:

		Fl_Spinner(int X, int Y, int W, int H, const char *L = 0)
		  : Fl_Group(X, Y, W, H, L),
		    input_(X, Y, W - H / 2 - 2, H),
		    up_button_(X + W - H / 2 - 2, Y, H / 2 + 2, H / 2, "@-22<"),
		    down_button_(X + W - H / 2 - 2, Y + H - H / 2,
		                 H / 2 + 2, H / 2, "@-22>") {
		  end();

		  value_   = 1.0;
		  minimum_ = 1.0;
		  maximum_ = 100.0;
		  step_    = 1.0;
		  format_  = "%.0f";

		  align(FL_ALIGN_LEFT);

		  input_.value("1");
		  input_.type(FL_INT_INPUT);
		  input_.when(FL_WHEN_CHANGED);
		  input_.callback((Fl_Callback *)sb_cb, this);

		  up_button_.callback((Fl_Callback *)sb_cb, this);

		  down_button_.callback((Fl_Callback *)sb_cb, this);
		}

  const char	*format() { return (format_); }
  void		format(const char *f) { format_ = f; update(); }
  double	maxinum() const { return (maximum_); }
  void		maximum(double m) { maximum_ = m; }
  double	mininum() const { return (minimum_); }
  void		minimum(double m) { minimum_ = m; }
  void		range(double a, double b) { minimum_ = a; maximum_ = b; }
  void		resize(int X, int Y, int W, int H) {
		  Fl_Group::resize(X,Y,W,H);

		  input_.resize(X, Y, W - H / 2 - 2, H);
		  up_button_.resize(X + W - H / 2 - 2, Y, H / 2 + 2, H / 2);
		  down_button_.resize(X + W - H / 2 - 2, Y + H - H / 2,
		                      H / 2 + 2, H / 2);
		}
  double	step() const { return (step_); }
  void		step(double s) { step_ = s; }
  Fl_Color	textcolor() const {
		  return (input_.textcolor());
		}
  void		textcolor(Fl_Color c) {
		  input_.textcolor(c);
		}
  uchar		textfont() const {
		  return (input_.textfont());
		}
  void		textfont(uchar f) {
		  input_.textfont(f);
		}
  uchar		textsize() const {
		  return (input_.textsize());
		}
  void		textsize(uchar s) {
		  input_.textsize(s);
		}
  double	value() const { return (value_); }
  void		value(double v) { value_ = v; update(); }
};

#endif // !Fl_Spinner_H

//
// End of "$Id$".
//
