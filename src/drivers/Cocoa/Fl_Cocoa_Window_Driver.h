//
// "$Id: quartz.H 11017 2016-01-20 21:40:12Z matt $"
//
// Definition of Apple Cocoa window driver
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
 \file Fl_Cocoa_Window_Driver.h
 \brief Definition of Apple Cocoa window driver.
 */

#ifndef FL_COCOA_WINDOW_DRIVER_H
#define FL_COCOA_WINDOW_DRIVER_H

#include <FL/Fl_Window_Driver.H>

/*
 Move everything here that manages the native window interface.

 There is one window driver for each Fl_Window. Window drivers manage window
 actions such as resizing, events, decoration, fullscreen modes, etc. . All
 drawing and rendering is managed by the Surface device and the associated 
 graphics driver.

 - window specific event handling
 - window types and styles, depth, etc.
 - decorations
 
 ? where do we handle the interface between OpenGL/DirectX and Cocoa/WIN32/Glx?
 */

class FL_EXPORT Fl_Cocoa_Window_Driver : public Fl_Window_Driver
{
public:
  Fl_Cocoa_Window_Driver(Fl_Window*);
  virtual void take_focus();
  int double_flush(int eraseoverlay);
};



#endif // FL_COCOA_WINDOW_DRIVER_H

//
// End of "$Id: quartz.H 11017 2016-01-20 21:40:12Z matt $".
//
