//
// Definition of class Fl_Gl_Window_Driver, and of its platform-specific derived classes
// for the Fast Light Tool Kit (FLTK).
//
// Copyright 2016-2018 by Bill Spitzak and others.
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

/**
 \cond DriverDev
 \addtogroup DriverDeveloper
 \{
 */

#ifndef Fl_Gl_Window_Driver_H
#define Fl_Gl_Window_Driver_H

#include <FL/Fl_Gl_Window.H>

class Fl_Gl_Choice;
class Fl_Font_Descriptor;

/* The constructor of each Fl_Gl_Window object creates also an object from a
 platform-specific derived class from this class.
 */
class Fl_Gl_Window_Driver {
protected:
  Fl_Gl_Window *pWindow;
public:
  static GLContext cached_context;
  static Fl_Window* cached_window;
  static int nContext;
  static GLContext *context_list;
  static Fl_Gl_Choice *first;
  static int copy;
  static float gl_scale;
  static GLContext gl_start_context;
  Fl_Gl_Choice* g() {return pWindow->g;}
  void g(Fl_Gl_Choice *c) {pWindow->g = c;}
  int mode() {return pWindow->mode_;}
  void mode(int m) { pWindow->mode_ = m;}
  const int *alist() {return pWindow->alist;}
  void alist(const int *l) { pWindow->alist = l;}
  void* overlay() {return pWindow->overlay;}
  void draw_overlay() {pWindow->draw_overlay();}

  Fl_Gl_Window_Driver(Fl_Gl_Window *win) : pWindow(win) {}
  virtual ~Fl_Gl_Window_Driver() {}
  static Fl_Gl_Window_Driver *newGlWindowDriver(Fl_Gl_Window *w);
  static Fl_Gl_Window_Driver *global();
  virtual float pixels_per_unit() {return 1;}
  virtual void before_show(int& need_after) {}
  virtual void after_show() {}
  virtual void invalidate();
  virtual int mode_(int m, const int *a) {return 0;}
  virtual void make_current_before() {}
  virtual void make_current_after() {}
  virtual void swap_buffers() {}
  virtual void resize(int is_a_resize, int w, int h) {}
  virtual char swap_type();
  virtual int flush_begin(char& valid_f) {return 0;}
  virtual void gl_hide_before(void *& overlay) {} // the default implementation may be enough
  static Fl_Gl_Choice *find_begin(int m, const int *alistp);
  static void add_context(GLContext ctx);
  static void del_context(GLContext ctx);
  // Return one of these structures for a given gl mode.
  // The second argument is a glX attribute list, and is used if mode is zero.
  // This is not supported on Win32:
  virtual Fl_Gl_Choice *find(int mode, const int *alistp) {return NULL;}
  virtual GLContext create_gl_context(Fl_Window* window, const Fl_Gl_Choice* g, int layer = 0) {return 0;}
  virtual void set_gl_context(Fl_Window* w, GLContext context) {}
  virtual void delete_gl_context(GLContext) {}
  virtual void make_overlay(void* &o);
  virtual void hide_overlay() {} // the default implementation may be enough
  virtual void make_overlay_current() {}
  virtual void redraw_overlay() {}
  virtual int can_do_overlay() {return 0;}
  virtual void waitGL() {} // support for gl_finish() function
  virtual void gl_visual(Fl_Gl_Choice*); // support for Fl::gl_visual() function
  virtual void gl_start() {} // support for gl_start() function
  virtual void* GetProcAddress(const char *procName); // support for glutGetProcAddress()
  virtual void draw_string_legacy(const char* str, int n); // support for gl_draw()
  void draw_string_legacy_get_list(const char* str, int n); // support for gl_draw()
  static void draw_string_legacy_glut(const char* str, int n); // support for gl_draw()
  virtual void get_list(Fl_Font_Descriptor* fd, int r) {} // support for gl_draw() without textures
  virtual void gl_bitmap_font(Fl_Font_Descriptor *fl_fontsize) {} // support for gl_font() without textures
  virtual int overlay_color(Fl_Color i) {return 0;} // support for gl_color() with HAVE_GL_OVERLAY
  static void draw_string_with_texture(const char* str, int n); // cross-platform
  // support for gl_draw(). The cross-platform version may be enough.
  virtual char *alpha_mask_for_string(const char *str, int n, int w, int h);
  virtual int genlistsize() { return 0; } // support for gl_draw()
  virtual Fl_Font_Descriptor** fontnum_to_fontdescriptor(int fnum);
};

#endif /* Fl_Gl_Window_Driver_H */

/**
 \}
 \endcond
 */
