//
// Definition of class Fl_Wayland_Graphics_Driver.
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

/**
 \file Fl_Wayland_Graphics_Driver.H
 \brief Definition of Wayland graphics driver.
 */

#ifndef FL_WAYLAND_GRAPHICS_DRIVER_H
#define FL_WAYLAND_GRAPHICS_DRIVER_H


/* Implementation note about buffers FLTK uses to support display graphics under Wayland.

 Each window is associated to an FLTK-defined object of type struct wld_window
 containing itself an FLTK-defined struct fl_wld_buffer object holding all graphics data.
 Among members of this latter structure are:
 - struct wl_buffer wl_buffer
 is a Wayland-defined type for a graphics buffer able to be attached to a wl_surface;
 - void *data
 points to the beginning of the memory zone where wl_buffer stores its graphics data;
 - unsigned char *draw_buffer
 contains a graphics buffer to which all Cairo drawings are directed;
 draw_buffer and data both have the same organization called CAIRO_FORMAT_ARGB32 in Cairo parlance
 and WL_SHM_FORMAT_ARGB8888 in Wayland parlance which means BGRA byte order.
 - int width
 gives the pixel width of the graphics buffer;
 - int stride
 gives the stride of this buffer;
 - size_t data_size
 gives the total buffer size in bytes (thus, data_size / stride gives the buffer height);
 - struct wl_callback *cb
 is used to synchronize drawing with the compositor during progressive drawing.

 When a graphics scene is to be committed, the data_size bytes of draw_buffer are copied by memcpy()
 starting at data, and wl_buffer is attached to the wl_surface which is committed for display
 by wl_surface_commit().
 */


#include "../Cairo/Fl_Cairo_Graphics_Driver.H"
#include <stdint.h> // for uint32_t

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
};
struct wld_window;


class Fl_Wayland_Graphics_Driver : public Fl_Cairo_Graphics_Driver {
private:
  struct fl_wld_buffer *buffer_;
public:
  Fl_Wayland_Graphics_Driver();
  static const uint32_t wld_format;
  void set_buffer(struct fl_wld_buffer *buffer, float scale = 0);
  virtual void copy_offscreen(int x, int y, int w, int h, Fl_Offscreen osrc, int srcx, int srcy);
  static struct fl_wld_buffer *create_shm_buffer(int width, int height);
  static void buffer_release(struct wld_window *window);
  static void buffer_commit(struct wld_window *window, bool need_damage = true);
  static void cairo_init(struct fl_wld_buffer *buffer, int width, int height, int stride, cairo_format_t format);
  virtual void *gc();
  virtual void gc(void *gc);
};

#endif // FL_WAYLAND_GRAPHICS_DRIVER_H
