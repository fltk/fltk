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
#include <FL/gl.h>
#include "../../Fl_Gl_Choice.H"
#include "../../Fl_Screen_Driver.H"
#include "Fl_Cocoa_Window_Driver.H"
#include "../../Fl_Gl_Window_Driver.H"
#include <FL/Fl_Graphics_Driver.H>
#include <OpenGL/OpenGL.h>
#include <FL/Fl_Image_Surface.H>

extern void gl_texture_reset();

#ifdef __OBJC__
@class NSOpenGLPixelFormat;
#else
class NSOpenGLPixelFormat;
#endif // __OBJC__

class Fl_Cocoa_Gl_Window_Driver : public Fl_Gl_Window_Driver {
  friend class Fl_Gl_Window_Driver;
  friend class Fl_OpenGL_Display_Device;
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
};

// Describes crap needed to create a GLContext.
class Fl_Cocoa_Gl_Choice : public Fl_Gl_Choice {
  friend class Fl_Cocoa_Gl_Window_Driver;
private:
  NSOpenGLPixelFormat* pixelformat;
public:
  Fl_Cocoa_Gl_Choice(int m, const int *alistp, Fl_Gl_Choice *n) : Fl_Gl_Choice(m, alistp, n) {
    pixelformat = NULL;
  }
};


Fl_Gl_Choice *Fl_Cocoa_Gl_Window_Driver::find(int m, const int *alistp)
{
  Fl::screen_driver()->open_display(); // useful when called through gl_start()
  Fl_Cocoa_Gl_Choice *g = (Fl_Cocoa_Gl_Choice*)Fl_Gl_Window_Driver::find_begin(m, alistp);
  if (g) return g;
  NSOpenGLPixelFormat* fmt = Fl_Cocoa_Window_Driver::mode_to_NSOpenGLPixelFormat(m, alistp);
  if (!fmt) return 0;
  g = new Fl_Cocoa_Gl_Choice(m, alistp, first);
  first = g;
  g->pixelformat = fmt;
  return g;
}

GLContext Fl_Cocoa_Gl_Window_Driver::create_gl_context(Fl_Window* window, const Fl_Gl_Choice* g, int layer) {
  GLContext context, shared_ctx = 0;
  if (context_list && nContext) shared_ctx = context_list[0];
  // resets the pile of string textures used to draw strings
  // necessary before the first context is created
  if (!shared_ctx) gl_texture_reset();
  context = Fl_Cocoa_Window_Driver::create_GLcontext_for_window(((Fl_Cocoa_Gl_Choice*)g)->pixelformat, shared_ctx, window);
  if (!context) return 0;
  add_context(context);
  return (context);
}

void Fl_Cocoa_Gl_Window_Driver::set_gl_context(Fl_Window* w, GLContext context) {
  if (context != cached_context || w != cached_window) {
    cached_context = context;
    cached_window = w;
    Fl_Cocoa_Window_Driver::GLcontext_makecurrent(context);
  }
}

void Fl_Cocoa_Gl_Window_Driver::delete_gl_context(GLContext context) {
  if (cached_context == context) {
    cached_context = 0;
    cached_window = 0;
    Fl_Cocoa_Window_Driver::GL_cleardrawable();
  }
  Fl_Cocoa_Window_Driver::GLcontext_release(context);
  del_context(context);
}

void Fl_Cocoa_Gl_Window_Driver::make_overlay_current() {
  // this is not very useful, but unfortunately, Apple decided
  // that front buffer drawing can no longer (OS X 10.4) be supported on their platforms.
  pWindow->make_current();
}

void Fl_Cocoa_Gl_Window_Driver::redraw_overlay() {
  pWindow->redraw();
}


Fl_Gl_Window_Driver *Fl_Gl_Window_Driver::newGlWindowDriver(Fl_Gl_Window *w)
{
  return new Fl_Cocoa_Gl_Window_Driver(w);
}

void Fl_Cocoa_Gl_Window_Driver::before_show(int& need_after) {
  need_after = 1;
}

