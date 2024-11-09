//
// Arrow drawing code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2023 by Bill Spitzak and others.
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

// These functions implement drawing of all "arrow like" GUI elements in scrollbars,
// choice widgets, menus, etc.

// Implementation of fl_draw_arrow(...) dependent on the active FLTK Scheme.

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/fl_utf8.h>
#include "fl_oxy.h"

// Debug mode: if you design a widget or want to check its layout,
// then enable one or both flags of DEBUG_ARROW (below) so you can
// see where the arrows (i.e. their bounding boxes) are positioned

#ifndef DEBUG_ARROW
#define DEBUG_ARROW (0)         // 0 = off, 1 = green background, 2 = red frame, 3 = both
#endif

void debug_arrow(Fl_Rect r) {

#if (DEBUG_ARROW & 1)
  fl_color(fl_lighter(FL_GREEN));
  fl_rectf(r);
#endif

#if (DEBUG_ARROW & 2)
  fl_color(FL_RED);
  fl_line_style(FL_SOLID, 1);  // work around X11 bug with default line width 0
  fl_rect(r);
  fl_line_style(FL_SOLID, 0);  // reset line style
#endif

} // debug_arrow

// Calculate the applicable arrow size.
// Imagine an arrow pointing to the right side:
// - the calculated size s is the width of the arrow,
// - the height of the arrow is 2 * s.
// The calculation takes into account that we need one pixel padding at
// all sides and that the available space doesn't need to be a square,
// i.e. it's possible that r.w() != r.h().

static int arrow_size(Fl_Rect r, Fl_Orientation o, int num = 1) {

  int s, d1, d2;

  switch(o) {
    case FL_ORIENT_LEFT:
    case FL_ORIENT_RIGHT:
      d1 = (r.w() - 2) / num;
      d2 = (r.h() - 2) / 2;
      break;
    default:  // up or down arrow
      d1 = (r.h() - 2) / num;
      d2 = (r.w() - 2) / 2;
      break;
  }
  s = d1 < d2 ? d1 : d2;
  if (s < 2) s = 2;
  else if (s > 6) s = 6;
  return s;
}

// Draw a "Single Arrow" in an arbitrary direction (0°, 90°, 180°, 270°).
// This is the basic arrow drawing function for all "standard" widgets.
// It is used in Fl_Scrollbars and similar and in all combinations, for
// instance when "Double Arrows" or other combinations are needed.

static int fl_draw_arrow_single(Fl_Rect r, Fl_Orientation o, Fl_Color col, int d = -1) {

  int x1, y1;

  // Revert gtk+ specific "chevron style" arrow drawing: see GitHub Issue #1117.
  //  - gtk_chevron == true  : use gtk+ specific ("chevron style") arrows
  //  - gtk_chevron == false : use standard ("triangle") arrows
  //
  // Note 1: the "chevron style" was initially copied from Fl_Scrollbar and
  //   then used in all "arrow" drawings, e.g. in Fl_Menu to unify arrow
  //   appearance across all widgets and per scheme. This was probably
  //   too much as mentioned in GitHub Issue #1117. The consequence is to
  //   set 'gtk_chevron' to false to prevent the "chevron style".
  //
  // Note 2: In the future we may use more specific arrow types if needed and
  //   integrate arrow drawing in Fl_Scheme_* classes.

  static const bool gtk_chevron = false; // ... or: Fl::is_scheme("gtk+");

  x1 = r.x();
  y1 = r.y();
  if (d < 0)
    d = arrow_size(r, o);

  fl_color(col);

  switch(o) {

    case FL_ORIENT_LEFT:
      x1 += (r.w()-d)/2 - 1;
      y1 += r.h()/2;
      if (gtk_chevron)
        fl_polygon(x1, y1, x1+d, y1-d, x1+d-1, y1, x1+d, y1+d);
      else
        fl_polygon(x1, y1, x1+d, y1-d, x1+d, y1+d);
      return 1;

    case FL_ORIENT_RIGHT:
      x1 += (r.w()-d)/2;
      y1 += r.h()/2;
      if (gtk_chevron)
        fl_polygon(x1, y1-d, x1+1, y1, x1, y1+d, x1+d, y1);
      else
        fl_polygon(x1, y1-d, x1, y1+d, x1+d, y1);
      return 1;

    case FL_ORIENT_UP:
      x1 += r.w()/2;
      y1 += (r.h()-d)/2 - 1;
      if (gtk_chevron)
        fl_polygon(x1, y1, x1+d, y1+d, x1, y1+d-1, x1-d, y1+d);
      else
        fl_polygon(x1, y1, x1+d, y1+d, x1-d, y1+d);
      return 1;

    case FL_ORIENT_DOWN:
      x1 += r.w()/2-d;
      y1 += (r.h()-d)/2;
      if (gtk_chevron) {
        fl_polygon(x1, y1, x1+d, y1+1, x1+d, y1+d);
        fl_polygon(x1+d, y1+1, x1+2*d, y1, x1+d, y1+d);
      } else {
        fl_polygon(x1, y1, x1+d, y1+d, x1+2*d, y1);
      }
      return 1;

    default:            // orientation not handled: return error
      return 0;
  }
  return 0;
} // fl_draw_arrow_single()


