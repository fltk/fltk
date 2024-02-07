//
// Special Cairo support for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2024 by Bill Spitzak and others.
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

// This file implements the FLTK Cairo Window support (since 1.3.x):
//
// - ./configure --enable-cairo  and/or  --enable-cairoext
// - cmake       -D FLTK_OPTION_CAIRO_WINDOW  and/or  -D FLTK_OPTION_CAIRO_EXT
//
// Preprocessor macro FLTK_HAVE_CAIRO is defined for both options.
// Preprocessor macro FLTK_HAVE_CAIRO_EXT is defined only for "cairoext".
// Both macros are defined in 'FL/fl_config.h'.

#include <FL/Fl.H> // includes <FL/fl_config.h>

#ifdef FLTK_HAVE_CAIRO

// Define USE_MAC_OS for convenience (below). We use macOS specific features
// if USE_MAC_OS is defined, otherwise we're using X11 (XQuartz) on macOS

#if defined __APPLE__ && !defined(FLTK_USE_X11)
#define USE_MAC_OS
#include <ApplicationServices/ApplicationServices.h>
#endif

#include <FL/platform.H>
#include <FL/Fl_Window.H>

// Cairo is currently supported for the following platforms:
// Windows, macOS (Apple Quartz), X11, Wayland

#if defined(_WIN32)               // Windows
#  include <cairo-win32.h>
#elif defined(FLTK_USE_WAYLAND)   // Wayland or hybrid
#  include "../src/drivers/Wayland/Fl_Wayland_Graphics_Driver.H"
#  include "../src/drivers/Wayland/Fl_Wayland_Window_Driver.H"
#  if defined(FLTK_USE_X11)
#    include <cairo-xlib.h>
#  else
     static void *fl_gc = NULL;
#  endif
#elif defined(FLTK_USE_X11)       // X11
#  include <cairo-xlib.h>
#elif defined(__APPLE__)          // macOS
#  include <cairo-quartz.h>
#else
#  error Cairo is not supported on this platform.
#endif

// static initialization

Fl_Cairo_State Fl::cairo_state_; ///< current Cairo context information

// Fl_Cairo_State

void Fl_Cairo_State::autolink(bool b) {
#ifdef FLTK_HAVE_CAIROEXT
  autolink_ = b;
#else
  Fl::fatal("In Fl::autolink(bool): Cairo autolink() feature is only "
            "available with CMake FLTK_OPTION_CAIRO_EXT "
            "or the enable-cairoext configure option.\n"
            "Quitting now.");
#endif
}

/**
  Provides a Cairo context for window \a wi.

  This is needed in a draw() override if Fl::cairo_autolink_context()
  returns false, which is the default.
  The cairo_context() does not need to be freed as it is freed every time
  a new Cairo context is created. When the program terminates,
  a call to Fl::cairo_make_current(0) will destroy any residual context.

  \note A new Cairo context is not always re-created when this method
    is used. In particular, if the current graphical context and the current
    window didn't change between two calls, the previous gc is internally kept,
    thus optimizing the drawing performances.
    Also, after this call, Fl::cairo_cc() is adequately updated with this
    Cairo context.

  \note Only available when configure has the --enable-cairo option

  \return The valid cairo_t *cairo context associated to this window.
  \retval NULL if \a wi is NULL or maybe with GL windows under Wayland
*/
cairo_t *Fl::cairo_make_current(Fl_Window *wi) {
  if (!wi)
    return NULL;
  cairo_t *cairo_ctxt;

#if defined(FLTK_USE_WAYLAND)
  if (fl_wl_display()) { // true means using wayland backend
    struct wld_window *xid = fl_wl_xid(wi);
    if (!xid->buffer)
      return NULL; // this may happen with GL windows
    cairo_ctxt = xid->buffer->draw_buffer.cairo_;
    cairo_state_.cc(cairo_ctxt, false);
    return cairo_ctxt;
  }
#endif

  if (fl_gc == 0) {  // means remove current cc
    Fl::cairo_cc(0); // destroy any previous cc
    cairo_state_.window(0);
    return 0;
  }

  // don't re-create a context if it's the same gc/window combination
  if (fl_gc == Fl::cairo_state_.gc() && fl_xid(wi) == (Window)Fl::cairo_state_.window())
    return Fl::cairo_cc();

  cairo_state_.window((void *)fl_xid(wi));

  // Scale the Cairo context appropriately. This is platform dependent

#if !defined(USE_MAC_OS)
  float scale = Fl::screen_scale(wi->screen_num()); // get the screen scaling factor
#endif

#if defined(FLTK_USE_X11)
  cairo_ctxt = Fl::cairo_make_current(0, wi->w() * scale, wi->h() * scale);
#else
  // on macOS, scaling is done before by Fl_Window::make_current(), on Windows, the size is not used
  cairo_ctxt = Fl::cairo_make_current(fl_gc, wi->w(), wi->h());
#endif

#if !defined(USE_MAC_OS)
  cairo_scale(cairo_ctxt, scale, scale);
#endif
  return cairo_ctxt;
}

