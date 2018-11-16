//
// "$Id$"
//
// OpenGL window code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2015 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

#include "flstring.h"
#if HAVE_GL

extern int fl_gl_load_plugin;

#include <FL/Fl.H>
#include <FL/x.H>
#include "Fl_Gl_Choice.H"
#ifdef __APPLE__
#include <FL/gl.h>
#include <OpenGL/OpenGL.h>
#endif
#include <FL/Fl_Gl_Window.H>
#include <stdlib.h>
#include <FL/fl_utf8.h>

////////////////////////////////////////////////////////////////

// The symbol SWAP_TYPE defines what is in the back buffer after doing
// a glXSwapBuffers().

// The OpenGl documentation says that the contents of the backbuffer
// are "undefined" after glXSwapBuffers().  However, if we know what
// is in the backbuffers then we can save a good deal of time.  For
// this reason you can define some symbols to describe what is left in
// the back buffer.

// Having not found any way to determine this from glx (or wgl) I have
// resorted to letting the user specify it with an environment variable,
// GL_SWAP_TYPE, it should be equal to one of these symbols:

// contents of back buffer after glXSwapBuffers():
#define UNDEFINED 1 	// anything
#define SWAP 2		// former front buffer (same as unknown)
#define COPY 3		// unchanged
#define NODAMAGE 4	// unchanged even by X expose() events

static char SWAP_TYPE = 0 ; // 0 = determine it from environment variable

////////////////////////////////////////////////////////////////

/**  Returns non-zero if the hardware supports the given or current OpenGL  mode. */
int Fl_Gl_Window::can_do(int a, const int *b) {
  return Fl_Gl_Choice::find(a,b) != 0;
}

void Fl_Gl_Window::show() {
#if defined(__APPLE__)
  int need_redraw = 0;
#endif
  if (!shown()) {
    if (!g) {
      g = Fl_Gl_Choice::find(mode_,alist);
      if (!g && (mode_ & FL_DOUBLE) == FL_SINGLE) {
        g = Fl_Gl_Choice::find(mode_ | FL_DOUBLE,alist);
	if (g) mode_ |= FL_FAKE_SINGLE;
      }

      if (!g) {
        Fl::error("Insufficient GL support");
	return;
      }
    }
#if !defined(WIN32) && !defined(__APPLE__)
    Fl_X::make_xid(this, g->vis, g->colormap);
    if (overlay && overlay != this) ((Fl_Gl_Window*)overlay)->show();
#elif defined(__APPLE__)
	if( ! parent() ) need_redraw=1;
#endif
  }
  Fl_Window::show();

#ifdef __APPLE__
  set_visible();
  if(need_redraw) redraw();//necessary only after creation of a top-level GL window
#endif /* __APPLE__ */
}

#if defined(__APPLE__)

float Fl_Gl_Window::pixels_per_unit()
{
  return (fl_mac_os_version >= 100700 && Fl::use_high_res_GL() && Fl_X::i(this) && Fl_X::i(this)->mapped_to_retina()) ? 2 : 1;
}

#endif // __APPLE__

/**
  The invalidate() method turns off valid() and is
  equivalent to calling value(0).
*/
void Fl_Gl_Window::invalidate() {
  valid(0);
  context_valid(0);
#ifndef WIN32
  if (overlay) {
    ((Fl_Gl_Window*)overlay)->valid(0);
    ((Fl_Gl_Window*)overlay)->context_valid(0);
  }
#endif
}

int Fl_Gl_Window::mode(int m, const int *a) {
  if (m == mode_ && a == alist) return 0;
#ifndef __APPLE__
  int oldmode = mode_;
#endif
#if defined(__APPLE__) || defined(USE_X11)
  if (a) { // when the mode is set using the a array of system-dependent values, and if asking for double buffer,
           // the FL_DOUBLE flag must be set in the mode_ member variable
    const int *aa = a;
    while (*aa) {
      if (*(aa++) ==
#  if defined(__APPLE__)
          kCGLPFADoubleBuffer
#  else
          GLX_DOUBLEBUFFER
#  endif
          ) { m |= FL_DOUBLE; break; }
    }
  }
#endif // !__APPLE__
#if !defined(WIN32) && !defined(__APPLE__)
  Fl_Gl_Choice* oldg = g;
#endif // !WIN32 && !__APPLE__
  context(0);
  mode_ = m; alist = a;
  if (shown()) {
    g = Fl_Gl_Choice::find(m, a);

#if defined(USE_X11)
    // under X, if the visual changes we must make a new X window (yuck!):
    if (!g || g->vis->visualid!=oldg->vis->visualid || (oldmode^m)&FL_DOUBLE) {
      hide();
      show();
    }
#elif defined(WIN32)
    if (!g || (oldmode^m)&(FL_DOUBLE|FL_STEREO)) {
      hide();
      show();
    }
#elif defined(__APPLE_QUARTZ__)
    redraw();
#else
#  error unsupported platform
#endif
  } else {
    g = 0;
  }
  return 1;
}

