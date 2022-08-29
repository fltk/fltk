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

#include "../Xlib/Fl_Xlib_Copy_Surface_Driver.H"
#include <cairo-xlib.h>
#include "../Cairo/Fl_Display_Cairo_Graphics_Driver.H"
#include "../X11/Fl_X11_Screen_Driver.H"
#include "../X11/Fl_X11_System_Driver.H"
#include "../X11/Fl_X11_Window_Driver.H"
#include "../Xlib/Fl_Xlib_Image_Surface_Driver.H"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>


void fl_disable_wayland() {
  if (Fl_Wayland_Screen_Driver::wl_display) {
    wl_display_disconnect(Fl_Wayland_Screen_Driver::wl_display);
    Fl_Wayland_Screen_Driver::wl_display = NULL;
    delete Fl_Screen_Driver::system_driver;
    Fl_Screen_Driver::system_driver = NULL;
  }
  Fl_Wayland_Screen_Driver::wld_disabled = true;
  Fl::system_driver();
}


Fl_System_Driver *Fl_System_Driver::newSystemDriver()
{
  const char *backend = ::getenv("FLTK_BACKEND");
  // fprintf(stderr, "FLTK_BACKEND='%s' XDG_RUNTIME_DIR='%s'\n",backend ? backend : "", xdg ? xdg : "");
  if (backend && strcmp(backend, "wayland") == 0) {
    Fl_Wayland_Screen_Driver::wl_display = wl_display_connect(NULL);
    if (!Fl_Wayland_Screen_Driver::wl_display) {
      fprintf(stderr, "Error: no Wayland connection available, FLTK_BACKEND = '%s'\n", backend);
      exit(1);
    }
    return new Fl_Wayland_System_Driver();
  }
  else if (backend && strcmp(backend, "x11") == 0) {
    return new Fl_X11_System_Driver();
  }
  else if (!backend) {
    if (!Fl_Wayland_Screen_Driver::wld_disabled && ::getenv("XDG_RUNTIME_DIR")) {
      // env var XDG_RUNTIME_DIR is necessary for wayland
      // is a Wayland connection available ?
      Fl_Wayland_Screen_Driver::wl_display = wl_display_connect(NULL);
      if (Fl_Wayland_Screen_Driver::wl_display) { // Yes, use Wayland drivers
        // puts("using wayland");
        return new Fl_Wayland_System_Driver();
      }
    }
    return new Fl_X11_System_Driver();
  }
  fprintf(stderr, "Error: unexpected value of FLTK_BACKEND: '%s'\n", backend);
  exit(1);
  return NULL;
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
  if (Fl_Wayland_Screen_Driver::wl_display) {
    fl_graphics_driver = new Fl_Wayland_Graphics_Driver();
puts("using Fl_Wayland_Graphics_Driver");
  } else {
    fl_graphics_driver = new Fl_Display_Cairo_Graphics_Driver();
puts("using Fl_Display_Cairo_Graphics_Driver");
  }
  return fl_graphics_driver;
}


Fl_Copy_Surface_Driver *Fl_Copy_Surface_Driver::newCopySurfaceDriver(int w, int h) {
  if (Fl_Wayland_Screen_Driver::wl_display) return new Fl_Wayland_Copy_Surface_Driver(w, h);
  return new Fl_Xlib_Copy_Surface_Driver(w, h);
}


Fl_Screen_Driver *Fl_Screen_Driver::newScreenDriver() {
  if (Fl_Wayland_Screen_Driver::wl_display) return new Fl_Wayland_Screen_Driver();

  Fl_X11_Screen_Driver *d = new Fl_X11_Screen_Driver();
  for (int i = 0;  i < MAX_SCREENS; i++) d->screens[i].scale = 1;
  d->current_xft_dpi = 0.; // means the value of the Xft.dpi resource is still unknown
  return d;
}


Fl_Window_Driver *Fl_Window_Driver::newWindowDriver(Fl_Window *w)
{
  if (Fl_Wayland_Screen_Driver::wl_display) return new Fl_Wayland_Window_Driver(w);
  return new Fl_X11_Window_Driver(w);
}


Fl_Image_Surface_Driver *Fl_Image_Surface_Driver::newImageSurfaceDriver(int w, int h, int high_res, Fl_Offscreen off)
{
  if (Fl_Wayland_Screen_Driver::wl_display) return new Fl_Wayland_Image_Surface_Driver(w, h, high_res, off);
  return new Fl_Xlib_Image_Surface_Driver(w, h, high_res, off);
}
