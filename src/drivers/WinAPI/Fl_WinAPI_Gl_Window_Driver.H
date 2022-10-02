//
// Class Fl_Wayland_Gl_Window_Driver for the Fast Light Tool Kit (FLTK).
//
// Copyright 2021-2022 by Bill Spitzak and others.
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

#ifndef FL_WINAPI_GL_WINDOW_DRIVER_H
#define FL_WINAPI_GL_WINDOW_DRIVER_H

#include <config.h>
#if HAVE_GL
#include <FL/platform.H>
#include "../../Fl_Gl_Window_Driver.H"
#include <FL/gl.h>

/* Implementation note about OpenGL drawing on the Wayland platform

After eglCreateWindowSurface() with attributes {EGL_RENDER_BUFFER, EGL_SINGLE_BUFFER, EGL_NONE},
eglQueryContext() reports that EGL_RENDER_BUFFER equals EGL_BACK_BUFFER.
This experiment suggests that the platform only supports double-buffer drawing.
Consequently, FL_DOUBLE is enforced in all Fl_Gl_Window::mode_ values under Wayland.
*/

class Fl_WinAPI_Gl_Window_Driver : public Fl_Gl_Window_Driver {
  friend Fl_Gl_Window_Driver* Fl_Gl_Window_Driver::newGlWindowDriver(Fl_Gl_Window *);
  Fl_WinAPI_Gl_Window_Driver(Fl_Gl_Window *win) : Fl_Gl_Window_Driver(win) {}
  virtual float pixels_per_unit();
  virtual int mode_(int m, const int *a);
  virtual void make_current_after();
  virtual void swap_buffers();
  virtual void invalidate() {}
  virtual int flush_begin(char& valid_f);
  virtual Fl_Gl_Choice *find(int m, const int *alistp);
  virtual GLContext create_gl_context(Fl_Window* window, const Fl_Gl_Choice* g);
  GLContext do_create_gl_context(Fl_Window* window, const Fl_Gl_Choice* g,
                                 int layer);
  virtual void set_gl_context(Fl_Window* w, GLContext context);
  virtual void delete_gl_context(GLContext);
  virtual void make_overlay_current();
  virtual void redraw_overlay();
  virtual void* GetProcAddress(const char *procName);
  virtual void draw_string_legacy(const char* str, int n);
  virtual void gl_bitmap_font(Fl_Font_Descriptor *fl_fontsize);
  virtual void get_list(Fl_Font_Descriptor *fd, int r);
  virtual int genlistsize();
  virtual void switch_to_GL1();
  virtual void switch_back();
#if HAVE_GL_OVERLAY
  virtual void gl_hide_before(void *& overlay);
  virtual int can_do_overlay();
  virtual int overlay_color(Fl_Color i);
  void make_overlay(void*&overlay);
#endif
};

#endif // HAVE_GL

#endif // FL_WINAPI_GL_WINDOW_DRIVER_H
