//
// Class Fl_Cocoa_Gl_Window_Driver for the Fast Light Tool Kit (FLTK).
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
#include <FL/gl.h>
#include "../../Fl_Gl_Choice.H"
#include "../../Fl_Screen_Driver.H"
#include "Fl_Cocoa_Window_Driver.H"
#include "Fl_Cocoa_Gl_Window_Driver.H"
#include <FL/Fl_Graphics_Driver.H>
#include <OpenGL/OpenGL.h>
#include <FL/Fl_Image_Surface.H>
#include <dlfcn.h>

#import <Cocoa/Cocoa.h>

/* macOS offers only core contexts when using GL3. This forbids to draw
 FLTK widgets in a GL3-using NSOpenGLContext because these widgets are drawn
 with the GL1-based Fl_OpenGL_Graphics_Driver. The solution implemented here
 is to create an additional NSView and an associated additional NSOpenGLContext
 (gl1ctxt) placed above and sized as the GL3-based window, to set the new
 NSOpenGLContext non opaque and GL1-based, and to draw the FLTK widgets in the
 new view/GL1 context.
 */

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


Fl_Cocoa_Gl_Window_Driver::Fl_Cocoa_Gl_Window_Driver(Fl_Gl_Window *win) :
                              Fl_Gl_Window_Driver(win) {
  gl1ctxt = NULL;
}


Fl_Gl_Window_Driver *Fl_Gl_Window_Driver::newGlWindowDriver(Fl_Gl_Window *w)
{
  return new Fl_Cocoa_Gl_Window_Driver(w);
}


static NSOpenGLPixelFormat* mode_to_NSOpenGLPixelFormat(int m, const int *alistp)
{
  NSOpenGLPixelFormatAttribute attribs[32];
  int n = 0;
  // AGL-style code remains commented out for comparison
  if (!alistp) {
    if (m & FL_INDEX) {
      //list[n++] = AGL_BUFFER_SIZE; list[n++] = 8;
    } else {
      //list[n++] = AGL_RGBA;
      //list[n++] = AGL_GREEN_SIZE;
      //list[n++] = (m & FL_RGB8) ? 8 : 1;
      attribs[n++] = NSOpenGLPFAColorSize;
      attribs[n++] = (NSOpenGLPixelFormatAttribute)((m & FL_RGB8) ? 32 : 1);
      if (m & FL_ALPHA) {
        //list[n++] = AGL_ALPHA_SIZE;
        attribs[n++] = NSOpenGLPFAAlphaSize;
        attribs[n++] = (NSOpenGLPixelFormatAttribute)((m & FL_RGB8) ? 8 : 1);
      }
      if (m & FL_ACCUM) {
        //list[n++] = AGL_ACCUM_GREEN_SIZE; list[n++] = 1;
        attribs[n++] = NSOpenGLPFAAccumSize;
        attribs[n++] = (NSOpenGLPixelFormatAttribute)1;
        if (m & FL_ALPHA) {
          //list[n++] = AGL_ACCUM_ALPHA_SIZE; list[n++] = 1;
        }
      }
    }
    if (m & FL_DOUBLE) {
      //list[n++] = AGL_DOUBLEBUFFER;
      attribs[n++] = NSOpenGLPFADoubleBuffer;
    }
    if (m & FL_DEPTH) {
      //list[n++] = AGL_DEPTH_SIZE; list[n++] = 24;
      attribs[n++] = NSOpenGLPFADepthSize;
      attribs[n++] = (NSOpenGLPixelFormatAttribute)24;
    }
    if (m & FL_STENCIL) {
      //list[n++] = AGL_STENCIL_SIZE; list[n++] = 1;
      attribs[n++] = NSOpenGLPFAStencilSize;
      attribs[n++] = (NSOpenGLPixelFormatAttribute)1;
    }
    if (m & FL_STEREO) {
      //list[n++] = AGL_STEREO;
      attribs[n++] = 6/*NSOpenGLPFAStereo*/;
    }
    if ((m & FL_MULTISAMPLE) && fl_mac_os_version >= 100400) {
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
      attribs[n++] = NSOpenGLPFAMultisample; // 10.4
#endif
      attribs[n++] = NSOpenGLPFASampleBuffers; attribs[n++] = (NSOpenGLPixelFormatAttribute)1;
      attribs[n++] = NSOpenGLPFASamples; attribs[n++] = (NSOpenGLPixelFormatAttribute)4;
    }
#if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_7
#define NSOpenGLPFAOpenGLProfile      (NSOpenGLPixelFormatAttribute)99
#define kCGLPFAOpenGLProfile          NSOpenGLPFAOpenGLProfile
#define NSOpenGLProfileVersionLegacy  (NSOpenGLPixelFormatAttribute)0x1000
#define NSOpenGLProfileVersion3_2Core  (NSOpenGLPixelFormatAttribute)0x3200
#define kCGLOGLPVersion_Legacy        NSOpenGLProfileVersionLegacy
#endif
    if (fl_mac_os_version >= 100700) {
      attribs[n++] = NSOpenGLPFAOpenGLProfile;
      attribs[n++] =  (m & FL_OPENGL3) ? NSOpenGLProfileVersion3_2Core : NSOpenGLProfileVersionLegacy;
    }
  } else {
    while (alistp[n] && n < 30) {
      if (alistp[n] == kCGLPFAOpenGLProfile) {
        if (fl_mac_os_version < 100700) {
          if (alistp[n+1] != kCGLOGLPVersion_Legacy) return nil;
          n += 2;
          continue;
        }
      }
      attribs[n] = (NSOpenGLPixelFormatAttribute)alistp[n];
      n++;
    }
  }
  attribs[n] = (NSOpenGLPixelFormatAttribute)0;
  NSOpenGLPixelFormat *pixform = [[NSOpenGLPixelFormat alloc] initWithAttributes:attribs];
  /*GLint color,alpha,accum,depth;
  [pixform getValues:&color forAttribute:NSOpenGLPFAColorSize forVirtualScreen:0];
  [pixform getValues:&alpha forAttribute:NSOpenGLPFAAlphaSize forVirtualScreen:0];
  [pixform getValues:&accum forAttribute:NSOpenGLPFAAccumSize forVirtualScreen:0];
  [pixform getValues:&depth forAttribute:NSOpenGLPFADepthSize forVirtualScreen:0];
  NSLog(@"color=%d alpha=%d accum=%d depth=%d",color,alpha,accum,depth);*/
  return pixform;
}


