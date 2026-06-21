//
// Implementation of Wayland Tablet/Pen event driver for FLTK.
//
// Copyright 2025-2026 by Bill Spitzak and others.
// Original Implementation by ClaudeAI.  Bug fixing by Grok.
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
 Design notes — Wayland vs. Cocoa tablet architecture
 ─────────────────────────────────────────────────────
 Cocoa delivers each tablet event as a single, complete NSEvent.
 fl_cocoa_tablet_handler() reads everything from that one object and
 dispatches immediately.

 The Wayland zwp_tablet_stable_v2 protocol splits the same information
 across a burst of per-tool callbacks (motion, pressure, tilt, down, …)
 that are bracketed by a "frame" event.  Only when frame() fires is the
 accumulated data coherent and ready to dispatch.  This file follows the
 same logical structure as the Cocoa driver but defers all dispatch to
 tool_cb_frame().

 Tool lifecycle
 ──────────────
 tool_added   → allocate TabletTool, receive type/serial/capability events
 tool_done    → capabilities are stable; finalize pen_id and trait set
 proximity_in → pen approaches a surface; note focus window
 (motion / pressure / tilt / … events accumulate)
 frame        → dispatch FLTK pen events to subscribers
 proximity_out→ pen left all surfaces; send OUT_OF_RANGE
 tool_removed → free TabletTool

 Coordinate mapping
 ──────────────────
 Wayland surface coordinates are wl_fixed_t (24.8 fixed-point) in logical
 pixels.  Dividing by the screen scale factor gives FLTK logical pixels,
 which is what Fl::Pen::e.x/y expects.  Unlike Cocoa there is no Y-flip;
 Wayland is already top-down.

 Capability / trait discovery
 ─────────────────────────────
 Cocoa requires watching the first few motion events to discover which
 axes a pen actually moves.  Wayland reports capabilities explicitly via
 capability events before tool_done(), so no discovery countdown is needed.

 Shared helper functions
 ───────────────────────
 offset_subwindow_event(), event_inside(), find_below_pen(), copy_state(),
 pen_send(), and pen_send_all() are identical in the Cocoa and Wayland
 drivers.  They are duplicated here intentionally rather than elevated to
 Fl_Base_Pen_Events to avoid touching the shared API in this patch.
 TODO: move them to Fl_Base_Pen_Events.cxx and expose via the header.
 */

#include "Fl_Wayland_Pen_Events.H"
#include "src/drivers/Base/Fl_Base_Pen_Events.H"
#include "Fl_Wayland_Screen_Driver.H"
#include "Fl_Wayland_Window_Driver.H"
#include "../../Fl_Window_Driver.H"

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/platform.H>

#include "tablet-client-protocol.h"

#include <cmath>
#include <cstring>
#include <cstdint>

// Physical stylus button codes from linux/input-event-codes.h.
// Defined locally so we don't depend on the kernel header directly.
#ifndef BTN_STYLUS
#  define BTN_STYLUS  0x14b   // first barrel / side button
#endif
#ifndef BTN_STYLUS2
#  define BTN_STYLUS2 0x14c   // second barrel button
#endif
#ifndef BTN_STYLUS3
#  define BTN_STYLUS3 0x149   // third barrel button (uncommon)
#endif

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

// C+11 Safe defined
static const State kButtonBits[] = {
    State::BUTTON0, State::BUTTON1, State::BUTTON2, State::BUTTON3
};

// ─────────────────────────────────────────────────────────────────────────────
// Per-tool state
// ─────────────────────────────────────────────────────────────────────────────

struct TabletTool {
  struct zwp_tablet_tool_v2      *wl_tool;
  enum zwp_tablet_tool_v2_type    type;
  uint64_t                        hardware_serial;
  int                             pen_id;       // int-sized pen identity
  Trait                           capabilities; // reported by capability events
  bool                            is_new;       // true until first proximity_in
  bool                            in_proximity;
  Fl_Window                      *focus_win;    // toplevel window under the pen
  struct wl_surface              *focus_surface;

  // Per-frame accumulated event data (flushed by tool_cb_frame)
  EventData  ev;
  State      prev_state;          // ev.state committed after last frame

  // Per-frame event occurrence flags (reset at end of each frame)
  bool  frame_proximity_in;
  bool  frame_proximity_out;
  bool  frame_down;               // tip touched surface this frame
  bool  frame_up;                 // tip left surface this frame
  bool  frame_motion;             // position changed this frame
  State frame_buttons_pressed;    // which BUTTON bits went active this frame
  State frame_buttons_released;   // which BUTTON bits went inactive this frame

  struct wl_list link;            // node in g_tool_list
};

