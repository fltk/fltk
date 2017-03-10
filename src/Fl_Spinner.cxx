//
// "$Id$"
//
// Spinner widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2017 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

/* \file
   Fl_Spinner widget . */

#include <stdio.h>
#include <stdlib.h>

#include <FL/Fl_Spinner.H>

/*
  This widget is a combination of the input widget and repeat buttons.

  The user can either type into the input area or use the buttons to
  change the value.
*/

void Fl_Spinner::sb_cb(Fl_Widget *w, Fl_Spinner *sb) {
  double v;				// New value

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
    if (v > sb->maximum_) {
      if (sb->wrap_)
	v = sb->minimum_;
      else
	v = sb->maximum_;
    }
    sb->value_ = v;
    sb->update();
  } else if (w == &(sb->down_button_)) {
    // Down button pressed...
    v = sb->value_ - sb->step_;
    if (v < sb->minimum_) {
      if (sb->wrap_)
	v = sb->maximum_;
      else
	v = sb->minimum_;
    }
    sb->value_ = v;
    sb->update();
  }

  sb->set_changed();
  sb->do_callback();
}

void Fl_Spinner::update() {
  char s[255];    // Value string

  if (format_[0] == '%' && format_[1] == '.' && format_[2] == '*') { // precision argument
    // this code block is a simplified version of
    // Fl_Valuator::format() and works well (but looks ugly)
    int c = 0;
    char temp[64], *sp = temp;
    sprintf(temp, "%.12f", step_);
    while (*sp) sp++;
    sp--;
    while (sp > temp && *sp == '0') sp--;
    while (sp > temp && (*sp >= '0' && *sp <= '9')) { sp--; c++; }
    sprintf(s, format_, c, value_);
  } else {
    sprintf(s, format_, value_);
  }
  input_.value(s);
}

#define FL_UP_ARROW_TX "@-42<"
#define FL_DOWN_ARROW_TX "@-42>"

/**
  Creates a new Fl_Spinner widget using the given position, size,
  and label string.

  The inherited destructor destroys the widget and any value associated with it.
*/

Fl_Spinner::Fl_Spinner(int X, int Y, int W, int H, const char *L)
: Fl_Group(X, Y, W, H, L),
  input_(X, Y, W - H / 2 - 2, H),
  up_button_(X + W - H / 2 - 2, Y, H / 2 + 2, H / 2, FL_UP_ARROW_TX),
  down_button_(X + W - H / 2 - 2, Y + H - H / 2,
               H / 2 + 2, H / 2, FL_DOWN_ARROW_TX)
{
  end();

  value_   = 1.0;
  minimum_ = 1.0;
  maximum_ = 100.0;
  step_    = 1.0;
  wrap_    = 1;
  format_  = "%g";

  align(FL_ALIGN_LEFT);

  input_.value("1");
  input_.type(FL_INT_INPUT);
  input_.when(FL_WHEN_ENTER_KEY | FL_WHEN_RELEASE);
  input_.callback((Fl_Callback *)sb_cb, this);

  up_button_.callback((Fl_Callback *)sb_cb, this);

  down_button_.callback((Fl_Callback *)sb_cb, this);
}

int Fl_Spinner::handle(int event) {

  switch (event) {

    case FL_KEYDOWN:
    case FL_SHORTCUT:
      if (Fl::event_key() == FL_Up) {
	up_button_.do_callback();
	return 1;
      } else if (Fl::event_key() == FL_Down) {
	down_button_.do_callback();
	return 1;
      }
      return 0;

    case FL_FOCUS:
      if (input_.take_focus()) return 1;
      return 0;
  }

  return Fl_Group::handle(event);
}

void Fl_Spinner::resize(int X, int Y, int W, int H) {
  Fl_Group::resize(X,Y,W,H);

  input_.resize(X, Y, W - H / 2 - 2, H);
  up_button_.resize(X + W - H / 2 - 2, Y, H / 2 + 2, H / 2);
  down_button_.resize(X + W - H / 2 - 2, Y + H - H / 2,
                      H / 2 + 2, H / 2);
}

/**
  Sets or returns the amount to change the value when the user clicks a button.
  Before setting step to a non-integer value, the spinner
  type() should be changed to floating point.

  \see double Fl_Spinner::step() const
*/

void Fl_Spinner::step(double s) {
  step_ = s;
  if (step_ != (int)step_) input_.type(FL_FLOAT_INPUT);
  else input_.type(FL_INT_INPUT);
  update();
}

/** Sets the numeric representation in the input field.

  Valid values are FL_INT_INPUT and FL_FLOAT_INPUT.
  Also changes the format() template.
  Setting a new spinner type via a superclass pointer will not work.
  \note type() is not a virtual function.
*/

void Fl_Spinner::type(uchar v) {
  if (v == FL_FLOAT_INPUT) {
    format("%.*f");
  } else {
    format("%.0f");
  }
  input_.type(v);
}


//
// End of "$Id$".
//
