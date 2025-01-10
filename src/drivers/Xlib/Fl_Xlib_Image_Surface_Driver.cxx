//
// Draw-to-image code for the Fast Light Tool Kit (FLTK).
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

#include <FL/platform.H>
#include "Fl_Xlib_Image_Surface_Driver.H"
#include "../../Fl_Screen_Driver.H"
#include <stdlib.h>
#if FLTK_USE_CAIRO
#  include <cairo-xlib.h>
#  include "../Cairo/Fl_X11_Cairo_Graphics_Driver.H"
#else
#  include "Fl_Xlib_Graphics_Driver.H"
#endif // FLTK_USE_CAIRO



Fl_Xlib_Image_Surface_Driver::Fl_Xlib_Image_Surface_Driver(int w, int h, int high_res, Fl_Offscreen off) : Fl_Image_Surface_Driver(w, h, high_res, off) {
  float d = 1;
  if (!off) {
    fl_open_display();
    d =  Fl_Graphics_Driver::default_driver().scale();
    if (d != 1 && high_res) {
      w = int(w*d);
      h = int(h*d);
    }
    offscreen = (Fl_Offscreen)XCreatePixmap(fl_display, RootWindow(fl_display, fl_screen), w, h, fl_visual->depth);
  }
  shape_data_ = NULL;
#if FLTK_USE_CAIRO
  driver(new Fl_X11_Cairo_Graphics_Driver());
  cairo_surface_t *s = cairo_xlib_surface_create(fl_display, offscreen, fl_visual->visual, w, h);
  cairo_ = cairo_create(s);
  cairo_surface_destroy(s);
  cairo_save(cairo_);
  ((Fl_X11_Cairo_Graphics_Driver*)driver())->set_cairo(cairo_);
#else
  driver(new Fl_Xlib_Graphics_Driver());
#endif
  if (d != 1 && high_res) driver()->scale(d);
}

Fl_Xlib_Image_Surface_Driver::~Fl_Xlib_Image_Surface_Driver() {
#if FLTK_USE_CAIRO
  if (shape_data_) {
    cairo_surface_t *surf;
    cairo_pattern_get_surface(shape_data_->mask_pattern_, &surf);
    unsigned char *bits = cairo_image_surface_get_data(surf);
    cairo_pattern_destroy(shape_data_->mask_pattern_);
    delete[] bits;
    Pixmap p = cairo_xlib_surface_get_drawable(cairo_get_target(shape_data_->bg_cr));
    XFreePixmap(fl_display, p);
    cairo_destroy(shape_data_->bg_cr);
    free(shape_data_);
  }
  cairo_destroy(cairo_);
#else
  if (shape_data_) {
    XFreePixmap(fl_display, shape_data_->background);
    delete shape_data_->mask;
    free(shape_data_);
  }
#endif
  if (offscreen && !external_offscreen) XFreePixmap(fl_display, (Pixmap)offscreen);
  delete driver();
}

void Fl_Xlib_Image_Surface_Driver::set_current() {
  Fl_Surface_Device::set_current();
  pre_window = fl_window;
  fl_window = offscreen;
#if FLTK_USE_CAIRO
  Fl_Cairo_Graphics_Driver *dr = (Fl_Cairo_Graphics_Driver*)driver();
  if (!dr->cr()) dr->set_cairo(cairo_);
#endif
}

void Fl_Xlib_Image_Surface_Driver::translate(int x, int y) {
#if FLTK_USE_CAIRO
  cairo_save(cairo_);
  cairo_translate(cairo_, x, y);
#else
  ((Fl_Xlib_Graphics_Driver*)driver())->translate_all(x, y);
#endif
}

void Fl_Xlib_Image_Surface_Driver::untranslate() {
#if FLTK_USE_CAIRO
  cairo_restore(cairo_);
#else
  ((Fl_Xlib_Graphics_Driver*)driver())->untranslate_all();
#endif
}


