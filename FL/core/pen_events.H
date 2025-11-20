//
// Pen event header file for the Fast Light Tool Kit (FLTK).
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

/**
 \file FL/core/pen_events.H
 \brief Pen event handling variables and functions.
*/

#ifndef Fl_core_pen_events_H
#define Fl_core_pen_events_H

#include <FL/fl_config.h>             // build configuration
#include <FL/Fl_Export.H>             // for FL_EXPORT
#include <FL/core/function_types.H>   // widget callbacks and services

#include <cstdint>

class Fl_Widget;

namespace Fl {

/** FLTK Pen/Stylus/Tablet input driver API. */
namespace Pen {

/**
 \defgroup fl_pen_events Pen and tablet event handling
 \ingroup fl_events
 \brief This chapter documents the Fl::Pen namespace API, declared in <FL/core/pen_events.H>

 The FL::Pen namespace contains everything needed to work with a pen type input
 device, either in connection with an external tablet, or as a stylus for
 drawing directly onto a screen.

 To receive pen input, call Fl::Pen::subscribe() for one or more widgets. The
 widget will receive a Fl::Pen::ENTER event when the stylus enters the widget
 area. By returning 1 to Fl::Pen::ENTER, all further pen events are sent to
 this widget, and no mouse events are generated until Fl::Pen::LEAVE.

 Returning 0 Fl::Pen::ENTER tells FLTK to suppress further pen events until
 Fl::Pen::LEAVE, and convert them into mouse events instead.

 Pen events also set Fl::event_x(), Fl::event_y(), Fl::event_x_root(),
 Fl::event_y_root(), Fl::event_is_click(), and Fl::event_clicks().

 @{
 */

/**
 \brief Bitfield of traits.
 This is used in Fl::Pen::driver_traits() and Fl::Pen::pen_traits().
 */
enum class Trait : uint32_t {
  /// No bits set
  NONE              = 0x0000,
  /// Set if FLTK supports tablets and pens on this platform
  DRIVER_AVAILABLE  = 0x0001,
  /// Set after the system detected a pen, stylus, or tablet. This bit may not be
  /// set until a pen is brought into proximity of the tablet.
  DETECTED          = 0x0002,
  /// If set, this is a digitizer for a display; if clear, this is a standalone tablet
  DISPLAY           = 0x0004,
  /// Driver provides different device IDs for different pens
  PEN_ID            = 0x0008,
  /// Pen may have an eraser tip
  ERASER            = 0x0010,
  /// Pen returns a pressure value
  PRESSURE          = 0x0020,
  /// Pen returns a barrel pressure value (tangential pressure)
  BARREL_PRESSURE   = 0x0040,
  /// Pen returns tilt in X direction
  TILT_X            = 0x0080,
  /// Pen returns tilt in Y direction
  TILT_Y            = 0x0100,
  /// Pen returns a twist value
  TWIST             = 0x0200,
  /// Pen returns a proximity value
  PROXIMITY         = 0x0400,
};

/**
 \brief Bitwise OR operator for Trait enum.
 \param lhs Left-hand side trait flags
 \param rhs Right-hand side trait flags
 \return Combined trait flags
 */
inline constexpr Trait operator|(Trait lhs, Trait rhs) {
  return static_cast<Trait>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

/**
 \brief Bitwise AND operator for Trait enum.
 \param lhs Left-hand side trait flags
 \param rhs Right-hand side trait flags
 \return Intersection of trait flags
 */
inline constexpr Trait operator&(Trait lhs, Trait rhs) {
  return static_cast<Trait>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

/**
 \brief Bitwise OR assignment operator for Trait enum.
 \param lhs Left-hand side trait flags (modified in place)
 \param rhs Right-hand side trait flags
 \return Reference to modified lhs
 */
inline Trait& operator|=(Trait& lhs, Trait rhs) {
  lhs = lhs | rhs;
  return lhs;
}


/**
 \brief Bitfield of pen state flags.
 \see event_state(), event_trigger()
 */
enum class State : uint32_t {
  /// No button pressed
  NONE              = 0x0000,
  /// The tip hovers over the surface but does not touch it
  TIP_HOVERS        = 0x0001,
  /// The tip touches the surface
  TIP_DOWN          = 0x0002,
  /// The eraser hovers over the surface but does not touch it
  ERASER_HOVERS     = 0x0004,
  /// The eraser touches the surface
  ERASER_DOWN       = 0x0008,
  /// Barrel button 0, usually the lower button on a pen, is pressed
  BUTTON0           = 0x0100,
  /// Barrel button 1, usually the upper button on a pen, is pressed
  BUTTON1           = 0x0200,
  /// Barrel button 2 is pressed
  BUTTON2           = 0x0400,
  /// Barrel button 3 is pressed
  BUTTON3           = 0x0800,
  /// Mask for all buttons, tip, and eraser down
  ANY_DOWN          = BUTTON0 | BUTTON1 | BUTTON2 | BUTTON3 | TIP_DOWN | ERASER_DOWN,
};

/**
 \brief Bitwise OR operator for State enum.
 \param lhs Left-hand side state flags
 \param rhs Right-hand side state flags
 \return Combined state flags
 */
inline constexpr State operator|(State lhs, State rhs) {
  return static_cast<State>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

/**
 \brief Bitwise AND operator for State enum.
 \param lhs Left-hand side state flags
 \param rhs Right-hand side state flags
 \return Intersection of state flags
 */
inline constexpr State operator&(State lhs, State rhs) {
  return static_cast<State>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

/**
 \brief Bitwise OR assignment operator for State enum.
 \param lhs Left-hand side state flags (modified in place)
 \param rhs Right-hand side state flags
 \return Reference to modified lhs
 */
inline State& operator|=(State& lhs, State rhs) {
  lhs = lhs | rhs;
  return lhs;
}


/**
 \brief List of pen events.
 These events extend the standard Fl_Event enumeration.
 \see enum Fl_Event
 */
enum Event {
  /**
   Pen entered the proximity of the tablet with a new pen.
   */
  DETECTED = 0x1000,

  /**
   Pen entered the proximity of the tablet with a known, but changed pen.
   User changed to a different pen (event_id() > 0) or the pen or tablet
   was disconnected (event_id() == -1). Pen IDs, if supported, are assigned by
   the tablet manufacturer.
   */
  CHANGED,

  /**
   Pen entered the proximity of the tablet with a known pen.
   */
  IN_RANGE,

  /**
   Pen left the proximity of the tablet.
   */
  OUT_OF_RANGE,

  /**
   Pen entered the widget area, either by moving in x/y, or by
   a proximity change (pen gets closer to the surface).
   event_trigger() returns 0, TIP_HOVERS, or ERASER_HOVERS.
   */
  ENTER,

  /**
   If no button is pressed, indicates that the pen left the widget area.
   While any pen button is held down, or the pen touches the surface,
   Fl::pushed() is set, and the pushed widgets receives DRAG events, even
   if the pen leaves the widget area. If all buttons are released outside the
   widget area, a LEAVE event is sent as well as LIFT or BUTTON_RELEASE.
   */
  LEAVE,

  /**
   Pen went from hovering to touching the surface.
   event_trigger() returns TIP_DOWN or ERASER_DOWN.
   */
  TOUCH,

  /**
   Pen went from touching to hovering over the surface.
   event_trigger() returns TIP_HOVERS or ERASER_HOVERS.
  */
  LIFT,

  /** Pen moved without touching the surface and no button is pressed. */
  HOVER,

  /** Pen moved while touching the surface, or any button is pressed. */
  DRAW,

  /**
   A pen button was pushed.
   event_trigger() returns BUTTON0, BUTTON1, BUTTON2, or BUTTON3.
   */
  BUTTON_PUSH,

  /**
   A pen button was released.
   event_trigger() returns BUTTON0, BUTTON1, BUTTON2, or BUTTON3.
   */
  BUTTON_RELEASE

};

/**
 \brief Query the traits supported by the pen/tablet driver.

 This function returns a bitfield of traits that are supported by the FLTK driver
 for this platform. If a trait is not supported, the corresponding event value
 will not return a useful value. Note that even if the FLTK driver support a
 trait, the underlying pen device or driver may not. Fl::Pen will return a
 known default for those event values.

 The bitfield returned is static.

 \return a bitfield of supported traits
 \see pen_traits()
 */
FL_EXPORT extern Trait driver_traits();

/**
 \brief Return true if the corresponding bit is set in the driver traits.
 \param[in] bits check for one or more trait bits
 \return true if any bit is set
 */
inline bool driver_traits(Trait bits) {
  return ((driver_traits() & bits) != Trait::NONE);
}

/**
 \brief Query traits of the current pen or stylus.
 The value returned by this function may change when pens change or when more
 information becomes known about the currently used pen.
 \param[in] pen_id a know pen ID as returned from event_pen_id(),
    or 0 for the current pen
 \return a bitfield of supported traits
 */
FL_EXPORT extern Trait pen_traits(int pen_id = 0);

/**
 \brief Return true if the corresponding bit is set in the pen traits.
 \param[in] bits check for one or more trait bits
 \param[in] pen_id a know pen ID as returned from event_pen_id(),
  or 0 for the current pen
 \return true if any bit is set
 */
inline bool pen_traits(Trait bits, int pen_id = 0) {
  return ((pen_traits() & bits) != Trait::NONE);
}

/**
 \brief Receives a Pen::ENTER event when the pen enters this widget.
 Multiple widgets may subscribe to pen events. Additional subscription
 requests for the same widget are ignored.
 \param widget The widget subscribing to pen events.
 */
FL_EXPORT extern void subscribe(Fl_Widget* widget);

/**
 \brief Stop receiving Pen::ENTER for this widget.
 Deleting a widget will automatically unsubscribe it.
 \param widget Widget to unsubscribe from pen events
 */
FL_EXPORT extern void unsubscribe(Fl_Widget* widget);

/**
 Clear the "pushed" state and forward pen events as mouse events.
 Call this if another window is popped up during pen event handling, so
 mouse event handling can resume normal.
 */
FL_EXPORT extern void release();

/// \name Query values during event handling
/// @{

/**
 \brief Returns the pen x and y position inside the handling widget as doubles.
 These functions provide high-precision pen coordinates relative to the widget
 that received the pen event. For integer coordinates, use Fl::event_x() and
 Fl::event_y() instead.
 \return Pen position as floating-point coordinate, defaults to 0.0
 \see Fl::event_x(), Fl::event_y()
 */
FL_EXPORT extern double event_x();
/** \brief Returns pen Y coordinate in widget space, see event_x(). */
FL_EXPORT extern double event_y();

/**
 \brief Returns the pen x and y position in global coordinates as doubles.
 For integer coordinates, use Fl::event_x_root() and Fl::event_y_root().
 \return Pen position as floating-point coordinate in screen space, defaults to 0.0
 \see Fl::event_x_root(), Fl::event_y_root()
 */
FL_EXPORT extern double event_x_root();
/** \brief Returns pen Y coordinate in screen space, see event_x_root(). */
FL_EXPORT extern double event_y_root();

/**
 \brief Returns the ID of the pen used in the last event.
 \return Unique pen identifier, or -1 if pen was removed, defaults to 0
 \see Trait::PEN_ID
 */
FL_EXPORT extern int event_pen_id();

/**
 \brief Returns the pressure between the tip or eraser and the surface.
 \return pressure value from 0.0 (no pressure) to 1.0 (maximum pressure),
      defaults to 1.0.
 \see Trait::PRESSURE
 */
FL_EXPORT extern double event_pressure();

/**
 \brief Returns barrel pressure or tangential pressure.
 \return Pressure value from -1.0 to 1.0 , defaults to 0.0 .
 \see Trait::BARREL_PRESSURE
 */
FL_EXPORT extern double event_barrel_pressure();

/**
 \brief Returns the tilt of the pen in the x and y directions between -1 and 1.

 X-axis tilt returns -1.0 when the pen tilts all the way to the left, 0.0 when
 it is perfectly vertical, and 1.0 all the way to the right. Most pens seem to
 return a maximum range of -0.7 to 0.7.

 Y-axis tilt returns -1.0 when the pen tilts away from the user, and 1.0 when
 it tilts toward the user.

 \return Tilt value from -1.0 to 1.0, defaults to 0.0
 \see Trait::TILT_X, Trait::TILT_Y
 */
FL_EXPORT extern double event_tilt_x();
/** \brief Returns pen Y-axis tilt, see event_tilt_x() */
FL_EXPORT extern double event_tilt_y();

/**
 \brief Returns the pen's axial rotation in degrees.
 The twist angle is reported in degrees, with:
  - 0° : pen’s “top” (the button side) facing upward
  - +180° : twisted halfway clockwise, looking at the end of the pen toward the tip
  - –180° → twisted half way counter clockwise
 So the full range is usually –180° to +180°.
 \return Twist angle in degrees, defaults to 0.0.
 \see Trait::TWIST
 */
FL_EXPORT extern double event_twist();

/**
 \brief Returns the proximity of the pen to the surface between 0 and 1.
 A proximity of 0 is closest to the surface, 1 is farthest away.
 \return Proximity value from 0.0 (touching) to 1.0 (far away), defaults to 0.0 .
 \see Trait::PROXIMITY
 */
FL_EXPORT extern double event_proximity();

/**
 \brief Returns the state of the various buttons and tips.
 \return Current state flags (combination of State values)
 */
FL_EXPORT extern State event_state();

/**
 \brief Return true if any of the bits are set in the event state.
 \param[in] bits check for one or more event state bits
 \return true if any bit is set
 */
inline bool event_state(State bits) {
  return ((event_state() & bits) != State::NONE);
}

/**
 \brief Returns the state change that triggered the event.
 \return a state with one bit set for the action that triggered this event
 */
FL_EXPORT extern State event_trigger();

/** @} */ // group fl_pen_events


} // namespace Pen

} // namespace Fl


/*
 Resources:

 Windows:
 1. Legacy WinTab API (Win2k), Wintab32.dll, wintab.h
    https://developer.wacom.com/en-us/developer-dashboard/downloads
 2. Windows Ink API (Modern, Win10), Windows.UI.Input.Inking (WinRT API), InkCanvas(), etc.
    https://learn.microsoft.com/windows/uwp/design/input/windows-ink
 3. Pointer Input / WM_POINTER API (Win8), WM_POINTERUPDATE, GetPointerPenInfo
    https://learn.microsoft.com/windows/win32/inputmsg/wm-pointerupdate
      return WTInfo(0, 0, NULL) > 0; // Wintab check

 Linux:
 1. Low-level: evdev, /dev/input/event*, libevdev,
    https://www.kernel.org/doc/html/latest/input/event-codes.html
 2. Mid-level: XInput2 (for X11), XI_Motion, XI_ButtonPress
    https://www.x.org/releases/current/doc/inputproto/XI2proto.txt
    https://www.freedesktop.org/wiki/Software/libevdev/
 3. Mid-level: Wayland tablet protocol, tablet-v2 protocol,
    zwp_tablet_tool_v2_listener, zwp_tablet_v2, zwp_tablet_seat_v2
    https://wayland.app/protocols/tablet-v2

 SDL3:
    https://github.com/libsdl-org/SDL/blob/main/include/SDL3/SDL_pen.h
    https://wiki.libsdl.org/SDL3/CategoryPen
 */


#endif // !Fl_core_pen_events_H
