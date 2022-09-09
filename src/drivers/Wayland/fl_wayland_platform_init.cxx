//
// Wayland-specific code to initialize wayland support.
//
// Copyright 2022 by Bill Spitzak and others.
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


#include "Fl_Wayland_Copy_Surface_Driver.H"
#include "Fl_Wayland_Graphics_Driver.H"
#include "Fl_Wayland_Screen_Driver.H"
#include "Fl_Wayland_System_Driver.H"
#include "Fl_Wayland_Window_Driver.H"
#include "Fl_Wayland_Image_Surface_Driver.H"
#if FLTK_USE_X11
#  include "../Xlib/Fl_Xlib_Copy_Surface_Driver.H"
#  include <cairo-xlib.h>
#  include "../Cairo/Fl_Display_Cairo_Graphics_Driver.H"
#  include "../X11/Fl_X11_Screen_Driver.H"
#  include "../X11/Fl_X11_System_Driver.H"
#  include "../X11/Fl_X11_Window_Driver.H"
#  include "../Xlib/Fl_Xlib_Image_Surface_Driver.H"
#endif
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


Fl_System_Driver *Fl_System_Driver::newSystemDriver() {
#if FLTK_USE_X11
  const char *backend = ::getenv("FLTK_BACKEND");
  const char *xdgrt = ::getenv("XDG_RUNTIME_DIR");
  // fprintf(stderr, "FLTK_BACKEND='%s' XDG_RUNTIME_DIR='%s'\n",
  //         backend ? backend : "", xdgrt ? xdgrt : "");

  if (backend && strcmp(backend, "x11") == 0) {
    return new Fl_X11_System_Driver();
  }

  if (backend && strcmp(backend, "wayland") == 0) {
    Fl_Wayland_Screen_Driver::wl_display = wl_display_connect(NULL);
    if (!Fl_Wayland_Screen_Driver::wl_display) {
      fprintf(stderr, "Error: no Wayland connection available, FLTK_BACKEND = '%s'\n", backend);
      exit(1);
    }
    return new Fl_Wayland_System_Driver();
  }

  if (!backend) {
    // env var XDG_RUNTIME_DIR is required for Wayland
    if (xdgrt) {
      // is a Wayland connection available ?
      Fl_Wayland_Screen_Driver::wl_display = wl_display_connect(NULL);
      if (Fl_Wayland_Screen_Driver::wl_display) { // Yes, use Wayland drivers
        // puts("using wayland");
        return new Fl_Wayland_System_Driver();
      }
    }
    // no Wayland connection or environment variable XDG_RUNTIME_DIR not set,
    // falling back to X11
    return new Fl_X11_System_Driver();
  }

  fprintf(stderr, "Error: unexpected value of FLTK_BACKEND: '%s'\n", backend);
  exit(1);
  return NULL;
#else
  return new Fl_Wayland_System_Driver();
#endif
}


static Fl_Fontdesc built_in_table[] = {  // Pango font names
  {"Sans"},
  {"Sans Bold"},
  {"Sans Italic"},
  {"Sans Bold Italic"},
  {"Monospace"},
  {"Monospace Bold"},
  {"Monospace Italic"},
  {"Monospace Bold Italic"},
  {"Serif"},
  {"Serif Bold"},
  {"Serif Italic"},
  {"Serif Bold Italic"},
  {"Standard Symbols PS"}, // FL_SYMBOL
  {"Monospace"},           // FL_SCREEN
  {"Monospace Bold"},      // FL_SCREEN_BOLD
  {"D050000L"},            // FL_ZAPF_DINGBATS
};


FL_EXPORT Fl_Fontdesc *fl_fonts = built_in_table;


Fl_Graphics_Driver *Fl_Graphics_Driver::newMainGraphicsDriver() {
#if FLTK_USE_X11
  Fl_Wayland_Screen_Driver::undo_wayland_backend_if_needed();
  if (Fl_Wayland_Screen_Driver::wl_display) {
    fl_graphics_driver = new Fl_Wayland_Graphics_Driver();
  } else {
    fl_graphics_driver = new Fl_Display_Cairo_Graphics_Driver();
  }
  return fl_graphics_driver;
#else
  return new Fl_Wayland_Graphics_Driver();
#endif
}


Fl_Copy_Surface_Driver *Fl_Copy_Surface_Driver::newCopySurfaceDriver(int w, int h) {
#if FLTK_USE_X11
  if (Fl_Wayland_Screen_Driver::wl_display) return new Fl_Wayland_Copy_Surface_Driver(w, h);
  return new Fl_Xlib_Copy_Surface_Driver(w, h);
#else
  return new Fl_Wayland_Copy_Surface_Driver(w, h);
#endif
}


Fl_Screen_Driver *Fl_Screen_Driver::newScreenDriver() {
#if FLTK_USE_X11
  if (!Fl_Screen_Driver::system_driver) Fl::system_driver();
  Fl_Wayland_Screen_Driver::undo_wayland_backend_if_needed();
  if (Fl_Wayland_Screen_Driver::wl_display) {
    Fl_Wayland_System_Driver::too_late_to_disable = true;
    return new Fl_Wayland_Screen_Driver();
  }

  Fl_X11_Screen_Driver *d = new Fl_X11_Screen_Driver();
  for (int i = 0;  i < MAX_SCREENS; i++) d->screens[i].scale = 1;
  d->current_xft_dpi = 0.; // means the value of the Xft.dpi resource is still unknown
  return d;
#else
  return new Fl_Wayland_Screen_Driver();
#endif
}


Fl_Window_Driver *Fl_Window_Driver::newWindowDriver(Fl_Window *w)
{
#if FLTK_USE_X11
  if (!Fl_Screen_Driver::system_driver) Fl::system_driver();
  static bool been_here = false;
  if (!been_here) {
    been_here = true;
    Fl_Wayland_System_Driver::too_late_to_disable = true;
    Fl_Wayland_Screen_Driver::undo_wayland_backend_if_needed();
  }
  if (Fl_Wayland_Screen_Driver::wl_display) return new Fl_Wayland_Window_Driver(w);
  return new Fl_X11_Window_Driver(w);
#else
  return new Fl_Wayland_Window_Driver(w);
#endif
}


Fl_Image_Surface_Driver *Fl_Image_Surface_Driver::newImageSurfaceDriver(int w, int h, int high_res, Fl_Offscreen off)
{
#if FLTK_USE_X11
  if (Fl_Wayland_Screen_Driver::wl_display) return new Fl_Wayland_Image_Surface_Driver(w, h, high_res, off);
  return new Fl_Xlib_Image_Surface_Driver(w, h, high_res, off);
#else
  return new Fl_Wayland_Image_Surface_Driver(w, h, high_res, off);
#endif
}
