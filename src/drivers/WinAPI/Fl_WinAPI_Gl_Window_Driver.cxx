//
// Class Fl_WinAPI_Gl_Window_Driver for the Fast Light Tool Kit (FLTK).
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
#include "../../Fl_Screen_Driver.H"
#include <FL/gl.h>
#include "Fl_WinAPI_Gl_Window_Driver.H"
#include "../../Fl_Gl_Choice.H"
#include "Fl_WinAPI_Window_Driver.H"
#include "../GDI/Fl_Font.H"
extern void fl_save_dc(HWND, HDC);

#ifndef GL_CURRENT_PROGRAM
#  define GL_CURRENT_PROGRAM 0x8B8D // from glew.h
#endif

// STR #3119: select pixel format with composition support
// ... and no more than 32 color bits (8 bits/color)
// Ref: PixelFormatDescriptor Object
// https://msdn.microsoft.com/en-us/library/cc231189.aspx
#if !defined(PFD_SUPPORT_COMPOSITION)
# define PFD_SUPPORT_COMPOSITION (0x8000)
#endif

#define DEBUG_PFD (0) // 1 = PFD selection debug output, 0 = no debug output


// Describes crap needed to create a GLContext.
class Fl_WinAPI_Gl_Choice : public Fl_Gl_Choice {
  friend class Fl_WinAPI_Gl_Window_Driver;
private:
  int pixelformat;           // the visual to use
  PIXELFORMATDESCRIPTOR pfd; // some wgl calls need this thing
public:
  Fl_WinAPI_Gl_Choice(int m, const int *alistp, Fl_Gl_Choice *n) : Fl_Gl_Choice(m, alistp, n) {
    pixelformat = 0;
  }
};


Fl_Gl_Window_Driver *Fl_Gl_Window_Driver::newGlWindowDriver(Fl_Gl_Window *w)
{
  return new Fl_WinAPI_Gl_Window_Driver(w);
}


Fl_Gl_Choice *Fl_WinAPI_Gl_Window_Driver::find(int m, const int *alistp)
{
  Fl_WinAPI_Gl_Choice *g = (Fl_WinAPI_Gl_Choice*)Fl_Gl_Window_Driver::find_begin(m, alistp);
  if (g) return g;

  // Replacement for ChoosePixelFormat() that finds one with an overlay if possible:
  HDC gc = (HDC)(fl_graphics_driver ? fl_graphics_driver->gc() : 0);
  if (!gc) gc = fl_GetDC(0);
  int pixelformat = 0;
  PIXELFORMATDESCRIPTOR chosen_pfd;
  for (int i = 1; ; i++) {
    PIXELFORMATDESCRIPTOR pfd;
    if (!DescribePixelFormat(gc, i, sizeof(pfd), &pfd)) break;
    // continue if it does not satisfy our requirements:
    if (~pfd.dwFlags & (PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL)) continue;
    if (pfd.iPixelType != ((m&FL_INDEX)?PFD_TYPE_COLORINDEX:PFD_TYPE_RGBA)) continue;
    if ((m & FL_ALPHA) && !pfd.cAlphaBits) continue;
    if ((m & FL_ACCUM) && !pfd.cAccumBits) continue;
    if ((!(m & FL_DOUBLE)) != (!(pfd.dwFlags & PFD_DOUBLEBUFFER))) continue;
    if ((!(m & FL_STEREO)) != (!(pfd.dwFlags & PFD_STEREO))) continue;
    if ((m & FL_DEPTH) && !pfd.cDepthBits) continue;
    if ((m & FL_STENCIL) && !pfd.cStencilBits) continue;

#if DEBUG_PFD
    printf("pfd #%d supports composition: %s\n", i, (pfd.dwFlags & PFD_SUPPORT_COMPOSITION) ? "yes" : "no");
    printf("    ... & PFD_GENERIC_FORMAT: %s\n", (pfd.dwFlags & PFD_GENERIC_FORMAT) ? "generic" : "accelerated");
    printf("    ... Overlay Planes      : %d\n", pfd.bReserved & 15);
    printf("    ... Color & Depth       : %d, %d\n", pfd.cColorBits, pfd.cDepthBits);
    if (pixelformat)
      printf("        current pixelformat : %d\n", pixelformat);
    fflush(stdout);
#endif // DEBUG_PFD

    // see if better than the one we have already:
    if (pixelformat) {
      // offering non-generic rendering is better (read: hardware acceleration)
      if (!(chosen_pfd.dwFlags & PFD_GENERIC_FORMAT) &&
          (pfd.dwFlags & PFD_GENERIC_FORMAT)) continue;
      // offering overlay is better:
      else if (!(chosen_pfd.bReserved & 15) && (pfd.bReserved & 15)) {}
      // otherwise prefer a format that supports composition (STR #3119)
      else if ((chosen_pfd.dwFlags & PFD_SUPPORT_COMPOSITION) &&
               !(pfd.dwFlags & PFD_SUPPORT_COMPOSITION)) continue;
      // otherwise more bit planes is better, but no more than 32 (8 bits per channel):
      else if (pfd.cColorBits > 32 || chosen_pfd.cColorBits > pfd.cColorBits) continue;
      else if (chosen_pfd.cDepthBits > pfd.cDepthBits) continue;
    }
    pixelformat = i;
    chosen_pfd = pfd;
  }

#if DEBUG_PFD
  static int bb = 0;
  if (!bb) {
    bb = 1;
    printf("PFD_SUPPORT_COMPOSITION = 0x%x\n", PFD_SUPPORT_COMPOSITION);
  }
  printf("Chosen pixel format is %d\n", pixelformat);
  printf("Color bits = %d, Depth bits = %d\n", chosen_pfd.cColorBits, chosen_pfd.cDepthBits);
  printf("Pixel format supports composition: %s\n", (chosen_pfd.dwFlags & PFD_SUPPORT_COMPOSITION) ? "yes" : "no");
  fflush(stdout);
#endif // DEBUG_PFD

  if (!pixelformat) return 0;

  g = new Fl_WinAPI_Gl_Choice(m, alistp, first);
  first = g;

  g->pixelformat = pixelformat;
  g->pfd = chosen_pfd;

  return g;
}


