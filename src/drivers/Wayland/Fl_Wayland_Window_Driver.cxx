//
// Implementation of the Wayland window driver.
//
// Copyright 1998-2024 by Bill Spitzak and others.
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

#include <FL/platform.H>
#include "Fl_Wayland_Window_Driver.H"
#include "Fl_Wayland_Screen_Driver.H"
#include "Fl_Wayland_Graphics_Driver.H"
#include <FL/filename.H>
#include <wayland-cursor.h>
#include "../../../libdecor/build/fl_libdecor.h"
#include "xdg-shell-client-protocol.h"
#include "gtk-shell-client-protocol.h"
#include <pango/pangocairo.h>
#include <FL/Fl_Overlay_Window.H>
#include <FL/Fl_Tooltip.H>
#include <FL/fl_draw.H>
#include <FL/fl_ask.H>
#include <FL/Fl.H>
#include <FL/Fl_Image_Surface.H>
#include <string.h>
#include <math.h> // for ceil()
#include <sys/types.h> // for pid_t
#include <unistd.h> // for getpid()

struct cursor_image { // as in wayland-cursor.c of the Wayland project source code
  struct wl_cursor_image image;
  struct wl_cursor_theme *theme;
  struct wl_buffer *buffer;
  int offset; /* data offset of this image in the shm pool */
};

extern "C" {
# include "../../../libdecor/src/libdecor-plugin.h"
  uchar *fl_libdecor_titlebar_buffer(struct libdecor_frame *frame, int *w, int *h, int *stride);
}

#define fl_max(a,b) ((a) > (b) ? (a) : (b))
#define fl_min(a,b) ((a) < (b) ? (a) : (b))

#if !defined(FLTK_USE_X11)
Window fl_window = 0;
#endif


struct wld_window *Fl_Wayland_Window_Driver::wld_window = NULL;
bool Fl_Wayland_Window_Driver::new_popup = false; // to support tall menu buttons
// A menutitle to be mapped later as the child of a menuwindow
Fl_Window *Fl_Wayland_Window_Driver::previous_floatingtitle = NULL;


Fl_Wayland_Window_Driver::Fl_Wayland_Window_Driver(Fl_Window *win) : Fl_Window_Driver(win)
{
  shape_data_ = NULL;
  standard_cursor_ = FL_CURSOR_DEFAULT;
  in_handle_configure = false;
  screen_num_ = -1;
  gl_start_support_ = NULL;
  subRect_ = NULL;
  is_popup_window_ = false;
  can_expand_outside_parent_ = false;
}


void Fl_Wayland_Window_Driver::delete_cursor(
      struct Fl_Wayland_Window_Driver::custom_cursor *custom, bool delete_rgb) {
  struct wl_cursor *wl_cursor = custom->wl_cursor;
  struct cursor_image *new_image = (struct cursor_image*)wl_cursor->images[0];
  struct Fl_Wayland_Graphics_Driver::wld_buffer *offscreen =
  (struct Fl_Wayland_Graphics_Driver::wld_buffer *)
  wl_buffer_get_user_data(new_image->buffer);
  struct wld_window fake_xid;
  memset(&fake_xid, 0, sizeof(fake_xid));
  fake_xid.buffer = offscreen;
  Fl_Wayland_Graphics_Driver::buffer_release(&fake_xid);
  free(new_image);
  free(wl_cursor->images);
  free(wl_cursor->name);
  free(wl_cursor);
  Fl_Wayland_Screen_Driver *scr_driver = (Fl_Wayland_Screen_Driver*)Fl::screen_driver();
  if (scr_driver->default_cursor() == wl_cursor) {
    scr_driver->default_cursor(scr_driver->xc_cursor[Fl_Wayland_Screen_Driver::arrow]);
  }
  if (delete_rgb) delete custom->rgb;
  delete custom;
}


Fl_Wayland_Window_Driver::~Fl_Wayland_Window_Driver()
{
  if (shape_data_) {
    cairo_surface_t *surface;
    cairo_pattern_get_surface(shape_data_->mask_pattern_, &surface);
    uchar *data = cairo_image_surface_get_data(surface);
    cairo_pattern_destroy(shape_data_->mask_pattern_);
    delete[] data;
    delete shape_data_;
  }
  if (subRect_) delete subRect_;
  if (gl_start_support_) { // occurs only if gl_start/gl_finish was used
    gl_plugin()->destroy(gl_start_support_);
  }
}


void Fl_Wayland_Window_Driver::decorated_win_size(int &w, int &h)
{
  Fl_Window *win = pWindow;
  w = win->w();
  h = win->h();
  if (!win->shown() || win->parent() || !win->border() || !win->visible()) return;
  int X, titlebar_height;
  libdecor_frame_translate_coordinate(fl_wl_xid(win)->frame, 0, 0, &X, &titlebar_height);
//printf("titlebar_height=%d\n",titlebar_height);
  h = win->h() + ceil(titlebar_height / Fl::screen_scale(win->screen_num()));
}


int Fl_Wayland_Window_Driver::decorated_h()
{
  int w, h;
  decorated_win_size(w, h);
  return h;
}


int Fl_Wayland_Window_Driver::decorated_w()
{
  int w, h;
  decorated_win_size(w, h);
  return w;
}


struct xdg_toplevel *Fl_Wayland_Window_Driver::xdg_toplevel() {
  struct wld_window * w = fl_wl_xid(pWindow);
  struct xdg_toplevel *top = NULL;
  if (w->kind == DECORATED) top = libdecor_frame_get_xdg_toplevel(w->frame);
  else if (w->kind == UNFRAMED) top = w->xdg_toplevel;
  return top;
}


void Fl_Wayland_Window_Driver::take_focus()
{
  struct wld_window *w = fl_wl_xid(pWindow);
  if (w) {
    Fl_Window *old_first = Fl::first_window();
    struct wld_window *first_xid = (old_first ? fl_wl_xid(old_first->top_window()) : NULL);
    if (first_xid && first_xid != w && xdg_toplevel()) {
      // this will move the target window to the front
      Fl_Wayland_Window_Driver *top_dr =
        Fl_Wayland_Window_Driver::driver(old_first->top_window());
      xdg_toplevel_set_parent(xdg_toplevel(), top_dr->xdg_toplevel());
      // this will remove the parent-child relationship
      xdg_toplevel_set_parent(xdg_toplevel(), NULL);
      wl_display_roundtrip(Fl_Wayland_Screen_Driver::wl_display);
    }
    // this sets the first window
    fl_wl_find(w);
  }
}


void Fl_Wayland_Window_Driver::flush_overlay()
{
  if (!shown()) return;
  Fl_Overlay_Window *oWindow = pWindow->as_overlay_window();
  int erase_overlay = (pWindow->damage()&FL_DAMAGE_OVERLAY) | (overlay() == oWindow);
  pWindow->clear_damage((uchar)(pWindow->damage()&~FL_DAMAGE_OVERLAY));
  pWindow->make_current();
  if (!other_xid) {
    other_xid = new Fl_Image_Surface(oWindow->w(), oWindow->h(), 1);
    oWindow->clear_damage(FL_DAMAGE_ALL);
  }
  if (oWindow->damage() & ~FL_DAMAGE_EXPOSE) {
    Fl_X *myi = Fl_X::flx(pWindow);
    fl_clip_region(myi->region); myi->region = 0;
    Fl_Surface_Device::push_current(other_xid);
    draw();
    Fl_Surface_Device::pop_current();
  }
  if (erase_overlay) fl_clip_region(0);
  if (other_xid) {
    struct Fl_Wayland_Graphics_Driver::draw_buffer *buffer =
      Fl_Wayland_Graphics_Driver::offscreen_buffer(other_xid->offscreen());
    struct wld_window *xid = fl_wl_xid(pWindow);
    struct Fl_Wayland_Graphics_Driver::wld_buffer *wbuffer = xid->buffer;
    if (wbuffer->draw_buffer.data_size != buffer->data_size) {
      fl_copy_offscreen(0, 0, oWindow->w(), oWindow->h(), other_xid->offscreen(), 0, 0);
    } else {
      memcpy(wbuffer->draw_buffer.buffer, buffer->buffer, wbuffer->draw_buffer.data_size);
    }
  }
  if (overlay() == oWindow) oWindow->draw_overlay();
}


const Fl_Image* Fl_Wayland_Window_Driver::shape() {
  return shape_data_ ? shape_data_->shape_ : NULL;
}


void Fl_Wayland_Window_Driver::shape_bitmap_(Fl_Image* b) {
  shape_data_->mask_pattern_ = Fl_Cairo_Graphics_Driver::bitmap_to_pattern(
                                (Fl_Bitmap*)b, true, NULL);
  shape_data_->shape_ = b;
  shape_data_->lw_ = b->data_w();
  shape_data_->lh_ = b->data_h();
}


void Fl_Wayland_Window_Driver::shape_alpha_(Fl_Image* img, int offset) {
  int i, j, d = img->d(), w = img->data_w(), h = img->data_h();
  int bytesperrow = cairo_format_stride_for_width(CAIRO_FORMAT_A1, w);
  unsigned u;
  uchar byte, onebit;
  // build a CAIRO_FORMAT_A1 surface covering the non-fully transparent/black part of the image
  uchar* bits = new uchar[h*bytesperrow]; // to store the surface data
  const uchar* alpha = (const uchar*)*img->data() + offset; // points to alpha value of pixels
  for (i = 0; i < h; i++) {
    uchar *p = (uchar*)bits + i * bytesperrow;
    byte = 0;
    onebit = 1;
    for (j = 0; j < w; j++) {
      if (d == 3) {
        u = *alpha;
        u += *(alpha+1);
        u += *(alpha+2);
      }
      else u = *alpha;
      if (u > 0) { // if the pixel is not fully transparent/black
        byte |= onebit; // turn on the corresponding bit of the bitmap
      }
      onebit = onebit << 1; // move the single set bit one position to the left
      if (onebit == 0 || j == w-1) {
        onebit = 1;
        *p++ = ~byte; // store in bitmap one pack of bits, complemented
        byte = 0;
      }
      alpha += d; // point to alpha value of next img pixel
    }
  }
  cairo_surface_t *mask_surf = cairo_image_surface_create_for_data(bits, CAIRO_FORMAT_A1,
                                                                   w, h, bytesperrow);
  shape_data_->mask_pattern_ = cairo_pattern_create_for_surface(mask_surf);
  cairo_surface_destroy(mask_surf);
  shape_data_->shape_ = img;
  shape_data_->lw_ = w;
  shape_data_->lh_ = h;
}


void Fl_Wayland_Window_Driver::shape(const Fl_Image* img) {
  if (shape_data_) {
    if (shape_data_->mask_pattern_) {
      cairo_surface_t *surface;
      cairo_pattern_get_surface(shape_data_->mask_pattern_, &surface);
      uchar *data = cairo_image_surface_get_data(surface);
      cairo_pattern_destroy(shape_data_->mask_pattern_);
      delete[] data;
    }
  }
  else {
    shape_data_ = new shape_data_type;
  }
  memset(shape_data_, 0, sizeof(shape_data_type));
  pWindow->border(false);
  int d = img->d();
  if (d && img->count() >= 2) {
    shape_pixmap_((Fl_Image*)img);
    shape_data_->shape_ = (Fl_Image*)img;
  }
  else if (d == 0) shape_bitmap_((Fl_Image*)img);
  else if (d == 2 || d == 4) shape_alpha_((Fl_Image*)img, d - 1);
  else if ((d == 1 || d == 3) && img->count() == 1) shape_alpha_((Fl_Image*)img, 0);
}


void Fl_Wayland_Window_Driver::draw_end()
{
  if (shape_data_ && shape_data_->mask_pattern_) {
    Fl_Wayland_Graphics_Driver *gr_dr = (Fl_Wayland_Graphics_Driver*)fl_graphics_driver;
    cairo_t *cr = gr_dr->cr();
    cairo_matrix_t matrix;
    cairo_matrix_init_scale(&matrix, double(shape_data_->lw_) / (pWindow->w() + 1),
                            double(shape_data_->lh_) / (pWindow->h() + 1) );
    cairo_matrix_translate(&matrix, 1, 1);
    cairo_pattern_set_matrix(shape_data_->mask_pattern_, &matrix);
    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_mask(cr, shape_data_->mask_pattern_);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
  }
}


/* Returns images of the captures of the window title-bar, and the left, bottom and right window borders
 (or NULL if a particular border is absent).
 Returned images can be deleted after use. Their depth and size may be platform-dependent.
 The top and bottom images extend from left of the left border to right of the right border.
 */
