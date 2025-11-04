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

namespace Pen {

/**
 \defgroup fl_pen_events Pen and tablet event handling
 \ingroup fl_events
 \brief The Fl::Pen namespace API, declared in <FL/core/pen_events.H>

 The FL::Pen namespace contains everything needed to work with a pen type input
 device, either in connection with an external tablet, or as a stylus for
 drawing directly onto a screen.

 To receive pen input, call Fl::Pen::subscribe() for one or more widgets. The
 widget will receive a Fl::Pen::ENTER event when the stylus enters the widget.
 By returning 1 to pen events, the user can ask for more detailed events.

 Returning 0 form the event handler indicates that the event was not handled.
 FLTK may convert the tablet event into a mouse event and resend it to
 the relevant widget.

 @{
 */

/**
 \brief Bitfield of available pen features.
 If a bit is clear, the corresponding event value will not be set.
 */
enum Features : uint32_t {
  /// Return this if this implementation of FLTK does not support pens and tablets.
  NONE             = 0x0000,
  /// Set if FLTK supports tablets and pens on this platform
  SUPPORTED        = 0x0001,
  /// Set if the system found a pen and/or tablet device
  AVAILABLE        = 0x0002,
  /// If set, this is a digitizer for a display; if clear, this is a standalone tablet
  DISPLAY          = 0x0004,
  /// Every pen has its own ID, so users can distinguish multiple pens
  DEVICE_ID        = 0x0008,
  /// Pen may have an eraser tip
  ERASER_TIP       = 0x0010,
  /// Pen returns a pressure value
  PRESSURE         = 0x0020,
  /// Pen returns a tangential pressure value
  TANGENTIAL_PRESSURE = 0x0040,
  /// Pen returns tilt in X direction
  TILT_X           = 0x0080,
  /// Pen returns tilt in Y direction
  TILT_Y           = 0x0100,
  /// Pen returns a twist value
  TWIST            = 0x0200,
  /// Pen returns a proximity value
  PROXIMITY        = 0x0400,
};


/**
 \brief Bitfield of pen state flags.
 \see event_state(), event_trigger()
 */
enum State : uint32_t {
  /// Barrel button 0 is pressed
  BUTTON_0 = 0x0001,
  /// Barrel button 1 is pressed
  BUTTON_1 = 0x0002,
  /// Barrel button 2 is pressed
  BUTTON_2 = 0x0004,
  /// Barrel button 3 is pressed
  BUTTON_3 = 0x0008,
  /// The tip hovers over the surface but does not touch it
  TIP_HOVERS = 0x0010,
  /// The tip touches the surface
  TIP_DOWN = 0x0020,
  /// The tip was pressed down enough to be seen as a mouse event
  TIP_PUSHED = 0x0040,
  /// The eraser hovers over the surface but does not touch it
  ERASER_HOVERS = 0x0080,
  /// The eraser touches the surface
  ERASER_DOWN = 0x0100,
  /// The eraser was pressed down enough to be seen as a mouse event
  ERASER_PUSHED = 0x0200,
};

/**
 \brief List of pen events.
 These events extend the standard Fl_Event enumeration.
 */
typedef enum {
  /**
   User changed to a different pen (event_id() > 0) or the pen or tablet
   was disconnected (event_id() == -1). Pen IDs, if supported, are assigned by
   the tablet manufacturer.

   Return 1 to receive ENTER and LEAVE events.
   */
  ID_CHANGED,

  /**
   Pen entered the widget area, either by moving in x/y, or by
   a proximity change (pen gets closer to the surface).
   event_trigger() returns 0, TIP_HOVERS, or ERASER_HOVERS.
   Return 1 to receive MOVE events.
   */
  ENTER,

  /** Pen left the widget area. */
  LEAVE,

  /**
   Pen went from hovering to touching the surface.
   event_trigger() returns TIP_DOWN  or ERASER_DOWN.
   Return 1 to receive DRAW and DRAG events.
   */
  TOUCH,

  /**
   Pen went from touching to hovering over the surface.
   event_trigger() returns TIP_HOVERS or ERASER_HOVERS.
  */
  LIFT,

  /**
   Pen was pushed hard enough to cause an FL_PUSH or FL_RELEASE mouse event.
   Return 1 to suppress the mouse event.
   */
  PUSH,
  /** Pen pressure released. See PUSH */
  RELEASE,

  /** Pen moved without touching the surface. */
  MOVE,

  /** Pen moved while lightly touching the surface. */
  DRAW,

  /** Pen moved while pressed down. */
  DRAG,

  /**
   A pen button was pushed.
   event_trigger() returns BUTTON_0, BUTTON_1, BUTTON_2, or BUTTON_3.
   */
  BUTTON_PUSH,

  /**
   A pen button was released.
   event_trigger() returns BUTTON_0, BUTTON_1, BUTTON_2, or BUTTON_3.
   */
  BUTTON_RELEASE

} Event;

/**
 \brief Query features supported by the pen/tablet device.
 \return a bitfield of supported features
 */
FL_EXPORT extern Features features();

/**
 \brief Receive pen events when the pen is inside this widget, or all events.

 Multiple widgets can subscribe to pen events.

 Only one widget can subscribe to *all* pen events. Expect a lot of events
 flowing in as the user is moving the stylus.

 \param widget Widget to subscribe to pen events
 \param outside_events If true, receive events even when pen is outside widget
 */
FL_EXPORT extern void subscribe(Fl_Widget* widget, bool outside_events = false);

/**
 \brief Stop receiving pen events for this widget.
 Deleting a widget will automatically unsubscribe it.
 \param widget Widget to unsubscribe from pen events
 */
FL_EXPORT extern void unsubscribe(Fl_Widget* widget);

/**
 \brief Grab all pen events, including those outside the widget, until released.
 Send all pen events exclusively to this widget to allow consistent drawing,
 even when the user crosses the edge of the window.
 \param widget Widget that will receive all pen events
 */
FL_EXPORT extern void grab(Fl_Widget* widget);

/**
 * \brief Release pen event handling after grab().
 */
FL_EXPORT extern void release();

/// \name Query values during event handling
/// @{

/**
 * \brief Returns the pen x and y position inside the handling widget as doubles.
 *
 * These functions provide high-precision pen coordinates relative to the widget
 * that received the pen event. For integer coordinates, use Fl::event_x() and
 * Fl::event_y() instead.
 *
 * \return Pen position as floating-point coordinate
 * \see Fl::event_x(), Fl::event_y()
 */
FL_EXPORT extern double event_x();
/** \brief Returns pen Y coordinate in widget space. \see event_x() */
FL_EXPORT extern double event_y();

/**
 * \brief Returns the pen x and y position in global coordinates as doubles.
 *
 * For integer coordinates, use Fl::event_x_root() and Fl::event_y_root().
 *
 * \return Pen position as floating-point coordinate in screen space
 * \see Fl::event_x_root(), Fl::event_y_root()
 */
FL_EXPORT extern double event_x_root();
/** \brief Returns pen Y coordinate in screen space. \see event_x_root() */
FL_EXPORT extern double event_y_root();

/**
 * \brief Returns the ID of the pen used in the last event.
 * \return Unique pen identifier, or -1 if pen was removed
 * \see Features::DEVICE_ID
 */
FL_EXPORT extern int event_id();

/**
 * \brief Returns the pressure of the tip or eraser on the surface between 0 and 1.
 * \return Pressure value from 0.0 (no pressure) to 1.0 (maximum pressure)
 * \see Features::PRESSURE, Features::TANGENTIAL_PRESSURE
 */
FL_EXPORT extern double event_pressure();
/** \brief Returns tangential pressure. \see event_pressure() */
FL_EXPORT extern double event_tangential_pressure();

/**
 * \brief Returns the tilt of the pen in the x and y directions between -1 and 1.
 *
 * X-axis tilt (left/right): For -1, the pen tilts fully to the left, and the
 * tip points to the right; for +1, the pen tilts fully to the right. A value of
 * 0 indicates a vertical pen.
 *
 * Y-axis tilt (forward/back): For -1, the pen tilts away from the user, and the
 * tip points toward the user.
 *
 * \return Tilt value from -1.0 to +1.0
 * \see Features::TILT_X, Features::TILT_Y
 */
FL_EXPORT extern double event_tilt_x();
/** \brief Returns pen Y-axis tilt. \see event_tilt_x() */
FL_EXPORT extern double event_tilt_y();

/**
 * \brief Returns the twist of the pen between -180 and +180 degrees.
 * \return Twist angle in degrees
 * \see Features::TWIST
 */
FL_EXPORT extern double event_twist();

/**
 * \brief Returns the proximity of the pen to the surface between 0 and 1.
 *
 * A proximity of 0 is closest to the surface, 1 is farthest away.
 *
 * \return Proximity value from 0.0 (touching) to 1.0 (far away)
 * \see Features::PROXIMITY
 */
FL_EXPORT extern double event_proximity();

/**
 * \brief Returns the state of the various buttons and tips.
 * \return Current state flags (combination of State values)
 */
FL_EXPORT extern State event_state();

/**
 * \brief Returns the state change that triggered the event.
 * \return State flags that changed to trigger this event
 */
FL_EXPORT extern State event_trigger();
/// @}

/*
 Resources:

 https://github.com/Fuzzyzilla/octotablet
 https://wiki.libsdl.org/SDL3/CategoryPen
 https://developer-docs.wacom.com/docs/overview/sdks/sdk-for-ink/
 https://learn.microsoft.com/en-us/windows/win32/tablet/mobile-pc-and-tablet-pc-samples
  WM_POINTER..., GetPointerInfo, GetPointerFramePenInfoHistory
 https://github.com/linuxwacom/input-wacom/wiki/Device-IDs
 https://wayland.freedesktop.org/libinput/doc/latest/tablet-support.html

 Windows:
 1. Legacy WinTab API (Win2k), Wintab32.dll, wintab.h
    https://developer.wacom.com/en-us/developer-dashboard/downloads
 2. Windows Ink API (Modern, Win10), Windows.UI.Input.Inking (WinRT API), InkCanvas(), etc.
    https://learn.microsoft.com/windows/uwp/design/input/windows-ink
 3. Pointer Input / WM_POINTER API (Win8), WM_POINTERUPDATE, GetPointerPenInfo
    https://learn.microsoft.com/windows/win32/inputmsg/wm-pointerupdate
      return WTInfo(0, 0, NULL) > 0; // Wintab check

 macOS:
 1. Pointer Events: NSEventSubtypeTabletPoint NSEventSubtypeTabletProximity
    https://developer.apple.com/documentation/appkit/nsevent/eventtype/tabletpoint?utm_source=chatgpt.com&language=objc
 2. AppKit
    https://developer.apple.com/documentation/appkit/nspressureconfiguration?changes=_7&language=objc&utm_source=chatgpt.com
 3. Wacom Device Kit API
    https://developer.wacom.com/en-us/developer-dashboard/downloads

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

/** @} */ // group fl_pen_events

} // namespace Pen

} // namespace Fl

