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

/* Improvements to libdecor.c without modifying libdecor.c itself */
#if ! USE_SYSTEM_LIBDECOR

#include "xdg-shell-client-protocol.h"
#ifdef XDG_TOPLEVEL_STATE_SUSPENDED_SINCE_VERSION
#  define HAVE_XDG_SHELL_V6 1
#endif

#include <dlfcn.h>
#include <stdlib.h>
static void *dlopen_corrected(const char *, int);
#define dlopen(A, B) dlopen_corrected(A, B)
#include "fl_libdecor.h"
#undef libdecor_new
#define libdecor_new libdecor_new_orig
#undef libdecor_frame_set_minimized
#define libdecor_frame_set_minimized libdecor_frame_set_minimized_orig
#include "../src/libdecor.c"
#undef dlopen
#undef libdecor_frame_set_minimized
#undef libdecor_new
#define libdecor_new fl_libdecor_new
#define libdecor_frame_set_minimized fl_libdecor_frame_set_minimized

extern bool fl_libdecor_using_weston(void);
extern const struct libdecor_plugin_description *fl_libdecor_plugin_description;
//#include <stdio.h>

// we have a built-in plugin so don't need a fallback one
struct libdecor_plugin *libdecor_fallback_plugin_new(struct libdecor *context) {
  return NULL;
}

// see get_libdecor_plugin_description() explaining why this is useful
static void *dlopen_corrected(const char *filename, int flags) {
  static int best_priority = -1;
  void *retval = dlopen(filename, flags);
  if (retval) {
    const struct libdecor_plugin_description *description =
        (const struct libdecor_plugin_description*)dlsym(retval, "libdecor_plugin_description");
    if (description && description->priorities->priority > best_priority) {
        fl_libdecor_plugin_description = description;
        best_priority = description->priorities->priority;
    }
  }
  return retval;
}


void fl_libdecor_frame_set_minimized(struct libdecor_frame *frame)
{
  static bool done = false;
  static bool using_weston = false;
  if (!done) {
    typedef bool (*ext_f)(void);
    volatile ext_f ext = fl_libdecor_using_weston;
    done = true;
    if (ext) using_weston = fl_libdecor_using_weston();
//fprintf(stderr, "fl_libdecor_using_weston=%p using_weston=%d\n", fl_libdecor_using_weston, using_weston);
    if (using_weston) { // determine the version of the running Weston compositor
      FILE *pipe = popen("weston --version", "r");
      if (pipe) {
        char line[50], *p;
        int version = 0;
        p = fgets(line, sizeof(line), pipe);
        pclose(pipe);
        if (p) p = strchr(line, ' ');
        if (p) {
          sscanf(p, "%d", &version);
          // Weston version 10 has fixed the bug handled here
          if (version >= 10) using_weston = false;
        }
      }
    }
  }
  if (using_weston) libdecor_frame_set_visibility(frame, false);
  libdecor_frame_set_minimized_orig(frame);
}


/*
 By default, FLTK modifies libdecor's libdecor_new() function to determine the plugin as follows :
 1) the directory pointed by environment variable LIBDECOR_PLUGIN_DIR or, in absence of this variable,
    by -DLIBDECOR_PLUGIN_DIR=xxx at build time is searched for a libdecor plugin;
 2) if this directory does not exist or contains no plugin, the built-in plugin is used.
    * if FLTK was built with package libgtk-3-dev, the GTK plugin is used
    * if FLTK was built without package libgtk-3-dev, the Cairo plugin is used
 
 If FLTK was built with FLTK_USE_SYSTEM_LIBDECOR turned ON, the present modification
 isn't compiled, so the plugin-searching algorithm of libdecor_new() in libdecor-0.so is used.
 This corresponds to step 1) above and to use no titlebar is no plugin is found.
 
 N.B.: only the system package is built with a meaningful value of -DLIBDECOR_PLUGIN_DIR=
 so a plugin may be loaded that way only if FLTK was built with FLTK_USE_SYSTEM_LIBDECOR turned ON.
 
 */
struct libdecor *fl_libdecor_new(struct wl_display *wl_display, const struct libdecor_interface *iface)
{
  struct libdecor *context;
  context = zalloc(sizeof *context);
  context->ref_count = 1;
  context->iface = iface;
  context->wl_display = wl_display;
  context->wl_registry = wl_display_get_registry(wl_display);
  wl_registry_add_listener(context->wl_registry, &registry_listener, context);
  context->init_callback = wl_display_sync(context->wl_display);
  wl_callback_add_listener(context->init_callback, &init_wl_display_callback_listener, context);
  wl_list_init(&context->frames);
  // attempt to dynamically load a libdecor plugin with dlopen()
  if (init_plugins(context) != 0) { // attempt to load plugin by dlopen()
    // no plug-in was found by dlopen(), use built-in plugin instead
    // defined in the source code of the built-in plugin:  libdecor-cairo.c or libdecor-gtk.c
    extern const struct libdecor_plugin_description libdecor_plugin_description;
#if HAVE_GTK
    bool gdk_caution = false;
    if (getenv("GDK_BACKEND") && strcmp(getenv("GDK_BACKEND"), "x11") == 0) {
      // Environment variable GDK_BACKEND=x11 makes the .constructor below fail
      // for the built-in GTK plugin and then FLTK crashes (#1029).
      // Temporarily unset GDK_BACKEND to prevent that.
      gdk_caution = true;
      unsetenv("GDK_BACKEND");
    }
#endif
    context->plugin = libdecor_plugin_description.constructor(context);
#if HAVE_GTK
    if (gdk_caution) putenv("GDK_BACKEND=x11");
#endif
 }

  wl_display_flush(wl_display);
  return context;
}

#endif //! USE_SYSTEM_LIBDECOR
