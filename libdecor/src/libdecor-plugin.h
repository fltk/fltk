/*
 * Copyright © 2017-2018 Red Hat Inc.
 * Copyright © 2018 Jonas Ådahl
 * Copyright © 2019 Christian Rauch
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef LIBDECOR_PLUGIN_H
#define LIBDECOR_PLUGIN_H

#include "libdecor.h"

struct libdecor_frame_private;

struct libdecor_frame {
	struct libdecor_frame_private *priv;
	struct wl_list link;
};

struct libdecor_plugin_private;

struct libdecor_plugin {
	struct libdecor_plugin_private *priv;
};

typedef struct libdecor_plugin * (* libdecor_plugin_constructor)(struct libdecor *context);

#define LIBDECOR_PLUGIN_PRIORITY_HIGH 1000
#define LIBDECOR_PLUGIN_PRIORITY_MEDIUM 100
#define LIBDECOR_PLUGIN_PRIORITY_LOW 0

struct libdecor_plugin_priority {
	const char *desktop;
	int priority;
};

enum libdecor_plugin_capabilities {
	LIBDECOR_PLUGIN_CAPABILITY_BASE = 1 << 0,
};

struct libdecor_plugin_description {
	/* API version the plugin is compatible with. */
	int api_version;

	/* Human readable string describing the plugin. */
	char *description;

	/* A plugin has a bitmask of capabilities. The plugin loader can use this
	 * to load a plugin with the right capabilities. */
	enum libdecor_plugin_capabilities capabilities;

	/*
	 * The priorities field points to a list of per desktop priorities.
	 * properties[i].desktop is matched against XDG_CURRENT_DESKTOP when
	 * determining what plugin to use. The last entry in the list MUST have
	 * the priorities[i].desktop pointer set to NULL as a default
	 * priority.
	 */
	const struct libdecor_plugin_priority *priorities;

	/* Vfunc used for constructing a plugin instance. */
	libdecor_plugin_constructor constructor;

	/* NULL terminated list of incompatible symbols. */
	char *conflicting_symbols[1024];
};

struct libdecor_plugin_interface {
	void (* destroy)(struct libdecor_plugin *plugin);

	int (* get_fd)(struct libdecor_plugin *plugin);
	int (* dispatch)(struct libdecor_plugin *plugin,
			 int timeout);

	struct libdecor_frame * (* frame_new)(struct libdecor_plugin *plugin);
	void (* frame_free)(struct libdecor_plugin *plugin,
			    struct libdecor_frame *frame);
	void (* frame_commit)(struct libdecor_plugin *plugin,
			      struct libdecor_frame *frame,
			      struct libdecor_state *state,
			      struct libdecor_configuration *configuration);
	void (*frame_property_changed)(struct libdecor_plugin *plugin,
				       struct libdecor_frame *frame);
	void (* frame_popup_grab)(struct libdecor_plugin *plugin,
				  struct libdecor_frame *frame,
				  const char *seat_name);
	void (* frame_popup_ungrab)(struct libdecor_plugin *plugin,
				    struct libdecor_frame *frame,
				    const char *seat_name);

	bool (* frame_get_border_size)(struct libdecor_plugin *plugin,
				       struct libdecor_frame *frame,
				       struct libdecor_configuration *configuration,
				       int *left,
				       int *right,
				       int *top,
				       int *bottom);

	/* Reserved */
	void (* reserved0)(void);
	void (* reserved1)(void);
	void (* reserved2)(void);
	void (* reserved3)(void);
	void (* reserved4)(void);
	void (* reserved5)(void);
	void (* reserved6)(void);
	void (* reserved7)(void);
	void (* reserved8)(void);
	void (* reserved9)(void);
};

struct wl_surface *
libdecor_frame_get_wl_surface(struct libdecor_frame *frame);

int
libdecor_frame_get_content_width(struct libdecor_frame *frame);

int
libdecor_frame_get_content_height(struct libdecor_frame *frame);

enum libdecor_window_state
libdecor_frame_get_window_state(struct libdecor_frame *frame);

enum libdecor_capabilities
libdecor_frame_get_capabilities(const struct libdecor_frame *frame);

void
libdecor_frame_dismiss_popup(struct libdecor_frame *frame,
			     const char *seat_name);

void
libdecor_frame_toplevel_commit(struct libdecor_frame *frame);

struct wl_display *
libdecor_get_wl_display(struct libdecor *context);

void
libdecor_notify_plugin_ready(struct libdecor *context);

void
libdecor_notify_plugin_error(struct libdecor *context,
			     enum libdecor_error error,
			     const char *__restrict fmt,
			     ...);

int
libdecor_state_get_content_width(struct libdecor_state *state);

int
libdecor_state_get_content_height(struct libdecor_state *state);

enum libdecor_window_state
libdecor_state_get_window_state(struct libdecor_state *state);

int
libdecor_plugin_init(struct libdecor_plugin *plugin,
		     struct libdecor *context,
		     struct libdecor_plugin_interface *iface);

void
libdecor_plugin_release(struct libdecor_plugin *plugin);

#endif /* LIBDECOR_PLUGIN_H */
