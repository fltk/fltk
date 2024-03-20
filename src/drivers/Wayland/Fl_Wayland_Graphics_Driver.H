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


class Fl_Wayland_Graphics_Driver : public Fl_Cairo_Graphics_Driver {
public:
  struct draw_buffer {
    unsigned char *buffer;
    cairo_t *cairo_;
    size_t data_size; // of wl_buffer and buffer
    int stride;
    int width;
  };
  struct wld_buffer {
    struct draw_buffer draw_buffer;
    struct wl_list link; // links all buffers from the same wl_shm_pool
    struct wl_buffer *wl_buffer;
    void *data;
    struct wl_shm_pool *shm_pool;
    bool draw_buffer_needs_commit;
    bool in_use; // true while being committed
    bool released; // true after buffer_release() was called
  };
  struct wld_shm_pool_data { // one record attached to each wl_shm_pool object
    char *pool_memory; // start of mmap'ed memory encapsulated by the wl_shm_pool
    size_t pool_size; // size of encapsulated memory
    struct wl_list buffers; // to list of fl_wld_buffer's from this pool
  };
  static const uint32_t wld_format;
  static struct wl_shm_pool *current_pool;
  static FL_EXPORT const struct wl_callback_listener *p_surface_frame_listener;
  void copy_offscreen(int x, int y, int w, int h, Fl_Offscreen osrc,
                      int srcx, int srcy) FL_OVERRIDE;
  void cache_size(Fl_Image *img, int &width, int &height) FL_OVERRIDE;
  static struct wld_buffer *create_wld_buffer(int width, int height, bool with_shm = true);
  static void create_shm_buffer(wld_buffer *buffer);
  static void buffer_release(struct wld_window *window);
  static void buffer_commit(struct wld_window *window, cairo_region_t *r = NULL);
  static void cairo_init(struct draw_buffer *buffer, int width, int height, int stride,
                         cairo_format_t format);
  // used by class Fl_Wayland_Gl_Window_Driver
  static FL_EXPORT struct draw_buffer *offscreen_buffer(Fl_Offscreen);
  static const cairo_user_data_key_t key;
  static Fl_Image_Surface *custom_offscreen(int w, int h, struct wld_buffer **buffer);
};

#endif // FL_WAYLAND_GRAPHICS_DRIVER_H
