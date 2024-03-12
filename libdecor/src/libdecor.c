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

#include "config.h"

#include <errno.h>
#include <dirent.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "libdecor.h"
#include "libdecor-fallback.h"
#include "libdecor-plugin.h"
#include "utils.h"

#include "xdg-shell-client-protocol.h"
#include "xdg-decoration-client-protocol.h"

#ifdef HAVE_XDG_SHELL_V6
#define XDG_WM_BASE_VER 6
#else
#define XDG_WM_BASE_VER 2
#endif

struct libdecor {
	int ref_count;

	const struct libdecor_interface *iface;
	void *user_data;

	struct libdecor_plugin *plugin;
	bool plugin_ready;

	struct wl_display *wl_display;
	struct wl_registry *wl_registry;
	struct xdg_wm_base *xdg_wm_base;
	struct zxdg_decoration_manager_v1 *decoration_manager;

	struct wl_callback *init_callback;
	bool init_done;
	bool has_error;

	struct wl_list frames;
};

struct libdecor_state {
	enum libdecor_window_state window_state;

	int content_width;
	int content_height;
};

struct libdecor_limits {
	int min_width;
	int min_height;
	int max_width;
	int max_height;
};

struct libdecor_configuration {
	uint32_t serial;

	bool has_window_state;
	enum libdecor_window_state window_state;

	bool has_size;
	int window_width;
	int window_height;
};

struct libdecor_frame_private {
	int ref_count;

	struct libdecor *context;

	struct wl_surface *wl_surface;

	const struct libdecor_frame_interface *iface;
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

	enum libdecor_wm_capabilities wm_capabilities;

	/* original limits for interactive resize */
	struct libdecor_limits interactive_limits;

	bool visible;
};

struct libdecor_plugin_private {
	struct libdecor_plugin_interface *iface;
};

/* gather all states at which a window is non-floating */
static const enum libdecor_window_state states_non_floating =
	LIBDECOR_WINDOW_STATE_MAXIMIZED | LIBDECOR_WINDOW_STATE_FULLSCREEN |
	LIBDECOR_WINDOW_STATE_TILED_LEFT | LIBDECOR_WINDOW_STATE_TILED_RIGHT |
	LIBDECOR_WINDOW_STATE_TILED_TOP | LIBDECOR_WINDOW_STATE_TILED_BOTTOM;

static bool
streql(const char *str1, const char *str2)
{
	return (str1 && str2) && (strcmp(str1, str2) == 0);
}


static void
do_map(struct libdecor_frame *frame);

static bool
state_is_floating(enum libdecor_window_state window_state)
{
	return !(window_state & states_non_floating);
}

static void
constrain_content_size(const struct libdecor_frame *frame,
		       int *width,
		       int *height)
{
	const struct libdecor_limits lim = frame->priv->state.content_limits;

	if (lim.min_width > 0)
		*width = MAX(lim.min_width, *width);
	if (lim.max_width > 0)
		*width = MIN(*width, lim.max_width);

	if (lim.min_height > 0)
		*height = MAX(lim.min_height, *height);
	if (lim.max_height > 0)
		*height = MIN(*height, lim.max_height);
}

static bool
frame_has_visible_client_side_decoration(struct libdecor_frame *frame)
{
	/* visibility by client configuration */
	const bool vis_client = frame->priv->visible;
	/* visibility by compositor configuration */
	const bool vis_server = (frame->priv->decoration_mode ==
				 ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE);

	return vis_client && vis_server;
}

LIBDECOR_EXPORT int
libdecor_state_get_content_width(struct libdecor_state *state)
{
	return state->content_width;
}

LIBDECOR_EXPORT int
libdecor_state_get_content_height(struct libdecor_state *state)
{
	return state->content_height;
}

LIBDECOR_EXPORT enum libdecor_window_state
libdecor_state_get_window_state(struct libdecor_state *state)
{
	return state->window_state;
}

LIBDECOR_EXPORT struct libdecor_state *
libdecor_state_new(int width,
		   int height)
{
	struct libdecor_state *state;

	state = zalloc(sizeof *state);
	state->content_width = width;
	state->content_height = height;

	return state;
}

LIBDECOR_EXPORT void
libdecor_state_free(struct libdecor_state *state)
{
	free(state);
}

static struct libdecor_configuration *
libdecor_configuration_new(void)
{
	struct libdecor_configuration *configuration;

	configuration = zalloc(sizeof *configuration);

	return configuration;
}

static void
libdecor_configuration_free(struct libdecor_configuration *configuration)
{
	free(configuration);
}

static bool
frame_get_window_size_for(struct libdecor_frame *frame,
			  struct libdecor_state *state,
			  int *window_width,
			  int *window_height)
{
	struct libdecor_frame_private *frame_priv = frame->priv;
	struct libdecor *context = frame_priv->context;
	struct libdecor_plugin *plugin = context->plugin;

	*window_width = state->content_width;
	*window_height = state->content_height;

	if (frame_has_visible_client_side_decoration(frame) &&
	    plugin->priv->iface->frame_get_border_size) {
		int left, right, top, bottom;
		if (!plugin->priv->iface->frame_get_border_size(
			plugin, frame, NULL, &left, &right, &top, &bottom))
			return false;
		*window_width += left + right;
		*window_height += top + bottom;
	}

	return true;
}

static void
frame_set_window_geometry(struct libdecor_frame *frame,
			  int32_t content_width, int32_t content_height)
{
	struct libdecor_plugin *plugin = frame->priv->context->plugin;
	int x, y, width, height;
	int left, right, top, bottom;

	plugin->priv->iface->frame_get_border_size(plugin, frame, NULL,
						   &left, &right, &top, &bottom);
	x = -left;
	y = -top;
	width = content_width + left + right;
	height = content_height + top + bottom;
	xdg_surface_set_window_geometry(frame->priv->xdg_surface, x, y, width, height);
}

LIBDECOR_EXPORT bool
libdecor_configuration_get_content_size(struct libdecor_configuration *configuration,
					struct libdecor_frame *frame,
					int *width,
					int *height)
{
	struct libdecor_plugin *plugin = frame->priv->context->plugin;

	/* get configured toplevel dimensions */
	if (!configuration->has_size)
		return false;

	if (configuration->window_width == 0 || configuration->window_height == 0)
		return false;

	*width = configuration->window_width;
	*height = configuration->window_height;

	/* remove plugin-specific border size */
	if (frame_has_visible_client_side_decoration(frame) &&
	    plugin->priv->iface->frame_get_border_size) {
		int left, right, top, bottom;

		/* Update window state for correct border size calculation */
		frame->priv->window_state = configuration->window_state;
		if (!plugin->priv->iface->frame_get_border_size(
			plugin, frame, configuration, &left, &right, &top, &bottom))
			return false;

		*width -= (left + right);
		*height -= (top + bottom);
	}

	/* constrain content dimensions manually */
	if (state_is_floating(configuration->window_state)) {
		constrain_content_size(frame, width, height);
	}

	return true;
}

