//
// Implementation of Windows (WinAPI) Tablet/Pen event driver for FLTK.
//
// Copyright 2025-2026 by Bill Spitzak and others.
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

/*
 Design notes — Windows Pointer Input vs. Wayland/Cocoa tablet architecture
 ───────────────────────────────────────────────────────────────────────────
 Cocoa and Wayland deliver tablet data as a stream of partial updates that
 must be accumulated and only dispatched once a "frame"/flush event arrives.

 The Windows Pointer Input API (WM_POINTER*, available since Windows 8) is
 simpler: every WM_POINTERDOWN / WM_POINTERUPDATE / WM_POINTERUP message
 already carries a complete, self-consistent snapshot of the pen, retrievable
 in one call to GetPointerPenInfo(). There is no separate "frame" message —
 each WM_POINTER* message *is* a frame. So this driver dispatches directly
 from the message handler, computing what changed by comparing against the
 pen's previously stored EventData/state.

 Tool lifecycle
 ──────────────
 First WM_POINTER* message for a given source device → device record created
 WM_POINTERENTER  → pen entered a window's hit-test area; DETECTED / CHANGED /
                     IN_RANGE broadcast (mirrors Wayland proximity_in)
 WM_POINTERDOWN/UPDATE/UP → TOUCH / LIFT / DRAW / HOVER / BUTTON_* dispatch
 WM_POINTERLEAVE  → if the pen also left tablet range, OUT_OF_RANGE broadcast
                     and full cleanup (mirrors Wayland proximity_out);
                     otherwise just window-local LEAVE bookkeeping
 WM_POINTERCAPTURECHANGED → forced cleanup if capture was lost mid-drag

 Coordinate mapping
 ──────────────────
 POINTER_INFO::ptPixelLocation is in physical screen pixels. It is converted
 to client coordinates with ScreenToClient(), then divided by the FLTK screen
 scale factor to obtain logical pixels, matching what Fl::Pen::e.x/y expects.
 Windows is top-down, like Wayland, so no Y-flip is required.

 Mouse fallback — IMPORTANT DIFFERENCE from Wayland/Cocoa
 ─────────────────────────────────────────────────────────
 On Wayland/Cocoa, a tablet tool produces *either* tablet events *or* pointer
 events for a given surface, never both — so those drivers must synthesize
 FL_PUSH/FL_DRAG/FL_MOVE/FL_RELEASE themselves when no Fl::Pen subscriber
 claims the event ("mouse fallback").

 On Windows, the system *always* additionally synthesizes legacy
 WM_*BUTTON* / WM_MOUSEMOVE messages from pen input (unless the process calls
 EnableMouseInPointer(TRUE), which this driver deliberately avoids). So this
 driver performs NO mouse fallback: when no subscriber wants the pen event,
 it simply does nothing and lets the normal WinAPI mouse path — driven by the
 legacy messages Windows already sends — handle it. See
 Fl_WinAPI_Pen_Events.H for the corresponding note about suppressing the
 legacy message when a pen event *is* consumed by a subscriber.

 Shared helper functions
 ───────────────────────
 offset_subwindow_event(), event_inside(), find_below_pen(), copy_state(),
 pen_send(), and pen_send_all() are identical in spirit to the Cocoa and
 Wayland drivers. They are duplicated here intentionally rather than
 elevated to Fl_Base_Pen_Events to avoid touching the shared API in this
 patch (same TODO as in Fl_Wayland_Pen_Events.cxx: factor these into
 Fl_Base_Pen_Events.cxx and expose via the header).
 */

#include "Fl_WinAPI_Pen_Events.H"
#include "src/drivers/Base/Fl_Base_Pen_Events.H"

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/platform.H>

#include <cmath>
#include <cstdint>
#include <map>
#include <iostream>

// fl_xmousewin tracks which window last received pointer/pen events.
extern Fl_Window *fl_xmousewin;

// Click detection needs the mouse-down position stored by Fl internals.
namespace Fl {
namespace Private {
extern int e_x_down;
extern int e_y_down;
} // namespace Private
} // namespace Fl

using namespace Fl::Pen;

static const State kButtonBits[] = {
    State::BUTTON0, State::BUTTON1, State::BUTTON2, State::BUTTON3
};


// ─────────────────────────────────────────────────────────────────────────────
// Per-device state
// ─────────────────────────────────────────────────────────────────────────────

