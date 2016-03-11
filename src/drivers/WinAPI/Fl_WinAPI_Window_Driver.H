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
 \file Fl_WinAPI_Window_Driver.H
 \brief Definition of MSWindows window driver.
 */

#ifndef FL_WINAPI_WINDOW_DRIVER_H
#define FL_WINAPI_WINDOW_DRIVER_H

#include <FL/Fl_Window_Driver.H>
#include <windows.h>

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

struct Fl_Window_Driver::icon_data {
  const void *legacy_icon;
  Fl_RGB_Image **icons;
  int count;
  HICON big_icon;
  HICON small_icon;
};

struct Fl_Window_Driver::shape_data_type {
  int lw_; ///<  width of shape image
  int lh_; ///<  height of shape image
  Fl_Image* shape_; ///<  shape image
  Fl_Bitmap *todelete_; ///<  auxiliary bitmap image
};

class FL_EXPORT Fl_WinAPI_Window_Driver : public Fl_Window_Driver
{
private:
  void shape_bitmap_(Fl_Image* b);
  void shape_alpha_(Fl_Image* img, int offset);
public:
  Fl_WinAPI_Window_Driver(Fl_Window*);
  ~Fl_WinAPI_Window_Driver();
  virtual void shape(const Fl_Image* img);
  virtual void draw();
  virtual void icons(const Fl_RGB_Image *icons[], int count);
  virtual const void *icon() const;
  virtual void icon(const void * ic);
  virtual void free_icons();
  void icons(HICON big_icon, HICON small_icon);
  // this one is implemented in Fl_win32.cxx
  virtual void capture_titlebar_and_borders(Fl_Shared_Image*& top, Fl_Shared_Image*& left, Fl_Shared_Image*& bottom, Fl_Shared_Image*& right);
  virtual void wait_for_expose();
};


#endif // FL_WINAPI_WINDOW_DRIVER_H

//
// End of "$Id: quartz.H 11017 2016-01-20 21:40:12Z matt $".
//
