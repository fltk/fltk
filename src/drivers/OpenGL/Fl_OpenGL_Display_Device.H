//
// "$Id$"
//
// implementation of class Fl_Gl_Device_Plugin for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2016 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems to:
//
//     http://www.fltk.org/str.php
//

#include <FL/Fl_Device.H>

class Fl_OpenGL_Graphics_Driver;
class Fl_Gl_Window;
class Fl_RGB_Image;

/**
 OpenGL Surface.
 This surface is needed as an interface between GL windows and the GL graphics driver.
 */
class FL_EXPORT Fl_OpenGL_Display_Device : public Fl_Surface_Device {
  Fl_OpenGL_Display_Device(Fl_OpenGL_Graphics_Driver *graphics_driver);
public:
  static Fl_OpenGL_Display_Device *display_device();
  static Fl_RGB_Image* capture_gl_rectangle(Fl_Gl_Window*, int, int, int, int);
};

//
// End of "$Id$".
//