/*
  One record per physical pen, keyed by POINTER_INFO::sourceDevice (the HID
  device handle for the digitizer, stable for the lifetime of the device).

  Windows does not expose a manufacturer pen serial number through the basic
  Pointer Input API, so pen_id is simply a small integer assigned the first
  time a given source device is seen (mirrors the "no hardware serial" path
  in the Wayland driver).
*/
struct PenDevice {
  HANDLE     source_device;
  int        pen_id;
  Trait      capabilities;
  bool       is_new;          // true until the first WM_POINTERENTER
  bool       in_proximity;
  Fl_Window *focus_win;        // top-level FLTK window currently under the pen
  HWND       focus_hwnd;

  EventData  ev;               // current accumulated event data
  State      prev_state;       // ev.state as of the last dispatched event
};

static std::map<HANDLE, PenDevice> g_devices;
// Most-recently-active device; used for DETECTED / CHANGED / IN_RANGE logic.
static PenDevice *g_current_tool { nullptr };
// Counter for assigning pen_ids to newly seen devices.
static int g_next_pen_id { 1 };


// ─────────────────────────────────────────────────────────────────────────────
// WinAPI pen driver class
// ─────────────────────────────────────────────────────────────────────────────

namespace Fl {
namespace Pen {

static WinAPI_Driver winapi_driver_instance;
// Define the extern Driver& declared in Fl_Base_Pen_Events.H.
Driver& driver = winapi_driver_instance;

Trait WinAPI_Driver::traits() {
  // The Pointer Input API used by this driver is available on Windows 8 and
  // later, which this build targets (see Fl_WinAPI_Pen_Events.H), so the
  // driver itself is always considered available.
  Trait t = Trait::DRIVER_AVAILABLE | Trait::PEN_ID | Trait::ERASER |
            Trait::PRESSURE | Trait::TILT_X | Trait::TILT_Y | Trait::TWIST;
  if (!g_devices.empty())
    t |= Trait::DETECTED;
  // Note: BARREL_PRESSURE (tangential/slider pressure) and PROXIMITY (hover
  // distance) are not reported by POINTER_PEN_INFO and are therefore not
  // advertised; Fl::Pen::event_barrel_pressure() and event_proximity() will
  // return their documented defaults (0.0).
  return t;
}

Trait WinAPI_Driver::pen_traits(int pen_id) {
  for (auto &kv : g_devices) {
    PenDevice &dev = kv.second;
    if (pen_id == 0) {
      if (&dev == g_current_tool) return dev.capabilities;
    } else if (dev.pen_id == pen_id) {
      return dev.capabilities;
    }
  }
  return Trait::NONE;
}

void WinAPI_Driver::release() {
  if (pushed_) ReleaseCapture();
  Driver::release();
}

} // namespace Pen
} // namespace Fl


// ─────────────────────────────────────────────────────────────────────────────
// Platform-independent helper functions
// (TODO: factor into Fl_Base_Pen_Events.cxx, same code as Cocoa/Wayland drivers)
// ─────────────────────────────────────────────────────────────────────────────

/*
 Walk the widget's window ancestry and subtract each sub-window's origin from
 (x, y) so that the coordinates become widget-local.
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
  }
}

/*
 Return true if (x, y) in top-window space falls inside widget w.
 */
static bool event_inside(Fl_Widget *w, double x, double y) {
  offset_subwindow_event(w, x, y);
  if (w->as_window())
    return (x >= 0 && y >= 0 && x < w->w() && y < w->h());
  return (x >= w->x() && y >= w->y() &&
          x < w->x() + w->w() && y < w->y() + w->h());
}

/*
 Search the subscriber list for the topmost subscribed widget inside (x, y)
 that belongs to top-window win.
 */
static Fl_Widget *find_below_pen(Fl_Window *win, double x, double y) {
  for (auto &sub : subscriber_list_) {
    Fl_Widget *w = sub.second->widget();
    if (w && w->top_window() == win && w->visible() && event_inside(w, x, y))
      return w;
  }
  return nullptr;
}

/*
 Commit dev->ev into the global Fl::Pen::e and update Fl::e_x/y/root.
 The trigger is the XOR of the old and new state (bits that changed).
 */
static void copy_state(PenDevice *dev) {
  State tr = (State)((uint32_t)e.state ^ (uint32_t)dev->ev.state);
  e = dev->ev;
  e.trigger    = tr;
  Fl::e_x      = (int)dev->ev.x;
  Fl::e_y      = (int)dev->ev.y;
  Fl::e_x_root = (int)dev->ev.rx;
  Fl::e_y_root = (int)dev->ev.ry;
}