// Convenience: reset all per-frame flags at the end of frame processing.
static inline void tablet_tool_reset_frame(TabletTool *t) {
  t->frame_proximity_in    = false;
  t->frame_proximity_out   = false;
  t->frame_down            = false;
  t->frame_up              = false;
  t->frame_motion          = false;
  t->frame_buttons_pressed  = (State)0;
  t->frame_buttons_released = (State)0;
}


// ─────────────────────────────────────────────────────────────────────────────
// Driver-level state (one per Wayland display/seat)
// ─────────────────────────────────────────────────────────────────────────────

static struct zwp_tablet_manager_v2 *g_tablet_manager { nullptr };
static struct wl_seat               *g_wl_seat         { nullptr };
static struct zwp_tablet_seat_v2    *g_tablet_seat     { nullptr };
static struct wl_list                g_tool_list;       // list<TabletTool>
// Most-recently-active tool; used for DETECTED / CHANGED / IN_RANGE logic.
static TabletTool *g_current_tool { nullptr };
// Counter for assigning pen_ids to tools that report no hardware serial.
static int g_next_pen_id { 1 };

// ─────────────────────────────────────────────────────────────────────────────
// Wayland pen driver class
// ─────────────────────────────────────────────────────────────────────────────
namespace Fl {
namespace Pen {

class Wayland_Driver : public Driver {
public:
  Wayland_Driver() = default;
  void subscribe(Fl_Widget* widget) override;
  Trait traits()            override;
  Trait pen_traits(int id)  override;
};

static Wayland_Driver wayland_driver_instance;
// Define the extern Driver& declared in Fl_Base_Pen_Events.H.
Driver& driver = wayland_driver_instance;

void Wayland_Driver::subscribe(Fl_Widget* widget)
{
    if (subscriber_list_.size() == 0)
    {
        Fl_Wayland_Screen_Driver* scr_driver = (Fl_Wayland_Screen_Driver*)Fl::screen_driver();
        if (scr_driver)
        {
            fl_open_display();
            wl_seat* seat = scr_driver->get_wl_seat();
            if (!seat)
            {
                Fl::warning("No screen to wl_seat init tablet");
            }
            else
            {
                fl_wayland_tablet_init_seat(seat);
            }
        }
        else
        {
            Fl::warning("No screen driver to init tablet");
        }
    }
    Driver::subscribe(widget);
}

Trait Wayland_Driver::traits() {
  if (!g_tablet_seat) return Trait::NONE;
  // Aggregate across all known tools; a connected tablet can do at least this.
  return Trait::DRIVER_AVAILABLE | Trait::PEN_ID | Trait::ERASER |
         Trait::PRESSURE | Trait::BARREL_PRESSURE |
         Trait::TILT_X | Trait::TILT_Y | Trait::TWIST;
}

Trait Wayland_Driver::pen_traits(int pen_id) {
  if (!g_tablet_seat) return Trait::NONE;
  // pen_id == 0 means "current tool"
  TabletTool *match = nullptr;
  TabletTool *t;
  wl_list_for_each(t, &g_tool_list, link) {
    if (pen_id == 0) {
      if (t == g_current_tool) { match = t; break; }
    } else {
      if (t->pen_id == pen_id) { match = t; break; }
    }
  }
  return match ? match->capabilities : Trait::NONE;
}

} // namespace Pen
} // namespace Fl


// ─────────────────────────────────────────────────────────────────────────────
// Platform-independent helper functions
// (TODO: factor into Fl_Base_Pen_Events.cxx, same code as Cocoa driver)
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
 Commit tool->ev into the global Fl::Pen::e and update Fl::e_x/y/root.
 The trigger is the XOR of the old and new state (bits that changed).
 */
static void copy_state(TabletTool *tool) {
  State tr = (State)((uint32_t)e.state ^ (uint32_t)tool->ev.state);
  e = tool->ev;
  e.trigger    = tr;
  Fl::e_x      = (int)tool->ev.x;
  Fl::e_y      = (int)tool->ev.y;
  Fl::e_x_root = (int)tool->ev.rx;
  Fl::e_y_root = (int)tool->ev.ry;
}

/*
 Dispatch a single pen event to widget w.
 Commits event data the first time (lazy copy), then recomputes w's local
 coordinates before calling w->handle().
 */
static int pen_send(TabletTool *tool, Fl_Widget *w, int event,
                    State trigger, bool &copied) {
  if (!copied) {
    copy_state(tool);
    copied = true;
  }
  // Recompute widget-local coordinates for this specific widget each call,
  // because different subscribers may sit at different depths.
  e.x = tool->ev.x;
  e.y = tool->ev.y;
  offset_subwindow_event(w, e.x, e.y);
  Fl::e_x    = (int)e.x;
  Fl::e_y    = (int)e.y;
  e.trigger  = trigger;
  return w->handle(event);
}

/*
 Broadcast event+trigger to every subscriber.
 */
