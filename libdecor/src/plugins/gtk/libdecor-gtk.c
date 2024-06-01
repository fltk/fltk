/*
 * Copyright © 2018 Jonas Ådahl
 * Copyright © 2021 Christian Rauch
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

#include <linux/input.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <wayland-client-core.h>
#include <wayland-cursor.h>

#include "libdecor-plugin.h"
#include "utils.h"
#include "desktop-settings.h"
#include "os-compatibility.h"

#include <cairo/cairo.h>

#include "common/libdecor-cairo-blur.h"
#include <poll.h>

#include <gtk/gtk.h>

static const size_t SHADOW_MARGIN = 24;	/* grabbable part of the border */

static const char *cursor_names[] = {
	"top_side",
	"bottom_side",
	"left_side",
	"top_left_corner",
	"bottom_left_corner",
	"right_side",
	"top_right_corner",
	"bottom_right_corner"
};

enum header_element {
	HEADER_NONE,
	HEADER_FULL, /* entire header bar */
	HEADER_TITLE, /* label */
	HEADER_MIN,
	HEADER_MAX,
	HEADER_CLOSE,
};

enum titlebar_gesture_state {
	TITLEBAR_GESTURE_STATE_INIT,
	TITLEBAR_GESTURE_STATE_BUTTON_PRESSED,
	TITLEBAR_GESTURE_STATE_CONSUMED,
	TITLEBAR_GESTURE_STATE_DISCARDED,
};

struct header_element_data {
	const char *name;
	enum header_element type;
	/* pointer to button or NULL if not found*/
	GtkWidget *widget;
	GtkStateFlags state;
};

static void
find_widget_by_name(GtkWidget *widget, void *data)
{
	if (GTK_IS_WIDGET(widget)) {
		char *style_ctx = gtk_style_context_to_string(
					  gtk_widget_get_style_context(widget),
					  GTK_STYLE_CONTEXT_PRINT_SHOW_STYLE);
		if (strstr(style_ctx, ((struct header_element_data *)data)->name)) {
			((struct header_element_data *)data)->widget = widget;
			free(style_ctx);
			return;
		}
		free(style_ctx);
	}

	if (GTK_IS_CONTAINER(widget)) {
		/* recursively traverse container */
		gtk_container_forall(GTK_CONTAINER(widget), &find_widget_by_name, data);
	}
}

static struct header_element_data
find_widget_by_type(GtkWidget *widget, enum header_element type)
{
	char* name = NULL;
	switch (type) {
	case HEADER_FULL:
		name = "headerbar.titlebar:";
		break;
	case HEADER_TITLE:
		name = "label.title:";
		break;
	case HEADER_MIN:
		name = ".minimize";
		break;
	case HEADER_MAX:
		name = ".maximize";
		break;
	case HEADER_CLOSE:
		name = ".close";
		break;
	default:
		break;
	}

	struct header_element_data data = {.name = name, .type = type, .widget = NULL};
	find_widget_by_name(widget, &data);
	return data;
}

static bool
in_region(const cairo_rectangle_int_t *rect, const int *x, const int *y)
{
	return (*x>=rect->x) & (*y>=rect->y) &
		(*x<(rect->x+rect->width)) & (*y<(rect->y+rect->height));
}

static struct header_element_data
get_header_focus(const GtkHeaderBar *header_bar, const int x, const int y)
{
	/* we have to check child widgets (buttons, title) before the 'HDR_HDR' root widget */
	static const enum header_element elems[] =
		{HEADER_TITLE, HEADER_MIN, HEADER_MAX, HEADER_CLOSE};

	for (size_t i = 0; i < ARRAY_SIZE(elems); i++) {
		struct header_element_data elem =
			find_widget_by_type(GTK_WIDGET(header_bar), elems[i]);
		if (elem.widget) {
			GtkAllocation allocation;
			gtk_widget_get_allocation(GTK_WIDGET(elem.widget), &allocation);
			if (in_region(&allocation, &x, &y))
				return elem;
		}
	}

	struct header_element_data elem_none = { .widget=NULL};
	return elem_none;
}

static bool
streq(const char *str1,
      const char *str2)
{
	if (!str1 && !str2)
		return true;

	if (str1 && str2)
		return strcmp(str1, str2) == 0;

	return false;
}

enum decoration_type {
	DECORATION_TYPE_NONE,
	DECORATION_TYPE_ALL,
	DECORATION_TYPE_TITLE_ONLY
};

enum component {
	NONE = 0,
	SHADOW,
	HEADER,
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
	/* cursors for resize edges and corners */
	struct wl_cursor *cursors[ARRAY_LENGTH(cursor_names)];
	struct wl_cursor *cursor_left_ptr;

	struct wl_surface *pointer_focus;
	struct wl_surface *touch_focus;

	int pointer_x, pointer_y;

	uint32_t touch_down_time_stamp;

	uint32_t serial;

	bool grabbed;

	struct wl_list link;
};

struct output {
	struct libdecor_plugin_gtk *plugin_gtk;

	struct wl_output *wl_output;
	uint32_t id;
	int scale;

	struct wl_list link;
};

struct buffer {
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

struct border_component {
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

struct surface_output {
	struct output *output;
	struct wl_list link;
};

struct cursor_output {
	struct output *output;
	struct wl_list link;
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

	struct border_component *active;
	struct border_component *touch_active;

	struct border_component *focus;
	struct border_component *grab;

	bool shadow_showing;
	struct border_component shadow;

	GtkWidget *window; /* offscreen window for rendering */
	GtkWidget *header; /* header bar with widgets */
	struct border_component headerbar;
	struct header_element_data hdr_focus;

	/* store pre-processed shadow tile */
	cairo_surface_t *shadow_blur;

	struct wl_list link;

	struct {
		enum titlebar_gesture_state state;
		int button_pressed_count;
		uint32_t first_pressed_button;
		uint32_t first_pressed_time;
		double pressed_x;
		double pressed_y;
		uint32_t pressed_serial;
	} titlebar_gesture;
};

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

	uint32_t color_scheme_setting;

