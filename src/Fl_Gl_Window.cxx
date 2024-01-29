//
// OpenGL window code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2022 by Bill Spitzak and others.
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

extern int fl_gl_load_plugin;

#include <FL/gl.h>
#include <FL/Fl_Gl_Window.H>
#include "Fl_Gl_Window_Driver.H"
#include "Fl_Window_Driver.H"
#include <FL/Fl_Graphics_Driver.H>
#include <FL/fl_utf8.h>
#include "drivers/OpenGL/Fl_OpenGL_Display_Device.H"
#include "drivers/OpenGL/Fl_OpenGL_Graphics_Driver.H"

#include <stdlib.h>
#  if (HAVE_DLSYM && HAVE_DLFCN_H)
#    include <dlfcn.h>
#  endif // (HAVE_DLSYM && HAVE_DLFCN_H)
#  ifdef HAVE_GLXGETPROCADDRESSARB
#    define GLX_GLXEXT_LEGACY
#    include <GL/glx.h>
#  endif // HAVE_GLXGETPROCADDRESSARB


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
#define UNDEFINED 1     // anything
#define SWAP 2          // former front buffer (same as unknown)
#define COPY 3          // unchanged
#define NODAMAGE 4      // unchanged even by X expose() events

static char SWAP_TYPE = 0 ; // 0 = determine it from environment variable


/**  Returns non-zero if the hardware supports the given or current OpenGL  mode. */
int Fl_Gl_Window::can_do(int a, const int *b) {
  return Fl_Gl_Window_Driver::global()->find(a,b) != 0;
}

void Fl_Gl_Window::show() {
  int need_after = 0;
  if (!shown()) {
    Fl_Window::default_size_range();
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
    pGlWindowDriver->before_show(need_after);
  }
  Fl_Window::show();
  if (need_after) pGlWindowDriver->after_show();
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

/**
 Sets the rate at which the GL windows swaps buffers.
 This method can be called after the OpenGL context was created, typically
 within the user overridden `Fl_Gl_Window::draw()` method that will be
 overridden by the user.
 \note This method depends highly on the underlying OpenGL contexts and driver
    implementation. Most driver seem to accept only 0 and 1 to swap buffer
    asynchronously or in sync with the vertical blank.
 \param[in] frames set the number of vertical frame blanks between OpenGL
    buffer swaps
 */
void Fl_Gl_Window::swap_interval(int frames) {
  pGlWindowDriver->swap_interval(frames);
}

/**
 Gets the rate at which the GL windows swaps buffers.
 This method can be called after the OpenGL context was created, typically
 within the user overridden `Fl_Gl_Window::draw()` method that will be
 overridden by the user.
 \note This method depends highly on the underlying OpenGL contexts and driver
    implementation. Some drivers return no information, most drivers don't
    support intervals with multiple frames and return only 0 or 1.
 \note Some drivers have the ability to set the swap interval but no way
    to query it, hence this method may return -1 even though the interval was
    set correctly. Conversely a return value greater zero does not guarantee
    that the driver actually honors the setting.
 \return an integer greater zero if vertical blanking is taken into account
    when swapping OpenGL buffers
 \return 0 if the vertical blanking is ignored
 \return -1 if the information can not be retrieved
 */
int Fl_Gl_Window::swap_interval() const {
  return pGlWindowDriver->swap_interval();
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
    if (overlay==this && SWAP_TYPE != SWAP) { // fake overlay in front buffer
      glDrawBuffer(GL_FRONT);
      draw_overlay();
      glDrawBuffer(GL_BACK);
      glFlush();
    }

  } else {      // single-buffered context is simpler:

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

  int is_a_resize = (W != Fl_Widget::w() || H != Fl_Widget::h() || is_a_rescale());
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
  pGlWindowDriver->gl_hide_before(overlay);
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

/**
 Supports drawing to an Fl_Gl_Window with the FLTK 2D drawing API.
 \see \ref opengl_with_fltk_widgets
 */
void Fl_Gl_Window::draw_begin() {
  if (mode() & FL_OPENGL3) pGlWindowDriver->switch_to_GL1();
  damage(FL_DAMAGE_ALL); // always redraw all GL widgets above the GL scene
  Fl_Surface_Device::push_current( Fl_OpenGL_Display_Device::display_device() );
  Fl_OpenGL_Graphics_Driver *drv = (Fl_OpenGL_Graphics_Driver*)Fl_Surface_Device::surface()->driver();
  drv->pixels_per_unit_ = pixels_per_unit();

  if (!valid()) {
    glViewport(0, 0, pixel_w(), pixel_h());
    valid(1);
  }

  glPushAttrib(GL_ALL_ATTRIB_BITS);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0.0, w(), h(), 0.0, -1.0, 1.0);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);
  glEnable(GL_POINT_SMOOTH);

  glLineWidth((GLfloat)(drv->pixels_per_unit_*drv->line_width_));
  glPointSize((GLfloat)(drv->pixels_per_unit_));
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  if (!pGlWindowDriver->need_scissor()) glDisable(GL_SCISSOR_TEST);
}

