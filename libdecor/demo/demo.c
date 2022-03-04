/*
 * Copyright © 2011 Benjamin Franzke
 * Copyright © 2010 Intel Corporation
 * Copyright © 2018 Jonas Ådahl
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
#include <fcntl.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-cursor.h>
#include <xkbcommon/xkbcommon.h>

#include "libdecor.h"
#include "utils.h"
#include "cursor-settings.h"
#include "os-compatibility.h"

#include "xdg-shell-client-protocol.h"

struct window;

static const size_t chk = 16;
static const int DEFAULT_WIDTH = 30*chk;
static const int DEFAULT_HEIGHT = 20*chk;

static const int POPUP_WIDTH = 100;
static const int POPUP_HEIGHT = 300;

static const char *proxy_tag = "libdecor-demo";

static const char *titles[] = {
	"Hello!",
	"Hallå!",
	"Привет!",
	"Γειά σου!",
	"שלום!",
	"你好！",
	"สวัสดี!",
	"こんにちは！",
	"👻❤️🤖➕🍰",
};

static const size_t N_TITLES = ARRAY_SIZE(titles);


static bool
own_proxy(struct wl_proxy *proxy)
{
	return (wl_proxy_get_tag(proxy) == &proxy_tag);
}

static bool
own_output(struct wl_output *output)
{
	return own_proxy((struct wl_proxy *) output);
}

struct buffer {
	struct wl_buffer *wl_buffer;
	void *data;
	size_t data_size;
};

struct popup {
	struct wl_surface *wl_surface;
	struct xdg_surface *xdg_surface;
	struct xdg_popup *xdg_popup;
	struct xdg_surface *parent;
	struct seat *seat;
	struct window *window;
};

struct window {
	struct wl_surface *wl_surface;
	struct buffer *buffer;
	struct libdecor_frame *frame;
	int content_width;
	int content_height;
	int configured_width;
	int configured_height;
	int floating_width;
	int floating_height;
	enum libdecor_window_state window_state;
	struct wl_list outputs;
	int scale;
	struct popup *popup;
	size_t title_index;
};

struct seat {
	struct wl_seat *wl_seat;
	struct wl_keyboard *wl_keyboard;
	struct wl_pointer *wl_pointer;
	struct wl_list link;
	struct wl_list pointer_outputs;
	struct wl_cursor_theme *cursor_theme;
	struct wl_cursor *left_ptr_cursor;
	struct wl_surface *cursor_surface;
	struct wl_surface *pointer_focus;
	int pointer_scale;
	uint32_t serial;
	wl_fixed_t pointer_sx;
	wl_fixed_t pointer_sy;
	char *name;

	struct xkb_context *xkb_context;
	struct xkb_state *xkb_state;
};

struct output {
	uint32_t id;
	struct wl_output *wl_output;
	int scale;
	struct wl_list link;
};

struct window_output {
	struct output* output;
	struct wl_list link;
};

struct pointer_output {
	struct output* output;
	struct wl_list link;
};

static struct wl_compositor *wl_compositor;
static struct wl_shm *wl_shm;
static struct xdg_wm_base *xdg_wm_base;
static struct wl_list seats;
static struct wl_list outputs;

static bool has_xrgb = false;

static struct window *window;

static void
redraw(struct window *window);

static void
resize(struct window *window, int width, int height)
{
	struct libdecor_state *state;

	if (!libdecor_frame_is_floating(window->frame)) {
		printf("... ignoring in non-floating mode\n");
		return;
	}

	/* commit changes to decorations */
	state = libdecor_state_new( width, height);
	libdecor_frame_commit(window->frame, state, NULL);
	libdecor_state_free(state);
	/* force redraw of content and commit */
	window->configured_width = width;
	window->configured_height = height;
	/* store floating dimensions */
	window->floating_width = width;
	window->floating_height = height;
	redraw(window);
}

static struct buffer *
create_shm_buffer(int width,
		  int height,
		  uint32_t format);

static void
update_scale(struct window *window)
{
	int scale = 1;
	struct window_output *window_output;

	wl_list_for_each(window_output, &window->outputs, link) {
		scale = MAX(scale, window_output->output->scale);
	}
	if (scale != window->scale) {
		window->scale = scale;
		redraw(window);
	}
}

