//
// Class Fl_Cocoa_Gl_Window_Driver for the Fast Light Tool Kit (FLTK).
//
// Copyright 2021 by Bill Spitzak and others.
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
#if HAVE_GL
#include <FL/platform.H>
#include "../../Fl_Gl_Window_Driver.H"

class Fl_Gl_Choice;
#ifdef __OBJC__
  @class NSOpenGLContext;
#else
  class NSOpenGLContext;
#endif

class Fl_Cocoa_Gl_Window_Driver : public Fl_Gl_Window_Driver {
  NSOpenGLContext *gl1ctxt; // GL1 context in addition to GL3 context
  friend Fl_Gl_Window_Driver* Fl_Gl_Window_Driver::newGlWindowDriver(Fl_Gl_Window *);
  Fl_Cocoa_Gl_Window_Driver(Fl_Gl_Window *win);
  float pixels_per_unit() FL_OVERRIDE;
  void before_show(int& need_after) FL_OVERRIDE;
  void after_show() FL_OVERRIDE;
  int mode_(int m, const int *a) FL_OVERRIDE;
  void make_current_before() FL_OVERRIDE;
  void swap_buffers() FL_OVERRIDE;
  void resize(int is_a_resize, int w, int h) FL_OVERRIDE;
  char swap_type() FL_OVERRIDE;
  void swap_interval(int) FL_OVERRIDE;
  int swap_interval() const FL_OVERRIDE;
  Fl_Gl_Choice *find(int m, const int *alistp) FL_OVERRIDE;
  GLContext create_gl_context(Fl_Window* window, const Fl_Gl_Choice* g) FL_OVERRIDE;
  void set_gl_context(Fl_Window* w, GLContext context) FL_OVERRIDE;
  void delete_gl_context(GLContext) FL_OVERRIDE;
  void make_overlay_current() FL_OVERRIDE;
  void redraw_overlay() FL_OVERRIDE;
  void gl_start() FL_OVERRIDE;
  char *alpha_mask_for_string(const char *str, int n, int w, int h, Fl_Fontsize fs) FL_OVERRIDE;
  Fl_RGB_Image* capture_gl_rectangle(int x, int y, int w, int h) FL_OVERRIDE;
  bool need_scissor() FL_OVERRIDE { return true; }
  void* GetProcAddress(const char *procName) FL_OVERRIDE;
  void apply_scissor();
  void switch_to_GL1() FL_OVERRIDE;
  void switch_back() FL_OVERRIDE;
};


#endif // HAVE_GL