static int pen_send_all(TabletTool *tool, int event, State trigger) {
  bool copied = false;
  for (auto &it : subscriber_list_) {
    Fl_Widget *w = it.second->widget();
    if (w) pen_send(tool, w, event, trigger, copied);
  }
  return 1;
}


// ─────────────────────────────────────────────────────────────────────────────
// Coordinate conversion (Wayland-specific)
// ─────────────────────────────────────────────────────────────────────────────

/*
 Convert a Wayland surface coordinate pair (wl_fixed_t) to FLTK logical-pixel
 coordinates and store them in tool->ev.

 Mirrors event_coords_from_surface() from Fl_Wayland_Screen_Driver.cxx but
 writes into the tool's EventData rather than the global Fl::e_x/y so that
 the data can be inspected before committing.
 */
static void coords_from_surface(TabletTool *tool,
                                wl_fixed_t sx, wl_fixed_t sy) {
  if (!tool->focus_surface) return;

  Fl_Window *win = Fl_Wayland_Window_Driver::surface_to_window(tool->focus_surface);
  if (!win) return;

  // Accumulate sub-window offsets while walking to the top-level window.
  int delta_x = 0, delta_y = 0;
  Fl_Window *w = win;
  while (w->parent()) {
    delta_x += w->x();
    delta_y += w->y();
    w = w->window();
  }
  Fl_Window *top = w; // top-level window

  float f = Fl::screen_scale(top->screen_num());

  // Apply the same menu-offset correction used by pointer_motion.
  int menu_ofs = 0;
  int *poffset = Fl_Window_Driver::menu_offset_y(win);
  if (poffset) menu_ofs = *poffset;

  tool->ev.x  = wl_fixed_to_double(sx) / f + delta_x;
  tool->ev.y  = wl_fixed_to_double(sy) / f + delta_y - menu_ofs;
  tool->ev.rx = tool->ev.x + top->x();
  tool->ev.ry = tool->ev.y + top->y();

  // Keep focus_win pointing at the top-level so the dispatch code can use it
  // as an event window without further walking.
  tool->focus_win = top;
}


// ─────────────────────────────────────────────────────────────────────────────
// zwp_tablet_tool_v2 static property callbacks
// (fire once per tool at connection time, before proximity events)
// ─────────────────────────────────────────────────────────────────────────────

static void tool_cb_type(void *data, struct zwp_tablet_tool_v2 *,
                          uint32_t tool_type) {
  static_cast<TabletTool *>(data)->type =
    static_cast<enum zwp_tablet_tool_v2_type>(tool_type);
}

static void tool_cb_hardware_serial(void *data, struct zwp_tablet_tool_v2 *,
                                     uint32_t serial_hi, uint32_t serial_lo) {
  TabletTool *tool = static_cast<TabletTool *>(data);
  tool->hardware_serial =
    (static_cast<uint64_t>(serial_hi) << 32) | serial_lo;
  // Derive a positive int pen_id.  If the compositor reports no serial,
  // assign a monotonically increasing id so tools remain distinguishable.
  if (tool->hardware_serial)
    tool->pen_id = static_cast<int>(tool->hardware_serial & 0x7fffffff);
  else
    tool->pen_id = g_next_pen_id++;
}

static void tool_cb_hardware_id_wacom(void * /*data*/,
                                       struct zwp_tablet_tool_v2 *,
                                       uint32_t /*id_hi*/,
                                       uint32_t /*id_lo*/) {
  // Wacom-specific product ID; not currently needed.
}

static void tool_cb_capability(void *data, struct zwp_tablet_tool_v2 *,
                                uint32_t cap) {
  TabletTool *tool = static_cast<TabletTool *>(data);
  // Map Wayland capability flags to FLTK trait bits.
  switch (cap) {
    case ZWP_TABLET_TOOL_V2_CAPABILITY_PRESSURE:
      tool->capabilities |= Trait::PRESSURE;         break;
    case ZWP_TABLET_TOOL_V2_CAPABILITY_TILT:
      tool->capabilities |= Trait::TILT_X | Trait::TILT_Y; break;
    case ZWP_TABLET_TOOL_V2_CAPABILITY_ROTATION:
      tool->capabilities |= Trait::TWIST;             break;
    case ZWP_TABLET_TOOL_V2_CAPABILITY_SLIDER:
      tool->capabilities |= Trait::BARREL_PRESSURE;   break;
    case ZWP_TABLET_TOOL_V2_CAPABILITY_DISTANCE:
      // ev.proximity is filled; no FLTK Trait::PROXIMITY yet.
      break;
    case ZWP_TABLET_TOOL_V2_CAPABILITY_WHEEL:
      // Not yet exposed through the FLTK pen API.
      break;
  }
}

