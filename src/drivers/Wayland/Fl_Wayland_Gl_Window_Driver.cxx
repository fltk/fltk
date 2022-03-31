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

#include <config.h>
#if HAVE_GL
#include <FL/platform.H>
#include <FL/Fl_Image_Surface.H>
#include "../../Fl_Gl_Choice.H"
#include "../../Fl_Screen_Driver.H"
#include "Fl_Wayland_Window_Driver.H"
#include "Fl_Wayland_Graphics_Driver.H"
#include "Fl_Wayland_Gl_Window_Driver.H"
#include <wayland-egl.h>
#include <EGL/egl.h>
#include <FL/gl.h>

/* Implementation note about OpenGL drawing on the Wayland platform

After eglCreateWindowSurface() with attributes {EGL_RENDER_BUFFER, EGL_SINGLE_BUFFER, EGL_NONE},
eglQueryContext() reports that EGL_RENDER_BUFFER equals EGL_BACK_BUFFER.
This experiment suggests that the platform only supports double-buffer drawing.
Consequently, FL_DOUBLE is enforced in all Fl_Gl_Window::mode_ values under Wayland.
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

EGLDisplay Fl_Wayland_Gl_Window_Driver::egl_display = EGL_NO_DISPLAY;
EGLint Fl_Wayland_Gl_Window_Driver::configs_count = 0;
struct wl_event_queue *Fl_Wayland_Gl_Window_Driver::gl_event_queue = NULL;


Fl_Wayland_Gl_Window_Driver::Fl_Wayland_Gl_Window_Driver(Fl_Gl_Window *win) : Fl_Gl_Window_Driver(win) {
  if (egl_display == EGL_NO_DISPLAY) init();
  egl_window = NULL;
  egl_surface = NULL;
  egl_resize_in_progress = false;
}


void Fl_Wayland_Gl_Window_Driver::init() {
  EGLint major, minor;

  if (!Fl_Wayland_Screen_Driver::wl_display) Fl::screen_driver()->open_display();
  egl_display = eglGetDisplay((EGLNativeDisplayType) Fl_Wayland_Screen_Driver::wl_display);
  if (egl_display == EGL_NO_DISPLAY) {
    Fl::fatal("Can't create egl display\n");
  }

  if (eglInitialize(egl_display, &major, &minor) != EGL_TRUE) {
    Fl::fatal("Can't initialise egl display\n");
  }
  //printf("EGL major: %d, minor %d\n", major, minor);

  eglGetConfigs(egl_display, NULL, 0, &configs_count);
  //printf("EGL has %d configs\n", configs_count);
  eglBindAPI(EGL_OPENGL_API);

  gl_event_queue = wl_display_create_queue(Fl_Wayland_Screen_Driver::wl_display);
}


char *Fl_Wayland_Gl_Window_Driver::alpha_mask_for_string(const char *str, int n, int w, int h, Fl_Fontsize fs)
{
  // write str to a bitmap just big enough
  struct wld_window *save_win = Fl_Wayland_Window_Driver::wld_window;
  fl_window = Fl_Wayland_Window_Driver::wld_window = NULL;
  Fl_Image_Surface *surf = new Fl_Image_Surface(w, h);
  fl_window = Fl_Wayland_Window_Driver::wld_window = save_win;
  Fl_Font f=fl_font();
  Fl_Surface_Device::push_current(surf);
  fl_color(FL_BLACK);
  fl_rectf(0, 0, w, h);
  fl_color(FL_WHITE);
  fl_font(f, fs);
  fl_draw(str, n, 0, fl_height() - fl_descent());
  // get the R channel only of the bitmap
  char *alpha_buf = new char[w*h], *r = alpha_buf, *q;
  for (int i = 0; i < h; i++) {
    q = (char*)surf->offscreen()->draw_buffer + i * surf->offscreen()->stride;
    for (int j = 0; j < w; j++) {
      *r++ = *q;
      q += 4;
    }
  }
  Fl_Surface_Device::pop_current();
  delete surf;
  return alpha_buf;
}


Fl_Gl_Choice *Fl_Wayland_Gl_Window_Driver::find(int m, const int *alistp)
{
  m |= FL_DOUBLE;
  Fl_Wayland_Gl_Choice *g = (Fl_Wayland_Gl_Choice*)Fl_Gl_Window_Driver::find_begin(m, alistp);
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
    EGL_NONE
  };

  if (m & FL_DEPTH) config_attribs[11] = 1;
  if (m & FL_MULTISAMPLE) config_attribs[13] = 1;
  if (m & FL_STENCIL) config_attribs[15] = 1;

  static EGLConfig *configs = (void**)calloc(configs_count, sizeof(EGLConfig));
  eglChooseConfig(egl_display, config_attribs, configs, configs_count, &n);
  if (n == 0 && (m & FL_MULTISAMPLE)) {
    config_attribs[13] = 0;
    eglChooseConfig(egl_display, config_attribs, configs, configs_count, &n);
  }
  if (n == 0) {
    Fl::fatal("failed to choose an EGL config\n");
  }

  g = new Fl_Wayland_Gl_Choice(m, alistp, first);
  /*for (int i = 0; i < n; i++) {
    EGLint size;
    eglGetConfigAttrib(egl_display, configs[i], EGL_BUFFER_SIZE, &size);
    printf("Buffer size for config %d is %d\n", i, size);
    eglGetConfigAttrib(egl_display, configs[i], EGL_RED_SIZE, &size);
    printf("Red size for config %d is %d\n", i, size);
    // just choose the first one
    g->egl_conf = configs[i];
    break;
  }*/
  // just choose the first config
  g->egl_conf = configs[0];
  first = g;
  return g;
}


