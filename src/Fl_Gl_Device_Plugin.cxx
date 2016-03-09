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

#include "config_lib.h"
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/Fl_Shared_Image.H>
#include "drivers/OpenGL/Fl_OpenGL_Display_Device.H"


/**
 This class will make sure that OpenGL printing/screen capture is available if fltk_gl
 was linked to the program
 */
class Fl_Gl_Device_Plugin : public Fl_Device_Plugin {
public:
  Fl_Gl_Device_Plugin() : Fl_Device_Plugin(name()) { }
  virtual const char *name() { return "opengl.device.fltk.org"; }
  virtual int print(Fl_Widget *w, int x, int y, int height /*useless*/) {
    Fl_Gl_Window *glw = w->as_gl_window();
    if (!glw) return 0;
    Fl_RGB_Image *img = Fl_OpenGL_Display_Device::capture_gl_rectangle(glw, 0, 0, glw->w(), glw->h());
    Fl_Shared_Image *shared = Fl_Shared_Image::get(img);
    shared->scale(glw->w(), glw->h());
    shared->draw(x, y);
    shared->release();
    return 1;
  }
  virtual Fl_RGB_Image* rectangle_capture(Fl_Widget *widget, int x, int y, int w, int h) {
    Fl_Gl_Window *glw = widget->as_gl_window();
    if (!glw) return NULL;
    return Fl_OpenGL_Display_Device::capture_gl_rectangle(glw, x, y, w, h);
  }
};

static Fl_Gl_Device_Plugin Gl_Device_Plugin;

// The purpose of this variable, used in Fl_Gl_Window.cxx, is only to force this file to be loaded
// whenever Fl_Gl_Window.cxx is loaded, that is, whenever fltk_gl is.
FL_EXPORT int fl_gl_load_plugin = 0;

//
// End of "$Id$".
//
