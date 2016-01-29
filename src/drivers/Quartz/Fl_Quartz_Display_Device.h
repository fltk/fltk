//
// "$Id$"
//
// Definition of class Fl_Quartz_Display_Device
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

/** \file Fl_Quartz_Disply_Device.h
 \brief Implement a connection between the Cocoa window management and the 
 Quartz graphics driver on OS X.
*/

#ifndef FL_QUARTZ_DISPLAY_DEVICE_H
#define FL_QUARTZ_DISPLAY_DEVICE_H

// FIXME: implement this
#if 0
#include <FL/x.H>
#include <FL/Fl_Plugin.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Bitmap.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_RGB_Image.H>
#include <stdlib.h>

/**
 A display to which the computer can draw.
 When the program begins running, an Fl_Display_Device instance has been created and made the current drawing surface.
 There is no need to create any other object of this class.
 */
class FL_EXPORT Fl_Display_Device : public Fl_Surface_Device {
  friend class Fl_Quartz_Graphics_Driver;
  static Fl_Display_Device *_display; // the platform display device
#ifdef __APPLE__
  friend class Fl_X;
  friend class Fl_Graphics_Driver;
  static bool high_res_window_; //< true when drawing to a window of a retina display (Mac OS X only)
  static bool high_resolution() {return high_res_window_;}
#elif defined(WIN32)
#elif defined(FL_PORTING)
# pragma message "FL_PORTING: implement functions for extra high res drawing if your platform supports it"
#else
#endif
public:
  static const char *class_id;
  const char *class_name() {return class_id;};
  Fl_Display_Device(Fl_Graphics_Driver *graphics_driver);
  static Fl_Display_Device *display_device();
};

#endif

#endif // FL_QUARTZ_DISPLAY_DEVICE_H

//
// End of "$Id$".
//
