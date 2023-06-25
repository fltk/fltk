//
// Implementation of Wayland Screen interface
//
// Copyright 1998-2023 by Bill Spitzak and others.
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

#include "Fl_Wayland_Screen_Driver.H"
#include "Fl_Wayland_Window_Driver.H"
#include "Fl_Wayland_Graphics_Driver.H"
#include <wayland-cursor.h>
#include "../../../libdecor/src/libdecor.h"
#include "xdg-shell-client-protocol.h"
#include "../Posix/Fl_Posix_System_Driver.H"
#include <FL/Fl.H>
#include <FL/platform.H>
#include <FL/fl_ask.H>
#include <FL/filename.H>
#include <FL/Fl_Int_Vector.H>
#include "../../print_button.h"
#include <dlfcn.h>
#include <linux/input.h>
#include <stdlib.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-compose.h>
#include "text-input-client-protocol.h"
#include <assert.h>
#include <sys/mman.h>
#include <poll.h>
#include <errno.h>
#include <string.h> // for strerror()
extern "C" {
  bool libdecor_get_cursor_settings(char **theme, int *size);
}


#define fl_max(a,b) ((a) > (b) ? (a) : (b))
#define fl_min(a,b) ((a) < (b) ? (a) : (b))

struct pointer_output {
  Fl_Wayland_Screen_Driver::output* output;
  struct wl_list link;
};

/* Implementation note about support of 3 Wayland compositors: Mutter, Weston, KDE.

- About CSD and SSD :
 * Mutter and Weston use CSD (client-side decoration) which means that libdecor.so draws all window
 titlebars and responds to resize, minimization and maximization events.
 * KDE uses SSD (server-side decoration) which means the OS draws titlebars according to its own rules
 and triggers resize, minimization and maximization events.

- Function registry_handle_global() runs within fl_open_display() and sets public static variable
 Fl_Wayland_Screen_Driver::compositor to either Fl_Wayland_Screen_Driver::MUTTER, ::WESTON, or ::KDE.

- Specific operations for WESTON:
 * When a libdecor-framed window is minimized under Weston, the frame remains on display. To avoid
 that, function libdecor_frame_set_minimized() is modified so it turns off the frame's visibility, with
 function libdecor_frame_set_visibility(), when the window is minimized. That's implemented in file
 libdecor/build/fl_libdecor.c. The modified libdecor_frame_set_minimized() function, part of libdecor.so,
 needs access to variable Fl_Wayland_Screen_Driver::compositor, part of libfltk.a. This is achieved
 calling FLTK function fl_libdecor_using_weston() which returns whether the running compositor
 is Weston. This Weston bug has been corrected in Weston version 10. Thus, this special processing
 is not performed when Weston version is ≥ 10.

- Synchronization between drawing to buffer and committing buffer to screen.
 Before committing a new graphics scene for display, Wayland requires to make sure the compositor is
 ready for commit. FLTK uses frame callbacks for that.
 A frame callback is created when an app calls Fl_Wayland_Window_Driver::make_current()
 directly. This directs a callback listener function, called surface_frame_done, to be called by the
 compositor when it's ready to commit a new graphics scene. This function schedules a new frame callback
 and commits the buffer to the display.
 A frame callback is also created by Fl_Wayland_Window_Driver::flush() when a window redraw operation
 is needed. FLTK processes wayland events until the compositor is ready for commit and then commits
 the new window content.

 - Support of Fl_Window::border(int) :
 FLTK uses libdecor_frame_set_visibility() to show or hide a toplevel window's frame. This doesn't work
 with KDE which uses Server-Side Decoration. In that case, FLTK hides and re-shows the window to toggle
 between presence and absence of a window's frame.
*/


/* Implementation note about screen-related information

 struct wl_output : Wayland-defined, contains info about a screen, one such record for each screen

 struct Fl_Wayland_Screen_Driver::output { // FLTK defined
    uint32_t id; // screen identification
    short width; // screen width in pixels
    short height; // screen height in pixels
    float dpi;
    struct wl_output *wl_output;
    int wld_scale; // Wayland scale
    float gui_scale; // user-set scale
    struct wl_list link;
 };

 The unique Fl_Wayland_Screen_Driver object contains a member
   "outputs" of type struct wl_list = list of Fl_Wayland_Screen_Driver::output records
   - this list is initialised by open-display
   - registry_handle_global() feeds the list with 1 record for each screen
   - registry_handle_global_remove() runs when a screen is removed. It removes
   the output record that corresponds to that screen from the unique list of screens
   (outputs member of the Fl_Wayland_Screen_Driver) and the list of struct output objects attached
   to each window.

 Each struct wld_window object contains a member
   "output" of type pter to Fl_Wayland_Screen_Driver::output
   - this pointer is set by surface_enter() (when a surface is mapped)
   - surface_leave() sets it to NULL
   - hide() sets it to NULL
 */

static Fl_Int_Vector key_vector; // used by Fl_Wayland_Screen_Driver::event_key()

Fl_Wayland_Screen_Driver::compositor_name Fl_Wayland_Screen_Driver::compositor = Fl_Wayland_Screen_Driver::unspecified;


extern "C" {
  bool fl_libdecor_using_weston(void) {
    return Fl_Wayland_Screen_Driver::compositor == Fl_Wayland_Screen_Driver::WESTON;
  }
}

static void xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial)
{
    xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_ping,
};


// these are set by Fl::args() and override any system colors: from Fl_get_system_colors.cxx
extern const char *fl_fg;
extern const char *fl_bg;
extern const char *fl_bg2;
// end of extern additions workaround


static void do_set_cursor(struct Fl_Wayland_Screen_Driver::seat *seat, struct wl_cursor *wl_cursor = NULL)
{
  struct wl_cursor_image *image;
  struct wl_buffer *buffer;
  const int scale = seat->pointer_scale;

  if (!seat->cursor_theme)
    return;

  if (!wl_cursor) wl_cursor = seat->default_cursor;
  image = wl_cursor->images[0];
  buffer = wl_cursor_image_get_buffer(image);
  wl_pointer_set_cursor(seat->wl_pointer, seat->pointer_enter_serial,
            seat->cursor_surface,
            image->hotspot_x / scale,
            image->hotspot_y / scale);
  wl_surface_attach(seat->cursor_surface, buffer, 0, 0);
  wl_surface_set_buffer_scale(seat->cursor_surface, scale);
  wl_surface_damage_buffer(seat->cursor_surface, 0, 0,
         image->width, image->height);
  wl_surface_commit(seat->cursor_surface);
}

static uint32_t ptime;
static uint32_t wld_event_time;
static int px, py;


static void set_event_xy(Fl_Window *win) {
  // turn off is_click if enough time or mouse movement has passed:
  if (abs(Fl::e_x_root-px)+abs(Fl::e_y_root-py) > 3 ||
      wld_event_time >= ptime+1000) {
    Fl::e_is_click = 0;
//fprintf(stderr, "Fl::e_is_click = 0\n");
  }
}

// if this is same event as last && is_click, increment click count:
static inline void checkdouble() {
  if (Fl::e_is_click == Fl::e_keysym) {
    Fl::e_clicks++;
//fprintf(stderr, "Fl::e_clicks = %d\n", Fl::e_clicks);
  } else {
    Fl::e_clicks = 0;
    Fl::e_is_click = Fl::e_keysym;
//fprintf(stderr, "Fl::e_is_click = %d\n", Fl::e_is_click);
  }
  px = Fl::e_x_root;
  py = Fl::e_y_root;
  ptime = wld_event_time;
}


struct wl_display *Fl_Wayland_Screen_Driver::wl_display = NULL;


static Fl_Window *event_coords_from_surface(struct wl_surface *surface,
                                       wl_fixed_t surface_x, wl_fixed_t surface_y) {
  Fl_Window *win = Fl_Wayland_Window_Driver::surface_to_window(surface);
  if (!win) return NULL;
  int delta_x = 0, delta_y = 0;
  while (win->parent()) {
    delta_x += win->x();
    delta_y += win->y();
    win = win->window();
  }
  float f = Fl::screen_scale(win->screen_num());
  Fl::e_x = wl_fixed_to_int(surface_x) / f + delta_x;
  Fl::e_x_root = Fl::e_x + win->x();
  Fl::e_y = wl_fixed_to_int(surface_y) / f + delta_y;
  int *poffset = Fl_Window_Driver::menu_offset_y(win);
  if (poffset) Fl::e_y -= *poffset;
  Fl::e_y_root = Fl::e_y + win->y();
  return win;
}


static void pointer_enter(void *data,
        struct wl_pointer *wl_pointer,
        uint32_t serial,
        struct wl_surface *surface,
        wl_fixed_t surface_x,
        wl_fixed_t surface_y)
{
  Fl_Window *win = event_coords_from_surface(surface, surface_x, surface_y);
  if (!win) return;
  // use custom cursor if present
  struct wl_cursor *cursor = fl_wl_xid(win)->custom_cursor ? fl_wl_xid(win)->custom_cursor->wl_cursor : NULL;
  struct Fl_Wayland_Screen_Driver::seat *seat = (struct Fl_Wayland_Screen_Driver::seat*)data;
  do_set_cursor(seat, cursor);
  seat->serial = serial;
  seat->pointer_enter_serial = serial;
  set_event_xy(win);
  Fl::handle(FL_ENTER, win);
  //fprintf(stderr, "pointer_enter window=%p\n", win);
  seat->pointer_focus = surface;
}


