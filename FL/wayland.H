//
// Wayland platform header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2021 by Bill Spitzak and others.
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

#if !defined(FL_PLATFORM_H)
#  error "Never use <FL/wayland.H> directly; include <FL/platform.H> instead."
#endif // !FL_PLATFORM_H

typedef struct wld_window *Window;

struct flCairoRegion {
  int count;
  struct _cairo_rectangle *rects;
}; // a region is the union of a series of rectangles

#include <stdint.h>

FL_EXPORT struct wl_display *fl_wl_display();
FL_EXPORT struct wl_surface *fl_wl_surface(Window xid);
FL_EXPORT struct _cairo *fl_wl_cairo();