Fl_RGB_Image* Fl_Xlib_Image_Surface_Driver::image()
{
  if (shape_data_) {
#if FLTK_USE_CAIRO
    // draw above the secondary offscreen the main offscreen masked by mask_pattern_
    cairo_t *c = ((Fl_Cairo_Graphics_Driver*)driver())->cr();
    cairo_pattern_t *paint_pattern = cairo_pattern_create_for_surface(cairo_get_target(c));
    cairo_set_source(shape_data_->bg_cr, paint_pattern);
    cairo_mask(shape_data_->bg_cr, shape_data_->mask_pattern_);
    cairo_pattern_destroy(paint_pattern);
    // copy secondary offscreen to the main offscreen
    cairo_pattern_t *pat = cairo_pattern_create_for_surface(cairo_get_target(shape_data_->bg_cr));
    cairo_scale(c, shape_data_->scale, shape_data_->scale);
    cairo_set_source(c, pat),
    cairo_paint(c);
    cairo_pattern_destroy(pat);
    // delete secondary offscreen
    cairo_surface_t *surf;
    cairo_pattern_get_surface(shape_data_->mask_pattern_, &surf);
    unsigned char *bits = cairo_image_surface_get_data(surf);
    cairo_pattern_destroy(shape_data_->mask_pattern_);
    delete[] bits;
    Pixmap p = cairo_xlib_surface_get_drawable(cairo_get_target(shape_data_->bg_cr));
    XFreePixmap(fl_display, p);
    cairo_destroy(shape_data_->bg_cr);
#else // !FLTK_USE_CAIRO
  // draw the main offscreen masked by shape_data_->mask above the background offscreen
    int w, h;
    printable_rect(&w, &h);
    Fl_RGB_Image *img_main = Fl::screen_driver()->read_win_rectangle(0, 0, w, h, 0);
    fl_window = shape_data_->background; // temporary change
    Fl_RGB_Image *img_background = Fl::screen_driver()->read_win_rectangle(0, 0, w, h, 0);
    fl_window = offscreen;
    Fl_Image_Surface_Driver::copy_with_mask(shape_data_->mask,
                                            (uchar*)img_background->array,
                                            (uchar*)img_main->array,
                                            3 * shape_data_->mask->w(), false);
    delete img_main;
  // copy background offscreen to main offscreen
    float s = driver()->scale();
    driver()->scale(1);
    fl_draw_image(img_background->array, 0, 0,
                  img_background->data_w(), img_background->data_h());
    driver()->scale(s);
    delete img_background;
  // delete background offscreen
    XFreePixmap(fl_display, shape_data_->background);
    delete shape_data_->mask;
#endif // FLTK_USE_CAIRO
    free(shape_data_);
    shape_data_ = NULL;
}
  Fl_RGB_Image *image = Fl::screen_driver()->read_win_rectangle(0, 0, width, height, 0);
  return image;
}


void Fl_Xlib_Image_Surface_Driver::end_current()
{
  fl_window = pre_window;
  Fl_Surface_Device::end_current();
}


#if FLTK_USE_CAIRO

void Fl_Xlib_Image_Surface_Driver::mask(const Fl_RGB_Image *mask) {
  bool using_copy = false;
  shape_data_ =  (struct shape_data_type*)calloc(1, sizeof(struct shape_data_type));
  int W, H;
  cairo_t *c = ((Fl_Cairo_Graphics_Driver*)driver())->cr();
  cairo_surface_t *c_surface = cairo_get_target(c);
  W = cairo_xlib_surface_get_width(c_surface);
  H = cairo_xlib_surface_get_height(c_surface);
  if (W != mask->data_w() || H != mask->data_h()) {
    Fl_RGB_Image *copy = (Fl_RGB_Image*)mask->copy(W, H);
    mask = copy;
    using_copy = true;
  }
  shape_data_->mask_pattern_ = Fl_Cairo_Graphics_Driver::calc_cairo_mask(mask);
  //duplicate current offscreen content to new cairo_t* shape_data_->bg_cr
  int width, height;
  printable_rect(&width, &height);
  Pixmap pxm = XCreatePixmap(fl_display, RootWindow(fl_display, fl_screen), W, H, fl_visual->depth);
  cairo_surface_t *background = cairo_xlib_surface_create(fl_display, pxm, fl_visual->visual, W, H);
  shape_data_->bg_cr = cairo_create(background);
  cairo_surface_destroy(background);
  cairo_surface_flush(c_surface);
  cairo_pattern_t *pat = cairo_pattern_create_for_surface(c_surface);
  cairo_set_source(shape_data_->bg_cr, pat),
  cairo_paint(shape_data_->bg_cr);
  cairo_pattern_destroy(pat);
  shape_data_->scale = double(width) / W;
  if (using_copy) delete mask;
}

#else

void Fl_Xlib_Image_Surface_Driver::mask(const Fl_RGB_Image *mask) {
  shape_data_ =  (struct shape_data_type*)calloc(1, sizeof(struct shape_data_type));
  // get dimensions
  int W, H;
  Fl::screen_driver()->offscreen_size(offscreen, W, H);
  // compute depth-1 mask
  shape_data_->mask = Fl_Image_Surface_Driver::RGB3_to_RGB1(mask, W, H);

  // duplicate current offscreen content to new, background offscreen
  shape_data_->background = XCreatePixmap(fl_display, RootWindow(fl_display, fl_screen), W, H, fl_visual->depth);
  driver()->restore_clip();
  XCopyArea(fl_display, (Pixmap)offscreen, shape_data_->background, (GC)driver()->gc(), 0, 0, W, H,  0, 0);
}

#endif // FLTK_USE_CAIRO