	int double_click_time_ms;
	int drag_threshold;
};

static const char *libdecor_gtk_proxy_tag = "libdecor-gtk";

static bool
own_proxy(struct wl_proxy *proxy)
{
	if (!proxy)
		return false;

	return (wl_proxy_get_tag(proxy) == &libdecor_gtk_proxy_tag);
}

static bool
own_surface(struct wl_surface *surface)
{
	return own_proxy((struct wl_proxy *) surface);
}

static bool
own_output(struct wl_output *output)
{
	return own_proxy((struct wl_proxy *) output);
}

static bool
moveable(struct libdecor_frame_gtk *frame_gtk) {
	return libdecor_frame_has_capability(&frame_gtk->frame,
					     LIBDECOR_ACTION_MOVE);
}

static bool
resizable(struct libdecor_frame_gtk *frame_gtk) {
	return libdecor_frame_has_capability(&frame_gtk->frame,
					     LIBDECOR_ACTION_RESIZE);
}

static bool
minimizable(struct libdecor_frame_gtk *frame_gtk) {
	return libdecor_frame_has_capability(&frame_gtk->frame,
					     LIBDECOR_ACTION_MINIMIZE);
}

static bool
closeable(struct libdecor_frame_gtk *frame_gtk) {
	return libdecor_frame_has_capability(&frame_gtk->frame,
					     LIBDECOR_ACTION_CLOSE);
}

static void
buffer_free(struct buffer *buffer);

static void
draw_border_component(struct libdecor_frame_gtk *frame_gtk,
		      struct border_component *border_component,
		      enum component component);

static void
send_cursor(struct seat *seat);

static bool
update_local_cursor(struct seat *seat);

static void
libdecor_plugin_gtk_destroy(struct libdecor_plugin *plugin)
{
	struct libdecor_plugin_gtk *plugin_gtk =
		(struct libdecor_plugin_gtk *) plugin;
	struct seat *seat, *seat_tmp;
	struct output *output, *output_tmp;
	struct libdecor_frame_gtk *frame, *frame_tmp;

	if (plugin_gtk->globals_callback)
		wl_callback_destroy(plugin_gtk->globals_callback);
	if (plugin_gtk->globals_callback_shm)
		wl_callback_destroy(plugin_gtk->globals_callback_shm);
	if (plugin_gtk->shm_callback)
		wl_callback_destroy(plugin_gtk->shm_callback);
	wl_registry_destroy(plugin_gtk->wl_registry);

	wl_list_for_each_safe(seat, seat_tmp, &plugin_gtk->seat_list, link) {
		struct cursor_output *cursor_output, *tmp;

		if (seat->wl_pointer)
			wl_pointer_destroy(seat->wl_pointer);
		if (seat->wl_touch)
			wl_touch_destroy(seat->wl_touch);
		if (seat->cursor_surface)
			wl_surface_destroy(seat->cursor_surface);
		wl_seat_destroy(seat->wl_seat);
		if (seat->cursor_theme)
			wl_cursor_theme_destroy(seat->cursor_theme);

		wl_list_for_each_safe(cursor_output, tmp, &seat->cursor_outputs, link) {
			wl_list_remove(&cursor_output->link);
			free(cursor_output);
		}

		free(seat->name);
		free(seat);
	}

	wl_list_for_each_safe(output, output_tmp,
			      &plugin_gtk->output_list, link) {
		if (wl_output_get_version (output->wl_output) >=
		    WL_OUTPUT_RELEASE_SINCE_VERSION)
			wl_output_release(output->wl_output);
		else
			wl_output_destroy(output->wl_output);
		free(output);
	}

	wl_list_for_each_safe(frame, frame_tmp,
			      &plugin_gtk->visible_frame_list, link) {
		wl_list_remove(&frame->link);
	}

	free(plugin_gtk->cursor_theme_name);

	if (plugin_gtk->wl_shm)
		wl_shm_destroy(plugin_gtk->wl_shm);

	if (plugin_gtk->wl_compositor)
		wl_compositor_destroy(plugin_gtk->wl_compositor);
	if (plugin_gtk->wl_subcompositor)
		wl_subcompositor_destroy(plugin_gtk->wl_subcompositor);

	libdecor_plugin_release(&plugin_gtk->plugin);
	free(plugin_gtk);
}

static struct libdecor_frame_gtk *
libdecor_frame_gtk_new(struct libdecor_plugin_gtk *plugin_gtk)
{
	struct libdecor_frame_gtk *frame_gtk = zalloc(sizeof *frame_gtk);
	cairo_t *cr;

	static const int size = 128;
	static const int boundary = 32;

	frame_gtk->plugin_gtk = plugin_gtk;
	frame_gtk->shadow_blur = cairo_image_surface_create(
					CAIRO_FORMAT_ARGB32, size, size);
	wl_list_insert(&plugin_gtk->visible_frame_list, &frame_gtk->link);

	cr = cairo_create(frame_gtk->shadow_blur);
	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
	cairo_set_source_rgba(cr, 0, 0, 0, 1);
	cairo_rectangle(cr, boundary, boundary,
			size - 2 * boundary,
			size - 2 * boundary);
	cairo_fill(cr);
	cairo_destroy(cr);
	blur_surface(frame_gtk->shadow_blur, 64);

	return frame_gtk;
}

static int
libdecor_plugin_gtk_get_fd(struct libdecor_plugin *plugin)
{
	struct libdecor_plugin_gtk *plugin_gtk =
		(struct libdecor_plugin_gtk *) plugin;
	struct wl_display *wl_display =
		libdecor_get_wl_display(plugin_gtk->context);

	return wl_display_get_fd(wl_display);
}

static int
libdecor_plugin_gtk_dispatch(struct libdecor_plugin *plugin,
			     int timeout)
{
	struct libdecor_plugin_gtk *plugin_gtk =
		(struct libdecor_plugin_gtk *) plugin;
	struct wl_display *wl_display =
		libdecor_get_wl_display(plugin_gtk->context);
	struct pollfd fds[1];
	int ret;
	int dispatch_count = 0;

	while (g_main_context_iteration(NULL, FALSE));

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
libdecor_plugin_gtk_frame_new(struct libdecor_plugin *plugin)
{
	struct libdecor_plugin_gtk *plugin_gtk =
		(struct libdecor_plugin_gtk *) plugin;
	struct libdecor_frame_gtk *frame_gtk;

	frame_gtk = libdecor_frame_gtk_new(plugin_gtk);

	return &frame_gtk->frame;
}

static void
toggle_maximized(struct libdecor_frame *const frame)
{
	if (!resizable((struct libdecor_frame_gtk *)frame))
		return;

	if (!(libdecor_frame_get_window_state(frame) &
	      LIBDECOR_WINDOW_STATE_MAXIMIZED))
		libdecor_frame_set_maximized(frame);
	else
		libdecor_frame_unset_maximized(frame);
}

static void
buffer_release(void *user_data,
	       struct wl_buffer *wl_buffer)
{
	struct buffer *buffer = user_data;

	if (buffer->is_detached)
		buffer_free(buffer);
	else
		buffer->in_use = false;
}

static const struct wl_buffer_listener buffer_listener = {
	buffer_release
};

static struct buffer *
create_shm_buffer(struct libdecor_plugin_gtk *plugin_gtk,
		  int width,
		  int height,
		  bool opaque,
		  int scale)
{
	struct wl_shm_pool *pool;
	int fd, size, buffer_width, buffer_height, stride;
	void *data;
	struct buffer *buffer;
	enum wl_shm_format buf_fmt;

	buffer_width = width * scale;
	buffer_height = height * scale;
	stride = buffer_width * 4;
	size = stride * buffer_height;

	fd = libdecor_os_create_anonymous_file(size);
	if (fd < 0) {
		fprintf(stderr, "creating a buffer file for %d B failed: %s\n",
			size, strerror(errno));
		return NULL;
	}

	data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (data == MAP_FAILED) {
		fprintf(stderr, "mmap failed: %s\n", strerror(errno));
		close(fd);
		return NULL;
	}

	buf_fmt = opaque ? WL_SHM_FORMAT_XRGB8888 : WL_SHM_FORMAT_ARGB8888;

	pool = wl_shm_create_pool(plugin_gtk->wl_shm, fd, size);
	buffer = zalloc(sizeof *buffer);
	buffer->wl_buffer = wl_shm_pool_create_buffer(pool, 0,
						      buffer_width, buffer_height,
						      stride,
						      buf_fmt);
	wl_buffer_add_listener(buffer->wl_buffer, &buffer_listener, buffer);
	wl_shm_pool_destroy(pool);
	close(fd);

	buffer->data = data;
	buffer->data_size = size;
	buffer->width = width;
	buffer->height = height;
	buffer->scale = scale;
	buffer->buffer_width = buffer_width;
	buffer->buffer_height = buffer_height;

	return buffer;
}

static void
buffer_free(struct buffer *buffer)
{
	if (buffer->wl_buffer) {
		wl_buffer_destroy(buffer->wl_buffer);
		munmap(buffer->data, buffer->data_size);
		buffer->wl_buffer = NULL;
		buffer->in_use = false;
	}
	free(buffer);
}

static void
free_border_component(struct border_component *border_component)
{
	if (border_component->wl_surface) {
		wl_subsurface_destroy(border_component->wl_subsurface);
		border_component->wl_subsurface = NULL;
		wl_surface_destroy(border_component->wl_surface);
		border_component->wl_surface = NULL;
	}
	if (border_component->buffer) {
		buffer_free(border_component->buffer);
		border_component->buffer = NULL;
	}
	if (border_component->output_list.next != NULL) {
		struct surface_output *surface_output, *surface_output_tmp;
		wl_list_for_each_safe(surface_output, surface_output_tmp,
				      &border_component->output_list, link) {
			wl_list_remove(&surface_output->link);
			free(surface_output);
		}
	}
}

static void
libdecor_plugin_gtk_frame_free(struct libdecor_plugin *plugin,
				 struct libdecor_frame *frame)
{
	struct libdecor_frame_gtk *frame_gtk =
		(struct libdecor_frame_gtk *) frame;

	g_clear_pointer (&frame_gtk->header, gtk_widget_destroy);
	g_clear_pointer (&frame_gtk->window, gtk_widget_destroy);

	free_border_component(&frame_gtk->headerbar);
	free_border_component(&frame_gtk->shadow);
	frame_gtk->shadow_showing = false;

	g_clear_pointer (&frame_gtk->shadow_blur, cairo_surface_destroy);

	g_clear_pointer (&frame_gtk->title, free);

	frame_gtk->decoration_type = DECORATION_TYPE_NONE;

	if (frame_gtk->link.next != NULL)
		wl_list_remove(&frame_gtk->link);
}

static bool
is_border_surfaces_showing(struct libdecor_frame_gtk *frame_gtk)
{
	return frame_gtk->shadow_showing;
}

static void
hide_border_component(struct border_component *border_component)
{
	if (!border_component->wl_surface)
		return;

	wl_surface_attach(border_component->wl_surface, NULL, 0, 0);
	wl_surface_commit(border_component->wl_surface);
}

static void
hide_border_surfaces(struct libdecor_frame_gtk *frame_gtk)
{
	hide_border_component(&frame_gtk->shadow);
	frame_gtk->shadow_showing = false;
}

static struct border_component *
get_component_for_surface(struct libdecor_frame_gtk *frame_gtk,
			  const struct wl_surface *surface)
{
	if (frame_gtk->shadow.wl_surface == surface)
		return &frame_gtk->shadow;
	if (frame_gtk->headerbar.wl_surface == surface)
		return &frame_gtk->headerbar;
	return NULL;
}

static bool
redraw_scale(struct libdecor_frame_gtk *frame_gtk,
	     struct border_component *cmpnt)
{
	struct surface_output *surface_output;
	int scale = 1;

	if (cmpnt->wl_surface == NULL)
		return false;

	wl_list_for_each(surface_output, &cmpnt->output_list, link) {
		scale = MAX(scale, surface_output->output->scale);
	}
	if (scale != cmpnt->scale) {
		cmpnt->scale = scale;
		if ((cmpnt->type != SHADOW) || is_border_surfaces_showing(frame_gtk)) {
			draw_border_component(frame_gtk, cmpnt, cmpnt->type);
			return true;
		}
	}
	return false;
}

static bool
add_surface_output(struct libdecor_plugin_gtk *plugin_gtk,
		   struct wl_output *wl_output,
		   struct wl_list *list)
{
	struct output *output;
	struct surface_output *surface_output;

	if (!own_output(wl_output))
		return false;

	output = wl_output_get_user_data(wl_output);

	if (output == NULL)
		return false;

	surface_output = zalloc(sizeof *surface_output);
	surface_output->output = output;
	wl_list_insert(list, &surface_output->link);
	return true;
}

static void
surface_enter(void *data,
	      struct wl_surface *wl_surface,
	      struct wl_output *wl_output)
{
	struct libdecor_frame_gtk *frame_gtk = data;
	struct border_component *cmpnt;

	if (!(own_surface(wl_surface) && own_output(wl_output)))
	    return;

	cmpnt = get_component_for_surface(frame_gtk, wl_surface);
	if (cmpnt == NULL)
		return;

	if (!add_surface_output(frame_gtk->plugin_gtk, wl_output,
				&cmpnt->output_list))
		return;

	if (redraw_scale(frame_gtk, cmpnt))
		libdecor_frame_toplevel_commit(&frame_gtk->frame);
}

static bool
remove_surface_output(struct wl_list *list, const struct wl_output *wl_output)
{
	struct surface_output *surface_output;
	wl_list_for_each(surface_output, list, link) {
		if (surface_output->output->wl_output == wl_output) {
			wl_list_remove(&surface_output->link);
			free(surface_output);
			return true;
		}
	}
	return false;
}

static void
surface_leave(void *data,
	      struct wl_surface *wl_surface,
	      struct wl_output *wl_output)
{
	struct libdecor_frame_gtk *frame_gtk = data;
	struct border_component *cmpnt;

	if (!(own_surface(wl_surface) && own_output(wl_output)))
	    return;

	cmpnt = get_component_for_surface(frame_gtk, wl_surface);
	if (cmpnt == NULL)
		return;

	if (!remove_surface_output(&cmpnt->output_list, wl_output))
		return;

	if (redraw_scale(frame_gtk, cmpnt))
		libdecor_frame_toplevel_commit(&frame_gtk->frame);
}

static struct wl_surface_listener surface_listener = {
	surface_enter,
	surface_leave,
};

static void
create_surface_subsurface_pair(struct libdecor_frame_gtk *frame_gtk,
			       struct wl_surface **out_wl_surface,
			       struct wl_subsurface **out_wl_subsurface)
{
	struct libdecor_plugin_gtk *plugin_gtk = frame_gtk->plugin_gtk;
	struct libdecor_frame *frame = &frame_gtk->frame;
	struct wl_compositor *wl_compositor = plugin_gtk->wl_compositor;
	struct wl_subcompositor *wl_subcompositor = plugin_gtk->wl_subcompositor;
	struct wl_surface *wl_surface;
	struct wl_surface *parent;
	struct wl_subsurface *wl_subsurface;

	wl_surface = wl_compositor_create_surface(wl_compositor);
	wl_proxy_set_tag((struct wl_proxy *) wl_surface,
			 &libdecor_gtk_proxy_tag);

	parent = libdecor_frame_get_wl_surface(frame);
	wl_subsurface = wl_subcompositor_get_subsurface(wl_subcompositor,
							wl_surface,
							parent);

	*out_wl_surface = wl_surface;
	*out_wl_subsurface = wl_subsurface;
}

static void
ensure_component(struct libdecor_frame_gtk *frame_gtk,
		 struct border_component *cmpnt)
{
	if (!cmpnt->wl_surface) {
		wl_list_init(&cmpnt->output_list);
		cmpnt->scale = 1;
		create_surface_subsurface_pair(frame_gtk,
					       &cmpnt->wl_surface,
					       &cmpnt->wl_subsurface);
		wl_surface_add_listener(cmpnt->wl_surface, &surface_listener,
					frame_gtk);
	}
}

static void
ensure_border_surfaces(struct libdecor_frame_gtk *frame_gtk)
{
	frame_gtk->shadow.type = SHADOW;
	frame_gtk->shadow.opaque = false;
	ensure_component(frame_gtk, &frame_gtk->shadow);
}

static void
ensure_title_bar_surfaces(struct libdecor_frame_gtk *frame_gtk)
{
	GtkStyleContext *context_hdr;

	frame_gtk->headerbar.type = HEADER;
	frame_gtk->headerbar.opaque = false;
	ensure_component(frame_gtk, &frame_gtk->headerbar);

	/* create an offscreen window with a header bar */
	/* TODO: This should only be done once at frame consutrction, but then
	 *       the window and headerbar would not change style (e.g. backdrop)
	 *       after construction. So we just destroy and re-create them.
	 */
	/* avoid warning when restoring previously turned off decoration */
	if (GTK_IS_WIDGET(frame_gtk->header)) {
		gtk_widget_destroy(frame_gtk->header);
		frame_gtk->header = NULL;
	}
	/* avoid warning when restoring previously turned off decoration */
	if (GTK_IS_WIDGET(frame_gtk->window)) {
		gtk_widget_destroy(frame_gtk->window);
		frame_gtk->window = NULL;
	}
	frame_gtk->window = gtk_offscreen_window_new();
	frame_gtk->header = gtk_header_bar_new();

	g_object_get(gtk_widget_get_settings(frame_gtk->window),
		     "gtk-double-click-time",
		     &frame_gtk->plugin_gtk->double_click_time_ms,
		     "gtk-dnd-drag-threshold",
		     &frame_gtk->plugin_gtk->drag_threshold,
		     NULL);
	/* set as "default" decoration */
	g_object_set(frame_gtk->header,
		     "title", libdecor_frame_get_title(&frame_gtk->frame),
		     "has-subtitle", FALSE,
		     "show-close-button", TRUE,
		     NULL);

	context_hdr = gtk_widget_get_style_context(frame_gtk->header);
	gtk_style_context_add_class(context_hdr, GTK_STYLE_CLASS_TITLEBAR);
	gtk_style_context_add_class(context_hdr, "default-decoration");

	gtk_window_set_titlebar(GTK_WINDOW(frame_gtk->window), frame_gtk->header);
	gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(frame_gtk->header), TRUE);

	gtk_window_set_resizable(GTK_WINDOW(frame_gtk->window), resizable(frame_gtk));
}

static void
calculate_component_size(struct libdecor_frame_gtk *frame_gtk,
			 enum component component,
			 int *component_x,
			 int *component_y,
			 int *component_width,
			 int *component_height)
{
	struct libdecor_frame *frame = &frame_gtk->frame;
	int content_width, content_height;

	content_width = libdecor_frame_get_content_width(frame);
	content_height = libdecor_frame_get_content_height(frame);

	/* avoid warning when restoring previously turned off decoration */
	const int title_height =
	 	GTK_IS_WIDGET(frame_gtk->header)
	 	? gtk_widget_get_allocated_height(frame_gtk->header) : 0;

	switch (component) {
	case NONE:
		*component_width = 0;
		*component_height = 0;
		return;
	case SHADOW:
		*component_x = -(int)SHADOW_MARGIN;
		*component_y = -(int)(SHADOW_MARGIN+title_height);
		*component_width = content_width + 2 * SHADOW_MARGIN;
		*component_height = content_height
				    + 2 * SHADOW_MARGIN
				    + title_height;
		return;
	case HEADER:
		*component_x = 0;
		/* reuse product of function call above */
		*component_y = - title_height;
		*component_width = gtk_widget_get_allocated_width(frame_gtk->header);
		/* reuse product of function call above */
		*component_height = title_height;
		return;
	}

	abort();
}

static void
array_append(enum header_element **array, size_t *n, enum header_element item)
{
	(*n)++;
	*array = realloc(*array, (*n) * sizeof (enum header_element));
	(*array)[(*n)-1] = item;
}

static void
draw_header_background(struct libdecor_frame_gtk *frame_gtk,
		       cairo_t *cr)
{
	/* background */
	GtkAllocation allocation;
	GtkStyleContext* style;

	gtk_widget_get_allocation(GTK_WIDGET(frame_gtk->header), &allocation);
	style = gtk_widget_get_style_context(frame_gtk->header);
	gtk_render_background(style, cr, allocation.x, allocation.y, allocation.width, allocation.height);
}

static void
draw_header_title(struct libdecor_frame_gtk *frame_gtk,
		  cairo_surface_t *surface)
{
	/* title */
	GtkWidget *label;
	GtkAllocation allocation;
	cairo_surface_t *label_surface = NULL;
	cairo_t *cr;

	label = find_widget_by_type(frame_gtk->header, HEADER_TITLE).widget;
	gtk_widget_get_allocation(label, &allocation);

	/* create subsection in which to draw label */
	label_surface = cairo_surface_create_for_rectangle(
				surface,
				allocation.x, allocation.y,
				allocation.width, allocation.height);
	cr = cairo_create(label_surface);
	gtk_widget_size_allocate(label, &allocation);
	gtk_widget_draw(label, cr);
	cairo_destroy(cr);
	cairo_surface_destroy(label_surface);
}

static void
draw_header_button(struct libdecor_frame_gtk *frame_gtk,
		   cairo_t *cr,
		   cairo_surface_t *surface,
		   enum header_element button_type,
		   enum libdecor_window_state window_state)
{
	struct header_element_data elem;
	GtkWidget *button;
	GtkStyleContext* button_style;
	GtkStateFlags style_state;