Fl_Gl_Choice *Fl_Cocoa_Gl_Window_Driver::find(int m, const int *alistp)
{
  Fl::screen_driver()->open_display(); // useful when called through gl_start()
  Fl_Cocoa_Gl_Choice *g = (Fl_Cocoa_Gl_Choice*)Fl_Gl_Window_Driver::find_begin(m, alistp);
  if (g) return g;
  NSOpenGLPixelFormat* fmt = mode_to_NSOpenGLPixelFormat(m, alistp);
  if (!fmt) return 0;
  g = new Fl_Cocoa_Gl_Choice(m, alistp, first);
  first = g;
  g->pixelformat = fmt;
  return g;
}


#if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_12
#  define NSOpenGLContextParameterSurfaceOpacity NSOpenGLCPSurfaceOpacity
#endif

static void remove_gl_context_opacity(NSOpenGLContext *ctx) {
  GLint gl_opacity;
  [ctx getValues:&gl_opacity forParameter:NSOpenGLContextParameterSurfaceOpacity];
  if (gl_opacity != 0) {
    gl_opacity = 0;
    [ctx setValues:&gl_opacity forParameter:NSOpenGLContextParameterSurfaceOpacity];
  }
}


static NSOpenGLContext *create_GLcontext_for_window(
                         NSOpenGLPixelFormat *pixelformat,
                         NSOpenGLContext *shared_ctx, Fl_Window *window)
{
  NSOpenGLContext *context = [[NSOpenGLContext alloc] initWithFormat:pixelformat shareContext:shared_ctx];
  if (shared_ctx && !context) context = [[NSOpenGLContext alloc] initWithFormat:pixelformat shareContext:nil];
  if (context) {
    NSView *view = [fl_xid(window) contentView];
    if (view && fl_mac_os_version >= 100700) {
      //replaces  [view setWantsBestResolutionOpenGLSurface:YES]  without compiler warning
      typedef void (*bestResolutionIMP)(id, SEL, BOOL);
      static bestResolutionIMP addr = (bestResolutionIMP)[NSView instanceMethodForSelector:@selector(setWantsBestResolutionOpenGLSurface:)];
      addr(view, @selector(setWantsBestResolutionOpenGLSurface:), Fl::use_high_res_GL() != 0);
    }
    [context setView:view];
    if (Fl_Cocoa_Window_Driver::driver(window)->subRect()) {
      remove_gl_context_opacity(context);
    }
  }
  return context;
}

