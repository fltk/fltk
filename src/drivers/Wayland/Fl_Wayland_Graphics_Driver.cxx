//
// Implementation of the Wayland graphics driver.
//
// Copyright 2021-2022 by Bill Spitzak and others.
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
#include "Fl_Wayland_Screen_Driver.H"
#include "Fl_Wayland_Window_Driver.H"
#include "text-input-client-protocol.h"
#include <pango/pangocairo.h>
#if ! PANGO_VERSION_CHECK(1,22,0)
#  error "Requires Pango 1.22 or higher"
#endif
#define _GNU_SOURCE 1
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

extern unsigned fl_cmap[256]; // defined in fl_color.cxx


static int create_anonymous_file(int size, char **pshared)
{
  int ret;
  int fd = memfd_create("FLTK-for-Wayland", MFD_CLOEXEC | MFD_ALLOW_SEALING);
  if (fd < 0) {
    Fl::fatal("memfd_create failed: %s\n", strerror(errno));
  }
  fcntl(fd, F_ADD_SEALS, F_SEAL_SHRINK);
  do {
    ret = posix_fallocate(fd, 0, size);
  } while (ret == EINTR);
  if (ret != 0) {
    close(fd);
    errno = ret;
    Fl::fatal("creating anonymous file of size %d failed: %s\n", size, strerror(errno));
  }
  *pshared = (char*)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (*pshared == MAP_FAILED) {
    close(fd);
    Fl::fatal("mmap failed: %s\n", strerror(errno));
  }
//printf("create_anonymous_file: %d\n",size);
  return fd;
}


struct fl_wld_buffer *Fl_Wayland_Graphics_Driver::create_shm_buffer(int width, int height)
{
  struct fl_wld_buffer *buffer;
  int stride = cairo_format_stride_for_width(Fl_Cairo_Graphics_Driver::cairo_format, width);
  int size = stride * height;
  static char *pool_memory = NULL;
  static int pool_size = 10000000; // gets increased if necessary
  static int chunk_offset = pool_size;
  static int fd = -1;
  static struct wl_shm_pool *pool = NULL;
  if (chunk_offset + size > pool_size) {
    chunk_offset = 0;
    if (pool) {
      wl_shm_pool_destroy(pool);
      close(fd);
    }
    if (size > pool_size) pool_size = 2 * size;
    fd = create_anonymous_file(pool_size, &pool_memory);
    Fl_Wayland_Screen_Driver *scr_driver = (Fl_Wayland_Screen_Driver*)Fl::screen_driver();
    pool = wl_shm_create_pool(scr_driver->wl_shm, fd, pool_size);
  }
  buffer = (struct fl_wld_buffer*)calloc(1, sizeof(struct fl_wld_buffer));
  buffer->stride = stride;
  buffer->wl_buffer = wl_shm_pool_create_buffer(pool, chunk_offset, width, height, stride, Fl_Wayland_Graphics_Driver::wld_format);
  buffer->data = (void*)(pool_memory + chunk_offset);
  chunk_offset += size;
  buffer->data_size = size;
  buffer->width = width;
  buffer->draw_buffer = new uchar[buffer->data_size];
  buffer->draw_buffer_needs_commit = false;
//fprintf(stderr, "create_shm_buffer: %dx%d = %d\n", width, height, size);
  cairo_init(buffer, width, height, stride, Fl_Cairo_Graphics_Driver::cairo_format);
  return buffer;
}


void Fl_Wayland_Graphics_Driver::buffer_commit(struct wld_window *window) {
  cairo_surface_t *surf = cairo_get_target(window->buffer->cairo_);
  cairo_surface_flush(surf);
  memcpy(window->buffer->data, window->buffer->draw_buffer, window->buffer->data_size);
  wl_surface_attach(window->wl_surface, window->buffer->wl_buffer, 0, 0);
  wl_surface_set_buffer_scale(window->wl_surface, window->scale);
  wl_surface_commit(window->wl_surface);
  window->buffer->draw_buffer_needs_commit = false;
//fprintf(stderr,"buffer_commit %s\n", window->fl_win->parent()?"child":"top");
}


