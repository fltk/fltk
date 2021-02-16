//
// Class Fl_X11_Gl_Window_Driver for the Fast Light Tool Kit (FLTK).
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
#include "../../Fl_Gl_Choice.H"
#include "../../Fl_Screen_Driver.H"
#include "../../Fl_Window_Driver.H"
#include "../../Fl_Gl_Window_Driver.H"
#include "../Xlib/Fl_Font.H"
#include "../Xlib/Fl_Xlib_Graphics_Driver.H"
#  include <GL/glx.h>
#  if ! defined(GLX_VERSION_1_3)
#    typedef void *GLXFBConfig;
#  endif

class Fl_X11_Gl_Window_Driver : public Fl_Gl_Window_Driver {
  friend class Fl_Gl_Window_Driver;
  Fl_X11_Gl_Window_Driver(Fl_Gl_Window *win) : Fl_Gl_Window_Driver(win) {}
  virtual float pixels_per_unit();
  virtual void before_show(int& need_after);
  virtual int mode_(int m, const int *a);
  virtual void swap_buffers();
  virtual void resize(int is_a_resize, int w, int h);
  virtual char swap_type();
  virtual Fl_Gl_Choice *find(int m, const int *alistp);
  virtual GLContext create_gl_context(Fl_Window* window, const Fl_Gl_Choice* g, int layer = 0);
  virtual void set_gl_context(Fl_Window* w, GLContext context);
  virtual void delete_gl_context(GLContext);
#if HAVE_GL_OVERLAY
  virtual void make_overlay(void *&o);
  virtual int can_do_overlay();
  virtual void hide_overlay();
  virtual int overlay_color(Fl_Color i);
#endif
  virtual void make_overlay_current();
  virtual void redraw_overlay();
  virtual void waitGL();
  virtual void gl_visual(Fl_Gl_Choice*); // support for Fl::gl_visual()
  virtual void gl_start();
  virtual void draw_string_legacy(const char* str, int n);
  virtual void gl_bitmap_font(Fl_Font_Descriptor *fl_fontsize);
  virtual void get_list(Fl_Font_Descriptor *fd, int r);
  virtual int genlistsize();
#if !USE_XFT
  virtual Fl_Font_Descriptor** fontnum_to_fontdescriptor(int fnum);
#endif
  public:
  static GLContext create_gl_context(XVisualInfo* vis);
};

// Describes crap needed to create a GLContext.
class Fl_X11_Gl_Choice : public Fl_Gl_Choice {
  friend class Fl_X11_Gl_Window_Driver;
private:
  XVisualInfo *vis; /* the visual to use */
  Colormap colormap; /* a colormap for that visual */
  GLXFBConfig best_fb;
public:
  Fl_X11_Gl_Choice(int m, const int *alistp, Fl_Gl_Choice *n) : Fl_Gl_Choice(m, alistp, n) {
    vis = NULL;
    colormap = 0;
    best_fb = NULL;
  }
};

void Fl_X11_Gl_Window_Driver::draw_string_legacy(const char* str, int n) {
  draw_string_legacy_get_list(str, n);
}

int Fl_X11_Gl_Window_Driver::genlistsize() {
#if USE_XFT
  return 256;
#else
  return 0x10000;
#endif
}

void Fl_X11_Gl_Window_Driver::gl_bitmap_font(Fl_Font_Descriptor *fl_fontsize) {
  /* This method should ONLY be triggered if our GL font texture pile mechanism
   * is not working on this platform. This code might not reliably render glyphs
   * from higher codepoints. */
  if (!fl_fontsize->listbase) {
#if USE_XFT
    /* Ideally, for XFT, we need a glXUseXftFont implementation here... But we
     * do not have such a thing. Instead, we try to find a legacy Xlib font that
     * matches the current XFT font and use that.
     * Ideally, we never come here - we hope the texture pile implementation
     * will work correctly so that XFT can render the face directly without the
     * need for this workaround. */
    XFontStruct *font = fl_xfont.value();
    int base = font->min_char_or_byte2;
    int count = font->max_char_or_byte2 - base + 1;
    fl_fontsize->listbase = glGenLists(genlistsize());
    glXUseXFont(font->fid, base, count, fl_fontsize->listbase+base);
#else
    /* Not using XFT to render text - the legacy Xlib fonts can usually be rendered
     * directly by using glXUseXFont mechanisms. */
    fl_fontsize->listbase = glGenLists(genlistsize());
#endif // !USE_XFT
  }
  glListBase(fl_fontsize->listbase);
}


