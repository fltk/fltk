//
// Class Fl_X11_Gl_Window_Driver for the Fast Light Tool Kit (FLTK).
//
// Copyright 2022-2025 by Bill Spitzak and others.
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

#ifndef FL_X11_GL_WINDOW_DRIVER_H
#define FL_X11_GL_WINDOW_DRIVER_H

#include <config.h>
#if HAVE_GL
#include <FL/platform.H>
#include "../../Fl_Gl_Window_Driver.H"

class Fl_Gl_Choice;

class Fl_X11_Gl_Window_Driver : public Fl_Gl_Window_Driver {
  friend Fl_Gl_Window_Driver* Fl_Gl_Window_Driver::newGlWindowDriver(Fl_Gl_Window *);
  Fl_X11_Gl_Window_Driver(Fl_Gl_Window *win) : Fl_Gl_Window_Driver(win) {}
  float pixels_per_unit() FL_OVERRIDE;
  void before_show(int& need_after) FL_OVERRIDE;
  int mode_(int m, const int *a) FL_OVERRIDE;
  void swap_buffers() FL_OVERRIDE;
  char swap_type() FL_OVERRIDE;
  void swap_interval(int) FL_OVERRIDE;
  int swap_interval() const FL_OVERRIDE;
  Fl_Gl_Choice *find(int m, const int *alistp) FL_OVERRIDE;
  GLContext create_gl_context(Fl_Window* window, const Fl_Gl_Choice* g) FL_OVERRIDE;
  void set_gl_context(Fl_Window* w, GLContext context) FL_OVERRIDE;
  void delete_gl_context(GLContext) FL_OVERRIDE;
  void make_overlay_current() FL_OVERRIDE;
  void redraw_overlay() FL_OVERRIDE;
  void waitGL() FL_OVERRIDE;
  void gl_visual(Fl_Gl_Choice*) FL_OVERRIDE; // support for Fl::gl_visual()
  void gl_start() FL_OVERRIDE;
  void draw_string_legacy(const char* str, int n) FL_OVERRIDE;
  void gl_bitmap_font(Fl_Font_Descriptor *fl_fontsize) FL_OVERRIDE;
  void get_list(Fl_Font_Descriptor *fd, int r) FL_OVERRIDE;
  int genlistsize() FL_OVERRIDE;
#if !(USE_XFT || FLTK_USE_CAIRO)
  virtual Fl_Font_Descriptor** fontnum_to_fontdescriptor(int fnum) FL_OVERRIDE;
#endif
  //static GLContext create_gl_context(XVisualInfo* vis);
};


#endif // HAVE_GL

#endif // FL_X11_GL_WINDOW_DRIVER_H