GLContext Fl_Cocoa_Gl_Window_Driver::create_gl_context(Fl_Window* window, const Fl_Gl_Choice* g) {
  GLContext context, shared_ctx = 0;
  if (context_list && nContext) shared_ctx = context_list[0];
  // resets the pile of string textures used to draw strings
  // necessary before the first context is created
  if (!shared_ctx) gl_texture_reset();
  context = create_GLcontext_for_window(((Fl_Cocoa_Gl_Choice*)g)->pixelformat, (NSOpenGLContext*)shared_ctx, window);
  if (!context) return 0;
  add_context(context);
  [(NSOpenGLContext*)context makeCurrentContext];
  glClearColor(0., 0., 0., 1.);
  apply_scissor();
  return (context);
}

void Fl_Cocoa_Gl_Window_Driver::set_gl_context(Fl_Window* w, GLContext context) {
  NSOpenGLContext *current_context = [NSOpenGLContext currentContext];
  if (context != current_context || w != cached_window) {
    cached_window = w;
    [(NSOpenGLContext*)context makeCurrentContext];
  }
}

void Fl_Cocoa_Gl_Window_Driver::delete_gl_context(GLContext context) {
  NSOpenGLContext *current_context = [NSOpenGLContext currentContext];
  if (current_context == context) {
    cached_window = 0;
    [current_context clearDrawable];
  }
  [(NSOpenGLContext*)context release];
  del_context(context);
  if (gl1ctxt) {
    [[gl1ctxt view] release];
    [gl1ctxt release];
    gl1ctxt = 0;
  }
}

void Fl_Cocoa_Gl_Window_Driver::make_overlay_current() {
  // this is not very useful, but unfortunately, Apple decided
  // that front buffer drawing can no longer (OS X 10.4) be supported on their platforms.
  if (pWindow->shown()) pWindow->make_current();
}

void Fl_Cocoa_Gl_Window_Driver::redraw_overlay() {
  pWindow->redraw();
}

void Fl_Cocoa_Gl_Window_Driver::before_show(int& need_after) {
  need_after = 1;
}