void Fl_X11_Gl_Window_Driver::get_list(Fl_Font_Descriptor *fd, int r) {
# if USE_XFT
  /* We hope not to come here: We hope that any system using XFT will also
   * have sufficient GL capability to support our font texture pile mechansim,
   * allowing XFT to render the face directly. */
  // Face already set by gl_bitmap_font in this case.
# else
  Fl_Xlib_Font_Descriptor *gl_fd = (Fl_Xlib_Font_Descriptor*)fd;
  if (gl_fd->glok[r]) return;
  gl_fd->glok[r] = 1;
  unsigned int ii = r * 0x400;
  for (int i = 0; i < 0x400; i++) {
    XFontStruct *font = NULL;
    unsigned short id;
    fl_XGetUtf8FontAndGlyph(gl_fd->font, ii, &font, &id);
    if (font) glXUseXFont(font->fid, id, 1, gl_fd->listbase+ii);
    ii++;
  }
# endif
}

#if !USE_XFT
Fl_Font_Descriptor** Fl_X11_Gl_Window_Driver::fontnum_to_fontdescriptor(int fnum) {
  Fl_Xlib_Fontdesc *s = ((Fl_Xlib_Fontdesc*)fl_fonts) + fnum;
  return &(s->first);
}
#endif


static XVisualInfo *gl3_getvisual(const int *blist, GLXFBConfig *pbestFB)
{
  int glx_major, glx_minor;

  // FBConfigs were added in GLX version 1.3.
  if ( !glXQueryVersion(fl_display, &glx_major, &glx_minor) ||
      ( ( glx_major == 1 ) && ( glx_minor < 3 ) ) || ( glx_major < 1 ) ) {
    return NULL;
  }

  //printf( "Getting matching framebuffer configs\n" );
  int fbcount;
  GLXFBConfig* fbc = glXChooseFBConfig(fl_display, DefaultScreen(fl_display), blist, &fbcount);
  if (!fbc) {
    //printf( "Failed to retrieve a framebuffer config\n" );
    return NULL;
  }
  //printf( "Found %d matching FB configs.\n", fbcount );

  // Pick the FB config/visual with the most samples per pixel
  int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;
  for (int i = 0; i < fbcount; ++i)
  {
    XVisualInfo *vi = glXGetVisualFromFBConfig( fl_display, fbc[i] );
    if (vi) {
      int samp_buf, samples;
      glXGetFBConfigAttrib(fl_display, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf);
      glXGetFBConfigAttrib(fl_display, fbc[i], GLX_SAMPLES       , &samples );
      /*printf( "  Matching fbconfig %d, visual ID 0x%2lx: SAMPLE_BUFFERS = %d, SAMPLES = %d\n",
             i, vi -> visualid, samp_buf, samples );*/
      if ( best_fbc < 0 || (samp_buf && samples > best_num_samp) )
        best_fbc = i, best_num_samp = samples;
      if ( worst_fbc < 0 || !samp_buf || samples < worst_num_samp )
        worst_fbc = i, worst_num_samp = samples;
    }
    XFree(vi);
  }

  GLXFBConfig bestFbc = fbc[ best_fbc ];
  // Be sure to free the FBConfig list allocated by glXChooseFBConfig()
  XFree(fbc);
  // Get a visual
  XVisualInfo *vi = glXGetVisualFromFBConfig(fl_display, bestFbc);
  *pbestFB = bestFbc;
  return vi;
}