static void pointer_leave(void *data,
        struct wl_pointer *wl_pointer,
        uint32_t serial,
        struct wl_surface *surface)
{
  struct Fl_Wayland_Screen_Driver::seat *seat = (struct Fl_Wayland_Screen_Driver::seat*)data;
  if (seat->pointer_focus == surface) seat->pointer_focus = NULL;
  Fl_Window *win = Fl_Wayland_Window_Driver::surface_to_window(surface);
  if (win) {
    Fl::belowmouse(0);
    set_event_xy(win);
    Fl::handle(FL_LEAVE, win->top_window());
  }
//fprintf(stderr, "pointer_leave surface=%p window=%p\n", surface, win);
}


static void pointer_motion(void *data,
         struct wl_pointer *wl_pointer,
         uint32_t time,
         wl_fixed_t surface_x,
         wl_fixed_t surface_y)
{
  struct Fl_Wayland_Screen_Driver::seat *seat = (struct Fl_Wayland_Screen_Driver::seat*)data;
  Fl_Window *win = event_coords_from_surface(seat->pointer_focus, surface_x, surface_y);
  if (!win) return;
  // If there's an active grab() and the pointer is in a window other than the grab(),
  // make e_x_root too large to be in any window
  if (Fl::grab() && !Fl::grab()->menu_window() && Fl::grab() != win) {
    Fl::e_x_root = 1000000;
  }
//fprintf(stderr, "FL_MOVE on win=%p to x:%dx%d root:%dx%d\n", win, Fl::e_x, Fl::e_y, Fl::e_x_root, Fl::e_y_root);
  wld_event_time = time;
  set_event_xy(win);
  Fl::handle(FL_MOVE, win);
}


//#include <FL/names.h>
static void pointer_button(void *data,
         struct wl_pointer *wl_pointer,
         uint32_t serial,
         uint32_t time,
         uint32_t button,
         uint32_t state)
{
  struct Fl_Wayland_Screen_Driver::seat *seat = (struct Fl_Wayland_Screen_Driver::seat*)data;
  seat->serial = serial;
  int event = 0;
  Fl_Window *win = Fl_Wayland_Window_Driver::surface_to_window(seat->pointer_focus);
  if (!win) return;
  win = win->top_window();
  wld_event_time = time;
  if (button == BTN_LEFT && state == WL_POINTER_BUTTON_STATE_PRESSED && seat->pointer_focus == NULL &&
      (fl_wl_xid(win))->kind == Fl_Wayland_Window_Driver::DECORATED) {
    // click on titlebar
    libdecor_frame_move(fl_wl_xid(win)->frame, seat->wl_seat, serial);
    return;
  }
  int b = 0;
  Fl::e_state &= ~FL_BUTTONS;
  if (state == WL_POINTER_BUTTON_STATE_PRESSED) {
    if (button == BTN_LEFT) {Fl::e_state |= FL_BUTTON1; b = 1;}
    else if (button == BTN_RIGHT) {Fl::e_state |= FL_BUTTON3; b = 3;}
    else if (button == BTN_MIDDLE) {Fl::e_state |= FL_BUTTON2; b = 2;}
    Fl::e_keysym = FL_Button + b;
  }
  Fl::e_dx = Fl::e_dy = 0;

  set_event_xy(win);
  if (state == WL_POINTER_BUTTON_STATE_PRESSED) {
    event = FL_PUSH;
    checkdouble();
  } else if (state == WL_POINTER_BUTTON_STATE_RELEASED) event = FL_RELEASE;
//fprintf(stderr, "%s %s\n", fl_eventnames[event], win->label() ? win->label():"[]");
  Fl::handle(event, win);
}

static void pointer_axis(void *data,
       struct wl_pointer *wl_pointer,
       uint32_t time,
       uint32_t axis,
       wl_fixed_t value)
{
  struct Fl_Wayland_Screen_Driver::seat *seat = (struct Fl_Wayland_Screen_Driver::seat*)data;
  Fl_Window *win = Fl_Wayland_Window_Driver::surface_to_window(seat->pointer_focus);
  if (!win) return;
  wld_event_time = time;
  int delta = wl_fixed_to_int(value) / 10;
//fprintf(stderr, "FL_MOUSEWHEEL: %c delta=%d\n", axis==WL_POINTER_AXIS_HORIZONTAL_SCROLL?'H':'V', delta);
  // allow both horizontal and vertical movements to be processed by the widget
  if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL) {
    Fl::e_dx = delta;
    Fl::e_dy = 0;
    Fl::handle(FL_MOUSEWHEEL, win->top_window());
  }
  if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL) {
    Fl::e_dx = 0;
    Fl::e_dy = delta;
    Fl::handle(FL_MOUSEWHEEL, win->top_window());
  }
}

static struct wl_pointer_listener pointer_listener = {
  pointer_enter,
  pointer_leave,
  pointer_motion,
  pointer_button,
  pointer_axis
};

static const char *proxy_tag = "FLTK for Wayland";

bool Fl_Wayland_Screen_Driver::own_output(struct wl_output *output)
{
  return wl_proxy_get_tag((struct wl_proxy *)output) == &proxy_tag;
}

static void init_cursors(struct Fl_Wayland_Screen_Driver::seat *seat);

static void try_update_cursor(struct Fl_Wayland_Screen_Driver::seat *seat)
{
  if (wl_list_empty(&seat->pointer_outputs)) return;
  struct pointer_output *pointer_output;
  int scale = 1;

  wl_list_for_each(pointer_output, &seat->pointer_outputs, link) {
    scale = fl_max(scale, pointer_output->output->wld_scale);
  }

  if (scale != seat->pointer_scale) {
    seat->pointer_scale = scale;
    init_cursors(seat);
    do_set_cursor(seat);
  }
}


static void output_scale(void *data, struct wl_output *wl_output, int32_t factor);


static void cursor_surface_enter(void *data,
        struct wl_surface *wl_surface,
        struct wl_output *wl_output)
{
  // Runs when the seat's cursor_surface enters a display
  struct Fl_Wayland_Screen_Driver::seat *seat = (struct Fl_Wayland_Screen_Driver::seat*)data;
  struct pointer_output *pointer_output;

  if (!Fl_Wayland_Screen_Driver::own_output(wl_output))
    return;

  pointer_output = (struct pointer_output *)calloc(1, sizeof(struct pointer_output));
  pointer_output->output = (Fl_Wayland_Screen_Driver::output *)wl_output_get_user_data(wl_output);
//fprintf(stderr, "cursor_surface_enter: wl_output_get_user_data(%p)=%p\n", wl_output, pointer_output->output);
  wl_list_insert(&seat->pointer_outputs, &pointer_output->link);
  try_update_cursor(seat);
  Fl_Wayland_Screen_Driver::output *output = (Fl_Wayland_Screen_Driver::output*)wl_output_get_user_data(wl_output);
  output_scale(output, wl_output, output->wld_scale); // rescale custom cursors
  // maintain custom or standard window cursor
  Fl_Window *win = Fl::first_window();
  if (win) {
    Fl_Wayland_Window_Driver *driver = Fl_Wayland_Window_Driver::driver(win);
    struct wld_window *xid = fl_wl_xid(win);
    if (xid->custom_cursor) do_set_cursor(seat, xid->custom_cursor->wl_cursor);
    else if (driver->cursor_default()) driver->set_cursor(driver->cursor_default());
    else win->cursor(driver->standard_cursor());
  }
}


static void cursor_surface_leave(void *data,
        struct wl_surface *wl_surface,
        struct wl_output *wl_output)
{
  struct Fl_Wayland_Screen_Driver::seat *seat = (struct Fl_Wayland_Screen_Driver::seat*)data;
  struct pointer_output *pointer_output, *tmp;

  wl_list_for_each_safe(pointer_output, tmp, &seat->pointer_outputs, link) {
    if (pointer_output->output->wl_output == wl_output) {
      wl_list_remove(&pointer_output->link);
      free(pointer_output);
    }
  }
  try_update_cursor(seat);
  // maintain custom window cursor
  Fl_Window *win = Fl::first_window();
  if (win) {
    struct wld_window *xid = fl_wl_xid(win);
    if (xid->custom_cursor) do_set_cursor(seat, xid->custom_cursor->wl_cursor);
  }

}

static struct wl_surface_listener cursor_surface_listener = {
  cursor_surface_enter,
  cursor_surface_leave,
};