LIBDECOR_EXPORT bool
libdecor_configuration_get_window_state(struct libdecor_configuration *configuration,
					enum libdecor_window_state *window_state)
{
	if (!configuration->has_window_state)
		return false;

	*window_state = configuration->window_state;
	return true;
}

static void
xdg_surface_configure(void *user_data,
		      struct xdg_surface *xdg_surface,
		      uint32_t serial)
{
	struct libdecor_frame *frame = user_data;
	struct libdecor_frame_private *frame_priv = frame->priv;
	struct libdecor_configuration *configuration;

	configuration = frame_priv->pending_configuration;
	frame_priv->pending_configuration = NULL;

	if (!configuration)
		configuration = libdecor_configuration_new();

	configuration->serial = serial;

	frame_priv->iface->configure(frame,
				     configuration,
				     frame_priv->user_data);

	libdecor_configuration_free(configuration);
}

static const struct xdg_surface_listener xdg_surface_listener = {
	xdg_surface_configure,
};

static enum libdecor_window_state
parse_states(struct wl_array *states)
{
	enum libdecor_window_state pending_state = LIBDECOR_WINDOW_STATE_NONE;
	uint32_t *p;

	wl_array_for_each(p, states) {
		enum xdg_toplevel_state state = *p;

		switch (state) {
		case XDG_TOPLEVEL_STATE_FULLSCREEN:
			pending_state |= LIBDECOR_WINDOW_STATE_FULLSCREEN;
			break;
		case XDG_TOPLEVEL_STATE_MAXIMIZED:
			pending_state |= LIBDECOR_WINDOW_STATE_MAXIMIZED;
			break;
		case XDG_TOPLEVEL_STATE_ACTIVATED:
			pending_state |= LIBDECOR_WINDOW_STATE_ACTIVE;
			break;
		case XDG_TOPLEVEL_STATE_TILED_LEFT:
			pending_state |= LIBDECOR_WINDOW_STATE_TILED_LEFT;
			break;
		case XDG_TOPLEVEL_STATE_TILED_RIGHT:
			pending_state |= LIBDECOR_WINDOW_STATE_TILED_RIGHT;
			break;
		case XDG_TOPLEVEL_STATE_TILED_TOP:
			pending_state |= LIBDECOR_WINDOW_STATE_TILED_TOP;
			break;
		case XDG_TOPLEVEL_STATE_TILED_BOTTOM:
			pending_state |= LIBDECOR_WINDOW_STATE_TILED_BOTTOM;
			break;
		case XDG_TOPLEVEL_STATE_RESIZING:
			pending_state |= LIBDECOR_WINDOW_STATE_RESIZING;
			break;
#ifdef HAVE_XDG_SHELL_V6
		case XDG_TOPLEVEL_STATE_SUSPENDED:
			pending_state |= LIBDECOR_WINDOW_STATE_SUSPENDED;
			break;
#endif
		default:
			break;
		}
	}

	return pending_state;
}

static void
xdg_toplevel_configure(void *user_data,
		       struct xdg_toplevel *xdg_toplevel,
		       int32_t width,
		       int32_t height,
		       struct wl_array *states)
{
	struct libdecor_frame *frame = user_data;
	struct libdecor_frame_private *frame_priv = frame->priv;
	enum libdecor_window_state window_state;

	window_state = parse_states(states);

	frame_priv->pending_configuration = libdecor_configuration_new();

	frame_priv->pending_configuration->has_size = true;
	frame_priv->pending_configuration->window_width = width;
	frame_priv->pending_configuration->window_height = height;

	frame_priv->pending_configuration->has_window_state = true;
	frame_priv->pending_configuration->window_state = window_state;
}

static void
xdg_toplevel_close(void *user_data,
		   struct xdg_toplevel *xdg_toplevel)
{
	struct libdecor_frame *frame = user_data;
	struct libdecor_frame_private *frame_priv = frame->priv;

	frame_priv->iface->close(frame, frame_priv->user_data);
}

#ifdef HAVE_XDG_SHELL_V6
static void
xdg_toplevel_configure_bounds(void *data,
			      struct xdg_toplevel *xdg_toplevel,
			      int32_t width,
			      int32_t height)
{
}

static void
xdg_toplevel_wm_capabilities(void *user_data,
			     struct xdg_toplevel *xdg_toplevel,
			     struct wl_array *capabilities)
{
	struct libdecor_frame *frame = user_data;
	struct libdecor_frame_private *frame_priv = frame->priv;
	enum xdg_toplevel_wm_capabilities *wm_cap;

	frame_priv->wm_capabilities = 0;

	wl_array_for_each(wm_cap, capabilities) {
		switch (*wm_cap) {
		case XDG_TOPLEVEL_WM_CAPABILITIES_WINDOW_MENU:
			frame_priv->wm_capabilities |= LIBDECOR_WM_CAPABILITIES_WINDOW_MENU;
			break;
		case XDG_TOPLEVEL_WM_CAPABILITIES_MAXIMIZE:
			frame_priv->wm_capabilities |= LIBDECOR_WM_CAPABILITIES_MAXIMIZE;
			break;
		case XDG_TOPLEVEL_WM_CAPABILITIES_FULLSCREEN:
			frame_priv->wm_capabilities |= LIBDECOR_WM_CAPABILITIES_FULLSCREEN;
			break;
		case XDG_TOPLEVEL_WM_CAPABILITIES_MINIMIZE:
			frame_priv->wm_capabilities |= LIBDECOR_WM_CAPABILITIES_MINIMIZE;
			break;
		default:
			break;
		}
	}
}
#endif

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
	xdg_toplevel_configure,
	xdg_toplevel_close,
#ifdef HAVE_XDG_SHELL_V6
	xdg_toplevel_configure_bounds,
	xdg_toplevel_wm_capabilities,
#endif
};

static void
toplevel_decoration_configure(
		void *data,
		struct zxdg_toplevel_decoration_v1 *zxdg_toplevel_decoration_v1,
		uint32_t mode)
{
	((struct libdecor_frame_private *)(data))->decoration_mode = mode;
}

