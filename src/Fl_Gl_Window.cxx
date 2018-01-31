//
// "$Id$"
//
// OpenGL window code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2018 by Bill Spitzak and others.
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

#include "config_lib.h"
#if HAVE_GL

extern int fl_gl_load_plugin;

#include <FL/gl.h>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Gl_Window_Driver.H>
#include <FL/Fl_Window_Driver.H>
#include <FL/Fl_Graphics_Driver.H>
#include <stdlib.h>
#include <FL/fl_utf8.h>
#  if (HAVE_DLSYM && HAVE_DLFCN_H)
#    include <dlfcn.h>
#  endif // (HAVE_DLSYM && HAVE_DLFCN_H)
#  ifdef HAVE_GLXGETPROCADDRESSARB
#    define GLX_GLXEXT_LEGACY
#    include <GL/glx.h>
#  endif // HAVE_GLXGETPROCADDRESSARB


#ifdef FL_CFG_GFX_OPENGL
#include "drivers/OpenGL/Fl_OpenGL_Display_Device.H"
#endif

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
  return Fl_Gl_Window_Driver::global()->find(a,b) != 0;
}

void Fl_Gl_Window::show() {
  int need_redraw = 0;
  if (!shown()) {
    if (!g) {
      g = pGlWindowDriver->find(mode_,alist);
      if (!g && (mode_ & FL_DOUBLE) == FL_SINGLE) {
        g = pGlWindowDriver->find(mode_ | FL_DOUBLE,alist);
	if (g) mode_ |= FL_FAKE_SINGLE;
      }

      if (!g) {
        Fl::error("Insufficient GL support");
	return;
      }
    }
    pGlWindowDriver->before_show(need_redraw);
  }
  Fl_Window::show();
  pGlWindowDriver->after_show(need_redraw);
}


/**
  The invalidate() method turns off valid() and is
  equivalent to calling value(0).
*/
void Fl_Gl_Window::invalidate() {
  valid(0);
  context_valid(0);
  pGlWindowDriver->invalidate();
}

int Fl_Gl_Window::mode(int m, const int *a) {
  if (m == mode_ && a == alist) return 0;
  return pGlWindowDriver->mode_(m, a);
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
  pGlWindowDriver->make_current_before();
  if (!context_) {
    mode_ &= ~NON_LOCAL_CONTEXT;
    context_ = pGlWindowDriver->create_gl_context(this, g);
    valid(0);
    context_valid(0);
  }
  pGlWindowDriver->set_gl_context(this, context_);
  pGlWindowDriver->make_current_after();
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
  pGlWindowDriver->swap_buffers();
}