void Fl_Wayland_Window_Driver::capture_titlebar_and_borders(Fl_RGB_Image*& top,
                  Fl_RGB_Image*& left, Fl_RGB_Image*& bottom, Fl_RGB_Image*& right)
{
  top = left = bottom = right = NULL;
  if (pWindow->decorated_h() == h()) return;
  int htop = pWindow->decorated_h() - pWindow->h();
  struct wld_window *wwin = fl_wl_xid(pWindow);
  int width, height, stride;
  uchar *cairo_data = fl_libdecor_titlebar_buffer(wwin->frame, &width, &height, &stride);
  if (!cairo_data) return;
  uchar *data = new uchar[width * height * 4];
  uchar *p = data;
  for (int j = 0; j < height; j++) {
    uchar *q = cairo_data + j * stride;
    for (int i = 0; i < width; i++) {
      *p++ = *(q+2); // R
      *p++ = *(q+1); // G
      *p++ = *q;     // B
      *p++ = *(q+3); // A
      q += 4;
    }
  }
  top = new Fl_RGB_Image(data, width, height, 4);
  top->alloc_array = 1;
  top->scale(pWindow->w(), htop);
}


// make drawing go into this window (called by subclass flush() impl.)
void Fl_Wayland_Window_Driver::make_current() {
  if (!shown()) {
    static const char err_message[] = "Fl_Window::make_current(), but window is not shown().";
    fl_alert(err_message);
    Fl::fatal(err_message);
  }

  struct wld_window *window = fl_wl_xid(pWindow);
  if (window->buffer) {
    ((Fl_Cairo_Graphics_Driver*)fl_graphics_driver)->needs_commit_tag(
                                            &window->buffer->draw_buffer_needs_commit);
  }

  // to support progressive drawing
  if ( (!Fl_Wayland_Window_Driver::in_flush_) && window->buffer && (!window->frame_cb)
      && window->buffer->draw_buffer_needs_commit && (!wait_for_expose_value) ) {
    Fl_Wayland_Graphics_Driver::buffer_commit(window);
  }

  Fl_Wayland_Window_Driver::wld_window = window;
  fl_window = (Window)window;
  float f = Fl::screen_scale(pWindow->screen_num());
  int wld_s = wld_scale();
  if (!window->buffer) {
    window->buffer = Fl_Wayland_Graphics_Driver::create_wld_buffer(
           int(pWindow->w() * f) * wld_s, int(pWindow->h() * f) * wld_s, false);
    ((Fl_Cairo_Graphics_Driver*)fl_graphics_driver)->needs_commit_tag(
                                            &window->buffer->draw_buffer_needs_commit);
  }
  ((Fl_Wayland_Graphics_Driver*)fl_graphics_driver)->set_cairo(
                      window->buffer->draw_buffer.cairo_, f * wld_s);
  ((Fl_Cairo_Graphics_Driver*)fl_graphics_driver)->wld_scale = wld_s;
  int *poffset = Fl_Window_Driver::menu_offset_y(pWindow);
  if (poffset) { // for tall menu windows under KWIN to offset drawing inside window
    cairo_translate(window->buffer->draw_buffer.cairo_, 0, *poffset);
  }
  cairo_rectangle_int_t *extents = subRect();
  if (extents) { // make damage-to-buffer not to leak outside parent
    Fl_Region clip_region = fl_graphics_driver->XRectangleRegion(extents->x, extents->y,
                                                      extents->width, extents->height);
//printf("make_current: %dx%d %dx%d\n",extents->x, extents->y, extents->width, extents->height);
    Fl_X::flx(pWindow)->region = clip_region;
  }
  else fl_graphics_driver->clip_region(0);

#ifdef FLTK_HAVE_CAIROEXT
  // update the cairo_t context
  if (Fl::cairo_autolink_context()) Fl::cairo_make_current(pWindow);
#endif
}


void Fl_Wayland_Window_Driver::flush() {
  if (!pWindow->damage()) return;
  if (pWindow->as_gl_window()) {
    int W = pWindow->w();
    int H = pWindow->h();
    float scale = fl_graphics_driver->scale();
    Fl_Wayland_Window_Driver::in_flush_ = true;
    Fl_Window_Driver::flush();
    Fl_Wayland_Window_Driver::in_flush_ = false;
    gl_plugin()->do_swap(pWindow); // useful only for GL win with overlay
    if (scale != fl_graphics_driver->scale() || W != pWindow->w() || H != pWindow->h()) {
      gl_plugin()->invalidate(pWindow);
    }
    return;
  }
  struct wld_window *window = fl_wl_xid(pWindow);
  if (!window || !window->configured_width) return;

  Fl_X *ip = Fl_X::flx(pWindow);
  cairo_region_t* r = (cairo_region_t*)ip->region;
  if (!window->buffer || pWindow->as_overlay_window()) r = NULL;

  Fl_Wayland_Window_Driver::in_flush_ = true;
  Fl_Window_Driver::flush();
  Fl_Wayland_Window_Driver::in_flush_ = false;
  if (!window->frame_cb) Fl_Wayland_Graphics_Driver::buffer_commit(window, r);
}


void Fl_Wayland_Window_Driver::show() {
  if (!shown()) {
    fl_open_display();
    makeWindow();
  } else {
    // Wayland itself gives no way to programmatically unminimize a minimized window
    Fl::handle(FL_SHOW, pWindow);
  }
}


static void popup_done(void *data, struct xdg_popup *xdg_popup);


static void destroy_surface_caution_pointer_focus(struct wl_surface *surface,
                                      struct Fl_Wayland_Screen_Driver::seat *seat) {
  if (seat->pointer_focus == surface) seat->pointer_focus = NULL;
  if (seat->keyboard_surface == surface) seat->keyboard_surface = NULL;
  wl_surface_destroy(surface);
}


void Fl_Wayland_Window_Driver::hide() {
  if (pWindow == Fl_Screen_Driver::transient_scale_parent) {
    // Delete also the running transient scale window
    // because the transient is a popup and MUST be deleted
    // before its parent.
    Fl::remove_timeout(Fl_Screen_Driver::del_transient_window);
    Fl_Screen_Driver::del_transient_window(NULL);
  }
  Fl_X* ip = Fl_X::flx(pWindow);
  if (hide_common()) return;
  if (ip->region) {
    Fl_Graphics_Driver::default_driver().XDestroyRegion(ip->region);
    ip->region = 0;
  }
  screen_num_ = -1;
  struct wld_window *wld_win = (struct wld_window*)ip->xid;
  if (wld_win) { // this test makes sure ip->xid has not been destroyed already
    Fl_Wayland_Graphics_Driver::buffer_release(wld_win);
    if (wld_win->kind == SUBWINDOW && wld_win->subsurface) {
      wl_subsurface_destroy(wld_win->subsurface);
      wld_win->subsurface = NULL;
    }
    if (wld_win->kind == DECORATED) {
      libdecor_frame_unref(wld_win->frame);
      wld_win->frame = NULL;
      wld_win->xdg_surface = NULL;
    } else {
      if (wld_win->kind == POPUP && wld_win->xdg_popup) {
        popup_done(xdg_popup_get_user_data(wld_win->xdg_popup), wld_win->xdg_popup);
        wld_win->xdg_popup = NULL;
      }
      if (wld_win->kind == UNFRAMED && wld_win->xdg_toplevel) {
        xdg_toplevel_destroy(wld_win->xdg_toplevel);
        wld_win->xdg_toplevel = NULL;
      }
      if (wld_win->xdg_surface) {
        xdg_surface_destroy(wld_win->xdg_surface);
        wld_win->xdg_surface = NULL;
      }
    }
    if (wld_win->custom_cursor) delete_cursor(wld_win->custom_cursor);
    if (wld_win->wl_surface) {
      Fl_Wayland_Screen_Driver *scr_driver = (Fl_Wayland_Screen_Driver*)Fl::screen_driver();
      destroy_surface_caution_pointer_focus(wld_win->wl_surface, scr_driver->seat);
      wld_win->wl_surface = NULL;
    }
    while (!wl_list_empty(&wld_win->outputs)) { // remove from screens where it belongs
      struct surface_output *s_output;
      s_output = wl_container_of(wld_win->outputs.next, s_output, link);
      wl_list_remove(&s_output->link);
      free(s_output);
    }
    if (Fl_Wayland_Window_Driver::wld_window == wld_win) {
      Fl_Wayland_Window_Driver::wld_window = NULL;
    }
    if (wld_win->frame_cb) wl_callback_destroy(wld_win->frame_cb); // useful for GL subwins
    free(wld_win);
  }
  delete ip;
}


void Fl_Wayland_Window_Driver::map() {
  Fl_X* ip = Fl_X::flx(pWindow);
  struct wld_window *wl_win = (struct wld_window*)ip->xid;
  if (wl_win->kind == SUBWINDOW && !wl_win->subsurface) {
    struct wld_window *parent = fl_wl_xid(pWindow->window());
    if (parent) {
      Fl_Wayland_Screen_Driver *scr_driver = (Fl_Wayland_Screen_Driver*)Fl::screen_driver();
      wl_win->subsurface = wl_subcompositor_get_subsurface(scr_driver->wl_subcompositor,
                                                wl_win->wl_surface, parent->wl_surface);
      float f = Fl::screen_scale(pWindow->top_window()->screen_num());
      wl_subsurface_set_position(wl_win->subsurface, pWindow->x() * f, pWindow->y() * f);
      wl_subsurface_set_desync(wl_win->subsurface); // important
      wl_subsurface_place_above(wl_win->subsurface, parent->wl_surface);
      wl_win->configured_width = pWindow->w();
      wl_win->configured_height = pWindow->h();
      wait_for_expose_value = 0;
      pWindow->redraw();
    }
  }
}


void Fl_Wayland_Window_Driver::unmap() {
  Fl_X* ip = Fl_X::flx(pWindow);
  struct wld_window *wl_win = (struct wld_window*)ip->xid;
  if (wl_win->kind == SUBWINDOW && wl_win->wl_surface) {
    wl_surface_attach(wl_win->wl_surface, NULL, 0, 0);
    Fl_Wayland_Graphics_Driver::buffer_release(wl_win);
    wl_subsurface_destroy(wl_win->subsurface);
    wl_win->subsurface = NULL;
  }
}


void Fl_Wayland_Window_Driver::size_range() {
  if (shown()) {
    Fl_X* ip = Fl_X::flx(pWindow);
    struct wld_window *wl_win = (struct wld_window*)ip->xid;
    float f = Fl::screen_scale(pWindow->screen_num());
    int minw, minh, maxw, maxh;
    pWindow->get_size_range(&minw, &minh, &maxw, &maxh, NULL, NULL, NULL);
    if (wl_win->kind == DECORATED && wl_win->frame) {
      int X,Y,W,H;
      Fl::screen_work_area(X,Y,W,H, Fl::screen_num(x(),y(),w(),h()));
      if (maxw && maxw < W && maxh && maxh < H) {
        libdecor_frame_unset_capabilities(wl_win->frame, LIBDECOR_ACTION_FULLSCREEN);
      } else {
        libdecor_frame_set_capabilities(wl_win->frame, LIBDECOR_ACTION_FULLSCREEN);
      }
      if (maxw && maxh && (minw >= maxw || minh >= maxh)) {
        libdecor_frame_unset_capabilities(wl_win->frame, LIBDECOR_ACTION_RESIZE);
      } else {
        libdecor_frame_set_capabilities(wl_win->frame, LIBDECOR_ACTION_RESIZE);
      }
      libdecor_frame_set_min_content_size(wl_win->frame, minw*f, minh*f);
      libdecor_frame_set_max_content_size(wl_win->frame, maxw*f, maxh*f);
      if (xdg_toplevel()) {
        struct libdecor_state *state = libdecor_state_new(int(w() * f), int(h() * f));
        libdecor_frame_commit(wl_win->frame, state, NULL);
        libdecor_state_free(state);
      }
    } else if (wl_win->kind == UNFRAMED && wl_win->xdg_toplevel) {
      xdg_toplevel_set_min_size(wl_win->xdg_toplevel, minw*f, minh*f);
      if (maxw && maxh)
          xdg_toplevel_set_max_size(wl_win->xdg_toplevel, maxw*f, maxh*f);
    }
  }
}


void Fl_Wayland_Window_Driver::iconize() {
  Fl_X* ip = Fl_X::flx(pWindow);
  struct wld_window *wl_win = (struct wld_window*)ip->xid;
  if (wl_win->kind == DECORATED) {
    libdecor_frame_set_minimized(wl_win->frame);
    Fl::handle(FL_HIDE, pWindow);
  }
  else if (wl_win->kind == UNFRAMED && wl_win->xdg_toplevel) xdg_toplevel_set_minimized(wl_win->xdg_toplevel);
}


