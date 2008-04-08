//
// "$Id$"
//
// Clock header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2005 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

/** \file
 * Fl_Clock widget definitions for the Fast Light Tool Kit (FLTK).
 */

#ifndef Fl_Clock_H
#define Fl_Clock_H

#ifndef Fl_Widget_H
#include "Fl_Widget.H"
#endif

// values for type:
#define FL_SQUARE_CLOCK		0
#define FL_ROUND_CLOCK		1
#define FL_ANALOG_CLOCK FL_SQUARE_CLOCK
#define FL_DIGITAL_CLOCK FL_SQUARE_CLOCK // nyi

// a Fl_Clock_Output can be used to display a program-supplied time:

/**
 * This widget can be used to display a program-supplied time.
 * The time shown on the clock is not updated. 
 * To display the current time, use <TT>Fl_Clock</A></TT> instead.
 *
 * \image html clock.gif 
 *
 * \image html round_clock.gif
 */
class FL_EXPORT Fl_Clock_Output : public Fl_Widget {
  int hour_, minute_, second_;
  ulong value_;
  void drawhands(Fl_Color,Fl_Color); // part of draw
protected:
  void draw(int, int, int, int);
  void draw();
public:
  
  /**
   * Creates a new <tt>Fl_Clock_Output</tt> widget.
   * Create an <tt>Fl_Clock_Output</tt> widget using the given position, 
   * size, and label string. The default boxtype is <tt>FL_NO_BOX</tt>.
   *
   * \param[in] x, y, w, h position and size of the widget
   * \param[in] label widget label, default is no label
   */
  Fl_Clock_Output(int x,int y,int w,int h, const char *l = 0);

  /**
   * Set the displayed time.
   * Set the time in seconds since the UNIX epoch (January 1, 1970).
   * \see value()
   */
  void value(ulong v);	// set to this Unix time
  
  /**
   * Set the displayed time.
   * Set the time in hours, minutes, and seconds.
   * \param[in] hour, minute, second displayed time
   * \see hour(), minute(), second()
   */
  void value(int hour, int minute, int second);
  
  /**
   * Returns the displayed time.
   * Returns the time in seconds since the UNIX epoch (January 1, 1970).
   * \see value(ulong)
   */
  ulong value() const {return value_;}

  /**
   * Returns the displayed time.
   * Returns the displayed hour (0 to 23).
   * \see value(), minute(), second()
   */
  int hour() const {return hour_;}

  /**
   * Returns the displayed time.
   * Returns the displayed minute (0 to 59).
   * \see value(), hour(), second()
   */
  int minute() const {return minute_;}

  /**
   * Returns the displayed time.
   * Returns the displayed second (0 to 60, 60=leap second).
   * \see value(), hour(), minute()
   */
  int second() const {return second_;}
};

// a Fl_Clock displays the current time always by using a timeout:

/**
 * This widget provides a round analog clock display.
 * <tt>Fl_Clock</tt> is provided for Forms compatibility. 
 * It installs a 1-second timeout callback using <tt>Fl::add_timeout()</tt>.
 *
 * \image html clock.gif 
 * 
 * \image html round_clock.gif
 */
class FL_EXPORT Fl_Clock : public Fl_Clock_Output {
public:
  int handle(int);
  void update();

  /**
   * Creates a new <tt>Fl_Clock</tt> widget.
   * Create an <tt>Fl_Clock</tt> widget using the given position, 
   * size, and label string. The default boxtype is <tt>FL_NO_BOX</tt>.
   *
   * \param[in] x, y, w, h position and size of the widget
   * \param[in] label widget label, default is no label
   */
  Fl_Clock(int x,int y,int w,int h, const char *l = 0);
  
  /**
   * Creates a new <tt>Fl_Clock</tt> widget.
   * Create an <tt>Fl_Clock</tt> widget using the given position, 
   * size, and label string.
   *
   * \param[in] t boxtype
   * \param[in] x, y, w, h position and size of the widget
   * \param[in] label widget label, default is no label
   */
  Fl_Clock(uchar t,int x,int y,int w,int h, const char *l);
  
  /**
   * The destructor removes the clock.
   */
  ~Fl_Clock();
};

#endif

//
// End of "$Id$".
//