static void
shm_format(void *data,
	   struct wl_shm *wl_shm,
	   uint32_t format)
{
	if (format == WL_SHM_FORMAT_XRGB8888)
		has_xrgb = true;
}

static struct wl_shm_listener shm_listener = {
	shm_format
};

static void
try_update_cursor(struct seat *seat);

static void
cursor_surface_enter(void *data,
	      struct wl_surface *wl_surface,
	      struct wl_output *wl_output)
{
	struct seat *seat = data;
	struct pointer_output *pointer_output;

	if (!own_output(wl_output))
		return;

	pointer_output = zalloc(sizeof *pointer_output);
	pointer_output->output = wl_output_get_user_data(wl_output);
	wl_list_insert(&seat->pointer_outputs, &pointer_output->link);
	try_update_cursor(seat);
}

static void
cursor_surface_leave(void *data,
	      struct wl_surface *wl_surface,
	      struct wl_output *wl_output)
{
	struct seat *seat = data;
	struct pointer_output *pointer_output, *tmp;

	wl_list_for_each_safe(pointer_output, tmp, &seat->pointer_outputs, link) {
		if (pointer_output->output->wl_output == wl_output) {
			wl_list_remove(&pointer_output->link);
			free(pointer_output);
		}
	}
}

static struct wl_surface_listener cursor_surface_listener = {
	cursor_surface_enter,
	cursor_surface_leave,
};

static void
init_cursors(struct seat *seat)
{
	char *name;
	int size;
	struct wl_cursor_theme *theme;

	if (!libdecor_get_cursor_settings(&name, &size)) {
		name = NULL;
		size = 24;
	}
	size *= seat->pointer_scale;

	theme = wl_cursor_theme_load(name, size, wl_shm);
	free(name);
	if (theme != NULL) {
		if (seat->cursor_theme)
			wl_cursor_theme_destroy(seat->cursor_theme);
		seat->cursor_theme = theme;
	}
	if (seat->cursor_theme)
		seat->left_ptr_cursor
		  = wl_cursor_theme_get_cursor(seat->cursor_theme, "left_ptr");
	if (!seat->cursor_surface) {
		seat->cursor_surface = wl_compositor_create_surface(
								wl_compositor);
		wl_surface_add_listener(seat->cursor_surface,
					&cursor_surface_listener, seat);
	}
}

static void
set_cursor(struct seat *seat)
{
	struct wl_cursor *wl_cursor;
	struct wl_cursor_image *image;
	struct wl_buffer *buffer;
	const int scale = seat->pointer_scale;

	if (!seat->cursor_theme)
		return;

	wl_cursor = seat->left_ptr_cursor;

	image = wl_cursor->images[0];
	buffer = wl_cursor_image_get_buffer(image);
	wl_pointer_set_cursor(seat->wl_pointer, seat->serial,
			      seat->cursor_surface,
			      image->hotspot_x / scale,
			      image->hotspot_y / scale);
	wl_surface_attach(seat->cursor_surface, buffer, 0, 0);
	wl_surface_set_buffer_scale(seat->cursor_surface, scale);
	wl_surface_damage_buffer(seat->cursor_surface, 0, 0,
				 image->width, image->height);
	wl_surface_commit(seat->cursor_surface);
}

static void
try_update_cursor(struct seat *seat)
{
	struct pointer_output *pointer_output;
	int scale = 1;

	wl_list_for_each(pointer_output, &seat->pointer_outputs, link) {
		scale = MAX(scale, pointer_output->output->scale);
	}

	if (scale != seat->pointer_scale) {
		seat->pointer_scale = scale;
		init_cursors(seat);
		set_cursor(seat);
	}
}

static void
pointer_enter(void *data,
	      struct wl_pointer *wl_pointer,
	      uint32_t serial,
	      struct wl_surface *surface,
	      wl_fixed_t surface_x,
	      wl_fixed_t surface_y)
{
	struct seat *seat = data;

	seat->pointer_focus = surface;
	seat->serial = serial;

	if (surface != window->wl_surface)
		return;

	set_cursor(seat);

	seat->pointer_sx = surface_x;
	seat->pointer_sy = surface_y;
}