static void tool_cb_done(void *data, struct zwp_tablet_tool_v2 *) {
  TabletTool *tool = static_cast<TabletTool *>(data);
  // All capability and identity events have been delivered; lock in the traits.
  tool->capabilities |= Trait::DRIVER_AVAILABLE | Trait::PEN_ID;
  if (tool->type == ZWP_TABLET_TOOL_V2_TYPE_ERASER)
    tool->capabilities |= Trait::ERASER;
  // Stable pen_id is now available; write it into the event template.
  tool->ev.pen_id = tool->pen_id;
}

static void tool_cb_removed(void *data, struct zwp_tablet_tool_v2 *wl_tool) {
  TabletTool *tool = static_cast<TabletTool *>(data);
  if (g_current_tool == tool) g_current_tool = nullptr;
  wl_list_remove(&tool->link);
  zwp_tablet_tool_v2_destroy(wl_tool);
  delete tool;
}


// ─────────────────────────────────────────────────────────────────────────────
// zwp_tablet_tool_v2 per-frame event callbacks
// (accumulate data; dispatch happens only in tool_cb_frame)
// ─────────────────────────────────────────────────────────────────────────────

static void tool_cb_proximity_in(void *data, struct zwp_tablet_tool_v2 *,
                                  uint32_t /*serial*/,
                                  struct zwp_tablet_v2 * /*tablet*/,
                                  struct wl_surface *surface) {
  TabletTool *tool = static_cast<TabletTool *>(data);
  tool->focus_surface      = surface;
  tool->focus_win          =
    Fl_Wayland_Window_Driver::surface_to_window(surface);
  tool->in_proximity       = true;
  tool->frame_proximity_in = true;
  // Set hover state immediately so ev.state is valid even without a tip-down.
  State btn_bits = tool->ev.state &
    (State::BUTTON0|State::BUTTON1|State::BUTTON2|State::BUTTON3);
  tool->ev.state = (tool->type == ZWP_TABLET_TOOL_V2_TYPE_ERASER
    ? State::ERASER_HOVERS : State::TIP_HOVERS) | btn_bits;
}

static void tool_cb_proximity_out(void *data, struct zwp_tablet_tool_v2 *) {
  TabletTool *tool           = static_cast<TabletTool *>(data);
  tool->in_proximity         = false;
  tool->frame_proximity_out  = true;
}

static void tool_cb_down(void *data, struct zwp_tablet_tool_v2 *,
                          uint32_t /*serial*/) {
  TabletTool *tool = static_cast<TabletTool *>(data);
  // Tip contact; preserve any side-button bits already present.
  State btn_bits = tool->ev.state &
    (State::BUTTON0|State::BUTTON1|State::BUTTON2|State::BUTTON3);
  tool->ev.state = (tool->type == ZWP_TABLET_TOOL_V2_TYPE_ERASER
    ? State::ERASER_DOWN : State::TIP_DOWN) | btn_bits;
  tool->frame_down = true;
}

static void tool_cb_up(void *data, struct zwp_tablet_tool_v2 *) {
  TabletTool *tool = static_cast<TabletTool *>(data);
  State btn_bits = tool->ev.state &
    (State::BUTTON0|State::BUTTON1|State::BUTTON2|State::BUTTON3);
  tool->ev.state = (tool->type == ZWP_TABLET_TOOL_V2_TYPE_ERASER
    ? State::ERASER_HOVERS : State::TIP_HOVERS) | btn_bits;
  tool->frame_up = true;
}

static void tool_cb_motion(void *data, struct zwp_tablet_tool_v2 *,
                            wl_fixed_t x, wl_fixed_t y) {
  TabletTool *tool = static_cast<TabletTool *>(data);
  coords_from_surface(tool, x, y);
  tool->frame_motion = true;
}

static void tool_cb_pressure(void *data, struct zwp_tablet_tool_v2 *,
                              uint32_t pressure) {
  // Wayland: [0, 65535] → FLTK: [0.0, 1.0]
  static_cast<TabletTool *>(data)->ev.pressure = pressure / 65535.0;
}

static void tool_cb_distance(void *data, struct zwp_tablet_tool_v2 *,
                              uint32_t distance) {
  // Wayland: [0, 65535] → FLTK: [0.0, 1.0]
  static_cast<TabletTool *>(data)->ev.proximity = distance / 65535.0;
}

static void tool_cb_tilt(void *data, struct zwp_tablet_tool_v2 *,
                          wl_fixed_t tilt_x, wl_fixed_t tilt_y) {
  TabletTool *tool = static_cast<TabletTool *>(data);
  // Wayland reports degrees in [-90, 90]; normalise to [-1, 1] to match
  // NSEvent's tilt range.  tilt_x is negated to match the Cocoa driver's
  // convention (ev.tilt_x = -tilt.x).
  tool->ev.tilt_x = -wl_fixed_to_double(tilt_x) / 90.0;
  tool->ev.tilt_y = -wl_fixed_to_double(tilt_y) / 90.0;
}