/*
 Dispatch a single pen event to widget w.
 Commits event data the first time (lazy copy), then recomputes w's local
 coordinates before calling w->handle().
 */
static int pen_send(PenDevice *dev, Fl_Widget *w, int event,
                     State trigger, bool &copied) {
  if (!copied) {
    copy_state(dev);
    copied = true;
  }
  // Recompute widget-local coordinates for this specific widget each call,
  // because different subscribers may sit at different depths.
  e.x = dev->ev.x;
  e.y = dev->ev.y;
  offset_subwindow_event(w, e.x, e.y);
  Fl::e_x    = (int)e.x;
  Fl::e_y    = (int)e.y;
  e.trigger  = trigger;
  return w->handle(event);
}

/*
 Broadcast event+trigger to every subscriber.
 */
static int pen_send_all(PenDevice *dev, int event, State trigger) {
  bool copied = false;
  for (auto &it : subscriber_list_) {
    Fl_Widget *w = it.second->widget();
    if (w) pen_send(dev, w, event, trigger, copied);
  }
  return 1;
}

/*
 Return the bits set in `mask` that are present in `b` but not in `a`
 (i.e. the bits that newly became set when the state changed from a to b).
 Defined locally because the Fl::Pen::State enum class only provides |, &,
 and |=.
 */
static inline State newly_set_bits(State a, State b, State mask) {
  uint32_t old_bits = (uint32_t)(a & mask);
  uint32_t new_bits = (uint32_t)(b & mask);
  return (State)(new_bits & ~old_bits);
}

/*
 Return true if state s represents the eraser end of the pen (hovering or
 down), as opposed to the tip.
 */
static inline bool is_eraser_state(State s) {
  return (s & (State::ERASER_HOVERS | State::ERASER_DOWN)) != (State)0;
}

/*
 Return true if state s represents the tip or eraser touching the surface.
 */
static inline bool is_down_state(State s) {
  return (s & (State::TIP_DOWN | State::ERASER_DOWN)) != (State)0;
}


// ─────────────────────────────────────────────────────────────────────────────
// POINTER_PEN_INFO → Fl::Pen::EventData / State conversion
// ─────────────────────────────────────────────────────────────────────────────

/*
 Convert POINTER_INFO::ptPixelLocation (physical screen pixels) into FLTK
 logical-pixel widget coordinates and store them in dev->ev, similarly to
 coords_from_surface() in the Wayland driver.
 */
static void update_position(PenDevice *dev, HWND hwnd,
                              const POINTER_INFO &pointer_info) {
  Fl_Window *win = fl_find(hwnd);
  if (!win) return;

  POINT pt = pointer_info.ptPixelLocation;
  ScreenToClient(hwnd, &pt);

  // Accumulate sub-window offsets while walking to the top-level window.
  // (FLTK sub-windows normally share their parent's HWND, so this loop is
  // usually a no-op; it mirrors the Wayland driver for the general case.)
  int delta_x = 0, delta_y = 0;
  Fl_Window *w = win;
  while (w->parent()) {
    delta_x += w->x();
    delta_y += w->y();
    w = w->window();
  }
  Fl_Window *top = w;

  float f = Fl::screen_scale(top->screen_num());

  dev->ev.x  = pt.x / f + delta_x;
  dev->ev.y  = pt.y / f + delta_y;
  dev->ev.rx = dev->ev.x + top->x();
  dev->ev.ry = dev->ev.y + top->y();

  dev->focus_win  = top;
  dev->focus_hwnd = hwnd;
}

/*
 Translate the contact/eraser/button bits of a POINTER_PEN_INFO into an
 Fl::Pen::State value.
 */
