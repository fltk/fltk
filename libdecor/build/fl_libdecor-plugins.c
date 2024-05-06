//
// Interface with the libdecor library for the Fast Light Tool Kit (FLTK).
//
// Copyright 2022-2024 by Bill Spitzak and others.
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

/* Support of interactions between FLTK and libdecor plugins, either dynamically
 loaded by dlopen() or built-in FLTK.
 
 Under USE_SYSTEM_LIBDECOR, the plugin can only be dynamically loaded.
 Under ! USE_SYSTEM_LIBDECOR, it can be dynamically loaded from a directory
 given in environment variable LIBDECOR_PLUGIN_DIR, or the built-in one is used.
 */

#include <dlfcn.h>
#include <string.h>
#include "fl_libdecor.h"
#include <pango/pangocairo.h>
#include <dlfcn.h>

#ifndef HAVE_GTK
#  define HAVE_GTK 0
#endif

enum plugin_kind { UNKNOWN, SSD, CAIRO, GTK3 };

#if USE_SYSTEM_LIBDECOR
#  include "../src/libdecor-plugin.h"

enum component {NONE}; /* details are not necessary*/
enum decoration_type {DECORATION_TYPE_NONE}; /* details are not necessary*/

struct buffer { // identical in libdecor-cairo.c and libdecor-gtk.c
  struct wl_buffer *wl_buffer;
  bool in_use;
  bool is_detached;

  void *data;
  size_t data_size;
  int width;
  int height;
  int scale;
  int buffer_width;
  int buffer_height;
};

#else // !USE_SYSTEM_LIBDECOR

const struct libdecor_plugin_description *fl_libdecor_plugin_description = NULL;

#  if HAVE_GTK
#    include "../src/plugins/gtk/libdecor-gtk.c"
#  else
#    include "../src/plugins/cairo/libdecor-cairo.c"
#  endif // HAVE_GTK

#endif // USE_SYSTEM_LIBDECOR


#if USE_SYSTEM_LIBDECOR || HAVE_GTK
/* these definitions derive from libdecor/src/plugins/cairo/libdecor-cairo.c */

enum composite_mode {COMPOSITE_SERVER}; /* details are not necessary*/

struct border_component_cairo {
  enum component type;

  bool is_hidden;
  bool opaque;

  enum composite_mode composite_mode;
  struct {
    struct wl_surface *wl_surface;
    struct wl_subsurface *wl_subsurface;
    struct buffer *buffer;
    struct wl_list output_list;
    int scale;
  } server;
  struct {
    cairo_surface_t *image;
    struct border_component_cairo *parent_component;
  } client;

  struct wl_list child_components; /* border_component::link */
  struct wl_list link; /* border_component::child_components */
};

struct libdecor_frame_cairo {
  struct libdecor_frame frame;

  struct libdecor_plugin_cairo *plugin_cairo;

  int content_width;
  int content_height;

  enum decoration_type decoration_type;

  enum libdecor_window_state window_state;

  char *title;

  enum libdecor_capabilities capabilities;

  struct border_component_cairo *focus;
  struct border_component_cairo *active;
  struct border_component_cairo *grab;

  bool shadow_showing;
  struct border_component_cairo shadow;

  struct {
    bool is_showing;
    struct border_component_cairo title;
    struct border_component_cairo min;
    struct border_component_cairo max;
    struct border_component_cairo close;
  } title_bar;

  /* store pre-processed shadow tile */
  cairo_surface_t *shadow_blur;

  struct wl_list link;
};
#endif // USE_SYSTEM_LIBDECOR || HAVE_GTK


#if USE_SYSTEM_LIBDECOR || !HAVE_GTK

/* Definitions derived from libdecor-gtk.c */

typedef struct _GtkWidget GtkWidget;
enum header_element { HEADER_NONE }; /* details are not needed */

typedef enum { GTK_STATE_FLAG_NORMAL = 0 } GtkStateFlags;

struct border_component_gtk {
  enum component type;
  struct wl_surface *wl_surface;
  struct wl_subsurface *wl_subsurface;
  struct buffer *buffer;
  bool opaque;
  struct wl_list output_list;
  int scale;
  struct wl_list child_components; /* border_component::link */
  struct wl_list link; /* border_component::child_components */
};

struct header_element_data {
  const char* name;
  enum header_element type;
  GtkWidget *widget;
  GtkStateFlags state;
};

struct libdecor_frame_gtk {
  struct libdecor_frame frame;
  struct libdecor_plugin_gtk *plugin_gtk;
  int content_width;
  int content_height;
  enum libdecor_window_state window_state;
  enum decoration_type decoration_type;
  char *title;
  enum libdecor_capabilities capabilities;
  struct border_component_gtk *active;
  struct border_component_gtk *touch_active;
  struct border_component_gtk *focus;
  struct border_component_gtk *grab;
  bool shadow_showing;
  struct border_component_gtk shadow;
  GtkWidget *window; /* offscreen window for rendering */
  GtkWidget *header; /* header bar with widgets */
  struct border_component_gtk headerbar;
  struct header_element_data hdr_focus;
  cairo_surface_t *shadow_blur;
  struct wl_list link;
  struct {
    enum titlebar_gesture_state {TITLEBAR_GESTURE_STATE_INIT} state;
    int button_pressed_count;
    uint32_t first_pressed_button;
    uint32_t first_pressed_time;
    double pressed_x;
    double pressed_y;
    uint32_t pressed_serial;
  } titlebar_gesture;
};