	GtkAllocation allocation;

	gchar *icon_name;
	int scale;
	GtkWidget *icon_widget;
	GtkAllocation allocation_icon;
	GtkIconInfo* icon_info;

	double sx, sy;

	gint icon_width, icon_height;

	GdkPixbuf* icon_pixbuf;
	cairo_surface_t* icon_surface;

	gint width = 0, height = 0;

	gint left = 0, top = 0, right = 0, bottom = 0;
	GtkBorder border;

	GtkBorder padding;

	elem = find_widget_by_type(frame_gtk->header, button_type);
	button = elem.widget;
	if (!button)
		return;
	button_style = gtk_widget_get_style_context(button);
	style_state = elem.state;

	/* change style based on window state and focus */
	if (!(window_state & LIBDECOR_WINDOW_STATE_ACTIVE)) {
		style_state |= GTK_STATE_FLAG_BACKDROP;
	}
	if (frame_gtk->hdr_focus.widget == button) {
		style_state |= GTK_STATE_FLAG_PRELIGHT;
		if (frame_gtk->hdr_focus.state & GTK_STATE_FLAG_ACTIVE) {
			style_state |= GTK_STATE_FLAG_ACTIVE;
		}
	}

	/* background */
	gtk_widget_get_clip(button, &allocation);

	gtk_style_context_save(button_style);
	gtk_style_context_set_state(button_style, style_state);
	gtk_render_background(button_style, cr,
			      allocation.x, allocation.y,
			      allocation.width, allocation.height);
	gtk_render_frame(button_style, cr,
			 allocation.x, allocation.y,
			 allocation.width, allocation.height);
	gtk_style_context_restore(button_style);

	/* symbol */
	switch (button_type) {
	case HEADER_MIN:
		icon_name = "window-minimize-symbolic";
		break;
	case HEADER_MAX:
		icon_name = (window_state & LIBDECOR_WINDOW_STATE_MAXIMIZED) ?
				    "window-restore-symbolic" :
				    "window-maximize-symbolic";
		break;
	case HEADER_CLOSE:
		icon_name = "window-close-symbolic";
		break;
	default:
		icon_name = NULL;
		break;
	}

	/* get scale */
	cairo_surface_get_device_scale(surface, &sx, &sy);
	scale = (sx+sy) / 2.0;

	/* get original icon dimensions */
	icon_widget = gtk_bin_get_child(GTK_BIN(button));
	gtk_widget_get_allocation(icon_widget, &allocation_icon);

	/* icon info */
	if (!gtk_icon_size_lookup(GTK_ICON_SIZE_MENU, &icon_width, &icon_height)) {
		icon_width = 16;
		icon_height = 16;
	}
	icon_info = gtk_icon_theme_lookup_icon_for_scale(
		gtk_icon_theme_get_default(), icon_name,
		icon_width, scale, (GtkIconLookupFlags)0);

	/* icon pixel buffer*/
	gtk_style_context_save(button_style);
	gtk_style_context_set_state(button_style, style_state);
	icon_pixbuf = gtk_icon_info_load_symbolic_for_context(
		icon_info, button_style, NULL, NULL);
	icon_surface = gdk_cairo_surface_create_from_pixbuf(icon_pixbuf, scale, NULL);
	gtk_style_context_restore(button_style);

	/* dimensions and position */
	gtk_style_context_get(button_style, gtk_style_context_get_state(button_style),
			      "min-width", &width, "min-height", &height, NULL);

	if (width < icon_width)
		width = icon_width;
	if (height < icon_height)
		height = icon_height;

	gtk_style_context_get_border(button_style, gtk_style_context_get_state(button_style), &border);
	left += border.left;
	right += border.right;
	top += border.top;
	bottom += border.bottom;

	gtk_style_context_get_padding(button_style, gtk_style_context_get_state(button_style), &padding);
	left += padding.left;
	right += padding.right;
	top += padding.top;
	bottom += padding.bottom;

	width += left + right;
	height += top + bottom;

	gtk_render_icon_surface(gtk_widget_get_style_context(icon_widget),
				cr, icon_surface,
				allocation.x + ((width - icon_width) / 2),
				allocation.y + ((height - icon_height) / 2));
	cairo_paint(cr);
	cairo_surface_destroy(icon_surface);
	g_object_unref(icon_pixbuf);
}

static void
draw_header_buttons(struct libdecor_frame_gtk *frame_gtk,
		    cairo_t *cr,
		    cairo_surface_t *surface)
{
	/* buttons */
	enum libdecor_window_state window_state;
	enum header_element *buttons = NULL;
	size_t nbuttons = 0;

	window_state = libdecor_frame_get_window_state(
			       (struct libdecor_frame*)frame_gtk);

	/* set buttons by capability */
	if (minimizable(frame_gtk))
		array_append(&buttons, &nbuttons, HEADER_MIN);
	if (resizable(frame_gtk))
		array_append(&buttons, &nbuttons, HEADER_MAX);
	if (closeable(frame_gtk))
		array_append(&buttons, &nbuttons, HEADER_CLOSE);

	for (size_t i = 0; i < nbuttons; i++) {
		draw_header_button(frame_gtk, cr, surface, buttons[i], window_state);
	} /* loop buttons */
	free(buttons);
}

static void
draw_header(struct libdecor_frame_gtk *frame_gtk,
	    cairo_t *cr,
	    cairo_surface_t *surface)
{
	draw_header_background(frame_gtk, cr);
	draw_header_title(frame_gtk, surface);
	draw_header_buttons(frame_gtk, cr, surface);
}

static void
draw_component_content(struct libdecor_frame_gtk *frame_gtk,
		       struct buffer *buffer,
		       int component_width,
		       int component_height,
		       enum component component)
{
	cairo_surface_t *surface;
	cairo_t *cr;

	/* clear buffer */
	memset(buffer->data, 0, buffer->data_size);

	surface = cairo_image_surface_create_for_data(
			  buffer->data, CAIRO_FORMAT_ARGB32,
			  buffer->buffer_width, buffer->buffer_height,
			  cairo_format_stride_for_width(
				  CAIRO_FORMAT_ARGB32,
				  buffer->buffer_width)
			  );

