//
// Class Fl_X11_Gl_Window_Driver for the Fast Light Tool Kit (FLTK).
//
// Copyright 2021-2024 by Bill Spitzak and others.
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
#include "Fl_X11_Gl_Window_Driver.H"
#include <GL/glx.h>
#if ! USE_XFT
#  include "../Xlib/Fl_Font.H"
#endif


// Describes crap needed to create a GLContext.
class Fl_X11_Gl_Choice : public Fl_Gl_Choice {
  friend class Fl_X11_Gl_Window_Driver;
private:
  XVisualInfo *vis; /* the visual to use */
  Colormap colormap; /* a colormap for that visual */
public:
  Fl_X11_Gl_Choice(int m, const int *alistp, Fl_Gl_Choice *n) : Fl_Gl_Choice(m, alistp, n) {
    vis = NULL;
    colormap = 0;
  }
};

#ifndef FLTK_USE_WAYLAND
Fl_Gl_Window_Driver *Fl_Gl_Window_Driver::newGlWindowDriver(Fl_Gl_Window *w)
{
  return new Fl_X11_Gl_Window_Driver(w);
}
#endif

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
#if USE_XFT && !FLTK_USE_CAIRO
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
  (void)fd; (void)r;
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
  XVisualInfo *visp = glXChooseVisual(fl_display, fl_screen, (int *)blist);
  if (!visp) {
#   if defined(GLX_VERSION_1_1) && defined(GLX_SGIS_multisample)
      if (m&FL_MULTISAMPLE) return find(m&~FL_MULTISAMPLE, 0);
#   endif
      return 0;
  }

  g = new Fl_X11_Gl_Choice(m, alistp, first);
  first = g;

  g->vis = visp;

  if (/*MaxCmapsOfScreen(ScreenOfDisplay(fl_display,fl_screen))==1 && */
      visp->visualid == fl_visual->visualid &&
      !fl_getenv("MESA_PRIVATE_CMAP"))
    g->colormap = fl_colormap;
  else
    g->colormap = XCreateColormap(fl_display, RootWindow(fl_display,fl_screen),
                                  visp->visual, AllocNone);
  return g;
}


GLContext Fl_X11_Gl_Window_Driver::create_gl_context(Fl_Window* window,
                                                     const Fl_Gl_Choice* g) {
  (void)window;
  GLContext shared_ctx = 0;
  if (context_list && nContext) shared_ctx = context_list[0];
  // use OpenGL 1-style context creation
  GLContext ctx = glXCreateContext(fl_display, ((Fl_X11_Gl_Choice*)g)->vis, (GLXContext)shared_ctx, true);
  if (ctx)
    add_context(ctx);
//glXMakeCurrent(fl_display, fl_xid(window), ctx);printf("%s\n", glGetString(GL_VERSION));
  return ctx;
}


void Fl_X11_Gl_Window_Driver::set_gl_context(Fl_Window* w, GLContext context) {
  GLContext current_context = glXGetCurrentContext();
  if (context != current_context || w != cached_window) {
    cached_window = w;
    glXMakeCurrent(fl_display, fl_xid(w), (GLXContext)context);
  }
}

void Fl_X11_Gl_Window_Driver::delete_gl_context(GLContext context) {
  GLContext current_context = glXGetCurrentContext();
  if (current_context == context) {
    cached_window = 0;
    glXMakeCurrent(fl_display, 0, 0);
  }
  glXDestroyContext(fl_display, (GLXContext)context);
  del_context(context);
}


void Fl_X11_Gl_Window_Driver::make_overlay_current() {
    glDrawBuffer(GL_FRONT);
}

void Fl_X11_Gl_Window_Driver::redraw_overlay() {
    pWindow->damage(FL_DAMAGE_OVERLAY);
}


void Fl_X11_Gl_Window_Driver::before_show(int&) {
  Fl_X11_Gl_Choice *g = (Fl_X11_Gl_Choice*)this->g();
  Fl_X::make_xid(pWindow, g->vis, g->colormap);
}

float Fl_X11_Gl_Window_Driver::pixels_per_unit()
{
  int ns = pWindow->screen_num();
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
  if (!fl_xid(pWindow)) // window not shown
    return;
  if (overlay()) {
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
      glReadBuffer(GL_BACK);
      glDrawBuffer(GL_FRONT);
      glCopyPixels(0, 0, wo, ho, GL_COLOR);         // copy GL_BACK to GL_FRONT
    }
    glPopMatrix(); // GL_MODELVIEW                  // restore model/proj matrices
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(matrixmode);
    glRasterPos3f(pos[0], pos[1], pos[2]);              // restore original glRasterPos
  } else
  glXSwapBuffers(fl_display, fl_xid(pWindow));
}

