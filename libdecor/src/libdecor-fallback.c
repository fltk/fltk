/*
 * Copyright © 2019 Jonas Ådahl
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

#include "libdecor-fallback.h"

#include <poll.h>
#include <errno.h>

#include "utils.h"

struct libdecor_plugin_fallback {
	struct libdecor_plugin plugin;
	struct libdecor *context;
};

static void
libdecor_plugin_fallback_destroy(struct libdecor_plugin *plugin)
{
	libdecor_plugin_release(plugin);
	free(plugin);
}

static int
libdecor_plugin_fallback_get_fd(struct libdecor_plugin *plugin)
{
	struct libdecor_plugin_fallback *plugin_fallback =
		(struct libdecor_plugin_fallback *) plugin;
	struct wl_display *wl_display =
		libdecor_get_wl_display(plugin_fallback->context);

	return wl_display_get_fd(wl_display);
}

static int
libdecor_plugin_fallback_dispatch(struct libdecor_plugin *plugin,
				  int timeout)
{
	struct libdecor_plugin_fallback *plugin_fallback =
		(struct libdecor_plugin_fallback *) plugin;
	struct wl_display *wl_display =
		libdecor_get_wl_display(plugin_fallback->context);
	struct pollfd fds[1];
	int ret;
	int dispatch_count = 0;

	while (wl_display_prepare_read(wl_display) != 0)
		dispatch_count += wl_display_dispatch_pending(wl_display);

	if (wl_display_flush(wl_display) < 0 &&
	    errno != EAGAIN) {
		wl_display_cancel_read(wl_display);
		return -errno;
	}

	fds[0] = (struct pollfd) { wl_display_get_fd(wl_display), POLLIN };

	ret = poll(fds, ARRAY_SIZE (fds), timeout);
	if (ret > 0) {
		if (fds[0].revents & POLLIN) {
			wl_display_read_events(wl_display);
			dispatch_count += wl_display_dispatch_pending(wl_display);
			return dispatch_count;
		} else {
			wl_display_cancel_read(wl_display);
			return dispatch_count;
		}
	} else if (ret == 0) {
		wl_display_cancel_read(wl_display);
		return dispatch_count;
	} else {
		wl_display_cancel_read(wl_display);
		return -errno;
	}
}

static struct libdecor_frame *
libdecor_plugin_fallback_frame_new(struct libdecor_plugin *plugin)
{
	struct libdecor_frame *frame;

	frame = zalloc(sizeof *frame);

	return frame;
}

static void
libdecor_plugin_fallback_frame_free(struct libdecor_plugin *plugin,
				    struct libdecor_frame *frame)
{
}

static void
libdecor_plugin_fallback_frame_commit(struct libdecor_plugin *plugin,
				      struct libdecor_frame *frame,
				      struct libdecor_state *state,
				      struct libdecor_configuration *configuration)
{
}

static void
libdecor_plugin_fallback_frame_property_changed(struct libdecor_plugin *plugin,
						struct libdecor_frame *frame)
{
}

static void
libdecor_plugin_fallback_frame_popup_grab(struct libdecor_plugin *plugin,
					  struct libdecor_frame *frame,
					  const char *seat_name)
{
}

static void
libdecor_plugin_fallback_frame_popup_ungrab(struct libdecor_plugin *plugin,
					    struct libdecor_frame *frame,
					    const char *seat_name)
{
}

static bool
libdecor_plugin_fallback_frame_get_border_size(struct libdecor_plugin *plugin,
					       struct libdecor_frame *frame,
					       struct libdecor_configuration *configuration,
					       int *left,
					       int *right,
					       int *top,
					       int *bottom)
{
	if (left)
		*left = 0;
	if (right)
		*right = 0;
	if (top)
		*top = 0;
	if (bottom)
		*bottom = 0;

	return true;
}

static struct libdecor_plugin_interface fallback_plugin_iface = {
	.destroy = libdecor_plugin_fallback_destroy,
	.get_fd = libdecor_plugin_fallback_get_fd,
	.dispatch = libdecor_plugin_fallback_dispatch,
	.frame_new = libdecor_plugin_fallback_frame_new,
	.frame_free = libdecor_plugin_fallback_frame_free,
	.frame_commit = libdecor_plugin_fallback_frame_commit,
	.frame_property_changed = libdecor_plugin_fallback_frame_property_changed,
	.frame_popup_grab = libdecor_plugin_fallback_frame_popup_grab,
	.frame_popup_ungrab = libdecor_plugin_fallback_frame_popup_ungrab,
	.frame_get_border_size = libdecor_plugin_fallback_frame_get_border_size,
};

struct libdecor_plugin *
libdecor_fallback_plugin_new(struct libdecor *context)
{
	struct libdecor_plugin_fallback *plugin;

	plugin = zalloc(sizeof *plugin);
	libdecor_plugin_init(&plugin->plugin, context, &fallback_plugin_iface);
	plugin->context = context;

	libdecor_notify_plugin_ready(context);

	return &plugin->plugin;
}