	cr = cairo_create(surface);

	cairo_surface_set_device_scale(surface, buffer->scale, buffer->scale);

	/* background */
	switch (component) {
	case NONE:
		break;
	case SHADOW:
		render_shadow(cr,
			      frame_gtk->shadow_blur,
			      -(int)SHADOW_MARGIN/2,
			      -(int)SHADOW_MARGIN/2,
			      buffer->width + SHADOW_MARGIN,
			      buffer->height + SHADOW_MARGIN,
			      64,
			      64);
		break;
	case HEADER:
		draw_header(frame_gtk, cr, surface);
		break;
	}

	/* mask the toplevel surface */
	if (component == SHADOW) {
		int component_x, component_y, component_width, component_height;
		calculate_component_size(frame_gtk, component,
					 &component_x, &component_y,
					 &component_width, &component_height);
		cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
		cairo_rectangle(cr, -component_x, -component_y,
				libdecor_frame_get_content_width(
					&frame_gtk->frame),
				libdecor_frame_get_content_height(
					&frame_gtk->frame));
		cairo_fill(cr);
	}

	cairo_destroy(cr);
	cairo_surface_destroy(surface);
}

static void
set_component_input_region(struct libdecor_frame_gtk *frame_gtk,
			   struct border_component *border_component)
{
	if (border_component->type == SHADOW && frame_gtk->shadow_showing) {
		struct wl_region *input_region;
		int component_x;
		int component_y;
		int component_width;
		int component_height;

		calculate_component_size(frame_gtk, border_component->type,
					 &component_x, &component_y,
					 &component_width, &component_height);

		/*
		 * the input region is the outer surface size minus the inner
		 * content size
		 */
		input_region = wl_compositor_create_region(
				       frame_gtk->plugin_gtk->wl_compositor);
		wl_region_add(input_region, 0, 0,
			      component_width, component_height);
		wl_region_subtract(input_region, -component_x, -component_y,
			libdecor_frame_get_content_width(&frame_gtk->frame),
			libdecor_frame_get_content_height(&frame_gtk->frame));
		wl_surface_set_input_region(border_component->wl_surface,
					    input_region);
		wl_region_destroy(input_region);
	}
}

static void
draw_border_component(struct libdecor_frame_gtk *frame_gtk,
		      struct border_component *border_component,
		      enum component component)
{
	struct libdecor_plugin_gtk *plugin_gtk = frame_gtk->plugin_gtk;
	struct buffer *old_buffer;
	struct buffer *buffer = NULL;
	int component_x;
	int component_y;
	int component_width;
	int component_height;
	int scale = border_component->scale;

	if (border_component->wl_surface == NULL)
		return;

	calculate_component_size(frame_gtk, component,
				 &component_x, &component_y,
				 &component_width, &component_height);

	set_component_input_region(frame_gtk, border_component);

	old_buffer = border_component->buffer;
	if (old_buffer) {
		if (!old_buffer->in_use &&
		    old_buffer->buffer_width == component_width * scale &&
		    old_buffer->buffer_height == component_height * scale) {
			buffer = old_buffer;
		} else {
			buffer_free(old_buffer);
			border_component->buffer = NULL;
		}
	}

	if (!buffer)
		buffer = create_shm_buffer(plugin_gtk,
					   component_width,
					   component_height,
					   border_component->opaque,
					   border_component->scale);

	draw_component_content(frame_gtk, buffer,
			       component_width, component_height,
			       component);

	wl_surface_attach(border_component->wl_surface, buffer->wl_buffer, 0, 0);
	wl_surface_set_buffer_scale(border_component->wl_surface, buffer->scale);
	buffer->in_use = true;
	wl_surface_commit(border_component->wl_surface);
	wl_surface_damage_buffer(border_component->wl_surface, 0, 0,
				 component_width * scale,
				 component_height * scale);
	wl_subsurface_set_position(border_component->wl_subsurface,
				   component_x, component_y);

	border_component->buffer = buffer;
}

static void
draw_border(struct libdecor_frame_gtk *frame_gtk)
{
	draw_border_component(frame_gtk, &frame_gtk->shadow, SHADOW);
	frame_gtk->shadow_showing = true;
}

static void
draw_title_bar(struct libdecor_frame_gtk *frame_gtk)
{
	GtkAllocation allocation = {0, 0, frame_gtk->content_width, 0};
	enum libdecor_window_state state;
	GtkStyleContext *style;
	int pref_width;
	int current_min_w, current_min_h, current_max_w, current_max_h, W, H;

	state = libdecor_frame_get_window_state((struct libdecor_frame*)frame_gtk);
	style = gtk_widget_get_style_context(frame_gtk->window);

	if (!(state & LIBDECOR_WINDOW_STATE_ACTIVE)) {
		gtk_widget_set_state_flags(frame_gtk->window, GTK_STATE_FLAG_BACKDROP, true);
	} else {
		gtk_widget_unset_state_flags(frame_gtk->window, GTK_STATE_FLAG_BACKDROP);
	}

	if (libdecor_frame_is_floating(&frame_gtk->frame)) {
		gtk_style_context_remove_class(style, "maximized");
	} else {
		gtk_style_context_add_class(style, "maximized");
	}

	gtk_widget_show_all(frame_gtk->window);

	/* set default width, using an empty title to estimate its smallest admissible value */
	gtk_header_bar_set_title(GTK_HEADER_BAR(frame_gtk->header), "");
	gtk_widget_get_preferred_width(frame_gtk->header, NULL, &pref_width);
	gtk_header_bar_set_title(GTK_HEADER_BAR(frame_gtk->header),
		libdecor_frame_get_title(&frame_gtk->frame));
	libdecor_frame_get_min_content_size(&frame_gtk->frame, &current_min_w, &current_min_h);
	if (current_min_w < pref_width) {
		current_min_w = pref_width;
		libdecor_frame_set_min_content_size(&frame_gtk->frame, current_min_w, current_min_h);
	}
	libdecor_frame_get_max_content_size(&frame_gtk->frame, &current_max_w, &current_max_h);
	if (current_max_w && current_max_w < current_min_w) {
		libdecor_frame_set_max_content_size(&frame_gtk->frame, current_min_w, current_max_h);
	}
	W = libdecor_frame_get_content_width(&frame_gtk->frame);
	H = libdecor_frame_get_content_height(&frame_gtk->frame);
	if (W < current_min_w) {
		W = current_min_w;
		struct libdecor_state *libdecor_state = libdecor_state_new(W, H);
		libdecor_frame_commit(&frame_gtk->frame, libdecor_state, NULL);
		libdecor_state_free(libdecor_state);
		return;
	}
	/* set default height */
	gtk_widget_get_preferred_height(frame_gtk->header, NULL, &allocation.height);

	gtk_widget_size_allocate(frame_gtk->header, &allocation);

	draw_border_component(frame_gtk, &frame_gtk->headerbar, HEADER);
}

static void
draw_decoration(struct libdecor_frame_gtk *frame_gtk)
{
	switch (frame_gtk->decoration_type) {
	case DECORATION_TYPE_NONE:
		if (frame_gtk->link.next != NULL)
			wl_list_remove(&frame_gtk->link);
		if (is_border_surfaces_showing(frame_gtk))
			hide_border_surfaces(frame_gtk);
		hide_border_component(&frame_gtk->headerbar);
		break;
	case DECORATION_TYPE_ALL:
		/* show borders */
		ensure_border_surfaces(frame_gtk);
		draw_border(frame_gtk);
		/* show title bar */
		ensure_title_bar_surfaces(frame_gtk);
		draw_title_bar(frame_gtk);
		/* link frame */
		if (frame_gtk->link.next == NULL)
			wl_list_insert(
				&frame_gtk->plugin_gtk->visible_frame_list,
				&frame_gtk->link);
		break;
	case DECORATION_TYPE_TITLE_ONLY:
		/* hide borders */
		if (is_border_surfaces_showing(frame_gtk))
			hide_border_surfaces(frame_gtk);
		/* show title bar */
		ensure_title_bar_surfaces(frame_gtk);
		draw_title_bar(frame_gtk);
		/* link frame */
		if (frame_gtk->link.next == NULL)
			wl_list_insert(
				&frame_gtk->plugin_gtk->visible_frame_list,
				&frame_gtk->link);
		break;
	}
}

static enum decoration_type
window_state_to_decoration_type(enum libdecor_window_state window_state)
{
	if (window_state & LIBDECOR_WINDOW_STATE_FULLSCREEN)
		return DECORATION_TYPE_NONE;
	else if (window_state & LIBDECOR_WINDOW_STATE_MAXIMIZED ||
		 window_state & LIBDECOR_WINDOW_STATE_TILED_LEFT ||
		 window_state & LIBDECOR_WINDOW_STATE_TILED_RIGHT ||
		 window_state & LIBDECOR_WINDOW_STATE_TILED_TOP ||
		 window_state & LIBDECOR_WINDOW_STATE_TILED_BOTTOM)
		/* title bar, no shadows */
		return DECORATION_TYPE_TITLE_ONLY;
	else
		/* title bar, shadows */
		return DECORATION_TYPE_ALL;
}

static void
libdecor_plugin_gtk_frame_commit(struct libdecor_plugin *plugin,
				   struct libdecor_frame *frame,
				   struct libdecor_state *state,
				   struct libdecor_configuration *configuration)
{
	struct libdecor_frame_gtk *frame_gtk =
		(struct libdecor_frame_gtk *) frame;
	enum libdecor_window_state old_window_state;
	enum libdecor_window_state new_window_state;
	int old_content_width, old_content_height;
	int new_content_width, new_content_height;
	enum decoration_type old_decoration_type;
	enum decoration_type new_decoration_type;

	old_window_state = frame_gtk->window_state;
	new_window_state = libdecor_frame_get_window_state(frame);

	old_content_width = frame_gtk->content_width;
	old_content_height = frame_gtk->content_height;
	new_content_width = libdecor_frame_get_content_width(frame);
	new_content_height = libdecor_frame_get_content_height(frame);

	old_decoration_type = frame_gtk->decoration_type;
	new_decoration_type = window_state_to_decoration_type(new_window_state);

	if (old_decoration_type == new_decoration_type &&
	    old_content_width == new_content_width &&
	    old_content_height == new_content_height &&
	    old_window_state == new_window_state)
		return;

	frame_gtk->content_width = new_content_width;
	frame_gtk->content_height = new_content_height;
	frame_gtk->window_state = new_window_state;
	frame_gtk->decoration_type = new_decoration_type;

	draw_decoration(frame_gtk);

	/* set fixed window size */
	if (!resizable(frame_gtk)) {
		libdecor_frame_set_min_content_size(frame,
						    frame_gtk->content_width,
						    frame_gtk->content_height);
		libdecor_frame_set_max_content_size(frame,
						    frame_gtk->content_width,
						    frame_gtk->content_height);
	}
}

static void
libdecor_plugin_gtk_frame_property_changed(struct libdecor_plugin *plugin,
					     struct libdecor_frame *frame)
{
	struct libdecor_frame_gtk *frame_gtk =
		(struct libdecor_frame_gtk *) frame;
	bool redraw_needed = false;
	const char *new_title;