char Fl_X11_Gl_Window_Driver::swap_type() {
  return copy;
}


// Start of swap_interval implementation in the three possibel ways for X11

// -1 = not yet initialized, 0 = none found, 1 = GLX, 2 = MESA, 3 = SGI
static signed char swap_interval_type = -1;

typedef void (*Fl_GLX_Set_Swap_Iterval_Proc) (Display *dpy, GLXDrawable drawable, int interval);
typedef int (*Fl_MESA_Set_Swap_Iterval_Proc) (unsigned int interval);
typedef int (*Fl_MESA_Get_Swap_Iterval_Proc) ();
typedef int (*Fl_SGI_Set_Swap_Iterval_Proc) (int interval);

static union {
  Fl_GLX_Set_Swap_Iterval_Proc EXT;
  Fl_MESA_Set_Swap_Iterval_Proc MESA;
  Fl_SGI_Set_Swap_Iterval_Proc SGI;
} fl_glXSwapInterval = { NULL };

static Fl_MESA_Get_Swap_Iterval_Proc fl_glXGetSwapIntervalMESA = NULL;

static void init_swap_interval() {
  if (swap_interval_type != -1) return;
  int major = 1, minor = 0;
  glXQueryVersion(fl_display, &major, &minor);
  swap_interval_type = 0;
  const char *extensions = glXQueryExtensionsString(fl_display, fl_screen);
  if (strstr(extensions, "GLX_EXT_swap_control") && ((major > 1) || (minor >= 3))) {
    fl_glXSwapInterval.EXT = (Fl_GLX_Set_Swap_Iterval_Proc)glXGetProcAddressARB((const GLubyte*)"glXSwapIntervalEXT");
    swap_interval_type = 1;
  } else if (strstr(extensions, "GLX_MESA_swap_control")) {
    fl_glXSwapInterval.MESA = (Fl_MESA_Set_Swap_Iterval_Proc)glXGetProcAddressARB((const GLubyte*)"glXSwapIntervalMESA");
    fl_glXGetSwapIntervalMESA = (Fl_MESA_Get_Swap_Iterval_Proc)glXGetProcAddressARB((const GLubyte*)"glXGetSwapIntervalMESA");
    swap_interval_type = 2;
  } else if (strstr(extensions, "GLX_SGI_swap_control")) {
    fl_glXSwapInterval.SGI = (Fl_SGI_Set_Swap_Iterval_Proc)glXGetProcAddressARB((const GLubyte*)"glXSwapIntervalSGI");
    swap_interval_type = 3;
  }
}

void Fl_X11_Gl_Window_Driver::swap_interval(int interval) {
  if (!fl_xid(pWindow))
    return;
  if (swap_interval_type == -1)
    init_swap_interval();
  switch (swap_interval_type) {
    case 1:
      if (fl_glXSwapInterval.EXT)
        fl_glXSwapInterval.EXT(fl_display, fl_xid(pWindow), interval);
      break;
    case 2:
      if (fl_glXSwapInterval.MESA)
        fl_glXSwapInterval.MESA((unsigned int)interval);
      break;
    case 3:
      if (fl_glXSwapInterval.SGI)
        fl_glXSwapInterval.SGI(interval);
      break;
  }
}

int Fl_X11_Gl_Window_Driver::swap_interval() const {
  if (!fl_xid(pWindow))
    return -1;
  if (swap_interval_type == -1)
    init_swap_interval();
  int interval = -1;
  switch (swap_interval_type) {
    case 1: {
      unsigned int val = 0;
      glXQueryDrawable(fl_display, fl_xid(pWindow), 0x20F1 /*GLX_SWAP_INTERVAL_EXT*/, &val);
      interval = (int)val;
      break; }
    case 2:
      if (fl_glXGetSwapIntervalMESA)
        interval = fl_glXGetSwapIntervalMESA();
      break;
    case 3:
      // not available
      break;
  }
  return interval;
}

// end of swap_interval implementation

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


FL_EXPORT GLXContext fl_x11_glcontext(GLContext rc) { return (GLXContext)rc; }


#endif // HAVE_GL
