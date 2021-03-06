//
// OpenGL context routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2020 by Bill Spitzak and others.
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

// You MUST use gl_visual() to select the default visual before doing
// show() of any windows.  Mesa will crash if you try to use a visual
// not returned by glxChooseVisual.

// This does not work with Fl_Double_Window's!  It will try to draw
// into the front buffer.  Depending on the system this will either
// crash or do nothing (when pixmaps are being used as back buffer
// and GL is being done by hardware), work correctly (when GL is done
// with software, such as Mesa), or draw into the front buffer and
// be erased when the buffers are swapped (when double buffer hardware
// is being used)

#include <config.h>
#if HAVE_GL

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>
#include <FL/gl.h>
class Fl_Gl_Choice;
#include <FL/Fl_Gl_Window.H>
#include "Fl_Gl_Window_Driver.H"

/**
 \cond DriverDev
 \addtogroup DriverDeveloper
 \{
 */
GLContext Fl_Gl_Window_Driver::gl_start_context;
/**
 \}
 \endcond
 */

static int clip_state_number=-1;
static int pw, ph;
float gl_start_scale = 1;

static Fl_Gl_Choice* gl_choice = NULL;

/** Creates an OpenGL context */
void gl_start() {
  gl_start_scale = Fl_Display_Device::display_device()->driver()->scale();
  if (!Fl_Gl_Window_Driver::gl_start_context) {
    if (!gl_choice) Fl::gl_visual(0);
    Fl_Gl_Window_Driver::gl_start_context = Fl_Gl_Window_Driver::global()->create_gl_context(Fl_Window::current(), gl_choice);
  }
  Fl_Gl_Window_Driver::global()->set_gl_context(Fl_Window::current(), Fl_Gl_Window_Driver::gl_start_context);
  Fl_Gl_Window_Driver::global()->gl_start();
  if (pw != int(Fl_Window::current()->w() * gl_start_scale) || ph != int(Fl_Window::current()->h() * gl_start_scale)) {
    pw = int(Fl_Window::current()->w() * gl_start_scale);
    ph = int(Fl_Window::current()->h() * gl_start_scale);
    glLoadIdentity();
    glViewport(0, 0, pw, ph);
    glOrtho(0, Fl_Window::current()->w(), 0, Fl_Window::current()->h(), -1, 1);
    glDrawBuffer(GL_FRONT);
  }
  if (clip_state_number != fl_graphics_driver->fl_clip_state_number) {
    clip_state_number = fl_graphics_driver->fl_clip_state_number;
    int x = 0, y = 0, w = 0, h = 0;
    if (fl_clip_box(0, 0, Fl_Window::current()->w(), Fl_Window::current()->h(),
                    x, y, w, h)) {
      fl_clip_region(Fl_Graphics_Driver::default_driver().XRectangleRegion(x,y,w,h));
      glScissor(int(x*gl_start_scale), int((Fl_Window::current()->h()-(y+h))*gl_start_scale), int(w*gl_start_scale), int(h*gl_start_scale));
      glEnable(GL_SCISSOR_TEST);
    } else {
      glDisable(GL_SCISSOR_TEST);
    }
  }
  Fl_Display_Device::display_device()->driver()->scale(1);
}

/** Releases an OpenGL context */
void gl_finish() {
  glFlush();
  Fl_Gl_Window_Driver::global()->waitGL();
  Fl_Display_Device::display_device()->driver()->scale(gl_start_scale);
  gl_start_scale = 1;
}

/**
 \cond DriverDev
 \addtogroup DriverDeveloper
 \{
 */

void Fl_Gl_Window_Driver::gl_visual(Fl_Gl_Choice *c) {
  gl_choice = c;
}

int Fl::gl_visual(int mode, int *alist) {
  Fl_Gl_Choice *c = Fl_Gl_Window_Driver::global()->find(mode,alist);
  if (!c) return 0;
  Fl_Gl_Window_Driver::global()->gl_visual(c);
  return 1;
}

/**
 \}
 \endcond
 */

#endif // HAVE_GL