#define NON_LOCAL_CONTEXT 0x80000000

/**
  The make_current() method selects the OpenGL context for the
  widget.  It is called automatically prior to the draw() method
  being called and can also be used to implement feedback and/or
  selection within the handle() method.
*/

void Fl_Gl_Window::make_current() {
//  puts("Fl_Gl_Window::make_current()");
//  printf("make_current: context_=%p\n", context_);
#if defined(__APPLE__)
  // detect if the window was moved between low and high resolution displays
  if (Fl_X::i(this)->changed_resolution()){
    Fl_X::i(this)->changed_resolution(false);
    invalidate();
    Fl_X::GLcontext_update(context_);
  }
#endif
  if (!context_) {
    mode_ &= ~NON_LOCAL_CONTEXT;
    context_ = fl_create_gl_context(this, g);
    valid(0);
    context_valid(0);
  }
  fl_set_gl_context(this, context_);

#if defined(WIN32) && USE_COLORMAP
  if (fl_palette) {
    fl_GetDC(fl_xid(this));
    SelectPalette(fl_gc, fl_palette, FALSE);
    RealizePalette(fl_gc);
  }
#endif // USE_COLORMAP
  if (mode_ & FL_FAKE_SINGLE) {
    glDrawBuffer(GL_FRONT);
    glReadBuffer(GL_FRONT);
  }
  current_ = this;
}

/**
  Sets the projection so 0,0 is in the lower left of the window and each
  pixel is 1 unit wide/tall.  If you are drawing 2D images, your 
  draw() method may want to call this if valid() is false.
*/
void Fl_Gl_Window::ortho() {
// Alpha NT seems to have a broken OpenGL that does not like negative coords:
#ifdef _M_ALPHA
  glLoadIdentity();
  glViewport(0, 0, w(), h());
  glOrtho(0, w(), 0, h(), -1, 1);
#else
  GLint v[2];
  glGetIntegerv(GL_MAX_VIEWPORT_DIMS, v);
  glLoadIdentity();
  glViewport(pixel_w()-v[0], pixel_h()-v[1], v[0], v[1]);
  glOrtho(pixel_w()-v[0], pixel_w(), pixel_h()-v[1], pixel_h(), -1, 1);
#endif
}

/**
  The swap_buffers() method swaps the back and front buffers.
  It is called automatically after the draw() method is called.
*/
void Fl_Gl_Window::swap_buffers() {
#if defined(USE_X11)
  glXSwapBuffers(fl_display, fl_xid(this));
#elif defined(WIN32)
#  if HAVE_GL_OVERLAY
  // Do not swap the overlay, to match GLX:
  BOOL ret = wglSwapLayerBuffers(Fl_X::i(this)->private_dc, WGL_SWAP_MAIN_PLANE);
  DWORD err = GetLastError();;
#  else
  SwapBuffers(Fl_X::i(this)->private_dc);
#  endif
#elif defined(__APPLE_QUARTZ__)
  if(overlay != NULL) {
    // STR# 2944 [1]
    //    Save matrixmode/proj/modelview/rasterpos before doing overlay.
    //
    int wo=pixel_w(), ho=pixel_h();
    GLint matrixmode;
    GLfloat pos[4];
    glGetIntegerv(GL_MATRIX_MODE, &matrixmode);
    glGetFloatv(GL_CURRENT_RASTER_POSITION, pos);       // save original glRasterPos
    glMatrixMode(GL_PROJECTION);			// save proj/model matrices
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
  }
  /* // nothing to do here under Cocoa because [NSOpenGLContext -flushBuffer] done later replaces it
   else
    aglSwapBuffers((AGLContext)context_);
   */
#else
# error unsupported platform
#endif
}

#if HAVE_GL_OVERLAY && defined(WIN32)
uchar fl_overlay; // changes how fl_color() works
int fl_overlay_depth = 0;
#endif


