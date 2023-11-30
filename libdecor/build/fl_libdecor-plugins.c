//
// Interface with the libdecor library for the Fast Light Tool Kit (FLTK).
//
// Copyright 2022-2023 by Bill Spitzak and others.
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
#include "../src/libdecor.h"
#include <pango/pangocairo.h>

#ifndef HAVE_GTK
#  define HAVE_GTK 0
#endif

#if HAVE_GTK
#  include "gtk-shell-client-protocol.h"
#endif

#if USE_SYSTEM_LIBDECOR
#include "../src/libdecor-plugin.h"
#if HAVE_GTK
#include <linux/input.h>
#include "gtk-shell-protocol.c"
#endif

enum zxdg_toplevel_decoration_v1_mode {
  ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE = 1,
  ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE = 2,
};

enum component {
  NONE = 0,
  SHADOW,
  HEADER,
};
enum decoration_type {DECORATION_TYPE_NONE};

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
#    include <gtk/gtk.h>
#    include "../src/plugins/gtk/libdecor-gtk.c"
#  else
#    include "../src/plugins/cairo/libdecor-cairo.c"
#    undef libdecor_frame_set_min_content_size
#  endif // HAVE_GTK

#endif // USE_SYSTEM_LIBDECOR


#if USE_SYSTEM_LIBDECOR || HAVE_GTK
/* these definitions derive from libdecor/src/plugins/cairo/libdecor-cairo.c */

struct libdecor_plugin_cairo {
  struct libdecor_plugin plugin;

  struct wl_callback *globals_callback;
  struct wl_callback *globals_callback_shm;

  struct libdecor *context;

  struct wl_registry *wl_registry;
  struct wl_subcompositor *wl_subcompositor;
  struct wl_compositor *wl_compositor;

  struct wl_shm *wl_shm;
  struct wl_callback *shm_callback;
  bool has_argb;

  struct wl_list visible_frame_list;
  struct wl_list seat_list;
  struct wl_list output_list;

  char *cursor_theme_name;
  int cursor_size;

  PangoFontDescription *font;
};

enum composite_mode {
  COMPOSITE_SERVER,
  COMPOSITE_CLIENT,
};

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
#endif


#if USE_SYSTEM_LIBDECOR || !HAVE_GTK

/* Definitions derived from libdecor-gtk.c */

typedef struct _GtkWidget GtkWidget;
enum header_element {
  HEADER_NONE,
  HEADER_FULL, /* entire header bar */
  HEADER_TITLE, /* label */
  HEADER_MIN,
  HEADER_MAX,
  HEADER_CLOSE,
};

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
};

#if USE_SYSTEM_LIBDECOR
struct libdecor_plugin_gtk {
  struct libdecor_plugin plugin;
  struct wl_callback *globals_callback;
  struct wl_callback *globals_callback_shm;
  struct libdecor *context;
  struct wl_registry *wl_registry;
  struct wl_subcompositor *wl_subcompositor;
  struct wl_compositor *wl_compositor;
  struct wl_shm *wl_shm;
  struct wl_callback *shm_callback;
  bool has_argb;
  struct wl_list visible_frame_list;
  struct wl_list seat_list;
  struct wl_list output_list;
  char *cursor_theme_name;
  int cursor_size;
  int double_click_time_ms;
};

struct seat {
  struct libdecor_plugin_gtk *plugin_gtk;
  char *name;
  struct wl_seat *wl_seat;
  struct wl_pointer *wl_pointer;
  struct wl_touch *wl_touch;
  struct wl_surface *cursor_surface;
  struct wl_cursor *current_cursor;
  int cursor_scale;
  struct wl_list cursor_outputs;
  struct wl_cursor_theme *cursor_theme;
  struct wl_cursor *cursors[8]; // 8 replaces ARRAY_LENGTH(cursor_names)
  struct wl_cursor *cursor_left_ptr;
  struct wl_surface *pointer_focus;
  struct wl_surface *touch_focus;
  int pointer_x, pointer_y;
  uint32_t pointer_button_time_stamp;
  uint32_t touch_down_time_stamp;
  uint32_t serial;
  bool grabbed;
  struct wl_list link;
};

static bool own_surface(struct wl_surface *surface)
{
  return true;
}
#endif // USE_SYSTEM_LIBDECOR

#endif // USE_SYSTEM_LIBDECOR || !HAVE_GTK


static unsigned char *gtk_titlebar_buffer(struct libdecor_frame *frame,
                                          int *width, int *height, int *stride)
{
  struct libdecor_frame_gtk *lfg = (struct libdecor_frame_gtk *)frame;
#if USE_SYSTEM_LIBDECOR || !HAVE_GTK
  struct border_component_gtk *bc;
#else
  struct border_component *bc;
#endif
  bc = &lfg->headerbar;
  struct buffer *buffer = bc->buffer;
  *width = buffer->buffer_width;
  *height = buffer->buffer_height;
  *stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, buffer->buffer_width);
  return (unsigned char*)buffer->data;
}