static const struct zxdg_toplevel_decoration_v1_listener
		xdg_toplevel_decoration_listener = {
	toplevel_decoration_configure,
};

void
libdecor_frame_create_xdg_decoration(struct libdecor_frame_private *frame_priv)
{
	if (!frame_priv->context->decoration_manager)
		return;

	frame_priv->toplevel_decoration =
		zxdg_decoration_manager_v1_get_toplevel_decoration(
				frame_priv->context->decoration_manager,
				frame_priv->xdg_toplevel);

	zxdg_toplevel_decoration_v1_add_listener(
				frame_priv->toplevel_decoration,
				&xdg_toplevel_decoration_listener,
				frame_priv);
}

static void
init_shell_surface(struct libdecor_frame *frame)
{
	struct libdecor_frame_private *frame_priv = frame->priv;
	struct libdecor *context = frame_priv->context;

	if (frame_priv->xdg_surface)
		return;

	frame_priv->xdg_surface =
		xdg_wm_base_get_xdg_surface(context->xdg_wm_base,
					    frame_priv->wl_surface);
	xdg_surface_add_listener(frame_priv->xdg_surface,
				 &xdg_surface_listener,
				 frame);

	frame_priv->xdg_toplevel =
		xdg_surface_get_toplevel(frame_priv->xdg_surface);
	xdg_toplevel_add_listener(frame_priv->xdg_toplevel,
				  &xdg_toplevel_listener,
				  frame);

	frame_priv->decoration_mode =
			ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE;
	frame_priv->toplevel_decoration = NULL;
	libdecor_frame_create_xdg_decoration(frame_priv);

	if (frame_priv->state.parent) {
		xdg_toplevel_set_parent(frame_priv->xdg_toplevel,
					frame_priv->state.parent);
	}
	if (frame_priv->state.title) {
		xdg_toplevel_set_title(frame_priv->xdg_toplevel,
				       frame_priv->state.title);
	}
	if (frame_priv->state.app_id) {
		xdg_toplevel_set_app_id(frame_priv->xdg_toplevel,
					frame_priv->state.app_id);
	}

	if (frame_priv->pending_map)
		do_map(frame);
}

LIBDECOR_EXPORT struct libdecor_frame *
libdecor_decorate(struct libdecor *context,
		  struct wl_surface *wl_surface,
		  const struct libdecor_frame_interface *iface,
		  void *user_data)
{
	struct libdecor_plugin *plugin = context->plugin;
	struct libdecor_frame *frame;
	struct libdecor_frame_private *frame_priv;

	if (context->has_error)
		return NULL;

	frame = plugin->priv->iface->frame_new(plugin);
	if (!frame)
		return NULL;

	frame_priv = zalloc(sizeof *frame_priv);
	frame->priv = frame_priv;

	frame_priv->ref_count = 1;
	frame_priv->context = context;

	frame_priv->wl_surface = wl_surface;
	frame_priv->iface = iface;
	frame_priv->user_data = user_data;
	frame_priv->wm_capabilities = LIBDECOR_WM_CAPABILITIES_WINDOW_MENU |
				      LIBDECOR_WM_CAPABILITIES_MAXIMIZE |
				      LIBDECOR_WM_CAPABILITIES_FULLSCREEN |
				      LIBDECOR_WM_CAPABILITIES_MINIMIZE;

	wl_list_insert(&context->frames, &frame->link);

	libdecor_frame_set_capabilities(frame,
					LIBDECOR_ACTION_MOVE |
					LIBDECOR_ACTION_RESIZE |
					LIBDECOR_ACTION_MINIMIZE |
					LIBDECOR_ACTION_FULLSCREEN |
					LIBDECOR_ACTION_CLOSE);

	frame_priv->visible = true;

	if (context->init_done)
		init_shell_surface(frame);

	return frame;
}

LIBDECOR_EXPORT void
libdecor_frame_ref(struct libdecor_frame *frame)
{
	struct libdecor_frame_private *frame_priv = frame->priv;

	frame_priv->ref_count++;
}

LIBDECOR_EXPORT void
libdecor_frame_unref(struct libdecor_frame *frame)
{
	struct libdecor_frame_private *frame_priv = frame->priv;

	frame_priv->ref_count--;
	if (frame_priv->ref_count == 0) {
		struct libdecor *context = frame_priv->context;
		struct libdecor_plugin *plugin = context->plugin;

		if (context->decoration_manager && frame_priv->toplevel_decoration) {
        		zxdg_toplevel_decoration_v1_destroy(frame_priv->toplevel_decoration);
        		frame_priv->toplevel_decoration = NULL;
        	}

		wl_list_remove(&frame->link);

		if (frame_priv->xdg_toplevel)
			xdg_toplevel_destroy(frame_priv->xdg_toplevel);
		if (frame_priv->xdg_surface)
			xdg_surface_destroy(frame_priv->xdg_surface);

		plugin->priv->iface->frame_free(plugin, frame);

		free(frame_priv->state.title);
		free(frame_priv->state.app_id);

		free(frame_priv);

		free(frame);
	}
}

LIBDECOR_EXPORT void *
libdecor_frame_get_user_data(struct libdecor_frame *frame)
{
	return frame->priv->user_data;
}

LIBDECOR_EXPORT void
libdecor_frame_set_user_data(struct libdecor_frame *frame, void *user_data)
{
	frame->priv->user_data = user_data;
}

LIBDECOR_EXPORT void
libdecor_frame_set_visibility(struct libdecor_frame *frame,
			      bool visible)
{
	struct libdecor_frame_private *frame_priv = frame->priv;
	struct libdecor *context = frame_priv->context;
	struct libdecor_plugin *plugin = context->plugin;

	frame_priv->visible = visible;

	/* enable/disable decorations that are managed by the compositor,
	 * only xdg-decoration version 2 and above allows to toggle decoration */
	if (context->decoration_manager &&
	    zxdg_decoration_manager_v1_get_version(context->decoration_manager) > 1) {
		if (frame_priv->visible &&
		    frame_priv->toplevel_decoration == NULL) {
			/* - request to SHOW decorations
			 * - decorations are NOT HANDLED
			 * => create new decorations for already mapped surface */
			libdecor_frame_create_xdg_decoration(frame_priv);
		} else if (!frame_priv->visible &&
			 frame_priv->toplevel_decoration != NULL) {
			/* - request to HIDE decorations
			 * - decorations are HANDLED
			 * => destroy decorations */
			zxdg_toplevel_decoration_v1_destroy(frame_priv->toplevel_decoration);
			frame_priv->toplevel_decoration = NULL;
		}
	}

	/* enable/disable decorations that are managed by a plugin */
	if (frame_has_visible_client_side_decoration(frame)) {
		/* show client-side decorations */
		plugin->priv->iface->frame_commit(plugin, frame, NULL, NULL);
	} else {
		/* destroy client-side decorations */
		plugin->priv->iface->frame_free(plugin, frame);
	}

	frame_set_window_geometry(frame,
				  frame_priv->content_width,
				  frame_priv->content_height);

	libdecor_frame_toplevel_commit(frame);
}

