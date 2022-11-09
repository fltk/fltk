//
// Definition of X11 Screen interface
// for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2022 by Bill Spitzak and others.
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

/**
 \file Fl_Wayland_Screen_Driver.H
 \brief Definition of Wayland Screen interface
 */

#ifndef FL_WAYLAND_SCREEN_DRIVER_H
#define FL_WAYLAND_SCREEN_DRIVER_H

#include "../Unix/Fl_Unix_Screen_Driver.H"
#include <wayland-client.h>

class Fl_Window;

struct seat {
  struct wl_seat *wl_seat;
  struct wl_pointer *wl_pointer;
  struct wl_keyboard *wl_keyboard;
  uint32_t keyboard_enter_serial;
  struct wl_surface *keyboard_surface;
  struct wl_list link;
  struct wl_list pointer_outputs;
  struct wl_cursor_theme *cursor_theme;
  struct wl_cursor *default_cursor;
  struct wl_surface *cursor_surface;
  struct wl_surface *pointer_focus;
  int pointer_scale;
  uint32_t serial;
  uint32_t pointer_enter_serial;
  struct wl_data_device_manager *data_device_manager;
  struct wl_data_device *data_device;
  struct wl_data_source *data_source;
  struct xkb_state *xkb_state;
  struct xkb_context *xkb_context;
  struct xkb_keymap *xkb_keymap;
  struct xkb_compose_state *xkb_compose_state;
  char *name;
  struct zwp_text_input_v3 *text_input;
};

class Fl_Wayland_Screen_Driver : public Fl_Unix_Screen_Driver
{
  friend class Fl_Screen_Driver;
  static int insertion_point_x;
  static int insertion_point_y;
  static int insertion_point_width;
  static int insertion_point_height;
  static bool insertion_point_location_is_valid;
public:
  static FL_EXPORT struct wl_display *wl_display;
  // use it to make sure the Wayland leg was selected and fl_open_display() has run
  static struct wl_registry *wl_registry;
  static void insertion_point_location(int x, int y, int height);
  static bool insertion_point_location(int *px, int *py, int *pwidth, int *pheight);
  int get_mouse_unscaled(int &xx, int &yy);
  void screen_count_set(int count) {num_screens = count;}

  void reset_cursor();
  struct wl_cursor *xc_arrow;
  struct wl_cursor *xc_ns;
  struct wl_cursor *xc_wait;
  struct wl_cursor *xc_insert;
  struct wl_cursor *xc_hand;
  struct wl_cursor *xc_help;
  struct wl_cursor *xc_cross;
  struct wl_cursor *xc_move;
  struct wl_cursor *xc_north;
  struct wl_cursor *xc_south;
  struct wl_cursor *xc_west;
  struct wl_cursor *xc_east;
  struct wl_cursor *xc_we;
  struct wl_cursor *xc_nesw;
  struct wl_cursor *xc_nwse;
  struct wl_cursor *xc_sw;
  struct wl_cursor *xc_se;
  struct wl_cursor *xc_ne;
  struct wl_cursor *xc_nw;
  static const struct wl_data_device_listener *p_data_device_listener;

public:
  struct wl_compositor *wl_compositor;
  struct wl_subcompositor *wl_subcompositor;
  struct wl_shm *wl_shm;
  struct wl_list seats;
  struct seat *seat;
  struct wl_list outputs; // linked list of all screens in system
  struct output { // one record for each screen
    uint32_t id;
    short x_org;
    short y_org;
    short width; // in pixels
    short height; // in pixels
    float dpi;
    struct wl_output *wl_output;
    int wld_scale; // Wayland scale factor
    float gui_scale; // FLTK scale factor
    struct wl_list link;
  };
  struct libdecor *libdecor_context;
  struct xdg_wm_base *xdg_wm_base;
  struct zwp_text_input_manager_v3 *text_input_base;

  Fl_Wayland_Screen_Driver();
  virtual APP_SCALING_CAPABILITY rescalable() { return PER_SCREEN_APP_SCALING; }
  virtual float scale(int n);
  virtual void scale(int n, float f);
  int screen_num_unscaled(int x, int y);

  void copy_image(const unsigned char* data, int W, int H);
  // --- screen configuration
  void init_workarea();
  virtual void init();
  virtual int x();
  virtual int y();
  virtual int w();
  virtual int h();
  virtual void screen_xywh(int &X, int &Y, int &W, int &H, int n);
  virtual void screen_dpi(float &h, float &v, int n=0);
  virtual void screen_work_area(int &X, int &Y, int &W, int &H, int n);
  // --- audible output
  virtual void beep(int type);
  // --- global events
  virtual void flush();
  virtual void grab(Fl_Window* win);
  // --- global colors
  virtual void get_system_colors();
  virtual const char *get_system_scheme();
  virtual int dnd(int unused);
  virtual int compose(int &del);
  virtual void compose_reset();
  virtual Fl_RGB_Image *read_win_rectangle(int X, int Y, int w, int h, Fl_Window *win, bool may_capture_subwins, bool *did_capture_subwins);
  virtual int get_mouse(int &x, int &y);
  virtual void open_display_platform();
  virtual void close_display();
  virtual void display(const char *d);
  // --- compute dimensions of an Fl_Offscreen
  virtual void offscreen_size(Fl_Offscreen o, int &width, int &height);
  virtual int has_marked_text() const;
  static int next_marked_length; // next length of marked text after current marked text will have been replaced
  // --- clipboard operations
  // this one is in fl_wayland_clipboard_dnd.cxx
  virtual void copy(const char *stuff, int len, int clipboard, const char *type);
  // this one is in fl_wayland_clipboard_dnd.cxx
  virtual void paste(Fl_Widget &receiver, int clipboard, const char *type);
  // this one is in fl_wayland_clipboard_dnd.cxx
  virtual int clipboard_contains(const char *type);
  // --- Wayland-special
  void set_cursor();
  struct wl_cursor *default_cursor();
  void default_cursor(struct wl_cursor *cursor);
  struct wl_cursor *cache_cursor(const char *cursor_name);
  static Fl_Window *surface_to_window(struct wl_surface *);
  uint32_t get_serial();
  struct wl_seat *get_wl_seat();
  char  *get_seat_name();
  struct xkb_keymap *get_xkb_keymap();
  static bool own_output(struct wl_output *output);
  typedef enum {unspecified, MUTTER, WESTON, KDE} compositor_name;
  static compositor_name compositor; // identifies the used Wayland compositor
  void set_spot(int font, int height, int x, int y, int w, int h, Fl_Window *win);
  void reset_spot();
  virtual void *control_maximize_button(void *data);
  int event_key(int k);
  int get_key(int k);
};


#endif // FL_WAYLAND_SCREEN_DRIVER_H