static void
pointer_leave(void *data,
	      struct wl_pointer *wl_pointer,
	      uint32_t serial,
	      struct wl_surface *surface)
{
	struct seat *seat = data;
	if (seat->pointer_focus == surface)
		seat->pointer_focus = NULL;
}

static void
pointer_motion(void *data,
	       struct wl_pointer *wl_pointer,
	       uint32_t time,
	       wl_fixed_t surface_x,
	       wl_fixed_t surface_y)
{
	struct seat *seat = data;

	seat->pointer_sx = surface_x;
	seat->pointer_sy = surface_y;
}

static struct xdg_positioner *
create_positioner(struct seat *seat)
{
	struct xdg_positioner *positioner;
	enum xdg_positioner_constraint_adjustment constraint_adjustment;
	int x, y;

	positioner = xdg_wm_base_create_positioner(xdg_wm_base);
	xdg_positioner_set_size(positioner, POPUP_WIDTH, POPUP_HEIGHT);

	libdecor_frame_translate_coordinate(window->frame,
					    wl_fixed_to_int(seat->pointer_sx),
					    wl_fixed_to_int(seat->pointer_sy),
					    &x, &y);

	xdg_positioner_set_anchor_rect(positioner, x, y, 1, 1);

	constraint_adjustment = (XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_FLIP_Y |
				 XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_X);
	xdg_positioner_set_constraint_adjustment (positioner,
						  constraint_adjustment);

	xdg_positioner_set_anchor (positioner,
				   XDG_POSITIONER_ANCHOR_BOTTOM_RIGHT);
	xdg_positioner_set_gravity (positioner,
				    XDG_POSITIONER_GRAVITY_BOTTOM_RIGHT);

	return positioner;
}

static void
xdg_popup_configure(void *data,
		    struct xdg_popup *xdg_popup,
		    int32_t x,
		    int32_t y,
		    int32_t width,
		    int32_t height)
{
}

static void
popup_destroy(struct popup *popup)
{
	libdecor_frame_popup_ungrab(popup->window->frame,
				    popup->seat->name);
	xdg_popup_destroy(popup->xdg_popup);
	xdg_surface_destroy(popup->xdg_surface);
	wl_surface_destroy(popup->wl_surface);
	popup->window->popup = NULL;
	free(popup);
}

static void
xdg_popup_done(void             *data,
	       struct xdg_popup *xdg_popup)
{
	struct popup *popup = data;

	popup_destroy(popup);
}

static const struct xdg_popup_listener xdg_popup_listener = {
	xdg_popup_configure,
	xdg_popup_done,
};

static void
xdg_surface_configure(void *data,
		      struct xdg_surface *xdg_surface,
		      uint32_t serial)
{
	struct popup *popup = data;
	uint32_t *pixels;
	struct buffer *buffer;
	int y;

	buffer = create_shm_buffer(POPUP_WIDTH, POPUP_HEIGHT,
				   WL_SHM_FORMAT_XRGB8888);
	pixels = buffer->data;
	for (y = 0; y < POPUP_HEIGHT; y++) {
		int x;

		for (x = 0; x < POPUP_WIDTH; x++)
			pixels[y * POPUP_WIDTH + x] = 0xff4455ff;
	}

	wl_surface_attach(popup->wl_surface, buffer->wl_buffer, 0, 0);
	wl_surface_set_buffer_scale(window->wl_surface, window->scale);
	wl_surface_damage(window->wl_surface, 0, 0,
			  POPUP_WIDTH, POPUP_HEIGHT);
	xdg_surface_ack_configure(popup->xdg_surface, serial);
	wl_surface_commit(popup->wl_surface);
}

static const struct xdg_surface_listener xdg_surface_listener = {
	xdg_surface_configure,
};

