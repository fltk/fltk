//
// implementation of class Fl_Gl_Device_Plugin for the Fast Light Tool Kit (FLTK).
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

#include <config.h>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/Fl_Device.H>
#include "Fl_Gl_Window_Driver.H"


/**
 This class will make sure that OpenGL printing/screen capture is available if fltk_gl
 was linked to the program
 */
class Fl_Gl_Device_Plugin : public Fl_Device_Plugin {
public:
  Fl_Gl_Device_Plugin() : Fl_Device_Plugin(name()) { }
  const char *name() FL_OVERRIDE { return "opengl.device.fltk.org"; }
  int print(Fl_Widget *w) FL_OVERRIDE {
    Fl_Gl_Window *glw = w->as_gl_window();
    if (!glw) return 0;
    Fl_RGB_Image *img =  Fl_Gl_Window_Driver::driver(glw)->capture_gl_rectangle(0, 0, glw->w(), glw->h());
    img->scale(glw->w(), glw->h());
    img->draw(0, 0);
    delete img;
    return 1;
  }
  Fl_RGB_Image* rectangle_capture(Fl_Widget *widget, int x, int y, int w, int h) FL_OVERRIDE {
    Fl_Gl_Window *glw = widget->as_gl_window();
    if (!glw) return NULL;
    return Fl_Gl_Window_Driver::driver(glw)->capture_gl_rectangle(x, y, w, h);
  }
};

static Fl_Gl_Device_Plugin Gl_Device_Plugin;

// The purpose of this variable, used in Fl_Gl_Window.cxx, is only to force this file to be loaded
// whenever Fl_Gl_Window.cxx is loaded, that is, whenever fltk_gl is.
FL_EXPORT int fl_gl_load_plugin = 0;
