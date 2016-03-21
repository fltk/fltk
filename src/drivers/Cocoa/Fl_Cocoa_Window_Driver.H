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
#include <ApplicationServices/ApplicationServices.h>

class Fl_Image;
class Fl_Window;
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
struct Fl_Window_Driver::shape_data_type {
  int lw_; ///<  width of shape image
  int lh_; ///<  height of shape image
  Fl_Image* shape_; ///<  shape image
  typedef struct CGImage* CGImageRef;
  CGImageRef mask;
};


class FL_EXPORT Fl_Cocoa_Window_Driver : public Fl_Window_Driver
{
private:
  void shape_bitmap_(Fl_Image* b);
  void shape_alpha_(Fl_Image* img, int offset);

public:
  Fl_Cocoa_Window_Driver(Fl_Window*);
  ~Fl_Cocoa_Window_Driver();

  // --- window data
  virtual int decorated_w();
  virtual int decorated_h();

  // --- window management
  virtual void take_focus();
  virtual void flush_double();
  virtual void flush_overlay();
  virtual void draw_begin();
  virtual void draw_end();
  virtual void make_current();

  virtual void shape(const Fl_Image* img);
  // that one is implemented in Fl_Cocoa.mm because it uses Objective-c
  virtual void capture_titlebar_and_borders(Fl_Shared_Image*& top, Fl_Shared_Image*& left, Fl_Shared_Image*& bottom, Fl_Shared_Image*& right);
  //this one is in Fl_cocoa.mm because it uses Objective-c
  virtual void wait_for_expose();
};



#endif // FL_COCOA_WINDOW_DRIVER_H

//
// End of "$Id: quartz.H 11017 2016-01-20 21:40:12Z matt $".
//