	/*
	 * when in SSD mode, the window title is not to be managed by GTK;
	 * this is detected by frame_gtk->header not being a proper GTK widget
	 */
	if (!GTK_IS_WIDGET(frame_gtk->header)) return;

	new_title = libdecor_frame_get_title(frame);
	if (!streq(frame_gtk->title, new_title))
		redraw_needed = true;
	free(frame_gtk->title);
	frame_gtk->title = NULL;
	if (new_title)
		frame_gtk->title = strdup(new_title);

	if (frame_gtk->capabilities != libdecor_frame_get_capabilities(frame)) {
		frame_gtk->capabilities = libdecor_frame_get_capabilities(frame);
		redraw_needed = true;
	}

	if (redraw_needed) {
		draw_decoration(frame_gtk);
		libdecor_frame_toplevel_commit(frame);
	}
}

static void
update_component_focus(struct libdecor_frame_gtk *frame_gtk,
		       struct wl_surface *surface,
		       struct seat *seat)
{
	static struct border_component *border_component;
	static struct border_component *child_component;
	static struct border_component *focus_component;

	border_component = get_component_for_surface(frame_gtk, surface);

	focus_component = border_component;
	wl_list_for_each(child_component, &border_component->child_components, link) {
		int component_x = 0, component_y = 0;
		int component_width = 0, component_height = 0;

		calculate_component_size(frame_gtk, child_component->type,
					 &component_x, &component_y,
					 &component_width, &component_height);
		if (seat->pointer_x >= component_x &&
		    seat->pointer_x < component_x + component_width &&
		    seat->pointer_y >= component_y &&
		    seat->pointer_y < component_y + component_height) {
			focus_component = child_component;
			break;
		}
	}

	if (frame_gtk->grab)
		frame_gtk->active = frame_gtk->grab;
	else
		frame_gtk->active = focus_component;
	frame_gtk->focus = focus_component;

}

static void
sync_active_component(struct libdecor_frame_gtk *frame_gtk,
		      struct seat *seat)
{
	struct border_component *old_active;

	if (!seat->pointer_focus)
		return;

	old_active = frame_gtk->active;
	update_component_focus(frame_gtk, seat->pointer_focus, seat);
	if (old_active != frame_gtk->active) {
		draw_decoration(frame_gtk);
		libdecor_frame_toplevel_commit(&frame_gtk->frame);
	}

	if (update_local_cursor(seat))
		send_cursor(seat);
}

static void
synthesize_pointer_enter(struct seat *seat)
{
	struct wl_surface *surface;
	struct libdecor_frame_gtk *frame_gtk;

	surface = seat->pointer_focus;
	if (!surface)
		return;

	frame_gtk = wl_surface_get_user_data(surface);
	if (!frame_gtk)
		return;

	update_component_focus(frame_gtk, seat->pointer_focus, seat);
	frame_gtk->grab = NULL;

	/* update decorations */
	if (frame_gtk->active) {
		draw_decoration(frame_gtk);
		libdecor_frame_toplevel_commit(&frame_gtk->frame);
	}

	update_local_cursor(seat);
	send_cursor(seat);
}

static void
synthesize_pointer_leave(struct seat *seat)
{
	struct wl_surface *surface;
	struct libdecor_frame_gtk *frame_gtk;

	surface = seat->pointer_focus;
	if (!surface)
		return;

	frame_gtk = wl_surface_get_user_data(surface);
	if (!frame_gtk)
		return;

	if (!frame_gtk->active)
		return;

	frame_gtk->active = NULL;
	draw_decoration(frame_gtk);
	libdecor_frame_toplevel_commit(&frame_gtk->frame);
	update_local_cursor(seat);
}

static void
libdecor_plugin_gtk_frame_popup_grab(struct libdecor_plugin *plugin,
				     struct libdecor_frame *frame,
				     const char *seat_name)
{
	struct libdecor_frame_gtk *frame_gtk =
		(struct libdecor_frame_gtk *) frame;
	struct libdecor_plugin_gtk *plugin_gtk = frame_gtk->plugin_gtk;
	struct seat *seat;

	wl_list_for_each(seat, &plugin_gtk->seat_list, link) {
		if (streq(seat->name, seat_name)) {
			if (seat->grabbed) {
				fprintf(stderr, "libdecor-WARNING: Application "
					"tried to grab seat twice\n");
			}
			synthesize_pointer_leave(seat);
			seat->grabbed = true;
			return;
		}
	}

	fprintf(stderr,
		"libdecor-WARNING: Application tried to grab unknown seat\n");
}

static void
libdecor_plugin_gtk_frame_popup_ungrab(struct libdecor_plugin *plugin,
				       struct libdecor_frame *frame,
				       const char *seat_name)
{
	struct libdecor_frame_gtk *frame_gtk =
		(struct libdecor_frame_gtk *) frame;
	struct libdecor_plugin_gtk *plugin_gtk = frame_gtk->plugin_gtk;
	struct seat *seat;

	wl_list_for_each(seat, &plugin_gtk->seat_list, link) {
		if (streq(seat->name, seat_name)) {
			if (!seat->grabbed) {
				fprintf(stderr, "libdecor-WARNING: Application "
					"tried to ungrab seat twice\n");
			}
			seat->grabbed = false;
			synthesize_pointer_enter(seat);
			sync_active_component(frame_gtk, seat);
			return;
		}
	}

	fprintf(stderr,
		"libdecor-WARNING: Application tried to ungrab unknown seat\n");
}

static bool
libdecor_plugin_gtk_frame_get_border_size(struct libdecor_plugin *plugin,
					  struct libdecor_frame *frame,
					  struct libdecor_configuration *configuration,
					  int *left,
					  int *right,
					  int *top,
					  int *bottom)
{
	enum libdecor_window_state window_state;

	if (configuration) {
		if (!libdecor_configuration_get_window_state(
			    configuration, &window_state))
			return false;
	} else {
		window_state = libdecor_frame_get_window_state(frame);
	}

	if (left)
		*left = 0;
	if (right)
		*right = 0;
	if (bottom)
		*bottom = 0;
	if (top) {
		GtkWidget *header = ((struct libdecor_frame_gtk *)frame)->header;
		enum decoration_type type = window_state_to_decoration_type(window_state);

		/* avoid warnings after decoration has been turned off */
		if (GTK_IS_WIDGET(header) && (type != DECORATION_TYPE_NONE)) {
			/* Redraw title bar to ensure size will be up-to-date */
			if (configuration && type == DECORATION_TYPE_TITLE_ONLY)
				draw_title_bar((struct libdecor_frame_gtk *) frame);
			*top = gtk_widget_get_allocated_height(header);
		} else {
			*top = 0;
		}
	}

	return true;
}

static struct libdecor_plugin_interface gtk_plugin_iface = {
	.destroy = libdecor_plugin_gtk_destroy,
	.get_fd = libdecor_plugin_gtk_get_fd,
	.dispatch = libdecor_plugin_gtk_dispatch,

	.frame_new = libdecor_plugin_gtk_frame_new,
	.frame_free = libdecor_plugin_gtk_frame_free,
	.frame_commit = libdecor_plugin_gtk_frame_commit,
	.frame_property_changed = libdecor_plugin_gtk_frame_property_changed,

	.frame_popup_grab = libdecor_plugin_gtk_frame_popup_grab,
	.frame_popup_ungrab = libdecor_plugin_gtk_frame_popup_ungrab,

	.frame_get_border_size = libdecor_plugin_gtk_frame_get_border_size,
};

static void
init_wl_compositor(struct libdecor_plugin_gtk *plugin_gtk,
		   uint32_t id,
		   uint32_t version)
{
	plugin_gtk->wl_compositor =
		wl_registry_bind(plugin_gtk->wl_registry,
				 id, &wl_compositor_interface,
				 MIN(version, 4));
}

static void
init_wl_subcompositor(struct libdecor_plugin_gtk *plugin_gtk,
		      uint32_t id,
		      uint32_t version)
{
	plugin_gtk->wl_subcompositor =
		wl_registry_bind(plugin_gtk->wl_registry,
				 id, &wl_subcompositor_interface, 1);
}

static void
shm_format(void *user_data,
	   struct wl_shm *wl_shm,
	   uint32_t format)
{
	struct libdecor_plugin_gtk *plugin_gtk = user_data;

	if (format == WL_SHM_FORMAT_ARGB8888)
		plugin_gtk->has_argb = true;
}

struct wl_shm_listener shm_listener = {
	shm_format
};

static void
shm_callback(void *user_data,
	     struct wl_callback *callback,
	     uint32_t time)
{
	struct libdecor_plugin_gtk *plugin_gtk = user_data;
	struct libdecor *context = plugin_gtk->context;

	wl_callback_destroy(callback);
	plugin_gtk->globals_callback_shm = NULL;

	if (!plugin_gtk->has_argb) {
		libdecor_notify_plugin_error(
				context,
				LIBDECOR_ERROR_COMPOSITOR_INCOMPATIBLE,
				"Compositor is missing required shm format");
		return;
	}

	libdecor_notify_plugin_ready(context);
}

static const struct wl_callback_listener shm_callback_listener = {
	shm_callback
};

static void
init_wl_shm(struct libdecor_plugin_gtk *plugin_gtk,
	    uint32_t id,
	    uint32_t version)
{
	struct libdecor *context = plugin_gtk->context;
	struct wl_display *wl_display = libdecor_get_wl_display(context);

	plugin_gtk->wl_shm =
		wl_registry_bind(plugin_gtk->wl_registry,
				 id, &wl_shm_interface, 1);
	wl_shm_add_listener(plugin_gtk->wl_shm, &shm_listener, plugin_gtk);

	plugin_gtk->globals_callback_shm = wl_display_sync(wl_display);
	wl_callback_add_listener(plugin_gtk->globals_callback_shm,
				 &shm_callback_listener,
				 plugin_gtk);
}

static void
cursor_surface_enter(void *data,
		     struct wl_surface *wl_surface,
		     struct wl_output *wl_output)
{
	struct seat *seat = data;

	if (own_output(wl_output)) {
		struct cursor_output *cursor_output;
		cursor_output = zalloc(sizeof *cursor_output);
		cursor_output->output = wl_output_get_user_data(wl_output);
		wl_list_insert(&seat->cursor_outputs, &cursor_output->link);
		if (update_local_cursor(seat))
			send_cursor(seat);
	}
}

static void
cursor_surface_leave(void *data,
		     struct wl_surface *wl_surface,
		     struct wl_output *wl_output)
{
	struct seat *seat = data;