static void init_cursors(struct Fl_Wayland_Screen_Driver::seat *seat)
{
  char *name;
  int size;
  struct wl_cursor_theme *theme;

  if (!libdecor_get_cursor_settings(&name, &size)) {
    name = NULL;
    size = 24;
  }
  size *= seat->pointer_scale;
  Fl_Wayland_Screen_Driver *scr_driver = (Fl_Wayland_Screen_Driver*)Fl::screen_driver();
  theme = wl_cursor_theme_load(name, size, scr_driver->wl_shm);
  free(name);
  //struct wl_cursor_theme *old_theme = seat->cursor_theme;
  if (theme != NULL) {
    if (seat->cursor_theme) {
     // caution to destroy theme because Fl_Wayland_Window_Driver::set_cursor(Fl_Cursor) caches used cursors
      scr_driver->reset_cursor();
      wl_cursor_theme_destroy(seat->cursor_theme);
    }
    seat->cursor_theme = theme;
  }
  if (seat->cursor_theme)
    seat->default_cursor = scr_driver->xc_arrow = wl_cursor_theme_get_cursor(seat->cursor_theme, "left_ptr");
  if (!seat->cursor_surface) {
    seat->cursor_surface = wl_compositor_create_surface(scr_driver->wl_compositor);
    wl_surface_add_listener(seat->cursor_surface, &cursor_surface_listener, seat);
  }
}


