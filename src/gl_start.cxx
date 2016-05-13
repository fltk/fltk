//
// "$Id$"
//
// OpenGL context routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2016 by Bill Spitzak and others.
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

#include "config_lib.h"
#if HAVE_GL

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>
#include <FL/gl.h>
class Fl_Gl_Choice;
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Gl_Window_Driver.H>

static GLContext context;
static int clip_state_number=-1;
static int pw, ph;

static Fl_Gl_Choice* gl_choice;

/** Creates an OpenGL context */
void gl_start() {
  if (!context) {
    if (!gl_choice) Fl::gl_visual(0);
    context = Fl_Gl_Window_Driver::global()->create_gl_context(Fl_Window::current(), gl_choice);
  }
  Fl_Gl_Window_Driver::global()->set_gl_context(Fl_Window::current(), context);
  Fl_Gl_Window_Driver::global()->gl_start();
  if (pw != Fl_Window::current()->w() || ph != Fl_Window::current()->h()) {
    pw = Fl_Window::current()->w();
    ph = Fl_Window::current()->h();
    glLoadIdentity();
    glViewport(0, 0, pw, ph);
    glOrtho(0, pw, 0, ph, -1, 1);
    glDrawBuffer(GL_FRONT);
  }
  if (clip_state_number != fl_graphics_driver->fl_clip_state_number) {
    clip_state_number = fl_graphics_driver->fl_clip_state_number;
    int x, y, w, h;
    if (fl_clip_box(0, 0, Fl_Window::current()->w(), Fl_Window::current()->h(),
		    x, y, w, h)) {
      fl_clip_region(Fl_Graphics_Driver::default_driver().XRectangleRegion(x,y,w,h));
      glScissor(x, Fl_Window::current()->h()-(y+h), w, h);
      glEnable(GL_SCISSOR_TEST);
    } else {
      glDisable(GL_SCISSOR_TEST);
    }
  }
}

/** Releases an OpenGL context */
void gl_finish() {
  glFlush();
  Fl_Gl_Window_Driver::global()->waitGL();
}

void Fl_Gl_Window_Driver::gl_visual(Fl_Gl_Choice *c) {
  gl_choice = c;
}

#ifdef FL_CFG_GFX_QUARTZ

void Fl_Cocoa_Gl_Window_Driver::gl_start() {
  GLcontext_update(context); // supports window resizing
}

#endif


#ifdef FL_CFG_GFX_XLIB
#include <FL/x.H>
#include "Fl_Gl_Choice.H"

void Fl_X11_Gl_Window_Driver::gl_visual(Fl_Gl_Choice *c) {
  Fl_Gl_Window_Driver::gl_visual(c);
  fl_visual = c->vis;
  fl_colormap = c->colormap;
}

void Fl_X11_Gl_Window_Driver::gl_start() {
  glXWaitX();
}

#endif // FL_CFG_GFX_XLIB

int Fl::gl_visual(int mode, int *alist) {
  Fl_Gl_Choice *c = Fl_Gl_Window_Driver::global()->find(mode,alist);
  if (!c) return 0;
  Fl_Gl_Window_Driver::global()->gl_visual(c);
  return 1;
}

#endif // HAVE_GL

//
// End of "$Id$".
//