void Fl_Gl_Window::flush() {
  if (!shown()) return;
  uchar save_valid = valid_f_ & 1;
#if HAVE_GL_OVERLAY && defined(WIN32)
  uchar save_valid_f = valid_f_;
#endif
  
#ifdef __APPLE__
  Fl_X *i = Fl_X::i(this);
  if (i->wait_for_expose) {
    Fl_X::GLcontext_update((GLContext)context());
    i->wait_for_expose = 0;
  }
#endif

#if HAVE_GL_OVERLAY && defined(WIN32)

  // Draw into hardware overlay planes if they are damaged:
  if (overlay && overlay != this
      && (damage()&(FL_DAMAGE_OVERLAY|FL_DAMAGE_EXPOSE) || !save_valid)) {
    fl_set_gl_context(this, (GLContext)overlay);
    if (fl_overlay_depth)
      wglRealizeLayerPalette(Fl_X::i(this)->private_dc, 1, TRUE);
    glDisable(GL_SCISSOR_TEST);
    glClear(GL_COLOR_BUFFER_BIT);
    fl_overlay = 1;
    draw_overlay();
    fl_overlay = 0;
    valid_f_ = save_valid_f;
    wglSwapLayerBuffers(Fl_X::i(this)->private_dc, WGL_SWAP_OVERLAY1);
    // if only the overlay was damaged we are done, leave main layer alone:
    if (damage() == FL_DAMAGE_OVERLAY) {
      return;
    }
  }
#endif

  make_current();

  if (mode_ & FL_DOUBLE) {

    glDrawBuffer(GL_BACK);

    if (!SWAP_TYPE) {
#if defined (__APPLE_QUARTZ__) || defined (USE_X11)
      SWAP_TYPE = COPY;
#else
      SWAP_TYPE = UNDEFINED;
#endif
      const char* c = fl_getenv("GL_SWAP_TYPE");
      if (c) {
	if (!strcmp(c,"COPY")) SWAP_TYPE = COPY;
	else if (!strcmp(c, "NODAMAGE")) SWAP_TYPE = NODAMAGE;
	else if (!strcmp(c, "SWAP")) SWAP_TYPE = SWAP;
	else SWAP_TYPE = UNDEFINED;
      }
    }

    if (SWAP_TYPE == NODAMAGE) {

      // don't draw if only overlay damage or expose events:
      if ((damage()&~(FL_DAMAGE_OVERLAY|FL_DAMAGE_EXPOSE)) || !save_valid)
	draw();
      swap_buffers();

    } else if (SWAP_TYPE == COPY) {

      // don't draw if only the overlay is damaged:
      if (damage() != FL_DAMAGE_OVERLAY || !save_valid) draw();
	  swap_buffers();

    } else if (SWAP_TYPE == SWAP){
      damage(FL_DAMAGE_ALL);
      draw();
      if (overlay == this) draw_overlay();
      swap_buffers();
    } else if (SWAP_TYPE == UNDEFINED){ // SWAP_TYPE == UNDEFINED

      // If we are faking the overlay, use CopyPixels to act like
      // SWAP_TYPE == COPY.  Otherwise overlay redraw is way too slow.
      if (overlay == this) {
	// don't draw if only the overlay is damaged:
	if (damage1_ || damage() != FL_DAMAGE_OVERLAY || !save_valid) draw();
	// we use a separate context for the copy because rasterpos must be 0
	// and depth test needs to be off:
	static GLContext ortho_context = 0;
	static Fl_Gl_Window* ortho_window = 0;
	int orthoinit = !ortho_context;
	if (orthoinit) ortho_context = fl_create_gl_context(this, g);
	fl_set_gl_context(this, ortho_context);
	if (orthoinit || !save_valid || ortho_window != this) {
	  glDisable(GL_DEPTH_TEST);
	  glReadBuffer(GL_BACK);
	  glDrawBuffer(GL_FRONT);
	  glLoadIdentity();
	  glViewport(0, 0, pixel_w(), pixel_h());
	  glOrtho(0, pixel_w(), 0, pixel_h(), -1, 1);
	  glRasterPos2i(0,0);
	  ortho_window = this;
	}
	glCopyPixels(0,0,pixel_w(),pixel_h(),GL_COLOR);
	make_current(); // set current context back to draw overlay
	damage1_ = 0;

      } else {
        damage1_ = damage();
        clear_damage(0xff); draw();
        swap_buffers();
      }

    }
#ifdef __APPLE__
    Fl_X::GLcontext_flushbuffer(context_);
#endif

    if (overlay==this && SWAP_TYPE != SWAP) { // fake overlay in front buffer
      glDrawBuffer(GL_FRONT);
      draw_overlay();
      glDrawBuffer(GL_BACK);
      glFlush();
    }

  } else {	// single-buffered context is simpler:

    draw();
    if (overlay == this) draw_overlay();
    glFlush();

  }

  valid(1);
  context_valid(1);
}

