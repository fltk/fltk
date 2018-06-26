//
// "$Id$"
//
// Clock widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2017 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

#include <FL/Fl.H>
#include <FL/Fl_Clock.H>
#include <FL/Fl_Round_Clock.H>
#include "Fl_System_Driver.H"
#include <FL/fl_draw.H>
#include <math.h>
#include <time.h>

// Original clock display written by Paul Haeberli at SGI.
// Modifications by Mark Overmars for Forms
// Further changes by Bill Spitzak for fltk

const float hourhand[4][2] = {{-0.5f, 0}, {0, 1.5f}, {0.5f, 0}, {0, -7.0f}};
const float  minhand[4][2] = {{-0.5f, 0}, {0, 1.5f}, {0.5f, 0}, {0, -11.5f}};
const float  sechand[4][2] = {{-0.1f, 0}, {0, 2.0f}, {0.1f, 0}, {0, -11.5f}};

static void drawhand(double ang,const float v[][2],Fl_Color fill,Fl_Color line)
{
  fl_push_matrix();
  fl_rotate(ang);
  fl_color(fill);
  fl_begin_polygon();
  int i; for (i=0; i<4; i++) fl_vertex(v[i][0],v[i][1]);
  fl_end_polygon();
  fl_color(line);
  fl_begin_loop();
  for (i=0; i<4; i++) fl_vertex(v[i][0],v[i][1]);
  fl_end_loop();
  fl_pop_matrix();
}

void Fl_Clock_Output::drawhands(Fl_Color fill, Fl_Color line) {
  if (!active_r()) {
    fill = fl_inactive(fill);
    line = fl_inactive(line);
  }
  drawhand(-360*(hour()+minute()/60.0)/12, hourhand, fill, line);
  drawhand(-360*(minute()+second()/60.0)/60, minhand, fill, line);
  drawhand(-360*(second()/60.0), sechand, fill, line);
}

static void rect(double x, double y, double w, double h) {
  double r = x+w;
  double t = y+h;
  fl_begin_polygon();
  fl_vertex(x, y);
  fl_vertex(r, y);
  fl_vertex(r, t);
  fl_vertex(x, t);
  fl_end_polygon();
}

/**
  Draw clock with the given position and size.
  \param[in] X, Y, W, H position and size
*/
void Fl_Clock_Output::draw(int X, int Y, int W, int H) {
  Fl_Color box_color = type()==FL_ROUND_CLOCK ? FL_GRAY : color();
  draw_box(box(), X, Y, W, H, box_color);
  fl_push_matrix();
  fl_translate(X+W/2.0-.5, Y+H/2.0-.5);
  fl_scale((W-1)/28.0, (H-1)/28.0);
  if (type() == FL_ROUND_CLOCK) {
    fl_color(active_r() ? color() : fl_inactive(color()));
    fl_begin_polygon(); fl_circle(0,0,14); fl_end_polygon();
    fl_color(active_r() ? FL_FOREGROUND_COLOR : fl_inactive(FL_FOREGROUND_COLOR));
    fl_begin_loop(); fl_circle(0,0,14); fl_end_loop();
  }

  // draw the shadows:
  if (shadow_) {
    Fl_Color shadow_color = fl_color_average(box_color, FL_BLACK, 0.5);
    fl_push_matrix();
    fl_translate(0.60, 0.60);
    drawhands(shadow_color, shadow_color);
    fl_pop_matrix();
  }

  // draw the tick marks:
  fl_push_matrix();
  fl_color(active_r() ? FL_FOREGROUND_COLOR : fl_inactive(FL_FOREGROUND_COLOR));
  for (int i=0; i<12; i++) {
    if (i==6) rect(-0.5, 9, 1, 2);
    else if (i==3 || i==0 || i== 9) rect(-0.5, 9.5, 1, 1);
    else rect(-0.25, 9.5, .5, 1);
    fl_rotate(-30);
  }
  fl_pop_matrix();

  // draw the hands:
  drawhands(selection_color(), FL_FOREGROUND_COLOR); // color was 54
  fl_pop_matrix();
}

/**
  Draw clock with current position and size.
*/
void Fl_Clock_Output::draw() {
  draw(x(), y(), w(), h());
  draw_label();
}

/**
  Set the displayed time.
  Set the time in hours, minutes, and seconds.
  \param[in] H, m, s displayed time
  \see hour(), minute(), second()
 */