static void
open_popup(struct seat *seat)
{
	struct popup *popup;
	struct xdg_positioner *positioner;

	popup = zalloc(sizeof *popup);

	popup->wl_surface = wl_compositor_create_surface(wl_compositor);
	popup->xdg_surface = xdg_wm_base_get_xdg_surface (xdg_wm_base,
							  popup->wl_surface);
	popup->parent = libdecor_frame_get_xdg_surface(window->frame);
	popup->window = window;
	popup->seat = seat;
	positioner = create_positioner(seat);
	popup->xdg_popup = xdg_surface_get_popup(popup->xdg_surface,
						 popup->parent,
						 positioner);
	xdg_positioner_destroy(positioner);

	xdg_surface_add_listener (popup->xdg_surface,
				  &xdg_surface_listener,
				  popup);
	xdg_popup_add_listener (popup->xdg_popup,
				&xdg_popup_listener,
				popup);

	window->popup = popup;

	xdg_popup_grab(popup->xdg_popup, seat->wl_seat, seat->serial);
	wl_surface_commit(popup->wl_surface);

	libdecor_frame_popup_grab(window->frame, seat->name);
}

static void
close_popup(struct window *window)
{
	struct popup *popup = window->popup;

	popup_destroy(popup);
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

	if (seat->pointer_focus != window->wl_surface)
		return;

	seat->serial = serial;

	if (window->popup &&
	    state == WL_POINTER_BUTTON_STATE_PRESSED)
		close_popup(window);

	if (button == BTN_LEFT &&
	    state == WL_POINTER_BUTTON_STATE_PRESSED) {
		libdecor_frame_move(window->frame, seat->wl_seat, serial);
	} else if (button == BTN_MIDDLE &&
	    state == WL_POINTER_BUTTON_STATE_PRESSED) {
		libdecor_frame_show_window_menu(window->frame,
						seat->wl_seat,
						serial,
						wl_fixed_to_int(seat->pointer_sx),
						wl_fixed_to_int(seat->pointer_sy));
	} else if (button == BTN_RIGHT &&
		   state == WL_POINTER_BUTTON_STATE_PRESSED) {
		if (!window->popup)
			open_popup(seat);
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
keyboard_keymap(void *data,
		struct wl_keyboard *wl_keyboard,
		uint32_t format,
		int32_t fd,
		uint32_t size)
{
	struct seat *seat = data;

	if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
	  close(fd);
	  return;
	}

	char *map_str = (char *)(mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0));
	if (map_str == MAP_FAILED) {
		close(fd);
		fprintf(stderr, "keymap mmap failed: %s", strerror(errno));
		return;
	}

	struct xkb_keymap *keymap = xkb_keymap_new_from_string(
				seat->xkb_context, map_str,
				XKB_KEYMAP_FORMAT_TEXT_V1,
				XKB_KEYMAP_COMPILE_NO_FLAGS);
	munmap(map_str, size);
	close(fd);

	if (!keymap)
		return;

	seat->xkb_state = xkb_state_new(keymap);

	xkb_keymap_unref(keymap);
}

static void
keyboard_enter(void *data,
	       struct wl_keyboard *wl_keyboard,
	       uint32_t serial,
	       struct wl_surface *surface,
	       struct wl_array *keys)
{
}

static void
keyboard_leave(void *data,
	       struct wl_keyboard *wl_keyboard,
	       uint32_t serial,
	       struct wl_surface *surface)
{
}