void Fl_Wayland_Window_Driver::decoration_sizes(int *top, int *left,  int *right, int *bottom)
{
  struct wld_window *xid = (struct wld_window*)fl_xid(pWindow);
  if (xid && xid->kind == DECORATED) {
    libdecor_frame_translate_coordinate(xid->frame, 0, 0, left, top);
    *right = *left;
    *bottom = 0;
  } else {
    Fl_Window_Driver::decoration_sizes(top, left, right, bottom);
  }
}


int Fl_Wayland_Window_Driver::scroll(int src_x, int src_y, int src_w, int src_h,
                                     int dest_x, int dest_y,
                                     void (*draw_area)(void*, int,int,int,int), void* data)
{
  struct wld_window * xid = fl_wl_xid(pWindow);
  struct Fl_Wayland_Graphics_Driver::wld_buffer *buffer = xid->buffer;
  float s = wld_scale() * fl_graphics_driver->scale();
  if (s != 1) {
    src_x = src_x * s;
    src_y = src_y * s;
    src_w = src_w * s;
    src_h = src_h * s;
    dest_x = dest_x * s;
    dest_y = dest_y * s;
  }
  if (src_x == dest_x) { // vertical scroll
    int i, to, step;
    if (src_y > dest_y) {
      i = 0; to = src_h; step = 1;
    } else {
      i = src_h - 1; to = -1; step = -1;
    }
    while (i != to) {
      memcpy(
        buffer->draw_buffer.buffer + (dest_y + i) * buffer->draw_buffer.stride + 4 * dest_x,
        buffer->draw_buffer.buffer + (src_y + i) * buffer->draw_buffer.stride + 4 * src_x,
        4 * src_w);
      i += step;
    }
  } else { // horizontal scroll
    int i, to, step;
    if (src_x > dest_x) {
      i = 0; to = src_h; step = 1;
    } else {
      i = src_h - 1; to = -1; step = -1;
    }
    while (i != to) {
      memmove(
        buffer->draw_buffer.buffer + (src_y + i) * buffer->draw_buffer.stride + 4 * dest_x,
        buffer->draw_buffer.buffer + (src_y + i) * buffer->draw_buffer.stride + 4 * src_x,
        4 * src_w);
      i += step;
    }
  }
  return 0;
}


static void handle_error(struct libdecor *libdecor_context, enum libdecor_error error, const char *message)
{
  Fl::fatal("Caught error (%d): %s\n", error, message);
}


static struct libdecor_interface libdecor_iface = {
  .error = handle_error,
};


void change_scale(Fl_Wayland_Screen_Driver::output *output, struct wld_window *window,
                  float pre_scale) {
  Fl_Wayland_Window_Driver *win_driver = Fl_Wayland_Window_Driver::driver(window->fl_win);
  if (!window->fl_win->parent()) {
    // for top-level, set its screen number when the 1st screen for this surface changes
    Fl_Wayland_Screen_Driver::output *running_output;
    Fl_Wayland_Screen_Driver *scr_dr = (Fl_Wayland_Screen_Driver*)Fl::screen_driver();
    int i = 0;
    wl_list_for_each(running_output, &scr_dr->outputs, link) { // each screen of the system
      if (running_output == output) { // we've found our screen of the system
        win_driver->screen_num(i);
        break;
      }
      i++;
    }
  }
  float post_scale = Fl::screen_scale(win_driver->screen_num()) * output->wld_scale;
//printf("pre_scale=%.1f post_scale=%.1f\n", pre_scale, post_scale);
  if (post_scale != pre_scale) {
    if (window->kind == Fl_Wayland_Window_Driver::POPUP) {
      Fl_Wayland_Graphics_Driver::buffer_release(window);
      window->fl_win->redraw();
    } else {
      win_driver->is_a_rescale(true);
      window->fl_win->size(window->fl_win->w(), window->fl_win->h());
      win_driver->is_a_rescale(false);
    }
  }
  if (window->fl_win->as_gl_window())
    wl_surface_set_buffer_scale(window->wl_surface, output->wld_scale);
}


static void surface_enter(void *data, struct wl_surface *wl_surface,
                          struct wl_output *wl_output) {
  struct wld_window *window = (struct wld_window*)data;

  if (!Fl_Wayland_Screen_Driver::own_output(wl_output))
    return;

  Fl_Wayland_Screen_Driver::output *output =
    (Fl_Wayland_Screen_Driver::output*)wl_output_get_user_data(wl_output);
  if (output == NULL)
    return;

  Fl_Wayland_Window_Driver *win_driver = Fl_Wayland_Window_Driver::driver(window->fl_win);
  float pre_scale = Fl::screen_scale(win_driver->screen_num()) * win_driver->wld_scale();
  bool list_was_empty = wl_list_empty(&window->outputs);
  struct Fl_Wayland_Window_Driver::surface_output *surface_output =
  (struct Fl_Wayland_Window_Driver::surface_output*)malloc(
                    sizeof(struct Fl_Wayland_Window_Driver::surface_output));
  surface_output->output = output;
  // add to end of the linked list of displays of this surface
  struct wl_list *e = &window->outputs;
  while (e->next != &window->outputs) e = e->next; // move e to end of linked list
  wl_list_insert(e, &surface_output->link);
//printf("window %p enters screen id=%d length=%d\n", window->fl_win, output->id, wl_list_length(&window->outputs));
  if (list_was_empty) {
    change_scale(output, window, pre_scale);
  }
}


static void surface_leave(void *data, struct wl_surface *wl_surface,
                          struct wl_output *wl_output) {
  if (!Fl_Wayland_Screen_Driver::own_output(wl_output))
      return;
  struct wld_window *window = (struct wld_window*)data;
  Fl_Wayland_Screen_Driver::output *output =
    (Fl_Wayland_Screen_Driver::output*)wl_output_get_user_data(wl_output);
  Fl_Wayland_Window_Driver *win_driver = Fl_Wayland_Window_Driver::driver(window->fl_win);
  float pre_scale = Fl::screen_scale(win_driver->screen_num()) * win_driver->wld_scale();
  struct Fl_Wayland_Window_Driver::surface_output *s_output;
  int count = 0;
  wl_list_for_each(s_output, &window->outputs, link) {
    count++;
    if (s_output->output == output) {
      wl_list_remove(&s_output->link);
      free(s_output);
//printf("window %p leaves screen id=%d length=%d\n", window->fl_win, output->id, wl_list_length(&window->outputs));
      break;
    }
  }
  if (count == 1 && !wl_list_empty(&window->outputs)) {
    s_output = wl_container_of(window->outputs.next, s_output, link);
    change_scale(s_output->output, window, pre_scale);
  }
}


static struct wl_surface_listener surface_listener = {
  surface_enter,
  surface_leave,
};


Fl_Window *Fl_Wayland_Window_Driver::surface_to_window(struct wl_surface *surface) {
  if (surface) {
    if (wl_proxy_get_listener((struct wl_proxy *)surface) == &surface_listener) {
      return ((struct wld_window *)wl_surface_get_user_data(surface))->fl_win;
    }
  }
  return NULL;
}


static struct Fl_Wayland_Screen_Driver::output *screen_num_to_output(int num_screen) {
  Fl_Wayland_Screen_Driver *scr_driver = (Fl_Wayland_Screen_Driver*)Fl::screen_driver();
  int i = 0;
  Fl_Wayland_Screen_Driver::output *output;
  wl_list_for_each(output, &(scr_driver->outputs), link) { // all screens of the system
    if (i++ == num_screen) return output;
  }
  return NULL;
}


#define LIBDECOR_MR131 1 // this means libdecor does not include MR!131 yet

#ifdef LIBDECOR_MR131
/* === Beginning of hack that would become un-needed if libdecor accepted MR!131 === */

// true while the GUI is interactively resizing a decorated window
static bool in_decorated_window_resizing = false;


// libdecor's configure cb function for xdg_toplevel objects
static void (*decor_xdg_toplevel_configure)(void*, struct xdg_toplevel *, int32_t,
                                            int32_t, struct wl_array *);


static void fltk_xdg_toplevel_configure(void *user_data, struct xdg_toplevel *xdg_toplevel,
                                        int32_t width, int32_t height,
                                        struct wl_array *states) {
  uint32_t *p;
  in_decorated_window_resizing = false;
  // Replace wl_array_for_each(p, states) rejected by C++
  for (p = (uint32_t *)(states)->data;
      (const char *) p < ((const char *) (states)->data + (states)->size);
      (p)++) {
    if (*p == XDG_TOPLEVEL_STATE_RESIZING) {
      in_decorated_window_resizing = true;
      break;
    }
  }
  decor_xdg_toplevel_configure(user_data, xdg_toplevel, width, height, states);
}


struct wl_object { // copied from wayland-private.h
  const struct wl_interface *interface;
  const void *implementation;
  uint32_t id;
};


// replace libdecor's toplevel configure cb by FLTK's
static void use_FLTK_toplevel_configure_cb(struct libdecor_frame *frame) {
  struct wl_object *object = (struct wl_object *)libdecor_frame_get_xdg_toplevel(frame);
  static struct xdg_toplevel_listener *fltk_listener = NULL;
  if (!fltk_listener) {
    struct xdg_toplevel_listener *decor_listener = (struct xdg_toplevel_listener*)
        object->implementation;
    fltk_listener = (struct xdg_toplevel_listener*)
        malloc(sizeof(struct xdg_toplevel_listener));
    // initialize FLTK's listener with libdecor's values
    *fltk_listener = *decor_listener;
    // memorize libdecor's toplevel configure cb
    decor_xdg_toplevel_configure = decor_listener->configure;
    // replace libdecor's toplevel configure cb by FLTK's
    fltk_listener->configure = fltk_xdg_toplevel_configure;
  }
  // replace the toplevel listener by a copy whose configure member is FLTK's
  object->implementation = fltk_listener;
}

/* === End of hack that would become un-needed if libdecor accepted MR!131 === */
#endif // LIBDECOR_MR131


// does win entirely cover its parent ?
static void does_window_cover_parent(Fl_Window *win) {
  Fl_Window *parent = win->window();
  fl_wl_xid(parent)->covered = (win->x() <= 0 && win->y() <= 0 &&
                                win->w() >= parent->w() && win->h() >= parent->h());
}


// recursively explore all shown subwindows in a window and call f for each
static void scan_subwindows(Fl_Group *g, void (*f)(Fl_Window *)) {
  for (int i = 0; i < g->children(); i++) {
    Fl_Widget *o = g->child(i);
    if (o->as_window()) {
      if (!o->as_window()->shown()) continue;
      f(o->as_window());
    }
    if (o->as_group()) scan_subwindows(o->as_group(), f);
  }
}


static void handle_configure(struct libdecor_frame *frame,
     struct libdecor_configuration *configuration, void *user_data)
{
  struct wld_window *window = (struct wld_window*)user_data;
  if (!window->wl_surface) return;
  int width, height;
  enum libdecor_window_state window_state;
  struct libdecor_state *state;
  Fl_Wayland_Window_Driver *driver = Fl_Wayland_Window_Driver::driver(window->fl_win);
  // true exactly for the 1st run of handle_configure() for this window
  bool is_1st_run = (window->xdg_surface == 0);
  // true exactly for the 2nd run of handle_configure() for this window
  bool is_2nd_run = (window->xdg_surface != 0 && driver->wait_for_expose_value);
  float f = Fl::screen_scale(window->fl_win->screen_num());

  if (!window->xdg_surface) window->xdg_surface = libdecor_frame_get_xdg_surface(frame);

#ifdef LIBDECOR_MR131
  if (is_1st_run) use_FLTK_toplevel_configure_cb(frame);
#endif
  struct wl_output *wl_output = NULL;
  if (window->fl_win->fullscreen_active()) {
    if (!(window->state & LIBDECOR_WINDOW_STATE_FULLSCREEN)) {
      if (Fl_Window_Driver::driver(window->fl_win)->force_position()) {
        struct Fl_Wayland_Screen_Driver::output *output =
        screen_num_to_output(window->fl_win->screen_num());
        if (output) wl_output = output->wl_output;
      }
      libdecor_frame_set_fullscreen(window->frame, wl_output);
    }
  } else if (driver->show_iconic()) {
    libdecor_frame_set_minimized(window->frame);
    driver->show_iconic(0);
  }
  if (!libdecor_configuration_get_window_state(configuration, &window_state))
    window_state = LIBDECOR_WINDOW_STATE_NONE;
  if ((window->state & LIBDECOR_WINDOW_STATE_FULLSCREEN) &&
      !(window_state & LIBDECOR_WINDOW_STATE_FULLSCREEN) && !window->fl_win->border()) {
    // necessary so Mutter correctly positions borderless window back from fullscreen
    window->fl_win->redraw();
  }
  window->state = window_state;

