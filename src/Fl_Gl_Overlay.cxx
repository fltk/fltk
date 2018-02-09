//
// "$Id$"
//
// OpenGL overlay code for the Fast Light Tool Kit (FLTK).
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

#include <FL/Fl.H>
#include <FL/gl.h>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Gl_Window_Driver.H>
#include <stdlib.h>

/**
 Returns true if the hardware overlay is possible.  If this is false,
 FLTK will try to simulate the overlay, with significant loss of update
 speed.  Calling this will cause FLTK to open the display.
 */
int Fl_Gl_Window::can_do_overlay() {
  return pGlWindowDriver->can_do_overlay();
}

void Fl_Gl_Window_Driver::make_overlay(void *&o) {
  o = pWindow;
}

/**
 Causes draw_overlay() to be called at a later time.
 Initially the overlay is clear. If you want the window to display
 something in the overlay when it first appears, you must call this
 immediately after you show() your window.
 */
void Fl_Gl_Window::redraw_overlay() {
  if (!shown()) return;
  pGlWindowDriver->make_overlay(overlay);
  pGlWindowDriver->redraw_overlay();
}

/**
 Selects the OpenGL context for the widget's overlay.
 This method is called automatically prior to the
 draw_overlay() method being called and can also be used to
 implement feedback and/or selection within the handle()
 method.
 */
void Fl_Gl_Window::make_overlay_current() {
  pGlWindowDriver->make_overlay(overlay);
  pGlWindowDriver->make_overlay_current();
}

/** Hides the window if it is not this window, does nothing in Windows. */
void Fl_Gl_Window::hide_overlay() {
  pGlWindowDriver->hide_overlay();
}


// Methods on Fl_Gl_Window that create an overlay window.  Because
// many programs don't need the overlay, this is separated into this
// source file so it is not linked in if not used.

// Under X this is done by creating another window, of class _Fl_Gl_Overlay
// which is a subclass of Fl_Gl_Window except it uses the overlay planes.
// A pointer to this is stored in the "overlay" pointer of the Fl_Gl_Window.

// Under win32 another GLX context is created to draw into the overlay
// and it is stored in the "overlay" pointer.

// In both cases if overlay hardware is unavailable, the overlay is
// "faked" by drawing into the main layers.  This is indicated by
// setting overlay == this.

#ifdef FL_CFG_GFX_QUARTZ

void Fl_Cocoa_Gl_Window_Driver::make_overlay_current() {
  // this is not very useful, but unfortunately, Apple decided
  // that front buffer drawing can no longer (OS X 10.4) be supported on their platforms.
  pWindow->make_current();
}

void Fl_Cocoa_Gl_Window_Driver::redraw_overlay() {
  pWindow->redraw();
}

#endif // FL_CFG_GFX_QUARTZ


#ifdef FL_CFG_GFX_XLIB
#include <FL/platform.H>
////////////////////////////////////////////////////////////////
// X version

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

extern XVisualInfo *fl_find_overlay_visual();
extern XVisualInfo *fl_overlay_visual;
extern Colormap fl_overlay_colormap;
extern unsigned long fl_transparent_pixel;
extern uchar fl_overlay;

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
  fl_overlay = 1;
  w->gl_driver()->draw_overlay();
  fl_overlay = 0;
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

#endif // FL_CFG_GFX_XLIB


#ifdef FL_CFG_GFX_GDI
#include "drivers/WinAPI/Fl_WinAPI_Window_Driver.H"

////////////////////////////////////////////////////////////////
// Windows version:

void Fl_WinAPI_Gl_Window_Driver::hide_overlay(void *& overlay) {
#if HAVE_GL_OVERLAY
  if (overlay && overlay != pWindow) {
    delete_gl_context((GLContext)overlay);
    overlay = 0;
  }
#endif
}

void Fl_WinAPI_Gl_Window_Driver::make_overlay_current() {
#if HAVE_GL_OVERLAY
  if (overlay != this) {
    pGlWindowDriver->set_gl_context(this, (GLContext)overlay);
    //  if (fl_overlay_depth)
    //    wglRealizeLayerPalette(Fl_X::i(this)->private_dc, 1, TRUE);
  } else
#endif
    glDrawBuffer(GL_FRONT);
}

void Fl_WinAPI_Gl_Window_Driver::redraw_overlay() {
  pWindow->damage(FL_DAMAGE_OVERLAY);
}

#if HAVE_GL_OVERLAY

//static COLORREF *palette;
extern int fl_overlay_depth;

void Fl_WinAPI_Gl_Window_Driver::make_overlay(void*&overlay) {
  if (overlay) return;

  GLContext context = create_gl_context(pWindow, g, 1);
  if (!context) {overlay = pWindow; return;} // fake the overlay

  HDC hdc = Fl_WinAPI_Window_Driver::driver(pWindow)->private_dc;
  overlay = context;
  LAYERPLANEDESCRIPTOR pfd;
  wglDescribeLayerPlane(hdc, g->pixelformat, 1, sizeof(pfd), &pfd);
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
  valid(0);
  return;
}

int Fl_WinAPI_Gl_Window_Driver::can_do_overlay() {
  if (!g()) {
    g( find(mode(), alist()) );
    if (!g()) return 0;
  }
  return (g()->pfd.bReserved & 15) != 0;
}
#endif // HAVE_GL_OVERLAY

#endif // FL_CFG_GFX_GDI

#endif // HAVE_GL

//
// End of "$Id$".
//