static void wl_keyboard_keymap(void *data, struct wl_keyboard *wl_keyboard,
               uint32_t format, int32_t fd, uint32_t size)
{
  struct Fl_Wayland_Screen_Driver::seat *seat = (struct Fl_Wayland_Screen_Driver::seat*)data;
  assert(format == WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1);

  char *map_shm = (char*)mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
  assert(map_shm != MAP_FAILED);

  struct xkb_keymap *xkb_keymap = xkb_keymap_new_from_string(seat->xkb_context, map_shm,
                                                             XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
  munmap(map_shm, size);
  close(fd);
  if (xkb_keymap) {
    struct xkb_state *xkb_state = xkb_state_new(xkb_keymap);
    xkb_keymap_unref(seat->xkb_keymap);
    if (seat->xkb_state) xkb_state_unref(seat->xkb_state);
    seat->xkb_keymap = xkb_keymap;
    seat->xkb_state = xkb_state;
  }
}


static int search_int_vector(Fl_Int_Vector& v, int val) {
  for (unsigned pos = 0; pos < v.size(); pos++) {
    if (v[pos] == val) return pos;
  }
  return -1;
}


static void remove_int_vector(Fl_Int_Vector& v, int val) {
  int pos = search_int_vector(v, val);
  if (pos < 0) return;
  int last = v.pop_back();
  if (last != val) v[pos] = last;
}


static int process_wld_key(struct xkb_state *xkb_state, uint32_t key,
                           uint32_t *p_keycode, xkb_keysym_t *p_sym) {
  uint32_t keycode = key + 8;
  xkb_keysym_t sym = xkb_state_key_get_one_sym(xkb_state, keycode);
  if (sym == 0xfe20) sym = FL_Tab;
  if (sym >= 'A' && sym <= 'Z') sym += 32; // replace uppercase by lowercase letter
  int for_key_vector = sym; // for support of Fl::event_key(int)
  // special processing for number keys == keycodes 10-19 :
  if (keycode >= 10 && keycode <= 18) {
    for_key_vector = '1' + (keycode - 10);
  } else if (keycode == 19) {
    for_key_vector = '0';
  }
  if (p_keycode) *p_keycode = keycode;
  if (p_sym) *p_sym = sym;
  return for_key_vector;
}


static void wl_keyboard_enter(void *data, struct wl_keyboard *wl_keyboard,
               uint32_t serial, struct wl_surface *surface,
               struct wl_array *keys)
{
  struct Fl_Wayland_Screen_Driver::seat *seat = (struct Fl_Wayland_Screen_Driver::seat*)data;
//fprintf(stderr, "keyboard enter fl_win=%p; keys pressed are: ", Fl_Wayland_Window_Driver::surface_to_window(surface));
  key_vector.size(0);
  // Replace wl_array_for_each(p, keys) rejected by C++
  for (uint32_t *p = (uint32_t *)(keys)->data;
      (const char *) p < ((const char *) (keys)->data + (keys)->size);
      (p)++) {
    int for_key_vector = process_wld_key(seat->xkb_state, *p, NULL, NULL);
//fprintf(stderr, "%d ", for_key_vector);
    if (search_int_vector(key_vector, for_key_vector) < 0) key_vector.push_back(for_key_vector);
  }
//fprintf(stderr, "\n");
  seat->keyboard_surface = surface;
  seat->keyboard_enter_serial = serial;
  Fl_Window *win = Fl_Wayland_Window_Driver::surface_to_window(surface);
  if (win) {
    Fl::handle(FL_FOCUS, win);
    fl_wl_find(fl_wl_xid(win));
  }
}

struct key_repeat_data_t {
  uint32_t time;
  Fl_Window *window;
};

#define KEY_REPEAT_DELAY 0.5 // sec
#define KEY_REPEAT_INTERVAL 0.05 // sec

static void key_repeat_timer_cb(key_repeat_data_t *key_repeat_data) {
  if ((Fl::event() == FL_KEYDOWN || (Fl_Window_Driver::menu_parent() && Fl::event() == FL_ENTER)) && wld_event_time == key_repeat_data->time) {
    Fl::handle(FL_KEYDOWN, key_repeat_data->window);
    Fl::add_timeout(KEY_REPEAT_INTERVAL, (Fl_Timeout_Handler)key_repeat_timer_cb, key_repeat_data);
  }
  else delete key_repeat_data;
}

int Fl_Wayland_Screen_Driver::next_marked_length = 0;

int Fl_Wayland_Screen_Driver::has_marked_text() const {
  return 1;
}

int Fl_Wayland_Screen_Driver::insertion_point_x = 0;
int Fl_Wayland_Screen_Driver::insertion_point_y = 0;
int Fl_Wayland_Screen_Driver::insertion_point_width = 0;
int Fl_Wayland_Screen_Driver::insertion_point_height = 0;
bool Fl_Wayland_Screen_Driver::insertion_point_location_is_valid = false;


// inform TIM about location of the insertion point, and memorize this info.
void Fl_Wayland_Screen_Driver::insertion_point_location(int x, int y, int height) {
//printf("insertion_point_location %dx%d\n",x,y);
  Fl_Wayland_Screen_Driver *scr_driver = (Fl_Wayland_Screen_Driver*)Fl::screen_driver();
  if (scr_driver->seat->text_input) {
    if (Fl::focus()) {
      Fl_Widget *focuswin = Fl::focus()->window();
      while (focuswin && focuswin->parent()) {
        x += focuswin->x(); y += focuswin->y();
        focuswin = focuswin->window();
      }
    }
    float s = fl_graphics_driver->scale();
    insertion_point_location_is_valid = true;
    insertion_point_x = s*x;
    insertion_point_y = s*(y-height);
    insertion_point_width = s*5;
    insertion_point_height = s*height;
    if (zwp_text_input_v3_get_user_data(scr_driver->seat->text_input) ) {
      zwp_text_input_v3_set_cursor_rectangle(scr_driver->seat->text_input, insertion_point_x,
        insertion_point_y, insertion_point_width, insertion_point_height);
      zwp_text_input_v3_commit(scr_driver->seat->text_input);
    }
  }
}


// computes window coordinates & size of insertion point
bool Fl_Wayland_Screen_Driver::insertion_point_location(int *px, int *py, int *pwidth, int *pheight)
// return true if the current coordinates and size of the insertion point are available
{
  if ( ! insertion_point_location_is_valid ) return false;
  *px = insertion_point_x;
  *py = insertion_point_y;
  *pwidth = insertion_point_width;
  *pheight = insertion_point_height;
  return true;
}

int Fl_Wayland_Screen_Driver::compose(int& del) {
  unsigned char ascii = (unsigned char)Fl::e_text[0];
  int condition = (Fl::e_state & (FL_ALT | FL_META | FL_CTRL)) && ascii < 128 ; // letter+modifier key
  condition |= (Fl::e_keysym >= FL_Shift_L && Fl::e_keysym <= FL_Alt_R); // pressing modifier key
  condition |= (Fl::e_keysym >= FL_Home && Fl::e_keysym <= FL_Help);
  condition |= Fl::e_keysym == FL_Tab;
//fprintf(stderr, "compose: condition=%d e_state=%x ascii=%d\n", condition, Fl::e_state, ascii);
  if (condition) { del = 0; return 0;}
//fprintf(stderr, "compose: del=%d compose_state=%d next_marked_length=%d \n", del, Fl::compose_state, next_marked_length);
  del = Fl::compose_state;
  Fl::compose_state = next_marked_length;
  // no-underlined-text && (ascii non-printable || ascii == delete)
  if (ascii && (!Fl::compose_state) && (ascii <= 31 || ascii == 127)) { del = 0; return 0; }
  return 1;
}

void Fl_Wayland_Screen_Driver::compose_reset()
{
  if (!Fl_Wayland_Screen_Driver::wl_registry) open_display();
  Fl::compose_state = 0;
  next_marked_length = 0;
  xkb_compose_state_reset(seat->xkb_compose_state);
}

struct dead_key_struct {
  xkb_keysym_t keysym; // the keysym obtained when hitting a dead key
  const char *marked_text; // the temporary text to display for that dead key
};

static dead_key_struct dead_keys[] = {
  {XKB_KEY_dead_grave, "`"},
  {XKB_KEY_dead_acute, "´"},
  {XKB_KEY_dead_circumflex, "^"},
  {XKB_KEY_dead_tilde, "~"},
  {XKB_KEY_dead_macron, "¯"},
  {XKB_KEY_dead_breve, "˘"},
  {XKB_KEY_dead_abovedot, "˙"},
  {XKB_KEY_dead_diaeresis, "¨"},
  {XKB_KEY_dead_abovering, "˚"},
  {XKB_KEY_dead_doubleacute, "˝"},
  {XKB_KEY_dead_caron, "ˇ"},
  {XKB_KEY_dead_cedilla, "¸"},
  {XKB_KEY_dead_ogonek, "˛"},
  {XKB_KEY_dead_iota, "ι"},
  {XKB_KEY_dead_doublegrave, " ̏"},
};

const int dead_key_count = sizeof(dead_keys)/sizeof(struct dead_key_struct);


static void wl_keyboard_key(void *data, struct wl_keyboard *wl_keyboard,
               uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
  struct Fl_Wayland_Screen_Driver::seat *seat = (struct Fl_Wayland_Screen_Driver::seat*)data;
  seat->serial = serial;
  static char buf[128];
  uint32_t keycode;
  xkb_keysym_t sym;
  int for_key_vector = process_wld_key(seat->xkb_state, key, &keycode, &sym);
/*xkb_keysym_get_name(sym, buf, sizeof(buf));
const char *action = (state == WL_KEYBOARD_KEY_STATE_PRESSED ? "press" : "release");
fprintf(stderr, "key %s: sym: %-12s(%d) code:%u fl_win=%p, ", action, buf, sym, keycode, Fl_Wayland_Window_Driver::surface_to_window(seat->keyboard_surface));*/
  xkb_state_key_get_utf8(seat->xkb_state, keycode, buf, sizeof(buf));
//fprintf(stderr, "utf8: '%s' e_length=%d [%d]\n", buf, (int)strlen(buf), *buf);
  Fl::e_keysym = for_key_vector;
  if (state == WL_KEYBOARD_KEY_STATE_PRESSED) {
    if (search_int_vector(key_vector, for_key_vector) < 0) key_vector.push_back(for_key_vector);
  } else {
    remove_int_vector(key_vector, for_key_vector);
  }
  Fl::e_text = buf;
  Fl::e_length = strlen(buf);
  // Process dead keys and compose sequences :
  enum xkb_compose_status status = XKB_COMPOSE_NOTHING;
  // This part is useful only if the compositor doesn't support protocol text-input-unstable-v3
  if (seat->xkb_compose_state && state == WL_KEYBOARD_KEY_STATE_PRESSED &&
      !(sym >= FL_Shift_L && sym <= FL_Alt_R) && sym != XKB_KEY_ISO_Level3_Shift) {
    xkb_compose_state_feed(seat->xkb_compose_state, sym);
    status = xkb_compose_state_get_status(seat->xkb_compose_state);
    if (status == XKB_COMPOSE_COMPOSING) {
      if (Fl::e_length == 0) { // dead keys produce e_length = 0
        int i;
        for (i = 0; i < dead_key_count; i++) {
          if (dead_keys[i].keysym == sym) break;
        }
        if (i < dead_key_count) strcpy(buf, dead_keys[i].marked_text);
        else buf[0] = 0;
        Fl::e_length = strlen(buf);
        Fl::compose_state = 0;
      }
      Fl_Wayland_Screen_Driver::next_marked_length = Fl::e_length;
    } else if (status == XKB_COMPOSE_COMPOSED) {
      Fl::e_length = xkb_compose_state_get_utf8(seat->xkb_compose_state, buf, sizeof(buf));
      Fl::compose_state = Fl_Wayland_Screen_Driver::next_marked_length;
      Fl_Wayland_Screen_Driver::next_marked_length = 0;
    } else if (status == XKB_COMPOSE_CANCELLED) {
      Fl::e_length = 0;
      Fl::compose_state = Fl_Wayland_Screen_Driver::next_marked_length;
      Fl_Wayland_Screen_Driver::next_marked_length = 0;
    }
//fprintf(stderr, "xkb_compose_status=%d ctxt=%p state=%p l=%d[%s]\n", status, seat->xkb_context, seat->xkb_compose_state, Fl::e_length, buf);
  }
  // end of part used only without text-input-unstable-v3

  wld_event_time = time;
  int event = (state == WL_KEYBOARD_KEY_STATE_PRESSED ? FL_KEYDOWN : FL_KEYUP);
  // Send event to focus-containing top window as defined by FLTK,
  // otherwise send it to Wayland-defined focus window
  Fl_Window *win = ( Fl::focus() ? Fl::focus()->top_window() : Fl_Wayland_Window_Driver::surface_to_window(seat->keyboard_surface) );
  if (win) {
    set_event_xy(win);
    Fl::e_is_click = 0;
    Fl::handle(event, win);
  }
  if (event == FL_KEYDOWN && status == XKB_COMPOSE_NOTHING && !(sym >= FL_Shift_L && sym <= FL_Alt_R)) {
    key_repeat_data_t *key_repeat_data = new key_repeat_data_t;
    key_repeat_data->time = time;
    key_repeat_data->window = win;
    Fl::add_timeout(KEY_REPEAT_DELAY, (Fl_Timeout_Handler)key_repeat_timer_cb, key_repeat_data);
  }
}

static void wl_keyboard_leave(void *data, struct wl_keyboard *wl_keyboard,
               uint32_t serial, struct wl_surface *surface)
{
  struct Fl_Wayland_Screen_Driver::seat *seat = (struct Fl_Wayland_Screen_Driver::seat*)data;
//fprintf(stderr, "keyboard leave fl_win=%p\n", Fl_Wayland_Window_Driver::surface_to_window(surface));
  seat->keyboard_surface = NULL;
  Fl_Window *win = Fl_Wayland_Window_Driver::surface_to_window(surface);
  if (!win && Fl::focus()) win = Fl::focus()->top_window();
  if (win) Fl::handle(FL_UNFOCUS, win);
  key_vector.size(0);
}

static void wl_keyboard_modifiers(void *data, struct wl_keyboard *wl_keyboard,
               uint32_t serial, uint32_t mods_depressed,
               uint32_t mods_latched, uint32_t mods_locked,
               uint32_t group)
{
  struct Fl_Wayland_Screen_Driver::seat *seat = (struct Fl_Wayland_Screen_Driver::seat*)data;
  xkb_state_update_mask(seat->xkb_state, mods_depressed, mods_latched, mods_locked, 0, 0, group);
  Fl::e_state &= ~(FL_SHIFT+FL_CTRL+FL_ALT+FL_CAPS_LOCK+FL_NUM_LOCK);
  if (xkb_state_mod_name_is_active(seat->xkb_state, XKB_MOD_NAME_SHIFT, XKB_STATE_MODS_DEPRESSED))
    Fl::e_state |= FL_SHIFT;
  if (xkb_state_mod_name_is_active(seat->xkb_state, XKB_MOD_NAME_CTRL, XKB_STATE_MODS_DEPRESSED))
    Fl::e_state |= FL_CTRL;
  if (xkb_state_mod_name_is_active(seat->xkb_state, XKB_MOD_NAME_ALT, XKB_STATE_MODS_DEPRESSED))
    Fl::e_state |= FL_ALT;
  if (xkb_state_mod_name_is_active(seat->xkb_state, XKB_MOD_NAME_CAPS, XKB_STATE_MODS_LOCKED))
    Fl::e_state |= FL_CAPS_LOCK;
  if (xkb_state_mod_name_is_active(seat->xkb_state, XKB_MOD_NAME_NUM, XKB_STATE_MODS_LOCKED))
    Fl::e_state |= FL_NUM_LOCK;
//fprintf(stderr, "mods_depressed=%u Fl::e_state=%X\n", mods_depressed, Fl::e_state);
}

static void wl_keyboard_repeat_info(void *data, struct wl_keyboard *wl_keyboard, int32_t rate, int32_t delay)
{
  // wl_keyboard is version 3 under Debian, but that event isn't sent until version 4
}

static const struct wl_keyboard_listener wl_keyboard_listener = {
       .keymap = wl_keyboard_keymap,
       .enter = wl_keyboard_enter,
       .leave = wl_keyboard_leave,
       .key = wl_keyboard_key,
       .modifiers = wl_keyboard_modifiers,
       .repeat_info = wl_keyboard_repeat_info,
};


void text_input_enter(void *data, struct zwp_text_input_v3 *zwp_text_input_v3,
                      struct wl_surface *surface) {
//puts("text_input_enter");
  zwp_text_input_v3_set_user_data(zwp_text_input_v3, surface);
  zwp_text_input_v3_enable(zwp_text_input_v3);
  int x, y, width, height;
  if (Fl_Wayland_Screen_Driver::insertion_point_location(&x, &y, &width, &height)) {
    zwp_text_input_v3_set_cursor_rectangle(zwp_text_input_v3,  x,  y,  width, height);
  }
  zwp_text_input_v3_commit(zwp_text_input_v3);
  wl_display_roundtrip(Fl_Wayland_Screen_Driver::wl_display);
}

void text_input_leave(void *data, struct zwp_text_input_v3 *zwp_text_input_v3,
                      struct wl_surface *surface) {
//puts("text_input_leave");
  zwp_text_input_v3_disable(zwp_text_input_v3);
  zwp_text_input_v3_set_user_data(zwp_text_input_v3, NULL);
  zwp_text_input_v3_commit(zwp_text_input_v3);
}

void text_input_preedit_string(void *data, struct zwp_text_input_v3 *zwp_text_input_v3,
           const char *text, int32_t cursor_begin, int32_t cursor_end) {
//printf("text_input_preedit_string %s cursor_begin=%d cursor_end=%d\n",text, cursor_begin, cursor_end);
  // goes to widget as marked text
  Fl_Wayland_Screen_Driver::next_marked_length = text ? strlen(text) : 0;
  Fl::e_text = text ? (char*)text : (char*)"";
  Fl::e_length = text ? strlen(text) : 0;
  Fl::e_keysym = 'a'; // fake a simple key
  struct wl_surface *surface = (struct wl_surface*)data;
  Fl_Window *win =  Fl_Wayland_Window_Driver::surface_to_window(surface);
  set_event_xy(win);
  Fl::e_is_click = 0;
  Fl::handle(FL_KEYDOWN, win);
}

void text_input_commit_string(void *data, struct zwp_text_input_v3 *zwp_text_input_v3,
                              const char *text) {
//printf("text_input_commit_string %s\n",text);
  Fl::e_text = (char*)text;
  Fl::e_length = strlen(text);
  struct wl_surface *surface = (struct wl_surface*)data;
  Fl_Window *win =  Fl_Wayland_Window_Driver::surface_to_window(surface);
  set_event_xy(win);
  Fl::e_is_click = 0;
  Fl::handle(FL_KEYDOWN, win);
  zwp_text_input_v3_commit(zwp_text_input_v3);
  Fl_Wayland_Screen_Driver::next_marked_length = 0;
  Fl::compose_state = 0;
}

void text_input_delete_surrounding_text(void *data, struct zwp_text_input_v3 *zwp_text_input_v3,
        uint32_t before_length, uint32_t after_length) {
  fprintf(stderr, "delete_surrounding_text before=%d adfter=%d\n",before_length,after_length);
}

void text_input_done(void *data, struct zwp_text_input_v3 *zwp_text_input_v3,
                     uint32_t serial) {
//puts("text_input_done");
}

static const struct zwp_text_input_v3_listener text_input_listener = {
  .enter = text_input_enter,
  .leave = text_input_leave,
  .preedit_string = text_input_preedit_string,
  .commit_string = text_input_commit_string,
  .delete_surrounding_text = text_input_delete_surrounding_text,
  .done = text_input_done,
};


void Fl_Wayland_Screen_Driver::enable_im() {
  if (text_input_base && !seat->text_input) {
    seat->text_input = zwp_text_input_manager_v3_get_text_input(text_input_base, seat->wl_seat);
    //printf("seat->text_input=%p\n",seat->text_input);
    zwp_text_input_v3_add_listener(seat->text_input, &text_input_listener, NULL);
  }
}


void Fl_Wayland_Screen_Driver::disable_im() {
  if (seat->text_input) {
    zwp_text_input_v3_disable(seat->text_input);
    zwp_text_input_v3_commit(seat->text_input);
    zwp_text_input_v3_destroy(seat->text_input);
    seat->text_input = NULL;
  }
}


static void seat_capabilities(void *data, struct wl_seat *wl_seat, uint32_t capabilities)
{
  struct Fl_Wayland_Screen_Driver::seat *seat = (struct Fl_Wayland_Screen_Driver::seat*)data;
  if ((capabilities & WL_SEAT_CAPABILITY_POINTER) && !seat->wl_pointer) {
    seat->wl_pointer = wl_seat_get_pointer(wl_seat);
    wl_pointer_add_listener(seat->wl_pointer, &pointer_listener, seat);
    seat->pointer_scale = 1;
    init_cursors(seat);
  } else if (!(capabilities & WL_SEAT_CAPABILITY_POINTER) && seat->wl_pointer) {
    wl_pointer_release(seat->wl_pointer);
    seat->wl_pointer = NULL;
  }

  bool have_keyboard = seat->xkb_context && (capabilities & WL_SEAT_CAPABILITY_KEYBOARD);
  if (have_keyboard && seat->wl_keyboard == NULL) {
          seat->wl_keyboard = wl_seat_get_keyboard(wl_seat);
          wl_keyboard_add_listener(seat->wl_keyboard,
                          &wl_keyboard_listener, seat);
//fprintf(stderr, "wl_keyboard version=%d\n", wl_keyboard_get_version(seat->wl_keyboard));

  } else if (!have_keyboard && seat->wl_keyboard != NULL) {
          wl_keyboard_release(seat->wl_keyboard);
          seat->wl_keyboard = NULL;
  }
  Fl_Wayland_Screen_Driver *scr_driver = (Fl_Wayland_Screen_Driver*)Fl::screen_driver();
  scr_driver->enable_im();
}

static void seat_name(void *data, struct wl_seat *wl_seat, const char *name) {
  struct Fl_Wayland_Screen_Driver::seat *seat = (struct Fl_Wayland_Screen_Driver::seat*)data;
  seat->name = strdup(name);
}

static struct wl_seat_listener seat_listener = {
  seat_capabilities,
  seat_name
};

static void output_geometry(void *data,
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
  //fprintf(stderr, "output_geometry: x=%d y=%d physical=%dx%d\n",x,y,physical_width,physical_height);
  Fl_Wayland_Screen_Driver::output *output = (Fl_Wayland_Screen_Driver::output*)data;
  output->x = int(x);
  output->y = int(y);
  output->dpi = 96; // to elaborate
}

static void output_mode(void *data, struct wl_output *wl_output, uint32_t flags,
      int32_t width, int32_t height, int32_t refresh)
{
  Fl_Wayland_Screen_Driver::output *output = (Fl_Wayland_Screen_Driver::output*)data;
  output->width = int(width);
  output->height = int(height);
//fprintf(stderr, "output_mode: [%p]=%dx%d\n",output->wl_output,width,height);
}

static void output_done(void *data, struct wl_output *wl_output)
{
  // Runs at startup and when desktop scale factor is changed
  Fl_Wayland_Screen_Driver::output *output = (Fl_Wayland_Screen_Driver::output*)data;
//fprintf(stderr, "output_done output=%p\n",output);
  Fl_X *xp = Fl_X::first;
  while (xp) { // all mapped windows
    struct wld_window *win = (struct wld_window*)xp->xid;
    Fl_Window *W = win->fl_win;
    if (win->buffer || W->as_gl_window()) {
      if (W->as_gl_window()) {
        wl_surface_set_buffer_scale(win->wl_surface, output->wld_scale);
        Fl_Window_Driver::driver(W)->is_a_rescale(true);
        W->resize(W->x(), W->y(), W->w(), W->h());
        Fl_Window_Driver::driver(W)->is_a_rescale(false);
      } else {
        Fl_Wayland_Graphics_Driver::buffer_release(win);
      }
      W->redraw();
      Fl_Window_Driver::driver(W)->flush();
    }
    xp = xp->next;
  }
  output->done = true;

  Fl_Wayland_Screen_Driver *scr_driver = (Fl_Wayland_Screen_Driver*)Fl::screen_driver();
  if (scr_driver->seat) try_update_cursor(scr_driver->seat);
  scr_driver->init_workarea();
  Fl::handle(FL_SCREEN_CONFIGURATION_CHANGED, NULL);
}


static void output_scale(void *data, struct wl_output *wl_output, int32_t factor) {
  Fl_Wayland_Screen_Driver::output *output = (Fl_Wayland_Screen_Driver::output*)data;
  output->wld_scale = factor;
//fprintf(stderr,"output_scale: wl_output=%p factor=%d\n",wl_output, factor);
  // rescale cursors of windows that map here and have a custom cursor
  Fl_Window *win = Fl::first_window();
  while (win) {
    struct wld_window *xid = fl_wl_xid(win);
    struct Fl_Wayland_Window_Driver::surface_output *s_output;
    // get 1st screen where window appears
    s_output = wl_container_of(xid->outputs.next, s_output, link);
    if (xid->custom_cursor && output == s_output->output) {
      Fl_Wayland_Window_Driver *driver = Fl_Wayland_Window_Driver::driver(win);
      driver->set_cursor_4args(xid->custom_cursor->rgb,
                               xid->custom_cursor->hotx, xid->custom_cursor->hoty, false);
    };
    win = Fl::next_window(win);
  }
}


static struct wl_output_listener output_listener = {
  output_geometry,
  output_mode,
  output_done,
  output_scale
};


// Notice: adding use of unstable protocol "XDG output" would allow FLTK to be notified
// in real time of changes to the relative location of multiple displays;
// with the present code, that information is received at startup only.
static void registry_handle_global(void *user_data, struct wl_registry *wl_registry,
           uint32_t id, const char *interface, uint32_t version) {
//fprintf(stderr, "interface=%s version=%u\n", interface, version);
  Fl_Wayland_Screen_Driver *scr_driver = (Fl_Wayland_Screen_Driver*)Fl::screen_driver();
  if (strcmp(interface, "wl_compositor") == 0) {
    if (version < 4) {
      Fl::fatal("wl_compositor version >= 4 required");
    }
    scr_driver->wl_compositor = (struct wl_compositor*)wl_registry_bind(wl_registry,
           id, &wl_compositor_interface, 4);

  } else if (strcmp(interface, "wl_subcompositor") == 0) {
    scr_driver->wl_subcompositor = (struct wl_subcompositor*)wl_registry_bind(wl_registry,
           id, &wl_subcompositor_interface, 1);

  } else if (strcmp(interface, "wl_shm") == 0) {
    scr_driver->wl_shm = (struct wl_shm*)wl_registry_bind(wl_registry,
            id, &wl_shm_interface, 1);

  } else if (strcmp(interface, "wl_seat") == 0) {
    if (version < 2) {
      Fl::fatal("%s version 2 required but only version %i is available\n", interface, version);
    }
    if (!scr_driver->seat) scr_driver->seat = (struct Fl_Wayland_Screen_Driver::seat*)calloc(1, sizeof(struct Fl_Wayland_Screen_Driver::seat));
//fprintf(stderr, "registry_handle_global: seat=%p\n", scr_driver->seat);
    wl_list_init(&scr_driver->seat->pointer_outputs);
    scr_driver->seat->wl_seat = (wl_seat*)wl_registry_bind(wl_registry, id, &wl_seat_interface, 2);
    scr_driver->seat->xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (scr_driver->seat->xkb_context) {
      const char *locale = getenv("LC_ALL");
      if (!locale || !*locale)
        locale = getenv("LC_CTYPE");
      if (!locale || !*locale)
        locale = getenv("LANG");
      if (!locale || !*locale)
        locale = "C";
      struct xkb_compose_table *table = xkb_compose_table_new_from_locale(scr_driver->seat->xkb_context, locale, XKB_COMPOSE_COMPILE_NO_FLAGS);
      if (table) scr_driver->seat->xkb_compose_state = xkb_compose_state_new(table, XKB_COMPOSE_STATE_NO_FLAGS);
    }
    wl_seat_add_listener(scr_driver->seat->wl_seat, &seat_listener, scr_driver->seat);
    if (scr_driver->seat->data_device_manager) {
      scr_driver->seat->data_device = wl_data_device_manager_get_data_device(scr_driver->seat->data_device_manager, scr_driver->seat->wl_seat);
      wl_data_device_add_listener(scr_driver->seat->data_device, Fl_Wayland_Screen_Driver::p_data_device_listener, NULL);
    }

  } else if (strcmp(interface, wl_data_device_manager_interface.name) == 0) {
    if (!scr_driver->seat) scr_driver->seat = (struct Fl_Wayland_Screen_Driver::seat*)calloc(1, sizeof(struct Fl_Wayland_Screen_Driver::seat));
    scr_driver->seat->data_device_manager = (struct wl_data_device_manager*)wl_registry_bind(wl_registry, id, &wl_data_device_manager_interface, fl_min(version, 3));
    if (scr_driver->seat->wl_seat) {
      scr_driver->seat->data_device = wl_data_device_manager_get_data_device(scr_driver->seat->data_device_manager, scr_driver->seat->wl_seat);
      wl_data_device_add_listener(scr_driver->seat->data_device, Fl_Wayland_Screen_Driver::p_data_device_listener, NULL);
    }
//fprintf(stderr, "registry_handle_global: %s\n", interface);

  } else if (strcmp(interface, "wl_output") == 0) {
    if (version < 2) {
      Fl::fatal("%s version 2 required but only version %i is available\n", interface, version);
    }
    Fl_Wayland_Screen_Driver::output *output = (Fl_Wayland_Screen_Driver::output*)calloc(1, sizeof *output);
    output->id = id;
    output->wld_scale = 1;
    output->wl_output = (struct wl_output*)wl_registry_bind(wl_registry,
                 id, &wl_output_interface, 2);
    output->gui_scale = 1.f;
    wl_proxy_set_tag((struct wl_proxy *) output->wl_output, &proxy_tag);
    wl_output_add_listener(output->wl_output, &output_listener, output);
    wl_list_insert(&(scr_driver->outputs), &output->link);
    scr_driver->screen_count_set( wl_list_length(&(scr_driver->outputs)) );
//fprintf(stderr, "wl_output: id=%d wl_output=%p screen_count()=%d\n", id, output->wl_output, Fl::screen_count());

  } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
//fprintf(stderr, "registry_handle_global interface=%s\n", interface);
    scr_driver->xdg_wm_base = (struct xdg_wm_base *)wl_registry_bind(wl_registry, id, &xdg_wm_base_interface, 1);
      xdg_wm_base_add_listener(scr_driver->xdg_wm_base, &xdg_wm_base_listener, NULL);
  } else if (strcmp(interface, "gtk_shell1") == 0) {
    Fl_Wayland_Screen_Driver::compositor = Fl_Wayland_Screen_Driver::MUTTER;
    //fprintf(stderr, "Running the Mutter compositor\n");
  } else if (strcmp(interface, "weston_desktop_shell") == 0) {
    Fl_Wayland_Screen_Driver::compositor = Fl_Wayland_Screen_Driver::WESTON;
    //fprintf(stderr, "Running the Weston compositor\n");
  } else if (strcmp(interface, "org_kde_plasma_shell") == 0) {
    Fl_Wayland_Screen_Driver::compositor = Fl_Wayland_Screen_Driver::KDE;
    //fprintf(stderr, "Running the KDE compositor\n");
  } else if (strncmp(interface, "zowl_mach_ipc", 13) == 0) {
    Fl_Wayland_Screen_Driver::compositor = Fl_Wayland_Screen_Driver::OWL;
    //fprintf(stderr, "Running the Owl compositor\n");
    if (wl_list_length(&scr_driver->outputs) == 0) {
      Fl_Wayland_Screen_Driver::output *output = (Fl_Wayland_Screen_Driver::output*)calloc(1, sizeof *output);
      output->id = 1;
      output->wld_scale = 1;
      output->gui_scale = 1.f;
      output->width = 1440; output->height = 900;
      output->done = true;
      wl_list_insert(&(scr_driver->outputs), &output->link);
      scr_driver->screen_count_set(1);
    }
  } else if (strcmp(interface, zwp_text_input_manager_v3_interface.name) == 0) {
    scr_driver->text_input_base = (struct zwp_text_input_manager_v3 *) wl_registry_bind(wl_registry, id, &zwp_text_input_manager_v3_interface, 1);
    //printf("scr_driver->text_input_base=%p version=%d\n",scr_driver->text_input_base,version);
  }
}