  // Weston, KWin, and some versions of Mutter, on purpose, don't set the
  // window width x height when xdg_toplevel_configure runs twice
  // during resizable window creation
  // (see https://gitlab.freedesktop.org/wayland/wayland-protocols/-/issues/6).
  // Consequently, libdecor_configuration_get_content_size() may return false twice.
  // In that case libdecor_frame_get_content_{width,height}() give the desired window size
  if (!libdecor_configuration_get_content_size(configuration, frame, &width, &height)) {
    if (is_2nd_run) {
      width = libdecor_frame_get_content_width(frame);
      height = libdecor_frame_get_content_height(frame);
      if (!driver->is_resizable()) {
        libdecor_frame_set_min_content_size(frame, width, height);
        libdecor_frame_set_max_content_size(frame, width, height);
      }
    } else { width = height = 0; }
  }
  if (is_2nd_run && Fl_Wayland_Screen_Driver::compositor == Fl_Wayland_Screen_Driver::MUTTER) {
    scan_subwindows(window->fl_win, does_window_cover_parent); // issue #878
  }

  if (window->fl_win->fullscreen_active() &&
       Fl_Window_Driver::driver(window->fl_win)->force_position()) {
    int X, Y, W, H;
    Fl::screen_xywh(X, Y, W, H, window->fl_win->screen_num());
    width = W * f; height = H * f;
  }

  if (width == 0) {
    width = window->floating_width;
    height = window->floating_height;
    //fprintf(stderr,"handle_configure: using floating %dx%d\n",width,height);
  }

#ifndef LIBDECOR_MR131
  bool in_decorated_window_resizing = (window->state & LIBDECOR_WINDOW_STATE_RESIZING);
#endif
  bool condition = in_decorated_window_resizing;
  if (condition) { // see issue #878
    condition = (window->covered ? (window->buffer && window->buffer->in_use) : (window->frame_cb != NULL));
  }
  if (condition) {
    // Skip resizing & redrawing. The last resize request won't be skipped because
    // LIBDECOR_WINDOW_STATE_RESIZING will be off or cb will be NULL then.
    return;
  }

  driver->in_handle_configure = true;
  window->fl_win->resize(0, 0, ceil(width / f), ceil(height / f));
  driver->in_handle_configure = false;
  if (wl_output) window->fl_win->redraw();
  window->configured_width = ceil(width / f);
  window->configured_height = ceil(height / f);
  if (is_2nd_run) driver->wait_for_expose_value = 0;
//fprintf(stderr, "handle_configure fl_win=%p size:%dx%d state=%x wait_for_expose_value=%d is_2nd_run=%d\n", window->fl_win, width,height,window_state,driver->wait_for_expose_value, is_2nd_run);

/* We would like to do FL_HIDE when window is minimized but :
 "There is no way to know if the surface is currently minimized, nor is there any way to
 unset minimization on this surface. If you are looking to throttle redrawing when minimized,
 please instead use the wl_surface.frame event" */
  if (window_state & LIBDECOR_WINDOW_STATE_ACTIVE) {
    if (Fl_Wayland_Screen_Driver::compositor == Fl_Wayland_Screen_Driver::WESTON) {
      // After click on titlebar, weston calls wl_keyboard_enter() for a
      // titlebar-related surface that FLTK can't identify, so we send FL_FOCUS here.
      Fl::handle(FL_FOCUS, window->fl_win);
    }
    if (!window->fl_win->border()) libdecor_frame_set_visibility(window->frame, false);
    else if (!libdecor_frame_is_visible(window->frame)) {
      libdecor_frame_set_visibility(window->frame, true);
    }
  }

  if (window->fl_win->border())
    driver->is_maximized(window_state & LIBDECOR_WINDOW_STATE_MAXIMIZED);
  if (window_state & LIBDECOR_WINDOW_STATE_MAXIMIZED) state = libdecor_state_new(width,
                                                                                 height);
  else state = libdecor_state_new(int(ceil(width/f)*f), int(ceil(height/f)*f));
  libdecor_frame_commit(frame, state, configuration);
  if (libdecor_frame_is_floating(frame)) { // store floating dimensions
     window->floating_width = int(ceil(width/f)*f);
     window->floating_height = int(ceil(height/f)*f);
     //fprintf(stderr,"set floating_width+height %dx%d\n",width,height);
   }
  libdecor_state_free(state);

  driver->flush();
  if (Fl_Wayland_Screen_Driver::compositor != Fl_Wayland_Screen_Driver::WESTON || !is_1st_run) {
    window->fl_win->clear_damage();
  }
}


void Fl_Wayland_Window_Driver::wait_for_expose()
{
  Fl_Window_Driver::wait_for_expose();
  struct wld_window * xid = fl_wl_xid(pWindow);
  if (!xid) return;
  if (pWindow->fullscreen_active()) {
    if (xid->kind == DECORATED) {
      while (!(xid->state & LIBDECOR_WINDOW_STATE_FULLSCREEN) ||
             !(xid->state & LIBDECOR_WINDOW_STATE_ACTIVE)) {
        wl_display_dispatch(Fl_Wayland_Screen_Driver::wl_display);
      }
    } else if (xid->kind == UNFRAMED) {
      wl_display_roundtrip(Fl_Wayland_Screen_Driver::wl_display);
    }
  } else if (xid->kind == DECORATED) {
    // necessary for the windowfocus demo program with recent Wayland versions
    if (!(xid->state & LIBDECOR_WINDOW_STATE_ACTIVE)) {
      wl_display_dispatch(Fl_Wayland_Screen_Driver::wl_display);
    }
  }
}


static void delayed_close(Fl_Window *win) {
  Fl::remove_check((Fl_Timeout_Handler)delayed_close, win);
  Fl::handle(FL_CLOSE, win);
}


static void handle_close(struct libdecor_frame *frame, void *user_data)
{ // runs when the close button of a window titlebar is pushed
  // or after "Quit" of the application menu
  // or after the Kill command of Sway
  Fl_Window* win = ((struct wld_window*)user_data)->fl_win;
  int X, Y;
  libdecor_frame_translate_coordinate(frame, 0, 0, &X, &Y);
  if (Y == 0) Fl::handle(FL_CLOSE, win);
  else {
    // the close window attempt is delayed because libdecor
    // uses the frame after return from this function
    Fl::add_check((Fl_Timeout_Handler)delayed_close, win);
  }
}


static void handle_commit(struct libdecor_frame *frame, void *user_data)
{
  struct wld_window* wl_win = (struct wld_window*)user_data;
  if (wl_win->wl_surface) wl_surface_commit(wl_win->wl_surface);
}


static void handle_dismiss_popup(struct libdecor_frame *frame, const char *seat_name, void *user_data)
{
}


static struct libdecor_frame_interface libdecor_frame_iface = {
  handle_configure,
  handle_close,
  handle_commit,
  handle_dismiss_popup,
};


static void xdg_surface_configure(void *data, struct xdg_surface *xdg_surface, uint32_t serial)
{
  // runs for borderless windows and popup (menu,tooltip) windows
  struct wld_window *window = (struct wld_window*)data;
  xdg_surface_ack_configure(xdg_surface, serial);
//fprintf(stderr, "xdg_surface_configure: surface=%p\n", window->wl_surface);

  if (window->fl_win->w() != window->configured_width ||
      window->fl_win->h() != window->configured_height) {
    if (window->buffer) {
      Fl_Wayland_Graphics_Driver::buffer_release(window);
    }
  }
  window->configured_width = window->fl_win->w();
  window->configured_height = window->fl_win->h();
  Fl_Window_Driver::driver(window->fl_win)->flush();
  window->fl_win->clear_damage();
}


static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure,
};


static bool parse_states_fullscreen(struct wl_array *states)
{
  uint32_t *p;
  // Replace wl_array_for_each(p, states) rejected by C++
  for (p = (uint32_t *)(states)->data;
      (const char *) p < ((const char *) (states)->data + (states)->size);
      (p)++) {
    if (*p == XDG_TOPLEVEL_STATE_FULLSCREEN) return true;
  }
  return false;
}


static void xdg_toplevel_configure(void *data, struct xdg_toplevel *xdg_toplevel,
                                   int32_t width, int32_t height, struct wl_array *states)
{
  // runs for borderless top-level windows
  // under Weston: width & height are 0 during both calls, except if fullscreen
  struct wld_window *window = (struct wld_window*)data;
//fprintf(stderr, "xdg_toplevel_configure: surface=%p size: %dx%d\n", window->wl_surface, width, height);
  if (window->fl_win->fullscreen_active() && !parse_states_fullscreen(states)) {
    struct wl_output *wl_output = NULL;
    if (Fl_Window_Driver::driver(window->fl_win)->force_position()) {
      struct Fl_Wayland_Screen_Driver::output *output  =
        screen_num_to_output(window->fl_win->screen_num());
      if (output) wl_output = output->wl_output;
    }
    xdg_toplevel_set_fullscreen(xdg_toplevel, wl_output);
    if (wl_output) {
      int X, Y;
      Fl::screen_xywh(X, Y, width, height, window->fl_win->screen_num());
    }
  }
  if (window->configured_width) {
    Fl_Window_Driver::driver(window->fl_win)->wait_for_expose_value = 0;
  }
  float f = Fl::screen_scale(window->fl_win->screen_num());
  if (width == 0 || height == 0) {
    width = window->fl_win->w() * f;
    height = window->fl_win->h() * f;
  }
  window->fl_win->size(ceil(width / f), ceil(height / f));
  if (window->buffer && (ceil(width / f) != window->configured_width ||
                         ceil(height / f) != window->configured_height)) {
    Fl_Wayland_Graphics_Driver::buffer_release(window);
  }
  window->configured_width = ceil(width / f);
  window->configured_height = ceil(height / f);
}


static void xdg_toplevel_close(void *data, struct xdg_toplevel *toplevel)
{
}


static const struct xdg_toplevel_listener xdg_toplevel_listener = {
  .configure = xdg_toplevel_configure,
  .close = xdg_toplevel_close,
};


struct win_positioner {
  struct wld_window *window;
  int x, y;
  Fl_Window *child_popup;
};


static void popup_configure(void *data, struct xdg_popup *xdg_popup, int32_t x, int32_t y,
                            int32_t width, int32_t height) {
  struct win_positioner *win_pos = (struct win_positioner *)data;
  struct wld_window *window = win_pos->window;
//printf("popup_configure %p asked:%dx%d got:%dx%d\n",window->fl_win, win_pos->x,win_pos->y, x,y);
  Fl_Window_Driver::driver(window->fl_win)->wait_for_expose_value = 0;
  int HH;
  Fl_Window_Driver::menu_parent(&HH);
  if (window->fl_win->h() > HH && y != win_pos->y) { // A menu taller than the display
    // Under KWin, height is set to the display height or less: we ignore that.
    window->state = (y - win_pos->y);
    // make selected item visible, if there's one
    Fl_Window_Driver::scroll_to_selected_item(window->fl_win);
  }
}


static struct xdg_popup *mem_grabbing_popup = NULL;


static void popup_done(void *data, struct xdg_popup *xdg_popup) {
  struct win_positioner *win_pos = (struct win_positioner *)data;
  struct wld_window *window = win_pos->window;
//fprintf(stderr, "popup_done: popup=%p data=%p xid=%p fl_win=%p\n", xdg_popup, data, window, window->fl_win);
  if (win_pos->child_popup) win_pos->child_popup->hide();
  xdg_popup_destroy(xdg_popup);
  delete win_pos;
  // The sway compositor calls popup_done directly and hides the menu
  // when the app looses focus.
  // Thus, we hide the window so FLTK and Wayland are in matching states.
  window->xdg_popup = NULL;
  window->fl_win->hide();
  if (mem_grabbing_popup == xdg_popup) {
    mem_grabbing_popup = NULL;
  }
}


static const struct xdg_popup_listener popup_listener = {
  .configure = popup_configure,
  .popup_done = popup_done,
};


bool Fl_Wayland_Window_Driver::in_flush_ = false;


static const char *get_prog_name() {
  pid_t pid = getpid();
  char fname[100];
  snprintf(fname, 100, "/proc/%u/cmdline", pid);
  FILE *in = fopen(fname, "r");
  if (in) {
    static char line[200];
    const char *p = fgets(line, sizeof(line), in);
    fclose(in);
    p = strrchr(line, '/'); if (!p) p = line; else p++;
    return p;
  }
  return "unknown";
}


