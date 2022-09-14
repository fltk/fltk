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

class Fl_Cocoa_Gl_Window_Driver : public Fl_Gl_Window_Driver {
  friend Fl_Gl_Window_Driver* Fl_Gl_Window_Driver::newGlWindowDriver(Fl_Gl_Window *);
  Fl_Cocoa_Gl_Window_Driver(Fl_Gl_Window *win) : Fl_Gl_Window_Driver(win) {}
  virtual float pixels_per_unit();
  virtual void before_show(int& need_after);
  virtual void after_show();
  virtual int mode_(int m, const int *a);
  virtual void make_current_before();
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
  virtual char *alpha_mask_for_string(const char *str, int n, int w, int h, Fl_Fontsize fs);
  virtual Fl_RGB_Image* capture_gl_rectangle(int x, int y, int w, int h);
  virtual bool need_scissor() { return true; }
  virtual void* GetProcAddress(const char *procName);
  void apply_scissor();
};


#endif // HAVE_GL