static void
keyboard_key(void *data,
	     struct wl_keyboard *wl_keyboard,
	     uint32_t serial,
	     uint32_t time,
	     uint32_t key,
	     uint32_t state)
{
	struct seat *seat = data;

	if (state & WL_KEYBOARD_KEY_STATE_PRESSED) {
		const xkb_keysym_t *syms;

		if (xkb_state_key_get_syms(seat->xkb_state, key + 8, &syms) != 1)
			return;

		switch (syms[0]) {
		case XKB_KEY_Escape:
			printf("close\n");
			libdecor_frame_close(window->frame);
			break;
		case XKB_KEY_1: /* toggle resizability */
			if (libdecor_frame_has_capability(
				    window->frame, LIBDECOR_ACTION_RESIZE)) {
				printf("set fixed-size\n");
				libdecor_frame_unset_capabilities(window->frame,
							LIBDECOR_ACTION_RESIZE);
			}
			else {
				printf("set resizeable\n");
				libdecor_frame_set_capabilities(window->frame,
							LIBDECOR_ACTION_RESIZE);
			}
			break;
		case XKB_KEY_2: /* maximize */
			printf("maximize\n");
			libdecor_frame_set_maximized(window->frame);
			break;
		case XKB_KEY_3: /* un-maximize / restore */
			printf("un-maximize\n");
			libdecor_frame_unset_maximized(window->frame);
			break;
		case XKB_KEY_4: /* fullscreen */
			printf("fullscreen\n");
			libdecor_frame_set_fullscreen(window->frame, NULL);
			break;
		case XKB_KEY_5: /* un-fullscreen / restore */
			printf("un-fullscreen\n");
			libdecor_frame_unset_fullscreen(window->frame);
			break;
		case XKB_KEY_minus:
		case XKB_KEY_plus:
			{
				const int dd = (syms[0] == XKB_KEY_minus ? -1 : +1) * chk/2;
				printf("resize to: %i x %i\n",
				       window->configured_width + dd,
				       window->configured_height + dd);
				resize(window,
				       window->configured_width + dd,
				       window->configured_height + dd);
			}
			break;
		case XKB_KEY_v: /* VGA: 640x480 */
			printf("set VGA resolution: 640x480\n");
			resize(window, 640, 480);
			break;
		case XKB_KEY_s: /* SVGA: 800x600 */
			printf("set SVGA resolution: 800x600\n");
			resize(window, 800, 600);
			break;
		case XKB_KEY_x: /* XVGA: 1024x768 */
			printf("set XVGA resolution: 1024x768\n");
			resize(window, 1024, 768);
			break;
		case XKB_KEY_t:
			libdecor_frame_set_title(window->frame, titles[window->title_index]);
			window->title_index = (window->title_index + 1) % N_TITLES;
			break;
		case XKB_KEY_h: /* toggle decorations */
			libdecor_frame_set_visibility(
					window->frame,
					!libdecor_frame_is_visible(window->frame));
			printf("decorations %s\n",
			       libdecor_frame_is_visible(window->frame) ?
				       "visible" : "hidden");
			break;
		}
	}
}

static void
keyboard_modifiers(void *data,
		   struct wl_keyboard *wl_keyboard,
		   uint32_t serial,
		   uint32_t mods_depressed,
		   uint32_t mods_latched,
		   uint32_t mods_locked,
		   uint32_t group)
{
	struct seat *seat = data;

	xkb_state_update_mask(seat->xkb_state,
			      mods_depressed, mods_latched, mods_locked,
			      0, 0, group);
}

static void
keyboard_repeat_info(void *data,
		     struct wl_keyboard *wl_keyboard,
		     int32_t rate,
		     int32_t delay)
{
}

static struct wl_keyboard_listener keyboard_listener = {
	keyboard_keymap,
	keyboard_enter,
	keyboard_leave,
	keyboard_key,
	keyboard_modifiers,
	keyboard_repeat_info,
};

static void
seat_capabilities(void *data,
		  struct wl_seat *wl_seat,
		  uint32_t capabilities)
{
	struct seat *seat = data;
	if (capabilities & WL_SEAT_CAPABILITY_POINTER &&
	    !seat->wl_pointer) {
		seat->wl_pointer = wl_seat_get_pointer(wl_seat);
		wl_pointer_add_listener(seat->wl_pointer, &pointer_listener,
					seat);
		seat->pointer_scale = 1;
		init_cursors(seat);
	} else if (!(capabilities & WL_SEAT_CAPABILITY_POINTER) &&
		   seat->wl_pointer) {
		wl_pointer_release(seat->wl_pointer);
		seat->wl_pointer = NULL;
	}

	if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD &&
	    !seat->wl_keyboard) {
		seat->wl_keyboard = wl_seat_get_keyboard(wl_seat);
		seat->xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
		wl_keyboard_add_listener(seat->wl_keyboard, &keyboard_listener,
					 seat);
	} else if (!(capabilities & WL_SEAT_CAPABILITY_KEYBOARD) &&
		   seat->wl_keyboard) {
		xkb_context_unref(seat->xkb_context);
		wl_keyboard_release(seat->wl_keyboard);
		seat->wl_keyboard = NULL;
	}
}