// Draw a "Double Arrow" in an arbitrary direction (0°, 90°, 180°, 270°).
// This is the basic arrow drawing function for all "standard" widgets.
// It is used in Fl_Scrollbars and similar and in all combinations, for
// instance when "Double Arrows" or other combinations are needed.

static int fl_draw_arrow_double(Fl_Rect r, Fl_Orientation o, Fl_Color col) {

  int d = arrow_size(r, o, 2);
  int x1 = r.x();
  int y1 = r.y();
  int da = (d+1)/2;

  switch(o) {

    case FL_ORIENT_LEFT:
    case FL_ORIENT_RIGHT:
      r.x(x1 - da);
      fl_draw_arrow_single(r, o, col, d);
      r.x(x1 + da);
      return fl_draw_arrow_single(r, o, col, d);

    case FL_ORIENT_UP:
    case FL_ORIENT_DOWN:
      r.y(y1 - da);
      fl_draw_arrow_single(r, o, col, d);
      r.y(y1 + da);
      return fl_draw_arrow_single(r, o, col, d);

    default:            // orientation not handled: return error
      return 0;
  }
  return 0;
} // fl_draw_arrow_double()


// Draw a "Choice Arrow". The direction and type is determined by the scheme.

static int fl_draw_arrow_choice(Fl_Rect r, Fl_Color col) {

  int w1 = (r.w() - 4) / 3; if (w1 < 1) w1 = 1;
  int x1 = r.x() + (r.w() - 2 * w1 - 1) / 2;
  int y1 = r.y() + (r.h() - w1 - 1) / 2;

  if (Fl::is_scheme("gtk+") ||
      Fl::is_scheme("gleam")) {
    // Show smaller up/down arrows ...
    int x1 = r.x() + (r.w() - 6)/2;
    int y1 = r.y() + r.h() / 2;
    fl_color(col);
    fl_polygon(x1, y1 - 2, x1 + 3, y1 - 5, x1 + 6, y1 - 2);
    fl_polygon(x1, y1 + 2, x1 + 3, y1 + 5, x1 + 6, y1 + 2);
    return 1;
  }
  else if (Fl::is_scheme("plastic")) {
    // Show larger up/down arrows...
    fl_color(col);
    fl_polygon(x1, y1 + 3, x1 + w1, y1 + w1 + 3, x1 + 2 * w1, y1 + 3);
    fl_polygon(x1, y1 + 1, x1 + w1, y1 - w1 + 1, x1 + 2 * w1, y1 + 1);
    return 1;
  }
  else { // none, default             // single down arrow
    return fl_draw_arrow_single(r, FL_ORIENT_DOWN, col);
  }
  return 0;
} // fl_draw_arrow_double()


/**
  Draw an "arrow like" GUI element for the selected scheme.

  In the future this function should be integrated in Fl_Scheme
  as a virtual method, i.e. it would call a method like ...
  \code
    Fl_Scheme::current()->draw_arrow(r, t, o, col);
  \endcode

  \param[in]  r    bounding box
  \param[in]  t    arrow type
  \param[in]  o    orientation
  \param[in]  col  arrow color

  \since 1.4.0
*/

void fl_draw_arrow(Fl_Rect r, Fl_Arrow_Type t, Fl_Orientation o, Fl_Color col) {

  int ret = 0;
  Fl_Color saved_color = fl_color();

  debug_arrow(r);

  // special case: arrows for the "oxy" scheme

  if (Fl::is_scheme("oxy")) {
    oxy_arrow(r, t, o, col);
    return;
  }

  // implementation of all arrow types for other schemes

  switch(t) {
    case FL_ARROW_SINGLE:
      ret = fl_draw_arrow_single(r, o, col);
      break;

    case FL_ARROW_DOUBLE:
      ret = fl_draw_arrow_double(r, o, col);
      break;

    case FL_ARROW_CHOICE:
      ret = fl_draw_arrow_choice(r, col);
      break;

    default:        // unknown arrow type
      ret = 0;
      break;
  }

  // draw an error flag (red rectangle with cross) if not successful

  if (!ret) {
    fl_color(FL_RED);
    fl_rectf(r);
    fl_color(FL_BLACK);
    fl_rect(r);
    fl_line(r.x(), r.y(), r.r(), r.b());
    fl_line(r.x(), r.b(), r.r(), r.y());
  }

  fl_color(saved_color);

} // fl_draw_arrow()