/* Implementation note about menu windows under Wayland.
 Wayland offers a way to position popup windows such as menu windows using constraints.
 Each popup is located relatively to a parent window which can be a popup itself and
 MUST overlap or at least touch this parent.
 Constraints determine how a popup is positioned relatively to a defined area (called
 the anchor rectangle) of its parent popup/window and what happens when this position
 would place the popup all or partly outside the display.
 In contrast, FLTK computes the adequate positions of menu windows in the display using
 knowledge about the display size and the location of the window in the display, and then
 maps them at these positions.
 These 2 logics are quite different because Wayland hides the position of windows inside the
 display, whereas FLTK uses the location of windows inside the display to position popups.
 Let's call "source window" the non-popup window above which all popups are mapped.
 The approach implemented here is two-fold.
 1) If a menu window is not taller than the display, use Wayland constraints to position it.
 Wayland imposes that the first constructed popup must overlap or touch the source window.
 Other popups can be placed below, above, at right, or at left of a previous popup which
 allows them to expand outside the source window, while constraints can ensure they won't
 extend outside the display.
 2) A menu window taller than the display is initially mapped with the constraint to
 begin at the top border of the display. This allows FLTK to know the distance between
 the source window and the display top. FLTK can later reposition the same tall popup,
 without the constraint not to go beyond the display top, at the exact position so that
 the desired series of menu items appear in the visible part of the tall popup.

 In case 1) above, the values that represent the display bounds are given very
 large values. That's done by member function Fl_Wayland_Window_Driver::menu_window_area().
 Consequently, FLTK computes an initial layout of future popups relatively to
 the source window as if it was mapped on an infinitely large display. Then, the location
 of the first popup to be mapped is modified if necessary so it overlaps or touches the
 source window. Finally, other popups are located using Wayland logic below, above or to the
 right of previous popups. Wayland constraints mechanism also allows a popup tentatively
 placed below a previous one to be flipped above it if that prevents the popup from expanding
 beyond display limits. This is used to unfold menu bar menus below or above the menu bar.
 After each popup is created and scheduled for being mapped on display by function
 process_menu_or_tooltip(), makeWindow() calls Fl_Window::wait_for_expose() so its constrained
 position is known before computing the position of the next popup. This ensures each
 popup is correctly placed relatively to its parent.

 Groups of popups containing a menutitle, the associated menuwindow, and optionally
 a submenu window and that don't belong to an Fl_Menu_Bar are mapped in a different order:
 the menuwindow is mapped first, and the menutitle is mapped second above it as a child popup.
 Fl_Window_Driver::is_floating_title() detects when such a menutitle is created,
 static member variable previous_floatingtitle is assigned the value of this menutitle, and
 the menutitle is mapped only after the menuwindow has been mapped, as a child of it.
 This positions better the popup group in the display relatively to where the popup
 was created.

 In case 2) above, a tall popup is mapped with XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_Y
 which puts its top at the display top border. The Wayland system then calls the
 popup_configure() callback function with the x,y coordinates of the top left corner
 where the popup is mapped relatively to an anchor point in the source window.
 The difference between the asked window position and the effective position is stored
 in the state member variable of the tall popup's struct wld_window. This information
 allows FLTK to compute the distance between the source window top and the display top border.
 Function Fl_Wayland_Window_Driver::menu_window_area() sets the top of the display to
 a value such that function Fl_Wayland_Window_Driver::reposition_menu_window(), called by
 menuwindow::autoscroll(int n), ensures that menu item #n is visible. Static boolean member
 variable Fl_Wayland_Window_Driver::new_popup is useful to position tall menuwindows created
 by an Fl_Menu_Button or Fl_Choice. It is set to true when any menu popup is created.
 It is used each time menu_window_area() runs for a particular Fl_Menu_Button or Fl_Choice,
 and is reset to false after its first use. This allows menu_window_area() to give the top of
 the display an adequate value the first time and to keep this value next times it runs.
 Fl_Window_Driver::scroll_to_selected_item() scrolls the tall popup so its selected
 item, when there's one, is visible immediately after the tall popup is mapped on display.
 */


bool Fl_Wayland_Window_Driver::process_menu_or_tooltip(struct wld_window *new_window) {
  // a menu window or tooltip
  new_window->kind = Fl_Wayland_Window_Driver::POPUP;
  Fl_Wayland_Screen_Driver *scr_driver = (Fl_Wayland_Screen_Driver*)Fl::screen_driver();
  if (Fl_Window_Driver::is_floating_title(pWindow)) {
    previous_floatingtitle = pWindow;
    return true;
  }
  new_window->xdg_surface = xdg_wm_base_get_xdg_surface(scr_driver->xdg_wm_base,
                                                        new_window->wl_surface);
  xdg_surface_add_listener(new_window->xdg_surface, &xdg_surface_listener, new_window);
  Fl_Wayland_Window_Driver::new_popup = true;
  Fl_Window *menu_origin = NULL;
  if (pWindow->menu_window()) {
    menu_origin = Fl_Window_Driver::menu_leftorigin(pWindow);
    if (!menu_origin && !previous_floatingtitle) menu_origin =
      Fl_Window_Driver::menu_title(pWindow);
  }
  Fl_Widget *target = (pWindow->tooltip_window() ? Fl_Tooltip::current() : NULL);
  if (pWindow->user_data() == &Fl_Screen_Driver::transient_scale_display &&
      Fl_Screen_Driver::transient_scale_parent) {
    target = Fl_Screen_Driver::transient_scale_parent;
  }
  if (!target) target = Fl_Window_Driver::menu_parent();
  if (!target) target = Fl::belowmouse();
  if (!target) target = Fl::first_window();
  Fl_Window *parent_win = target->top_window();
  while (parent_win && parent_win->menu_window() && driver(parent_win)->popup_window()) {
    parent_win = Fl::next_window(parent_win);
  }
  Fl_Window *origin_win = (menu_origin ? menu_origin : parent_win);
  struct wld_window * parent_xid = fl_wl_xid(origin_win);
  struct xdg_surface *parent_xdg = parent_xid->xdg_surface;
  float f = Fl::screen_scale(parent_win->screen_num());
  //fprintf(stderr, "menu parent_win=%p pos:%dx%d size:%dx%d\n", parent_win, pWindow->x(), pWindow->y(), pWindow->w(), pWindow->h());
//printf("window=%p menutitle=%p bartitle=%d leftorigin=%p y=%d\n", pWindow, Fl_Window_Driver::menu_title(pWindow), Fl_Window_Driver::menu_bartitle(pWindow), Fl_Window_Driver::menu_leftorigin(pWindow), pWindow->y());
  struct xdg_positioner *positioner = xdg_wm_base_create_positioner(scr_driver->xdg_wm_base);
  //xdg_positioner_get_version(positioner) <== gives 1 under Debian and Sway
  int popup_x, popup_y;
  if (Fl_Window_Driver::menu_title(pWindow) && Fl_Window_Driver::menu_bartitle(pWindow)) {
    xdg_positioner_set_anchor_rect(positioner, 0, 0,
                                   Fl_Window_Driver::menu_title(pWindow)->w() * f,
                                   Fl_Window_Driver::menu_title(pWindow)->h() * f);
    popup_x = 0;
    popup_y = Fl_Window_Driver::menu_title(pWindow)->h() * f;
  } else {
    popup_x = pWindow->x() * f; popup_y = pWindow->y() * f;
    if (popup_x + pWindow->w() * f < 0) popup_x = - pWindow->w() * f;
    if (menu_origin) {
      popup_x -= menu_origin->x() * f;
      popup_y -= menu_origin->y() * f;
    }
    if (popup_x >= origin_win->w() * f) popup_x = origin_win->w() * f - 1;
    if (!Fl_Window_Driver::menu_title(pWindow) && !Fl_Window_Driver::menu_bartitle(pWindow) &&
        !Fl_Window_Driver::menu_leftorigin(pWindow)) {
      // prevent first popup from going above the permissible source window
      popup_y = fl_max(popup_y, - pWindow->h() * f);
    }
    if (parent_xid->kind == Fl_Wayland_Window_Driver::DECORATED)
      libdecor_frame_translate_coordinate(parent_xid->frame, popup_x, popup_y,
                                          &popup_x, &popup_y);
    xdg_positioner_set_anchor_rect(positioner, popup_x, 0, 1, 1);
    popup_y++;
  }
  xdg_positioner_set_size(positioner, pWindow->w() * f , pWindow->h() * f );
  xdg_positioner_set_anchor(positioner, XDG_POSITIONER_ANCHOR_BOTTOM_LEFT);
  xdg_positioner_set_gravity(positioner, XDG_POSITIONER_GRAVITY_BOTTOM_RIGHT);
  // prevent menuwindow from expanding beyond display limits
  int constraint = 0;
  if ( !(parent_win->fullscreen_active() &&
        Fl_Wayland_Screen_Driver::compositor == Fl_Wayland_Screen_Driver::MUTTER &&
        ((!Fl_Window_Driver::menu_title(pWindow) && !Fl_Window_Driver::menu_leftorigin(pWindow)) ||
          Fl_Window_Driver::menu_bartitle(pWindow)))
     ) {
    // Condition above is only to bypass Mutter bug for fullscreen windows (see #1061)
    constraint |= (XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_X | XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_Y);
    if (Fl_Window_Driver::menu_bartitle(pWindow) && !Fl_Window_Driver::menu_leftorigin(pWindow)) {
      constraint |= XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_FLIP_Y;
    }
    xdg_positioner_set_constraint_adjustment(positioner, constraint);
  }
  if (!(Fl_Window_Driver::menu_title(pWindow) && Fl_Window_Driver::menu_bartitle(pWindow))) {
    xdg_positioner_set_offset(positioner, 0, popup_y);
  }
  new_window->xdg_popup = xdg_surface_get_popup(new_window->xdg_surface,
                                                parent_xdg, positioner);
  struct win_positioner *win_pos = new struct win_positioner;
  win_pos->window = new_window;
  win_pos->x = popup_x;
  win_pos->y = popup_y;
  win_pos->child_popup = NULL;
//printf("create xdg_popup=%p data=%p xid=%p fl_win=%p\n",new_window->xdg_popup,win_pos,new_window,new_window->fl_win);
  xdg_positioner_destroy(positioner);
  xdg_popup_add_listener(new_window->xdg_popup, &popup_listener, win_pos);
  if (!mem_grabbing_popup) {
    mem_grabbing_popup = new_window->xdg_popup;
    //xdg_popup_grab(new_window->xdg_popup, scr_driver->get_wl_seat(), scr_driver->get_serial());
    //libdecor_frame_popup_grab(parent_xid->frame, scr_driver->get_seat_name());
  }
  wl_surface_commit(new_window->wl_surface);
  // put it on same screen as parent_win
  this->screen_num(parent_win->screen_num());
  return false;
}


