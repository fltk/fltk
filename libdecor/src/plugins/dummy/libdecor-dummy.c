/*
 * Copyright © 2021 Jonas Ådahl
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

#include "libdecor-plugin.h"

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdlib.h>
#include <wayland-cursor.h>

#include "utils.h"

struct libdecor_plugin_dummy {
	struct libdecor_plugin plugin;
	struct libdecor *context;
};

static void
libdecor_plugin_dummy_destroy(struct libdecor_plugin *plugin)
{
	struct libdecor_plugin_dummy *plugin_dummy =
		(struct libdecor_plugin_dummy *) plugin;

	free(plugin_dummy);
}

static struct libdecor_frame *
libdecor_plugin_dummy_frame_new(struct libdecor_plugin *plugin)
{
	struct libdecor_frame *frame;

	frame = zalloc(sizeof *frame);

	return frame;
}

static void
libdecor_plugin_dummy_frame_free(struct libdecor_plugin *plugin,
				 struct libdecor_frame *frame)
{
}

static void
libdecor_plugin_dummy_frame_commit(struct libdecor_plugin *plugin,
				   struct libdecor_frame *frame,
				   struct libdecor_state *state,
				   struct libdecor_configuration *configuration)
{
}

static void
libdecor_plugin_dummy_frame_property_changed(struct libdecor_plugin *plugin,
					     struct libdecor_frame *frame)
{
}

static void
libdecor_plugin_dummy_frame_translate_coordinate(struct libdecor_plugin *plugin,
						 struct libdecor_frame *frame,
						 int content_x,
						 int content_y,
						 int *frame_x,
						 int *frame_y)
{
	*frame_x = content_x;
	*frame_y = content_y;
}

static void
libdecor_plugin_dummy_frame_popup_grab(struct libdecor_plugin *plugin,
				       struct libdecor_frame *frame,
				       const char *seat_name)
{
}

static void
libdecor_plugin_dummy_frame_popup_ungrab(struct libdecor_plugin *plugin,
					 struct libdecor_frame *frame,
					 const char *seat_name)
{
}

static bool
libdecor_plugin_dummy_configuration_get_content_size(
		struct libdecor_plugin *plugin,
		struct libdecor_configuration *configuration,
		struct libdecor_frame *frame,
		int *content_width,
		int *content_height)
{
	return libdecor_configuration_get_window_size(configuration,
						      content_width,
						      content_height);
}

static bool
libdecor_plugin_dummy_frame_get_window_size_for(
		struct libdecor_plugin *plugin,
		struct libdecor_frame *frame,
		struct libdecor_state *state,
		int *window_width,
		int *window_height)
{
	*window_width = libdecor_state_get_content_width (state);
	*window_height = libdecor_state_get_content_height (state);
	return true;
}

static struct libdecor_plugin_interface dummy_plugin_iface = {
	.destroy = libdecor_plugin_dummy_destroy,

	.frame_new = libdecor_plugin_dummy_frame_new,
	.frame_free = libdecor_plugin_dummy_frame_free,
	.frame_commit = libdecor_plugin_dummy_frame_commit,
	.frame_property_changed = libdecor_plugin_dummy_frame_property_changed,
	.frame_translate_coordinate =
		libdecor_plugin_dummy_frame_translate_coordinate,
	.frame_popup_grab = libdecor_plugin_dummy_frame_popup_grab,
	.frame_popup_ungrab = libdecor_plugin_dummy_frame_popup_ungrab,

	.configuration_get_content_size =
			libdecor_plugin_dummy_configuration_get_content_size,
	.frame_get_window_size_for =
			libdecor_plugin_dummy_frame_get_window_size_for,
};

static struct libdecor_plugin *
libdecor_plugin_new(struct libdecor *context)
{
	struct libdecor_plugin_dummy *plugin_dummy;

	plugin_dummy = zalloc(sizeof *plugin_dummy);
	plugin_dummy->plugin.iface = &dummy_plugin_iface;
	plugin_dummy->context = context;

	libdecor_notify_plugin_ready(context);

	return &plugin_dummy->plugin;
}

static struct libdecor_plugin_priority priorities[] = {
	{ NULL, LIBDECOR_PLUGIN_PRIORITY_LOW }
};

LIBDECOR_EXPORT const struct libdecor_plugin_description
libdecor_plugin_description = {
	.api_version = LIBDECOR_PLUGIN_API_VERSION,
	.description = "dummy libdecor plugin",
	.priorities = priorities,
	.constructor = libdecor_plugin_new,
};