Fl_Gl_Choice *Fl_X11_Gl_Window_Driver::find(int m, const int *alistp)
{
  Fl_X11_Gl_Choice *g = (Fl_X11_Gl_Choice*)Fl_Gl_Window_Driver::find_begin(m, alistp);
  if (g) return g;

  const int *blist;
  int list[32];

  if (alistp)
    blist = alistp;
  else {
    int n = 0;
    if (m & FL_INDEX) {
      list[n++] = GLX_BUFFER_SIZE;
      list[n++] = 8; // glut tries many sizes, but this should work...
    } else {
      list[n++] = GLX_RGBA;
      list[n++] = GLX_GREEN_SIZE;
      list[n++] = (m & FL_RGB8) ? 8 : 1;
      if (m & FL_ALPHA) {
        list[n++] = GLX_ALPHA_SIZE;
        list[n++] = (m & FL_RGB8) ? 8 : 1;
      }
      if (m & FL_ACCUM) {
        list[n++] = GLX_ACCUM_GREEN_SIZE;
        list[n++] = 1;
        if (m & FL_ALPHA) {
          list[n++] = GLX_ACCUM_ALPHA_SIZE;
          list[n++] = 1;
        }
      }
    }
    if (m & FL_DOUBLE) {
      list[n++] = GLX_DOUBLEBUFFER;
    }
    if (m & FL_DEPTH) {
      list[n++] = GLX_DEPTH_SIZE; list[n++] = 1;
    }
    if (m & FL_STENCIL) {
      list[n++] = GLX_STENCIL_SIZE; list[n++] = 1;
    }
    if (m & FL_STEREO) {
      list[n++] = GLX_STEREO;
    }
#    if defined(GLX_VERSION_1_1) && defined(GLX_SGIS_multisample)
    if (m & FL_MULTISAMPLE) {
      list[n++] = GLX_SAMPLES_SGIS;
      list[n++] = 4; // value Glut uses
    }
#    endif
    list[n] = 0;
    blist = list;
  }

  fl_open_display();
  XVisualInfo *visp = NULL;
  GLXFBConfig best_fb = NULL;
  if (m & FL_OPENGL3) {
    visp = gl3_getvisual((const int *)blist, &best_fb);
  }
  if (!visp) {
    visp = glXChooseVisual(fl_display, fl_screen, (int *)blist);
    if (!visp) {
#     if defined(GLX_VERSION_1_1) && defined(GLX_SGIS_multisample)
        if (m&FL_MULTISAMPLE) return find(m&~FL_MULTISAMPLE, 0);
#     endif
      return 0;
    }
  }

  g = new Fl_X11_Gl_Choice(m, alistp, first);
  first = g;

  g->vis = visp;
  g->best_fb = best_fb;

  if (/*MaxCmapsOfScreen(ScreenOfDisplay(fl_display,fl_screen))==1 && */
      visp->visualid == fl_visual->visualid &&
      !fl_getenv("MESA_PRIVATE_CMAP"))
    g->colormap = fl_colormap;
  else
    g->colormap = XCreateColormap(fl_display, RootWindow(fl_display,fl_screen),
                                  visp->visual, AllocNone);
  return g;
}

static bool ctxErrorOccurred = false;
static int ctxErrorHandler( Display *dpy, XErrorEvent *ev )
{
  ctxErrorOccurred = true;
  return 0;
}

GLContext Fl_X11_Gl_Window_Driver::create_gl_context(Fl_Window* window, const Fl_Gl_Choice* g, int layer) {
  GLContext shared_ctx = 0;
  if (context_list && nContext) shared_ctx = context_list[0];

  typedef GLContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLContext, Bool, const int*);
  // It is not necessary to create or make current to a context before calling glXGetProcAddressARB
  static glXCreateContextAttribsARBProc glXCreateContextAttribsARB =
#if defined(HAVE_GLXGETPROCADDRESSARB)
    (glXCreateContextAttribsARBProc)glXGetProcAddressARB((const GLubyte *)"glXCreateContextAttribsARB");
#else
  NULL;
#endif

  GLContext ctx = 0;
  // Check for the GLX_ARB_create_context extension string and the function.
  // If either is not present, use GLX 1.3 context creation method.
  const char *glxExts = glXQueryExtensionsString(fl_display, fl_screen);
  if (((Fl_X11_Gl_Choice*)g)->best_fb && strstr(glxExts, "GLX_ARB_create_context") && glXCreateContextAttribsARB ) {
    int context_attribs[] =
    {
      GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
      GLX_CONTEXT_MINOR_VERSION_ARB, 2,
      //GLX_CONTEXT_FLAGS_ARB        , GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
      //GLX_CONTEXT_PROFILE_MASK_ARB , GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
      None
    };
    ctxErrorOccurred = false;
    XErrorHandler oldHandler = XSetErrorHandler(&ctxErrorHandler);
    ctx = glXCreateContextAttribsARB(fl_display, ((Fl_X11_Gl_Choice*)g)->best_fb, shared_ctx, true, context_attribs);
    XSync(fl_display, false); // Sync to ensure any errors generated are processed.
    if (ctxErrorOccurred) ctx = 0;
    XSetErrorHandler(oldHandler);
  }
  if (!ctx) { // use OpenGL 1-style context creation
    ctx = glXCreateContext(fl_display, ((Fl_X11_Gl_Choice*)g)->vis, shared_ctx, true);
  }
  if (ctx)
    add_context(ctx);