void Fl_Wayland_Window_Driver::makeWindow()
{
  Fl_Group::current(0); // get rid of very common user bug: forgot end()
  struct wld_window *new_window;
  bool is_floatingtitle = false;
  wait_for_expose_value = 1;

  if (pWindow->parent() && !pWindow->window()) return;
  if (pWindow->parent() && !pWindow->window()->shown()) return;

  if (!pWindow->parent() && !popup_window()) {
    x(0); y(0); // toplevel, non-popup windows must have origin at 0,0
  }
  new_window = (struct wld_window *)calloc(1, sizeof *new_window);
  new_window->fl_win = pWindow;
  wl_list_init(&new_window->outputs);
  Fl_Wayland_Screen_Driver *scr_driver = (Fl_Wayland_Screen_Driver*)Fl::screen_driver();

  new_window->wl_surface = wl_compositor_create_surface(scr_driver->wl_compositor);
  //Fl::warning("makeWindow:%p wayland-scale=%d user-scale=%.2f\n", pWindow, new_window->scale, Fl::screen_scale(0));
  wl_surface_add_listener(new_window->wl_surface, &surface_listener, new_window);

  if (!shape()) { // rectangular FLTK windows are opaque
    struct wl_region *opaque = wl_compositor_create_region(scr_driver->wl_compositor);
    wl_region_add(opaque, 0, 0, 1000000, 1000000);
    wl_surface_set_opaque_region(new_window->wl_surface, opaque);
    wl_region_destroy(opaque);
  }

  if (pWindow->user_data() == &Fl_Screen_Driver::transient_scale_display &&
      Fl::first_window()) {
  // put transient scale win at center of top window by making it a tooltip of top
    Fl_Screen_Driver::transient_scale_parent = Fl::first_window();
    pWindow->set_tooltip_window();
    set_popup_window();
    pWindow->position(
              (Fl_Screen_Driver::transient_scale_parent->w() - pWindow->w())/2 ,
              (Fl_Screen_Driver::transient_scale_parent->h() - pWindow->h())/2);
  }

  if (popup_window()) { // a menu window or tooltip
    is_floatingtitle = process_menu_or_tooltip(new_window);

  } else if (pWindow->border() && !pWindow->parent() ) { // a decorated window
    new_window->kind = DECORATED;
    if (!scr_driver->libdecor_context)
      scr_driver->libdecor_context = libdecor_new(Fl_Wayland_Screen_Driver::wl_display,
                                                  &libdecor_iface);
    new_window->frame = libdecor_decorate(scr_driver->libdecor_context, new_window->wl_surface,
                                          &libdecor_frame_iface, new_window);
    // appears in the Gnome desktop menu bar
    libdecor_frame_set_app_id(new_window->frame, get_prog_name());
    libdecor_frame_set_title(new_window->frame, pWindow->label()?pWindow->label():"");
    if (!is_resizable()) {
      libdecor_frame_unset_capabilities(new_window->frame, LIBDECOR_ACTION_RESIZE);
      libdecor_frame_unset_capabilities(new_window->frame, LIBDECOR_ACTION_FULLSCREEN);
    }
    libdecor_frame_map(new_window->frame);
    float f = Fl::screen_scale(pWindow->screen_num());
    new_window->floating_width = pWindow->w() * f;
    new_window->floating_height = pWindow->h() * f;

  } else if (pWindow->parent()) { // for subwindows (GL or non-GL)
    new_window->kind = SUBWINDOW;
    struct wld_window *parent = fl_wl_xid(pWindow->window());
    new_window->subsurface = wl_subcompositor_get_subsurface(scr_driver->wl_subcompositor,
                                                             new_window->wl_surface,
                                                             parent->wl_surface);
//fprintf(stderr, "makeWindow: subsurface=%p\n", new_window->subsurface);
    float f = Fl::screen_scale(pWindow->top_window()->screen_num());
    wl_subsurface_set_position(new_window->subsurface, pWindow->x() * f, pWindow->y() * f);
    wl_subsurface_set_desync(new_window->subsurface); // important
    // Next 5 statements ensure the subsurface will be mapped because:
    // "The effect of adding a sub-surface becomes visible on the next time
    // the state of the parent surface is applied."
    new_window->configured_width = pWindow->w();
    new_window->configured_height = pWindow->h();
    if (!pWindow->as_gl_window())  {
      parent->fl_win->wait_for_expose();
      wl_surface_commit(parent->wl_surface);
    }
    wait_for_expose_value = 0;
    pWindow->border(0);
    checkSubwindowFrame(); // make sure subwindow doesn't leak outside parent

  } else { // a window without decoration
    new_window->kind = UNFRAMED;
    new_window->xdg_surface = xdg_wm_base_get_xdg_surface(scr_driver->xdg_wm_base,
                                                          new_window->wl_surface);
//fprintf(stderr, "makeWindow: xdg_wm_base_get_xdg_surface=%p\n", new_window->xdg_surface);
    xdg_surface_add_listener(new_window->xdg_surface, &xdg_surface_listener, new_window);
    new_window->xdg_toplevel = xdg_surface_get_toplevel(new_window->xdg_surface);
    xdg_toplevel_add_listener(new_window->xdg_toplevel, &xdg_toplevel_listener, new_window);
    if (pWindow->label()) xdg_toplevel_set_title(new_window->xdg_toplevel, pWindow->label());
    wl_surface_commit(new_window->wl_surface);
    pWindow->border(0);
  }

  Fl_Window *old_first = Fl::first_window();
  struct wld_window * first_xid = (old_first ? fl_wl_xid(old_first) : NULL);
  Fl_X *xp = new Fl_X;
  xp->xid = (fl_uintptr_t)new_window;
  other_xid = 0;
  xp->w = pWindow;
  flx(xp);
  xp->region = 0;
  if (!pWindow->parent()) {
    xp->next = Fl_X::first;
    Fl_X::first = xp;
  } else if (Fl_X::first) {
    xp->next = Fl_X::first->next;
    Fl_X::first->next = xp;
  } else {
    xp->next = NULL;
    Fl_X::first = xp;
  }

  if (pWindow->modal() || pWindow->non_modal()) {
    if (pWindow->modal()) Fl::modal_ = pWindow;
    if (new_window->kind == DECORATED && first_xid && first_xid->kind == DECORATED) {
     if (first_xid->frame) libdecor_frame_set_parent(new_window->frame, first_xid->frame);
    } else if (new_window->kind == UNFRAMED && new_window->xdg_toplevel && first_xid) {
      Fl_Wayland_Window_Driver *top_dr = Fl_Wayland_Window_Driver::driver(first_xid->fl_win);
      if (top_dr->xdg_toplevel()) xdg_toplevel_set_parent(new_window->xdg_toplevel,
                                                          top_dr->xdg_toplevel());
    }
    if (scr_driver->seat->gtk_shell && pWindow->modal() &&
        (new_window->kind == DECORATED || new_window->kind == UNFRAMED)) {
      // Useful to position modal windows above their parent with "gnome-shell --version" ≤ 45.2,
      // useless but harmless with "gnome-shell --version" ≥ 46.0.
      struct gtk_surface1 *gtk_surface = gtk_shell1_get_gtk_surface(scr_driver->seat->gtk_shell,
                                                                    new_window->wl_surface);
      gtk_surface1_set_modal(gtk_surface);
      if (gtk_surface1_get_version(gtk_surface) >= GTK_SURFACE1_RELEASE_SINCE_VERSION)
        gtk_surface1_release(gtk_surface); // very necessary
      else
        gtk_surface1_destroy(gtk_surface);
    }
  }

  size_range();
  pWindow->set_visible();
  int old_event = Fl::e_number;
  pWindow->redraw();
  pWindow->handle(Fl::e_number = FL_SHOW); // get child windows to appear
  Fl::e_number = old_event;
  if (pWindow->menu_window() && popup_window() && !is_floatingtitle) {
    // make sure each menu window is mapped with its constraints before mapping next popup
    pWindow->wait_for_expose();
    if (previous_floatingtitle) { // a menuwindow with a menutitle
      //puts("previous_floatingtitle");
      int HH;
      Fl_Window_Driver::menu_parent(&HH);
      if (pWindow->h() > HH) {
        // a tall menuwindow with a menutitle: don't create the menutitle at all
        // and undo what has been created/allocated before
        struct wld_window *xid = fl_wl_xid(previous_floatingtitle);
        destroy_surface_caution_pointer_focus(xid->wl_surface, scr_driver->seat);
        free(xid);
        Fl_Window_Driver::driver(previous_floatingtitle)->hide_common();
        previous_floatingtitle = NULL;
        return;
      }
      // map the menutitle popup now as child of pWindow
      struct wld_window *xid = fl_wl_xid(previous_floatingtitle);
      xid->xdg_surface = xdg_wm_base_get_xdg_surface(scr_driver->xdg_wm_base, xid->wl_surface);
      xdg_surface_add_listener(xid->xdg_surface, &xdg_surface_listener, xid);
      struct xdg_positioner *positioner =
        xdg_wm_base_create_positioner(scr_driver->xdg_wm_base);
      xdg_positioner_set_anchor_rect(positioner, 0, 0, 1, 1);
      int snum = Fl_Window_Driver::menu_parent()->screen_num();
      float f = Fl::screen_scale(snum);
      // put it on same screen as parent menu
      Fl_Window_Driver::driver(previous_floatingtitle)->screen_num(snum);
      xdg_positioner_set_size(positioner, previous_floatingtitle->w() * f ,
                              previous_floatingtitle->h() * f );
      xdg_positioner_set_anchor(positioner, XDG_POSITIONER_ANCHOR_TOP_LEFT);
      xdg_positioner_set_gravity(positioner, XDG_POSITIONER_GRAVITY_TOP_RIGHT);
      xid->xdg_popup = xdg_surface_get_popup(xid->xdg_surface, new_window->xdg_surface,
                                             positioner);
      xdg_positioner_destroy(positioner);
      struct win_positioner *win_pos = new struct win_positioner;
      win_pos->window = xid;
      win_pos->x = 0;
      win_pos->y = 0;
      win_pos->child_popup = NULL;
      xdg_popup_add_listener(xid->xdg_popup, &popup_listener, win_pos);
      wl_surface_commit(xid->wl_surface);
      struct win_positioner *parent_win_pos =
        (struct win_positioner*)xdg_popup_get_user_data(new_window->xdg_popup);
      parent_win_pos->child_popup = previous_floatingtitle;
      previous_floatingtitle = NULL;
    }
  }
}


Fl_Wayland_Window_Driver::type_for_resize_window_between_screens Fl_Wayland_Window_Driver::data_for_resize_window_between_screens_ = {0, false};


void Fl_Wayland_Window_Driver::resize_after_screen_change(void *data) {
  Fl_Window *win = (Fl_Window*)data;
  float f = Fl::screen_driver()->scale(data_for_resize_window_between_screens_.screen);
  Fl_Window_Driver::driver(win)->resize_after_scale_change(
                                data_for_resize_window_between_screens_.screen, f, f);
  data_for_resize_window_between_screens_.busy = false;
}


int Fl_Wayland_Window_Driver::set_cursor(Fl_Cursor c) {
  Fl_Wayland_Screen_Driver *scr_driver = (Fl_Wayland_Screen_Driver*)Fl::screen_driver();
  struct wld_window *xid = (struct wld_window *)Fl_Window_Driver::xid(pWindow);
  if (!scr_driver->seat->cursor_theme) return 1;
  // Cursor names are the files of directory /usr/share/icons/XXXX/cursors/
  // where XXXX is the name of the current 'cursor theme'.
  static struct cursor_file_struct {
    Fl_Cursor c;
    const char *fname;
    Fl_Wayland_Screen_Driver::cursor_shapes wld_c;
  } cursor_file_array[] = {
    {FL_CURSOR_ARROW,  "left_ptr",            Fl_Wayland_Screen_Driver::arrow },
    {FL_CURSOR_CROSS,  "cross",               Fl_Wayland_Screen_Driver::cross },
    {FL_CURSOR_WAIT,   "watch",               Fl_Wayland_Screen_Driver::wait },
    {FL_CURSOR_INSERT, "xterm",               Fl_Wayland_Screen_Driver::insert },
    {FL_CURSOR_HAND,   "hand1",               Fl_Wayland_Screen_Driver::hand },
    {FL_CURSOR_HELP,   "help",                Fl_Wayland_Screen_Driver::help },
    {FL_CURSOR_MOVE,   "move",                Fl_Wayland_Screen_Driver::move },
    {FL_CURSOR_N,      "top_side",            Fl_Wayland_Screen_Driver::north },
    {FL_CURSOR_E,      "right_side",          Fl_Wayland_Screen_Driver::east },
    {FL_CURSOR_W,      "left_side",           Fl_Wayland_Screen_Driver::west },
    {FL_CURSOR_S,      "bottom_side",         Fl_Wayland_Screen_Driver::south },
    {FL_CURSOR_NS,     "sb_v_double_arrow",   Fl_Wayland_Screen_Driver::north_south },
    {FL_CURSOR_WE,     "sb_h_double_arrow",   Fl_Wayland_Screen_Driver::west_east },
    {FL_CURSOR_SW,     "bottom_left_corner",  Fl_Wayland_Screen_Driver::south_west },
    {FL_CURSOR_SE,     "bottom_right_corner", Fl_Wayland_Screen_Driver::south_east },
    {FL_CURSOR_NE,     "top_right_corner",    Fl_Wayland_Screen_Driver::north_east },
    {FL_CURSOR_NW,     "top_left_corner",     Fl_Wayland_Screen_Driver::north_west },
    {FL_CURSOR_NESW,   "fd_double_arrow",     Fl_Wayland_Screen_Driver::nesw },
    {FL_CURSOR_NWSE,   "bd_double_arrow",     Fl_Wayland_Screen_Driver::nwse }
  };

  int found = -1;
  for (unsigned i = 0; i < sizeof(cursor_file_array) / sizeof(struct cursor_file_struct); i++) {
    if (cursor_file_array[i].c == c) {
      found = cursor_file_array[i].wld_c;
      if (!scr_driver->xc_cursor[found]) scr_driver->xc_cursor[found] =
        scr_driver->cache_cursor(cursor_file_array[i].fname);
      if (scr_driver->xc_cursor[found]) {
        scr_driver->default_cursor(scr_driver->xc_cursor[found]);
      }
      break;
    }
  }
  if (found < 0 || !scr_driver->xc_cursor[found]) return 0;

  if (xid->custom_cursor) {
    delete_cursor(xid->custom_cursor);
    xid->custom_cursor = NULL;
  }
  standard_cursor_ = c;
  scr_driver->set_cursor();
  return 1;
}