LIBDECOR_EXPORT bool
libdecor_frame_is_visible(struct libdecor_frame *frame)
{
	return frame->priv->visible;
}

LIBDECOR_EXPORT void
libdecor_frame_set_parent(struct libdecor_frame *frame,
			  struct libdecor_frame *parent)
{
	struct libdecor_frame_private *frame_priv = frame->priv;

	if (!frame_priv->xdg_toplevel)
		return;

	frame_priv->state.parent = parent->priv->xdg_toplevel;

	xdg_toplevel_set_parent(frame_priv->xdg_toplevel,
				parent->priv->xdg_toplevel);
}

LIBDECOR_EXPORT void
libdecor_frame_set_title(struct libdecor_frame *frame,
			 const char *title)
{
	struct libdecor_frame_private *frame_priv = frame->priv;
	struct libdecor_plugin *plugin = frame_priv->context->plugin;

	if (!streql(frame_priv->state.title, title)) {
		free(frame_priv->state.title);
		frame_priv->state.title = strdup(title);

		if (!frame_priv->xdg_toplevel)
			return;

		xdg_toplevel_set_title(frame_priv->xdg_toplevel, title);

		plugin->priv->iface->frame_property_changed(plugin, frame);
	}
}

LIBDECOR_EXPORT const char *
libdecor_frame_get_title(struct libdecor_frame *frame)
{
	return frame->priv->state.title;
}

LIBDECOR_EXPORT void
libdecor_frame_set_app_id(struct libdecor_frame *frame,
			  const char *app_id)
{
	struct libdecor_frame_private *frame_priv = frame->priv;

	free(frame_priv->state.app_id);
	frame_priv->state.app_id = strdup(app_id);

	if (!frame_priv->xdg_toplevel)
		return;

	xdg_toplevel_set_app_id(frame_priv->xdg_toplevel, app_id);
}

static void
notify_on_capability_change(struct libdecor_frame *frame,
			    const enum libdecor_capabilities old_capabilities)
{
	struct libdecor_plugin *plugin = frame->priv->context->plugin;
	struct libdecor_state *state;

	if (frame->priv->capabilities == old_capabilities)
		return;

	if (frame->priv->content_width == 0 ||
	    frame->priv->content_height == 0)
		return;

	plugin->priv->iface->frame_property_changed(plugin, frame);

	if (!libdecor_frame_has_capability(frame, LIBDECOR_ACTION_RESIZE)) {
		frame->priv->interactive_limits = frame->priv->state.content_limits;
		/* set fixed window size */
		libdecor_frame_set_min_content_size(frame,
						    frame->priv->content_width,
						    frame->priv->content_height);
		libdecor_frame_set_max_content_size(frame,
						    frame->priv->content_width,
						    frame->priv->content_height);
	} else {
		/* restore old limits */
		frame->priv->state.content_limits = frame->priv->interactive_limits;
	}

	state = libdecor_state_new(frame->priv->content_width,
				   frame->priv->content_height);
	libdecor_frame_commit(frame, state, NULL);
	libdecor_state_free(state);

	libdecor_frame_toplevel_commit(frame);
}

LIBDECOR_EXPORT void
libdecor_frame_set_capabilities(struct libdecor_frame *frame,
				enum libdecor_capabilities capabilities)
{
	const enum libdecor_capabilities old_capabilities =
			frame->priv->capabilities;

	frame->priv->capabilities |= capabilities;

	notify_on_capability_change(frame, old_capabilities);
}

LIBDECOR_EXPORT void
libdecor_frame_unset_capabilities(struct libdecor_frame *frame,
				  enum libdecor_capabilities capabilities)
{
	const enum libdecor_capabilities old_capabilities =
			frame->priv->capabilities;

	frame->priv->capabilities &= ~capabilities;

	notify_on_capability_change(frame, old_capabilities);
}

LIBDECOR_EXPORT bool
libdecor_frame_has_capability(struct libdecor_frame *frame,
			      enum libdecor_capabilities capability)
{
	return frame->priv->capabilities & capability;
}

LIBDECOR_EXPORT void
libdecor_frame_popup_grab(struct libdecor_frame *frame,
			  const char *seat_name)
{
	struct libdecor_frame_private *frame_priv = frame->priv;
	struct libdecor *context = frame_priv->context;
	struct libdecor_plugin *plugin = context->plugin;

	plugin->priv->iface->frame_popup_grab(plugin, frame, seat_name);
}

LIBDECOR_EXPORT void
libdecor_frame_popup_ungrab(struct libdecor_frame *frame,
			    const char *seat_name)
{
	struct libdecor_frame_private *frame_priv = frame->priv;
	struct libdecor *context = frame_priv->context;
	struct libdecor_plugin *plugin = context->plugin;

	plugin->priv->iface->frame_popup_ungrab(plugin, frame, seat_name);
}

LIBDECOR_EXPORT void
libdecor_frame_dismiss_popup(struct libdecor_frame *frame,
			     const char *seat_name)
{
	struct libdecor_frame_private *frame_priv = frame->priv;

	frame_priv->iface->dismiss_popup(frame, seat_name, frame_priv->user_data);
}

LIBDECOR_EXPORT void
libdecor_frame_show_window_menu(struct libdecor_frame *frame,
				struct wl_seat *wl_seat,
				uint32_t serial,
				int x,
				int y)
{
	struct libdecor_frame_private *frame_priv = frame->priv;

	if (!frame_priv->xdg_toplevel) {
		fprintf(stderr, "Can't show window menu before being mapped\n");
		return;
	}

	xdg_toplevel_show_window_menu(frame_priv->xdg_toplevel,
				      wl_seat, serial,
				      x, y);
}

