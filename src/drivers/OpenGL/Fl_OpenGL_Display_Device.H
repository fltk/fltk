//
// Definition of class Fl_OpenGL_Display_Device for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2022 by Bill Spitzak and others.
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

#include <FL/Fl_Device.H>

class Fl_OpenGL_Graphics_Driver;

/**
 OpenGL Surface.
 This surface is needed as an interface between GL windows and the GL graphics driver.
 */
class Fl_OpenGL_Display_Device : public Fl_Surface_Device {
  Fl_OpenGL_Display_Device(Fl_OpenGL_Graphics_Driver *graphics_driver);
public:
  static Fl_OpenGL_Display_Device *display_device();
};
