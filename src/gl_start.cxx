// Code to switch current fltk drawing context in/out of GL "mode":

// You MUST use gl_visual() to select the default visual before doing
// show() of any windows.  Mesa will crash if you try to use a visual
// not returned by glxChooseVisual.

// This does not work with Fl_Double_Window's!  It will try to draw
// into the front buffer.  Depending on the system this will either
// crash or do nothing (when pixmaps are being used as back buffer
// and GL is being done by hardware), work correctly (when GL is done
// with software, such as Mesa), or draw into the front buffer and
// be erased when the buffers are swapped (when double buffer hardware
// is being used)

#include <config.h>
#if HAVE_GL

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/x.H>
#include <FL/fl_draw.H>

#include "Fl_Gl_Choice.H"

extern GLXContext fl_first_context; // in Fl_Gl_Choice.C
extern int fl_clip_state_number; // in fl_rect.C

static GLXContext context;
static int clip_state_number=-1;
static int pw, ph;

#ifdef WIN32
static int default_mode;
#endif

Region XRectangleRegion(int x, int y, int w, int h); // in fl_rect.C

void gl_start() {
#ifdef WIN32
  HDC hdc = fl_private_dc(Fl_Window::current(), default_mode,0);
  if (!context) {
    context = wglCreateContext(hdc);
    if (!fl_first_context) fl_first_context = context;
    else wglShareLists(fl_first_context, context);
  }
  wglMakeCurrent(hdc, context);
#else
  if (!context) {
    context = glXCreateContext(fl_display, fl_visual, fl_first_context, 1);
    if (!context) Fl::fatal("OpenGL does not support this visual");
    if (!fl_first_context) fl_first_context = context;
  }
  glXMakeCurrent(fl_display, fl_window, context);
  glXWaitX();
#endif
  if (pw != Fl_Window::current()->w() || ph != Fl_Window::current()->h()) {
    pw = Fl_Window::current()->w();
    ph = Fl_Window::current()->h();
    glLoadIdentity();
    glViewport(0, 0, pw, ph);
    glOrtho(0, pw, 0, ph, -1, 1);
    glDrawBuffer(GL_FRONT);
  }
  if (clip_state_number != fl_clip_state_number) {
    clip_state_number = fl_clip_state_number;
    int x, y, w, h;
    if (fl_clip_box(0, 0, Fl_Window::current()->w(), Fl_Window::current()->h(),
		    x, y, w, h)) {
      fl_clip_region(XRectangleRegion(x,y,w,h));
      glScissor(x, Fl_Window::current()->h()-(y+h), w, h);
      glEnable(GL_SCISSOR_TEST);
    } else {
      glDisable(GL_SCISSOR_TEST);
    }
  }
}

void gl_finish() {
#ifdef WIN32
  glFlush();
#else
  glXWaitGL();
#endif
}

int Fl::gl_visual(int mode, int *alist) {
#ifdef WIN32
  default_mode = mode;
#else
  Fl_Gl_Choice *c = Fl_Gl_Choice::find(mode,alist);
  if (!c) return 0;
  fl_visual = c->vis;
  fl_colormap = c->colormap;
#endif
  return 1;
}

#endif