static void
seat_name(void *data,
	  struct wl_seat *wl_seat,
	  const char *name)
{
	struct seat *seat = data;

	seat->name = strdup(name);
}

static struct wl_seat_listener seat_listener = {
	seat_capabilities,
	seat_name
};

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
	struct seat *seat;

	if (window) {
		if (output->scale != window->scale)
			update_scale(window);
	}

	wl_list_for_each(seat, &seats, link) {
		try_update_cursor(seat);
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
registry_handle_global(void *user_data,
		       struct wl_registry *wl_registry,
		       uint32_t id,
		       const char *interface,
		       uint32_t version)
{
	struct seat *seat;
	struct output *output;

	if (strcmp(interface, "wl_compositor") == 0) {
		if (version < 4) {
			fprintf(stderr, "wl_compositor version >= 4 required");
			exit(EXIT_FAILURE);
		}
		wl_compositor =
			wl_registry_bind(wl_registry,
					 id, &wl_compositor_interface, 4);
	} else if (strcmp(interface, "wl_shm") == 0) {
		wl_shm = wl_registry_bind(wl_registry,
					  id, &wl_shm_interface, 1);
		wl_shm_add_listener(wl_shm, &shm_listener, NULL);
	} else if (strcmp(interface, "wl_seat") == 0) {
		if (version < 3) {
			fprintf(stderr, "%s version 3 required but only version "
					"%i is available\n", interface, version);
			exit(EXIT_FAILURE);
		}
		seat = zalloc(sizeof *seat);
		wl_list_init(&seat->pointer_outputs);
		seat->wl_seat = wl_registry_bind(wl_registry,
						 id, &wl_seat_interface, 3);
		wl_seat_add_listener(seat->wl_seat, &seat_listener, seat);
	} else if (strcmp(interface, "wl_output") == 0) {
		if (version < 2) {
			fprintf(stderr, "%s version 3 required but only version "
					"%i is available\n", interface, version);
			exit(EXIT_FAILURE);
		}
		output = zalloc(sizeof *output);
		output->id = id;
		output->scale = 1;
		output->wl_output = wl_registry_bind(wl_registry,
						     id, &wl_output_interface,
						     2);
		wl_proxy_set_tag((struct wl_proxy *) output->wl_output,
				 &proxy_tag);
		wl_output_add_listener(output->wl_output, &output_listener,
				       output);
		wl_list_insert(&outputs, &output->link);
	} else if (strcmp(interface, "xdg_wm_base") == 0) {
		xdg_wm_base = wl_registry_bind(wl_registry, id,
					       &xdg_wm_base_interface,
					       1);
		xdg_wm_base_add_listener(xdg_wm_base,
					 &xdg_wm_base_listener,
					 NULL);
	}
}

static void
registry_handle_global_remove(void *data, struct wl_registry *registry,
			      uint32_t name)
{
	struct output *output;
	struct window_output *window_output;

	wl_list_for_each(output, &outputs, link) {
		if (output->id == name) {
			wl_list_for_each(window_output, &window->outputs,
					 link) {
				if (window_output->output == output) {
					wl_list_remove(&window_output->link);
					free(window_output);
				}
			}
			wl_list_remove(&output->link);
			wl_output_destroy(output->wl_output);
			free(output);
			break;
		}
	}
}

static const struct wl_registry_listener registry_listener = {
	registry_handle_global,
	registry_handle_global_remove
};

static void
handle_error(struct libdecor *context,
	     enum libdecor_error error,
	     const char *message)
{
	fprintf(stderr, "Caught error (%d): %s\n", error, message);
	exit(EXIT_FAILURE);
}

static struct libdecor_interface libdecor_iface = {
	.error = handle_error,
};

static void
buffer_release(void *user_data,
	       struct wl_buffer *wl_buffer)
{
	struct buffer *buffer = user_data;

	wl_buffer_destroy(buffer->wl_buffer);
	munmap(buffer->data, buffer->data_size);
	free(buffer);
}

static const struct wl_buffer_listener buffer_listener = {
	buffer_release
};

static struct buffer *
create_shm_buffer(int width,
		  int height,
		  uint32_t format)
{
	struct wl_shm_pool *pool;
	int fd, size, stride;
	void *data;
	struct buffer *buffer;

	stride = width * 4;
	size = stride * height;

	fd = os_create_anonymous_file(size);
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

	buffer = zalloc(sizeof *buffer);

	pool = wl_shm_create_pool(wl_shm, fd, size);
	buffer->wl_buffer = wl_shm_pool_create_buffer(pool, 0,
						      width, height,
						      stride, format);
	wl_buffer_add_listener(buffer->wl_buffer, &buffer_listener, buffer);
	wl_shm_pool_destroy(pool);
	close(fd);

	buffer->data = data;
	buffer->data_size = size;

	return buffer;
}

static void
paint_buffer(struct buffer *buffer,
	     int width,
	     int height,
	     int scale,
	     enum libdecor_window_state window_state)
{
	uint32_t *pixels = buffer->data;
	uint32_t bg, fg, color;
	int y, x, sx, sy;
	size_t off;
	int stride = width * scale;

	if (window_state & LIBDECOR_WINDOW_STATE_ACTIVE) {
		fg = 0xffbcbcbc;
		bg = 0xff8e8e8e;
	} else {
		fg = 0xff8e8e8e;
		bg = 0xff484848;
	}

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			color = (x & chk) ^ (y & chk) ? fg : bg;
			for (sx = 0; sx < scale; sx++) {
				for (sy = 0; sy < scale; sy++) {
					off = x * scale + sx
					      + (y * scale + sy) * stride;
					pixels[off] = color;
				}
			}
		}
	}
}

