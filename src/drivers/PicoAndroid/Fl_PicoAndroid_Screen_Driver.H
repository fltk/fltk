//
// "$Id: Fl_PicoAndroid_Screen_Driver.H 11017 2016-01-20 21:40:12Z matt $"
//
// Definition of Android Screen interface based on Pico
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
 \file Fl_PicoAndroid_Screen_Driver.H
 \brief Definition of SDL Screen interface based on Pico.
 */

#ifndef FL_PICOANDROID_SCREEN_DRIVER_H
#define FL_PICOANDROID_SCREEN_DRIVER_H

#include "../Pico/Fl_Pico_Screen_Driver.H"

#include <jni.h>
#include <errno.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>



class FL_EXPORT Fl_PicoAndroid_Screen_Driver : public Fl_Pico_Screen_Driver
{
  struct android_app* pApp;

  static void handleAppCmdCB(struct android_app* app, int32_t cmd);
  void handleAppCmd(struct android_app* app, int32_t cmd);
  static int32_t handleInputEventCB(struct android_app* app, AInputEvent* event);
  int32_t handleInputEvent(struct android_app* app, AInputEvent* event);

public:
  Fl_PicoAndroid_Screen_Driver();
  virtual ~Fl_PicoAndroid_Screen_Driver();
  virtual double wait(double time_to_wait);
  
public:
  void android_main(struct android_app* state);
};


#endif // FL_PICOANDROID_SCREEN_DRIVER_H

//
// End of "$Id: Fl_PicoAndroid_Screen_Driver.H 11017 2016-01-20 21:40:12Z matt $".
//
