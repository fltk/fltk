//
// Interface with the libdecor library for the Fast Light Tool Kit (FLTK).
//
// Copyright 2022 by Bill Spitzak and others.
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

#if USE_SYSTEM_LIBDECOR
#include "../src/libdecor-plugin.h"
enum zxdg_toplevel_decoration_v1_mode {
  ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE = 1,
  ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE = 2,
};

enum component {NONE};
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

#  include "xdg-decoration-client-protocol.h"

const struct libdecor_plugin_description *fl_libdecor_plugin_description = NULL;

#  ifdef HAVE_GTK
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
enum header_element { HDR_NONE };
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

#endif // USE_SYSTEM_LIBDECOR || !HAVE_GTK

/* these definitions are copied from libdecor/src/libdecor.c */

struct libdecor_limits {
  int min_width;
  int min_height;
  int max_width;
  int max_height;
};

struct libdecor_frame_private {
  int ref_count;
  struct libdecor *context;
  struct wl_surface *wl_surface;
  struct libdecor_frame_interface *iface;
  void *user_data;
  struct xdg_surface *xdg_surface;
  struct xdg_toplevel *xdg_toplevel;
  struct zxdg_toplevel_decoration_v1 *toplevel_decoration;
  bool pending_map;
  struct {
    char *app_id;
    char *title;
    struct libdecor_limits content_limits;
    struct xdg_toplevel *parent;
  } state;
  struct libdecor_configuration *pending_configuration;
  int content_width;
  int content_height;
  enum libdecor_window_state window_state;
  enum zxdg_toplevel_decoration_v1_mode decoration_mode;
  enum libdecor_capabilities capabilities;
  struct libdecor_limits interactive_limits;
  bool visible;
};


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
 KDE has its own size limit, similar to that of GDK plugin
 */
static const char *get_libdecor_plugin_description(struct libdecor_frame *frame) {
  static const struct libdecor_plugin_description *plugin_description = NULL;
  if (frame->priv->decoration_mode == ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE) {
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