static void
redraw(struct window *window)
{
	struct buffer *buffer;

	buffer = create_shm_buffer(window->configured_width * window->scale,
				   window->configured_height * window->scale,
				   WL_SHM_FORMAT_XRGB8888);
	paint_buffer(buffer, window->configured_width,
		     window->configured_height, window->scale,
		     window->window_state);

	wl_surface_attach(window->wl_surface, buffer->wl_buffer, 0, 0);
	wl_surface_set_buffer_scale(window->wl_surface, window->scale);
	wl_surface_damage_buffer(window->wl_surface, 0, 0,
				 window->configured_width * window->scale,
				 window->configured_height * window->scale);
	wl_surface_commit(window->wl_surface);
}

static void
handle_configure(struct libdecor_frame *frame,
		 struct libdecor_configuration *configuration,
		 void *user_data)
{
	struct window *window = user_data;
	int width, height;
	enum libdecor_window_state window_state;
	struct libdecor_state *state;

	if (!libdecor_configuration_get_content_size(configuration, frame,
						     &width, &height)) {
		width = window->content_width;
		height = window->content_height;
	}

	width = (width == 0) ? window->floating_width : width;
	height = (height == 0) ? window->floating_height : height;

	window->configured_width = width;
	window->configured_height = height;

	if (!libdecor_configuration_get_window_state(configuration,
						     &window_state))
		window_state = LIBDECOR_WINDOW_STATE_NONE;

	window->window_state = window_state;

	state = libdecor_state_new(width, height);
	libdecor_frame_commit(frame, state, configuration);
	libdecor_state_free(state);

	/* store floating dimensions */
	if (libdecor_frame_is_floating(window->frame)) {
		window->floating_width = width;
		window->floating_height = height;
	}

	redraw(window);
}

static void
handle_close(struct libdecor_frame *frame,
	     void *user_data)
{
	exit(EXIT_SUCCESS);
}

static void
handle_commit(struct libdecor_frame *frame,
	      void *user_data)
{
	wl_surface_commit(window->wl_surface);
}

static void
handle_dismiss_popup(struct libdecor_frame *frame,
		     const char *seat_name,
		     void *user_data)
{
	popup_destroy(window->popup);
}

