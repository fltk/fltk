//
// Definition of default Pen/Tablet event driver.
//
// Copyright 2025 by Bill Spitzak and others.
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


#include <config.h>
#include <FL/platform.H>
#include <FL/core/pen_events.H>
#include <FL/Fl.H>

class Fl_Widget;

namespace Fl {

namespace Pen {

// double e_pressure_;

} // namespace Pen

} // namespace Fl


using namespace Fl::Pen;

Features Fl::Pen::features() { return NONE; }

void Fl::Pen::subscribe(Fl_Widget*, bool) { }

void Fl::Pen::unsubscribe(Fl_Widget*) { }

void Fl::Pen::grab(Fl_Widget*) { }

void Fl::Pen::release() { }

double Fl::Pen::event_x() { return static_cast<double>(Fl::event_x()); }

double Fl::Pen::event_y() { return static_cast<double>(Fl::event_y()); }

double Fl::Pen::event_x_root() { return static_cast<double>(Fl::event_x_root()); }

double Fl::Pen::event_y_root() { return static_cast<double>(Fl::event_y_root()); }

int Fl::Pen::event_id() { return -1; }

double Fl::Pen::event_pressure() { return 0.0; }

double Fl::Pen::event_tangential_pressure() { return 0.0; }

double Fl::Pen::event_tilt_x() { return 0.0; }

double Fl::Pen::event_tilt_y() { return 0.0; }

double Fl::Pen::event_twist() { return 0.0; }

double Fl::Pen::event_proximity() { return 0.0; }

State Fl::Pen::event_state() { return static_cast<State>(0); }

State Fl::Pen::event_trigger() { return static_cast<State>(0); }

