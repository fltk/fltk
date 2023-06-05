//
// Definition of class Fl_Wayland_Graphics_Driver.
//
// Copyright 2021-2023 by Bill Spitzak and others.
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

/**
 \file Fl_Wayland_Graphics_Driver.H
 \brief Definition of Wayland graphics driver.
 */

#ifndef FL_WAYLAND_GRAPHICS_DRIVER_H
#define FL_WAYLAND_GRAPHICS_DRIVER_H

#include "../Cairo/Fl_Cairo_Graphics_Driver.H"
#include <stdint.h> // for uint32_t
#include <wayland-client.h> // for wl_list


struct fl_wld_buffer {
  struct wl_buffer *wl_buffer;
  void *data;
  size_t data_size; // of wl_buffer and draw_buffer
  int stride;
  int width;
  unsigned char *draw_buffer;
  struct wl_callback *cb;
  bool draw_buffer_needs_commit;
  cairo_t *cairo_;
  struct wl_shm_pool *shm_pool;
  struct wl_list link; // links all buffers from the same wl_shm_pool
};


class Fl_Wayland_Graphics_Driver : public Fl_Cairo_Graphics_Driver {
private:
  struct fl_wld_buffer *buffer_;
public:
  struct wld_shm_pool_data { // one record attached to each wl_shm_pool object
    char *pool_memory; // start of mmap'ed memory encapsulated by the wl_shm_pool
    int pool_size; // size of encapsulated memory
    struct wl_list buffers; // to list of fl_wld_buffer's from this pool
  };
  Fl_Wayland_Graphics_Driver();
  static const uint32_t wld_format;
  void set_buffer(struct fl_wld_buffer *buffer, float scale = 0);
  void copy_offscreen(int x, int y, int w, int h, Fl_Offscreen osrc, int srcx, int srcy) FL_OVERRIDE;
  static struct fl_wld_buffer *create_shm_buffer(int width, int height);
  static void buffer_release(struct wld_window *window);
  static void buffer_commit(struct wld_window *window, struct flCairoRegion *r = NULL);
  static void cairo_init(struct fl_wld_buffer *buffer, int width, int height, int stride, cairo_format_t format);
};

#endif // FL_WAYLAND_GRAPHICS_DRIVER_H