static struct libdecor_frame_interface libdecor_frame_iface = {
	handle_configure,
	handle_close,
	handle_commit,
	handle_dismiss_popup,
};

static void
surface_enter(void *data,
	      struct wl_surface *wl_surface,
	      struct wl_output *wl_output)
{
	struct window *window = data;
	struct output *output;
	struct window_output *window_output;

	if (!own_output(wl_output))
		return;

	output = wl_output_get_user_data(wl_output);

	if (output == NULL)
		return;

	window_output = zalloc(sizeof *window_output);
	window_output->output = output;
	wl_list_insert(&window->outputs, &window_output->link);
	update_scale(window);
}

static void
surface_leave(void *data,
	      struct wl_surface *wl_surface,
	      struct wl_output *wl_output)
{
	struct window *window = data;
	struct window_output *window_output;

	wl_list_for_each(window_output, &window->outputs, link) {
		if (window_output->output->wl_output == wl_output) {
			wl_list_remove(&window_output->link);
			free(window_output);
			update_scale(window);
			break;
		}
	}
}

static struct wl_surface_listener surface_listener = {
	surface_enter,
	surface_leave,
};

static void
free_outputs()
{
	struct output *output;
	struct window_output *window_output, *window_output_tmp;

	wl_list_for_each(output, &outputs, link) {
		wl_list_for_each_safe(window_output, window_output_tmp, &window->outputs, link) {
			if (window_output->output == output) {
				wl_list_remove(&window_output->link);
				free(window_output);
			}
		}
		wl_list_remove(&output->link);
		wl_output_destroy(output->wl_output);
		free(output);
		break;
	}
}

int
main(int argc,
     char **argv)
{
	struct timeval start;
	struct timeval now;
	struct wl_display *wl_display;
	struct wl_registry *wl_registry;
	struct libdecor *context;
	struct output *output;
	int timeout = -1;
	bool timed_quit = false;

	if (argc > 1 && strcmp(argv[1], "--timed-quit") == 0) {
		timed_quit = true;
		timeout = 500; /* ms */
	}

	/* write all output to stdout immediately */
	setbuf(stdout, NULL);

	wl_display = wl_display_connect(NULL);
	if (!wl_display) {
		fprintf(stderr, "No Wayland connection\n");
		return EXIT_FAILURE;
	}

	wl_list_init(&seats);
	wl_list_init(&outputs);

	wl_registry = wl_display_get_registry(wl_display);
	wl_registry_add_listener(wl_registry,
				 &registry_listener,
				 NULL);
	wl_display_roundtrip(wl_display);
	wl_display_roundtrip(wl_display);
	if (!has_xrgb) {
		fprintf(stderr, "No XRGB shm format\n");
		return EXIT_FAILURE;
	}

	window = zalloc(sizeof *window);
	window->scale = 1;
	window->title_index = 0;
	wl_list_for_each(output, &outputs, link) {
		window->scale = MAX(window->scale, output->scale);
	}
	wl_list_init(&window->outputs);
	window->wl_surface = wl_compositor_create_surface(wl_compositor);
	wl_surface_add_listener(window->wl_surface, &surface_listener, window);

	/* initialise content dimensions */
	window->floating_width = DEFAULT_WIDTH;
	window->floating_height = DEFAULT_HEIGHT;

	context = libdecor_new(wl_display, &libdecor_iface);
	window->frame = libdecor_decorate(context, window->wl_surface,
					  &libdecor_frame_iface, window);
	libdecor_frame_set_app_id(window->frame, "libdecor-demo");
	libdecor_frame_set_title(window->frame, "libdecor demo");
	libdecor_frame_map(window->frame);

	gettimeofday(&start, NULL);
	libdecor_frame_set_min_content_size(window->frame, 15 * chk, 10 * chk);

	while (libdecor_dispatch(context, timeout) >= 0) {
		if (timed_quit) {
			gettimeofday(&now, NULL);
			if (now.tv_sec >= start.tv_sec + 2) {
				fprintf(stderr, "Exiting due to --timed-quit\n");
				libdecor_frame_close(window->frame);
			}
		}
	}

	free_outputs();

	free(window);

	return EXIT_SUCCESS;
}
