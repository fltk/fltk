//
// Definition of Windows Pen/Tablet event driver.
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

#include <FL/platform.H>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include "../../Fl_Screen_Driver.H"

#include <windows.h>
#include <ole2.h>
#include <shellapi.h>
// Some versions of MinGW now require us to explicitly include winerror to get S_OK defined
#include <winerror.h>


extern Fl_Window *fl_xmousewin;

static constexpr uint8_t _FL_PEN = 0; // internal use
static constexpr uint8_t _FL_ERASER = 1; // internal use
static uint8_t device_type_ = _FL_PEN;

static int _e_x_down = 0;
static int _e_y_down = 0;

// Click counting state
static DWORD last_click_time_ = 0;
static int last_click_x_ = 0;
static int last_click_y_ = 0;
static  Fl::Pen::State last_click_trigger_ =  Fl::Pen::State::NONE;

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
  Fl::Pen::Trait::TILT_X |
  Fl::Pen::Trait::TILT_Y | Fl::Pen::Trait::TWIST
  // Notably missing: PROXIMITY, BARREL_PRESSURE
};

// Temporary storage of event data for the driver;
static Fl::Pen::EventData ev;


namespace Fl {

// namespace Private {

// // Global mouse position at mouse down event
// extern int e_x_down;
// extern int e_y_down;

// }; // namespace Private

namespace Pen {

class Windows_Driver : public Driver {
public:
  Windows_Driver() = default;
  //virtual void subscribe(Fl_Widget* widget) override;
  //virtual void unsubscribe(Fl_Widget* widget) override;
  //virtual void release() override;
  virtual Trait traits() override { return driver_traits_; }
  virtual Trait pen_traits(int pen_id) override {
    auto it = trait_list_.find(pen_id);
    if (pen_id == 0)
      return current_pen_trait_;
    if (it == trait_list_.end()) {
      return Trait::DRIVER_AVAILABLE;
    } else {
      return it->second;
    }
  }
};

Windows_Driver windows_driver;
Driver& driver { windows_driver };

} // namespace Pen

} // namespace Fl


using namespace Fl::Pen;

/*
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

/*
 Check if coordinates are within the widget box.
 Coordinates are in top_window space. We iterate up the hierarchy to ensure
 that we handle subwindows correctly.
 */
static bool event_inside(Fl_Widget *w, double x, double y) {
  if (w->as_window()) {
    return ((x >= 0) && (y >= 0) && (x < w->w()) && (y < w->h()));
  } else {
    return ((x >= w->x()) && (y >= w->y()) && (x < w->x() + w->w()) && (y < w->y() + w->h()));
  }
}

/*
 Find the widget under the pen event.
 Search the subscriber list for widgets that are inside the same window,
 are visible, and are within the give coordinates. Subwindow aware.
 */