GLContext Fl_WinAPI_Gl_Window_Driver::do_create_gl_context(Fl_Window* window,
                      const Fl_Gl_Choice* g, int layer)
{
  Fl_X* i = Fl_X::flx(window);
  HDC hdc = Fl_WinAPI_Window_Driver::driver(window)->private_dc;
  if (!hdc) {
    hdc = Fl_WinAPI_Window_Driver::driver(window)->private_dc = GetDCEx((HWND)i->xid, 0, DCX_CACHE);
    fl_save_dc((HWND)i->xid, hdc);
    SetPixelFormat(hdc, ((Fl_WinAPI_Gl_Choice*)g)->pixelformat, (PIXELFORMATDESCRIPTOR*)(&((Fl_WinAPI_Gl_Choice*)g)->pfd));
#    if USE_COLORMAP
    if (fl_palette) SelectPalette(hdc, fl_palette, FALSE);
#    endif
  }
  GLContext context = layer ? wglCreateLayerContext(hdc, layer) : wglCreateContext(hdc);
  if (context) {
    if (context_list && nContext)
      wglShareLists((HGLRC)context_list[0], (HGLRC)context);
    add_context(context);
  }
  return context;
}


GLContext Fl_WinAPI_Gl_Window_Driver::create_gl_context(Fl_Window* window, const Fl_Gl_Choice* g)
{
  return do_create_gl_context(window, g, 0);
}

void Fl_WinAPI_Gl_Window_Driver::set_gl_context(Fl_Window* w, GLContext context) {
  GLContext current_context = wglGetCurrentContext();
  if (context != current_context || w != cached_window) {
    cached_window = w;
    wglMakeCurrent(Fl_WinAPI_Window_Driver::driver(w)->private_dc, (HGLRC)context);
  }
}

void Fl_WinAPI_Gl_Window_Driver::delete_gl_context(GLContext context) {
  GLContext current_context = wglGetCurrentContext();
  if (current_context == context) {
    cached_window = 0;
    wglMakeCurrent(0, 0);
  }
  wglDeleteContext((HGLRC)context);
  del_context(context);
}