GLContext Fl_Wayland_Gl_Window_Driver::create_gl_context(Fl_Window* window, const Fl_Gl_Choice* g, int layer) {
  GLContext shared_ctx = 0;
  if (context_list && nContext) shared_ctx = context_list[0];

  static const EGLint context_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
  GLContext ctx = (GLContext)eglCreateContext(egl_display, ((Fl_Wayland_Gl_Choice*)g)->egl_conf, shared_ctx?shared_ctx:EGL_NO_CONTEXT, context_attribs);
//fprintf(stderr, "eglCreateContext=%p shared_ctx=%p\n", ctx, shared_ctx);
  if (ctx)
    add_context(ctx);
  return ctx;
}


void Fl_Wayland_Gl_Window_Driver::set_gl_context(Fl_Window* w, GLContext context) {
  struct wld_window *win = fl_xid(w);
  if (!win || !egl_surface) return;
  if (context != cached_context || w != cached_window) {
    cached_context = context;
    cached_window = w;
    if (eglMakeCurrent(egl_display, egl_surface, egl_surface, (EGLContext)context)) {
//fprintf(stderr, "EGLContext %p made current\n", context);
    } else {
      Fl::error("eglMakeCurrent() failed\n");
    }
  }
}

void Fl_Wayland_Gl_Window_Driver::delete_gl_context(GLContext context) {
  if (cached_context == context) {
    cached_context = 0;
    cached_window = 0;
  }
//EGLBoolean b =
  eglDestroyContext(egl_display, context);
//fprintf(stderr,"EGL context %p destroyed %s\n", context, b==EGL_TRUE?"successfully":"w/ error");
//b =
  eglDestroySurface(egl_display, egl_surface);
//fprintf(stderr,"EGLSurface %p destroyed %s\n", egl_surface, b==EGL_TRUE?"successfully":"w/ error");
  egl_surface = NULL;
  wl_egl_window_destroy(egl_window);
  egl_window = NULL;
  del_context(context);
}


void Fl_Wayland_Gl_Window_Driver::make_overlay_current() {
//fprintf(stderr, "make_overlay_current\n");
  glDrawBuffer(GL_FRONT);
}

void Fl_Wayland_Gl_Window_Driver::redraw_overlay() {
//fprintf(stderr, "redraw_overlay\n");
  pWindow->redraw();
}


static void gl_frame_ready(void *data, struct wl_callback *cb, uint32_t time) {
  *(bool*)data = true;
}


static const struct wl_callback_listener gl_surface_frame_listener = {
  .done = gl_frame_ready,
};