/**
 To be used as a match for a previous call to Fl_Gl_Window::draw_begin().
 \see \ref opengl_with_fltk_widgets
 */
void Fl_Gl_Window::draw_end() {
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glPopAttrib(); // GL_ALL_ATTRIB_BITS

  Fl_Surface_Device::pop_current();
  if (mode() & FL_OPENGL3) pGlWindowDriver->switch_back();
}

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
            glViewport(0,0,pixel_w(),pixel_h());
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

  Regular FLTK widgets can be added as children to the Fl_Gl_Window. To
  correctly overlay the widgets, Fl_Gl_Window::draw() must be called after
  rendering the main scene.
  \code
  void mywindow::draw() {
    // draw 3d graphics scene
    Fl_Gl_Window::draw();
    // -- or --
    draw_begin();
    Fl_Window::draw();
    // other 2d drawing calls, overlays, etc.
    draw_end();
  }
  \endcode

*/
void Fl_Gl_Window::draw() {
  draw_begin();
  Fl_Window::draw();
  draw_end();
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
 This method dynamically adjusts its value when the GUI is rescaled or when the window
 is moved to/from displays of distinct resolutions. This method is useful, e.g., to convert,
 in a window's handle() method, the FLTK units returned by Fl::event_x() and
 Fl::event_y() to the pixel units used by the OpenGL source code.
 \version 1.3.4
 */
float Fl_Gl_Window::pixels_per_unit() {
  return pGlWindowDriver->pixels_per_unit();
}

/**
 \cond DriverDev
 \addtogroup DriverDeveloper
 \{
 */

int Fl_Gl_Window_Driver::copy = COPY;
Fl_Window* Fl_Gl_Window_Driver::cached_window = NULL;
float Fl_Gl_Window_Driver::gl_scale = 1; // scaling factor between FLTK and GL drawing units: GL = FLTK * gl_scale

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
#if defined(HAVE_GLXGETPROCADDRESSARB)
  return (void*)glXGetProcAddressARB((const GLubyte *)procName);

#elif (HAVE_DLSYM && HAVE_DLFCN_H)
#  ifdef RTLD_DEFAULT
      void *rtld_default = RTLD_DEFAULT;
#  else
      static void *rtld_default = dlopen(0, RTLD_LAZY);
#  endif
  char symbol[1024];
  snprintf(symbol, sizeof(symbol), "_%s", procName);
  return dlsym(rtld_default, symbol);

#endif // HAVE_DLSYM
  return NULL;
}

Fl_Font_Descriptor** Fl_Gl_Window_Driver::fontnum_to_fontdescriptor(int fnum) {
  extern FL_EXPORT Fl_Fontdesc *fl_fonts;
  return &(fl_fonts[fnum].first);
}

/* Captures a rectangle of a Fl_Gl_Window and returns it as an RGB image.
 This is the platform-independent version. Some platforms may override it.
 */
Fl_RGB_Image* Fl_Gl_Window_Driver::capture_gl_rectangle(int x, int y, int w, int h)
{
  Fl_Gl_Window *glw = pWindow;
  glw->flush(); // forces a GL redraw, necessary for the glpuzzle demo
  // Read OpenGL context pixels directly.
  // For extra safety, save & restore OpenGL states that are changed
  glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
  glPixelStorei(GL_PACK_ALIGNMENT, 4); /* Force 4-byte alignment */
  glPixelStorei(GL_PACK_ROW_LENGTH, 0);
  glPixelStorei(GL_PACK_SKIP_ROWS, 0);
  glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
  //
  float s = glw->pixels_per_unit();
  if (s != 1) {
    x = int(x * s); y = int(y * s); w = int(w * s); h = int(h * s);
  }
  // Read a block of pixels from the frame buffer
  int mByteWidth = w * 3;
  mByteWidth = (mByteWidth + 3) & ~3;    // Align to 4 bytes
  uchar *baseAddress = new uchar[mByteWidth * h];
  glReadPixels(x, glw->pixel_h() - (y+h), w, h,
               GL_RGB, GL_UNSIGNED_BYTE,
               baseAddress);
  glPopClientAttrib();
  // GL gives a bottom-to-top image, convert it to top-to-bottom
  uchar *tmp = new uchar[mByteWidth];
  uchar *p = baseAddress ;
  uchar *q = baseAddress + (h-1)*mByteWidth;
  for (int i = 0; i < h/2; i++, p += mByteWidth, q -= mByteWidth) {
    memcpy(tmp, p, mByteWidth);
    memcpy(p, q, mByteWidth);
    memcpy(q, tmp, mByteWidth);
  }
  delete[] tmp;

  Fl_RGB_Image *img = new Fl_RGB_Image(baseAddress, w, h, 3, mByteWidth);
  img->alloc_array = 1;
  return img;
}

/**
 \}
 \endcond
 */

#endif // HAVE_GL