static State compute_pen_state(const POINTER_PEN_INFO &pi) {
  bool in_contact = (pi.pointerInfo.pointerFlags & POINTER_FLAG_INCONTACT) != 0;
  // PEN_FLAG_INVERTED: pen is flipped to use the eraser end (hovering).
  // PEN_FLAG_ERASER:   the eraser end is pressed against the surface.
  bool eraser_end = (pi.penFlags & (PEN_FLAG_INVERTED | PEN_FLAG_ERASER)) != 0;

  State s;
  if (eraser_end)
    s = in_contact ? State::ERASER_DOWN : State::ERASER_HOVERS;
  else
    s = in_contact ? State::TIP_DOWN : State::TIP_HOVERS;

  // Barrel buttons. For pen pointers, FIRSTBUTTON corresponds to the tip
  // contact itself (already represented by TIP_DOWN/ERASER_DOWN above), so
  // the SECOND..FIFTH button flags are mapped to the FLTK barrel buttons.
  if (pi.pointerInfo.pointerFlags & POINTER_FLAG_SECONDBUTTON) s |= State::BUTTON0;
  if (pi.pointerInfo.pointerFlags & POINTER_FLAG_THIRDBUTTON)  s |= State::BUTTON1;
  if (pi.pointerInfo.pointerFlags & POINTER_FLAG_FOURTHBUTTON) s |= State::BUTTON2;
  if (pi.pointerInfo.pointerFlags & POINTER_FLAG_FIFTHBUTTON)  s |= State::BUTTON3;

  return s;
}

/*
 Update pressure, tilt, and rotation in dev->ev from the pen-specific axes
 reported in pi. Only fields whose corresponding PEN_MASK_* bit is set are
 considered valid; others retain their previous (or default) value.
 */
static void update_axes(PenDevice *dev, const POINTER_PEN_INFO &pi) {
  // Windows: pressure in [0, 1024] -> FLTK: [0.0, 1.0]
  if (pi.penMask & PEN_MASK_PRESSURE)
    dev->ev.pressure = pi.pressure / 1024.0;

  // Windows: tilt in [-90, 90] degrees -> FLTK: [-1.0, 1.0]
  if (pi.penMask & PEN_MASK_TILT_X)
    dev->ev.tilt_x = -pi.tiltX / 90.0;
  if (pi.penMask & PEN_MASK_TILT_Y)
    dev->ev.tilt_y = -pi.tiltY / 90.0;

  // Windows: rotation in [0, 360) degrees clockwise -> FLTK: [-180, 180]
  // degrees, matching the convention documented in pen_events.H.
  if (pi.penMask & PEN_MASK_ROTATION) {
    double deg = pi.rotation;
    dev->ev.twist = (deg > 180.0) ? deg - 360.0 : deg;
  }

  // barrel_pressure and proximity are not reported via POINTER_PEN_INFO and
  // keep their EventData defaults (0.0).
}


// ─────────────────────────────────────────────────────────────────────────────
// Device lookup / creation
// ─────────────────────────────────────────────────────────────────────────────

static PenDevice *get_or_create_device(HANDLE source_device) {
  auto it = g_devices.find(source_device);
  if (it != g_devices.end())
    return &it->second;

  PenDevice dev{};
  dev.source_device = source_device;
  dev.pen_id        = g_next_pen_id++;
  dev.capabilities  = Trait::DRIVER_AVAILABLE | Trait::PEN_ID | Trait::ERASER |
                       Trait::PRESSURE | Trait::TILT_X | Trait::TILT_Y |
                       Trait::TWIST;
  dev.is_new        = true;
  dev.in_proximity  = false;
  dev.focus_win     = nullptr;
  dev.focus_hwnd    = nullptr;
  dev.ev            = EventData(); // value-initialized defaults from struct
  dev.ev.pen_id     = dev.pen_id;
  dev.prev_state    = (State)0;

  auto res = g_devices.emplace(source_device, dev);
  return &res.first->second;
}


// ─────────────────────────────────────────────────────────────────────────────
// WM_POINTER* message handling
// ─────────────────────────────────────────────────────────────────────────────

/*
 WM_POINTERENTER — the pen entered the hit-test area of hwnd.
 Mirrors the proximity-in branch of the Wayland tool_cb_frame().
 */
static bool handle_proximity_in(PenDevice *dev, HWND hwnd,
                                 const POINTER_PEN_INFO &pi) {
  update_position(dev, hwnd, pi.pointerInfo);
  update_axes(dev, pi);
  dev->ev.pen_id = dev->pen_id;
  dev->ev.state  = compute_pen_state(pi);
  dev->in_proximity = true;

  if (dev->is_new) {
    dev->is_new = false;
    pen_send_all(dev, Fl::Pen::DETECTED, (State)0);
  } else if (g_current_tool && g_current_tool != dev) {
    pen_send_all(dev, Fl::Pen::CHANGED, (State)0);
  } else {
    pen_send_all(dev, Fl::Pen::IN_RANGE, (State)0);
  }

  g_current_tool   = dev;
  dev->prev_state  = dev->ev.state;

  return false; // Handled here, but also let WIN32 synthesize mouse events
}