void Fl_Cocoa_Gl_Window_Driver::after_show() {
  // Makes sure the GL context is created to avoid drawing twice the window when first shown
  pWindow->make_current();
  if ((mode() & FL_OPENGL3) && !gl1ctxt) {
    // Create transparent GL1 scene above the GL3 scene to hold child widgets and/or text
    NSView *view = [fl_mac_xid(pWindow) contentView];
    NSView *gl1view = [[NSView alloc] initWithFrame:[view frame]];
    [gl1view setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
    static NSOpenGLContext *shared_gl1_ctxt = nil;
    static NSOpenGLPixelFormat *gl1pixelformat = mode_to_NSOpenGLPixelFormat(
                                FL_RGB8 | FL_ALPHA | FL_SINGLE, NULL);
    gl1ctxt = [[NSOpenGLContext alloc] initWithFormat:gl1pixelformat shareContext:shared_gl1_ctxt];
    if (!shared_gl1_ctxt) {
      shared_gl1_ctxt = gl1ctxt;
      [shared_gl1_ctxt retain];
    }
    [view addSubview:gl1view];
  #if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
    if (fl_mac_os_version >= 100700 && Fl::use_high_res_GL()) {
      [gl1view setWantsBestResolutionOpenGLSurface:YES];
    }
  #endif
    [gl1ctxt setView:gl1view];
    remove_gl_context_opacity(gl1ctxt);
  }
}

float Fl_Cocoa_Gl_Window_Driver::pixels_per_unit()
{
  int retina = (fl_mac_os_version >= 100700 && Fl::use_high_res_GL() && Fl_X::flx(pWindow) &&
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
    [(NSOpenGLContext*)pWindow->context() update];
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
  } else {
    [(NSOpenGLContext*)pWindow->context() flushBuffer];
  }
}

char Fl_Cocoa_Gl_Window_Driver::swap_type() {return copy;}

#if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_12
#  define NSOpenGLContextParameterSwapInterval NSOpenGLCPSwapInterval
#endif

void Fl_Cocoa_Gl_Window_Driver::swap_interval(int n) {
  GLint interval = (GLint)n;
  NSOpenGLContext* ctx = (NSOpenGLContext*)pWindow->context();
  if (ctx)
    [ctx setValues:&interval forParameter:NSOpenGLContextParameterSwapInterval];
}

int Fl_Cocoa_Gl_Window_Driver::swap_interval() const {
  GLint interval = (GLint)-1;
  NSOpenGLContext* ctx = (NSOpenGLContext*)pWindow->context();
  if (ctx)
    [ctx getValues:&interval forParameter:NSOpenGLContextParameterSwapInterval];
  return interval;
}

void Fl_Cocoa_Gl_Window_Driver::resize(int is_a_resize, int w, int h) {
  if (pWindow->shown()) apply_scissor();
  [(NSOpenGLContext*)pWindow->context() update];
  if (gl1ctxt) {
    [gl1ctxt update];
  }
}

void Fl_Cocoa_Gl_Window_Driver::apply_scissor() {
  if (glIsEnabled(GL_SCISSOR_TEST)) glDisable(GL_SCISSOR_TEST);
  CGRect *extents = Fl_Cocoa_Window_Driver::driver(pWindow)->subRect();
  if (extents) {
    remove_gl_context_opacity((NSOpenGLContext*)pWindow->context());
    GLdouble vals[4];
    glGetDoublev(GL_COLOR_CLEAR_VALUE, vals);
    glClearColor(0., 0., 0., 0.);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(vals[0], vals[1], vals[2], vals[3]);
    float s = pWindow->pixels_per_unit();
    glScissor(s*extents->origin.x, s*extents->origin.y, s*extents->size.width, s*extents->size.height);
//printf("apply_scissor %dx%d %dx%d\n",extents->x, extents->y, extents->width, extents->height);
    glEnable(GL_SCISSOR_TEST);
  }
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
  q = (char*)CGBitmapContextGetData((CGContextRef)surf->offscreen());
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
  [(NSOpenGLContext*)gl_start_context update];
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
  [(NSOpenGLContext*)glw->context() makeCurrentContext];
// to capture also the overlay and for directGL demo
  [(NSOpenGLContext*)glw->context() flushBuffer];
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
  [(NSOpenGLContext*)glw->context() flushBuffer];
  return img;
}


void* Fl_Cocoa_Gl_Window_Driver::GetProcAddress(const char *procName) {
  return dlsym(RTLD_DEFAULT, procName);
}


FL_EXPORT NSOpenGLContext *fl_mac_glcontext(GLContext rc) {
  return (NSOpenGLContext*)rc;
}


void Fl_Cocoa_Gl_Window_Driver::switch_to_GL1() {
  [gl1ctxt makeCurrentContext];
  glClearColor(0., 0., 0., 0.);
  glClear(GL_COLOR_BUFFER_BIT);
}


void Fl_Cocoa_Gl_Window_Driver::switch_back() {
  glFlush();
  [(NSOpenGLContext*)pWindow->context() makeCurrentContext];
}


class Fl_Gl_Cocoa_Plugin : public Fl_Cocoa_Plugin {
public:
  Fl_Gl_Cocoa_Plugin() : Fl_Cocoa_Plugin(name()) { }
  const char *name() FL_OVERRIDE { return "gl.cocoa.fltk.org"; }
  void resize(Fl_Gl_Window *glw, int x, int y, int w, int h) FL_OVERRIDE {
    glw->Fl_Gl_Window::resize(x, y, w, h);
  }
};

static Fl_Gl_Cocoa_Plugin Gl_Cocoa_Plugin;

#endif // HAVE_GL
