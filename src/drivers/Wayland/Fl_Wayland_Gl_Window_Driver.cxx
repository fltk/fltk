//
// Class Fl_Wayland_Gl_Window_Driver for the Fast Light Tool Kit (FLTK).
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

#include <config.h>
#if HAVE_GL
#include <FL/platform.H>
#include <FL/Fl_Image_Surface.H>
#include "../../Fl_Gl_Choice.H"
#include "Fl_Wayland_Window_Driver.H"
#include "Fl_Wayland_Graphics_Driver.H"
#include "Fl_Wayland_Gl_Window_Driver.H"
#ifdef FLTK_USE_X11
#  include "../X11/Fl_X11_Gl_Window_Driver.H"
#endif
#include <wayland-egl.h>
#include <EGL/egl.h>
#include <FL/gl.h>

/* Implementation notes about OpenGL drawing on the Wayland platform

* After eglCreateWindowSurface() with attributes {EGL_RENDER_BUFFER, EGL_SINGLE_BUFFER, EGL_NONE},
eglQueryContext() reports that EGL_RENDER_BUFFER equals EGL_BACK_BUFFER.
This experiment suggests that the platform only supports double-buffer drawing.
Consequently, FL_DOUBLE is enforced in all Fl_Gl_Window::mode_ values under Wayland.

* Commented out code marked with CONTROL_LEAKING_SUB_GL_WINDOWS aims to prevent
 sub GL windows from leaking out from their parent by making leaking parts fully transparent.
 This code is commented out because it requires the FL_ALPHA flag to be on
 which not all client applications do.
*/

// Describes crap needed to create a GLContext.
class Fl_Wayland_Gl_Choice : public Fl_Gl_Choice {
  friend class Fl_Wayland_Gl_Window_Driver;
private:
  EGLConfig egl_conf;
public:
  Fl_Wayland_Gl_Choice(int m, const int *alistp, Fl_Gl_Choice *n) : Fl_Gl_Choice(m, alistp, n) {
  egl_conf = 0;
  }
};


struct gl_start_support { // to support use of gl_start / gl_finish
  struct wl_surface *surface;
  struct wl_subsurface *subsurface;
  struct wl_egl_window *egl_window;
  EGLSurface egl_surface;
};


static EGLConfig wld_egl_conf = NULL;
static EGLint swap_interval_ = 1;
static EGLint max_swap_interval = 1000;
static EGLint min_swap_interval = 0;


EGLDisplay Fl_Wayland_Gl_Window_Driver::egl_display = EGL_NO_DISPLAY;


Fl_Wayland_Gl_Window_Driver::Fl_Wayland_Gl_Window_Driver(Fl_Gl_Window *win) :
    Fl_Gl_Window_Driver(win) {
  if (egl_display == EGL_NO_DISPLAY) init();
  egl_window = NULL;
  egl_surface = NULL;
}


Fl_Gl_Window_Driver *Fl_Gl_Window_Driver::newGlWindowDriver(Fl_Gl_Window *w)
{
#ifdef FLTK_USE_X11
  if (!Fl_Wayland_Screen_Driver::wl_display) return new Fl_X11_Gl_Window_Driver(w);
#endif
  return new Fl_Wayland_Gl_Window_Driver(w);
}


void Fl_Wayland_Gl_Window_Driver::init() {
  EGLint major, minor;

  if (!fl_wl_display()) fl_open_display();
  egl_display = eglGetDisplay((EGLNativeDisplayType) fl_wl_display());
  if (egl_display == EGL_NO_DISPLAY) {
    Fl::fatal("Can't create egl display\n");
  }

  if (eglInitialize(egl_display, &major, &minor) != EGL_TRUE) {
    Fl::fatal("Can't initialise egl display\n");
  }
  //printf("EGL major: %d, minor %d\n", major, minor);
  //eglGetConfigs(egl_display, NULL, 0, &configs_count);
  //printf("EGL has %d configs\n", configs_count);
  eglBindAPI(EGL_OPENGL_API);
}