/*
 WM_POINTERLEAVE — the pen left hwnd's hit-test area, and possibly the
 tablet's proximity range entirely.

 IS_POINTER_INRANGE_WPARAM(wParam) tells us which case applies:
  - false: the pen left proximity altogether -> OUT_OF_RANGE broadcast and
    full cleanup, mirroring Wayland's proximity_out.
  - true: the pen is still in range but moved off this window. There is no
    further "frame" to naturally re-evaluate find_below_pen(), so the
    below_pen_ widget (if it belongs to this window) is sent LEAVE here.
 */
static bool handle_leave(PenDevice *dev, WPARAM wParam)
{
  bool ret = false;
  bool still_in_range = IS_POINTER_INRANGE_WPARAM(wParam);

  if (!still_in_range) {
    pen_send_all(dev, Fl::Pen::OUT_OF_RANGE, (State)0);

    if (below_pen_ && below_pen_->widget()) {
      bool copied = false;
      ret = pen_send(dev, below_pen_->widget(), Fl::Pen::LEAVE, (State)0, copied);
    }
    below_pen_ = nullptr;

    if (pushed_) {
      Fl::pushed(nullptr);
      pushed_ = nullptr;
      ReleaseCapture();
    }

    if (g_current_tool == dev) g_current_tool = nullptr;
    dev->in_proximity = false;
    dev->focus_win    = nullptr;
    dev->focus_hwnd   = nullptr;

    return ret;
  }

  // Still in range, but no longer over this window.
  if (below_pen_ && below_pen_->widget() &&
      below_pen_->widget()->top_window() == dev->focus_win) {
    bool copied = false;
    ret = pen_send(dev, below_pen_->widget(), Fl::Pen::LEAVE, (State)0, copied);
    below_pen_ = nullptr;
    Fl::belowmouse(nullptr);
  }
  // If pushed_ is set (an active tip-down drag) and the window called
  // SetCapture() in handle_update() below, WM_POINTERUPDATE messages will
  // keep arriving for hwnd even though the cursor is now outside its
  // bounds, so pushed_ is intentionally left untouched here.
  dev->focus_win  = nullptr;
  dev->focus_hwnd = nullptr;

  return ret;
}

/*
 WM_POINTERCAPTURECHANGED — pointer capture was taken away from us (e.g. a
 modal dialog or another window grabbed it) while a pen drag was in
 progress. Make sure the widget that was pushed gets a closing LIFT event
 and FLTK's pushed state is cleared, so it doesn't get stuck.
 */
static bool handle_capture_lost(PenDevice *dev) {
  bool ret = false;
  if (pushed_ && pushed_->widget()) {
    if (is_down_state(dev->ev.state)) {
      bool copied = false;
      State trigger = is_eraser_state(dev->ev.state)
        ? State::ERASER_HOVERS : State::TIP_HOVERS;
      pen_send(dev, pushed_->widget(), Fl::Pen::LIFT, trigger, copied);
      dev->ev.state = trigger; // clear the down bit, preserve barrel buttons
    }
    Fl::pushed(nullptr);
    pushed_ = nullptr;
    ret = true;
  }
  dev->prev_state = dev->ev.state;
  return ret;
}

/*
 WM_POINTERDOWN / WM_POINTERUPDATE / WM_POINTERUP — the main dispatch point.

 Each such message already carries a complete pen snapshot, so this function
 directly mirrors the body of the Wayland driver's tool_cb_frame() (steps
 3-9), comparing the new state against dev->prev_state to detect TOUCH /
 LIFT / BUTTON_PUSH / BUTTON_RELEASE transitions, then sending DRAW or HOVER
 for any positional change.

 Ordering:
   1. modal/grab guard
   2. below_pen ENTER/LEAVE tracking (or "pushed" receiver)
   3. tip/eraser down  -> TOUCH
   4. tip/eraser up    -> LIFT
   5. barrel buttons   -> BUTTON_PUSH / BUTTON_RELEASE
   6. motion           -> DRAW (if pushed) or HOVER
 */
