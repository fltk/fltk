//
// Class Fl_X11_Gl_Window_Driver for the Fast Light Tool Kit (FLTK).
//
// Copyright 2022 by Bill Spitzak and others.
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
  float pixels_per_unit() override;
  void before_show(int& need_after) override;
  int mode_(int m, const int *a) override;
  void swap_buffers() override;
  char swap_type() override;
  Fl_Gl_Choice *find(int m, const int *alistp) override;
  GLContext create_gl_context(Fl_Window* window, const Fl_Gl_Choice* g) override;
  void set_gl_context(Fl_Window* w, GLContext context) override;
  void delete_gl_context(GLContext) override;
  void make_overlay_current() override;
  void redraw_overlay() override;
  void waitGL() override;
  void gl_visual(Fl_Gl_Choice*) override; // support for Fl::gl_visual()
  void gl_start() override;
  void draw_string_legacy(const char* str, int n) override;
  void gl_bitmap_font(Fl_Font_Descriptor *fl_fontsize) override;
  void get_list(Fl_Font_Descriptor *fd, int r) override;
  int genlistsize() override;
#if !USE_XFT
  virtual Fl_Font_Descriptor** fontnum_to_fontdescriptor(int fnum);
#endif
  //static GLContext create_gl_context(XVisualInfo* vis);
};


#endif // HAVE_GL

#endif // FL_X11_GL_WINDOW_DRIVER_H
