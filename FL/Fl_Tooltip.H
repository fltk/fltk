//
// "$Id: Fl_Tooltip.H,v 1.16.2.2 2001/08/02 15:31:59 easysw Exp $"
//
// Tooltip header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2001 by Bill Spitzak and others.
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
// Please report all bugs and problems to "fltk-bugs@fltk.org".
//

#ifndef _Fl_Tooltip_H_
#define _Fl_Tooltip_H_

#include <FL/Fl.H>
#include <FL/Fl_Widget.H>


class Fl_TooltipBox;
class Fl_Menu_Window;

FL_EXPORT class Fl_Tooltip {

  friend class Fl_TooltipBox;
  friend class Fl_Widget;

  static float delay_;
  static Fl_TooltipBox *box;
  static Fl_Menu_Window *window;
  static Fl_Widget *widget;
  static int shown;
  static uchar color_;
  static int font_;
  static int size_;

  static void tooltip_timeout(void *);
  static void tooltip_exit(Fl_Widget *);

  static void (*tooltip_callback_)(void *);
  static void (*tooltip_exit_)(void *);

public:

  // These functions are user-called functions
  static float delay() { return delay_; }
  static void delay(float f) { delay_ = (f < 0.1) ? 0.1 : f; }
  static int font() { return font_; }
  static int size() { return size_; }
  static void font(int i) { font_ = i; }
  static void size(int s) { size_ = s; }
  static void color(uchar c) { color_ = c; }
  static uchar color() { return color_; }

  static void enter(Fl_Widget *w);
  static void exit(Fl_Widget *w);
  static int enabled() {return tooltip_callback_ != 0;}
  static void enable(int b = 1) {if (!b) enter(0); tooltip_callback_ = b ? tooltip_timeout : 0; tooltip_exit_ = b ? (void (*)(void *))tooltip_exit : 0;}
  static void disable() {enter(0); tooltip_callback_ = 0; tooltip_exit_ = 0;}
};


#endif

//
// End of "$Id: Fl_Tooltip.H,v 1.16.2.2 2001/08/02 15:31:59 easysw Exp $".
//
