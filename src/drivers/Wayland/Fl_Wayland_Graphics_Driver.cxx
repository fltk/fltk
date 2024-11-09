//
// Implementation of the Wayland graphics driver.
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

#include "Fl_Wayland_Graphics_Driver.H"
#include "Fl_Wayland_Screen_Driver.H"
#include "Fl_Wayland_Window_Driver.H"
#include <FL/Fl_Image_Surface.H>
#include <sys/mman.h>
#include <unistd.h> // for close()
#include <errno.h>
#include <string.h> // for strerror()
#include <cairo/cairo.h>

extern "C" {
#  include "../../../libdecor/src/os-compatibility.h" // for libdecor_os_create_anonymous_file()
}

// used by create_shm_buffer and do_buffer_release
struct wl_shm_pool *Fl_Wayland_Graphics_Driver::current_pool = NULL;


static void do_buffer_release(struct Fl_Wayland_Graphics_Driver::wld_buffer *);


static void buffer_release_listener(void *user_data, struct wl_buffer *wl_buffer)
{
  struct Fl_Wayland_Graphics_Driver::wld_buffer *buffer =
    (struct Fl_Wayland_Graphics_Driver::wld_buffer*)user_data;
  buffer->in_use = false;
  if (buffer->released) do_buffer_release(buffer);
}


static const struct wl_buffer_listener buffer_listener = {
  buffer_release_listener
};


void Fl_Wayland_Graphics_Driver::create_shm_buffer(Fl_Wayland_Graphics_Driver::wld_buffer *buffer) {
  int width = buffer->draw_buffer.width;
  int stride = buffer->draw_buffer.stride;
  int height = buffer->draw_buffer.data_size / stride;
  const size_t default_pool_size = 10000000; // larger pools are possible if needed
  int chunk_offset = 0; // offset to start of available memory in pool
  struct wld_shm_pool_data *pool_data = current_pool ? // data record attached to current pool
    (struct wld_shm_pool_data *)wl_shm_pool_get_user_data(current_pool) : NULL;
  size_t pool_size = current_pool ? pool_data->pool_size : default_pool_size; // current pool size
  if (current_pool && !wl_list_empty(&pool_data->buffers)) {
    // last wld_buffer created from current pool
    struct wld_buffer *record = wl_container_of(pool_data->buffers.next, record, link);
    chunk_offset = ((char*)record->data - pool_data->pool_memory) +
                   record->draw_buffer.data_size;
  }
  if (!current_pool || chunk_offset + buffer->draw_buffer.data_size > pool_size) {
    // if true, a new pool is needed
    if (current_pool && wl_list_empty(&pool_data->buffers)) {
      wl_shm_pool_destroy(current_pool);
      /*int err = */munmap(pool_data->pool_memory, pool_data->pool_size);
//      printf("create_shm_buffer munmap(%p)->%d\n", pool_data->pool_memory, err);
      free(pool_data);
    }
    chunk_offset = 0;
    pool_size = default_pool_size;
    if (buffer->draw_buffer.data_size > pool_size)
      pool_size = 2 * buffer->draw_buffer.data_size; // a larger pool is needed
    int fd = libdecor_os_create_anonymous_file(pool_size);
    if (fd < 0) {
      Fl::fatal("libdecor_os_create_anonymous_file failed: %s\n", strerror(errno));
    }
    pool_data = (struct wld_shm_pool_data*)calloc(1, sizeof(struct wld_shm_pool_data));
    pool_data->pool_memory = (char*)mmap(NULL, pool_size, PROT_READ | PROT_WRITE,
                                         MAP_SHARED, fd, 0);
    if (pool_data->pool_memory == MAP_FAILED) {
      close(fd);
      Fl::fatal("mmap failed: %s\n", strerror(errno));
    }
    Fl_Wayland_Screen_Driver *scr_driver = (Fl_Wayland_Screen_Driver*)Fl::screen_driver();
    current_pool = wl_shm_create_pool(scr_driver->wl_shm, fd, (int32_t)pool_size);
    close(fd); // does not prevent the mmap'ed memory from being used
    //printf("wl_shm_create_pool %p size=%lu\n",pool_data->pool_memory , pool_size);
    pool_data->pool_size = pool_size;
    wl_list_init(&pool_data->buffers);
    wl_shm_pool_set_user_data(current_pool, pool_data);
  }
  buffer->wl_buffer = wl_shm_pool_create_buffer(current_pool, chunk_offset,
                                                width, height, stride, wld_format);
  wl_buffer_add_listener(buffer->wl_buffer, &buffer_listener, buffer);
  // add this buffer to head of list of current pool's buffers
  wl_list_insert(&pool_data->buffers, &buffer->link);
  buffer->shm_pool = current_pool;
  buffer->data = (void*)(pool_data->pool_memory + chunk_offset);
//fprintf(stderr, "last=%p chunk_offset=%d ", pool_data->buffers.next, chunk_offset);
//fprintf(stderr, "create_shm_buffer: %dx%d = %d\n", width, height, size);
}