//glXMakeCurrent(fl_display, fl_xid(window), ctx);printf("%s\n", glGetString(GL_VERSION));
  return ctx;
}

GLContext Fl_X11_Gl_Window_Driver::create_gl_context(XVisualInfo *vis) {
  GLContext shared_ctx = 0;
  if (context_list && nContext) shared_ctx = context_list[0];
  GLContext context = glXCreateContext(fl_display, vis, shared_ctx, 1);
  if (context)
    add_context(context);
  return context;
}

void Fl_X11_Gl_Window_Driver::set_gl_context(Fl_Window* w, GLContext context) {
  if (context != cached_context || w != cached_window) {
    cached_context = context;
    cached_window = w;
    glXMakeCurrent(fl_display, fl_xid(w), context);
  }
}

void Fl_X11_Gl_Window_Driver::delete_gl_context(GLContext context) {
  if (cached_context == context) {
    cached_context = 0;
    cached_window = 0;
    glXMakeCurrent(fl_display, 0, 0);
  }
  glXDestroyContext(fl_display, context);
  del_context(context);
}


void Fl_X11_Gl_Window_Driver::make_overlay_current() {
#if HAVE_GL_OVERLAY
  if (overlay() != pWindow) {
    ((Fl_Gl_Window*)overlay())->make_current();
  } else
#endif
    glDrawBuffer(GL_FRONT);
}

void Fl_X11_Gl_Window_Driver::redraw_overlay() {
  if (overlay() != pWindow)
    ((Fl_Gl_Window*)overlay())->redraw();
  else
    pWindow->damage(FL_DAMAGE_OVERLAY);
}

#if HAVE_GL_OVERLAY

// Methods on Fl_Gl_Window_Driver that create an overlay window.

// Under X this is done by creating another window, of class _Fl_Gl_Overlay
// which is a subclass of Fl_Gl_Window except it uses the overlay planes.
// A pointer to this is stored in the "overlay" pointer of the Fl_Gl_Window.

// If overlay hardware is unavailable, the overlay is
// "faked" by drawing into the main layers.  This is indicated by
// setting overlay == this.

extern XVisualInfo *fl_find_overlay_visual();
extern XVisualInfo *fl_overlay_visual;
extern Colormap fl_overlay_colormap;
extern unsigned long fl_transparent_pixel;
//extern uchar fl_overlay;

int Fl_X11_Gl_Window_Driver::overlay_color(Fl_Color i) {
  if (Fl_Xlib_Graphics_Driver::fl_overlay) {glIndexi(int(fl_xpixel(i))); return 1;}
  return 0;
}


class _Fl_Gl_Overlay : public Fl_Gl_Window {
  void flush();
  void draw();
public:
  void show();
  _Fl_Gl_Overlay(int x, int y, int w, int h) :
    Fl_Gl_Window(x,y,w,h) {
    set_flag(INACTIVE);
  }
};

void _Fl_Gl_Overlay::flush() {
  make_current();
#ifdef BOXX_BUGS
  // The BoXX overlay is broken and you must not call swap-buffers. This
  // code will make it work, but we lose because machines that do support
  // double-buffered overlays will blink when they don't have to
  glDrawBuffer(GL_FRONT);
  draw();
#else
  draw();
  swap_buffers();
#endif
  glFlush();
  valid(1);
}

void _Fl_Gl_Overlay::draw() {
  if (!valid()) glClearIndex((GLfloat)fl_transparent_pixel);
  if (damage() != FL_DAMAGE_EXPOSE) glClear(GL_COLOR_BUFFER_BIT);
  Fl_Gl_Window *w = (Fl_Gl_Window *)parent();
  uchar save_valid = w->valid();
  w->valid(valid());
  Fl_Xlib_Graphics_Driver::fl_overlay = 1;
  w->gl_driver()->draw_overlay();
  Fl_Xlib_Graphics_Driver::fl_overlay = 0;
  valid(w->valid());
  w->valid(save_valid);
}