static void registry_handle_global_remove(void *data, struct wl_registry *registry, uint32_t name)
{//TODO to be tested
  Fl_Wayland_Screen_Driver::output *output, *tmp;
//fprintf(stderr, "registry_handle_global_remove data=%p id=%u\n", data, name);
  Fl_Wayland_Screen_Driver *scr_driver = (Fl_Wayland_Screen_Driver*)Fl::screen_driver();
  wl_list_for_each_safe(output, tmp, &(scr_driver->outputs), link) { // all screens of the system
    if (output->id == name) { // the screen being removed
      again:
      Fl_X *xp = Fl_X::first;
      while (xp) { // all mapped windows
        struct wld_window *win = (struct wld_window*)xp->xid;
        struct Fl_Wayland_Window_Driver::surface_output *s_output;
        wl_list_for_each(s_output, &win->outputs, link) {
          if (output == s_output->output) {
            delete win->fl_win;
            goto again;
          }
      }
        xp = xp->next;
      }
      wl_list_remove(&output->link);
      scr_driver->screen_count_set( wl_list_length(&(scr_driver->outputs)) );
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


static void wayland_socket_callback(int fd, struct wl_display *display) {
  struct pollfd fds = (struct pollfd) { fd, POLLIN, 0 };
  do {
    if (wl_display_dispatch(display) == -1) {
      Fl::fatal("Fatal error while communicating with the Wayland server: %s",
                strerror(errno));
    }
  }
  while (poll(&fds, 1, 0) > 0);
}


Fl_Wayland_Screen_Driver::Fl_Wayland_Screen_Driver() : Fl_Unix_Screen_Driver() {
  libdecor_context = NULL;
  seat = NULL;
  text_input_base = NULL;
  reset_cursor();
  wl_registry = NULL;
}


static void sync_done(void *data, struct wl_callback *cb, uint32_t time) {
  // runs after all calls to registry_handle_global()
  *(struct wl_callback **)data = NULL;
  wl_callback_destroy(cb);
  // keep processing until output_done() has run for each screen
  Fl_Wayland_Screen_Driver *scr_driver = (Fl_Wayland_Screen_Driver*)Fl::screen_driver();
  Fl_Wayland_Screen_Driver::output *output;
  wl_list_for_each(output, &scr_driver->outputs, link) { // each screen of the system
    while (!output->done) wl_display_dispatch(Fl_Wayland_Screen_Driver::wl_display);
  }
}


static const struct wl_callback_listener sync_listener = {
  sync_done
};


void Fl_Wayland_Screen_Driver::open_display_platform() {
  static bool beenHereDoneThat = false;
  if (beenHereDoneThat)
    return;

  beenHereDoneThat = true;

  if (!wl_display) {
    wl_display = wl_display_connect(NULL);
    if (!wl_display) {
      Fl::fatal("No Wayland connection\n");
    }
  }
  //puts("Using Wayland backend");
  wl_list_init(&outputs);

  wl_registry = wl_display_get_registry(wl_display);
  wl_registry_add_listener(wl_registry, &registry_listener, NULL);
  struct wl_callback *registry_cb = wl_display_sync(wl_display);
  wl_callback_add_listener(registry_cb, &sync_listener, &registry_cb);
  while (registry_cb) wl_display_dispatch(wl_display);
  Fl::add_fd(wl_display_get_fd(wl_display), FL_READ, (Fl_FD_Handler)wayland_socket_callback,
             wl_display);
  fl_create_print_window();
}


void Fl_Wayland_Screen_Driver::close_display() {
  if (text_input_base) {
    disable_im();
    zwp_text_input_manager_v3_destroy(text_input_base);
    text_input_base = NULL;
  }
  while (wl_list_length(&outputs) > 0) {
    Fl_Wayland_Screen_Driver::output *output;
    wl_list_for_each(output, &outputs, link) {
      wl_list_remove(&output->link);
      screen_count_set( wl_list_length(&outputs) );
      if (output->wl_output) wl_output_destroy(output->wl_output);
      free(output);
      break;
    }
  }
  wl_subcompositor_destroy(wl_subcompositor); wl_subcompositor = NULL;
  wl_surface_destroy(seat->cursor_surface); seat->cursor_surface = NULL;
  if (seat->cursor_theme) {
    wl_cursor_theme_destroy(seat->cursor_theme);
    seat->cursor_theme = NULL;
  }
  wl_compositor_destroy(wl_compositor); wl_compositor = NULL;
  wl_shm_destroy(wl_shm); wl_shm = NULL;
  if (seat->wl_keyboard) {
    if (seat->xkb_state) {
      xkb_state_unref(seat->xkb_state);
      seat->xkb_state = NULL;
    }
    if (seat->xkb_keymap) {
      xkb_keymap_unref(seat->xkb_keymap);
      seat->xkb_keymap = NULL;
    }
    wl_keyboard_destroy(seat->wl_keyboard);
    seat->wl_keyboard = NULL;
  }
  wl_pointer_destroy(seat->wl_pointer); seat->wl_pointer = NULL;
  if (seat->xkb_compose_state) {
    xkb_compose_state_unref(seat->xkb_compose_state);
    seat->xkb_compose_state = NULL;
  }
  if (seat->xkb_context) {
    xkb_context_unref(seat->xkb_context);
    seat->xkb_context = NULL;
  }
  if (seat->data_source) {
    wl_data_source_destroy(seat->data_source);
    seat->data_source = NULL;
  }
  wl_data_device_destroy(seat->data_device); seat->data_device = NULL;
  wl_data_device_manager_destroy(seat->data_device_manager); seat->data_device_manager = NULL;
  wl_seat_destroy(seat->wl_seat); seat->wl_seat = NULL;
  if (seat->name) free(seat->name);
  delete seat; seat = NULL;
  if (libdecor_context) {
    libdecor_unref(libdecor_context);
    libdecor_context = NULL;
  }
  xdg_wm_base_destroy(xdg_wm_base); xdg_wm_base = NULL;
  Fl_Wayland_Plugin *plugin = Fl_Wayland_Window_Driver::gl_plugin();
  if (plugin) plugin->terminate();

  Fl::remove_fd(wl_display_get_fd(Fl_Wayland_Screen_Driver::wl_display));
  wl_registry_destroy(wl_registry); wl_registry = NULL;
  wl_display_disconnect(Fl_Wayland_Screen_Driver::wl_display);
  Fl_Wayland_Screen_Driver::wl_display = NULL;
}


static int workarea_xywh[4] = { -1, -1, -1, -1 };


void Fl_Wayland_Screen_Driver::init_workarea()
{
  Fl_Wayland_Screen_Driver::output *output;
  wl_list_for_each(output, &outputs, link) {
    workarea_xywh[0] = output->x; // pixels
    workarea_xywh[1] = output->y; // pixels
    workarea_xywh[2] = output->width; // pixels
    workarea_xywh[3] = output->height; // pixels
    break;
  }
}


int Fl_Wayland_Screen_Driver::x() {
  if (!Fl_Wayland_Screen_Driver::wl_registry) open_display();
  Fl_Wayland_Screen_Driver::output *output;
  wl_list_for_each(output, &outputs, link) {
    break;
  }
  return workarea_xywh[0] / (output->gui_scale * output->wld_scale);
}

int Fl_Wayland_Screen_Driver::y() {
  if (!Fl_Wayland_Screen_Driver::wl_registry) open_display();
  Fl_Wayland_Screen_Driver::output *output;
  wl_list_for_each(output, &outputs, link) {
    break;
  }
  return workarea_xywh[1] / (output->gui_scale * output->wld_scale);
}

int Fl_Wayland_Screen_Driver::w() {
  if (!Fl_Wayland_Screen_Driver::wl_registry) open_display();
  Fl_Wayland_Screen_Driver::output *output;
  wl_list_for_each(output, &outputs, link) {
    break;
  }
  return workarea_xywh[2] / (output->gui_scale * output->wld_scale);
}

int Fl_Wayland_Screen_Driver::h() {
  if (!Fl_Wayland_Screen_Driver::wl_registry) open_display();
  Fl_Wayland_Screen_Driver::output *output;
  wl_list_for_each(output, &outputs, link) {
    break;
  }
  return workarea_xywh[3] / (output->gui_scale * output->wld_scale);
}


void Fl_Wayland_Screen_Driver::init() {
  if (!Fl_Wayland_Screen_Driver::wl_registry) open_display();
}


void Fl_Wayland_Screen_Driver::screen_work_area(int &X, int &Y, int &W, int &H, int n)
{
  if (num_screens < 0) init();
  if (n < 0 || n >= num_screens) n = 0;
  if (n == 0) { // for the main screen, these return the work area
    X = Fl::x();
    Y = Fl::y();
    W = Fl::w();
    H = Fl::h();
  } else { // for other screens, work area is full screen,
    screen_xywh(X, Y, W, H, n);
  }
}


void Fl_Wayland_Screen_Driver::screen_xywh(int &X, int &Y, int &W, int &H, int n)
{
  if (num_screens < 0) init();

  if ((n < 0) || (n >= num_screens))
    n = 0;

  if (num_screens > 0) {
    Fl_Wayland_Screen_Driver::output *output;
    int i = 0;
    wl_list_for_each(output, &outputs, link) {
      if (i++ == n) { // n'th screen of the system
        float s = output->gui_scale * output->wld_scale;
        X = output->x / s;
        Y = output->y / s;
        W = output->width / s;
        H = output->height / s;
        break;
      }
    }
  }
}


void Fl_Wayland_Screen_Driver::screen_dpi(float &h, float &v, int n)
{
  if (num_screens < 0) init();
  h = v = 0.0f;

  if (n >= 0 && n < num_screens) {
    Fl_Wayland_Screen_Driver::output *output;
    int i = 0;
    wl_list_for_each(output, &outputs, link) {
      if (i++ == n) { // n'th screen of the system
        h = output->dpi;
        v = output->dpi;
        break;
      }
    }
  }
}


// Implements fl_beep(). See documentation in src/fl_ask.cxx.
void Fl_Wayland_Screen_Driver::beep(int type)
{
  fprintf(stderr, "\007");
}


void Fl_Wayland_Screen_Driver::flush()
{
  if (Fl_Wayland_Screen_Driver::wl_display) {
    wl_display_flush(Fl_Wayland_Screen_Driver::wl_display);
  }
}


extern void fl_fix_focus(); // in Fl.cxx


void Fl_Wayland_Screen_Driver::grab(Fl_Window* win)
{
  if (win) {
    if (!Fl::grab()) {
    }
    Fl::grab_ = win;    // FIXME: Fl::grab_ "should be private", but we need
                        // a way to *set* the variable from the driver!
  } else {
    if (Fl::grab()) {
      // We must keep the grab in the non-EWMH fullscreen case
      Fl::grab_ = 0;    // FIXME: Fl::grab_ "should be private", but we need
                        // a way to *set* the variable from the driver!
      fl_fix_focus();
    }
  }
}


static void set_selection_color(uchar r, uchar g, uchar b)
{
  Fl::set_color(FL_SELECTION_COLOR,r,g,b);
}

static void getsyscolor(const char *key1, const char* key2, const char *arg, const char *defarg, void (*func)(uchar,uchar,uchar))
{
  uchar r, g, b;
  if (!arg) arg = defarg;
  if (!Fl::screen_driver()->parse_color(arg, r, g, b))
    Fl::error("Unknown color: %s", arg);
  else
    func(r, g, b);
}


void Fl_Wayland_Screen_Driver::get_system_colors()
{
  open_display();
  const char* key1 = 0;
  if (Fl::first_window()) key1 = Fl::first_window()->xclass();
  if (!key1) key1 = "fltk";
  if (!bg2_set)
    getsyscolor("Text","background",    fl_bg2, "#ffffff", Fl::background2);
  if (!fg_set)
    getsyscolor(key1,  "foreground",    fl_fg,  "#000000", Fl::foreground);
  if (!bg_set)
    getsyscolor(key1,  "background",    fl_bg,  "#c0c0c0", Fl::background);
  getsyscolor("Text", "selectBackground", 0, "#000080", set_selection_color);
}


Fl_RGB_Image *Fl_Wayland_Screen_Driver::read_win_rectangle(int X, int Y, int w, int h, Fl_Window *win,
                                                           bool ignore, bool *p_ignore) {
  struct wld_window* xid = win ? fl_wl_xid(win) : NULL;
  struct fl_wld_buffer *buffer = win ? xid->buffer : (struct fl_wld_buffer *)Fl_Surface_Device::surface()->driver()->gc();
  float s = win ?
    Fl_Wayland_Window_Driver::driver(win)->wld_scale() * scale(win->screen_num()) :
                  Fl_Surface_Device::surface()->driver()->scale();
  int Xs, Ys, ws, hs;
  if (s == 1) {
    Xs = X; Ys = Y; ws = w; hs = h;
  } else {
    Xs = Fl_Scalable_Graphics_Driver::floor(X, s);
    Ys = Fl_Scalable_Graphics_Driver::floor(Y, s);
    ws = Fl_Scalable_Graphics_Driver::floor(X+w, s) - Xs;
    hs = Fl_Scalable_Graphics_Driver::floor(Y+h, s) - Ys;
  }
  if (ws == 0 || hs == 0) return NULL;
  uchar *data = new uchar[ws * hs * 3];
  uchar *p = data, *q;
  for (int j = 0; j < hs; j++) {
    q = buffer->draw_buffer + (j+Ys) * buffer->stride + 4 * Xs;
    for (int i = 0; i < ws; i++) {
      *p++ = *(q+2); // R
      *p++ = *(q+1); // G
      *p++ = *q;     // B
      q += 4;
    }
  }
  Fl_RGB_Image *rgb = new Fl_RGB_Image(data, ws, hs, 3);
  rgb->alloc_array = 1;
  return rgb;
}


void Fl_Wayland_Screen_Driver::offscreen_size(Fl_Offscreen off_, int &width, int &height)
{
  struct fl_wld_buffer *off = (struct fl_wld_buffer *)off_;
  width = off->width;
  height = off->data_size / off->stride;
}


float Fl_Wayland_Screen_Driver::scale(int n) {
  Fl_Wayland_Screen_Driver::output *output;
  int i = 0;
  wl_list_for_each(output, &outputs, link) {
    if (i++ == n) break;
  }
  return output->gui_scale;
}


void Fl_Wayland_Screen_Driver::scale(int n, float f) {
  Fl_Wayland_Screen_Driver::output *output;
  int i = 0;
  wl_list_for_each(output, &outputs, link) {
    if (i++ == n) {
      output->gui_scale = f;
      return;
    }
  }
}


void Fl_Wayland_Screen_Driver::set_cursor() {
  do_set_cursor(seat);
}

struct wl_cursor *Fl_Wayland_Screen_Driver::default_cursor() {
  return seat->default_cursor;
}

void Fl_Wayland_Screen_Driver::default_cursor(struct wl_cursor *cursor) {
  seat->default_cursor = cursor;
  do_set_cursor(seat);
}

struct wl_cursor *Fl_Wayland_Screen_Driver::cache_cursor(const char *cursor_name) {
  return wl_cursor_theme_get_cursor(seat->cursor_theme, cursor_name);
}

void Fl_Wayland_Screen_Driver::reset_cursor() {
  xc_arrow = xc_ns = xc_wait = xc_insert = xc_hand = xc_help = xc_cross = xc_move =
  xc_north = xc_south = xc_west = xc_east = xc_we = xc_nesw = xc_nwse = xc_sw = xc_se =
  xc_ne = xc_nw = NULL;
}

uint32_t Fl_Wayland_Screen_Driver::get_serial() {
  return seat->serial;
}

struct wl_seat*Fl_Wayland_Screen_Driver::get_wl_seat() {
  return seat->wl_seat;
}

char *Fl_Wayland_Screen_Driver::get_seat_name() {
  return seat->name;
}

struct xkb_keymap *Fl_Wayland_Screen_Driver::get_xkb_keymap() {
  return seat->xkb_keymap;
}


int Fl_Wayland_Screen_Driver::get_mouse(int &xx, int &yy) {
  open_display();
  xx = Fl::e_x_root; yy = Fl::e_y_root;
  if (!seat->pointer_focus) return 0;
  Fl_Window *win = Fl_Wayland_Window_Driver::surface_to_window(seat->pointer_focus);
  if (!win) return 0;
  int snum = Fl_Window_Driver::driver(win)->screen_num();
//printf("get_mouse(%dx%d)->%d\n", xx, yy, snum);
  return snum;
}


void Fl_Wayland_Screen_Driver::set_spot(int font, int height, int x, int y, int w, int h, Fl_Window *win) {
  Fl_Wayland_Screen_Driver::insertion_point_location(x, y, height);
}


void Fl_Wayland_Screen_Driver::reset_spot() {
  Fl::compose_state = 0;
  Fl_Wayland_Screen_Driver::next_marked_length = 0;
  Fl_Wayland_Screen_Driver::insertion_point_location_is_valid = false;
}


void Fl_Wayland_Screen_Driver::display(const char *d)
{
  if (d && !wl_registry) { // if display was opened, it's too late
    if (wl_display) {
      // only the wl_display_connect() call was done, redo it because the target
      // Wayland compositor may be different
      wl_display_disconnect(wl_display);
    }
    wl_display = wl_display_connect(d);
    if (!wl_display) {
      fprintf(stderr, "Error: '%s' is not an active Wayland socket\n", d);
      exit(1);
    }
  }
}


void *Fl_Wayland_Screen_Driver::control_maximize_button(void *data) {
  // The code below aims at removing the calling window's fullscreen button
  // while dialog runs. Unfortunately, it doesn't work with some X11 window managers
  // (e.g., KDE, xfce) because the button goes away but doesn't come back,
  // so we move this code to a virtual member function.
  // Noticeably, this code works OK under Wayland.
  struct win_dims {
    Fl_Widget_Tracker *tracker;
    int minw, minh, maxw, maxh;
    struct win_dims *next;
  };

  if (!data) { // this call turns each decorated window's maximize button off
    struct win_dims *first_dim = NULL;
    // consider all bordered, top-level FLTK windows
    Fl_Window *win = Fl::first_window();
    while (win) {
      if (!win->parent() && win->border() &&
          !( ((struct wld_window*)Fl_X::flx(win)->xid)->state & LIBDECOR_WINDOW_STATE_MAXIMIZED) ) {
        win_dims *dim = new win_dims;
        dim->tracker = new Fl_Widget_Tracker(win);
        Fl_Window_Driver *dr = Fl_Window_Driver::driver(win);
        dim->minw = dr->minw();
        dim->minh = dr->minh();
        dim->maxw = dr->maxw();
        dim->maxh = dr->maxh();
        //make win un-resizable
        win->size_range(win->w(), win->h(), win->w(), win->h());
        dim->next = first_dim;
        first_dim = dim;
      }
      win = Fl::next_window(win);
    }
    return first_dim;
  } else { // this call returns each decorated window's maximize button to its previous state
    win_dims *first_dim = (win_dims *)data;
    while (first_dim) {
      win_dims *dim = first_dim;
      //give back win its resizing parameters
      if (dim->tracker->exists()) {
        Fl_Window *win = (Fl_Window*)dim->tracker->widget();
        win->size_range(dim->minw, dim->minh, dim->maxw, dim->maxh);
      }
      first_dim = dim->next;
      delete dim->tracker;
      delete dim;
    }
    return NULL;
  }
}


int Fl_Wayland_Screen_Driver::event_key(int k) {
  if (k >= 'A' && k <= 'Z') k += 32;
  return (search_int_vector(key_vector, k) >= 0);
}


int Fl_Wayland_Screen_Driver::get_key(int k) {
  return event_key(k);
}


struct wl_display *fl_wl_display() {
  return Fl_Wayland_Screen_Driver::wl_display;
}
