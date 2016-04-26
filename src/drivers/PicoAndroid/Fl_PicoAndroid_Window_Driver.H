//
// "$Id$"
//
// Definition of Android Window interface
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

/**
 \file Fl_PicoAndroid_Window_Driver.H
 \brief Definition of SDL Window interface.
 */

#ifndef FL_PICOANDROID_WINDOW_DRIVER_H
#define FL_PICOANDROID_WINDOW_DRIVER_H

#include "../Pico/Fl_Pico_Window_Driver.H"

#include <jni.h>
#include <android_native_app_glue.h>


class FL_EXPORT Fl_PicoAndroid_Window_Driver : public Fl_Pico_Window_Driver
{
  ANativeWindow *pNativeWindow;
public:
  Fl_PicoAndroid_Window_Driver(Fl_Window *win);
  virtual ~Fl_PicoAndroid_Window_Driver();

  virtual Fl_X *makeWindow();

  // --- window data
//  virtual int decorated_w() = 0;
//  virtual int decorated_h() = 0;

  // --- window management
//  virtual void take_focus();
  virtual void flush_single();
  virtual void flush_double();
  virtual void flush_overlay();
//  virtual void draw_begin();
//  virtual void draw_end();
};


#endif // FL_PICOSDL_WINDOW_DRIVER_H

//
// End of "$Id$".
//