void Fl_Wayland_Window_Driver::use_border() {
  if (!shown() || pWindow->parent()) return;
  pWindow->wait_for_expose(); // useful for border(0) just after show()
  struct libdecor_frame *frame = fl_wl_xid(pWindow)->frame;
  if (frame && Fl_Wayland_Screen_Driver::compositor != Fl_Wayland_Screen_Driver::KWIN) {
    if (fl_wl_xid(pWindow)->kind == DECORATED) {
      libdecor_frame_set_visibility(frame, pWindow->border());
    } else {
      pWindow->hide();
      pWindow->show();
    }
    pWindow->redraw();
  } else {
    Fl_Window_Driver::use_border();
  }
}


/* Change an existing window to fullscreen */
void Fl_Wayland_Window_Driver::fullscreen_on() {
    int top, bottom, left, right;

    top = fullscreen_screen_top();
    bottom = fullscreen_screen_bottom();
    left = fullscreen_screen_left();
    right = fullscreen_screen_right();

    if ((top < 0) || (bottom < 0) || (left < 0) || (right < 0)) {
      top = screen_num();
      bottom = top;
      left = top;
      right = top;
    }
  pWindow->wait_for_expose(); // make sure ->xdg_toplevel is initialized
  if (xdg_toplevel()) {
    xdg_toplevel_set_fullscreen(xdg_toplevel(), NULL);
    pWindow->_set_fullscreen();
    Fl::handle(FL_FULLSCREEN, pWindow);
  }
}


void Fl_Wayland_Window_Driver::fullscreen_off(int X, int Y, int W, int H) {
  if (!border()) pWindow->resize(X, Y, W, H);
  xdg_toplevel_unset_fullscreen(xdg_toplevel());
  pWindow->_clear_fullscreen();
  Fl::handle(FL_FULLSCREEN, pWindow);
}


void Fl_Wayland_Window_Driver::label(const char *name, const char *iname) {
  if (shown() && !parent() && fl_wl_xid(pWindow)->kind == DECORATED) {
    if (!name) name = "";
    if (!iname) iname = fl_filename_name(name);
    libdecor_frame_set_title(fl_wl_xid(pWindow)->frame, name);
  }
}


int Fl_Wayland_Window_Driver::set_cursor(const Fl_RGB_Image *rgb, int hotx, int hoty) {
  int retval = set_cursor_4args(rgb, hotx, hoty, true);
  if (retval) {
    Fl_Wayland_Screen_Driver *scr_driver = (Fl_Wayland_Screen_Driver*)Fl::screen_driver();
    struct wld_window *xid = (struct wld_window *)Fl_Window_Driver::xid(pWindow);
    Fl_Wayland_Screen_Driver::do_set_cursor(scr_driver->seat, xid->custom_cursor->wl_cursor);
  }
  return retval;
}


int Fl_Wayland_Window_Driver::set_cursor_4args(const Fl_RGB_Image *rgb, int hotx, int hoty,
                                               bool keep_copy) {
  if (keep_copy) {
    int ld = rgb->ld() ? rgb->ld() : rgb->data_w() * rgb->d();
    uchar *data = new uchar[ld * rgb->data_h()];
    memcpy(data, rgb->array, ld * rgb->data_h());
    Fl_RGB_Image *rgb2 = new Fl_RGB_Image(data, rgb->data_w(), rgb->data_h(), rgb->d(), rgb->ld());
    rgb2->alloc_array = 1;
    rgb2->scale(rgb->w(), rgb->h(), 0, 1);
    rgb = rgb2;
  }
// build a new wl_cursor and its image
  struct wld_window *xid = (struct wld_window *)Fl_Window_Driver::xid(pWindow);
  struct wl_cursor *new_cursor = (struct wl_cursor*)malloc(sizeof(struct wl_cursor));
  struct cursor_image *new_image = (struct cursor_image*)calloc(1,
                                                                sizeof(struct cursor_image));
  int scale = wld_scale();
  new_image->image.width = rgb->w() * scale;
  new_image->image.height = rgb->h() * scale;
  new_image->image.hotspot_x = hotx * scale;
  new_image->image.hotspot_y = hoty * scale;
  new_image->image.delay = 0;
  new_image->offset = 0;
  //create a Wayland buffer and have it used as an image of the new cursor
  struct Fl_Wayland_Graphics_Driver::wld_buffer *offscreen;
  Fl_Image_Surface *img_surf = Fl_Wayland_Graphics_Driver::custom_offscreen(
      new_image->image.width, new_image->image.height, &offscreen);
  new_image->buffer = offscreen->wl_buffer;
  wl_buffer_set_user_data(new_image->buffer, offscreen);
  new_cursor->image_count = 1;
  new_cursor->images = (struct wl_cursor_image**)malloc(sizeof(struct wl_cursor_image*));
  new_cursor->images[0] = (struct wl_cursor_image*)new_image;
  new_cursor->name = strdup("custom cursor");
  // draw the rgb image to the cursor's drawing buffer
  Fl_Surface_Device::push_current(img_surf);
  Fl_Wayland_Graphics_Driver *driver = (Fl_Wayland_Graphics_Driver*)img_surf->driver();
  cairo_scale(driver->cr(), scale, scale);
  ((Fl_RGB_Image*)rgb)->draw(0, 0);
  Fl_Surface_Device::pop_current();
  delete img_surf;
  memcpy(offscreen->data, offscreen->draw_buffer.buffer, offscreen->draw_buffer.data_size);
  // delete the previous custom cursor, if there was one,
  // and keep its Fl_RGB_Image if appropriate
  if (xid->custom_cursor) delete_cursor(xid->custom_cursor, keep_copy);
  //have this new cursor used
  xid->custom_cursor = new custom_cursor;
  xid->custom_cursor->wl_cursor = new_cursor;
  xid->custom_cursor->rgb = rgb;
  xid->custom_cursor->hotx = hotx;
  xid->custom_cursor->hoty = hoty;
  return 1;
}


struct xid_and_rect {
  struct wld_window *xid;
  Fl_Window *win;
  int X, Y, W, H;
  bool need_resize;
};


static void surface_frame_done(void *data, struct wl_callback *cb, uint32_t time) {
  struct xid_and_rect *xid_rect = (xid_and_rect *)data;
  wl_callback_destroy(cb);
  xid_rect->xid->frame_cb = NULL;
  if (xid_rect->need_resize) {
    xid_rect->win->Fl_Group::resize(xid_rect->X, xid_rect->Y, xid_rect->W, xid_rect->H);
    xid_rect->win->redraw();
  } else {
    xid_rect->win->Fl_Widget::resize(xid_rect->X, xid_rect->Y, xid_rect->W, xid_rect->H);
  }
  delete xid_rect;
}


static const struct wl_callback_listener surface_frame_listener = {
  .done = surface_frame_done,
};


void Fl_Wayland_Window_Driver::resize(int X, int Y, int W, int H) {
  static int depth = 0;
  struct wld_window *fl_win = fl_wl_xid(pWindow);
  if (fl_win && fl_win->kind == DECORATED && !xdg_toplevel()) {
    pWindow->wait_for_expose();
  }
  int is_a_move = (X != x() || Y != y());
  bool true_rescale = Fl_Window::is_a_rescale();
  float f = Fl::screen_scale(pWindow->screen_num());
  if (fl_win && fl_win->buffer) {
    int scale = wld_scale();
    int stride = cairo_format_stride_for_width(
                 Fl_Cairo_Graphics_Driver::cairo_format, int(W * f) * scale );
    size_t bsize = stride * int(H * f) * scale;
    true_rescale = (bsize != fl_win->buffer->draw_buffer.data_size);
  }
  int is_a_resize = (W != w() || H != h() || true_rescale);
  if (is_a_move) force_position(1);
  else if (!is_a_resize && !is_a_move) return;
  depth++;
  if (shown() && !(parent() || popup_window())) {
    X = Y = 0;
  }
  Fl_Window *parent = this->parent() ? pWindow->window() : NULL;
  struct wld_window *parent_xid = parent ? fl_wl_xid(parent) : NULL;
  xid_and_rect *xid_rect = NULL;
  if (parent_xid && parent_xid->frame_cb && wl_proxy_get_listener((struct wl_proxy*)parent_xid->frame_cb) == &surface_frame_listener) {
    xid_rect = (xid_and_rect*)wl_callback_get_user_data(parent_xid->frame_cb);
    if (xid_rect->win != pWindow) xid_rect = NULL;
  }
  // When moving or resizing a non-GL subwindow independently from its parent, this condition
  // delays application of X,Y,W,H values until the compositor signals
  // it's ready for a new frame using the frame callback mechanism.
  if ((parent && parent->damage()) || depth > 1 || pWindow->as_gl_window() || !parent_xid ||
      wait_for_expose_value || (parent_xid->frame_cb && !xid_rect)) {
    if (is_a_resize) {
      if (pWindow->parent()) {
        if (W < 1) W = 1;
        if (H < 1) H = 1;
      }
      pWindow->Fl_Group::resize(X,Y,W,H);
      //fprintf(stderr, "resize: win=%p to %dx%d\n", pWindow, W, H);
      if (shown()) {pWindow->redraw();}
    } else {
      x(X); y(Y);
      //fprintf(stderr, "move win=%p to %dx%d\n", pWindow, X, Y);
    }
  }

  if (shown()) {
    if (is_a_resize) {
      if (pWindow->as_overlay_window() && other_xid) {
        destroy_double_buffer();
      }
      if (fl_win->kind == DECORATED) { // a decorated window
        if (fl_win->buffer) {
          Fl_Wayland_Graphics_Driver::buffer_release(fl_win);
        }
        fl_win->configured_width = W;
        fl_win->configured_height = H;
        if (!in_handle_configure && xdg_toplevel()) {
          if (Fl_Window::is_a_rescale()) size_range();
          struct libdecor_state *state = libdecor_state_new(int(W * f), int(H * f));
          // necessary only if resize is initiated by prog
          libdecor_frame_commit(fl_win->frame, state, NULL);
          libdecor_state_free(state);
          if (libdecor_frame_is_floating(fl_win->frame)) {
            fl_win->floating_width = int(W*f);
            fl_win->floating_height = int(H*f);
          }
        }
      } else if (fl_win->kind == SUBWINDOW && fl_win->subsurface) { // a subwindow
        wl_subsurface_set_position(fl_win->subsurface, X * f, Y * f);
        if (!pWindow->as_gl_window()) Fl_Wayland_Graphics_Driver::buffer_release(fl_win);
        fl_win->configured_width = W;
        fl_win->configured_height = H;
      } else if (fl_win->xdg_surface) { // a window without border
        if (!pWindow->as_gl_window()) Fl_Wayland_Graphics_Driver::buffer_release(fl_win);
        fl_win->configured_width = W;
        fl_win->configured_height = H;
        W *= f; H *= f;
        if (!pWindow->fullscreen_active()) {
          xdg_toplevel_set_min_size(fl_win->xdg_toplevel, W, H);
          xdg_toplevel_set_max_size(fl_win->xdg_toplevel, W, H);
        }
        xdg_surface_set_window_geometry(fl_win->xdg_surface, 0, 0, W, H);
        //printf("xdg_surface_set_window_geometry: %dx%d\n",W, H);
      }
    } else {
      if (!in_handle_configure && xdg_toplevel()) {
        // Wayland doesn't seem to provide a reliable way for the app to set the
        // window position on screen. This is functional when the move is mouse-driven.
        Fl_Wayland_Screen_Driver *scr_driver = (Fl_Wayland_Screen_Driver*)Fl::screen_driver();
        if (Fl::e_state == FL_BUTTON1) {
          xdg_toplevel_move(xdg_toplevel(), scr_driver->seat->wl_seat, scr_driver->seat->serial);
          Fl::pushed(NULL);
          Fl::e_state = 0;
        }
      } else if (pWindow->as_gl_window() && fl_win->kind == SUBWINDOW && fl_win->subsurface) {
        wl_subsurface_set_position(fl_win->subsurface, X * f, Y * f);
      }
    }
  }

  if (fl_win && parent_xid) {
    if (pWindow->as_gl_window()) {
      if (fl_win->frame_cb) {
        wl_callback_destroy(fl_win->frame_cb);
        fl_win->frame_cb = NULL;
      }
      if (parent_xid->buffer) Fl_Wayland_Graphics_Driver::buffer_commit(parent_xid);
    } else {
      if (!(parent && parent->damage()) && !parent_xid->frame_cb) {
        // use the frame callback mechanism and memorize current X,Y,W,H values
        xid_rect = new xid_and_rect;
        xid_rect->xid = parent_xid;
        xid_rect->win = pWindow;
        parent_xid->frame_cb = wl_surface_frame(parent_xid->wl_surface);
        wl_callback_add_listener(parent_xid->frame_cb, &surface_frame_listener, xid_rect);
        xid_rect->X = X; xid_rect->Y = Y; xid_rect->W = W; xid_rect->H = H;
        xid_rect->need_resize = is_a_resize;
        wl_subsurface_set_position(fl_win->subsurface, X * f, Y * f);
        wl_surface_commit(parent_xid->wl_surface);
      } else if (!xid_rect) {
        wl_surface_commit(parent_xid->wl_surface);
      }
    }
    checkSubwindowFrame(); // make sure subwindow doesn't leak outside parent
  }
  depth--;
}


