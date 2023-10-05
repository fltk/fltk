//
// Wayland-specific code to initialize wayland support.
//
// Copyright 2022-2023 by Bill Spitzak and others.
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

#include <FL/fl_config.h>
#include "Fl_Wayland_Copy_Surface_Driver.H"
#include "Fl_Wayland_Graphics_Driver.H"
#include "Fl_Wayland_Screen_Driver.H"
#include "../Unix/Fl_Unix_System_Driver.H"
#include "Fl_Wayland_Window_Driver.H"
#include "Fl_Wayland_Image_Surface_Driver.H"
#ifdef FLTK_USE_X11
#  include "../Xlib/Fl_Xlib_Copy_Surface_Driver.H"
#  include "../Cairo/Fl_X11_Cairo_Graphics_Driver.H"
#  include "../X11/Fl_X11_Screen_Driver.H"
#  include "../X11/Fl_X11_Window_Driver.H"
#  include "../Xlib/Fl_Xlib_Image_Surface_Driver.H"
#endif
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


#ifdef FLTK_USE_X11

static bool attempt_wayland() {
  if (Fl_Wayland_Screen_Driver::wl_display) return true;
  static bool first = true;
  static bool disable_wl = false;
  if (first) { // get the value if it exists and cache it
    void *sym = Fl_Posix_System_Driver::dlopen_or_dlsym(NULL, "fl_disable_wayland");
    if (sym) {
      disable_wl = *(bool *)sym;
      // printf("fl_disable_wayland = %s\n", disable_wl ? "true" : "false");
    }
    first = false;
  }
  if (disable_wl)
    return false;
  const char *backend = ::getenv("FLTK_BACKEND");
  // fprintf(stderr, "FLTK_BACKEND='%s'\n", backend ? backend : "");
  if (backend && strcmp(backend, "x11") == 0) {
    return false;
  }

  if (backend && strcmp(backend, "wayland") == 0) {
    Fl_Wayland_Screen_Driver::wl_display = wl_display_connect(NULL);
    if (!Fl_Wayland_Screen_Driver::wl_display) {
      fprintf(stderr, "Error: no Wayland connection available, FLTK_BACKEND='wayland'\n");
      exit(1);
    }
    return true;
  }

  if (!backend) {
    // env var XDG_RUNTIME_DIR is required for Wayland
    const char *xdgrt = ::getenv("XDG_RUNTIME_DIR");
    if (xdgrt) {
      // is a Wayland connection available ?
      Fl_Wayland_Screen_Driver::wl_display = wl_display_connect(NULL);
      if (Fl_Wayland_Screen_Driver::wl_display) { // Yes, use Wayland drivers
        // puts("using wayland");
        return true;
      }
    }
    // no Wayland connection or environment variable XDG_RUNTIME_DIR not set,
    // falling back to X11
    return false;
  }

  fprintf(stderr, "Error: unexpected value of FLTK_BACKEND: '%s'\n", backend);
  exit(1);
  return false;
}

#endif // FLTK_USE_X11


Fl_System_Driver *Fl_System_Driver::newSystemDriver() {
  return new Fl_Unix_System_Driver();
}


Fl_Graphics_Driver *Fl_Graphics_Driver::newMainGraphicsDriver() {
#ifdef FLTK_USE_X11
  if (!attempt_wayland()) return new Fl_X11_Cairo_Graphics_Driver();
#endif
  return new Fl_Wayland_Graphics_Driver();
}


Fl_Copy_Surface_Driver *Fl_Copy_Surface_Driver::newCopySurfaceDriver(int w, int h) {
#ifdef FLTK_USE_X11
  if (!Fl_Wayland_Screen_Driver::wl_display) return new Fl_Xlib_Copy_Surface_Driver(w, h);
#endif
  return new Fl_Wayland_Copy_Surface_Driver(w, h);
}


Fl_Screen_Driver *Fl_Screen_Driver::newScreenDriver() {
  if (!Fl_Screen_Driver::system_driver) Fl::system_driver();
#ifdef FLTK_USE_X11
  if (attempt_wayland()) {
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
#ifdef FLTK_USE_X11
  if (!attempt_wayland()) return new Fl_X11_Window_Driver(w);
#endif
  return new Fl_Wayland_Window_Driver(w);
}


Fl_Image_Surface_Driver *Fl_Image_Surface_Driver::newImageSurfaceDriver(int w, int h, int high_res, Fl_Offscreen off)
{
#ifdef FLTK_USE_X11
  if (!attempt_wayland())
    return new Fl_Xlib_Image_Surface_Driver(w, h, high_res, off);
#endif
  return new Fl_Wayland_Image_Surface_Driver(w, h, high_res, off);
}