/**
 \brief Bitwise OR operator for Features enum.
 \param lhs Left-hand side feature flags
 \param rhs Right-hand side feature flags
 \return Combined feature flags
 */
inline constexpr Fl::Pen::Features operator|(Fl::Pen::Features lhs, Fl::Pen::Features rhs) {
  return static_cast<Fl::Pen::Features>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

/**
 \brief Bitwise AND operator for Features enum.
 \param lhs Left-hand side feature flags
 \param rhs Right-hand side feature flags
 \return Intersection of feature flags
 */
inline constexpr Fl::Pen::Features operator&(Fl::Pen::Features lhs, Fl::Pen::Features rhs) {
  return static_cast<Fl::Pen::Features>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

/**
 \brief Bitwise OR assignment operator for Features enum.
 \param lhs Left-hand side feature flags (modified in place)
 \param rhs Right-hand side feature flags
 \return Reference to modified lhs
 */
inline Fl::Pen::Features& operator|=(Fl::Pen::Features& lhs, Fl::Pen::Features rhs) {
  lhs = lhs | rhs;
  return lhs;
}

/**
 \brief Bitwise OR operator for State enum.
 \param lhs Left-hand side state flags
 \param rhs Right-hand side state flags
 \return Combined state flags
 */
inline constexpr Fl::Pen::State operator|(Fl::Pen::State lhs, Fl::Pen::State rhs) {
  return static_cast<Fl::Pen::State>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

/**
 \brief Bitwise AND operator for State enum.
 \param lhs Left-hand side state flags
 \param rhs Right-hand side state flags
 \return Intersection of state flags
 */
inline constexpr Fl::Pen::State operator&(Fl::Pen::State lhs, Fl::Pen::State rhs) {
  return static_cast<Fl::Pen::State>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

/**
 \brief Bitwise OR assignment operator for State enum.
 \param lhs Left-hand side state flags (modified in place)
 \param rhs Right-hand side state flags
 \return Reference to modified lhs
 */
inline Fl::Pen::State& operator|=(Fl::Pen::State& lhs, Fl::Pen::State rhs) {
  lhs = lhs | rhs;
  return lhs;
}

#endif // !Fl_core_pen_events_H