void Fl_Gl_Window::resize(int X,int Y,int W,int H) {
//  printf("Fl_Gl_Window::resize(X=%d, Y=%d, W=%d, H=%d)\n", X, Y, W, H);
//  printf("current: x()=%d, y()=%d, w()=%d, h()=%d\n", x(), y(), w(), h());

  int is_a_resize = (W != Fl_Widget::w() || H != Fl_Widget::h());
  if (is_a_resize) valid(0);
  
#ifdef __APPLE__
  Fl_X *flx = Fl_X::i(this);
  if (flx && flx->in_windowDidResize()) Fl_X::GLcontext_update(context_);
#endif

#if ! ( defined(__APPLE__) || defined(WIN32) )
  if (is_a_resize && !resizable() && overlay && overlay != this) {
    ((Fl_Gl_Window*)overlay)->resize(0,0,W,H);
  }
#endif

  Fl_Window::resize(X,Y,W,H);
}

/**
  Sets a pointer to the GLContext that this window is using.
  This is a system-dependent structure, but it is portable to copy
  the context from one window to another. You can also set it to NULL,
  which will force FLTK to recreate the context the next time make_current()
  is called, this is useful for getting around bugs in OpenGL implementations.
  
  If <i>destroy_flag</i> is true the context will be destroyed by
  fltk when the window is destroyed, or when the mode() is changed, 
  or the next time context(x) is called.
*/
void Fl_Gl_Window::context(void* v, int destroy_flag) {
  if (context_ && !(mode_&NON_LOCAL_CONTEXT)) fl_delete_gl_context(context_);
  context_ = (GLContext)v;
  if (destroy_flag) mode_ &= ~NON_LOCAL_CONTEXT;
  else mode_ |= NON_LOCAL_CONTEXT;
}    

/**
  Hides the window and destroys the OpenGL context.
*/
void Fl_Gl_Window::hide() {
  context(0);
#if HAVE_GL_OVERLAY && defined(WIN32)
  if (overlay && overlay != this) {
    fl_delete_gl_context((GLContext)overlay);
    overlay = 0;
  }
#endif
  Fl_Window::hide();
}

/**
  The destructor removes the widget and destroys the OpenGL context
  associated with it.
*/
Fl_Gl_Window::~Fl_Gl_Window() {
  hide();
//  delete overlay; this is done by ~Fl_Group
}

void Fl_Gl_Window::init() {
  end(); // we probably don't want any children
  box(FL_NO_BOX);

  mode_    = FL_RGB | FL_DEPTH | FL_DOUBLE;
  alist    = 0;
  context_ = 0;
  g        = 0;
  overlay  = 0;
  valid_f_ = 0;
  damage1_ = 0;

#if 0 // This breaks resizing on Linux/X11
  int H = h();
  h(1); // Make sure we actually do something in resize()...
  resize(x(), y(), w(), H);
#endif // 0
}

/**
  You must implement this virtual function if you want to draw into the
  overlay.  The overlay is cleared before this is called.  You should
  draw anything that is not clear using OpenGL.  You must use 
  gl_color(i) to choose colors (it allocates them from the colormap
  using system-specific calls), and remember that you are in an indexed
  OpenGL mode and drawing anything other than flat-shaded will probably
  not work.

  Both this function and Fl_Gl_Window::draw() should check 
  Fl_Gl_Window::valid() and set the same transformation.  If you
  don't your code may not work on other systems.  Depending on the OS,
  and on whether overlays are real or simulated, the OpenGL context may
  be the same or different between the overlay and main window.
*/
void Fl_Gl_Window::draw_overlay() {}

#endif

  /**
  You \e \b must subclass Fl_Gl_Window and provide an implementation for 
  draw().  You may also provide an implementation of draw_overlay()
  if you want to draw into the overlay planes.  You can avoid
  reinitializing the viewport and lights and other things by checking 
  valid() at the start of draw() and only doing the
  initialization if it is false.

  The draw() method can <I>only</I> use OpenGL calls.  Do not
  attempt to call X, any of the functions in <FL/fl_draw.H>, or glX
  directly.  Do not call gl_start() or gl_finish().

  If double-buffering is enabled in the window, the back and front
  buffers are swapped after this function is completed.
*/
void Fl_Gl_Window::draw() {
    Fl::fatal("Fl_Gl_Window::draw() *must* be overriden. Please refer to the documentation.");
}


/**
 Handle some FLTK events as needed.
 */
int Fl_Gl_Window::handle(int event) 
{
  return Fl_Window::handle(event);
}

// don't remove me! this serves only to force linking of Fl_Gl_Device_Plugin.o
int Fl_Gl_Window::gl_plugin_linkage() {
  return fl_gl_load_plugin;
}

//
// End of "$Id$".
//