LIBDECOR_EXPORT void
libdecor_frame_translate_coordinate(struct libdecor_frame *frame,
				    int content_x,
				    int content_y,
				    int *frame_x,
				    int *frame_y)
{
	struct libdecor_frame_private *frame_priv = frame->priv;
	struct libdecor *context = frame_priv->context;
	struct libdecor_plugin *plugin = context->plugin;

	*frame_x = content_x;
	*frame_y = content_y;

	if (frame_has_visible_client_side_decoration(frame) &&
	    plugin->priv->iface->frame_get_border_size) {
		int left, top;
		plugin->priv->iface->frame_get_border_size(plugin, frame, NULL,
							   &left, NULL, &top, NULL);
		*frame_x += left;
		*frame_y += top;
	}
}

LIBDECOR_EXPORT void
libdecor_frame_set_min_content_size(struct libdecor_frame *frame,
				    int content_width,
				    int content_height)
{
	struct libdecor_frame_private *frame_priv = frame->priv;

	frame_priv->state.content_limits.min_width = content_width;
	frame_priv->state.content_limits.min_height = content_height;
}

LIBDECOR_EXPORT void
libdecor_frame_set_max_content_size(struct libdecor_frame *frame,
				    int content_width,
				    int content_height)
{
	struct libdecor_frame_private *frame_priv = frame->priv;

	frame_priv->state.content_limits.max_width = content_width;
	frame_priv->state.content_limits.max_height = content_height;
}

LIBDECOR_EXPORT void
libdecor_frame_get_min_content_size(const struct libdecor_frame *frame,
				    int *content_width,
				    int *content_height)
{
	struct libdecor_frame_private *frame_priv = frame->priv;

	*content_width = frame_priv->state.content_limits.min_width;
	*content_height = frame_priv->state.content_limits.min_height;
}

LIBDECOR_EXPORT void
libdecor_frame_get_max_content_size(const struct libdecor_frame *frame,
				    int *content_width,
				    int *content_height)
{
	struct libdecor_frame_private *frame_priv = frame->priv;

	*content_width = frame_priv->state.content_limits.max_width;
	*content_height = frame_priv->state.content_limits.max_height;
}

LIBDECOR_EXPORT enum libdecor_capabilities
libdecor_frame_get_capabilities(const struct libdecor_frame *frame)
{
	return frame->priv->capabilities;
}

enum xdg_toplevel_resize_edge
edge_to_xdg_edge(enum libdecor_resize_edge edge)
{
	switch (edge) {
	case LIBDECOR_RESIZE_EDGE_NONE:
		return XDG_TOPLEVEL_RESIZE_EDGE_NONE;
	case LIBDECOR_RESIZE_EDGE_TOP:
		return XDG_TOPLEVEL_RESIZE_EDGE_TOP;
	case LIBDECOR_RESIZE_EDGE_BOTTOM:
		return XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM;
	case LIBDECOR_RESIZE_EDGE_LEFT:
		return XDG_TOPLEVEL_RESIZE_EDGE_LEFT;
	case LIBDECOR_RESIZE_EDGE_TOP_LEFT:
		return XDG_TOPLEVEL_RESIZE_EDGE_TOP_LEFT;
	case LIBDECOR_RESIZE_EDGE_BOTTOM_LEFT:
		return XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_LEFT;
	case LIBDECOR_RESIZE_EDGE_RIGHT:
		return XDG_TOPLEVEL_RESIZE_EDGE_RIGHT;
	case LIBDECOR_RESIZE_EDGE_TOP_RIGHT:
		return XDG_TOPLEVEL_RESIZE_EDGE_TOP_RIGHT;
	case LIBDECOR_RESIZE_EDGE_BOTTOM_RIGHT:
		return XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_RIGHT;
	}

	abort();
}

LIBDECOR_EXPORT void
libdecor_frame_resize(struct libdecor_frame *frame,
		      struct wl_seat *wl_seat,
		      uint32_t serial,
		      enum libdecor_resize_edge edge)
{
	struct libdecor_frame_private *frame_priv = frame->priv;
	enum xdg_toplevel_resize_edge xdg_edge;

	xdg_edge = edge_to_xdg_edge(edge);
	xdg_toplevel_resize(frame_priv->xdg_toplevel,
			    wl_seat, serial, xdg_edge);
}

LIBDECOR_EXPORT void
libdecor_frame_move(struct libdecor_frame *frame,
		    struct wl_seat *wl_seat,
		    uint32_t serial)
{
	struct libdecor_frame_private *frame_priv = frame->priv;

	xdg_toplevel_move(frame_priv->xdg_toplevel, wl_seat, serial);
}

LIBDECOR_EXPORT void
libdecor_frame_set_minimized(struct libdecor_frame *frame)
{
	xdg_toplevel_set_minimized(frame->priv->xdg_toplevel);
}

LIBDECOR_EXPORT void
libdecor_frame_set_maximized(struct libdecor_frame *frame)
{
	xdg_toplevel_set_maximized(frame->priv->xdg_toplevel);
}

LIBDECOR_EXPORT void
libdecor_frame_unset_maximized(struct libdecor_frame *frame)
{
	xdg_toplevel_unset_maximized(frame->priv->xdg_toplevel);
}

LIBDECOR_EXPORT void
libdecor_frame_set_fullscreen(struct libdecor_frame *frame,
			      struct wl_output *output)
{
	xdg_toplevel_set_fullscreen(frame->priv->xdg_toplevel, output);
}

LIBDECOR_EXPORT void
libdecor_frame_unset_fullscreen(struct libdecor_frame *frame)
{
	xdg_toplevel_unset_fullscreen(frame->priv->xdg_toplevel);
}

LIBDECOR_EXPORT bool
libdecor_frame_is_floating(struct libdecor_frame *frame)
{
	return state_is_floating(frame->priv->window_state);
}

LIBDECOR_EXPORT void
libdecor_frame_close(struct libdecor_frame *frame)
{
	xdg_toplevel_close(frame, frame->priv->xdg_toplevel);
}

bool
valid_limits(struct libdecor_frame_private *frame_priv)
{
	if (frame_priv->state.content_limits.min_width > 0 &&
	    frame_priv->state.content_limits.max_width > 0 &&
	    frame_priv->state.content_limits.min_width >
	    frame_priv->state.content_limits.max_width)
		return false;

	if (frame_priv->state.content_limits.min_height > 0 &&
	    frame_priv->state.content_limits.max_height > 0 &&
	    frame_priv->state.content_limits.min_height >
	    frame_priv->state.content_limits.max_height)
		return false;

	return true;
}

static void
libdecor_frame_apply_limits(struct libdecor_frame *frame,
			    enum libdecor_window_state window_state)
{
	struct libdecor_frame_private *frame_priv = frame->priv;