	if (own_output(wl_output)) {
		struct cursor_output *cursor_output, *tmp;
		wl_list_for_each_safe(cursor_output, tmp, &seat->cursor_outputs, link) {
			if (cursor_output->output->wl_output == wl_output) {
				wl_list_remove(&cursor_output->link);
				free(cursor_output);
			}
		}

		if (update_local_cursor(seat))
			send_cursor(seat);
	}
}

static struct wl_surface_listener cursor_surface_listener = {
	cursor_surface_enter,
	cursor_surface_leave,
};

static void
ensure_cursor_surface(struct seat *seat)
{
	struct wl_compositor *wl_compositor = seat->plugin_gtk->wl_compositor;

	if (seat->cursor_surface)
		return;

	seat->cursor_surface = wl_compositor_create_surface(wl_compositor);
	wl_surface_add_listener(seat->cursor_surface,
				&cursor_surface_listener, seat);
}

static bool
ensure_cursor_theme(struct seat *seat)
{
	struct libdecor_plugin_gtk *plugin_gtk = seat->plugin_gtk;
	int scale = 1;
	struct wl_cursor_theme *theme;
	struct cursor_output *cursor_output;

	wl_list_for_each(cursor_output, &seat->cursor_outputs, link) {
		scale = MAX(scale, cursor_output->output->scale);
	}

	if (seat->cursor_theme && seat->cursor_scale == scale)
		return false;

	seat->cursor_scale = scale;
	theme = wl_cursor_theme_load(plugin_gtk->cursor_theme_name,
				     plugin_gtk->cursor_size * scale,
				     plugin_gtk->wl_shm);
	if (theme == NULL)
		return false;

	if (seat->cursor_theme)
		wl_cursor_theme_destroy(seat->cursor_theme);

	seat->cursor_theme = theme;

	for (unsigned int i = 0; i < ARRAY_LENGTH(cursor_names); i++) {
		seat->cursors[i] = wl_cursor_theme_get_cursor(
						   seat->cursor_theme,
						   cursor_names[i]);
	}

	seat->cursor_left_ptr = wl_cursor_theme_get_cursor(seat->cursor_theme,
							   "left_ptr");
	seat->current_cursor = seat->cursor_left_ptr;

	return true;
}

enum libdecor_resize_edge
component_edge(const struct border_component *cmpnt,
	       const int pointer_x,
	       const int pointer_y,
	       const int margin)
{
	const bool top = pointer_y < margin * 2;
	const bool bottom = pointer_y > (cmpnt->buffer->height - margin * 2);
	const bool left = pointer_x < margin * 2;
	const bool right = pointer_x > (cmpnt->buffer->width - margin * 2);

	if (top) {
		if (left)
			return LIBDECOR_RESIZE_EDGE_TOP_LEFT;
		else if (right)
			return LIBDECOR_RESIZE_EDGE_TOP_RIGHT;
		else
			return LIBDECOR_RESIZE_EDGE_TOP;
	} else if (bottom) {
		if (left)
			return LIBDECOR_RESIZE_EDGE_BOTTOM_LEFT;
		else if (right)
			return LIBDECOR_RESIZE_EDGE_BOTTOM_RIGHT;
		else
			return LIBDECOR_RESIZE_EDGE_BOTTOM;
	} else if (left) {
		return LIBDECOR_RESIZE_EDGE_LEFT;
	} else if (right) {
		return LIBDECOR_RESIZE_EDGE_RIGHT;
	} else {
		return LIBDECOR_RESIZE_EDGE_NONE;
	}
}

static bool
update_local_cursor(struct seat *seat)
{
	if (!seat->pointer_focus) {
		seat->current_cursor = seat->cursor_left_ptr;
		return false;
	}

	if (!own_surface(seat->pointer_focus))
		return false;

	struct libdecor_frame_gtk *frame_gtk =
			wl_surface_get_user_data(seat->pointer_focus);
	struct wl_cursor *wl_cursor = NULL;

	if (!frame_gtk || !frame_gtk->active) {
		seat->current_cursor = seat->cursor_left_ptr;
		return false;
	}

	bool theme_updated = ensure_cursor_theme(seat);

	if (frame_gtk->active->type == SHADOW &&
	    is_border_surfaces_showing(frame_gtk) &&
	    resizable(frame_gtk)) {
		enum libdecor_resize_edge edge;
		edge = component_edge(frame_gtk->active,
				      seat->pointer_x,
				      seat->pointer_y, SHADOW_MARGIN);

		if (edge != LIBDECOR_RESIZE_EDGE_NONE)
			wl_cursor = seat->cursors[edge - 1];
	} else {
		wl_cursor = seat->cursor_left_ptr;
	}

	if (seat->current_cursor != wl_cursor) {
		seat->current_cursor = wl_cursor;
		return true;
	}

	return theme_updated;
}

static void
send_cursor(struct seat *seat)
{
	struct wl_cursor_image *image;
	struct wl_buffer *buffer;

	if (seat->pointer_focus == NULL || seat->current_cursor == NULL)
		return;

	image = seat->current_cursor->images[0];
	buffer = wl_cursor_image_get_buffer(image);
	wl_surface_attach(seat->cursor_surface, buffer, 0, 0);
	wl_surface_set_buffer_scale(seat->cursor_surface, seat->cursor_scale);
	wl_surface_damage_buffer(seat->cursor_surface, 0, 0,
				 image->width * seat->cursor_scale,
				 image->height * seat->cursor_scale);
	wl_surface_commit(seat->cursor_surface);
	wl_pointer_set_cursor(seat->wl_pointer, seat->serial,
			      seat->cursor_surface,
			      image->hotspot_x / seat->cursor_scale,
			      image->hotspot_y / seat->cursor_scale);
}

static void
pointer_enter(void *data,
	      struct wl_pointer *wl_pointer,
	      uint32_t serial,
	      struct wl_surface *surface,
	      wl_fixed_t surface_x,
	      wl_fixed_t surface_y)
{
	if (!surface)
		return;

	struct seat *seat = data;
	struct libdecor_frame_gtk *frame_gtk;

	if (!own_surface(surface))
		return;

	frame_gtk = wl_surface_get_user_data(surface);

	ensure_cursor_surface(seat);

	seat->pointer_x = wl_fixed_to_int(surface_x);
	seat->pointer_y = wl_fixed_to_int(surface_y);
	seat->serial = serial;
	seat->pointer_focus = surface;

	if (!frame_gtk)
		return;

	frame_gtk->active = get_component_for_surface(frame_gtk, surface);

	/* update decorations */
	if (frame_gtk->active) {
		draw_decoration(frame_gtk);
		libdecor_frame_toplevel_commit(&frame_gtk->frame);
	}

	update_local_cursor(seat);
	send_cursor(seat);
}

static void
pointer_leave(void *data,
	      struct wl_pointer *wl_pointer,
	      uint32_t serial,
	      struct wl_surface *surface)
{
	if (!surface)
		return;

	struct seat *seat = data;
	struct libdecor_frame_gtk *frame_gtk;

	if (!own_surface(surface))
		return;

	frame_gtk = wl_surface_get_user_data(surface);