Fl_Gl_Choice *Fl_Wayland_Gl_Window_Driver::find(int m, const int *alistp)
{
  m |= FL_DOUBLE;
  //if (pWindow->parent()) m |= FL_ALPHA; // CONTROL_LEAKING_SUB_GL_WINDOWS
  Fl_Wayland_Gl_Choice *g = (Fl_Wayland_Gl_Choice*)Fl_Gl_Window_Driver::find_begin(
      m, alistp);
  if (g) return g;

  EGLint n;
  EGLint config_attribs[] = {
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
    EGL_DEPTH_SIZE, 0, // set at 11
    EGL_SAMPLE_BUFFERS, 0,  // set at 13
    EGL_STENCIL_SIZE, 0, // set at 15
    EGL_ALPHA_SIZE, 0, // set at 17
    EGL_NONE
  };

  if (m & FL_DEPTH) config_attribs[11] = 1;
  if (m & FL_MULTISAMPLE) config_attribs[13] = 1;
  if (m & FL_STENCIL) config_attribs[15] = 1;
  if (m & FL_ALPHA) config_attribs[17] = (m & FL_RGB8) ? 8 : 1;

  g = new Fl_Wayland_Gl_Choice(m, alistp, first);
  eglChooseConfig(egl_display, config_attribs, &(g->egl_conf), 1, &n);
  if (n == 0 && (m & FL_MULTISAMPLE)) {
    config_attribs[13] = 0;
    eglChooseConfig(egl_display, config_attribs, &(g->egl_conf), 1, &n);
  }
  if (n == 0) {
    Fl::fatal("failed to choose an EGL config\n");
  }

  eglGetConfigAttrib(egl_display, g->egl_conf, EGL_MAX_SWAP_INTERVAL, &max_swap_interval);
  eglGetConfigAttrib(egl_display, g->egl_conf, EGL_MIN_SWAP_INTERVAL, &min_swap_interval);

  first = g;
  return g;
}


GLContext Fl_Wayland_Gl_Window_Driver::create_gl_context(Fl_Window* window,
                                                         const Fl_Gl_Choice* g) {
  GLContext shared_ctx = 0;
  if (context_list && nContext) shared_ctx = context_list[0];

  static const EGLint context_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
  GLContext ctx = (GLContext)eglCreateContext(egl_display,
    ((Fl_Wayland_Gl_Choice*)g)->egl_conf,
    (shared_ctx ? (EGLContext)shared_ctx : EGL_NO_CONTEXT),
    context_attribs);
//fprintf(stderr, "eglCreateContext=%p shared_ctx=%p\n", ctx, shared_ctx);
  if (ctx) {
    add_context(ctx);
    /* CONTROL_LEAKING_SUB_GL_WINDOWS
    if (egl_surface) {
      eglMakeCurrent(egl_display, egl_surface, egl_surface, (EGLContext)ctx);
      glClearColor(0., 0., 0., 1.); // set opaque black as starting background color
      apply_scissor();
    }*/
  }
  return ctx;
}


