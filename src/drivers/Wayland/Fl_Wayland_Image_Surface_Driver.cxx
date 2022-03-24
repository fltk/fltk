//
// Draw-to-image code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2021 by Bill Spitzak and others.
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
#include <FL/platform.H>
#include "Fl_Wayland_Graphics_Driver.H"
#include "Fl_Wayland_Window_Driver.H"
#include "Fl_Wayland_Image_Surface_Driver.H"


Fl_Wayland_Image_Surface_Driver::Fl_Wayland_Image_Surface_Driver(int w, int h, int high_res, Fl_Offscreen off) : Fl_Image_Surface_Driver(w, h, high_res, off) {
  float d = 1;
  if (!off) {
    fl_open_display();
    if (Fl_Wayland_Window_Driver::wld_window) {
      d = Fl_Wayland_Window_Driver::wld_window->scale;
    }
    d *= fl_graphics_driver->scale();
    if (d != 1 && high_res) {
      w = int(w*d);
      h = int(h*d);
    }
    offscreen = (struct fl_wld_buffer*)calloc(1, sizeof(struct fl_wld_buffer));
    offscreen->stride = cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, w);
    offscreen->data_size = offscreen->stride * h;
    offscreen->draw_buffer = (uchar*)malloc(offscreen->data_size);
    offscreen->width = w;
    Fl_Wayland_Graphics_Driver::cairo_init(offscreen, w, h, offscreen->stride, CAIRO_FORMAT_RGB24);
  }
  driver(new Fl_Wayland_Graphics_Driver());
  if (d != 1 && high_res) driver()->scale(d);
}


Fl_Wayland_Image_Surface_Driver::~Fl_Wayland_Image_Surface_Driver() {
  if (offscreen && !external_offscreen) {
    free(offscreen->draw_buffer);
    free(offscreen);
  }
  delete driver();
}

void Fl_Wayland_Image_Surface_Driver::set_current() {
  Fl_Surface_Device::set_current();
  ((Fl_Wayland_Graphics_Driver*)fl_graphics_driver)->activate(offscreen, driver()->scale());
  pre_window = Fl_Wayland_Window_Driver::wld_window;
  fl_window = Fl_Wayland_Window_Driver::wld_window = NULL;
}

void Fl_Wayland_Image_Surface_Driver::end_current() {
  cairo_surface_t *surf = cairo_get_target(offscreen->cairo_);
  cairo_surface_flush(surf);
  fl_window = Fl_Wayland_Window_Driver::wld_window = pre_window;
}

void Fl_Wayland_Image_Surface_Driver::translate(int x, int y) {
  ((Fl_Wayland_Graphics_Driver*)driver())->ps_translate(x, y);
}

void Fl_Wayland_Image_Surface_Driver::untranslate() {
  ((Fl_Wayland_Graphics_Driver*)driver())->ps_untranslate();
}

Fl_RGB_Image* Fl_Wayland_Image_Surface_Driver::image() {
  // Convert depth-4 image in draw_buffer to a depth-3 image while exchanging R and B colors
  int height = offscreen->data_size / offscreen->stride;
  uchar *rgb = new uchar[offscreen->width * height * 3];
  uchar *p = rgb;
  uchar *q;
  for (int j = 0; j < height; j++) {
    q = offscreen->draw_buffer + j*offscreen->stride;
    for (int i = 0; i < offscreen->width; i++) { // exchange R and B colors, transmit G
      *p = *(q+2);
      *(p+1) = *(q+1);
      *(p+2) = *q;
      p += 3; q += 4;
    }
  }
  Fl_RGB_Image *image = new Fl_RGB_Image(rgb, offscreen->width, height, 3);
  image->alloc_array = 1;
  return image;
}