#endif // USE_SYSTEM_LIBDECOR || !HAVE_GTK


static unsigned char *gtk_titlebar_buffer(struct libdecor_frame *frame,
                                          int *width, int *height, int *stride)
{
  struct libdecor_frame_gtk *lfg = (struct libdecor_frame_gtk *)frame;
  struct buffer *buffer = lfg->headerbar.buffer;
  *width = buffer->buffer_width;
  *height = buffer->buffer_height;
  *stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, buffer->buffer_width);
  return (unsigned char*)buffer->data;
}


static unsigned char *cairo_titlebar_buffer(struct libdecor_frame *frame,
                                                 int *width, int *height, int *stride)
{
  struct libdecor_frame_cairo *lfc = (struct libdecor_frame_cairo *)frame;
  struct buffer *buffer = lfc->title_bar.title.server.buffer;
  *width = buffer->buffer_width;
  *height = buffer->buffer_height;
  *stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, buffer->buffer_width);
  return (unsigned char*)buffer->data;
}


/*
 Although each plugin declares an exported global variable
 LIBDECOR_EXPORT const struct libdecor_plugin_description libdecor_plugin_description;
 these plugins are dlopen()'ed in libdecor.c without the RTLD_GLOBAL flag.
 Consequently their symbols are not discovered by dlsym(RTLD_DEFAULT, "symbol-name").
 
 Under USE_SYSTEM_LIBDECOR, we repeat the dlopen() for the same plugin
 then dlsym() will report the address of libdecor_plugin_description.
 
 Under !USE_SYSTEM_LIBDECOR, we compile fl_libdecor.c which modifies the dlopen()
 to call dlsym(ld, "libdecor_plugin_description") just after the dlopen and memorizes
 this address.
 
 A plugin is loaded also if SSD.
 KWin has its own size limit, similar to that of GDK plugin
 */
static const char *get_libdecor_plugin_description() {
  static const struct libdecor_plugin_description *plugin_description = NULL;
  if (!plugin_description) {
#if USE_SYSTEM_LIBDECOR
     char fname[PATH_MAX];
     const char *dir = getenv("LIBDECOR_PLUGIN_DIR");
     if (!dir) dir = LIBDECOR_PLUGIN_DIR;
     snprintf(fname, PATH_MAX, "%s/libdecor-gtk.so", dir);
     void *dl = dlopen(fname, RTLD_LAZY | RTLD_LOCAL);
     if (!dl) {
       snprintf(fname, PATH_MAX, "%s/libdecor-cairo.so", dir);
       dl = dlopen(fname, RTLD_LAZY | RTLD_LOCAL);
     }
     if (dl) plugin_description = (const struct libdecor_plugin_description*)dlsym(dl, "libdecor_plugin_description");
#else
     plugin_description = fl_libdecor_plugin_description;
     extern const struct libdecor_plugin_description libdecor_plugin_description;
     if (!plugin_description) plugin_description = &libdecor_plugin_description;
#endif
     //if (plugin_description) puts(plugin_description->description);
  }
  return plugin_description ? plugin_description->description : NULL;
}


static enum plugin_kind get_plugin_kind(struct libdecor_frame *frame) {
  static enum plugin_kind kind = UNKNOWN;
  if (kind == UNKNOWN) {
    if (frame) {
      int X, Y = 0;
      libdecor_frame_translate_coordinate(frame, 0, 0, &X, &Y);
      if (Y == 0) {
        return SSD;
      }
    }
    const char *name = get_libdecor_plugin_description();
    if (name && !strcmp(name, "GTK3 plugin")) kind = GTK3;
    else if (name && !strcmp(name, "libdecor plugin using Cairo")) kind = CAIRO;
  }
  return kind;
}

/*
 FLTK-added utility function to give access to the pixel array representing
 the titlebar of a window decorated by the cairo plugin of libdecor.
   frame: a libdecor-defined pointer given by fl_xid(win)->frame (with Fl_Window *win);
   *width, *height: returned assigned to the width and height in pixels of the titlebar;
   *stride: returned assigned to the number of bytes per line of the pixel array;
   return value: start of the pixel array, which is in BGRA order, or NULL.
 */
unsigned char *fl_libdecor_titlebar_buffer(struct libdecor_frame *frame,
                                                 int *width, int *height, int *stride)
{
  enum plugin_kind kind = get_plugin_kind(frame);
  if (kind == GTK3) {
    return gtk_titlebar_buffer(frame, width, height, stride);
  }
  else if (kind == CAIRO) {
    return cairo_titlebar_buffer(frame, width, height, stride);
  }
  return NULL;
}


/* Returns whether surface is the libdecor-created GTK-titlebar of frame */
bool fl_is_surface_from_GTK_titlebar (struct wl_surface *surface, struct libdecor_frame *frame,
                                      bool *using_GTK) {
  *using_GTK = (get_plugin_kind(NULL) == GTK3);
  if (!*using_GTK) return false;
  struct libdecor_frame_gtk *frame_gtk = (struct libdecor_frame_gtk*)frame;
  return (frame_gtk->headerbar.wl_surface == surface);
}