static void tool_cb_rotation(void *data, struct zwp_tablet_tool_v2 *,
                              wl_fixed_t degrees) {
  // Wayland: clockwise rotation in [0, 360).
  // Cocoa returns the same unit (degrees), so store as-is.
  static_cast<TabletTool *>(data)->ev.twist = wl_fixed_to_double(degrees);
}

static void tool_cb_slider(void *data, struct zwp_tablet_tool_v2 *,
                            int32_t position) {
  // Wayland: [-65535, 65535] → FLTK: [-1.0, 1.0]
  // Maps to barrel_pressure, which in Cocoa is tangentialPressure.
  static_cast<TabletTool *>(data)->ev.barrel_pressure = position / 65535.0;
}

static void tool_cb_wheel(void * /*data*/, struct zwp_tablet_tool_v2 *,
                           wl_fixed_t /*degrees*/, int32_t /*clicks*/) {
  // Tablet wheel / lens events; not yet exposed via the FLTK pen API.
}

static void tool_cb_button(void *data, struct zwp_tablet_tool_v2 *,
                            uint32_t /*serial*/, uint32_t button,
                            uint32_t button_state) {
  TabletTool *tool = static_cast<TabletTool *>(data);
  bool pressed = (button_state == ZWP_TABLET_TOOL_V2_BUTTON_STATE_PRESSED);

  // Map physical button codes to State bits.
  State bit = (State)0;
  switch (button) {
    case BTN_STYLUS:  bit = State::BUTTON1; break; // upper barell button
    case BTN_STYLUS2: bit = State::BUTTON0; break; // lower barrel button, closer to the pen tip
    case BTN_STYLUS3: bit = State::BUTTON2; break;
    default: break;
  }
  if ((uint32_t)bit == 0) return;

  if (pressed) {
    tool->ev.state              |= bit;
    tool->frame_buttons_pressed |= bit;
  } else {
    int state = static_cast<int>(tool->ev.state) & ~(static_cast<int>(bit));
    tool->ev.state = static_cast<Fl::Pen::State>(state);
    tool->frame_buttons_released |= bit;
  }
}

/*
 tool_cb_frame — the main dispatch point.

 All per-frame data has been accumulated by the callbacks above.  This
 function mirrors the body of fl_cocoa_tablet_handler() but reads from
 tool->ev rather than an NSEvent.

 Ordering within one frame:
   1. proximity-out  → OUT_OF_RANGE broadcast, below_pen cleanup, early return
   2. proximity-in   → DETECTED / CHANGED / IN_RANGE broadcast
   3. modal/grab guard
   4. below_pen ENTER/LEAVE tracking
   5. tip down       → TOUCH
   6. tip up         → LIFT
   7. barrel buttons → BUTTON_PUSH / BUTTON_RELEASE
   8. motion         → DRAW (if pushed) or HOVER
   9. reset per-frame flags
 */
