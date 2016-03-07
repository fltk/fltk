//
// "$Id$"
//
// A base class for platform specific window handling code
// for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2016 by Bill Spitzak and others.
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

/** \file Fl_Window_Driver.H
 \brief declaration of classe Fl_Window_Driver.
*/

#ifndef FL_WINDOW_DRIVER_H
#define FL_WINDOW_DRIVER_H

#include <FL/x.H>


class Fl_Window;


/**
 \brief A base class for platform specific window handling code.
  */
class FL_EXPORT Fl_Window_Driver {
protected:
  Fl_Window *pWindow;
public:
  Fl_Window_Driver(Fl_Window *);
  virtual ~Fl_Window_Driver();
  static Fl_Window_Driver *newWindowDriver(Fl_Window *);

  virtual void take_focus() { }
  virtual int double_flush(int eraseoverlay);
  virtual void destroy_double_buffer();
  void draw();
};


#endif // FL_WINDOW_DRIVER_H

//
// End of "$Id$".
//