void Fl_WinAPI_Gl_Window_Driver::make_overlay_current() {
#if HAVE_GL_OVERLAY
  if (overlay() != this) {
    set_gl_context(pWindow, (GLContext)overlay());
    //  if (fl_overlay_depth)
    //    wglRealizeLayerPalette(Fl_X::flx(this)->private_dc, 1, TRUE);
  } else
#endif
    glDrawBuffer(GL_FRONT);
}

void Fl_WinAPI_Gl_Window_Driver::redraw_overlay() {
  pWindow->damage(FL_DAMAGE_OVERLAY);
}

#if HAVE_GL_OVERLAY

// Methods on Fl_Gl_Window_driver that create an overlay window.

// Under win32 another GLX context is created to draw into the overlay
// and it is stored in the "overlay" pointer.

// If overlay hardware is unavailable, the overlay is
// "faked" by drawing into the main layers.  This is indicated by
// setting overlay == this.

//static COLORREF *palette;
static int fl_overlay_depth = 0;

void Fl_WinAPI_Gl_Window_Driver::gl_hide_before(void *& overlay) {
  if (overlay && overlay != pWindow) {
    delete_gl_context((GLContext)overlay);
    overlay = 0;
  }
}

void Fl_WinAPI_Gl_Window_Driver::make_overlay(void*&overlay) {
  if (overlay) return;

  GLContext context = do_create_gl_context(pWindow, g(), 1);
  if (!context) {overlay = pWindow; return;} // fake the overlay

  HDC hdc = Fl_WinAPI_Window_Driver::driver(pWindow)->private_dc;
  overlay = context;
  LAYERPLANEDESCRIPTOR pfd;
  wglDescribeLayerPlane(hdc, g()->pixelformat, 1, sizeof(pfd), &pfd);
  if (!pfd.iPixelType) {
    ; // full-color overlay
  } else {
    fl_overlay_depth = pfd.cColorBits; // used by gl_color()
    if (fl_overlay_depth > 8) fl_overlay_depth = 8;
    COLORREF palette[256];
    int n = (1<<fl_overlay_depth)-1;
    // copy all colors except #0 into the overlay palette:
    for (int i = 0; i <= n; i++) {
      uchar r,g,b; Fl::get_color((Fl_Color)i,r,g,b);
      palette[i] = RGB(r,g,b);
    }
    // always provide black & white in the last 2 pixels:
    if (fl_overlay_depth < 8) {
      palette[n-1] = RGB(0,0,0);
      palette[n] = RGB(255,255,255);
    }
    // and use it:
    wglSetLayerPaletteEntries(hdc, 1, 1, n, palette+1);
    wglRealizeLayerPalette(hdc, 1, TRUE);
  }
  pWindow->valid(0);
  return;
}

int Fl_WinAPI_Gl_Window_Driver::can_do_overlay() {
  if (!g()) {
    g( find(mode(), alist()) );
    if (!g()) return 0;
  }
  return (g()->pfd.bReserved & 15) != 0;
}

int Fl_WinAPI_Gl_Window_Driver::overlay_color(Fl_Color i) {
  if (Fl_Xlib_Graphics_Driver::fl_overlay && fl_overlay_depth) {
    if (fl_overlay_depth < 8) {
      // only black & white produce the expected colors.  This could
      // be improved by fixing the colormap set in Fl_Gl_Overlay.cxx
      int size = 1<<fl_overlay_depth;
      if (!i) glIndexi(size-2);
      else if (i >= size-2) glIndexi(size-1);
      else glIndexi(i);
    } else {
      glIndexi(i ? i : FL_GRAY_RAMP);
    }
    return 1;
  }
  return 0;
}

#endif // HAVE_GL_OVERLAY