	if (!valid_limits(frame_priv)) {
		libdecor_notify_plugin_error(
			frame_priv->context,
			LIBDECOR_ERROR_INVALID_FRAME_CONFIGURATION,
			"minimum size (%i,%i) must be smaller than maximum size (%i,%i)",
			frame_priv->state.content_limits.min_width,
			frame_priv->state.content_limits.min_height,
			frame_priv->state.content_limits.max_width,
			frame_priv->state.content_limits.max_height);
	}

	/* If the frame is configured as non-resizable before the first
	 * configure event is received, we have to manually set the min/max
	 * limits with the configured content size afterwards. */
	if (!libdecor_frame_has_capability(frame, LIBDECOR_ACTION_RESIZE)) {
		frame_priv->state.content_limits.min_width =
				frame_priv->content_width;
		frame_priv->state.content_limits.max_width =
				frame_priv->content_width;

		frame_priv->state.content_limits.min_height =
				frame_priv->content_height;
		frame_priv->state.content_limits.max_height =
				frame_priv->content_height;
	}

	if (frame_priv->state.content_limits.min_width > 0 &&
	    frame_priv->state.content_limits.min_height > 0) {
		struct libdecor_state state_min;
		int win_min_width, win_min_height;

		state_min.content_width = frame_priv->state.content_limits.min_width;
		state_min.content_height = frame_priv->state.content_limits.min_height;
		state_min.window_state = window_state;

		frame_get_window_size_for(frame, &state_min,
					  &win_min_width, &win_min_height);
		xdg_toplevel_set_min_size(frame_priv->xdg_toplevel,
					  win_min_width, win_min_height);
	} else {
		xdg_toplevel_set_min_size(frame_priv->xdg_toplevel, 0, 0);
	}

	if (frame_priv->state.content_limits.max_width > 0 &&
	    frame_priv->state.content_limits.max_height > 0) {
		struct libdecor_state state_max;
		int win_max_width, win_max_height;

		state_max.content_width = frame_priv->state.content_limits.max_width;
		state_max.content_height = frame_priv->state.content_limits.max_height;
		state_max.window_state = window_state;

		frame_get_window_size_for(frame, &state_max,
					  &win_max_width, &win_max_height);
		xdg_toplevel_set_max_size(frame_priv->xdg_toplevel,
					  win_max_width, win_max_height);
	} else {
		xdg_toplevel_set_max_size(frame_priv->xdg_toplevel, 0, 0);
	}
}

static void
libdecor_frame_apply_state(struct libdecor_frame *frame,
			   struct libdecor_state *state)
{
	struct libdecor_frame_private *frame_priv = frame->priv;

	frame_priv->content_width = state->content_width;
	frame_priv->content_height = state->content_height;

	/* do not set limits in non-floating states */
	if (state_is_floating(state->window_state)) {
		libdecor_frame_apply_limits(frame, state->window_state);
	}
}

LIBDECOR_EXPORT void
libdecor_frame_toplevel_commit(struct libdecor_frame *frame)
{
	struct libdecor_frame_private *frame_priv = frame->priv;

	frame_priv->iface->commit(frame, frame_priv->user_data);
}

LIBDECOR_EXPORT void
libdecor_frame_commit(struct libdecor_frame *frame,
		      struct libdecor_state *state,
		      struct libdecor_configuration *configuration)
{
	struct libdecor_frame_private *frame_priv = frame->priv;
	struct libdecor *context = frame_priv->context;
	struct libdecor_plugin *plugin = context->plugin;

	if (configuration && configuration->has_window_state) {
		frame_priv->window_state = configuration->window_state;
		state->window_state = configuration->window_state;
	} else {
		state->window_state = frame_priv->window_state;
	}

	libdecor_frame_apply_state(frame, state);

	/* switch between decoration modes */
	if (frame_has_visible_client_side_decoration(frame)) {
		plugin->priv->iface->frame_commit(plugin, frame, state,
						  configuration);
	} else {
		plugin->priv->iface->frame_free(plugin, frame);
	}

	frame_set_window_geometry(frame,
				  frame_priv->content_width,
				  frame_priv->content_height);

	if (configuration) {
		xdg_surface_ack_configure(frame_priv->xdg_surface,
					  configuration->serial);
	}
}

static void
do_map(struct libdecor_frame *frame)
{
	struct libdecor_frame_private *frame_priv = frame->priv;

	frame_priv->pending_map = false;
	wl_surface_commit(frame_priv->wl_surface);
}

LIBDECOR_EXPORT void
libdecor_frame_map(struct libdecor_frame *frame)
{
	struct libdecor_frame_private *frame_priv = frame->priv;

	if (!frame_priv->xdg_surface) {
		frame_priv->pending_map = true;
		return;
	}

	do_map(frame);
}

LIBDECOR_EXPORT struct wl_surface *
libdecor_frame_get_wl_surface(struct libdecor_frame *frame)
{
	struct libdecor_frame_private *frame_priv = frame->priv;

	return frame_priv->wl_surface;
}

LIBDECOR_EXPORT struct xdg_surface *
libdecor_frame_get_xdg_surface(struct libdecor_frame *frame)
{
	struct libdecor_frame_private *frame_priv = frame->priv;

	return frame_priv->xdg_surface;
}

LIBDECOR_EXPORT struct xdg_toplevel *
libdecor_frame_get_xdg_toplevel(struct libdecor_frame *frame)
{
	return frame->priv->xdg_toplevel;
}

LIBDECOR_EXPORT int
libdecor_frame_get_content_width(struct libdecor_frame *frame)
{
	struct libdecor_frame_private *frame_priv = frame->priv;

	return frame_priv->content_width;
}

LIBDECOR_EXPORT int
libdecor_frame_get_content_height(struct libdecor_frame *frame)
{
	struct libdecor_frame_private *frame_priv = frame->priv;

	return frame_priv->content_height;
}

LIBDECOR_EXPORT enum libdecor_window_state
libdecor_frame_get_window_state(struct libdecor_frame *frame)
{
	struct libdecor_frame_private *frame_priv = frame->priv;

	return frame_priv->window_state;
}

LIBDECOR_EXPORT enum libdecor_wm_capabilities
libdecor_frame_get_wm_capabilities(struct libdecor_frame *frame)
{
	struct libdecor_frame_private *frame_priv = frame->priv;

	return frame_priv->wm_capabilities;
}

LIBDECOR_EXPORT int
libdecor_plugin_init(struct libdecor_plugin *plugin,
		     struct libdecor *context,
		     struct libdecor_plugin_interface *iface)
{
	plugin->priv = zalloc(sizeof (struct libdecor_plugin_private));
	if (!plugin->priv)
		return -1;