static void tool_cb_frame(void *data, struct zwp_tablet_tool_v2 *,
                           uint32_t /*time*/) {
  TabletTool *tool = static_cast<TabletTool *>(data);

  // ── 1. Proximity-out ──────────────────────────────────────────────────────
  if (tool->frame_proximity_out) {
    pen_send_all(tool, Fl::Pen::OUT_OF_RANGE, (State)0);
    // Clean up below_pen_ if this tool owns it.
    if (below_pen_ && below_pen_->widget()) {
      bool copied = false;
      pen_send(tool, below_pen_->widget(),
               Fl::Pen::LEAVE, (State)0, copied);
    }
    below_pen_ = nullptr;
    if (pushed_) {
      Fl::pushed(nullptr);
      pushed_ = nullptr;
    }
    tool->focus_surface = nullptr;
    tool->focus_win     = nullptr;
    tablet_tool_reset_frame(tool);
    tool->prev_state = tool->ev.state;
    return;
  }

  // ── 2. Proximity-in notification ─────────────────────────────────────────
  if (tool->frame_proximity_in) {
    tool->ev.pen_id = tool->pen_id;
    if (tool->is_new) {
      tool->is_new = false;
      pen_send_all(tool, Fl::Pen::DETECTED, (State)0);
    } else if (g_current_tool && g_current_tool != tool) {
      pen_send_all(tool, Fl::Pen::CHANGED, (State)0);
    } else {
      pen_send_all(tool, Fl::Pen::IN_RANGE, (State)0);
    }
    g_current_tool = tool;
  }

  // No subscribers or no focus window → nothing more to do.
  if (!tool->focus_win || subscriber_list_.empty()) {
    tablet_tool_reset_frame(tool);
    tool->prev_state = tool->ev.state;
    return;
  }

  Fl_Window *eventWindow = nullptr;
  if (Fl::grab()) {
    eventWindow = Fl::grab();
  } else if (Fl::modal()) {
    eventWindow = Fl::modal();
  } else {
    eventWindow = tool->focus_win;
  }

  bool is_menu_window = eventWindow->menu_window();

  if (!is_menu_window) {
      // ── 3. Modal / grab guards ──────────────────────────────────────────
      if (Fl::grab() && Fl::grab() != eventWindow) {
          tablet_tool_reset_frame(tool);
          tool->prev_state = tool->ev.state;
          return;
      }
      if (Fl::modal() && Fl::modal() != eventWindow) {
          tablet_tool_reset_frame(tool);
          tool->prev_state = tool->ev.state;
          return;
      }
  }

  fl_xmousewin = eventWindow;

  bool event_data_copied = false;
  Fl_Widget *receiver    = nullptr;
  bool       is_pushed   = false;

  // Extract the mouse fallback block into a lambda to avoid duplication.
  auto mouse_fallback = [&](bool frame_down, bool frame_up, bool frame_motion) {
    Fl::e_x      = (int)tool->ev.x;
    Fl::e_y      = (int)tool->ev.y;
    Fl::e_x_root = (int)tool->ev.rx;
    Fl::e_y_root = (int)tool->ev.ry;
    if (frame_down) {
      Fl::e_state  |= FL_BUTTON1;
      Fl::e_keysym  = FL_Button + 1;
      Fl::e_is_click = 1;
      Fl::handle(FL_PUSH, eventWindow);
    } else if (frame_up) {
      Fl::e_state  &= ~FL_BUTTON1;
      Fl::e_keysym  = FL_Button + 1;
      Fl::handle(FL_RELEASE, eventWindow);
    } else if (frame_motion) {
      Fl::handle(Fl::pushed() ? FL_DRAG : FL_MOVE, eventWindow);
    }
  };

  // ── 4. Receiver selection & below_pen ENTER/LEAVE ────────────────────────
  if (pushed_ && pushed_->widget() && Fl::pushed() == pushed_->widget()) {
    // An earlier tip-down fixed this tool's receiver until the tip lifts.
    receiver  = pushed_->widget();
    is_pushed = true;
  } else {
    auto bpen_widget = below_pen_ ? below_pen_->widget() : nullptr;
    auto bpen_old    = (Fl::belowmouse() == bpen_widget) ? bpen_widget : nullptr;
    auto bpen_now    = find_below_pen(eventWindow, tool->ev.x, tool->ev.y);

    if (bpen_now != bpen_old) {
      if (bpen_old)
        pen_send(tool, bpen_old, Fl::Pen::LEAVE, (State)0,
                 event_data_copied);
      below_pen_ = nullptr;
      if (bpen_now) {
        State hover_state = (tool->type == ZWP_TABLET_TOOL_V2_TYPE_ERASER)
          ? State::ERASER_HOVERS : State::TIP_HOVERS;
        if (pen_send(tool, bpen_now, Fl::Pen::ENTER, hover_state,
                     event_data_copied)) {
          below_pen_ = subscriber_list_[bpen_now];
          Fl::belowmouse(bpen_now);
        }
      }
    }
    receiver = below_pen_ ? below_pen_->widget() : nullptr;
  }

  if (!receiver) {
    mouse_fallback(tool->frame_down, tool->frame_up, tool->frame_motion);
    tablet_tool_reset_frame(tool);
    tool->prev_state = tool->ev.state;
    return;
  }

  bool pen_handled = false;

  // ── 5. Tip down → TOUCH ──────────────────────────────────────────────────
  if (tool->frame_down) {
    if (!is_pushed) {
      pushed_ = subscriber_list_[receiver];
      Fl::pushed(receiver);
    }
    Fl::e_is_click = 1;
    Fl::Private::e_x_down = (int)tool->ev.x;
    Fl::Private::e_y_down = (int)tool->ev.y;
    Fl::e_clicks = 0;
    pen_handled |= pen_send(tool, receiver, Fl::Pen::TOUCH,
                            tool->ev.state & (State::TIP_DOWN|State::ERASER_DOWN),
                            event_data_copied);
    if (!pen_handled)
      mouse_fallback(true, false, false);
  }

  // ── 6. Tip up → LIFT ─────────────────────────────────────────────────────
  if (tool->frame_up) {
    if ((tool->ev.state & State::ANY_DOWN) == (State)0) {
      Fl::pushed(nullptr);
      pushed_ = nullptr;
    }
    State trigger = (tool->type == ZWP_TABLET_TOOL_V2_TYPE_ERASER)
      ? State::ERASER_HOVERS : State::TIP_HOVERS;
    bool handled = pen_send(tool, receiver, Fl::Pen::LIFT, trigger,
                            event_data_copied);
    pen_handled |= handled;
    if (!handled)
      mouse_fallback(false, true, false);
  }

  // ── 7. Barrel button events ───────────────────────────────────────────────
  for (State bit : kButtonBits) {
    if ((uint32_t)(tool->frame_buttons_pressed) & (uint32_t)bit)
      pen_handled |= pen_send(tool, receiver, Fl::Pen::BUTTON_PUSH, bit,
                              event_data_copied);
    if ((uint32_t)(tool->frame_buttons_released) & (uint32_t)bit)
      pen_handled |= pen_send(tool, receiver, Fl::Pen::BUTTON_RELEASE, bit,
                              event_data_copied);
    // No mouse fallback for barrel buttons — there's no standard mouse
    // equivalent, and the widget already ignored the pen event.
  }

  // ── 8. Motion → DRAW or HOVER ────────────────────────────────────────────
  if (tool->frame_motion) {
    if (Fl::e_is_click &&
        (std::fabs(tool->ev.x - Fl::Private::e_x_down) > 5.0 ||
         std::fabs(tool->ev.y - Fl::Private::e_y_down) > 5.0))
      Fl::e_is_click = 0;

    bool handled = pen_send(tool, receiver,
                            is_pushed ? Fl::Pen::DRAW : Fl::Pen::HOVER,
                            (State)0, event_data_copied);
    pen_handled |= handled;
    if (!handled)
      mouse_fallback(false, false, true);
  }

  // ── 9. Reset per-frame flags ──────────────────────────────────────────────
  tablet_tool_reset_frame(tool);
  tool->prev_state = tool->ev.state;

  // -- 10. Force standard mouse motion for menus (critical for highlighting)
  if (is_menu_window && tool->frame_motion) {
      mouse_fallback(false, false, true);   // sends FL_MOVE or FL_DRAG
  }
}