/*
  Creates transparently a cairo_surface_t object.
  gc is an HDC context in Windows, a CGContext* in Quartz, and
  a display on X11 (not used on this platform)
*/

static cairo_surface_t *cairo_create_surface(void *gc, int W, int H) {
#if defined(FLTK_USE_X11)
  return cairo_xlib_surface_create(fl_display, fl_window, fl_visual->visual, W, H);
#elif defined(FLTK_USE_WAYLAND)
  return NULL;
#elif defined(_WIN32)
  return cairo_win32_surface_create((HDC)gc);
#elif defined(__APPLE__)
  return cairo_quartz_surface_create_for_cg_context((CGContextRef)gc, W, H);
#else
#error Cairo is not supported on this platform.
#endif
}

/**
  Creates a Cairo context from a \a gc only, gets its window size or
  offscreen size if fl_window is null.

  \note Only available if CMake FLTK_OPTION_CAIRO_WINDOW is enabled
        or configure has the --enable-cairo option.
*/
cairo_t *Fl::cairo_make_current(void *gc) {
  int W = 0, H = 0;
#if defined(FLTK_USE_X11) || defined(FLTK_USE_WAYLAND)
  // FIXME X11 get W,H
  // gc will be the window handle here
  // # warning FIXME get W,H for cairo_make_current(void*)
#elif defined(__APPLE__)
  if (fl_window) {
    W = Fl_Window::current()->w();
    H = Fl_Window::current()->h();
  } else {
    W = CGBitmapContextGetWidth(fl_gc);
    H = CGBitmapContextGetHeight(fl_gc);
  }
#elif defined(_WIN32)
  // we don't need any W,H for Windows
#else
#error Cairo is not supported on this platform.
#endif

  if (!gc) {
    Fl::cairo_cc(0);
    cairo_state_.gc(0); // keep track for next time
    return 0;
  }
  if (gc == Fl::cairo_state_.gc() &&
      fl_window == (Window)Fl::cairo_state_.window() &&
      cairo_state_.cc() != 0)
    return Fl::cairo_cc();
  cairo_state_.gc(fl_gc); // keep track for next time
  cairo_surface_t *s = cairo_create_surface(gc, W, H);
  cairo_t *c = cairo_create(s);
  cairo_surface_destroy(s);
  cairo_state_.cc(c);
  return c;
}

/**
  Creates a Cairo context from a \p gc and the given size.

  \note Only available if CMake FLTK_OPTION_CAIRO_WINDOW is enabled
        or configure has the --enable-cairo option.
*/
cairo_t *Fl::cairo_make_current(void *gc, int W, int H) {
  if (gc == Fl::cairo_state_.gc() &&
      fl_window == (Window)Fl::cairo_state_.window() &&
      cairo_state_.cc() != 0) // no need to create a cc, just return that one
    return cairo_state_.cc();

  // we need to (re-)create a fresh cc ...
  cairo_state_.gc(gc); // keep track for next time
  cairo_surface_t *s = cairo_create_surface(gc, W, H);

#if defined(USE_MAC_OS) && defined(FLTK_HAVE_CAIROEXT)
  CGAffineTransform at = CGContextGetCTM((CGContextRef)gc);
  CGContextSaveGState((CGContextRef)gc);
  CGContextConcatCTM((CGContextRef)gc, CGAffineTransformInvert(at));
#endif

  cairo_t *c = cairo_create(s);

#if defined(USE_MAC_OS) && defined(FLTK_HAVE_CAIROEXT)
  CGContextRestoreGState((CGContextRef)gc);
#endif

  cairo_state_.cc(c); //  and purge any previously owned context
  cairo_surface_destroy(s);
  return c;
}

// Silence compiler warning if none of the Cairo options has been configured

#else
FL_EXPORT int fltk_cairo_dummy() {
  return 1;
}

#endif // FLTK_HAVE_CAIRO
