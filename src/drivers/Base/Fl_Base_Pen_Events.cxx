//
// Implementation of default Pen/Tablet event driver.
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

#include "src/drivers/Base/Fl_Base_Pen_Events.H"


class Fl_Widget;


namespace Fl {

namespace Pen {

EventData e;
SubscriberList subscriber_list_;
std::shared_ptr<Subscriber> pushed_;
std::shared_ptr<Subscriber> below_pen_;

} // namespace Pen

} // namespace Fl


using namespace Fl::Pen;


// ---- SubscriberList implementation ------------------------------------------

/* Remove subscribers that have a nullptr as a widget */
void Fl::Pen::SubscriberList::cleanup() {
  for (auto it = begin(); it != end(); ) {
    if (!it->second->widget()) {
      it = erase(it);
    } else {
      ++it;
    }
  }
}

/* Add a new subscriber, or return an existing one. */
std::shared_ptr<Subscriber> Fl::Pen::SubscriberList::add(Fl_Widget *w) {
  cleanup();
  auto it = find(w);
  if (it == end()) {
    auto sub = std::make_shared<Subscriber>(w);
    insert(std::make_pair(w, sub));
    return sub;
  } else {
    return it->second;
  }
}

/* Remove a subscriber from the list. */
void Fl::Pen::SubscriberList::remove(Fl_Widget *w) {
  auto it = find(w);
  if (it != end()) {
    it->second->clear();
    erase(it);
  }
}

// ---- Driver implementation --------------------------------------------------
// Override the methods below to handle subscriptions and queries by user apps.

void Fl::Pen::Driver::subscribe(Fl_Widget* widget) {
  if (widget == nullptr) return;
  subscriber_list_.add(widget);
}

void Fl::Pen::Driver::unsubscribe(Fl_Widget* widget) {
  if (widget == nullptr) return;
  subscriber_list_.remove(widget);
}

void Fl::Pen::Driver::release() {
  pushed_ = nullptr;
  below_pen_ = nullptr;
}

Trait Fl::Pen::Driver::traits() {
  return Trait::NONE;
}

Trait Fl::Pen::Driver::pen_traits(int pen_id) {
  (void)pen_id;
  return Trait::NONE;
}

// ---- Fl::Pen API ------------------------------------------------------------

void Fl::Pen::subscribe(Fl_Widget* widget) {
  driver.subscribe(widget);
}

void Fl::Pen::unsubscribe(Fl_Widget* widget) {
  driver.unsubscribe(widget);
}

void Fl::Pen::release() {
  driver.release();
}

Trait Fl::Pen::driver_traits() {
  return driver.traits();
}

Trait Fl::Pen::pen_traits(int pen_id) {
  return driver.pen_traits(pen_id);
}

double Fl::Pen::event_x() { return e.x; }

double Fl::Pen::event_y() { return e.y; }

double Fl::Pen::event_x_root() { return e.rx; }

double Fl::Pen::event_y_root() { return e.ry; }

int Fl::Pen::event_pen_id() { return e.pen_id; }

double Fl::Pen::event_pressure() { return e.pressure; }

double Fl::Pen::event_barrel_pressure() { return e.barrel_pressure; }

double Fl::Pen::event_tilt_x() { return e.tilt_x; }

double Fl::Pen::event_tilt_y() { return e.tilt_y; }

double Fl::Pen::event_twist() { return e.twist; }

double Fl::Pen::event_proximity() { return e.proximity; }

State Fl::Pen::event_state() { return e.state; }

State Fl::Pen::event_trigger() { return e.trigger; }