// ─────────────────────────────────────────────────────────────────────────────
// zwp_tablet_tool_v2 listener
// ─────────────────────────────────────────────────────────────────────────────

static const struct zwp_tablet_tool_v2_listener tablet_tool_listener = {
  .type               = tool_cb_type,
  .hardware_serial    = tool_cb_hardware_serial,
  .hardware_id_wacom  = tool_cb_hardware_id_wacom,
  .capability         = tool_cb_capability,
  .done               = tool_cb_done,
  .removed            = tool_cb_removed,
  .proximity_in       = tool_cb_proximity_in,
  .proximity_out      = tool_cb_proximity_out,
  .down               = tool_cb_down,
  .up                 = tool_cb_up,
  .motion             = tool_cb_motion,
  .pressure           = tool_cb_pressure,
  .distance           = tool_cb_distance,
  .tilt               = tool_cb_tilt,
  .rotation           = tool_cb_rotation,
  .slider             = tool_cb_slider,
  .wheel              = tool_cb_wheel,
  .button             = tool_cb_button,
  .frame              = tool_cb_frame,
};


// ─────────────────────────────────────────────────────────────────────────────
// zwp_tablet_v2 callbacks (physical tablet device object)
// ─────────────────────────────────────────────────────────────────────────────

// We do not need per-tablet state at the moment; the tablet object is created
// and destroyed but its properties (name, VID/PID, device path) are unused.

static void tablet_cb_name(void * /*data*/, struct zwp_tablet_v2 *,
                           const char * /*name*/) {}
static void tablet_cb_id(void * /*data*/, struct zwp_tablet_v2 *,
                          uint32_t /*vid*/, uint32_t /*pid*/) {}
static void tablet_cb_path(void * /*data*/, struct zwp_tablet_v2 *,
                            const char * /*path*/) {}
static void tablet_cb_done(void * /*data*/, struct zwp_tablet_v2 *) {}
static void tablet_cb_removed(void *data, struct zwp_tablet_v2 *wl_tablet) {
  // data was set to the zwp_tablet_v2* itself for easy cleanup
  zwp_tablet_v2_destroy(wl_tablet);
}

static const struct zwp_tablet_v2_listener tablet_listener = {
  .name    = tablet_cb_name,
  .id      = tablet_cb_id,
  .path    = tablet_cb_path,
  .done    = tablet_cb_done,
  .removed = tablet_cb_removed,
};


// ─────────────────────────────────────────────────────────────────────────────
// zwp_tablet_seat_v2 callbacks
// ─────────────────────────────────────────────────────────────────────────────

static void tablet_seat_cb_tablet_added(void * /*data*/,
                                         struct zwp_tablet_seat_v2 *,
                                         struct zwp_tablet_v2 *wl_tablet) {
  // Use the tablet object pointer itself as callback data; we only need it
  // for cleanup in tablet_cb_removed.
  zwp_tablet_v2_add_listener(wl_tablet, &tablet_listener, wl_tablet);
}