static void crect_intersect(cairo_rectangle_int_t *to, cairo_rectangle_int_t *with) {
  int x = fl_max(to->x, with->x);
  to->width = fl_min(to->x + to->width, with->x + with->width) - x;
  if (to->width < 0) to->width = 0;
  int y = fl_max(to->y, with->y);
  to->height = fl_min(to->y + to->height, with->y + with->height) - y;
  if (to->height < 0) to->height = 0;
  to->x = x;
  to->y = y;
}


static bool crect_equal(cairo_rectangle_int_t *to, cairo_rectangle_int_t *with) {
  return (to->x == with->x && to->y == with->y && to->width == with->width &&
          to->height == with->height);
}


void Fl_Wayland_Window_Driver::checkSubwindowFrame() {
  if (!pWindow->parent() || can_expand_outside_parent_) return;
  // make sure this subwindow doesn't leak out of its parent window
  Fl_Window *from = pWindow, *parent;
  cairo_rectangle_int_t full = {0, 0, pWindow->w(), pWindow->h()}; // full subwindow area
  cairo_rectangle_int_t srect = full; // will become new subwindow clip
  int fromx = 0, fromy = 0;
  while ((parent = from->window()) != NULL) { // loop over all parent windows
    fromx -= from->x(); // parent origin in subwindow's coordinates
    fromy -= from->y();
    cairo_rectangle_int_t prect = {fromx, fromy, parent->w(), parent->h()};
    crect_intersect(&srect, &prect); // area of subwindow inside its parent
    from = parent;
  }
  cairo_rectangle_int_t *r = subRect();
  // current subwindow clip
  cairo_rectangle_int_t current_clip = (r ? *r : full);
  if (!crect_equal(&srect, &current_clip)) { // if new clip differs from current clip
    if (crect_equal(&srect, &full)) r = NULL;
    else {
      r = &srect;
      if (r->width == 0 || r->height == 0) {
        r = NULL;
      }
    }
    subRect(r);
  }
}


void Fl_Wayland_Window_Driver::subRect(cairo_rectangle_int_t *r) {
  if (subRect_) delete subRect_;
  cairo_rectangle_int_t *r2 = NULL;
  if (r) {
    r2 = new cairo_rectangle_int_t;
    *r2 = *r;
  }
  subRect_ = r2;
}


void Fl_Wayland_Window_Driver::reposition_menu_window(int x, int y) {
  if (y == pWindow->y()) return;
  if (Fl_Wayland_Screen_Driver::compositor == Fl_Wayland_Screen_Driver::KWIN) {
    // The KWin compositor refuses to position a popup such that it extends above
    // the top of the screen. Therefore, instead of sliding the popup window
    // on the display, we slide the drawing inside the fixed popup via
    // member variable offset_y of the menuwindow class, and we redraw the popup
    // content. It's also useful to make such tall popup window transparent.
    *Fl_Window_Driver::menu_offset_y(pWindow) += (y - pWindow->y());
    struct wld_window *xid = fl_wl_xid(pWindow);
    wl_surface_set_opaque_region(xid->wl_surface, NULL);
    if (xid->buffer) memset(xid->buffer->draw_buffer.buffer, 0,
                            xid->buffer->draw_buffer.data_size);
    //printf("offset_y=%d\n", *Fl_Window_Driver::menu_offset_y(pWindow));
    this->y(y);
    pWindow->redraw();
    return;
  }
  struct wld_window * xid_menu = fl_wl_xid(pWindow);
//printf("reposition %dx%d[cur=%d] menu->state=%d\n", x, y, pWindow->y(), xid_menu->state);
  struct xdg_popup *old_popup = xid_menu->xdg_popup;
  struct xdg_surface *old_xdg = xid_menu->xdg_surface;
  struct wl_surface *old_surface = xid_menu->wl_surface;
  // menu_origin will be the parent of the processed menu window
  Fl_Window *menu_origin = Fl_Window_Driver::menu_title(pWindow);
  if (!menu_origin) menu_origin = Fl_Window_Driver::menu_leftorigin(pWindow);
  if (!menu_origin) menu_origin = Fl_Window_Driver::menu_parent();
  if (Fl_Window_Driver::menu_title(pWindow) && !Fl_Window_Driver::menu_bartitle(pWindow) &&
      !Fl_Window_Driver::menu_leftorigin(pWindow)) {
    // occurs with tall popup menu
    menu_origin = Fl_Window_Driver::menu_parent();
  }
  // create a new popup at position (x,y) and display it above the current one
  Fl_Wayland_Screen_Driver *scr_driver = (Fl_Wayland_Screen_Driver*)Fl::screen_driver();
  xid_menu->wl_surface = wl_compositor_create_surface(scr_driver->wl_compositor);
  wl_surface_add_listener(xid_menu->wl_surface, &surface_listener, xid_menu);
  xid_menu->xdg_surface = xdg_wm_base_get_xdg_surface(scr_driver->xdg_wm_base,
                                                      xid_menu->wl_surface);
  xdg_surface_add_listener(xid_menu->xdg_surface, &xdg_surface_listener, xid_menu);
  struct xdg_positioner *positioner = xdg_wm_base_create_positioner(scr_driver->xdg_wm_base);
  struct wld_window * parent_xid = fl_wl_xid(menu_origin);
  float f = Fl::screen_scale(Fl_Window_Driver::menu_parent()->screen_num());
  int popup_x = x * f, popup_y = y * f + xid_menu->state;
  if (menu_origin->menu_window() && driver(menu_origin)->popup_window()) {
    popup_x -= menu_origin->x() * f;
    popup_y -= menu_origin->y() * f;
  }
  if (popup_x >= menu_origin->w() * f) popup_x = menu_origin->w() * f - 1;
  if (parent_xid->kind == DECORATED) {
    libdecor_frame_translate_coordinate(parent_xid->frame, popup_x, popup_y,
                                        &popup_x, &popup_y);
  }
  xdg_positioner_set_anchor_rect(positioner, popup_x, 0, 1, 1);
  xdg_positioner_set_size(positioner, pWindow->w() * f , pWindow->h() * f );
  xdg_positioner_set_anchor(positioner, XDG_POSITIONER_ANCHOR_TOP_LEFT);
  xdg_positioner_set_gravity(positioner, XDG_POSITIONER_GRAVITY_BOTTOM_RIGHT);
  xdg_positioner_set_constraint_adjustment(positioner,
                                           XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_X);
  xdg_positioner_set_offset(positioner, 0, popup_y);
  xid_menu->xdg_popup = xdg_surface_get_popup(xid_menu->xdg_surface, parent_xid->xdg_surface,
                                              positioner);
  xdg_positioner_destroy(positioner);
  struct win_positioner *win_pos = new struct win_positioner;
  win_pos->window = xid_menu;
  win_pos->x = popup_x;
  win_pos->y = popup_y;
  win_pos->child_popup = NULL;
  xdg_popup_add_listener(xid_menu->xdg_popup, &popup_listener, win_pos);
  wl_surface_commit(xid_menu->wl_surface);
  wl_display_roundtrip(Fl_Wayland_Screen_Driver::wl_display); // necessary with sway
  // delete the previous popup
  struct win_positioner *old_win_pos =
    (struct win_positioner*)xdg_popup_get_user_data(old_popup);
  xdg_popup_destroy(old_popup);
  delete old_win_pos;
  xdg_surface_destroy(old_xdg);
  destroy_surface_caution_pointer_focus(old_surface, scr_driver->seat);
  this->y(y);
}


void Fl_Wayland_Window_Driver::menu_window_area(int &X, int &Y, int &W, int &H, int nscreen) {
  int HH;
  Fl_Window *parent = Fl_Window_Driver::menu_parent(&HH);
  if (parent) {
    if (pWindow->menu_window() && popup_window() && pWindow->h() > HH) {
      // tall menu: set top (Y) and bottom (Y+H) bounds relatively to reference window
      int ih = Fl_Window_Driver::menu_itemheight(pWindow);
      X = -50000;
      W = 1000000;
      H = HH - 2 * ih;
      Fl_Window *origin = Fl_Window_Driver::menu_leftorigin(pWindow);
      if (origin) { // has left parent
        int selected = fl_max(Fl_Window_Driver::menu_selected(origin), 0);
        Y = origin->y() + (selected + 0.5) * ih;
      } else if (!Fl_Window_Driver::menu_bartitle(pWindow)) { // tall menu button
        static int y_offset = 0;
        if (new_popup) {
          y_offset = pWindow->y()- ih;
          new_popup = false;
        }
        Y = 1.5 * ih + y_offset;
      } else { // has a menutitle
        Y = 1.5 * ih;
      }
    } else { // position the menu window by wayland constraints
      X = -50000;
      Y = -50000;
      W = 1000000;
      H = 1000000;
    }
    //printf("menu_window_area: %dx%d - %dx%d\n",X,Y,W,H);
  } else Fl_Window_Driver::menu_window_area(X, Y, W, H, nscreen);
}


int Fl_Wayland_Window_Driver::wld_scale() {
  Fl_X *flx = Fl_X::flx(pWindow);
  struct wld_window *xid = (flx ? (struct wld_window *)flx->xid : NULL);
  if (!xid || wl_list_empty(&xid->outputs)) {
    int scale = 1;
    Fl_Wayland_Screen_Driver *scr_driver = (Fl_Wayland_Screen_Driver*)Fl::screen_driver();
    Fl_Wayland_Screen_Driver::output *output;
    wl_list_for_each(output, &(scr_driver->outputs), link) {
      scale = fl_max(scale, output->wld_scale);
    }
    return scale;
  }
  struct surface_output *s_output;
  s_output = wl_container_of(xid->outputs.next, s_output, link);
  return s_output->output->wld_scale;
}


FL_EXPORT struct wl_surface *fl_wl_surface(struct wld_window *xid) {
  return xid->wl_surface;
}


cairo_t *fl_wl_gc() {
  return ((Fl_Cairo_Graphics_Driver*)fl_graphics_driver)->cr();
}


Fl_Window *fl_wl_find(struct wld_window *xid) {
  return Fl_Window_Driver::find((fl_uintptr_t)xid);
}


struct wld_window *fl_wl_xid(const Fl_Window *win) {
  return (struct wld_window *)Fl_Window_Driver::xid(win);
}


struct wl_compositor *fl_wl_compositor() {
  Fl_Wayland_Screen_Driver *screen_driver = (Fl_Wayland_Screen_Driver*)Fl::screen_driver();
  return screen_driver->wl_compositor;
}


int fl_wl_buffer_scale(Fl_Window *window) {
  return Fl_Wayland_Window_Driver::driver(window)->wld_scale();
}


Fl_Wayland_Plugin *Fl_Wayland_Window_Driver::gl_plugin() {
  static Fl_Wayland_Plugin *plugin = NULL;
  if (!plugin) {
    Fl_Plugin_Manager pm("wayland.fltk.org");
    plugin = (Fl_Wayland_Plugin*)pm.plugin("gl.wayland.fltk.org");
  }
  return plugin;
}


void Fl_Wayland_Window_Driver::maximize() {
  struct wld_window *xid = (struct wld_window *)Fl_X::flx(pWindow)->xid;
  if (xid->kind == DECORATED) libdecor_frame_set_maximized(xid->frame);
  else Fl_Window_Driver::maximize();
}


void Fl_Wayland_Window_Driver::un_maximize() {
  struct wld_window *xid = (struct wld_window *)Fl_X::flx(pWindow)->xid;
  if (xid->kind == DECORATED) libdecor_frame_unset_maximized(xid->frame);
  else Fl_Window_Driver::un_maximize();
}