void _Fl_Gl_Overlay::show() {
  if (!shown()) {
    fl_background_pixel = int(fl_transparent_pixel);
    Fl_X::make_xid(this, fl_overlay_visual, fl_overlay_colormap);
    fl_background_pixel = -1;
    // find the outermost window to tell wm about the colormap:
    Fl_Window *w = window();
    for (;;) {Fl_Window *w1 = w->window(); if (!w1) break; w = w1;}
    XSetWMColormapWindows(fl_display, fl_xid(w), &(Fl_X::i(this)->xid), 1);
    context(Fl_X11_Gl_Window_Driver::create_gl_context(fl_overlay_visual), 1);
    valid(0);
  }
  Fl_Gl_Window::show();
}

void Fl_X11_Gl_Window_Driver::hide_overlay() {
  if (overlay() && overlay() != pWindow) ((Fl_Gl_Window*)overlay())->hide();
}

int Fl_X11_Gl_Window_Driver::can_do_overlay() {
  return fl_find_overlay_visual() != 0;
}


void Fl_X11_Gl_Window_Driver::make_overlay(void *&current) {
  if (current) return;
  if (can_do_overlay()) {
    _Fl_Gl_Overlay* o = new _Fl_Gl_Overlay(0, 0, pWindow->w(), pWindow->h());
    current = o;
    pWindow->add(*o);
    o->show();
  } else {
    current = pWindow; // fake the overlay
  }
}
#endif // HAVE_GL_OVERLAY


Fl_Gl_Window_Driver *Fl_Gl_Window_Driver::newGlWindowDriver(Fl_Gl_Window *w)
{
  return new Fl_X11_Gl_Window_Driver(w);
}

void Fl_X11_Gl_Window_Driver::before_show(int&) {
  Fl_X11_Gl_Choice *g = (Fl_X11_Gl_Choice*)this->g();
  Fl_X::make_xid(pWindow, g->vis, g->colormap);
  if (overlay() && overlay() != pWindow) ((Fl_Gl_Window*)overlay())->show();
}

float Fl_X11_Gl_Window_Driver::pixels_per_unit()
{
  int ns = Fl_Window_Driver::driver(pWindow)->screen_num();
  return Fl::screen_driver()->scale(ns);
}

int Fl_X11_Gl_Window_Driver::mode_(int m, const int *a) {
  int oldmode = mode();
  if (a) { // when the mode is set using the a array of system-dependent values, and if asking for double buffer,
    // the FL_DOUBLE flag must be set in the mode_ member variable
    const int *aa = a;
    while (*aa) {
      if (*(aa++) ==
          GLX_DOUBLEBUFFER
          ) { m |= FL_DOUBLE; break; }
    }
  }
  Fl_X11_Gl_Choice* oldg = (Fl_X11_Gl_Choice*)g();
  pWindow->context(0);
  mode(m); alist(a);
  if (pWindow->shown()) {
    g( find(m, a) );
    // under X, if the visual changes we must make a new X window (yuck!):
    Fl_X11_Gl_Choice* g = (Fl_X11_Gl_Choice*)this->g();
    if (!g || g->vis->visualid != oldg->vis->visualid || (oldmode^m)&FL_DOUBLE) {
      pWindow->hide();
      pWindow->show();
    }
  } else {
    g(0);
  }
  return 1;
}

void Fl_X11_Gl_Window_Driver::swap_buffers() {
  glXSwapBuffers(fl_display, fl_xid(pWindow));
}

void Fl_X11_Gl_Window_Driver::resize(int is_a_resize, int W, int H) {
  if (is_a_resize && !pWindow->resizable() && overlay() && overlay() != pWindow) {
    ((Fl_Gl_Window*)overlay())->resize(0,0,W,H);
  }
}

char Fl_X11_Gl_Window_Driver::swap_type() {return copy;}

void Fl_X11_Gl_Window_Driver::waitGL() {
  glXWaitGL();
}


void Fl_X11_Gl_Window_Driver::gl_visual(Fl_Gl_Choice *c) {
  Fl_Gl_Window_Driver::gl_visual(c);
  fl_visual = ((Fl_X11_Gl_Choice*)c)->vis;
  fl_colormap = ((Fl_X11_Gl_Choice*)c)->colormap;
}

void Fl_X11_Gl_Window_Driver::gl_start() {
  glXWaitX();
}

#endif // HAVE_GL