struct Fl_Wayland_Graphics_Driver::wld_buffer *
    Fl_Wayland_Graphics_Driver::create_wld_buffer(int width, int height, bool with_shm) {
  struct wld_buffer *buffer = (struct wld_buffer*)calloc(1, sizeof(struct wld_buffer));
  int stride = cairo_format_stride_for_width(cairo_format, width);
  cairo_init(&buffer->draw_buffer, width, height, stride, cairo_format);
  buffer->draw_buffer_needs_commit = true;
  if (with_shm) create_shm_buffer(buffer);
  return buffer;
}


// used to support both normal and progressive drawing and for top-level GL windows
static void surface_frame_done(void *data, struct wl_callback *cb, uint32_t time) {
  struct wld_window *window = (struct wld_window *)data;
  wl_callback_destroy(cb);
  window->frame_cb = NULL;
  if (window->buffer && window->buffer->draw_buffer_needs_commit) {
    Fl_Wayland_Graphics_Driver::buffer_commit(window);
  }
}


static const struct wl_callback_listener surface_frame_listener = {
  .done = surface_frame_done,
};


const struct wl_callback_listener *Fl_Wayland_Graphics_Driver::p_surface_frame_listener =
  &surface_frame_listener;


// copy pixels in region r from the Cairo surface to the Wayland buffer
static void copy_region(struct wld_window *window, cairo_region_t *r) {
  struct Fl_Wayland_Graphics_Driver::wld_buffer *buffer = window->buffer;
  float f = Fl::screen_scale(window->fl_win->screen_num());
  int d = Fl_Wayland_Window_Driver::driver(window->fl_win)->wld_scale();
  int count = cairo_region_num_rectangles(r);
  cairo_rectangle_int_t rect;
  for (int i = 0; i < count; i++) {
    cairo_region_get_rectangle(r, i, &rect);
    int left = d * int(rect.x * f);
    int top = d * int(rect.y * f);
    int right = d * ceil((rect.x + rect.width) * f);
    if (right > d * int(window->fl_win->w() * f)) right = d * int(window->fl_win->w() * f);
    int width = right - left;
    int bottom = d * ceil((rect.y + rect.height) * f);
    if (bottom > d * int(window->fl_win->h() * f)) bottom = d * int(window->fl_win->h() * f);
    int height = bottom - top;
    int offset = top * buffer->draw_buffer.stride + 4 * left;
    int W4 = 4 * width;
    for (int l = 0; l < height; l++) {
      if (offset + W4 >= (int)buffer->draw_buffer.data_size) {
        W4 = buffer->draw_buffer.data_size - offset;
        if (W4 <= 0) break;
      }
      memcpy((uchar*)buffer->data + offset, buffer->draw_buffer.buffer + offset, W4);
      offset += buffer->draw_buffer.stride;
    }
    wl_surface_damage_buffer(window->wl_surface, left, top, width, height);
  }
}


void Fl_Wayland_Graphics_Driver::buffer_commit(struct wld_window *window, cairo_region_t *r)
{
  if (!window->buffer->wl_buffer) create_shm_buffer(window->buffer);
  cairo_surface_t *surf = cairo_get_target(window->buffer->draw_buffer.cairo_);
  cairo_surface_flush(surf);
  if (r) copy_region(window, r);
  else {
    memcpy(window->buffer->data, window->buffer->draw_buffer.buffer,
           window->buffer->draw_buffer.data_size);
    wl_surface_damage_buffer(window->wl_surface, 0, 0, 1000000, 1000000);
  }
  window->buffer->in_use = true;
  wl_surface_attach(window->wl_surface, window->buffer->wl_buffer, 0, 0);
  wl_surface_set_buffer_scale( window->wl_surface,
      Fl_Wayland_Window_Driver::driver(window->fl_win)->wld_scale() );
  if (!window->covered) { // see issue #878
    window->frame_cb = wl_surface_frame(window->wl_surface);
    wl_callback_add_listener(window->frame_cb, p_surface_frame_listener, window);
  }
  wl_surface_commit(window->wl_surface);
  window->buffer->draw_buffer_needs_commit = false;
}