void Fl_Wayland_Gl_Window_Driver::set_gl_context(Fl_Window* w, GLContext context) {
  struct wld_window *win = fl_wl_xid(w);
  if (!win) return;
  Fl_Wayland_Window_Driver *dr = Fl_Wayland_Window_Driver::driver(w);
  EGLSurface target_egl_surface = NULL;
  if (egl_surface) target_egl_surface = egl_surface;
  else if (dr->gl_start_support_) target_egl_surface = dr->gl_start_support_->egl_surface;
  if (!target_egl_surface) { // useful for gl_start()
    dr->gl_start_support_ = new struct gl_start_support;
    float s = Fl::screen_scale(w->screen_num());
    Fl_Wayland_Screen_Driver *scr_driver = (Fl_Wayland_Screen_Driver*)Fl::screen_driver();
    // the GL scene will be a transparent subsurface above the cairo-drawn surface
    dr->gl_start_support_->surface =
          wl_compositor_create_surface(scr_driver->wl_compositor);
    dr->gl_start_support_->subsurface = wl_subcompositor_get_subsurface(
          scr_driver->wl_subcompositor, dr->gl_start_support_->surface, win->wl_surface);
    wl_subsurface_set_position(dr->gl_start_support_->subsurface, w->x() * s, w->y() * s);
    wl_subsurface_place_above(dr->gl_start_support_->subsurface, win->wl_surface);
    dr->gl_start_support_->egl_window = wl_egl_window_create(
          dr->gl_start_support_->surface, w->w() * s, w->h() * s);
    target_egl_surface = dr->gl_start_support_->egl_surface = eglCreateWindowSurface(
        egl_display, wld_egl_conf, dr->gl_start_support_->egl_window, NULL);
  }
  GLContext current_context = eglGetCurrentContext();
  if (context != current_context || w != cached_window) {
    cached_window = w;
    if (eglMakeCurrent(egl_display, target_egl_surface, target_egl_surface,
                       (EGLContext)context)) {
//fprintf(stderr, "EGLContext %p made current\n", context);
    } else {
      Fl::error("eglMakeCurrent() failed\n");
    }
  }
  if (!(mode() & FL_ALPHA)) { // useful at least for Linux on MacBook hardware
    GLfloat vals[4];
    glGetFloatv(GL_COLOR_CLEAR_VALUE, vals);
    if (vals[3] == 0.) glClearColor(vals[0], vals[1], vals[2], 1.);
  }
}

/* CONTROL_LEAKING_SUB_GL_WINDOWS
void Fl_Wayland_Gl_Window_Driver::apply_scissor() {
  cairo_rectangle_int_t *extents = Fl_Wayland_Window_Driver::driver(pWindow)->subRect();
  if (extents) {
    glDisable(GL_SCISSOR_TEST);
    GLdouble vals[4];
    glGetDoublev(GL_COLOR_CLEAR_VALUE, vals);
    glClearColor(0., 0., 0., 0.);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(vals[0], vals[1], vals[2], vals[3]);
    float s = pWindow->pixels_per_unit();
    glScissor(s*extents->x, s*extents->y, s*extents->width, s*extents->height);
//printf("apply_scissor %dx%d %dx%d\n",extents->x, extents->y, extents->width, extents->height);
    glEnable(GL_SCISSOR_TEST);
  }
}*/