static bool handle_update(PenDevice *dev, HWND hwnd, UINT msg,
                           const POINTER_PEN_INFO &pi)
{
  int       pen_handled = 0;
  EventData prev_ev  = dev->ev;
  State     old_state = dev->prev_state;

  update_position(dev, hwnd, pi.pointerInfo);
  update_axes(dev, pi);
  dev->ev.pen_id = dev->pen_id;

  State new_state = compute_pen_state(pi);
  dev->ev.state   = new_state;

  bool frame_down   = !is_down_state(old_state) && is_down_state(new_state);
  bool frame_up     =  is_down_state(old_state) && !is_down_state(new_state);
  bool frame_motion = (dev->ev.x != prev_ev.x) || (dev->ev.y != prev_ev.y) ||
                       (msg == WM_POINTERUPDATE);

  if (!dev->focus_win || subscriber_list_.empty()) {
    dev->prev_state = new_state;
    return pen_handled;
  }

  Fl_Window *eventWindow = dev->focus_win;
  bool is_menu_window = eventWindow->menu_window();

  if (!is_menu_window) {
    // ── 1. Modal / grab guards ──────────────────────────────────────────────
    if (Fl::grab() && Fl::grab() != eventWindow) {
      dev->prev_state = new_state;
      return pen_handled;
    }
    if (Fl::modal() && Fl::modal() != eventWindow) {
      dev->prev_state = new_state;
      return pen_handled;
    }
  }

  fl_xmousewin = eventWindow;

  bool       event_data_copied = false;
  Fl_Widget *receiver  = nullptr;
  bool       is_pushed = false;

  // ── 2. Receiver selection & below_pen ENTER/LEAVE ─────────────────────────
  if (pushed_ && pushed_->widget() && Fl::pushed() == pushed_->widget()) {

    // An earlier tip-down fixed this device's receiver until the tip lifts.
    receiver  = pushed_->widget();
    is_pushed = true;
  } else {
    auto bpen_widget = below_pen_ ? below_pen_->widget() : nullptr;
    auto bpen_old    = (Fl::belowmouse() == bpen_widget) ? bpen_widget : nullptr;
    auto bpen_now    = find_below_pen(eventWindow, dev->ev.x, dev->ev.y);

    if (bpen_now != bpen_old) {
      if (bpen_old) {
        pen_send(dev, bpen_old, Fl::Pen::LEAVE, (State)0, event_data_copied);
      }
      below_pen_ = nullptr;
      if (bpen_now) {
        State hover_state = is_eraser_state(new_state)
          ? State::ERASER_HOVERS : State::TIP_HOVERS;
        if (pen_send(dev, bpen_now, Fl::Pen::ENTER, hover_state,
                      event_data_copied)) {
          below_pen_ = subscriber_list_[bpen_now];
          Fl::belowmouse(bpen_now);
        }
      }
    }
    receiver = below_pen_ ? below_pen_->widget() : nullptr;
  }

  if (!receiver) {
    // No subscribed widget claimed this pen position. Nothing further to do
    // here -- the legacy mouse messages Windows generates for this pen
    // input drive the normal FLTK mouse path. (See the "Mouse fallback"
    // section in the file header comment.)
    dev->prev_state = new_state;
    return pen_handled;
  }

  Fl_Widget_Tracker receiver_tracker(receiver);

  // ── 3. Tip/eraser down -> TOUCH ───────────────────────────────────────────
  if (frame_down) {
    if (!is_pushed) {
      pushed_ = subscriber_list_[receiver];
      Fl::pushed(receiver);
      // Capture pointer input so a drag that leaves hwnd's bounds keeps
      // generating WM_POINTERUPDATE for this pen.
      SetCapture(hwnd);
    }

    Fl::e_is_click        = 1;
    Fl::Private::e_x_down = (int)dev->ev.x;
    Fl::Private::e_y_down = (int)dev->ev.y;
    Fl::e_clicks          = 0;
    pen_handled |= pen_send(dev, receiver, Fl::Pen::TOUCH,
                             new_state & (State::TIP_DOWN | State::ERASER_DOWN),
                             event_data_copied);
    if (receiver_tracker.deleted()) {
      Fl::Pen::unsubscribe(receiver);
      return pen_handled;
    }
  }

  // ── 4. Tip/eraser up -> LIFT ──────────────────────────────────────────────
  if (frame_up) {
    if ((new_state & State::ANY_DOWN) == (State)0) {
      Fl::pushed(nullptr);
      pushed_ = nullptr;
      ReleaseCapture();
    }

    State trigger = is_eraser_state(new_state)
      ? State::ERASER_HOVERS : State::TIP_HOVERS;
    pen_handled |= pen_send(dev, receiver, Fl::Pen::LIFT, trigger,
                             event_data_copied);
    if (receiver_tracker.deleted()) {
      Fl::Pen::unsubscribe(receiver);
      return pen_handled;
    }
  }

  // ── 5. Barrel button events ───────────────────────────────────────────────
  for (State bit : kButtonBits) {
    if (newly_set_bits(old_state, new_state, bit) != (State)0) {
      pen_handled |= pen_send(dev, receiver, Fl::Pen::BUTTON_PUSH, bit,
                               event_data_copied);
      if (receiver_tracker.deleted()) {
        Fl::Pen::unsubscribe(receiver);
        return pen_handled;
      }
    }
    if (newly_set_bits(new_state, old_state, bit) != (State)0)
    {
      pen_handled |= pen_send(dev, receiver, Fl::Pen::BUTTON_RELEASE, bit,
                               event_data_copied);
      if (receiver_tracker.deleted()) {
        Fl::Pen::unsubscribe(receiver);
        return pen_handled;
      }
    }
  }

  // ── 6. Motion -> DRAW or HOVER ────────────────────────────────────────────
  if (frame_motion) {
    if (Fl::e_is_click &&
        (std::fabs(dev->ev.x - Fl::Private::e_x_down) > 5.0 ||
         std::fabs(dev->ev.y - Fl::Private::e_y_down) > 5.0))
      Fl::e_is_click = 0;
    pen_handled |= pen_send(dev, receiver,
                             is_pushed ? Fl::Pen::DRAW : Fl::Pen::HOVER,
                             (State)0, event_data_copied);
    if (receiver_tracker.deleted()) {
      Fl::Pen::unsubscribe(receiver);
      return pen_handled;
    }
  }

  dev->prev_state = new_state;

  return pen_handled;
}


