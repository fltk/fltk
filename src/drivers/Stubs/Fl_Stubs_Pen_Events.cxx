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


Trait Fl::Pen::driver_traits() { return Trait::NONE; }

Trait Fl::Pen::pen_traits(int pen_id) { return Trait::NONE; }

void Fl::Pen::subscribe(Fl_Widget* widget) { }

void Fl::Pen::unsubscribe(Fl_Widget* widget) { }

void Fl::Pen::release() { }

double Fl::Pen::event_x() { return 0.0; }

double Fl::Pen::event_y() { return 0.0; }

double Fl::Pen::event_x_root() { return 0.0; }

double Fl::Pen::event_y_root() { return 0.0; }

int Fl::Pen::event_pen_id() { return 0; }

double Fl::Pen::event_pressure() { return 1.0; }

double Fl::Pen::event_barrel_pressure() { return 0.0; }

double Fl::Pen::event_tilt_x() { return 0.0; }

double Fl::Pen::event_tilt_y() { return 0.0; }

double Fl::Pen::event_twist() { return 0.0; }

double Fl::Pen::event_proximity() { return 0.0; }

State Fl::Pen::event_state() { return Fl::Pen::State::NONE; }

State Fl::Pen::event_trigger() { return Fl::Pen::State::NONE; }