void Fl_Gl_Window::flush() {
  if (!shown()) return;
  uchar save_valid = valid_f_ & 1;
  if (pGlWindowDriver->flush_begin(valid_f_) ) return;
  make_current();

  if (mode_ & FL_DOUBLE) {

    glDrawBuffer(GL_BACK);

    if (!SWAP_TYPE) {
      SWAP_TYPE = pGlWindowDriver->swap_type();
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
	if (orthoinit) ortho_context = pGlWindowDriver->create_gl_context(this, g);
	pGlWindowDriver->set_gl_context(this, ortho_context);
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
    pGlWindowDriver->flush_context();
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

  int is_a_resize = (W != Fl_Widget::w() || H != Fl_Widget::h() || Fl_Window_Driver::is_a_rescale());
  if (is_a_resize) valid(0);
  pGlWindowDriver->resize(is_a_resize, W, H);
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
void Fl_Gl_Window::context(GLContext v, int destroy_flag) {
  if (context_ && !(mode_&NON_LOCAL_CONTEXT)) pGlWindowDriver->delete_gl_context(context_);
  context_ = v;
  if (destroy_flag) mode_ &= ~NON_LOCAL_CONTEXT;
  else mode_ |= NON_LOCAL_CONTEXT;
}    

/**
  Hides the window and destroys the OpenGL context.
*/
void Fl_Gl_Window::hide() {
  context(0);
  pGlWindowDriver->hide_overlay(overlay);
  Fl_Window::hide();
}

/**
  The destructor removes the widget and destroys the OpenGL context
  associated with it.
*/
Fl_Gl_Window::~Fl_Gl_Window() {
  hide();
//  delete overlay; this is done by ~Fl_Group
  delete pGlWindowDriver;
}

void Fl_Gl_Window::init() {
  pGlWindowDriver = Fl_Gl_Window_Driver::newGlWindowDriver(this);
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

#endif // HAVE_GL

/** Draws the Fl_Gl_Window.   
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

  The following pseudo-code shows how to use "if (!valid())"
  to initialize the viewport:

  \code
    void mywindow::draw() {
     if (!valid()) {
         glViewport(0,0,pixel_w(),pixel_h());
         glFrustum(...) or glOrtho(...)
         ...other initialization...
     }
     if (!context_valid()) {
         ...load textures, etc. ...
     }
     // clear screen
     glClearColor(...);
     glClear(...);
     ... draw your geometry here ...
    }
  \endcode

  Actual example code to clear screen to black and draw a 2D white "X":
  \code
    void mywindow::draw() {
        if (!valid()) {
            glLoadIdentity();
            glViewport(0,0,w(),h());
            glOrtho(-w(),w(),-h(),h(),-1,1);
        }
        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT);
        // Draw white 'X'
        glColor3f(1.0, 1.0, 1.0);
        glBegin(GL_LINE_STRIP); glVertex2f(w(), h()); glVertex2f(-w(),-h()); glEnd();
        glBegin(GL_LINE_STRIP); glVertex2f(w(),-h()); glVertex2f(-w(), h()); glEnd();
    } 
  \endcode

*/
void Fl_Gl_Window::draw() {
#ifdef FL_CFG_GFX_OPENGL
  Fl_Surface_Device::push_current( Fl_OpenGL_Display_Device::display_device() );
  glPushAttrib(GL_ENABLE_BIT);
  glDisable(GL_DEPTH_TEST);
  glPushMatrix();
  glLoadIdentity();
  GLint viewport[4];
  glGetIntegerv (GL_VIEWPORT, viewport);
  if (viewport[2] != pixel_w() || viewport[3] != pixel_h()) {
    glViewport(0, 0, pixel_w(), pixel_h());
  }
  glOrtho(-0.5, w()-0.5, h()-0.5, -0.5, -1, 1);
//  glOrtho(0, w(), h(), 0, -1, 1);
  glLineWidth((GLfloat)pixels_per_unit()); // should be 1 or 2 (2 if highres OpenGL)
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // FIXME: push on state stack
  glEnable(GL_BLEND); // FIXME: push on state stack
  
  Fl_Window::draw();

  glPopMatrix();
  glPopAttrib();
  Fl_Surface_Device::pop_current();
#else
  Fl::fatal("Fl_Gl_Window::draw() *must* be overriden. Please refer to the documentation.");
#endif
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

/** The number of pixels per FLTK unit of length for the window.
 Returns 1, except for a window mapped to
 an Apple 'retina' display, and if Fl::use_high_res_GL(bool) is set to true,
 when it returns 2. This method dynamically adjusts its value when the window
 is moved to/from a retina display. This method is useful, e.g., to convert,
 in a window's handle() method, the FLTK units returned by Fl::event_x() and
 Fl::event_y() to the pixel units used by the OpenGL source code.
 \version 1.3.4
 */
float Fl_Gl_Window::pixels_per_unit() {
  return pGlWindowDriver->pixels_per_unit();
}

// creates a unique, dummy Fl_Gl_Window_Driver object used when no Fl_Gl_Window is around
// necessary to support gl_start()/gl_finish()
Fl_Gl_Window_Driver *Fl_Gl_Window_Driver::global() {
  static Fl_Gl_Window_Driver *gwd = newGlWindowDriver(NULL);
  return gwd;
}

void Fl_Gl_Window_Driver::invalidate() {
  if (pWindow->overlay) {
    ((Fl_Gl_Window*)pWindow->overlay)->valid(0);
    ((Fl_Gl_Window*)pWindow->overlay)->context_valid(0);
  }
}


char Fl_Gl_Window_Driver::swap_type() {return UNDEFINED;}


void* Fl_Gl_Window_Driver::GetProcAddress(const char *procName) {
#if (HAVE_DLSYM && HAVE_DLFCN_H)
  char symbol[1024];
  
  snprintf(symbol, sizeof(symbol), "_%s", procName);
  
#    ifdef RTLD_DEFAULT
  return dlsym(RTLD_DEFAULT, symbol);
  
#    else // No RTLD_DEFAULT support, so open the current a.out symbols...
  static void *rtld_default = dlopen(0, RTLD_LAZY);
  
  if (rtld_default) return dlsym(rtld_default, symbol);
  else return 0;
  
#    endif // RTLD_DEFAULT
  
#elif defined(HAVE_GLXGETPROCADDRESSARB)
  return glXGetProcAddressARB((const GLubyte *)procName);
  
#else
  return 0;
#endif // HAVE_DLSYM
}


#ifdef FL_CFG_GFX_QUARTZ
#include <FL/platform.H>
#include <OpenGL/OpenGL.h>
#include "drivers/Cocoa/Fl_Cocoa_Window_Driver.H"

Fl_Gl_Window_Driver *Fl_Gl_Window_Driver::newGlWindowDriver(Fl_Gl_Window *w)
{
  return new Fl_Cocoa_Gl_Window_Driver(w);
}

void Fl_Cocoa_Gl_Window_Driver::before_show(int& need_redraw) {
  if( ! pWindow->parent() ) need_redraw=1;
}

void Fl_Cocoa_Gl_Window_Driver::after_show(int need_redraw) {
  pWindow->set_visible();
  if(need_redraw) pWindow->redraw();//necessary only after creation of a top-level GL window
}

float Fl_Cocoa_Gl_Window_Driver::pixels_per_unit()
{
  int retina = (fl_mac_os_version >= 100700 && Fl::use_high_res_GL() && Fl_X::i(pWindow) &&
          Fl_Cocoa_Window_Driver::driver(pWindow)->mapped_to_retina()) ? 2 : 1;
  return retina * Fl_Graphics_Driver::default_driver().scale();
}

int Fl_Cocoa_Gl_Window_Driver::mode_(int m, const int *a) {
  if (a) { // when the mode is set using the a array of system-dependent values, and if asking for double buffer,
    // the FL_DOUBLE flag must be set in the mode_ member variable
    const int *aa = a;
    while (*aa) {
      if (*(aa++) ==
          kCGLPFADoubleBuffer
          ) { m |= FL_DOUBLE; break; }
    }
  }
  pWindow->context(0);
  mode( m); alist(a);
  if (pWindow->shown()) {
    g( find(m, a) );
    pWindow->redraw();
  } else {
    g(0);
  }
  return 1;
}

void Fl_Cocoa_Gl_Window_Driver::make_current_before() {
  // detect if the window was moved between low and high resolution displays
  Fl_Cocoa_Window_Driver *d = Fl_Cocoa_Window_Driver::driver(pWindow);
  if (d->changed_resolution()){
    d->changed_resolution(false);
    invalidate();
    GLcontext_update(pWindow->context());
  }
}

void Fl_Cocoa_Gl_Window_Driver::swap_buffers() {
  if (overlay() != NULL) {
    // STR# 2944 [1]
    //    Save matrixmode/proj/modelview/rasterpos before doing overlay.
    //
    int wo = pWindow->pixel_w(), ho = pWindow->pixel_h();
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
}

void Fl_Cocoa_Gl_Window_Driver::resize(int is_a_resize, int unused, int also) {
  Fl_X *flx = Fl_X::i(pWindow);
  Fl_Cocoa_Window_Driver *d = Fl_Cocoa_Window_Driver::driver(pWindow);
  if (flx && d->in_windowDidResize()) GLcontext_update(pWindow->context());
}

char Fl_Cocoa_Gl_Window_Driver::swap_type() {return COPY;}

#endif // FL_CFG_GFX_QUARTZ

#if defined(FL_CFG_GFX_GDI)
#include "drivers/WinAPI/Fl_WinAPI_Window_Driver.H"
#include <FL/platform.H>
#include <FL/Fl_Graphics_Driver.H>
#include <FL/Fl_Screen_Driver.H>

Fl_Gl_Window_Driver *Fl_Gl_Window_Driver::newGlWindowDriver(Fl_Gl_Window *w)
{
  return new Fl_WinAPI_Gl_Window_Driver(w);
}

float Fl_WinAPI_Gl_Window_Driver::pixels_per_unit()
{
  int ns = pWindow->driver()->screen_num();
  return Fl::screen_driver()->scale(ns);
}


int Fl_WinAPI_Gl_Window_Driver::mode_(int m, const int *a) {
  int oldmode = mode();
  pWindow->context(0);
  mode( m); alist(a);
  if (pWindow->shown()) {
    g( find(m, a) );
    if (!g() || (oldmode^m)&(FL_DOUBLE|FL_STEREO)) {
      pWindow->hide();
      pWindow->show();
    }
  } else {
    g(0);
  }
  return 1;
}

void Fl_WinAPI_Gl_Window_Driver::make_current_after() {
#if USE_COLORMAP
  if (fl_palette) {
    fl_GetDC(fl_xid(pWindow));
    SelectPalette((HDC)fl_graphics_driver->gc(), fl_palette, FALSE);
    RealizePalette((HDC)fl_graphics_driver->gc());
  }
#endif // USE_COLORMAP
}

//#define HAVE_GL_OVERLAY 1 //test only

void Fl_WinAPI_Gl_Window_Driver::swap_buffers() {
#  if HAVE_GL_OVERLAY
  // Do not swap the overlay, to match GLX:
  BOOL ret = wglSwapLayerBuffers(Fl_WinAPI_Window_Driver::driver(pWindow)->private_dc, WGL_SWAP_MAIN_PLANE);
  DWORD err = GetLastError();
#  else
  SwapBuffers(Fl_WinAPI_Window_Driver::driver(pWindow)->private_dc);
#  endif
}

#if HAVE_GL_OVERLAY
uchar fl_overlay; // changes how fl_color() works
int fl_overlay_depth = 0;
#endif

int Fl_WinAPI_Gl_Window_Driver::flush_begin(char& valid_f_) {
#if HAVE_GL_OVERLAY
  char save_valid_f = valid_f_;
  // Draw into hardware overlay planes if they are damaged:
  if (overlay() && overlay() != pWindow
      && (pWindow->damage()&(FL_DAMAGE_OVERLAY|FL_DAMAGE_EXPOSE) || !save_valid_f & 1)) {
    set_gl_context(pWindow, (GLContext)overlay());
    if (fl_overlay_depth)
      wglRealizeLayerPalette(Fl_WinAPI_Window_Driver::driver(pWindow)->private_dc, 1, TRUE);
    glDisable(GL_SCISSOR_TEST);
    glClear(GL_COLOR_BUFFER_BIT);
    fl_overlay = 1;
    draw_overlay();
    fl_overlay = 0;
    valid_f_ = save_valid_f;
    wglSwapLayerBuffers(Fl_WinAPI_Window_Driver::driver(pWindow)->private_dc, WGL_SWAP_OVERLAY1);
    // if only the overlay was damaged we are done, leave main layer alone:
    if (pWindow->damage() == FL_DAMAGE_OVERLAY) {
      return 1;
    }
  }
#endif
  return 0;
}

void* Fl_WinAPI_Gl_Window_Driver::GetProcAddress(const char *procName) {
  return (void*)wglGetProcAddress((LPCSTR)procName);
}

#endif // FL_CFG_GFX_GDI


#if defined(FL_CFG_GFX_XLIB)
#include <FL/platform.H>
#include "Fl_Gl_Choice.H"
#include <FL/Fl_Screen_Driver.H>
#include <FL/Fl_Window_Driver.H>

Fl_Gl_Window_Driver *Fl_Gl_Window_Driver::newGlWindowDriver(Fl_Gl_Window *w)
{
  return new Fl_X11_Gl_Window_Driver(w);
}

void Fl_X11_Gl_Window_Driver::before_show(int& need_redraw) {
  Fl_X::make_xid(pWindow, g()->vis, g()->colormap);
  if (overlay() && overlay() != pWindow) ((Fl_Gl_Window*)overlay())->show();
}

float Fl_X11_Gl_Window_Driver::pixels_per_unit()
{
  int ns = pWindow->driver()->screen_num();
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
  Fl_Gl_Choice* oldg = g();
  pWindow->context(0);
  mode(m); alist(a);
  if (pWindow->shown()) {
    g( find(m, a) );
    // under X, if the visual changes we must make a new X window (yuck!):
    if (!g() || g()->vis->visualid != oldg->vis->visualid || (oldmode^m)&FL_DOUBLE) {
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

char Fl_X11_Gl_Window_Driver::swap_type() {return COPY;}

void Fl_X11_Gl_Window_Driver::waitGL() {
  glXWaitGL();
}

#endif // FL_CFG_GFX_XLIB

//
// End of "$Id$".
//