void Fl_Clock_Output::value(int H, int m, int s) {
  if (H!=hour_ || m!=minute_ || s!=second_) {
    hour_ = H; minute_ = m; second_ = s;
    value_ = (H * 60 + m) * 60 + s;
    damage(FL_DAMAGE_CHILD);
  }
}

/**
  Set the displayed time.
  Set the time in seconds since the UNIX epoch (January 1, 1970).
  \param[in] v seconds since epoch
  \see value()
 */
void Fl_Clock_Output::value(ulong v) {
  value_ = v;
  struct tm *timeofday;
  // Some platforms, notably Windows, now use a 64-bit time_t value...
  time_t vv = (time_t)v;
  timeofday = localtime(&vv);
  value(timeofday->tm_hour, timeofday->tm_min, timeofday->tm_sec);
}

/**
  Create a new Fl_Clock_Output widget with the given position, size and label.

  The default clock type is \c FL_SQUARE_CLOCK and the default boxtype is
  \c FL_UP_BOX.

  \param[in] X, Y, W, H position and size of the widget
  \param[in] L widget label, default is no label
 */
Fl_Clock_Output::Fl_Clock_Output(int X, int Y, int W, int H, const char *L)
: Fl_Widget(X, Y, W, H, L) {
  box(FL_UP_BOX);
  selection_color(fl_gray_ramp(5));
  align(FL_ALIGN_BOTTOM);
  hour_ = 0;
  minute_ = 0;
  second_ = 0;
  value_ = 0;
  shadow_ = 1;
}

////////////////////////////////////////////////////////////////

/**
  Create an Fl_Clock widget using the given position, size, and label string.

  The default clock type is FL_SQUARE_CLOCK and the default
  boxtype is \c FL_UP_BOX.

  \param[in] X, Y, W, H position and size of the widget
  \param[in] L widget label, default is no label
 */
Fl_Clock::Fl_Clock(int X, int Y, int W, int H, const char *L)
  : Fl_Clock_Output(X, Y, W, H, L) {}

/**
  Create an Fl_Clock widget using the given clock type \p t,
  position, size, and label string.

  The default clock type \p t is \c FL_SQUARE_CLOCK. You can set the
  clock type to FL_ROUND_CLOCK or any other valid clock type.
  See Fl_Clock_Output widget for applicable values.

  The default boxtype is \c FL_UP_BOX for \c FL_SQUARE_CLOCK
  and \c FL_NO_BOX for \c FL_ROUND_CLOCK, if set by the constructor.
  If you change the clock type with type() later you should also set
  the boxtype with box().

  \param[in] t type of clock: FL_ROUND_CLOCK or FL_SQUARE_CLOCK (0)
  \param[in] X, Y, W, H position and size of the widget
  \param[in] L widget label, default is no label

  \see class Fl_Clock_Output
*/

Fl_Clock::Fl_Clock(uchar t, int X, int Y, int W, int H, const char *L)
  : Fl_Clock_Output(X, Y, W, H, L) {
  type(t);
  box(t==FL_ROUND_CLOCK ? FL_NO_BOX : FL_UP_BOX);
}

static void tick(void *v) {
  time_t sec;
  int usec;
  Fl::system_driver()->gettime(&sec, &usec);
  ((Fl_Clock*)v)->value((ulong)sec);
  Fl::add_timeout((1000000 - usec)/1000000., tick, v); // time till next second
}

int Fl_Clock::handle(int event) {
  switch (event) {
  case FL_SHOW:
    tick(this);
    break;
  case FL_HIDE:
    Fl::remove_timeout(tick, this);
    break;
  }
  return Fl_Clock_Output::handle(event);
}

/**
  The destructor removes the clock.
 */
Fl_Clock::~Fl_Clock() {
  Fl::remove_timeout(tick, this);
}


/**
  Create an Fl_Round_Clock widget using the given
  position, size, and label string.

  The clock type is \c FL_ROUND_CLOCK and the boxtype is \c FL_NO_BOX.

  This construcktor is the same as Fl_Clock(FL_ROUND_CLOCK, X, Y, W, H, L).
  \see Fl_Clock(uchar, int, int, int, int, const char *)

  \param[in] X, Y, W, H position and size of the widget
  \param[in] L widget label, default is no label
*/

Fl_Round_Clock::Fl_Round_Clock(int X,int Y,int W,int H, const char *L)
: Fl_Clock(X, Y, W, H, L)
{
  type(FL_ROUND_CLOCK);
  box(FL_NO_BOX);
}


//
// End of "$Id$".
//
