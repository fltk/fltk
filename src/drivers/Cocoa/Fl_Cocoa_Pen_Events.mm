//
// Definition of macOS Cocoa Pen/Tablet event driver.
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
#include <FL/Fl_Window.H>
#include <FL/Fl_Tooltip.H>
#include <FL/Fl_Widget_Tracker.H>
#include "../../Fl_Screen_Driver.H"

#import <Cocoa/Cocoa.h>

#include <map>
#include <memory>

extern Fl_Window *fl_xmousewin;

/*
 Widgets and windows must subscribe to pen events. This is to reduce the amount
 of events sent into the widget hierarchy.

 Usually there is a pretty small number of subscribers, so looping through the
 subscriber list should not be an issue.

 All subscribers track their widget. If a widget is deleted while subscribed,
 including during event handling, the driver will remove the subscription.
 There is no need to explicitly unsubscribe.
 */
class Subscriber : public Fl_Widget_Tracker {
public:
  Subscriber(Fl_Widget *w) : Fl_Widget_Tracker(w) { }
};


/*
 Manage a list of subscribers.
 */
class SubscriberList : public std::map<Fl_Widget*, std::shared_ptr<Subscriber>> {
public:
  SubscriberList() = default;
  /* Remove subscribers that have a nullptr as a widget */
  void cleanup() {
    for (auto it = begin(); it != end(); ) {
      if (!it->second->widget()) {
        it = erase(it);
      } else {
        ++it;
      }
    }
  }
  /* Add a new subscriber, or return an existing one. */
  std::shared_ptr<Subscriber> add(Fl_Widget *w) {
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
  /* Remove a subscriber form the list. */
  void remove(Fl_Widget *w) {
    auto it = find(w);
    if (it != end()) {
      it->second->clear();
      erase(it);
    }
  }
};

static SubscriberList subscriber_list_;
static std::shared_ptr<Subscriber> pushed_;
static std::shared_ptr<Subscriber> below_pen_;
static NSPointingDeviceType device_type_ { NSPointingDeviceTypePen };

// The trait list keeps track of traits for every pen ID that appears while
// handling events.
// AppKit does not tell us what traits are available per pen or tablet, so
// we use the first 5 motion events to discover event values that are not
// the default value, and enter that knowledge into the traits database.
typedef std::map<int, Fl::Pen::Trait> TraitList;
static TraitList trait_list_;
static int trait_countdown_ { 5 };
static int current_pen_id_ { -1 };
static Fl::Pen::Trait current_pen_trait_ { Fl::Pen::Trait::DRIVER_AVAILABLE };
static Fl::Pen::Trait driver_traits_ {
  Fl::Pen::Trait::DRIVER_AVAILABLE | Fl::Pen::Trait::PEN_ID |
  Fl::Pen::Trait::ERASER | Fl::Pen::Trait::PRESSURE |
  Fl::Pen::Trait::BARREL_PRESSURE | Fl::Pen::Trait::TILT_X |
  Fl::Pen::Trait::TILT_Y | Fl::Pen::Trait::TWIST
  // Notably missing: PROXIMITY
};

struct EventData {
  double x { 0.0 };
  double y { 0.0 };
  double rx { 0.0 };
  double ry { 0.0 };
  double tilt_x { 0.0 };
  double tilt_y { 0.0 };
  double pressure { 1.0 };
  double barrel_pressure { 0.0 };
  double twist { 0.0 };
  int pen_id { 0 };
  Fl::Pen::State state { (Fl::Pen::State)0 };
  Fl::Pen::State trigger { (Fl::Pen::State)0 };
};

// Temporary storage of event data for the driver;
static struct EventData ev;


namespace Fl {

namespace Pen {

// The event data that is made available to the user during event handling
struct EventData e;

} // namespace Pen

} // namespace Fl


using namespace Fl::Pen;


// Return a bit for everything that AppKit could return.
Trait Fl::Pen::driver_traits() {
  return driver_traits_;
}

Trait Fl::Pen::pen_traits(int pen_id) {
  auto it = trait_list_.find(pen_id);
  if (pen_id == 0)
    return current_pen_trait_;
  if (it == trait_list_.end()) {
    return Trait::DRIVER_AVAILABLE;
  } else {
    return it->second;
  }
}

void Fl::Pen::subscribe(Fl_Widget* widget) {
  subscriber_list_.add(widget);
}

void Fl::Pen::unsubscribe(Fl_Widget* widget) {
  subscriber_list_.remove(widget);
}

// TODO: implement this - do we really need this interface?
void Fl::Pen::grab(Fl_Widget* widget) {
}

void Fl::Pen::release() { grab(nullptr); }

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

// Not supported in AppKit NSEvent
double Fl::Pen::event_proximity() { return 0.0; }

State Fl::Pen::event_state() { return e.state; }

State Fl::Pen::event_trigger() { return e.trigger; }

/**
 Copy the event state.
 */
static void copy_state() {
  Fl::Pen::State tr = (Fl::Pen::State)((uint32_t)Fl::Pen::e.state ^ (uint32_t)ev.state);
  Fl::Pen::e = ev;
  Fl::Pen::e.trigger = tr;
  Fl::e_x = (int)ev.x;
  Fl::e_y = (int)ev.y;
  Fl::e_x_root = (int)ev.rx;
  Fl::e_y_root = (int)ev.ry;
}

/**
 Offset coordinates for subwindows and subsubwindows.
 */
static void offset_subwindow_event(Fl_Widget *w, double &x, double &y) {
  Fl_Widget *p = w, *q;
  while (p) {
    q = p->parent();
    if (p->as_window() && q) {
      x -= p->x();
      y -= p->y();
    }
    p = q;
  };
}

/*
 Check if coordinates are within the widget box.
 Coordinates are in top_window space. We iterate up the hierarchy to ensure
 that we handle subwindows correctly.
 */
static bool event_inside(Fl_Widget *w, double x, double y) {
  offset_subwindow_event(w, x, y);
  if (w->as_window()) {
    return ((x >= 0) && (y >= 0) && (x < w->w()) && (y < w->h()));
  } else {
    return ((x >= w->x()) && (y >= w->y()) && (x < w->x() + w->w()) && (y < w->y() + w->h()));
  }
}

/*
 Find the widget under the pen event.
 Search the subscriber list for widgets that are inside the same top window,
 are visible, and are within the give coordinates. Subwindow aware.
 */
static Fl_Widget *find_below_pen(Fl_Window *win, double x, double y) {
  for (auto &sub: subscriber_list_) {
    Fl_Widget *candidate = sub.second->widget();
    if (candidate && (candidate->top_window() == win)) {
      if (candidate->visible() && event_inside(candidate, x, y)) {
        return candidate;
      }
    }
  }
  return nullptr;
}

/*
 Send the current event and event data to a widget.
 Note: we will get the wrong coordinates if the widget is not a child of
 the current event window (LEAVE events between windows).
 */
static int pen_send(Fl_Widget *w, int event, State trigger, bool &copied) {
  // Copy most event data only once
  if (!copied) {
    copy_state();
    copied = true;
  }
  // Copy the top_window coordinates again as they may change when w changes
  e.x = ev.x;
  e.y = ev.y;
  offset_subwindow_event(w, e.x, e.y);
  Fl::e_x = e.x;
  Fl::e_y = e.y;
  // Send the event.
  e.trigger = trigger;
  return w->handle(event);
}

/*
 Send an event to all subscribers.
 */
static int pen_send_all(int event, State trigger) {
  bool copied = false;
  // use local value because handler may still change ev values
  for (auto &it: subscriber_list_) {
    auto w = it.second->widget();
    if (w)
      pen_send(w, event, trigger, copied);
  }
}

/*
 Convert the NSEvent button number to Fl::Pen::State,
 */
static State button_to_trigger(NSInteger button, bool down)
{
  switch (button) {
    case 0:
      if ( (ev.state & (State::ERASER_DOWN | State::ERASER_HOVERS)) != State::NONE ) {
        return down ? State::ERASER_DOWN : State::ERASER_HOVERS;
      } else {
        return down ? State::TIP_DOWN : State::TIP_HOVERS;
      }
    case 1: return State::BUTTON0;
    case 2: return State::BUTTON1;
    case 3: return State::BUTTON2;
    case 4: return State::BUTTON3;
    default: return State::NONE;
  }
}

/*
 Handle events coming from Cocoa.
 TODO: clickCount: store in Fl::event_clicks()
 capabilityMask is useless, because it is vendor defined
 */
bool fl_cocoa_tablet_handler(NSEvent *event, Fl_Window *eventWindow)
{
  // Quick access to the main type.
  auto type = [event type];

  // There seems nothing useful here. Ignore for now.
  if ((type == NSEventTypeMouseEntered) || (type == NSEventTypeMouseExited)) {
    return false;
  }

  // Sort out tablet-only events and mouse plus tablet events.
  bool is_mouse = ((type != NSEventTypeTabletPoint) && (type != NSEventTypeTabletProximity));

  // Set the subtype if one is available. Only NSEventSubtypeTabletPoint and
  // NSEventSubtypeTabletProximity matter in this context
  NSEventSubtype subtype = is_mouse ? [event subtype] : NSEventSubtypeMouseEvent;

  // Is this a change in proximity event?
  bool is_proximity = ((type == NSEventTypeTabletProximity) || (subtype == NSEventSubtypeTabletProximity));

  // Is this a pen pointer event?
  bool is_point = ((type == NSEventTypeTabletPoint) || (subtype == NSEventSubtypeTabletPoint));

  // Check if any of the pen down, move, drag, or up events was triggered.
  bool is_down = ((type == NSEventTypeLeftMouseDown) || (type == NSEventTypeRightMouseDown) || (type == NSEventTypeOtherMouseDown));
  bool is_up = ((type == NSEventTypeLeftMouseUp) || (type == NSEventTypeRightMouseUp) || (type == NSEventTypeOtherMouseUp));
  bool is_drag = ((type == NSEventTypeLeftMouseDragged) || (type == NSEventTypeRightMouseDragged) || (type == NSEventTypeOtherMouseDragged));
  bool is_motion = is_drag || (type == NSEventTypeMouseMoved);

  // Find out if we can get the pen position
  bool has_position = (eventWindow != nullptr) && (is_up || is_down || is_motion || is_proximity || is_point);

  // Event has extended pen data set:
  if (has_position) {
    // Get the position data.
    auto pt = [event locationInWindow];
    double s = Fl::screen_driver()->scale(0);
    ev.x = pt.x/s;
    ev.y = eventWindow->h() - pt.y/s;
    // TODO: verify actual values: root coordinates may be used for popup windows
    ev.rx = ev.x*s + eventWindow->x();
    ev.ry = ev.y*s + eventWindow->y();
    if (!is_proximity) {
      // Get the pressure data.
      ev.pressure = [event pressure];
      ev.barrel_pressure = [event tangentialPressure];
      // Get the tilt
      auto tilt = [event tilt];
      ev.tilt_x = -tilt.x;
      ev.tilt_y =  tilt.y;
      // Other stuff
      ev.twist = [event rotation]; // TODO: untested
      // ev.proximity = [event proximity]; // not supported in AppKit
    }
    if (device_type_ == NSPointingDeviceTypeEraser) {
      if ([event buttonMask] & 1)
        ev.state = State::ERASER_DOWN;
      else
        ev.state = State::ERASER_HOVERS;
    } else {
      if ([event buttonMask] & 1)
        ev.state = State::TIP_DOWN;
      else
        ev.state = State::TIP_HOVERS;
    }
    if ([event buttonMask] & 0x0002) ev.state |= State::BUTTON0;
    if ([event buttonMask] & 0x0004) ev.state |= State::BUTTON1;
    if ([event buttonMask] & 0x0008) ev.state |= State::BUTTON2;
    if ([event buttonMask] & 0x0010) ev.state |= State::BUTTON3;
    // printf("0x%08x\n", [event buttonMask]);
  }
  if (is_proximity) {
    ev.pen_id = (int)[event vendorID];
    device_type_ = [event pointingDeviceType];
  }
  if (type == NSEventTypeTabletProximity) {
    if ([event isEnteringProximity]) {
      // Check if this is the first time we see this pen, or if the pen changed
      if (current_pen_id_ != ev.pen_id) {
        current_pen_id_ = ev.pen_id;
        auto it = trait_list_.find(current_pen_id_);
        if (it == trait_list_.end()) { // not found, create a new entry
          trait_list_[current_pen_id_] = Trait::DRIVER_AVAILABLE;
          trait_countdown_ = 5;
          pen_send_all(Fl::Pen::DETECTED, State::NONE);
          // printf("IN RANGE, NEW PEN\n");
        } else {
          pen_send_all(Fl::Pen::CHANGED, State::NONE);
          // printf("IN RANGE, CHANGED PEN\n");
        }
        trait_list_[0] = trait_list_[current_pen_id_];  // set current pen traits
      } else {
        pen_send_all(Fl::Pen::IN_RANGE, State::NONE);
        // printf("IN RANGE\n");
      }
    } else {
      pen_send_all(Fl::Pen::OUT_OF_RANGE, State::NONE);
      // printf("OUT OF RANGE\n");
    }
  }

  Fl_Widget *receiver = nullptr;
  bool pushed = false;
  bool event_data_copied = false;

  if (has_position) {
    if (trait_countdown_) {
      trait_countdown_--;
      if (ev.tilt_x != 0.0) current_pen_trait_ |= Trait::TILT_X;
      if (ev.tilt_y != 0.0) current_pen_trait_ |= Trait::TILT_Y;
      if (ev.pressure != 1.0) current_pen_trait_ |= Trait::PRESSURE;
      if (ev.barrel_pressure != 0.0) current_pen_trait_ |= Trait::BARREL_PRESSURE;
      if (ev.pen_id != 0) current_pen_trait_ |= Trait::PEN_ID;
      if (ev.twist != 0.0) current_pen_trait_ |= Trait::TWIST;
      //if (ev.proximity != 0) current_pen_trait_ |= Trait::PROXIMITY;
      trait_list_[current_pen_id_] = current_pen_trait_;
    }
    fl_xmousewin = eventWindow;
    if (pushed_ && pushed_->widget() && (Fl::pushed() == pushed_->widget())) {
      receiver = pushed_->widget();
      // TODO: check Fl::grab(), clear dnd_flag?
      pushed = true;
    } else {
      // TODO: check Fl::modal() and Fl::grab()
      auto bpen = below_pen_ ? below_pen_->widget() : nullptr;
      auto bmouse = Fl::belowmouse();
      auto bpen_old = bmouse && (bmouse == bpen) ? bpen : nullptr;
      auto bpen_now = find_below_pen(eventWindow, ev.x, ev.y);

      if (bpen_now != bpen_old) {
        if (bpen_old) {
          pen_send(bpen_old, Fl::Pen::LEAVE, State::NONE, event_data_copied);
        }
        below_pen_ = nullptr;
        if (bpen_now) {
          State state = (device_type_ == NSPointingDeviceTypeEraser) ? State::ERASER_HOVERS : State::TIP_HOVERS;
          if (pen_send(bpen_now, Fl::Pen::ENTER, state, event_data_copied)) {
            below_pen_ = subscriber_list_[bpen_now];
            Fl::belowmouse(bpen_now);
          }
        }
      }

      receiver = below_pen_ ? below_pen_->widget() : nullptr;
      if (!receiver)
        return 0;
    }
  } else {
    // Anything to do here?
  }

  if (!receiver)
    return 0;

  int ret = 0;
  if (is_down) {
    if (!pushed) {
      pushed_ = subscriber_list_[receiver];
      Fl::pushed(receiver);
    }
    State trigger = button_to_trigger([event buttonNumber], true);
    if ([event buttonNumber] == 0)
      ret = pen_send(receiver, Fl::Pen::TOUCH, trigger, event_data_copied);
    else
      ret = pen_send(receiver, Fl::Pen::BUTTON_PUSH, trigger, event_data_copied);
  } else if (is_up) {
    if ( (ev.state & State::ANY_DOWN) == State::NONE ) {
      Fl::pushed(nullptr);
      pushed_ = nullptr;
    }
    State trigger = button_to_trigger([event buttonNumber], true);
    if ([event buttonNumber] == 0)
      ret = pen_send(receiver, Fl::Pen::LIFT, trigger, event_data_copied);
    else
      ret = pen_send(receiver, Fl::Pen::BUTTON_RELEASE, trigger, event_data_copied);
  } else if (is_motion) {
    if (pushed) {
      ret = pen_send(receiver, Fl::Pen::DRAW, State::NONE, event_data_copied);
    } else {
      ret = pen_send(receiver, Fl::Pen::HOVER, State::NONE, event_data_copied);
    }
  }
  // Always return 1 because at this point, we capture pen events and don't
  // want mouse events anymore!
  return 1;
}