void Fl_Wayland_Gl_Window_Driver::delete_gl_context(GLContext context) {
  GLContext current_context = eglGetCurrentContext();
  if (current_context == context) {
    cached_window = 0;
  }
  if (current_context == (EGLContext)context) {
    eglMakeCurrent(egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
  }
  eglDestroyContext(egl_display, (EGLContext)context);
  eglDestroySurface(egl_display, egl_surface);
  egl_surface = NULL;
  wl_egl_window_destroy(egl_window);
  egl_window = NULL;
  del_context(context);
}


void Fl_Wayland_Gl_Window_Driver::make_overlay_current() {
  glDrawBuffer(GL_FRONT);
}


void Fl_Wayland_Gl_Window_Driver::redraw_overlay() {
  pWindow->redraw();
}


void Fl_Wayland_Gl_Window_Driver::make_current_before() {
  if (!egl_window) {
    struct wld_window *win = fl_wl_xid(pWindow);
    struct wl_surface *surface = win->wl_surface;
    int W = pWindow->pixel_w();
    int H = pWindow->pixel_h();
    int scale = Fl_Wayland_Window_Driver::driver(pWindow)->wld_scale();
    egl_window = wl_egl_window_create(surface, (W/scale)*scale, (H/scale)*scale);
    if (egl_window == EGL_NO_SURFACE) {
      Fl::fatal("Can't create egl window with wl_egl_window_create()\n");
    }
    Fl_Wayland_Gl_Choice *g = (Fl_Wayland_Gl_Choice*)this->g();
    egl_surface = eglCreateWindowSurface(egl_display, g->egl_conf, egl_window, NULL);
    wl_surface_set_buffer_scale(surface, scale);
    if (mode() & FL_ALPHA) wl_surface_set_opaque_region(surface, NULL);
    // Tested apps: shape, glpuzzle, cube, fractals, gl_overlay, fullscreen, unittests,
    //   OpenGL3-glut-test, OpenGL3test.
    // Tested wayland compositors: mutter, kde-plasma, weston, sway on FreeBSD.
    if (pWindow->parent()) win = fl_wl_xid(pWindow->top_window());
    while (wl_list_empty(&win->outputs)) wl_display_dispatch(fl_wl_display());
  }
}


float Fl_Wayland_Gl_Window_Driver::pixels_per_unit()
{
  int ns = pWindow->screen_num();
  int wld_scale = (pWindow->shown() ?
    Fl_Wayland_Window_Driver::driver(pWindow)->wld_scale() : 1);
  return wld_scale * Fl::screen_driver()->scale(ns);
}


int Fl_Wayland_Gl_Window_Driver::mode_(int m, const int *a) {
  mode(m | FL_DOUBLE);
  return 1;
}


void Fl_Wayland_Gl_Window_Driver::swap_buffers() {
  if (overlay()) {
    static bool overlay_buffer = true;
    int wo = pWindow->pixel_w(), ho = pWindow->pixel_h();
    GLint matrixmode;
    GLfloat pos[4];
    glGetIntegerv(GL_MATRIX_MODE, &matrixmode);
    glGetFloatv(GL_CURRENT_RASTER_POSITION, pos);       // save original glRasterPos
    glMatrixMode(GL_PROJECTION);                        // save proj/model matrices
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glScalef(2.0f/wo, 2.0f/ho, 1.0f);
    glTranslatef(-wo/2.0f, -ho/2.0f, 0.0f); // set transform so 0,0 is bottom/left of window
    glRasterPos2i(0,0);                             // set glRasterPos to bottom left corner
    {
      // Emulate overlay by doing copypixels
      glReadBuffer(overlay_buffer?GL_BACK:GL_FRONT);
      glDrawBuffer(overlay_buffer?GL_FRONT:GL_BACK);
      overlay_buffer = ! overlay_buffer;
      glCopyPixels(0, 0, wo, ho, GL_COLOR);
    }
    glPopMatrix(); // GL_MODELVIEW                  // restore model/proj matrices
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(matrixmode);
    glRasterPos3f(pos[0], pos[1], pos[2]);              // restore original glRasterPos
    if (!overlay_buffer) return; // don't call eglSwapBuffers until overlay has been drawn
  }

  if (egl_surface) {
    if (pWindow->parent()) { // issue #967
      struct wld_window *xid = fl_wl_xid(pWindow);
      if (xid->frame_cb) return;
      xid->frame_cb = wl_surface_frame(xid->wl_surface);
      wl_callback_add_listener(xid->frame_cb, Fl_Wayland_Graphics_Driver::p_surface_frame_listener,
                               xid);
    }
    eglSwapBuffers(Fl_Wayland_Gl_Window_Driver::egl_display, egl_surface);
  }
}


class Fl_Wayland_Gl_Plugin : public Fl_Wayland_Plugin {
public:
  Fl_Wayland_Gl_Plugin() : Fl_Wayland_Plugin(name()) { }
  const char *name() FL_OVERRIDE { return "gl.wayland.fltk.org"; }
  void do_swap(Fl_Window *w) FL_OVERRIDE {
    Fl_Gl_Window_Driver *gldr = Fl_Gl_Window_Driver::driver(w->as_gl_window());
    if (gldr->overlay() == w) gldr->swap_buffers();
  }
  void invalidate(Fl_Window *w) FL_OVERRIDE {
    w->as_gl_window()->valid(0);
  }
  void terminate() FL_OVERRIDE {
    if (Fl_Wayland_Gl_Window_Driver::egl_display != EGL_NO_DISPLAY) {
      eglTerminate(Fl_Wayland_Gl_Window_Driver::egl_display);
    }
  }
  void destroy(struct gl_start_support *gl_start_support_) FL_OVERRIDE {
    eglDestroySurface(Fl_Wayland_Gl_Window_Driver::egl_display,
                      gl_start_support_->egl_surface);
    wl_egl_window_destroy(gl_start_support_->egl_window);
    wl_subsurface_destroy(gl_start_support_->subsurface);
    wl_surface_destroy(gl_start_support_->surface);
    delete gl_start_support_;
  }
};


static Fl_Wayland_Gl_Plugin Gl_Overlay_Plugin;


/* CONTROL_LEAKING_SUB_GL_WINDOWS
static void delayed_scissor(Fl_Wayland_Gl_Window_Driver *dr) {
  dr->apply_scissor();
}*/


void Fl_Wayland_Gl_Window_Driver::resize(int is_a_resize, int W, int H) {
  if (!egl_window) return;
  float f = Fl::screen_scale(pWindow->screen_num());
  int s = Fl_Wayland_Window_Driver::driver(pWindow)->wld_scale();
  W = int(W * f) * s; // W, H must be multiples of int s
  H = int(H * f) * s;
  int W2, H2;
  wl_egl_window_get_attached_size(egl_window, &W2, &H2);
  if (W2 != W || H2 != H) {
    struct wld_window *xid = fl_wl_xid(pWindow);
    if (xid->kind == Fl_Wayland_Window_Driver::DECORATED && !xid->frame_cb) {
      xid->frame_cb = wl_surface_frame(xid->wl_surface);
      wl_callback_add_listener(xid->frame_cb,
                               Fl_Wayland_Graphics_Driver::p_surface_frame_listener, xid);
    }
    wl_egl_window_resize(egl_window, W, H, 0, 0);
  }
  /* CONTROL_LEAKING_SUB_GL_WINDOWS
  if (Fl_Wayland_Window_Driver::driver(pWindow)->subRect()) {
    pWindow->redraw();
    Fl::add_timeout(0.01, (Fl_Timeout_Handler)delayed_scissor, this);
  }*/
}

char Fl_Wayland_Gl_Window_Driver::swap_type() {
  return copy;
}


void Fl_Wayland_Gl_Window_Driver::gl_visual(Fl_Gl_Choice *c) {
  Fl_Gl_Window_Driver::gl_visual(c);
  wld_egl_conf = ((Fl_Wayland_Gl_Choice*)c)->egl_conf;
}


void Fl_Wayland_Gl_Window_Driver::gl_start() {
  float f = Fl::screen_scale(Fl_Window::current()->screen_num());
  int W = Fl_Window::current()->w() * f;
  int H = Fl_Window::current()->h() * f;
  int W2, H2;
  Fl_Wayland_Window_Driver *dr = Fl_Wayland_Window_Driver::driver(Fl_Window::current());
  wl_egl_window_get_attached_size(dr->gl_start_support_->egl_window, &W2, &H2);
  if (W2 != W || H2 != H) {
    wl_egl_window_resize(dr->gl_start_support_->egl_window, W, H, 0, 0);
  }
  glClearColor(0., 0., 0., 0.);
  glClear(GL_COLOR_BUFFER_BIT);
}

void Fl_Wayland_Gl_Window_Driver::swap_interval(int interval) {
  if (interval < min_swap_interval) interval = min_swap_interval;
  if (interval > max_swap_interval) interval = max_swap_interval;
  if (egl_display && eglSwapInterval(egl_display, interval))
    swap_interval_ = interval;
  // printf("swap_interval_=%d\n",swap_interval_);
}


int Fl_Wayland_Gl_Window_Driver::swap_interval() const {
  return swap_interval_;
}

FL_EXPORT EGLContext fl_wl_glcontext(GLContext rc) { return (EGLContext)rc; }

#endif // HAVE_GL