	plugin->priv->iface = iface;

	return 0;
}

LIBDECOR_EXPORT void
libdecor_plugin_release(struct libdecor_plugin *plugin)
{
	free(plugin->priv);
}

static void
xdg_wm_base_ping(void *user_data,
		 struct xdg_wm_base *xdg_wm_base,
		 uint32_t serial)
{
	xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
	xdg_wm_base_ping,
};

static void
init_xdg_wm_base(struct libdecor *context,
		 uint32_t id,
		 uint32_t version)
{
	context->xdg_wm_base = wl_registry_bind(context->wl_registry,
						id,
						&xdg_wm_base_interface,
						MIN(version,XDG_WM_BASE_VER));
	xdg_wm_base_add_listener(context->xdg_wm_base,
				 &xdg_wm_base_listener,
				 context);
}

static void
registry_handle_global(void *user_data,
		       struct wl_registry *wl_registry,
		       uint32_t id,
		       const char *interface,
		       uint32_t version)
{
	struct libdecor *context = user_data;

	if (!strcmp(interface, xdg_wm_base_interface.name)) {
		init_xdg_wm_base(context, id, version);
	} else if (!strcmp(interface, zxdg_decoration_manager_v1_interface.name)) {
		const char *force_csd = getenv("LIBDECOR_FORCE_CSD");

		if (force_csd && atoi(force_csd)) {
			return;
		}

		context->decoration_manager = wl_registry_bind(
				context->wl_registry, id,
				&zxdg_decoration_manager_v1_interface,
				MIN(version,2));
	}
}

static void
registry_handle_global_remove(void *user_data,
			      struct wl_registry *wl_registry,
			      uint32_t name)
{
}

static const struct wl_registry_listener registry_listener = {
	registry_handle_global,
	registry_handle_global_remove
};

static bool
is_compositor_compatible(struct libdecor *context)
{
	if (!context->xdg_wm_base)
		return false;

	return true;
}

static void
notify_error(struct libdecor *context,
	     enum libdecor_error error,
	     const char *message)
{
	context->has_error = true;
	context->iface->error(context, error, message);
	context->plugin->priv->iface->destroy(context->plugin);
}

static void
finish_init(struct libdecor *context)
{
	struct libdecor_frame *frame;

	wl_list_for_each(frame, &context->frames, link)
		init_shell_surface(frame);
}

static void
init_wl_display_callback(void *user_data,
			 struct wl_callback *callback,
			 uint32_t time)
{
	struct libdecor *context = user_data;

	context->init_done = true;

	wl_callback_destroy(callback);
	context->init_callback = NULL;

	if (!is_compositor_compatible(context)) {
		notify_error(context,
			     LIBDECOR_ERROR_COMPOSITOR_INCOMPATIBLE,
			     "Compositor is missing required interfaces");
	}

	if (context->plugin_ready) {
		finish_init(context);
	}
}

static const struct wl_callback_listener init_wl_display_callback_listener = {
	init_wl_display_callback
};

struct plugin_loader {
	struct wl_list link;
	void *lib;
	const struct libdecor_plugin_description *description;
	int priority;
	char *name;
};

static int
calculate_priority(const struct libdecor_plugin_description *plugin_description)
{
	const char *current_desktop;
	int i;

	if (!plugin_description->priorities)
		return -1;

	current_desktop = getenv("XDG_CURRENT_DESKTOP");

	i = 0;
	while (true) {
		struct libdecor_plugin_priority priority =
			plugin_description->priorities[i];

		i++;

		if (priority.desktop) {
			char *tokens;
			char *saveptr;
			char *token;

			if (!current_desktop)
				continue;

			tokens = strdup(current_desktop);
			token = strtok_r(tokens, ":", &saveptr);
			while (token) {
				if (strcmp(priority.desktop, token) == 0) {
					free(tokens);
					return priority.priority;
				}
				token = strtok_r(NULL, ":", &saveptr);
			}
			free(tokens);
		} else {
			return priority.priority;
		}
	}

	return -1;
}

static bool
check_symbol_conflicts(const struct libdecor_plugin_description *plugin_description, void *lib)
{
	char * const *symbol;

	symbol = plugin_description->conflicting_symbols;
	while (*symbol) {
		dlerror();
		void *sym = dlsym(RTLD_DEFAULT, *symbol);
		if (!dlerror()) {
			void *libsym = dlsym(lib, *symbol);
			if (!dlerror() && libsym != sym) {
				fprintf(stderr, "Plugin \"%s\" uses conflicting symbol \"%s\".\n",
					plugin_description->description, *symbol);
				return false;
			}
		}

		symbol++;
	}

	return true;
}

static struct plugin_loader *
load_plugin_loader(struct libdecor *context,
		   const char *path,
		   const char *name)
{
	char *ext;
	char *filename;
	void *lib;
	const struct libdecor_plugin_description *plugin_description;
	int priority;
	struct plugin_loader *plugin_loader;

	ext = strrchr(name, '.');
	if (ext == NULL || strcmp(ext, ".so") != 0)
		return NULL;

	if (asprintf(&filename, "%s/%s", path, name) == -1)
		return NULL;

	lib = dlopen(filename, RTLD_NOW | RTLD_LAZY);
	free(filename);
	if (!lib) {
		fprintf(stderr, "Failed to load plugin: '%s'\n", dlerror());
		return NULL;
	}

	plugin_description = dlsym(lib, "libdecor_plugin_description");
	if (!plugin_description) {
		fprintf(stderr,
			"Failed to load plugin '%s': no plugin description symbol\n",
			name);
		dlclose(lib);
		return NULL;
	}

	if (plugin_description->api_version != LIBDECOR_PLUGIN_API_VERSION) {
		fprintf(stderr,
			"Plugin '%s' found, but it's incompatible "
			"(expected API version %d, but got %d)\n",
			name,
			LIBDECOR_PLUGIN_API_VERSION,
			plugin_description->api_version);
		dlclose(lib);
		return NULL;
	}

	if (!(plugin_description->capabilities & LIBDECOR_PLUGIN_CAPABILITY_BASE)) {
		dlclose(lib);
		return NULL;
	}

	if (!check_symbol_conflicts(plugin_description, lib)) {
		dlclose(lib);
		return NULL;
	}

	priority = calculate_priority(plugin_description);
	if (priority == -1) {
		fprintf(stderr,
			"Plugin '%s' found, but has an invalid description\n",
			name);
		dlclose(lib);
		return NULL;
	}

	plugin_loader = zalloc(sizeof *plugin_loader);
	plugin_loader->description = plugin_description;
	plugin_loader->lib = lib;
	plugin_loader->priority = priority;
	plugin_loader->name = strdup(name);

	return plugin_loader;
}

