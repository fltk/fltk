//
// Copy-to-clipboard code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2023 by Bill Spitzak and others.
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
#include <FL/Fl_Image_Surface.H>
#include "Fl_Wayland_Graphics_Driver.H"
#include "Fl_Wayland_Screen_Driver.H"
#include "Fl_Wayland_Window_Driver.H"


Fl_Wayland_Copy_Surface_Driver::Fl_Wayland_Copy_Surface_Driver(int w, int h) : Fl_Copy_Surface_Driver(w, h) {
  struct Fl_Wayland_Window_Driver::surface_output *s_output = NULL;
  if (Fl_Wayland_Window_Driver::wld_window &&
      !wl_list_empty(&Fl_Wayland_Window_Driver::wld_window->outputs)) {
    s_output = wl_container_of(Fl_Wayland_Window_Driver::wld_window->outputs.next, s_output, link);
  }
  int os_scale = (s_output ? s_output->output->wld_scale : 1);
  img_surf = new Fl_Image_Surface(w * os_scale, h * os_scale);
  driver(img_surf->driver());
  driver()->scale(os_scale);
}


Fl_Wayland_Copy_Surface_Driver::~Fl_Wayland_Copy_Surface_Driver() {
  Fl_RGB_Image *rgb = img_surf->image();
  Fl_Wayland_Screen_Driver *scr_driver = (Fl_Wayland_Screen_Driver*)Fl::screen_driver();
  scr_driver->copy_image(rgb->array, rgb->data_w(), rgb->data_h());
  delete rgb;
  delete img_surf;
  driver(NULL);
}


void Fl_Wayland_Copy_Surface_Driver::set_current() {
  Fl_Surface_Device::set_current();
  ((Fl_Wayland_Graphics_Driver*)driver())->set_cairo((cairo_t*)img_surf->offscreen());
}


void Fl_Wayland_Copy_Surface_Driver::translate(int x, int y) {
  ((Fl_Wayland_Graphics_Driver*)driver())->ps_translate(x, y);
}


void Fl_Wayland_Copy_Surface_Driver::untranslate() {
  ((Fl_Wayland_Graphics_Driver*)driver())->ps_untranslate();
}
