//
// Draw-to-image code for the Fast Light Tool Kit (FLTK).
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

#include <FL/platform.H>
#include "Fl_Wayland_Graphics_Driver.H"
#include "Fl_Wayland_Window_Driver.H"
#include "Fl_Wayland_Image_Surface_Driver.H"


Fl_Wayland_Image_Surface_Driver::Fl_Wayland_Image_Surface_Driver(int w, int h,
      int high_res, Fl_Offscreen off) : Fl_Image_Surface_Driver(w, h, high_res, off) {
  float s = 1;
  int d = 1;
  if (!off) {
    fl_open_display();
    if (Fl_Wayland_Window_Driver::wld_window) {
      d = Fl_Wayland_Window_Driver::driver(
            Fl_Wayland_Window_Driver::wld_window->fl_win
                                           )->wld_scale();
    }
    s = fl_graphics_driver->scale();
    if (d*s != 1 && high_res) {
      w = int(w * s) * d;
      h = int(h * s) * d;
    }
    struct Fl_Wayland_Graphics_Driver::draw_buffer *off_ =
      (struct Fl_Wayland_Graphics_Driver::draw_buffer*)calloc(1,
                      sizeof(struct Fl_Wayland_Graphics_Driver::draw_buffer));
    Fl_Wayland_Graphics_Driver::cairo_init(off_, w, h,
              cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, w), CAIRO_FORMAT_RGB24);
    offscreen = (Fl_Offscreen)off_->cairo_;
    cairo_set_user_data(off_->cairo_, &Fl_Wayland_Graphics_Driver::key, off_, NULL);
    if (d*s != 1 && high_res) cairo_scale((cairo_t*)offscreen, d*s, d*s);
  }
  driver(new Fl_Wayland_Graphics_Driver());
  if (d*s != 1 && high_res) driver()->scale(d*s);
}


Fl_Wayland_Image_Surface_Driver::~Fl_Wayland_Image_Surface_Driver() {
  if (offscreen && !external_offscreen) {
    struct Fl_Wayland_Graphics_Driver::draw_buffer *buffer =
      Fl_Wayland_Graphics_Driver::offscreen_buffer(offscreen);
    cairo_destroy((cairo_t *)offscreen);
    delete[] buffer->buffer;
    free(buffer);
  }
  delete driver();
}


void Fl_Wayland_Image_Surface_Driver::set_current() {
  Fl_Surface_Device::set_current();
  ((Fl_Wayland_Graphics_Driver*)fl_graphics_driver)->set_cairo((cairo_t*)offscreen);
  pre_window = Fl_Wayland_Window_Driver::wld_window;
  Fl_Wayland_Window_Driver::wld_window = NULL;
  fl_window = 0;
}


void Fl_Wayland_Image_Surface_Driver::end_current() {
  cairo_surface_t *surf = cairo_get_target((cairo_t*)offscreen);
  cairo_surface_flush(surf);
  Fl_Wayland_Window_Driver::wld_window = pre_window;
  fl_window = (Window)pre_window;
  Fl_Surface_Device::end_current();
}


void Fl_Wayland_Image_Surface_Driver::translate(int x, int y) {
  ((Fl_Wayland_Graphics_Driver*)driver())->ps_translate(x, y);
}


void Fl_Wayland_Image_Surface_Driver::untranslate() {
  ((Fl_Wayland_Graphics_Driver*)driver())->ps_untranslate();
}


Fl_RGB_Image* Fl_Wayland_Image_Surface_Driver::image() {
  // Convert depth-4 image in draw_buffer to a depth-3 image while exchanging R and B colors
  struct Fl_Wayland_Graphics_Driver::draw_buffer *off_buf =
    Fl_Wayland_Graphics_Driver::offscreen_buffer(offscreen);
  int height = off_buf->data_size / off_buf->stride;
  uchar *rgb = new uchar[off_buf->width * height * 3];
  uchar *p = rgb;
  uchar *q;
  for (int j = 0; j < height; j++) {
    q = off_buf->buffer + j*off_buf->stride;
    for (int i = 0; i < off_buf->width; i++) { // exchange R and B colors, transmit G
      *p = *(q+2);
      *(p+1) = *(q+1);
      *(p+2) = *q;
      p += 3; q += 4;
    }
  }
  Fl_RGB_Image *image = new Fl_RGB_Image(rgb, off_buf->width, height, 3);
  image->alloc_array = 1;
  return image;
}
