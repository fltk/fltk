//
// Menu window header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
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

/* \file
   Fl_Menu_Window widget . */

#ifndef Fl_Menu_Window_H
#define Fl_Menu_Window_H

#include "Fl_Single_Window.H"

/**
  The Fl_Menu_Window widget is a window type originally used for menus.

  Historically this window is drawn in hardware overlay layers of the graphics
  card if they are available so that the menu doesn't force the rest of the
  window to redraw (older SGI machines running under IRIX and X11).

  With improved hardware and operating systems handling layerd windows and
  defaulting to double buffering, overlay support was no longer useful and was
  removed from FLTK in version 1.4.

  Fl_Menu_Window has currently no special behavior over Fl_Single_Window.
  In Fl_Menu.cxx, the local classes `Menu_Window_Basetype` and
  `Menu_Title_Window` are derived from `Fl_Menu_Window`, implementing additional
  popup functionality.
*/
class FL_EXPORT Fl_Menu_Window : public Fl_Single_Window {
public:
  ~Fl_Menu_Window();
  /** Creates a new Fl_Menu_Window widget using the given size, and label string. */
  Fl_Menu_Window(int W, int H, const char *l = 0);
  /** Creates a new Fl_Menu_Window widget using the given position, size, and label string. */
  Fl_Menu_Window(int X, int Y, int W, int H, const char *l = 0);
};

#endif
