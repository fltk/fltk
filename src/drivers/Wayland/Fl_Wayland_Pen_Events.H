//
// Definition of Wayland Tablet/Pen event driver for FLTK.
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

/**
 \file Fl_Wayland_Pen_Events.H
 \brief Wayland tablet/pen event driver interface.
 */

#ifndef FL_WAYLAND_PEN_EVENTS_H
#define FL_WAYLAND_PEN_EVENTS_H

struct wl_seat;
struct zwp_tablet_manager_v2;

/**
 Store the zwp_tablet_manager_v2 binding received from registry_handle_global.
 May be called before the wl_seat is ready; tablet seat init is deferred
 automatically until both the manager and the seat are available.
 */
void fl_wayland_tablet_set_manager(struct zwp_tablet_manager_v2 *manager);

/**
 Notify the pen driver that a wl_seat is available.
 Should be called from seat_capabilities() each time it fires; the function
 is idempotent once the tablet seat has been created.
 */
void fl_wayland_tablet_init_seat(struct wl_seat *wl_seat);

/**
 Destroy all tablet/tool objects and release the tablet manager.
 Must be called from close_display() before the wl_seat and wl_display are
 torn down.
 */
void fl_wayland_tablet_cleanup();

#endif // FL_WAYLAND_PEN_EVENTS_H