static void tablet_seat_cb_tool_added(void * /*data*/,
                                       struct zwp_tablet_seat_v2 *,
                                       struct zwp_tablet_tool_v2 *wl_tool) {
  TabletTool *tool = new TabletTool();
  tool->wl_tool       = wl_tool;
  tool->type          = ZWP_TABLET_TOOL_V2_TYPE_PEN;
  tool->hardware_serial = 0;
  tool->pen_id        = 0;  // set in tool_cb_hardware_serial or tool_cb_done
  tool->capabilities  = Trait::NONE;
  tool->is_new        = true;
  tool->in_proximity  = false;
  tool->focus_win     = nullptr;
  tool->focus_surface = nullptr;
  tool->ev            = EventData();  // value-initialised defaults from struct
  tool->prev_state    = (State)0;
  tablet_tool_reset_frame(tool);
  wl_list_insert(&g_tool_list, &tool->link);
  zwp_tablet_tool_v2_add_listener(wl_tool, &tablet_tool_listener, tool);
}

static void tablet_seat_cb_pad_added(void * /*data*/,
                                      struct zwp_tablet_seat_v2 *,
                                      struct zwp_tablet_pad_v2 *wl_pad) {
  // Tablet pad (touch ring, strip, buttons) — not yet mapped to FLTK pen API.
  // Destroy immediately to avoid resource leak.
  zwp_tablet_pad_v2_destroy(wl_pad);
}

static const struct zwp_tablet_seat_v2_listener tablet_seat_listener = {
  .tablet_added = tablet_seat_cb_tablet_added,
  .tool_added   = tablet_seat_cb_tool_added,
  .pad_added    = tablet_seat_cb_pad_added,
};


// ─────────────────────────────────────────────────────────────────────────────
// Public API called from Fl_Wayland_Screen_Driver.cxx
// ─────────────────────────────────────────────────────────────────────────────

/*
 Try to create the tablet seat once we have both the manager and the wl_seat.
 Called from both fl_wayland_tablet_set_manager() and
 fl_wayland_tablet_init_seat() to handle arbitrary ordering.
 */
static void tablet_try_init() {
  if (!g_tablet_manager || !g_wl_seat || g_tablet_seat) return;
  wl_list_init(&g_tool_list);
  g_tablet_seat = zwp_tablet_manager_v2_get_tablet_seat(
    g_tablet_manager, g_wl_seat);
  if (g_tablet_seat)
    zwp_tablet_seat_v2_add_listener(g_tablet_seat,
                                    &tablet_seat_listener, nullptr);
}

void fl_wayland_tablet_set_manager(struct zwp_tablet_manager_v2 *manager) {
  g_tablet_manager = manager;
  tablet_try_init(); // no-op if seat not yet available; seat-first path in subscribe()
}

void fl_wayland_tablet_init_seat(struct wl_seat *wl_seat) {
  if (!wl_seat || g_wl_seat) return;  // guard against null and double-init
  g_wl_seat = wl_seat;
  tablet_try_init();
}

void fl_wayland_tablet_cleanup() {
  if (g_tablet_seat) {
    // Destroy all tool objects first.
    TabletTool *t, *tmp;
    wl_list_for_each_safe(t, tmp, &g_tool_list, link) {
      wl_list_remove(&t->link);
      zwp_tablet_tool_v2_destroy(t->wl_tool);
      delete t;
    }
    zwp_tablet_seat_v2_destroy(g_tablet_seat);
    g_tablet_seat = nullptr;
  }
  if (g_tablet_manager) {
    zwp_tablet_manager_v2_destroy(g_tablet_manager);
    g_tablet_manager = nullptr;
  }
  g_wl_seat      = nullptr;
  g_current_tool = nullptr;
  below_pen_     = nullptr;
  pushed_        = nullptr;
}

/*
 Called from Fl_Wayland_Window_Driver::hide() just before a wl_surface is
 destroyed. Any tablet tool currently focused on that surface gets its
 cached surface/window pointers cleared, and any below_pen_/pushed_
 widget belonging to the window being closed is released, so a frame
 event already queued by the compositor can't dereference freed memory.
 */
void fl_wayland_tablet_surface_destroyed(struct wl_surface *surface) {
  if (!g_tablet_seat || !surface) return;

  TabletTool *t;
  wl_list_for_each(t, &g_tool_list, link) {
    if (t->focus_surface != surface) continue;

    Fl_Window *closing = t->focus_win;

    if (below_pen_ && below_pen_->widget() &&
        below_pen_->widget()->top_window() == closing) {
      bool copied = false;
      pen_send(t, below_pen_->widget(), Fl::Pen::LEAVE, (State)0, copied);
      below_pen_ = nullptr;
      Fl::belowmouse(nullptr);
    }
    if (pushed_ && pushed_->widget() &&
        pushed_->widget()->top_window() == closing) {
      Fl::pushed(nullptr);
      pushed_ = nullptr;
    }

    t->focus_surface = nullptr;
    t->focus_win     = nullptr;
    t->in_proximity  = false;
    tablet_tool_reset_frame(t);
  }
}
