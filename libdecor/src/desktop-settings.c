/*
 * Copyright © 2019 Christian Rauch
 * Copyright © 2024 Colin Kinloch
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

#include "desktop-settings.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "config.h"

static bool
get_cursor_settings_from_env(char **theme, int *size)
{
	char *env_xtheme;
	char *env_xsize;

	env_xtheme = getenv("XCURSOR_THEME");
	if (env_xtheme != NULL)
		*theme = strdup(env_xtheme);

	env_xsize = getenv("XCURSOR_SIZE");
	if (env_xsize != NULL)
		*size = atoi(env_xsize);

	return env_xtheme != NULL && env_xsize != NULL;
}

#ifdef HAS_DBUS
#include <dbus/dbus.h>

static DBusMessage *
get_setting_sync(DBusConnection *const connection,
		 const char *key,
		 const char *value)
{
	DBusError error;
	dbus_bool_t success;
	DBusMessage *message;
	DBusMessage *reply;

	message = dbus_message_new_method_call(
		"org.freedesktop.portal.Desktop",
		"/org/freedesktop/portal/desktop",
		"org.freedesktop.portal.Settings",
		"Read");

	success = dbus_message_append_args(message,
		DBUS_TYPE_STRING, &key,
		DBUS_TYPE_STRING, &value,
		DBUS_TYPE_INVALID);

	if (!success)
		return NULL;

	dbus_error_init(&error);

	reply = dbus_connection_send_with_reply_and_block(
			     connection,
			     message,
			     DBUS_TIMEOUT_USE_DEFAULT,
			     &error);

	dbus_message_unref(message);

	if (dbus_error_is_set(&error)) {
		dbus_error_free(&error);
		return NULL;
	}

	dbus_error_free(&error);
	return reply;
}

static bool
parse_type(DBusMessage *const reply,
	   const int type,
	   void *value)
{
	DBusMessageIter iter[3];

	dbus_message_iter_init(reply, &iter[0]);
	if (dbus_message_iter_get_arg_type(&iter[0]) != DBUS_TYPE_VARIANT)
		return false;

	dbus_message_iter_recurse(&iter[0], &iter[1]);
	if (dbus_message_iter_get_arg_type(&iter[1]) != DBUS_TYPE_VARIANT)
		return false;

	dbus_message_iter_recurse(&iter[1], &iter[2]);
	if (dbus_message_iter_get_arg_type(&iter[2]) != type)
		return false;

	dbus_message_iter_get_basic(&iter[2], value);

	return true;
}

bool
libdecor_get_cursor_settings(char **theme, int *size)
{
	static const char name[] = "org.gnome.desktop.interface";
	static const char key_theme[] = "cursor-theme";
	static const char key_size[] = "cursor-size";

	DBusError error;
	DBusConnection *connection;
	DBusMessage *reply;
	const char *value_theme = NULL;

	dbus_error_init(&error);

	connection = dbus_bus_get(DBUS_BUS_SESSION, &error);

	if (dbus_error_is_set(&error))
		goto fallback;

	reply = get_setting_sync(connection, name, key_theme);
	if (!reply)
		goto fallback;

	if (!parse_type(reply, DBUS_TYPE_STRING, &value_theme)) {
		dbus_message_unref(reply);
		goto fallback;
	}

	*theme = strdup(value_theme);

	dbus_message_unref(reply);

	reply = get_setting_sync(connection, name, key_size);
	if (!reply)
		goto fallback;

	if (!parse_type(reply, DBUS_TYPE_INT32, size)) {
		dbus_message_unref(reply);
		goto fallback;
	}

	dbus_message_unref(reply);

	return true;

fallback:
	return get_cursor_settings_from_env(theme, size);
}

enum libdecor_color_scheme
libdecor_get_color_scheme()
{
	static const char name[] = "org.freedesktop.appearance";
	static const char key_color_scheme[] = "color-scheme";
	uint32_t color = 0;

	DBusError error;
	DBusConnection *connection;
	DBusMessage *reply;

	dbus_error_init(&error);

	connection = dbus_bus_get(DBUS_BUS_SESSION, &error);

	if (dbus_error_is_set(&error))
		return 0;

	reply = get_setting_sync(connection, name, key_color_scheme);
	if (!reply)
		return 0;

	if (!parse_type(reply, DBUS_TYPE_UINT32, &color)) {
		dbus_message_unref(reply);
		return 0;
	}

	dbus_message_unref(reply);

	return color;
}
#else
bool
libdecor_get_cursor_settings(char **theme, int *size)
{
	return get_cursor_settings_from_env(theme, size);
}

uint32_t
libdecor_get_color_scheme()
{
	return LIBDECOR_COLOR_SCHEME_DEFAULT;
}
#endif
