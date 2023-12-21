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
  shape_data_ = NULL;
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
  if (shape_data_) {
    cairo_surface_t *surf;
    cairo_pattern_get_surface(shape_data_->mask_pattern_, &surf);
    unsigned char *bits = cairo_image_surface_get_data(surf);
    cairo_pattern_destroy(shape_data_->mask_pattern_);
    delete[] bits;
    struct Fl_Wayland_Graphics_Driver::draw_buffer *off_ =
    Fl_Wayland_Graphics_Driver::offscreen_buffer((Fl_Offscreen)shape_data_->bg_cr);
    delete[] off_->buffer;
    free(off_);
    cairo_destroy(shape_data_->bg_cr);
    free(shape_data_);
  }
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
  if (shape_data_ && shape_data_->mask_pattern_) {
    // draw above the secondary offscreen the main offscreen masked by mask_pattern_
    cairo_t *c = ((Fl_Cairo_Graphics_Driver*)driver())->cr();
    cairo_pattern_t *paint_pattern = cairo_pattern_create_for_surface(cairo_get_target(c));
    cairo_set_source(shape_data_->bg_cr, paint_pattern);
    cairo_mask(shape_data_->bg_cr, shape_data_->mask_pattern_);
    cairo_pattern_destroy(paint_pattern);
    // copy secondary offscreen to the main offscreen
    cairo_pattern_t *pat = cairo_pattern_create_for_surface(cairo_get_target(shape_data_->bg_cr));
    cairo_scale(c, shape_data_->scale, shape_data_->scale);
    cairo_set_source(c, pat),
    cairo_paint(c);
    cairo_pattern_destroy(pat);
    // delete secondary offscreen
    cairo_surface_t *surf;
    cairo_pattern_get_surface(shape_data_->mask_pattern_, &surf);
    unsigned char *bits = cairo_image_surface_get_data(surf);
    cairo_pattern_destroy(shape_data_->mask_pattern_);
    delete[] bits;
    struct Fl_Wayland_Graphics_Driver::draw_buffer *off_ =
    Fl_Wayland_Graphics_Driver::offscreen_buffer((Fl_Offscreen)shape_data_->bg_cr);
    delete[] off_->buffer;
    free(off_);
    cairo_destroy(shape_data_->bg_cr);
    free(shape_data_);
    shape_data_ = NULL;
  }

  // Convert depth-4 image in draw_buffer to a depth-3 image while exchanging R and B colors
  struct Fl_Wayland_Graphics_Driver::draw_buffer *off_buf =
    Fl_Wayland_Graphics_Driver::offscreen_buffer(offscreen);
  int height = int(off_buf->data_size / off_buf->stride);
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


void Fl_Wayland_Image_Surface_Driver::mask(const Fl_RGB_Image *mask) {
  bool using_copy = false;
  shape_data_ =  (struct shape_data_type*)calloc(1, sizeof(struct shape_data_type));
  int W, H;
  struct Fl_Wayland_Graphics_Driver::draw_buffer *off_buf =
  Fl_Wayland_Graphics_Driver::offscreen_buffer(offscreen);
  W = off_buf->width;
  H = (int)(off_buf->data_size / off_buf->stride);
  if (W != mask->data_w() || H != mask->data_h()) {
    Fl_RGB_Image *copy = (Fl_RGB_Image*)mask->copy(W, H);
    mask = copy;
    using_copy = true;
  }
  shape_data_->mask_pattern_ = Fl_Cairo_Graphics_Driver::calc_cairo_mask(mask);
  //duplicate current offscreen content to new cairo_t* shape_data_->bg_cr
  int width, height;
  printable_rect(&width, &height);
  struct Fl_Wayland_Graphics_Driver::draw_buffer *off_ =
  (struct Fl_Wayland_Graphics_Driver::draw_buffer*)calloc(1,
                                                          sizeof(struct Fl_Wayland_Graphics_Driver::draw_buffer));
  Fl_Wayland_Graphics_Driver::cairo_init(off_, W, H,
                                         cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, W),
                                         CAIRO_FORMAT_RGB24);
  cairo_set_user_data(off_->cairo_, &Fl_Wayland_Graphics_Driver::key, off_, NULL);
  shape_data_->bg_cr = off_->cairo_;
  memcpy(off_->buffer, off_buf->buffer, off_buf->data_size);
  shape_data_->scale = double(width) / W;
  if (using_copy) delete mask;
}