void Fl_Wayland_Gl_Window_Driver::make_current_before() {
  if (!egl_window) {
    struct wld_window *win = fl_xid(pWindow);
    struct wl_surface *surface = win->wl_surface;
    egl_window = wl_egl_window_create(surface, pWindow->pixel_w(), pWindow->pixel_h());
    if (egl_window == EGL_NO_SURFACE) {
      Fl::fatal("Can't create egl window with wl_egl_window_create()\n");
    } else {
      //fprintf(stderr, "Created egl window=%p\n", egl_window);
    }
    Fl_Wayland_Gl_Choice *g = (Fl_Wayland_Gl_Choice*)this->g();
    egl_surface = eglCreateWindowSurface(egl_display, g->egl_conf, egl_window, NULL);
//fprintf(stderr, "Created egl surface=%p at scale=%d\n", egl_surface, win->scale);
    wl_surface_set_buffer_scale(surface, win->scale);
    if (Fl_Wayland_Screen_Driver::compositor == Fl_Wayland_Screen_Driver::WESTON) {
      bool done = false;
      struct wl_callback *callback = wl_surface_frame(surface);
      wl_surface_commit(surface);
      wl_callback_add_listener(callback, &gl_surface_frame_listener, &done);
      while (!done) wl_display_dispatch(Fl_Wayland_Screen_Driver::wl_display);
    }
  }
}


float Fl_Wayland_Gl_Window_Driver::pixels_per_unit()
{
  int ns = Fl_Window_Driver::driver(pWindow)->screen_num();
  int wld_scale = pWindow->shown() ? fl_xid(pWindow)->scale : 1;
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
    glTranslatef(-wo/2.0f, -ho/2.0f, 0.0f);         // set transform so 0,0 is bottom/left of Gl_Window
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
    //eglSwapInterval(egl_display, 0); // doesn't seem to have any effect in this context
    if (!egl_resize_in_progress) {
      while (wl_display_prepare_read(Fl_Wayland_Screen_Driver::wl_display) != 0) {
        wl_display_dispatch_pending(Fl_Wayland_Screen_Driver::wl_display);
      }
      wl_display_read_events(Fl_Wayland_Screen_Driver::wl_display);
      wl_display_dispatch_queue_pending(Fl_Wayland_Screen_Driver::wl_display,  gl_event_queue);
    }
    egl_resize_in_progress = false;
    eglSwapBuffers(Fl_Wayland_Gl_Window_Driver::egl_display, egl_surface);
  }
}


class Fl_Wayland_Gl_Plugin : public Fl_Wayland_Plugin {
public:
  Fl_Wayland_Gl_Plugin() : Fl_Wayland_Plugin(name()) { }
  virtual const char *name() { return "gl.wayland.fltk.org"; }
  virtual void do_swap(Fl_Window *w) {
    Fl_Gl_Window_Driver *gldr = Fl_Gl_Window_Driver::driver(w->as_gl_window());
    if (gldr->overlay() == w) gldr->swap_buffers();
  }
  virtual void invalidate(Fl_Window *w) {
    w->as_gl_window()->valid(0);
  }
};

static Fl_Wayland_Gl_Plugin Gl_Overlay_Plugin;


void Fl_Wayland_Gl_Window_Driver::resize(int is_a_resize, int W, int H) {
  if (!egl_window) return;
  struct wld_window *win = fl_xid(pWindow);
  float f = Fl::screen_scale(pWindow->screen_num());
  W = (W * win->scale) * f;
  H = (H * win->scale) * f;
  int W2, H2;
  wl_egl_window_get_attached_size(egl_window, &W2, &H2);
  if (W2 != W || H2 != H) {
    wl_egl_window_resize(egl_window, W, H, 0, 0);
    //fprintf(stderr, "Fl_Wayland_Gl_Window_Driver::resize to %dx%d\n", W, H);
    egl_resize_in_progress = true;
  }
}

char Fl_Wayland_Gl_Window_Driver::swap_type() {
  return copy;
}

void Fl_Wayland_Gl_Window_Driver::waitGL() {
}

void Fl_Wayland_Gl_Window_Driver::gl_start() {
}


Fl_RGB_Image* Fl_Wayland_Gl_Window_Driver::capture_gl_rectangle(int x, int y, int w, int h) {
  Fl_Surface_Device::push_current(Fl_Display_Device::display_device());
  Fl_RGB_Image *rgb = Fl_Gl_Window_Driver::capture_gl_rectangle(x, y, w, h);
  Fl_Surface_Device::pop_current();
  return rgb;
}

#endif // HAVE_GL