void Fl_Wayland_Graphics_Driver::cairo_init(struct Fl_Wayland_Graphics_Driver::draw_buffer *buffer,
                                            int width, int height, int stride,
                                            cairo_format_t format) {
  buffer->data_size = stride * height;
  buffer->stride = stride;
  buffer->buffer = new uchar[buffer->data_size];
  buffer->width = width;
  cairo_surface_t *surf = cairo_image_surface_create_for_data(buffer->buffer, format,
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
  cairo_surface_destroy(surf);
  memset(buffer->buffer, 0, buffer->data_size); // useful for transparent windows
  cairo_set_source_rgba(buffer->cairo_, .0, .0, .0, 1.0); // Black default color
  cairo_save(buffer->cairo_);
}


// runs when buffer->in_use is false and buffer->released is true
static void do_buffer_release(struct Fl_Wayland_Graphics_Driver::wld_buffer *buffer) {
  struct wl_shm_pool *my_pool = buffer->shm_pool;
  if (buffer->wl_buffer) {
    struct Fl_Wayland_Graphics_Driver::wld_shm_pool_data *pool_data =
    (struct Fl_Wayland_Graphics_Driver::wld_shm_pool_data*)
    wl_shm_pool_get_user_data(my_pool);
    wl_buffer_destroy(buffer->wl_buffer);
    // remove wld_buffer from list of pool's buffers
    wl_list_remove(&buffer->link);
    if (wl_list_empty(&pool_data->buffers) && my_pool != Fl_Wayland_Graphics_Driver::current_pool) {
      // all buffers from pool are gone
      wl_shm_pool_destroy(my_pool);
      /*int err = */munmap(pool_data->pool_memory, pool_data->pool_size);
      //printf("do_buffer_release munmap(%p)->%d\n", pool_data->pool_memory, err);
      free(pool_data);
    }
  }
  free(buffer);
}


void Fl_Wayland_Graphics_Driver::buffer_release(struct wld_window *window)
{
  if (window->buffer && !window->buffer->released) {
    window->buffer->released = true;
    if (window->frame_cb) { wl_callback_destroy(window->frame_cb); window->frame_cb = NULL; }
    delete[] window->buffer->draw_buffer.buffer;
    window->buffer->draw_buffer.buffer = NULL;
    cairo_destroy(window->buffer->draw_buffer.cairo_);
    if (!window->buffer->in_use) do_buffer_release(window->buffer);
    window->buffer = NULL;
  }
}


// this refers to the same memory layout for pixel data as does CAIRO_FORMAT_ARGB32
const uint32_t Fl_Wayland_Graphics_Driver::wld_format = WL_SHM_FORMAT_ARGB8888;


void Fl_Wayland_Graphics_Driver::copy_offscreen(int x, int y, int w, int h,
                                                Fl_Offscreen src, int srcx, int srcy) {
  // draw portion srcx,srcy,w,h of osrc to position x,y (top-left) of
  // the graphics driver's surface
  cairo_matrix_t matrix;
  cairo_get_matrix(cairo_, &matrix);
  double s = matrix.xx;
  cairo_save(cairo_);
  cairo_rectangle(cairo_, x - 0.5, y - 0.5, w, h);
  cairo_set_antialias(cairo_, CAIRO_ANTIALIAS_NONE);
  cairo_clip(cairo_);
  cairo_set_antialias(cairo_, CAIRO_ANTIALIAS_DEFAULT);
  cairo_surface_t *surf = cairo_get_target((cairo_t *)src);
  cairo_pattern_t *pat = cairo_pattern_create_for_surface(surf);
  cairo_set_source(cairo_, pat);
  cairo_matrix_init_scale(&matrix, s, s);
  cairo_matrix_translate(&matrix, -(x - srcx), -(y - srcy));
  cairo_pattern_set_matrix(pat, &matrix);
  cairo_paint(cairo_);
  cairo_pattern_destroy(pat);
  cairo_restore(cairo_);
  surface_needs_commit();
}


const cairo_user_data_key_t Fl_Wayland_Graphics_Driver::key = {};


struct Fl_Wayland_Graphics_Driver::draw_buffer*
Fl_Wayland_Graphics_Driver::offscreen_buffer(Fl_Offscreen offscreen) {
  return (struct draw_buffer*)cairo_get_user_data((cairo_t*)offscreen, &key);
}


Fl_Image_Surface *Fl_Wayland_Graphics_Driver::custom_offscreen(int w, int h,
                    struct Fl_Wayland_Graphics_Driver::wld_buffer **p_off) {
  struct wld_buffer *off = create_wld_buffer(w, h);
  *p_off = off;
  cairo_set_user_data(off->draw_buffer.cairo_, &key, &off->draw_buffer, NULL);
  return new Fl_Image_Surface(w, h, 0, (Fl_Offscreen)off->draw_buffer.cairo_);
}


void Fl_Wayland_Graphics_Driver::cache_size(Fl_Image *img, int &width, int &height) {
  Fl_Graphics_Driver::cache_size(img, width, height);
  width *= wld_scale;
  height *=  wld_scale;
}