// ─────────────────────────────────────────────────────────────────────────────
// Public entry point — called from the WinAPI WndProc
// ─────────────────────────────────────────────────────────────────────────────

/**
  If the event is a WM_POINTER* message, handle it, and return true if no further processing is needed.
  Called from Fl_win32.cxx WndProc.
  \param hwnd The window receiving the message.
  \param msg The message code (e.g. WM_POINTERDOWN).
  \param wParam The WPARAM of the message, containing the pointer ID and flags.
  \param lParam The LPARAM of the message (unused).
  \return true if the message was handled as a pen event and should not be processed further;
  \return false to have WndProc call DefWindowProcW, which in turn will synthesize the
  the pen event into a matching mouse event.
 */
FL_EXPORT bool fl_winapi_pen_handle(HWND hwnd, UINT msg, WPARAM wParam, LPARAM /*lParam*/)
{
  bool ret = false;

  switch (msg) {
    case WM_POINTERENTER:
    case WM_POINTERLEAVE:
    case WM_POINTERDOWN:
    case WM_POINTERUP:
    case WM_POINTERUPDATE:
    case WM_POINTERCAPTURECHANGED:
      break;
    default:
      return ret;
  }


  if (subscriber_list_.empty()) {
    // No widget cares about pen events right now; let normal mouse handling
    // take over without doing any bookkeeping that would need to be undone
    // later if a subscription appears.
    return ret;
  }


  UINT32 pointer_id = GET_POINTERID_WPARAM(wParam);


  POINTER_INPUT_TYPE ptype;
  if (!GetPointerType(pointer_id, &ptype) || ptype != PT_PEN)
    return ret; // touch, mouse-emulated pointer, etc -- not ours.


  POINTER_PEN_INFO pi;
  if (!GetPointerPenInfo(pointer_id, &pi))
    return ret;


  PenDevice *dev = get_or_create_device(pi.pointerInfo.sourceDevice);
  if (!dev)
    return ret;

  switch (msg) {
    case WM_POINTERENTER:
      ret = handle_proximity_in(dev, hwnd, pi);
      break;
    case WM_POINTERLEAVE:
      ret = handle_leave(dev, wParam);
      break;
    case WM_POINTERCAPTURECHANGED:
      ret = handle_capture_lost(dev);
      break;
    case WM_POINTERDOWN:
    case WM_POINTERUPDATE:
    case WM_POINTERUP:
      ret = handle_update(dev, hwnd, msg, pi);
      break;
  }

  return ret;
}