static bool
plugin_loader_higher_priority(struct plugin_loader *plugin_loader,
			      struct plugin_loader *best_plugin_loader)
{
	return plugin_loader->priority > best_plugin_loader->priority;
}

static int
init_plugins(struct libdecor *context)
{
	const char *plugin_dir_env;
	char *all_plugin_dirs;
	char *plugin_dir;
	char *saveptr;
	DIR *dir;
	struct wl_list plugin_loaders;
	struct plugin_loader *plugin_loader, *tmp;
	struct plugin_loader *best_plugin_loader;
	struct libdecor_plugin *plugin;

	plugin_dir_env = getenv("LIBDECOR_PLUGIN_DIR");
	if (!plugin_dir_env) {
		plugin_dir_env = LIBDECOR_PLUGIN_DIR;
	}
	all_plugin_dirs = strdup(plugin_dir_env);

	wl_list_init(&plugin_loaders);
	plugin_dir = strtok_r(all_plugin_dirs, ":", &saveptr);
	while (plugin_dir) {
		dir = opendir(plugin_dir);
		if (!dir) {
			fprintf(stderr, "Couldn't open plugin directory: %s\n",
				strerror(errno));
		} else {

			while (true) {
				struct dirent *de;

				de = readdir(dir);
				if (!de)
					break;

				plugin_loader = load_plugin_loader(context,
								   plugin_dir,
								   de->d_name);
				if (!plugin_loader)
					continue;

				wl_list_insert(plugin_loaders.prev, &plugin_loader->link);
			}

			closedir(dir);
		}
		plugin_dir = strtok_r(NULL, ":", &saveptr);
	}
	free(all_plugin_dirs);

retry_next:
	best_plugin_loader = NULL;
	wl_list_for_each(plugin_loader, &plugin_loaders, link) {
		if (!best_plugin_loader) {
			best_plugin_loader = plugin_loader;
			continue;
		}

		if (plugin_loader_higher_priority(plugin_loader,
						  best_plugin_loader))
			best_plugin_loader = plugin_loader;
	}

	if (!best_plugin_loader)
		return -1;

	plugin_loader = best_plugin_loader;
	plugin = plugin_loader->description->constructor(context);
	if (!plugin) {
		fprintf(stderr,
			"Failed to load plugin '%s': failed to init\n",
			plugin_loader->name);
		dlclose(plugin_loader->lib);
		wl_list_remove(&plugin_loader->link);
		free(plugin_loader->name);
		free(plugin_loader);
		goto retry_next;
	}

	context->plugin = plugin;

	wl_list_remove(&plugin_loader->link);
	free(plugin_loader->name);
	free(plugin_loader);

	wl_list_for_each_safe(plugin_loader, tmp, &plugin_loaders, link) {
		dlclose(plugin_loader->lib);
		free(plugin_loader->name);
		free(plugin_loader);
	}

	return 0;
}

LIBDECOR_EXPORT void *
libdecor_get_user_data(struct libdecor *context)
{
	return context->user_data;
}

LIBDECOR_EXPORT void
libdecor_set_user_data(struct libdecor *context, void *user_data)
{
	context->user_data = user_data;
}

LIBDECOR_EXPORT int
libdecor_get_fd(struct libdecor *context)
{
	struct libdecor_plugin *plugin = context->plugin;

	return plugin->priv->iface->get_fd(plugin);
}

LIBDECOR_EXPORT int
libdecor_dispatch(struct libdecor *context,
		  int timeout)
{
	struct libdecor_plugin *plugin = context->plugin;

	return plugin->priv->iface->dispatch(plugin, timeout);
}

LIBDECOR_EXPORT struct wl_display *
libdecor_get_wl_display(struct libdecor *context)
{
	return context->wl_display;
}

LIBDECOR_EXPORT void
libdecor_notify_plugin_ready(struct libdecor *context)
{
	context->plugin_ready = true;

	if (context->init_done)
		finish_init(context);
}

LIBDECOR_EXPORT void
libdecor_notify_plugin_error(struct libdecor *context,
			     enum libdecor_error error,
			     const char *__restrict fmt,
			     ...)
{
	char *msg = NULL;
	int nbytes = 0;
	va_list argp;

	if (context->has_error)
		return;

	va_start(argp, fmt);
	nbytes = vasprintf(&msg, fmt, argp);
	va_end(argp);

	if (nbytes>0)
		notify_error(context, error, msg);

	if (msg)
		free(msg);
}

LIBDECOR_EXPORT void
libdecor_unref(struct libdecor *context)
{
	context->ref_count--;
	if (context->ref_count == 0) {
		if (context->plugin)
			context->plugin->priv->iface->destroy(context->plugin);
		if (context->init_callback)
			wl_callback_destroy(context->init_callback);
		wl_registry_destroy(context->wl_registry);
		if (context->xdg_wm_base)
			xdg_wm_base_destroy(context->xdg_wm_base);
		if (context->decoration_manager)
			zxdg_decoration_manager_v1_destroy(
						context->decoration_manager);
		free(context);
	}
}

LIBDECOR_EXPORT struct libdecor *
libdecor_new(struct wl_display *wl_display,
	     const struct libdecor_interface *iface)
{
	return libdecor_new_with_user_data(wl_display, iface, NULL);
}

LIBDECOR_EXPORT struct libdecor *
libdecor_new_with_user_data(struct wl_display *wl_display,
	     const struct libdecor_interface *iface,
	     void *user_data)
{
	struct libdecor *context;

	context = zalloc(sizeof *context);

	context->ref_count = 1;
	context->iface = iface;
	context->user_data = user_data;
	context->wl_display = wl_display;
	context->wl_registry = wl_display_get_registry(wl_display);
	wl_registry_add_listener(context->wl_registry,
				 &registry_listener,
				 context);
	context->init_callback = wl_display_sync(context->wl_display);
	wl_callback_add_listener(context->init_callback,
				 &init_wl_display_callback_listener,
				 context);

	wl_list_init(&context->frames);

	if (init_plugins(context) != 0) {
		fprintf(stderr,
			"No plugins found, falling back on no decorations\n");
		context->plugin = libdecor_fallback_plugin_new(context);
	}

	wl_display_flush(wl_display);

	return context;
}