void Fl_Wayland_Graphics_Driver::cairo_init(struct fl_wld_buffer *buffer, int width, int height, int stride, cairo_format_t format) {
  cairo_surface_t *surf = cairo_image_surface_create_for_data(buffer->draw_buffer, format,
                                                        width, height, stride);
  if (cairo_surface_status(surf) != CAIRO_STATUS_SUCCESS) {
    Fl::fatal("Can't create Cairo surface with cairo_image_surface_create_for_data()\n");
    return;
  }
  buffer->cairo_ = cairo_create(surf);
  cairo_status_t err;
  if ((err = cairo_status(buffer->cairo_)) != CAIRO_STATUS_SUCCESS) {
    Fl::fatal("Cairo error during cairo_create() %s\n", cairo_status_to_string(err));
    return;
  }
  cairo_set_source_rgba(buffer->cairo_, 1.0, 1.0, 1.0, 0.);
  cairo_paint(buffer->cairo_);
  cairo_set_source_rgba(buffer->cairo_, .0, .0, .0, 1.0); // Black default color
  buffer->pango_layout_ = pango_cairo_create_layout(buffer->cairo_);
  cairo_save(buffer->cairo_);
}


void Fl_Wayland_Graphics_Driver::buffer_release(struct wld_window *window)
{
  if (window->buffer) {
    wl_buffer_destroy(window->buffer->wl_buffer);
    delete[] window->buffer->draw_buffer;
    window->buffer->draw_buffer = NULL;
    cairo_surface_t *surf = cairo_get_target(window->buffer->cairo_);
    cairo_destroy(window->buffer->cairo_);
    cairo_surface_destroy(surf);
    g_object_unref(window->buffer->pango_layout_);
    free(window->buffer);
    window->buffer = NULL;
  }
}

// these 2 refer to the same memory layout for pixel data
const uint32_t Fl_Wayland_Graphics_Driver::wld_format = WL_SHM_FORMAT_ARGB8888;


Fl_Wayland_Graphics_Driver::Fl_Wayland_Graphics_Driver () : Fl_Cairo_Graphics_Driver() {
  buffer_ = NULL;
}


void Fl_Wayland_Graphics_Driver::activate(struct fl_wld_buffer *buffer, float scale) {
  if (dummy_pango_layout_) {
    cairo_surface_t *surf = cairo_get_target(cairo_);
    cairo_destroy(cairo_);
    cairo_surface_destroy(surf);
    g_object_unref(dummy_pango_layout_);
    dummy_pango_layout_ = NULL;
    pango_layout_ = NULL;
  }
  cairo_ = buffer->cairo_;
  if (pango_layout_ != buffer->pango_layout_) {
    if (pango_layout_) g_object_unref(pango_layout_);
    pango_layout_ = buffer->pango_layout_;
    g_object_ref(pango_layout_);
    Fl_Graphics_Driver::font(-1, -1); // signal that no font is current yet
  }
  this->buffer_ = buffer;
  cairo_restore(cairo_);
  cairo_save(cairo_);
  cairo_scale(cairo_, scale, scale);
  cairo_translate(cairo_, 0.5, 0.5);
  line_style(0);
}


void Fl_Wayland_Graphics_Driver::set_color(Fl_Color i, unsigned c) {
  if (fl_cmap[i] != c) {
    fl_cmap[i] = c;
  }
}


void Fl_Wayland_Graphics_Driver::set_spot(int font, int height, int x, int y, int w, int h, Fl_Window *win) {
  Fl_Wayland_Screen_Driver::insertion_point_location(x, y, height);
}


void Fl_Wayland_Graphics_Driver::reset_spot() {
  Fl::compose_state = 0;
  Fl_Wayland_Screen_Driver::next_marked_length = 0;
  Fl_Wayland_Screen_Driver::insertion_point_location_is_valid = false;
}


void Fl_Wayland_Graphics_Driver::copy_offscreen(int x, int y, int w, int h, Fl_Offscreen osrc, int srcx, int srcy) {
  // draw portion srcx,srcy,w,h of osrc to position x,y (top-left) of the graphics driver's surface
  int height = osrc->data_size / osrc->stride;
  cairo_matrix_t matrix;
  cairo_get_matrix(cairo_, &matrix);
  double s = matrix.xx;
  cairo_save(cairo_);
  cairo_rectangle(cairo_, x, y, w, h);
  cairo_clip(cairo_);
  cairo_surface_t *surf = cairo_image_surface_create_for_data(osrc->draw_buffer, Fl_Cairo_Graphics_Driver::cairo_format, osrc->width, height, osrc->stride);
  cairo_pattern_t *pat = cairo_pattern_create_for_surface(surf);
  cairo_set_source(cairo_, pat);
  cairo_matrix_init_scale(&matrix, s, s);
  cairo_matrix_translate(&matrix, -(x - srcx), -(y - srcy));
  cairo_pattern_set_matrix(pat, &matrix);
  cairo_mask(cairo_, pat);
  cairo_pattern_destroy(pat);
  cairo_surface_destroy(surf);
  cairo_restore(cairo_);
}


void Fl_Wayland_Graphics_Driver::gc(void *off) {} // equivalent is done by activate()


void *Fl_Wayland_Graphics_Driver::gc() {
  return buffer_;
}

