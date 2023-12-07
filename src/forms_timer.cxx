//
// "$Id$"
//
// Forms timer object for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
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

// Emulate the Forms Timer object
// You don't want to use this if you just want a timeout, call
// Fl::add_timeout directly!

#include <FL/Fl.H>
#include <FL/Fl_Timer.H>
#include <FL/fl_draw.H>
#include <FL/forms.H>
#ifdef WIN32
#  ifdef __MWERKS__
#    include <time.h>
#  else
#    include <sys/types.h> 
#    include <sys/timeb.h>
#  endif
#else
#  include <time.h>
#  include <sys/time.h>
#endif
#include <stdio.h>

#define FL_TIMER_BLINKRATE	0.2

void fl_gettime(long* sec, long* usec) {
#ifdef WIN32
# ifdef __MWERKS__
  time_t localTime = time(NULL);
  struct tm *now = localtime(&localTime);
  *sec = now->tm_sec + 60*now->tm_min + 3600*now->tm_hour + 24*3600*now->tm_yday;
  *usec = 0;
# else
  struct _timeb tp;
  _ftime(&tp);
  *sec = (long) tp.time;
  *usec = tp.millitm * 1000;
# endif
#else
  struct timeval tp;
  struct timezone tzp;
  gettimeofday(&tp, &tzp);
  *sec = tp.tv_sec;
  *usec = tp.tv_usec;
#endif
}

void Fl_Timer::draw() {
  int tt;
  Fl_Color col;
  char str[32];
  if (!on || delay>0.0)
    col = color();
  else if ((int) (delay / FL_TIMER_BLINKRATE) % 2)
    col = color();
  else
    col = selection_color();
  draw_box(box(), col);
  if (type() == FL_VALUE_TIMER && delay>0.0) {
    double d = direction_ ? total-delay : delay;
    if (d < 60.0)
      sprintf(str, "%.1f", d);
    else {
      tt = (int) ((d+0.05) / 60.0);
      sprintf(str, "%d:%04.1f", tt, d - 60.0 * tt);
    }
    fl_font(labelfont(), labelsize());
    fl_color(labelcolor());
    fl_draw(str, x(), y(), w(), h(), FL_ALIGN_CENTER);
  } else
    draw_label();
}

void Fl_Timer::stepcb(void* v) {
  ((Fl_Timer*)v)->step();
}

void Fl_Timer::step() {
  if (!on) return;
  double lastdelay = delay;
  long sec, usec; fl_gettime(&sec, &usec);
  delay -= (double) (sec - lastsec) + (double) (usec - lastusec) / 1000000.0;
  lastsec = sec; lastusec = usec;
  if (lastdelay > 0.0 && delay <= 0.0) {
    if (type() == FL_HIDDEN_TIMER) {
      on = 0;
      delay = 0;
    } else {
      redraw();
      Fl::add_timeout(FL_TIMER_BLINKRATE, stepcb, this);
    }
    set_changed();
    do_callback();
  } else {
    if (type() == FL_VALUE_TIMER) redraw();
    Fl::add_timeout(FL_TIMER_BLINKRATE, stepcb, this);
  }
}

int Fl_Timer::handle(int event) {
  if (event == FL_RELEASE && delay <= 0) value(0.0);
  return 0;
}

/**
 Destroys the timer and removes the timeout.
*/
Fl_Timer::~Fl_Timer() {
  Fl::remove_timeout(stepcb, this);
}

/**
  Creates a new Fl_Timer widget using the given type, position,
  size, and label string. The type parameter can be any of the
  following symbolic constants:

  \li	FL_NORMAL_TIMER - The timer just does the callback and
	displays the string "Timer" in the widget.
  \li	FL_VALUE_TIMER - The timer does the callback and displays
	the current timer value in the widget.
  \li	FL_HIDDEN_TIMER - The timer just does the callback and
	does not display anything.
*/
Fl_Timer::Fl_Timer(uchar t, int X, int Y, int W, int H, const char* l)

: Fl_Widget(X, Y, W, H, l) {
  box(FL_DOWN_BOX);
  selection_color(FL_RED);
  delay = 0;
  on = 0;
  direction_ = 0;
  type(t);
  if (t == FL_HIDDEN_TIMER) clear_visible();
  if (t == FL_VALUE_TIMER) align(FL_ALIGN_LEFT);
}
/** Sets the current timer value */
void Fl_Timer::value(double d) {
  delay = total = d;
  on = (d > 0.0);
  fl_gettime(&(lastsec), &(lastusec));
  if (type() != FL_HIDDEN_TIMER) redraw();
  Fl::remove_timeout(stepcb, this);
  if (on) Fl::add_timeout(FL_TIMER_BLINKRATE, stepcb, this);
}

/** Gets or sets whether the timer is suspended.*/
void Fl_Timer::suspended(char d) {
  if (!d) {
    if (on) return;
    on = (delay > 0.0);
    fl_gettime(&(lastsec), &(lastusec));
    if (on) Fl::add_timeout(FL_TIMER_BLINKRATE, stepcb, this);
  } else {
    if (!on) return;
    on = 0;
    Fl::remove_timeout(stepcb, this);
  }
}

//
// End of "$Id$".
//
