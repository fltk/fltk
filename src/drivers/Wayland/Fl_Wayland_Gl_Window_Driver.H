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

#ifndef FL_WAYLAND_GL_WINDOW_DRIVER_H
#define FL_WAYLAND_GL_WINDOW_DRIVER_H

#include <config.h>
#if HAVE_GL
#include <FL/platform.H>
#include "../../Fl_Gl_Window_Driver.H"
#include <wayland-egl.h>
#include <EGL/egl.h>
#include <FL/gl.h>

/* Implementation note about OpenGL drawing on the Wayland platform

After eglCreateWindowSurface() with attributes {EGL_RENDER_BUFFER, EGL_SINGLE_BUFFER, EGL_NONE},
eglQueryContext() reports that EGL_RENDER_BUFFER equals EGL_BACK_BUFFER.
This experiment suggests that the platform only supports double-buffer drawing.
Consequently, FL_DOUBLE is enforced in all Fl_Gl_Window::mode_ values under Wayland.
*/

class Fl_Wayland_Gl_Window_Driver : public Fl_Gl_Window_Driver {
  friend Fl_Gl_Window_Driver* Fl_Gl_Window_Driver::newGlWindowDriver(Fl_Gl_Window *);
  friend class Fl_Wayland_Gl_Plugin;
  bool egl_swap_in_progress;
  Fl_Wayland_Gl_Window_Driver(Fl_Gl_Window *win);
  virtual float pixels_per_unit();
  virtual void make_current_before();
  virtual int mode_(int m, const int *a);
  virtual void swap_buffers();
  virtual void resize(int is_a_resize, int w, int h);
  virtual char swap_type();
  virtual Fl_Gl_Choice *find(int m, const int *alistp);
  virtual GLContext create_gl_context(Fl_Window* window, const Fl_Gl_Choice* g, int layer = 0);
  virtual void set_gl_context(Fl_Window* w, GLContext context);
  virtual void delete_gl_context(GLContext);
  virtual void make_overlay_current();
  virtual void redraw_overlay();
  virtual void gl_start();
  virtual void gl_visual(Fl_Gl_Choice *c);
  char *alpha_mask_for_string(const char *str, int n, int w, int h, Fl_Fontsize fs);
  static EGLDisplay egl_display;
  static EGLint configs_count;
  static struct wl_event_queue *gl_event_queue;
  void init();
  struct wl_egl_window *egl_window;
  EGLSurface egl_surface;
public:
  //virtual bool need_scissor() { return true; } // CONTROL_LEAKING_SUB_GL_WINDOWS
  //void apply_scissor(); // CONTROL_LEAKING_SUB_GL_WINDOWS
};

#endif // HAVE_GL
#endif // FL_WAYLAND_GL_WINDOW_DRIVER_H