	seat->pointer_focus = NULL;
	if (frame_gtk) {
		frame_gtk->titlebar_gesture.state =
			TITLEBAR_GESTURE_STATE_INIT;
		frame_gtk->titlebar_gesture.first_pressed_button = 0;

		frame_gtk->active = NULL;
		frame_gtk->hdr_focus.widget = NULL;
		frame_gtk->hdr_focus.type = HEADER_NONE;
		draw_decoration(frame_gtk);
		libdecor_frame_toplevel_commit(&frame_gtk->frame);
		update_local_cursor(seat);
	}
}

static void
pointer_motion(void *data,
	       struct wl_pointer *wl_pointer,
	       uint32_t time,
	       wl_fixed_t surface_x,
	       wl_fixed_t surface_y)
{
	struct seat *seat = data;
	struct libdecor_frame_gtk *frame_gtk;
	struct header_element_data new_focus;

	if (!seat->pointer_focus || !own_surface(seat->pointer_focus))
		return;

	seat->pointer_x = wl_fixed_to_int(surface_x);
	seat->pointer_y = wl_fixed_to_int(surface_y);
	if (update_local_cursor(seat))
		send_cursor(seat);

	frame_gtk = wl_surface_get_user_data(seat->pointer_focus);
	/* avoid warnings after decoration has been turned off */
	if (!GTK_IS_WIDGET(frame_gtk->header) || frame_gtk->active->type != HEADER) {
		frame_gtk->hdr_focus.type = HEADER_NONE;
	}

	new_focus =  get_header_focus(GTK_HEADER_BAR(frame_gtk->header),
				      seat->pointer_x, seat->pointer_y);

	/* only update if widget change so that we keep the state */
	if (frame_gtk->hdr_focus.widget != new_focus.widget) {
		frame_gtk->hdr_focus = new_focus;
	}
	frame_gtk->hdr_focus.state |= GTK_STATE_FLAG_PRELIGHT;
	/* redraw with updated button visuals */
	draw_title_bar(frame_gtk);
	libdecor_frame_toplevel_commit(&frame_gtk->frame);

	switch (frame_gtk->titlebar_gesture.state) {
	case TITLEBAR_GESTURE_STATE_BUTTON_PRESSED:
		if (frame_gtk->titlebar_gesture.first_pressed_button == BTN_LEFT) {
			if (ABS ((double) seat->pointer_x -
				 (double) frame_gtk->titlebar_gesture.pressed_x) >
			    frame_gtk->plugin_gtk->drag_threshold ||
			    ABS ((double) seat->pointer_y -
				 (double) frame_gtk->titlebar_gesture.pressed_y) >
			    frame_gtk->plugin_gtk->drag_threshold) {
				libdecor_frame_move(&frame_gtk->frame,
						    seat->wl_seat,
						    frame_gtk->titlebar_gesture.pressed_serial);
			}
		}
	case TITLEBAR_GESTURE_STATE_INIT:
	case TITLEBAR_GESTURE_STATE_CONSUMED:
	case TITLEBAR_GESTURE_STATE_DISCARDED:
		break;
	}
}

static void
handle_button_on_shadow(struct libdecor_frame_gtk *frame_gtk,
			struct seat *seat,
			uint32_t serial,
			uint32_t time,
			uint32_t button,
			uint32_t state)
{
	enum libdecor_resize_edge edge = LIBDECOR_RESIZE_EDGE_NONE;

	edge = component_edge(frame_gtk->active,
			      seat->pointer_x,
			      seat->pointer_y,
			      SHADOW_MARGIN);

	if (edge != LIBDECOR_RESIZE_EDGE_NONE && resizable(frame_gtk)) {
		libdecor_frame_resize(&frame_gtk->frame,
				      seat->wl_seat,
				      serial,
				      edge);
	}
}

enum titlebar_gesture {
	TITLEBAR_GESTURE_DOUBLE_CLICK,
	TITLEBAR_GESTURE_MIDDLE_CLICK,
	TITLEBAR_GESTURE_RIGHT_CLICK,
};

static void
handle_titlebar_gesture(struct libdecor_frame_gtk *frame_gtk,
			struct seat *seat,
			uint32_t serial,
			enum titlebar_gesture gesture)
{
	switch (gesture) {
	case TITLEBAR_GESTURE_DOUBLE_CLICK:
		toggle_maximized(&frame_gtk->frame);
		break;
	case TITLEBAR_GESTURE_MIDDLE_CLICK:
		break;
	case TITLEBAR_GESTURE_RIGHT_CLICK:
		{
		const int title_height = gtk_widget_get_allocated_height(frame_gtk->header);
		libdecor_frame_show_window_menu(&frame_gtk->frame,
						seat->wl_seat,
						serial,
						seat->pointer_x,
						seat->pointer_y
						-title_height);
		}
		break;
	}
}

static void
handle_button_on_header(struct libdecor_frame_gtk *frame_gtk,
			struct seat *seat,
			uint32_t serial,
			uint32_t time,
			uint32_t button,
			uint32_t state)
{
	switch (frame_gtk->titlebar_gesture.state) {
	case TITLEBAR_GESTURE_STATE_INIT:
		if (state != WL_POINTER_BUTTON_STATE_PRESSED)
			return;

		if (button == BTN_RIGHT) {
			handle_titlebar_gesture(frame_gtk,
						seat,
						serial,
						TITLEBAR_GESTURE_RIGHT_CLICK);
			frame_gtk->titlebar_gesture.state =
				TITLEBAR_GESTURE_STATE_CONSUMED;
		} else {
			if (button == BTN_LEFT &&
			    frame_gtk->titlebar_gesture.first_pressed_button == BTN_LEFT &&
			    time - frame_gtk->titlebar_gesture.first_pressed_time <
			    (uint32_t) frame_gtk->plugin_gtk->double_click_time_ms) {
				handle_titlebar_gesture(frame_gtk,
							seat,
							serial,
							TITLEBAR_GESTURE_DOUBLE_CLICK);
				frame_gtk->titlebar_gesture.state =
					TITLEBAR_GESTURE_STATE_CONSUMED;
			} else {
				frame_gtk->titlebar_gesture.first_pressed_button = button;
				frame_gtk->titlebar_gesture.first_pressed_time = time;
				frame_gtk->titlebar_gesture.pressed_x = seat->pointer_x;
				frame_gtk->titlebar_gesture.pressed_y = seat->pointer_y;
				frame_gtk->titlebar_gesture.pressed_serial = serial;
				frame_gtk->titlebar_gesture.state =
					TITLEBAR_GESTURE_STATE_BUTTON_PRESSED;
			}
		}

		frame_gtk->titlebar_gesture.button_pressed_count = 1;

		switch (frame_gtk->hdr_focus.type) {
		case HEADER_MIN:
		case HEADER_MAX:
		case HEADER_CLOSE:
			frame_gtk->hdr_focus.state |= GTK_STATE_FLAG_ACTIVE;
			draw_title_bar(frame_gtk);
			libdecor_frame_toplevel_commit(&frame_gtk->frame);
			break;
		default:
			break;
		}

		break;
	case TITLEBAR_GESTURE_STATE_BUTTON_PRESSED:
		if (state == WL_POINTER_BUTTON_STATE_PRESSED) {
			frame_gtk->titlebar_gesture.state =
				TITLEBAR_GESTURE_STATE_DISCARDED;
			frame_gtk->titlebar_gesture.button_pressed_count++;
		} else {
			frame_gtk->titlebar_gesture.button_pressed_count--;

			if (frame_gtk->titlebar_gesture.button_pressed_count == 0) {
				frame_gtk->titlebar_gesture.state =
					TITLEBAR_GESTURE_STATE_INIT;
				if (frame_gtk->titlebar_gesture.first_pressed_button == button &&
				    button == BTN_LEFT) {
					libdecor_frame_ref(&frame_gtk->frame);
					switch (frame_gtk->hdr_focus.type) {
					case HEADER_MIN:
						if (minimizable(frame_gtk))
							libdecor_frame_set_minimized(
								&frame_gtk->frame);
						break;
					case HEADER_MAX:
						toggle_maximized(&frame_gtk->frame);
						break;
					case HEADER_CLOSE:
						if (closeable(frame_gtk)) {
							libdecor_frame_close(
								&frame_gtk->frame);
							seat->pointer_focus = NULL;
						}
						break;
					default:
						break;
					}

					frame_gtk->hdr_focus.state &= ~GTK_STATE_FLAG_ACTIVE;
					if (GTK_IS_WIDGET(frame_gtk->header)) {
						draw_title_bar(frame_gtk);
						libdecor_frame_toplevel_commit(&frame_gtk->frame);
					}
					libdecor_frame_unref(&frame_gtk->frame);
				}
			} else {
				frame_gtk->hdr_focus.state &= ~GTK_STATE_FLAG_ACTIVE;
				if (GTK_IS_WIDGET(frame_gtk->header)) {
					draw_title_bar(frame_gtk);
					libdecor_frame_toplevel_commit(&frame_gtk->frame);
				}
			}

		}
		break;
	case TITLEBAR_GESTURE_STATE_CONSUMED:
	case TITLEBAR_GESTURE_STATE_DISCARDED:
		if (state == WL_POINTER_BUTTON_STATE_PRESSED) {
			frame_gtk->titlebar_gesture.button_pressed_count++;
		} else {
			frame_gtk->titlebar_gesture.button_pressed_count--;
			if (frame_gtk->titlebar_gesture.button_pressed_count == 0) {
				frame_gtk->titlebar_gesture.state =
					TITLEBAR_GESTURE_STATE_INIT;
				frame_gtk->titlebar_gesture.first_pressed_button = 0;
			}
		}
		break;
	}
}

static void
pointer_button(void *data,
	       struct wl_pointer *wl_pointer,
	       uint32_t serial,
	       uint32_t time,
	       uint32_t button,
	       uint32_t state)
{
	struct seat *seat = data;
	struct libdecor_frame_gtk *frame_gtk;

	if (!seat->pointer_focus || !own_surface(seat->pointer_focus))
		return;

	frame_gtk = wl_surface_get_user_data(seat->pointer_focus);
	if (!frame_gtk)
		return;

	switch (frame_gtk->active->type) {
	case SHADOW:
		handle_button_on_shadow (frame_gtk, seat, serial, time, button, state);
		break;
	case HEADER:
		handle_button_on_header (frame_gtk, seat, serial, time, button, state);
		break;
	default:
		break;
	}
}

static void
pointer_axis(void *data,
	     struct wl_pointer *wl_pointer,
	     uint32_t time,
	     uint32_t axis,
	     wl_fixed_t value)
{
}

static struct wl_pointer_listener pointer_listener = {
	pointer_enter,
	pointer_leave,
	pointer_motion,
	pointer_button,
	pointer_axis
};

static void
update_touch_focus(struct seat *seat,
		   struct libdecor_frame_gtk *frame_gtk,
		   wl_fixed_t x,
		   wl_fixed_t y)
{
	/* avoid warnings after decoration has been turned off */
	if (GTK_IS_WIDGET(frame_gtk->header) && frame_gtk->touch_active->type == HEADER) {
		struct header_element_data new_focus = get_header_focus(
					  GTK_HEADER_BAR(frame_gtk->header),
					  wl_fixed_to_int(x), wl_fixed_to_int(y));
		/* only update if widget change so that we keep the state */
		if (frame_gtk->hdr_focus.widget != new_focus.widget) {
			frame_gtk->hdr_focus = new_focus;
		}
		frame_gtk->hdr_focus.state |= GTK_STATE_FLAG_PRELIGHT;
		/* redraw with updated button visuals */
		draw_title_bar(frame_gtk);
		libdecor_frame_toplevel_commit(&frame_gtk->frame);
	} else {
		frame_gtk->hdr_focus.type = HEADER_NONE;
	}
}

static void
touch_down(void *data,
	   struct wl_touch *wl_touch,
	   uint32_t serial,
	   uint32_t time,
	   struct wl_surface *surface,
	   int32_t id,
	   wl_fixed_t x,
	   wl_fixed_t y)
{
	struct seat *seat = data;
	struct libdecor_frame_gtk *frame_gtk;

	if (!surface || !own_surface(surface))
		return;

	frame_gtk = wl_surface_get_user_data(surface);
	if (!frame_gtk)
		return;

	seat->touch_focus = surface;
	frame_gtk->touch_active = get_component_for_surface(frame_gtk, surface);

	if (!frame_gtk->touch_active)
		return;

	update_touch_focus(seat, frame_gtk, x, y);

	/* update decorations */
	draw_decoration(frame_gtk);
	libdecor_frame_toplevel_commit(&frame_gtk->frame);

	enum libdecor_resize_edge edge =
		LIBDECOR_RESIZE_EDGE_NONE;
	switch (frame_gtk->touch_active->type) {
	case SHADOW:
		edge = component_edge(frame_gtk->touch_active,
						      wl_fixed_to_int(x),
						      wl_fixed_to_int(y),
						      SHADOW_MARGIN);
		break;
	case HEADER:
		switch (frame_gtk->hdr_focus.type) {
		case HEADER_MIN:
		case HEADER_MAX:
		case HEADER_CLOSE:
			frame_gtk->hdr_focus.state |= GTK_STATE_FLAG_ACTIVE;
			draw_title_bar(frame_gtk);
			libdecor_frame_toplevel_commit(&frame_gtk->frame);
			break;
		default:
			if (time - seat->touch_down_time_stamp <
				(uint32_t)frame_gtk->plugin_gtk->double_click_time_ms) {
				toggle_maximized(&frame_gtk->frame);
			}
			else if (moveable(frame_gtk)) {
				seat->touch_down_time_stamp = time;
				libdecor_frame_move(&frame_gtk->frame,
							seat->wl_seat,
							serial);
			}
			break;
		}
		break;
	default:
		break;
	}
	if (edge != LIBDECOR_RESIZE_EDGE_NONE &&
		resizable(frame_gtk)) {
		libdecor_frame_resize(
			&frame_gtk->frame,
			seat->wl_seat,
			serial,
			edge);
	}
}

static void
touch_up(void *data,
	 struct wl_touch *wl_touch,
	 uint32_t serial,
	 uint32_t time,
	 int32_t id)
{
	struct seat *seat = data;
	struct libdecor_frame_gtk *frame_gtk;

	if (!seat->touch_focus || !own_surface(seat->touch_focus))
		return;

	frame_gtk = wl_surface_get_user_data(seat->touch_focus);
	if (!frame_gtk)
		return;

	if (!frame_gtk->touch_active)
		return;

	switch (frame_gtk->touch_active->type) {
	case HEADER:
		libdecor_frame_ref(&frame_gtk->frame);
		switch (frame_gtk->hdr_focus.type) {
		case HEADER_MIN:
			if (minimizable(frame_gtk)) {
				libdecor_frame_set_minimized(
					&frame_gtk->frame);
			}
			break;
		case HEADER_MAX:
			toggle_maximized(&frame_gtk->frame);
			break;
		case HEADER_CLOSE:
			if (closeable(frame_gtk)) {
					libdecor_frame_close(
						&frame_gtk->frame);
					seat->touch_focus = NULL;
			}
			break;
		default:
			break;
		}
		/* unset active/clicked state once released */
		frame_gtk->hdr_focus.state &= ~GTK_STATE_FLAG_ACTIVE;
		if (GTK_IS_WIDGET(frame_gtk->header)) {
			draw_title_bar(frame_gtk);
			libdecor_frame_toplevel_commit(&frame_gtk->frame);
		}
		libdecor_frame_unref(&frame_gtk->frame);
		break;
	default:
		break;
	}

	seat->touch_focus = NULL;
	frame_gtk->touch_active = NULL;
	frame_gtk->hdr_focus.widget = NULL;
	frame_gtk->hdr_focus.type = HEADER_NONE;
	draw_decoration(frame_gtk);
	libdecor_frame_toplevel_commit(&frame_gtk->frame);
}

static void
touch_motion(void *data,
	     struct wl_touch *wl_touch,
	     uint32_t time,
	     int32_t id,
	     wl_fixed_t x,
	     wl_fixed_t y)
{
	struct seat *seat = data;
	struct libdecor_frame_gtk *frame_gtk;

	if (!seat->touch_focus || !own_surface(seat->touch_focus))
		return;

	frame_gtk = wl_surface_get_user_data(seat->touch_focus);
	if (!frame_gtk)
		return;

	update_touch_focus(seat, frame_gtk, x, y);
}

static void
touch_frame(void *data,
	    struct wl_touch *wl_touch)
{
}

static void
touch_cancel(void *data,
	     struct wl_touch *wl_touch)
{
}

static struct wl_touch_listener touch_listener = {
	touch_down,
	touch_up,
	touch_motion,
	touch_frame,
	touch_cancel
};

static void
seat_capabilities(void *data,
		  struct wl_seat *wl_seat,
		  uint32_t capabilities)
{
	struct seat *seat = data;

	if ((capabilities & WL_SEAT_CAPABILITY_POINTER) &&
	    !seat->wl_pointer) {
		seat->wl_pointer = wl_seat_get_pointer(wl_seat);
		wl_pointer_add_listener(seat->wl_pointer,
					&pointer_listener, seat);
	} else if (!(capabilities & WL_SEAT_CAPABILITY_POINTER) &&
		   seat->wl_pointer) {
		wl_pointer_release(seat->wl_pointer);
		seat->wl_pointer = NULL;
	}

	if ((capabilities & WL_SEAT_CAPABILITY_TOUCH) &&
		!seat->wl_touch) {
		seat->wl_touch = wl_seat_get_touch(wl_seat);
		wl_touch_add_listener(seat->wl_touch,
					&touch_listener, seat);
	} else if (!(capabilities & WL_SEAT_CAPABILITY_TOUCH) &&
		   seat->wl_touch) {
		wl_touch_release(seat->wl_touch);
		seat->wl_touch = NULL;
	}
}

static void
seat_name(void *data,
	  struct wl_seat *wl_seat,
	  const char *name)
{
	/* avoid warning messages when opening/closing popup window */
	struct seat *seat = (struct seat*)data;
	seat->name = strdup(name);
}

static struct wl_seat_listener seat_listener = {
	seat_capabilities,
	seat_name
};

static void
init_wl_seat(struct libdecor_plugin_gtk *plugin_gtk,
	     uint32_t id,
	     uint32_t version)
{
	struct seat *seat;

	if (version < 3) {
		libdecor_notify_plugin_error(
				plugin_gtk->context,
				LIBDECOR_ERROR_COMPOSITOR_INCOMPATIBLE,
				"%s version 3 required but only version %i is available\n",
				wl_seat_interface.name,
				version);
	}

	seat = zalloc(sizeof *seat);
	seat->cursor_scale = 1;
	seat->plugin_gtk = plugin_gtk;
	wl_list_init(&seat->cursor_outputs);
	wl_list_insert(&plugin_gtk->seat_list, &seat->link);
	seat->wl_seat =
		wl_registry_bind(plugin_gtk->wl_registry,
				 id, &wl_seat_interface, 3);
	wl_seat_add_listener(seat->wl_seat, &seat_listener, seat);
}

static void
output_geometry(void *data,
		struct wl_output *wl_output,
		int32_t x,
		int32_t y,
		int32_t physical_width,
		int32_t physical_height,
		int32_t subpixel,
		const char *make,
		const char *model,
		int32_t transform)
{
}

static void
output_mode(void *data,
	    struct wl_output *wl_output,
	    uint32_t flags,
	    int32_t width,
	    int32_t height,
	    int32_t refresh)
{
}

static void
output_done(void *data,
	    struct wl_output *wl_output)
{
	struct output *output = data;
	struct libdecor_frame_gtk *frame_gtk;
	struct seat *seat;