static Fl_Widget *find_below_pen(Fl_Window *win, double x, double y) {
  for (auto &sub: subscriber_list_) {
    Fl_Widget *candidate = sub.second->widget();
    if (candidate && ((candidate == win) || (!candidate->as_window() && candidate->window() == win))) {
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
  Fl::e_x = e.x = ev.x;
  Fl::e_y = e.y = ev.y;
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
  return 1;
}

/*
 Convert the NSEvent button number to Fl::Pen::State,
 */
static State button_to_trigger(POINTER_BUTTON_CHANGE_TYPE button, bool down) {
  switch (button) {
    case POINTER_CHANGE_FIRSTBUTTON_DOWN:
    case POINTER_CHANGE_FIRSTBUTTON_UP:
      if ( (ev.state & (State::ERASER_DOWN | State::ERASER_HOVERS)) != State::NONE ) {
        return down ? State::ERASER_DOWN : State::ERASER_HOVERS;
      } else {
        return down ? State::TIP_DOWN : State::TIP_HOVERS;
      }
    case POINTER_CHANGE_SECONDBUTTON_DOWN:
    case POINTER_CHANGE_SECONDBUTTON_UP:
      return State::BUTTON0;
    case POINTER_CHANGE_THIRDBUTTON_DOWN:
    case POINTER_CHANGE_THIRDBUTTON_UP:
      return State::BUTTON1;
    case POINTER_CHANGE_FOURTHBUTTON_DOWN:
    case POINTER_CHANGE_FOURTHBUTTON_UP:
      return State::BUTTON2;
    case POINTER_CHANGE_FIFTHBUTTON_DOWN:
    case POINTER_CHANGE_FIFTHBUTTON_UP:
      return State::BUTTON3;
    default: return State::NONE;
  }
}

/*
 Handle events coming from the Win32 API.
 WM_TABLET (Windows 2000 and up)
 WM_POINTER (Windows 8 and up)
 https://learn.microsoft.com/en-us/windows/win32/inputmsg/messages-and-notifications-portal
 #if(WINVER >= 0x0602) ... #endif
    \return -1 if we did not handle the event and want the main event handler to call DefWindowProc()
    \return any other value that will then be return from WndProc() directly.
 */
LRESULT fl_win32_tablet_handler(MSG& msg) {
  auto message = msg.message;
  if (message < WM_NCPOINTERUPDATE || message > WM_POINTERROUTEDRELEASED) {
    return -1;
  }

  Fl_Window *eventWindow = fl_find(msg.hwnd);  // can be nullptr
  bool is_proximity = false;
  bool is_down = false;
  bool is_up = false;
  bool is_motion = false;

  switch (msg.message) {
    case WM_NCPOINTERDOWN: // pen pushed over window decoration, don't care
    case WM_NCPOINTERUP: // pen released over window decoration, don't care
    case WM_NCPOINTERUPDATE: // pen moved over decoration, don't care
    case WM_POINTERACTIVATE: // shall the pointer activate an inactive window?
      return -1; // let the system handle this forwarding this to DefWindowProc

    case WM_POINTERENTER: // pointer moved into window area from top or sides
      is_proximity = true;
      break;
    case WM_POINTERLEAVE: // left window area to top or sides
      is_proximity = true;
      break;


    case WM_POINTERDOWN:
      is_down = true;
      break;
    case WM_POINTERUP:
      is_up = true;
      break;
    case WM_POINTERUPDATE:
      is_motion = true;
      break;

    case WM_POINTERCAPTURECHANGED:
    case WM_TOUCHHITTESTING:
    case WM_POINTERWHEEL:
    case WM_POINTERHWHEEL:
    case DM_POINTERHITTEST:
    case WM_POINTERROUTEDTO:
    case WM_POINTERROUTEDAWAY:
    case WM_POINTERROUTEDRELEASED:
    default:
      // printf("Windows message: msg=0x%04X wParam=0x%08X lParam=0x%08X\n",
      //     msg.message, (unsigned)msg.wParam, (unsigned)msg.lParam);
      return -1;
  }
  // printf("    msg=0x%04X wParam=0x%08X lParam=0x%08X\n",
  //       msg.message, (unsigned)msg.wParam, (unsigned)msg.lParam);

  POINTER_PEN_INFO info;
  BOOL has_position = GetPointerPenInfo(
    GET_POINTERID_WPARAM(msg.wParam),
    &info
  );
  // if (has_position && info.pointerInfo.ButtonChangeType!=0) {
  //   printf("  pointerFlags: %08x [", (unsigned)info.pointerInfo.pointerFlags);
  //   if (info.pointerInfo.pointerFlags & POINTER_FLAG_FIRSTBUTTON) printf(" 1ST");
  //   if (info.pointerInfo.pointerFlags & POINTER_FLAG_SECONDBUTTON) printf(" 2ND");
  //   if (info.pointerInfo.pointerFlags & POINTER_FLAG_THIRDBUTTON) printf(" 3RD");
  //   if (info.pointerInfo.pointerFlags & POINTER_FLAG_FOURTHBUTTON) printf(" 4TH");
  //   if (info.pointerInfo.pointerFlags & POINTER_FLAG_FIFTHBUTTON) printf(" 5TH");
  //   printf(" ]\n  penFlags: %08x [", (unsigned)info.penFlags);
  //   if (info.penFlags & PEN_FLAG_BARREL) printf(" BARREL");
  //   if (info.penFlags & PEN_FLAG_INVERTED) printf(" INVERTED");
  //   if (info.penFlags & PEN_FLAG_ERASER) printf(" ERASER");
  //   printf(" ]\n  penMask: %08x  ButtonChangeType: %d\n",
  //     (unsigned)info.penMask, info.pointerInfo.ButtonChangeType);
  // }

  // Event has extended pen data set:
  if (has_position) {
    // Get the position data.
    double s = Fl::screen_driver()->scale(0);
    double ex = info.pointerInfo.ptPixelLocation.x/s;
    double ey = info.pointerInfo.ptPixelLocation.y/s;

    // Go from global coordinates to event window coordinates
    Fl_Widget *p = eventWindow;
    while (p) {
      if (p->as_window()) {
        ex -= p->x();
        ey -= p->y();
      }
      p = p->parent();
    };
    printf("pos: %d,%d (scale %.2f) to %.2f,%.2f %s\n",
      info.pointerInfo.ptPixelLocation.x,
      info.pointerInfo.ptPixelLocation.y,
      s,
      ex,
      ey, eventWindow->label());

    ev.x = ex; //info.pointerInfo.ptPixelLocation.x/s - eventWindow->x();
    ev.y = ey; //info.pointerInfo.ptPixelLocation.y/s - eventWindow->y();
    ev.rx = info.pointerInfo.ptPixelLocation.x/s;
    ev.ry = info.pointerInfo.ptPixelLocation.y/s;
    if (!is_proximity) {
      // Get the extended data.
      if (info.penMask & PEN_MASK_PRESSURE)
        ev.pressure = info.pressure / 1024.0;
      if (info.penMask & PEN_MASK_TILT_X)
        ev.tilt_x = -info.tiltX / 90.0;
      if (info.penMask & PEN_MASK_TILT_Y)
        ev.tilt_y = -info.tiltY / 90.0;
      if (info.penMask & PEN_MASK_ROTATION)
        ev.twist = info.rotation > 180 ? (info.rotation - 360) : info.rotation;
      if (info.pointerInfo.pointerFlags & POINTER_FLAG_INCONTACT)
        ev.proximity = 0.0;
      else
        ev.proximity = 1.0;
    }
    if (info.penFlags & PEN_FLAG_INVERTED) {
      device_type_ = _FL_ERASER;
      if (info.pointerInfo.pointerFlags & POINTER_FLAG_INCONTACT)
        ev.state = State::ERASER_DOWN;
      else
        ev.state = State::ERASER_HOVERS;
    } else {
      device_type_ = _FL_PEN;
      if (info.pointerInfo.pointerFlags & POINTER_FLAG_INCONTACT)
        ev.state = State::TIP_DOWN;
      else
        ev.state = State::TIP_HOVERS;
    }
    // Add pen barrel button states
    // Note: POINTER_FLAG_FIRSTBUTTON is the pen tip
    // PEN_FLAG_BARREL and POINTER_FLAG_SECONDBUTTON both indicate the primary barrel button
    if ((info.penFlags & PEN_FLAG_BARREL) || (info.pointerInfo.pointerFlags & POINTER_FLAG_SECONDBUTTON))
      ev.state |= State::BUTTON0;
    // Note: the following code does not work very well with the Wayland driver
    // More research is needed to find out how to get these button states reliably.
    if (info.pointerInfo.pointerFlags & POINTER_FLAG_THIRDBUTTON)  ev.state |= State::BUTTON1;
    if (info.pointerInfo.pointerFlags & POINTER_FLAG_FOURTHBUTTON) ev.state |= State::BUTTON2;
    if (info.pointerInfo.pointerFlags & POINTER_FLAG_FIFTHBUTTON)  ev.state |= State::BUTTON3;
  }
  // printf(" %08x\n", (unsigned)ev.state);
  if (is_proximity) {
    ev.pen_id = GET_POINTERID_WPARAM(msg.wParam);
  }
  if ((msg.message == WM_POINTERENTER) || (msg.message == WM_POINTERLEAVE)) {
    if (msg.message == WM_POINTERENTER) {
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
      if (Fl::grab() && (Fl::grab() != receiver->top_window()))
        return -1;
      if (Fl::modal() && (Fl::modal() != receiver->top_window()))
        return -1;
      pushed = true;
    } else {
      if (Fl::grab() && (Fl::grab() != eventWindow))
        return -1;
      if (Fl::modal() && (Fl::modal() != eventWindow))
        return -1;
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
          State state = (device_type_ == _FL_ERASER) ? State::ERASER_HOVERS : State::TIP_HOVERS;
          if (pen_send(bpen_now, Fl::Pen::ENTER, state, event_data_copied)) {
            below_pen_ = subscriber_list_[bpen_now];
            Fl::belowmouse(bpen_now);
          }
        }
      }

      receiver = below_pen_ ? below_pen_->widget() : nullptr;
      if (!receiver)
        return -1;
    }
  } else {
    // Proximity events were handled earlier.
  }

  if (!receiver)
    return -1;

  if (is_down) {
    if (!pushed) {
      pushed_ = subscriber_list_[receiver];
      Fl::pushed(receiver);
    }
    State trigger = button_to_trigger(info.pointerInfo.ButtonChangeType, true);
    if (msg.message == WM_POINTERDOWN) {
      Fl::e_is_click = 1;
      _e_x_down = (int)ev.x;
      _e_y_down = (int)ev.y;

      // Implement click counting using Windows system metrics
      DWORD current_time = GetMessageTime();
      DWORD double_click_time = GetDoubleClickTime();
      int double_click_dx = GetSystemMetrics(SM_CXDOUBLECLK) / 2;
      int double_click_dy = GetSystemMetrics(SM_CYDOUBLECLK) / 2;

      // Check if this is a multi-click: same trigger, within time and distance thresholds
      if (trigger == last_click_trigger_ &&
          (current_time - last_click_time_) < double_click_time &&
          abs((int)ev.rx - last_click_x_) < double_click_dx &&
          abs((int)ev.ry - last_click_y_) < double_click_dy) {
        Fl::e_clicks++;
      } else {
        Fl::e_clicks = 0;
      }

      last_click_time_ = current_time;
      last_click_x_ = (int)ev.rx;
      last_click_y_ = (int)ev.ry;
      last_click_trigger_ = trigger;

      pen_send(receiver, Fl::Pen::TOUCH, trigger, event_data_copied);
    } else {
      pen_send(receiver, Fl::Pen::BUTTON_PUSH, trigger, event_data_copied);
    }
  } else if (is_up) {
    if ( (ev.state & State::ANY_DOWN) == State::NONE ) {
      Fl::pushed(nullptr);
      pushed_ = nullptr;
    }
    State trigger = button_to_trigger(info.pointerInfo.ButtonChangeType, true);
    if (info.pointerInfo.ButtonChangeType == 0)
      pen_send(receiver, Fl::Pen::LIFT, trigger, event_data_copied);
    else
      pen_send(receiver, Fl::Pen::BUTTON_RELEASE, trigger, event_data_copied);
  } else if (is_motion) {
    if (  Fl::e_is_click &&
         ( (fabs((int)ev.x - _e_x_down) > 5) ||
           (fabs((int)ev.y - _e_y_down) > 5) ) )
      Fl::e_is_click = 0;
    if (pushed) {
      pen_send(receiver, Fl::Pen::DRAW, State::NONE, event_data_copied);
    } else {
      pen_send(receiver, Fl::Pen::HOVER, State::NONE, event_data_copied);
    }
  }
  // Always return 0 because at this point, we capture pen events and don't
  // want mouse events anymore!
  return 0;
}