float Fl_WinAPI_Gl_Window_Driver::pixels_per_unit()
{
  int ns = Fl_Window_Driver::driver(pWindow)->screen_num();
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


// Start of swap_interval implementation in the three possibel ways for X11

// -1 = not yet initialized, 0 = none found, 1 = GLX, 2 = MESA, 3 = SGI
static signed char swap_interval_type = -1;

typedef const char *(WINAPI *WGL_Get_Extension_String_Proc)();
typedef BOOL (WINAPI *WGL_Swap_Iterval_Proc)(int interval);
typedef int (WINAPI *WGL_Get_Swap_Iterval_Proc)();

static WGL_Swap_Iterval_Proc wglSwapIntervalEXT = NULL;
static WGL_Get_Swap_Iterval_Proc wglGetSwapIntervalEXT = NULL;

static void init_swap_interval() {
  if (swap_interval_type != -1)
    return;
  swap_interval_type = 0;
  WGL_Get_Extension_String_Proc wglGetExtensionsStringEXT = NULL;
  wglGetExtensionsStringEXT = (WGL_Get_Extension_String_Proc)wglGetProcAddress("wglGetExtensionsStringEXT");
  if (!wglGetExtensionsStringEXT)
    return;
  const char *extensions = wglGetExtensionsStringEXT();
  if (extensions && strstr(extensions, "WGL_EXT_swap_control")) {
    wglSwapIntervalEXT = (WGL_Swap_Iterval_Proc)wglGetProcAddress("wglSwapIntervalEXT");
    wglGetSwapIntervalEXT = (WGL_Get_Swap_Iterval_Proc)wglGetProcAddress("wglGetSwapIntervalEXT");
    swap_interval_type = 1;
  }
}

void Fl_WinAPI_Gl_Window_Driver::swap_interval(int interval) {
  if (swap_interval_type == -1)
    init_swap_interval();
  if (swap_interval_type == 1) {
    if (wglSwapIntervalEXT)
      wglSwapIntervalEXT(interval);
  }
}

int Fl_WinAPI_Gl_Window_Driver::swap_interval() const {
  if (swap_interval_type == -1)
    init_swap_interval();
  int interval = -1;
  if (swap_interval_type == 1) {
    if (wglGetSwapIntervalEXT)
      interval = wglGetSwapIntervalEXT();
  }
  return interval;
}

// end of swap_interval implementation

#if HAVE_GL_OVERLAY
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
    Fl_Xlib_Graphics_Driver::fl_overlay = 1;
    draw_overlay();
    Fl_Xlib_Graphics_Driver::fl_overlay = 0;
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


void Fl_WinAPI_Gl_Window_Driver::draw_string_legacy(const char* str, int n) {
  draw_string_legacy_get_list(str, n);
}

int Fl_WinAPI_Gl_Window_Driver::genlistsize() {
  return 0x10000;
}

void Fl_WinAPI_Gl_Window_Driver::gl_bitmap_font(Fl_Font_Descriptor *fl_fontsize) {
  if (!fl_fontsize->listbase) {
    fl_fontsize->listbase = glGenLists(genlistsize());
  }
  glListBase(fl_fontsize->listbase);
}

void Fl_WinAPI_Gl_Window_Driver::get_list(Fl_Font_Descriptor *fd, int r) {
  Fl_GDI_Font_Descriptor* gl_fd = (Fl_GDI_Font_Descriptor*)fd;
  if (gl_fd->glok[r]) return;
  gl_fd->glok[r] = 1;
  unsigned int ii = r * 0x400;
  HFONT oldFid = (HFONT)SelectObject((HDC)fl_graphics_driver->gc(), gl_fd->fid);
  wglUseFontBitmapsW((HDC)fl_graphics_driver->gc(), ii, 0x400, gl_fd->listbase+ii);
  SelectObject((HDC)fl_graphics_driver->gc(), oldFid);
}


typedef void (WINAPI *glUseProgram_type)(GLint);
static glUseProgram_type glUseProgram_f = NULL;

void Fl_WinAPI_Gl_Window_Driver::switch_to_GL1() {
  if (!glUseProgram_f) {
    glUseProgram_f = (glUseProgram_type)GetProcAddress("glUseProgram");
  }
  glGetIntegerv(GL_CURRENT_PROGRAM, &current_prog);
  if (current_prog) glUseProgram_f(0);
}

void Fl_WinAPI_Gl_Window_Driver::switch_back() {
  if (current_prog) glUseProgram_f((GLuint)current_prog);
}


FL_EXPORT HGLRC fl_win32_glcontext(GLContext rc) { return (HGLRC)rc; }

#endif // HAVE_GL