	wl_list_for_each(frame_gtk,
			 &output->plugin_gtk->visible_frame_list, link) {
		bool updated = false;
		updated |= redraw_scale(frame_gtk, &frame_gtk->shadow);
		if (updated)
			libdecor_frame_toplevel_commit(&frame_gtk->frame);
	}
	wl_list_for_each(seat, &output->plugin_gtk->seat_list, link) {
		if (update_local_cursor(seat))
			send_cursor(seat);
	}
}

static void
output_scale(void *data,
	     struct wl_output *wl_output,
	     int32_t factor)
{
	struct output *output = data;

	output->scale = factor;
}

static struct wl_output_listener output_listener = {
	output_geometry,
	output_mode,
	output_done,
	output_scale
};

static void
init_wl_output(struct libdecor_plugin_gtk *plugin_gtk,
	       uint32_t id,
	       uint32_t version)
{
	struct output *output;

	if (version < 2) {
		libdecor_notify_plugin_error(
				plugin_gtk->context,
				LIBDECOR_ERROR_COMPOSITOR_INCOMPATIBLE,
				"%s version 2 required but only version %i is available\n",
				wl_output_interface.name,
				version);
	}

	output = zalloc(sizeof *output);
	output->plugin_gtk = plugin_gtk;
	wl_list_insert(&plugin_gtk->output_list, &output->link);
	output->id = id;
	output->wl_output =
		wl_registry_bind(plugin_gtk->wl_registry,
				 id, &wl_output_interface,
				 MIN (version, 3));
	wl_proxy_set_tag((struct wl_proxy *) output->wl_output,
			 &libdecor_gtk_proxy_tag);
	wl_output_add_listener(output->wl_output, &output_listener, output);
}

static void
registry_handle_global(void *user_data,
		       struct wl_registry *wl_registry,
		       uint32_t id,
		       const char *interface,
		       uint32_t version)
{
	struct libdecor_plugin_gtk *plugin_gtk = user_data;

	if (strcmp(interface, "wl_compositor") == 0)
		init_wl_compositor(plugin_gtk, id, version);
	else if (strcmp(interface, "wl_subcompositor") == 0)
		init_wl_subcompositor(plugin_gtk, id, version);
	else if (strcmp(interface, "wl_shm") == 0)
		init_wl_shm(plugin_gtk, id, version);
	else if (strcmp(interface, "wl_seat") == 0)
		init_wl_seat(plugin_gtk, id, version);
	else if (strcmp(interface, "wl_output") == 0)
		init_wl_output(plugin_gtk, id, version);
}

static void
remove_surface_outputs(struct border_component *cmpnt, const struct output *output)
{
	struct surface_output *surface_output;
	wl_list_for_each(surface_output, &cmpnt->output_list, link) {
		if (surface_output->output == output) {
			wl_list_remove(&surface_output->link);
			free(surface_output);
			break;
		}
	}
}

static void
output_removed(struct libdecor_plugin_gtk *plugin_gtk,
	       struct output *output)
{
	struct libdecor_frame_gtk *frame_gtk;
	struct seat *seat;

	wl_list_for_each(frame_gtk, &plugin_gtk->visible_frame_list, link) {
		remove_surface_outputs(&frame_gtk->shadow, output);
	}
	wl_list_for_each(seat, &plugin_gtk->seat_list, link) {
		struct cursor_output *cursor_output;
		wl_list_for_each(cursor_output, &seat->cursor_outputs, link) {
			if (cursor_output->output == output) {
				wl_list_remove(&cursor_output->link);
				free(cursor_output);
			}
		}
	}

	wl_list_remove(&output->link);
	wl_output_destroy(output->wl_output);
	free(output);
}

static void
registry_handle_global_remove(void *user_data,
			      struct wl_registry *wl_registry,
			      uint32_t name)
{
	struct libdecor_plugin_gtk *plugin_gtk = user_data;
	struct output *output;

	wl_list_for_each(output, &plugin_gtk->output_list, link) {
		if (output->id == name) {
			output_removed(plugin_gtk, output);
			break;
		}
	}
}

static const struct wl_registry_listener registry_listener = {
	registry_handle_global,
	registry_handle_global_remove
};

static bool
has_required_globals(struct libdecor_plugin_gtk *plugin_gtk)
{
	if (!plugin_gtk->wl_compositor)
		return false;
	if (!plugin_gtk->wl_subcompositor)
		return false;
	if (!plugin_gtk->wl_shm)
		return false;

	return true;
}

static void
globals_callback(void *user_data,
		 struct wl_callback *callback,
		 uint32_t time)
{
	struct libdecor_plugin_gtk *plugin_gtk = user_data;

	wl_callback_destroy(callback);
	plugin_gtk->globals_callback = NULL;
}

static const struct wl_callback_listener globals_callback_listener = {
	globals_callback
};

static struct libdecor_plugin *
libdecor_plugin_new(struct libdecor *context)
{
	struct libdecor_plugin_gtk *plugin_gtk;
	struct wl_display *wl_display;

	plugin_gtk = zalloc(sizeof *plugin_gtk);
	libdecor_plugin_init(&plugin_gtk->plugin,
			     context,
			     &gtk_plugin_iface);
	plugin_gtk->context = context;

	wl_list_init(&plugin_gtk->visible_frame_list);
	wl_list_init(&plugin_gtk->seat_list);
	wl_list_init(&plugin_gtk->output_list);

	/* fetch cursor theme and size*/
	if (!libdecor_get_cursor_settings(&plugin_gtk->cursor_theme_name,
					  &plugin_gtk->cursor_size)) {
		plugin_gtk->cursor_theme_name = NULL;
		plugin_gtk->cursor_size = 24;
	}

	plugin_gtk->color_scheme_setting = libdecor_get_color_scheme();

	wl_display = libdecor_get_wl_display(context);
	plugin_gtk->wl_registry = wl_display_get_registry(wl_display);
	wl_registry_add_listener(plugin_gtk->wl_registry,
				 &registry_listener,
				 plugin_gtk);

	plugin_gtk->globals_callback = wl_display_sync(wl_display);
	wl_callback_add_listener(plugin_gtk->globals_callback,
				 &globals_callback_listener,
				 plugin_gtk);
	wl_display_roundtrip(wl_display);

	if (!has_required_globals(plugin_gtk)) {
		fprintf(stderr, "libdecor-gtk-WARNING: Could not get required globals\n");
		libdecor_plugin_gtk_destroy(&plugin_gtk->plugin);
		return NULL;
	}

	/* setup GTK context */
	gdk_set_allowed_backends("wayland");
	gtk_disable_setlocale();

	if (!gtk_init_check(NULL, NULL)) {
		fprintf(stderr, "libdecor-gtk-WARNING: Failed to initialize GTK\n");
		libdecor_plugin_gtk_destroy(&plugin_gtk->plugin);
		return NULL;
	}

	g_object_set(gtk_settings_get_default(),
		     "gtk-application-prefer-dark-theme",
		     plugin_gtk->color_scheme_setting == LIBDECOR_COLOR_SCHEME_PREFER_DARK,
		     NULL);

	return &plugin_gtk->plugin;
}

static struct libdecor_plugin_priority priorities[] = {
	{ NULL, LIBDECOR_PLUGIN_PRIORITY_HIGH }
};

LIBDECOR_EXPORT const struct libdecor_plugin_description
libdecor_plugin_description = {
	.api_version = LIBDECOR_PLUGIN_API_VERSION,
	.capabilities = LIBDECOR_PLUGIN_CAPABILITY_BASE,
	.description = "GTK3 plugin",
	.priorities = priorities,
	.constructor = libdecor_plugin_new,
	.conflicting_symbols = {
		"png_free",
		NULL,
	},
};