void Fl_Cocoa_Gl_Window_Driver::after_show() {
  // Makes sure the GL context is created to avoid drawing twice the window when first shown
  pWindow->make_current();
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
    pWindow->invalidate();
    Fl_Cocoa_Window_Driver::GLcontext_update(pWindow->context());
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
  }
   else
     Fl_Cocoa_Window_Driver::flush_context(pWindow->context());//aglSwapBuffers((AGLContext)context_);
}

char Fl_Cocoa_Gl_Window_Driver::swap_type() {return copy;}

void Fl_Cocoa_Gl_Window_Driver::resize(int is_a_resize, int w, int h) {
  Fl_Cocoa_Window_Driver::GLcontext_update(pWindow->context());
}

/* Some old Apple hardware doesn't implement the GL_EXT_texture_rectangle extension.
 For it, draw_string_legacy_glut() is used to draw text. */

char *Fl_Cocoa_Gl_Window_Driver::alpha_mask_for_string(const char *str, int n, int w, int h, Fl_Fontsize fs)
{
  // write str to a bitmap just big enough
  Fl_Image_Surface *surf = new Fl_Image_Surface(w, h);
  Fl_Font f=fl_font();
  Fl_Surface_Device::push_current(surf);
  fl_color(FL_WHITE);
  fl_font(f, fs);
  fl_draw(str, n, 0, fl_height() - fl_descent());
  // get the alpha channel only of the bitmap
  char *alpha_buf = new char[w*h], *r = alpha_buf, *q;
  q = (char*)CGBitmapContextGetData(surf->offscreen());
  for (int i = 0; i < h; i++) {
    for (int j = 0; j < w; j++) {
      *r++ = *(q+3);
      q += 4;
    }
  }
  Fl_Surface_Device::pop_current();
  delete surf;
  return alpha_buf;
}

void Fl_Cocoa_Gl_Window_Driver::gl_start() {
  Fl_Cocoa_Window_Driver::gl_start(gl_start_context);
}

// convert BGRA to RGB and also exchange top and bottom
static uchar *convert_BGRA_to_RGB(uchar *baseAddress, int w, int h, int mByteWidth)
{
  uchar *newimg = new uchar[3*w*h];
  uchar *to = newimg;
  for (int i = h-1; i >= 0; i--) {
    uchar *from = baseAddress + i * mByteWidth;
    for (int j = 0; j < w; j++, from += 4) {
#if defined(__ppc__) && __ppc__
      memcpy(to, from + 1, 3);
      to += 3;
#else
      *(to++) = *(from+2);
      *(to++) = *(from+1);
      *(to++) = *from;
#endif
    }
  }
  delete[] baseAddress;
  return newimg;
}

Fl_RGB_Image* Fl_Cocoa_Gl_Window_Driver::capture_gl_rectangle(int x, int y, int w, int h)
{
  Fl_Gl_Window* glw = pWindow;
  float factor = glw->pixels_per_unit();
  if (factor != 1) {
    w *= factor; h *= factor; x *= factor; y *= factor;
  }
  Fl_Cocoa_Window_Driver::GLcontext_makecurrent(glw->context());
  Fl_Cocoa_Window_Driver::flush_context(glw->context()); // to capture also the overlay and for directGL demo
  // Read OpenGL context pixels directly.
  // For extra safety, save & restore OpenGL states that are changed
  glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
  glPixelStorei(GL_PACK_ALIGNMENT, 4); /* Force 4-byte alignment */
  glPixelStorei(GL_PACK_ROW_LENGTH, 0);
  glPixelStorei(GL_PACK_SKIP_ROWS, 0);
  glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
  // Read a block of pixels from the frame buffer
  int mByteWidth = w * 4;
  mByteWidth = (mByteWidth + 3) & ~3;    // Align to 4 bytes
  uchar *baseAddress = new uchar[mByteWidth * h];
  glReadPixels(x, glw->pixel_h() - (y+h), w, h,
               GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, baseAddress);
  glPopClientAttrib();
  baseAddress = convert_BGRA_to_RGB(baseAddress, w, h, mByteWidth);
  Fl_RGB_Image *img = new Fl_RGB_Image(baseAddress, w, h, 3, 3 * w);
  img->alloc_array = 1;
  Fl_Cocoa_Window_Driver::flush_context(glw->context());
  return img;
}

#endif // HAVE_GL
