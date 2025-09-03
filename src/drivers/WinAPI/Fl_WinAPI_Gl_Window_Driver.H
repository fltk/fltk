//
// Class Fl_WinAPI_Gl_Window_Driver for the Fast Light Tool Kit (FLTK).
//
// Copyright 2021-2023 by Bill Spitzak and others.
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

class Fl_WinAPI_Gl_Window_Driver : public Fl_Gl_Window_Driver {
  friend Fl_Gl_Window_Driver* Fl_Gl_Window_Driver::newGlWindowDriver(Fl_Gl_Window *);
  Fl_WinAPI_Gl_Window_Driver(Fl_Gl_Window *win) : Fl_Gl_Window_Driver(win) {}
  float pixels_per_unit() FL_OVERRIDE;
  int mode_(int m, const int *a) FL_OVERRIDE;
  void make_current_after() FL_OVERRIDE;
  void swap_buffers() FL_OVERRIDE;
  void swap_interval(int) FL_OVERRIDE;
  int swap_interval() const FL_OVERRIDE;
  void invalidate() FL_OVERRIDE {}
  int flush_begin(char& valid_f) FL_OVERRIDE;
  Fl_Gl_Choice *find(int m, const int *alistp) FL_OVERRIDE;
  GLContext create_gl_context(Fl_Window* window, const Fl_Gl_Choice* g) FL_OVERRIDE;
  GLContext do_create_gl_context(Fl_Window* window, const Fl_Gl_Choice* g,
                                 int layer);
  void set_gl_context(Fl_Window* w, GLContext context) FL_OVERRIDE;
  void delete_gl_context(GLContext) FL_OVERRIDE;
  void make_overlay_current() FL_OVERRIDE;
  void redraw_overlay() FL_OVERRIDE;
  void* GetProcAddress(const char *procName) FL_OVERRIDE;
  void draw_string_legacy(const char* str, int n) FL_OVERRIDE;
  void gl_bitmap_font(Fl_Font_Descriptor *fl_fontsize) FL_OVERRIDE;
  void get_list(Fl_Font_Descriptor *fd, int r) FL_OVERRIDE;
  int genlistsize() FL_OVERRIDE;
  void switch_to_GL1() FL_OVERRIDE;
  void switch_back() FL_OVERRIDE;
#if HAVE_GL_OVERLAY
  void gl_hide_before(void *& overlay) FL_OVERRIDE;
  int can_do_overlay() FL_OVERRIDE;
  int overlay_color(Fl_Color i) FL_OVERRIDE;
  void make_overlay(void*&overlay) FL_OVERRIDE;
#endif
};

#endif // HAVE_GL

#endif // FL_WINAPI_GL_WINDOW_DRIVER_H
