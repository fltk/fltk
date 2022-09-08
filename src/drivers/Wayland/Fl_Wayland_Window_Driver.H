//
// Definition of Wayland window driver for the Fast Light Tool Kit (FLTK).
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
 \file Fl_Wayland_Window_Driver.H
 \brief Definition of Wayland window driver.
 */

#ifndef FL_WAYLAND_WINDOW_DRIVER_H
#define FL_WAYLAND_WINDOW_DRIVER_H

#include "../../Fl_Window_Driver.H"
#include <FL/Fl_Plugin.H>
#include "Fl_Wayland_Screen_Driver.H"


/*
 Move everything here that manages the native window interface.

 There is one window driver for each Fl_Window. Window drivers manage window
 actions such as resizing, events, decoration, fullscreen modes, etc. . All
 drawing and rendering is managed by the Surface device and the associated
 graphics driver.

 - window specific event handling
 - window types and styles, depth, etc.
 - decorations
 */

typedef struct _cairo_pattern cairo_pattern_t;
typedef struct _cairo_rectangle_int cairo_rectangle_int_t;
class Fl_Wayland_Plugin;


class Fl_Wayland_Window_Driver : public Fl_Window_Driver
{
  friend class Fl_X;
  friend class Fl_Wayland_Gl_Window_Driver;
  friend class Fl_Wayland_Screen_Driver;
private:
  struct shape_data_type {
    int lw_; ///<  width of shape image
    int lh_; ///<  height of shape image
    Fl_Image* shape_; ///<  shape image
    cairo_pattern_t *mask_pattern_;
  } *shape_data_;
  cairo_rectangle_int_t *subRect_;      // makes sure subwindow remains inside its parent window
  static bool in_flush; // useful for progressive window drawing
  static Fl_Wayland_Plugin *gl_plugin();
  struct wl_cursor *cursor_;
  void delete_cursor_();
  struct gl_start_support *gl_start_support_; // for support of gl_start/gl_finish
public:
  struct wl_cursor *cursor() { return cursor_; };
  bool in_handle_configure; // distinguish OS and user window resize
  struct window_output {
    Fl_Wayland_Screen_Driver::output* output;
    struct wl_list link;
  };
  // --- support for screen-specific scaling factors
  struct type_for_resize_window_between_screens {
    int screen;
    bool busy;
  };
  static type_for_resize_window_between_screens data_for_resize_window_between_screens_;
  void decorated_win_size(int &w, int &h);
  void shape_bitmap_(Fl_Image* b);
  void shape_alpha_(Fl_Image* img, int offset);
  void update_scale();
  cairo_rectangle_int_t *subRect() { return subRect_; } // getter
  void subRect(cairo_rectangle_int_t *r); // setter
  void checkSubwindowFrame();
  enum kind {DECORATED, SUBWINDOW, POPUP, UNFRAMED};
  struct xdg_toplevel *xdg_toplevel();
  Fl_Wayland_Window_Driver(Fl_Window*);
  virtual ~Fl_Wayland_Window_Driver();
  static struct wld_window *wld_window;
  static void redraw(struct wld_window *window);

  static inline Fl_Wayland_Window_Driver* driver(const Fl_Window *w) {return (Fl_Wayland_Window_Driver*)Fl_Window_Driver::driver(w);}
  static void resize_after_screen_change(void *data);

  // --- window data
  virtual int decorated_w();
  virtual int decorated_h();
  virtual const Fl_Image* shape();

  // --- window management
  virtual Fl_X *makeWindow();
  virtual void take_focus();
  virtual void flush();
  virtual void flush_overlay();
  virtual void draw_end();
  virtual void make_current();
  virtual void show();
  virtual void resize(int X,int Y,int W,int H);
  virtual void label(const char *name, const char *mininame);
  virtual void destroy_double_buffer();
  virtual void hide();
  virtual void map();
  virtual void unmap();
  virtual void fullscreen_on();
  virtual void fullscreen_off(int X, int Y, int W, int H);
  virtual void use_border();
  virtual void size_range();
  virtual void iconize();
  virtual void decoration_sizes(int *top, int *left,  int *right, int *bottom);
  // --- window cursor stuff
  virtual int set_cursor(Fl_Cursor);
  virtual int set_cursor(const Fl_RGB_Image*, int, int);

  virtual void shape(const Fl_Image* img);
  virtual void capture_titlebar_and_borders(Fl_RGB_Image*& top, Fl_RGB_Image*& left, Fl_RGB_Image*& bottom, Fl_RGB_Image*& right);
  virtual int scroll(int src_x, int src_y, int src_w, int src_h, int dest_x, int dest_y, void (*draw_area)(void*, int,int,int,int), void* data);
  virtual void wait_for_expose();
  virtual void reposition_menu_window(int x, int y);
  virtual void menu_window_area(int &X, int &Y, int &W, int &H, int nscreen = -1);
};


struct wld_window {
  struct wl_list outputs;
  struct wl_surface *wl_surface;
  struct fl_wld_buffer *buffer;
  struct xdg_surface *xdg_surface;
  union {
    struct libdecor_frame *frame;
    struct wl_subsurface *subsurface;
    struct xdg_popup *xdg_popup;
    struct xdg_toplevel *xdg_toplevel;
  };
  Fl_Window *fl_win;
  enum Fl_Wayland_Window_Driver::kind kind;
  int configured_width;
  int configured_height;
  int floating_width;
  int floating_height;
  int scale;
  int state;
};


class Fl_Wayland_Plugin : public Fl_Plugin {
public:
  Fl_Wayland_Plugin(const char *pluginName)  : Fl_Plugin(klass(), pluginName) { }
  virtual const char *klass() { return "wayland.fltk.org"; }
  virtual const char *name() = 0;
  virtual void do_swap(Fl_Window*) = 0;
  virtual void invalidate(Fl_Window*) = 0;
  virtual void terminate() = 0;
  virtual void destroy(struct gl_start_support *) = 0;
};

#endif // FL_WAYLAND_WINDOW_DRIVER_H