static unsigned char *cairo_titlebar_buffer(struct libdecor_frame *frame,
                                                 int *width, int *height, int *stride)
{
  struct libdecor_frame_cairo *lfc = (struct libdecor_frame_cairo *)frame;
#if USE_SYSTEM_LIBDECOR || HAVE_GTK
  struct border_component_cairo *bc = &lfc->title_bar.title;
#else
  struct border_component *bc = &lfc->title_bar.title;
#endif
  struct buffer *buffer = bc->server.buffer;
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
static const char *get_libdecor_plugin_description(struct libdecor_frame *frame) {
  static const struct libdecor_plugin_description *plugin_description = NULL;
  int X, Y = 0;
  libdecor_frame_translate_coordinate(frame, 0, 0, &X, &Y);
  if (Y == 0) {
    return "Server-Side Decoration";
  }
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
  static const char *my_plugin = NULL;
  if (!my_plugin) my_plugin = get_libdecor_plugin_description(frame);
  if (my_plugin && !strcmp(my_plugin, "GTK3 plugin")) {
    return gtk_titlebar_buffer(frame, width, height, stride);
  }
  else if (my_plugin && !strcmp(my_plugin, "libdecor plugin using Cairo")) {
    return cairo_titlebar_buffer(frame, width, height, stride);
  }
  return NULL;
}


/* === Beginning of code to add support of GTK Shell to libdecor-gtk === */
#if HAVE_GTK

static struct gtk_shell1 *gtk_shell = NULL;
static uint32_t doubleclick_time = 250;

// libdecor's button member of wl_pointer_listener objects
static void (*decor_pointer_button)(void*, struct wl_pointer *,
                                    uint32_t ,
                                    uint32_t ,
                                    uint32_t ,
                                    uint32_t);

// FLTK's replacement for button member of wl_pointer_listener objects
static void fltk_pointer_button(void *data,
                                struct wl_pointer *wl_pointer,
                                uint32_t serial,
                                uint32_t time,
                                uint32_t button,
                                uint32_t state) {
  struct seat *seat = data;
  struct libdecor_frame_gtk *frame_gtk;
  if (!seat->pointer_focus || !own_surface(seat->pointer_focus))
    return;

  frame_gtk = wl_surface_get_user_data(seat->pointer_focus);
  if (!frame_gtk)
    return;

  if (button == BTN_MIDDLE && state == WL_POINTER_BUTTON_STATE_PRESSED) {
    struct gtk_surface1 *gtk_surface = gtk_shell1_get_gtk_surface(gtk_shell,
                                                  frame_gtk->headerbar.wl_surface);
    gtk_surface1_titlebar_gesture(gtk_surface, serial,
                                  seat->wl_seat, GTK_SURFACE1_GESTURE_MIDDLE_CLICK);
    gtk_surface1_release(gtk_surface);
    return;
  } else if (button == BTN_RIGHT && state == WL_POINTER_BUTTON_STATE_PRESSED) {
    struct gtk_surface1 *gtk_surface = gtk_shell1_get_gtk_surface(gtk_shell,
                                                  frame_gtk->headerbar.wl_surface);
    gtk_surface1_titlebar_gesture(gtk_surface, serial,
                                  seat->wl_seat, GTK_SURFACE1_GESTURE_RIGHT_CLICK);
    gtk_surface1_release(gtk_surface);
    return;
  } else if (button == BTN_LEFT && state == WL_POINTER_BUTTON_STATE_PRESSED) {
    if (time - seat->pointer_button_time_stamp < doubleclick_time) {
      seat->pointer_button_time_stamp = 0;
      struct gtk_surface1 *gtk_surface = gtk_shell1_get_gtk_surface(gtk_shell,
                                                    frame_gtk->headerbar.wl_surface);
      gtk_surface1_titlebar_gesture(gtk_surface, serial,
                                    seat->wl_seat, GTK_SURFACE1_GESTURE_DOUBLE_CLICK);
      gtk_surface1_release(gtk_surface);
      return;
    }
  }
  decor_pointer_button(data, wl_pointer, serial, time, button, state);
}

#if USE_SYSTEM_LIBDECOR
static struct border_component_gtk *
get_component_for_surface(struct libdecor_frame_gtk *frame_gtk,
        const struct wl_surface *surface)
{
  if (frame_gtk->shadow.wl_surface == surface)
    return &frame_gtk->shadow;
  if (frame_gtk->headerbar.wl_surface == surface)
    return &frame_gtk->headerbar;
  return NULL;
}
#endif


// libdecor's touch_down member of wl_touch_listener objects
void (*decor_touch_down)(void *data, struct wl_touch *wl_touch, uint32_t serial,
                         uint32_t time, struct wl_surface *surface, int32_t id,
                         wl_fixed_t x, wl_fixed_t y);


// FLTK's replacement for touch_down member of wl_touch_listener objects
static void fltk_touch_down(void *data, struct wl_touch *wl_touch, uint32_t serial,
     uint32_t time, struct wl_surface *surface, int32_t id, wl_fixed_t x, wl_fixed_t y)
{
  struct seat *seat = data;
  struct libdecor_frame_gtk *frame_gtk;
  if (!surface || !own_surface(surface)) return;
  frame_gtk = wl_surface_get_user_data(surface);
  if (!frame_gtk) return;
  seat->touch_focus = surface;
  frame_gtk->touch_active = get_component_for_surface(frame_gtk, surface);
  if (!frame_gtk->touch_active) return;
  
  if (frame_gtk->touch_active->type == HEADER &&
      frame_gtk->hdr_focus.type < HEADER_MIN &&
      time - seat->touch_down_time_stamp <
        (uint32_t)frame_gtk->plugin_gtk->double_click_time_ms
      ) {
    struct gtk_surface1 *gtk_surface = gtk_shell1_get_gtk_surface(
                                            gtk_shell, surface);
    gtk_surface1_titlebar_gesture(gtk_surface, serial, seat->wl_seat,
                                  GTK_SURFACE1_GESTURE_DOUBLE_CLICK);
    gtk_surface1_release(gtk_surface);
  } else {
    decor_touch_down(data, wl_touch, serial, time, surface, id, x, y);
  }
}


struct wl_object { // copied from wayland-private.h
  const struct wl_interface *interface;
  const void *implementation;
  uint32_t id;
};

#endif // HAVE_GTK


// replace libdecor's pointer_button by FLTK's
void use_FLTK_pointer_button(struct libdecor_frame *frame) {
#if HAVE_GTK
  static const char *my_plugin = NULL;
  if (!my_plugin) my_plugin = get_libdecor_plugin_description(frame);
  if (!my_plugin || strcmp(my_plugin, "GTK3 plugin")) return;
  static struct wl_pointer_listener *fltk_listener = NULL;
  if (!gtk_shell || fltk_listener) return;
  struct libdecor_frame_gtk *lfg = (struct libdecor_frame_gtk *)frame;
  if (wl_list_empty(&lfg->plugin_gtk->seat_list)) return;
  struct seat *seat;
  wl_list_for_each(seat, &lfg->plugin_gtk->seat_list, link) {
    break;
  }
  
  struct wl_surface *surface = lfg->headerbar.wl_surface;
  if (surface && !own_surface(surface)) {
    // occurs if libdecor-gtk.so was dynamically loaded via LIBDECOR_PLUGIN_DIR
    gtk_shell = NULL;
    return;
  }
  
  struct wl_object *object = (struct wl_object *)seat->wl_pointer;
  if (!object) return;
  struct wl_pointer_listener *decor_listener =
      (struct wl_pointer_listener*)object->implementation;
  fltk_listener =
      (struct wl_pointer_listener*)malloc(sizeof(struct wl_pointer_listener));
  // initialize FLTK's listener with libdecor's values
  *fltk_listener = *decor_listener;
  // memorize libdecor's button cb
  decor_pointer_button = decor_listener->button;
  // replace libdecor's button by FLTK's
  fltk_listener->button = fltk_pointer_button;
  // replace the pointer listener by a copy whose button member is FLTK's
  object->implementation = fltk_listener;
  
  object = (struct wl_object *)seat->wl_touch;
  if (object) {
    struct wl_touch_listener *decor_touch_listener =
      (struct wl_touch_listener*)object->implementation;
    struct wl_touch_listener *fltk_touch_listener =
      (struct wl_touch_listener*)malloc(sizeof(struct wl_touch_listener));
    // initialize FLTK's touch listener with libdecor's values
    *fltk_touch_listener = *decor_touch_listener;
    // memorize libdecor's down cb
    decor_touch_down = decor_touch_listener->down;
    // replace libdecor's down by FLTK's
    fltk_touch_listener->down = fltk_touch_down;
    // replace the touch listener by a copy whose down member is FLTK's
    object->implementation = fltk_touch_listener;
  }
  
  // get gnome's double_click_time_ms value
  doubleclick_time = lfg->plugin_gtk->double_click_time_ms;
#endif // HAVE_GTK
}


void bind_to_gtk_shell(struct wl_registry *wl_registry, uint32_t id) {
#if HAVE_GTK
  gtk_shell = (struct gtk_shell1*)wl_registry_bind(wl_registry, id,
                                              &gtk_shell1_interface, 5);
#endif // HAVE_GTK
}

/* === End of code to add support of GTK Shell to libdecor-gtk === */
