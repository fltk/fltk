//
// Fl_Graphics_Driver class for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2023 by Bill Spitzak and others.
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

/** \file Fl_Graphics_Driver.cxx
\brief Implementation of class Fl_Graphics_Driver.
*/
#include <config.h> // for HAVE_GL
#include <FL/Fl_Graphics_Driver.H>
/** Points to the driver that currently receives all graphics requests */
FL_EXPORT Fl_Graphics_Driver *fl_graphics_driver;

/**
 \cond DriverDev
 \addtogroup DriverDeveloper
 \{
 */

#include "Fl_Screen_Driver.H"
#include <FL/Fl_Image_Surface.H>
#include <FL/math.h> // for fabs(), sqrt()
#include <FL/platform.H> // for fl_open_display()
#include <stdlib.h>


const Fl_Graphics_Driver::matrix Fl_Graphics_Driver::m0 = {1, 0, 0, 1, 0, 0};

/** Used by the Windows platform to print Fl_Pixmap objects. */
unsigned Fl_Graphics_Driver::need_pixmap_bg_color = 0;

extern unsigned fl_cmap[256]; // defined in fl_color.cxx

/** Constructor */
Fl_Graphics_Driver::Fl_Graphics_Driver()
{
  font_ = 0;
  size_ = 0;
  color_ = FL_BLACK;
  sptr=0; rstackptr=0;
  rstack[0] = NULL;
  fl_clip_state_number=0;
  m = m0;
  font_descriptor_ = NULL;
  scale_ = 1;
  p_size = 0;
  xpoint = NULL;
  what = NONE;
  n = 0;
}

/** Destructor */
Fl_Graphics_Driver::~Fl_Graphics_Driver() {
  if (xpoint) free(xpoint);
}


/** Return the graphics driver used when drawing to the platform's display */
Fl_Graphics_Driver &Fl_Graphics_Driver::default_driver()
{
  static Fl_Graphics_Driver *pMainDriver = Fl_Display_Device::display_device()->driver();
  return *pMainDriver;
}


/** see fl_text_extents() */
void Fl_Graphics_Driver::text_extents(const char*t, int nChars, int& dx, int& dy, int& w, int& h)
{
  w = (int)width(t, nChars);
  h = - height();
  dx = 0;
  dy = descent();
}

/** see fl_focus_rect() */
void Fl_Graphics_Driver::focus_rect(int x, int y, int w, int h)
{
  line_style(FL_DOT);
  rect(x, y, w, h);
  line_style(FL_SOLID);
}

/** see fl_copy_offscreen() */
void Fl_Graphics_Driver::copy_offscreen(int x, int y, int w, int h, Fl_Offscreen pixmap, int srcx, int srcy)
{
  // This platform-independent version can be used by any graphics driver,
  // noticeably the PostScript driver.
  // More efficient platform-specific implementations exist for other graphics drivers.
  Fl_Image_Surface *surface = NULL;
  int px_width = w, px_height = h;
  Fl::screen_driver()->offscreen_size(pixmap, px_width, px_height);
  Fl_Surface_Device *current = Fl_Surface_Device::surface();
  fl_begin_offscreen(pixmap); // does nothing if pixmap was not created by fl_create_offscreen()
  float s = 1;
  if (current == Fl_Surface_Device::surface()) {// pixmap was not created by fl_create_offscreen()
    // happens, e.g., when drawing images under Windows
    surface = new Fl_Image_Surface(px_width, px_height, 0, pixmap);
    Fl_Surface_Device::push_current(surface);
  }
  else { // pixmap was created by fl_create_offscreen()
    Fl_Image_Surface *imgs = (Fl_Image_Surface*)Fl_Surface_Device::surface();
    int sw, sh;
    imgs->printable_rect(&sw, &sh);
    s = px_width / float(sw);
  }
  int px = srcx, py = srcy, pw = w, ph = h;
  if (px < 0) {px = 0; pw += srcx; x -= srcx;}
  if (py < 0) {py = 0; ph += srcy; y -= srcy;}
  if (px + pw > px_width/s) {pw = int(px_width/s) - px;}
  if (py + ph > px_height/s) {ph = int(px_height/s) - py;}
  uchar *img = fl_read_image(NULL, px, py, pw, ph, 0);
  if (surface) {
    Fl_Surface_Device::pop_current();
    delete surface;
  } else fl_end_offscreen();
  if (img) {
    fl_draw_image(img, x, y, pw, ph, 3, 0);
    delete[] img;
  }
}


/** Sets the value of the fl_gc global variable when it changes */
void Fl_Graphics_Driver::global_gc()
{
  // nothing to do, reimplement in driver if needed
}


/** see Fl::set_color(Fl_Color, unsigned) */
void Fl_Graphics_Driver::set_color(Fl_Color i, unsigned c)
{
    fl_cmap[i] = c;
}


/** see Fl::free_color(Fl_Color, int) */
void Fl_Graphics_Driver::free_color(Fl_Color i, int overlay)
{
  // nothing to do, reimplement in driver if needed
}

/** Add a rectangle to an Fl_Region */
void Fl_Graphics_Driver::add_rectangle_to_region(Fl_Region r, int x, int y, int w, int h)
{
  // nothing to do, reimplement in driver if needed
}

/** Create a rectangular Fl_Region */
Fl_Region Fl_Graphics_Driver::XRectangleRegion(int x, int y, int w, int h)
{
  // nothing to do, reimplement in driver if needed
  return 0;
}

/** Delete an Fl_Region */
void Fl_Graphics_Driver::XDestroyRegion(Fl_Region r)
{
  // nothing to do, reimplement in driver if needed
}

/** Helper function for image drawing by platform-specific graphics drivers */
int Fl_Graphics_Driver::start_image(Fl_Image *img, int XP, int YP, int WP, int HP, int &cx, int &cy,
                     int &X, int &Y, int &W, int &H)
{
  // account for current clip region (faster on Irix):
  clip_box(XP,YP,WP,HP,X,Y,W,H);
  cx += X-XP; cy += Y-YP;
  // clip the box down to the size of image, quit if empty:
  if (cx < 0) {W += cx; X -= cx; cx = 0;}
  if (cx+W > img->w()) W = img->w()-cx;
  if (W <= 0) return 1;
  if (cy < 0) {H += cy; Y -= cy; cy = 0;}
  if (cy+H > img->h()) H = img->h()-cy;
  if (H <= 0) return 1;
  return 0;
}

/** Support function for image drawing */
void Fl_Graphics_Driver::uncache_pixmap(fl_uintptr_t p) {
}


void Fl_Graphics_Driver::set_current_() {
}

/** Support for Fl::set_font() */
unsigned Fl_Graphics_Driver::font_desc_size() {
  return (unsigned)sizeof(Fl_Fontdesc);
}

/** Converts \p width and \p height from FLTK units to drawing units.
 The conversion performed consists in multiplying \p width and \p height by
 scale() and in slightly modifying that to help support tiled images. */
void Fl_Graphics_Driver::cache_size(Fl_Image *img, int &width, int &height)
{
  // Image tiling may require to convert the floating value of width * scale() or
  // height * scale() to a larger integer value to avoid undrawn space between adjacent images.
  float s = scale(), fs = width * s;
  width = (fs - int(fs) < 0.001 ? int(fs) :
           int((width+1) * s));
  fs = height * s;
  height = (fs - int(fs) < 0.001 ? int(fs) :
           int((height+1) * s));
  cache_size_finalize(img, width, height);
}

void Fl_Graphics_Driver::cache_size_finalize(Fl_Image *img, int &width, int &height) {
  if (img) img->cache_size_(width, height);
}

/** Draws an Fl_Pixmap object using this graphics driver.
 Specifies a bounding box for the image, with the origin (upper left-hand corner) of
 the image offset by the cx and cy arguments.
 */
void Fl_Graphics_Driver::draw_pixmap(Fl_Pixmap *pxm, int XP, int YP, int WP, int HP, int cx, int cy) {
  int X, Y, W, H;
  if (Fl_Graphics_Driver::start_image(pxm, XP, YP, WP, HP, cx, cy, X, Y, W, H)) {
    return;
  }
  // to allow rescale at runtime
  int w2=pxm->w(), h2=pxm->h();
  cache_size(pxm, w2, h2); // after this, w2 x h2 is size of desired cached image
  int *pw, *ph;
  cache_w_h(pxm, pw, ph); // after this, *pw x *ph is current size of cached form of bitmap
  if (*id(pxm) && (*pw != w2 || *ph != h2)) {
    pxm->uncache();
  }
  if (!*id(pxm)) {
    if (pxm->data_w() != w2 || pxm->data_h() != h2) { // build a scaled id_ & mask_ for pxm
      Fl_Pixmap *pxm2 = (Fl_Pixmap*)pxm->copy(w2, h2);
      cache(pxm2);
      *id(pxm) = *id(pxm2);
      *id(pxm2) = 0;
      *pw = w2; *ph = h2; // memorize size of cached form of pixmap
      *mask(pxm) = *mask(pxm2);
      *mask(pxm2) = 0;
      delete pxm2;
    } else cache(pxm);
  }
  // draw pxm using its scaled id_ & pixmap_
  draw_fixed(pxm, X, Y, W, H, cx, cy);
}


/** Draws an Fl_Bitmap object using this graphics driver.
 Specifies a bounding box for the image, with the origin (upper left-hand corner) of
 the image offset by the cx and cy arguments.
 */
void Fl_Graphics_Driver::draw_bitmap(Fl_Bitmap *bm, int XP, int YP, int WP, int HP, int cx, int cy) {
  int X, Y, W, H;
  if (Fl_Graphics_Driver::start_image(bm, XP, YP, WP, HP, cx, cy, X, Y, W, H)) {
    return;
  }
  int w2 = bm->w(), h2 = bm->h();
  cache_size(bm, w2, h2); // after this, w2 x h2 is size of desired cached image
  int *pw, *ph;
  cache_w_h(bm, pw, ph); // after this, *pw x *ph is current size of cached form of bitmap
  if (*id(bm) && (*pw != w2 || *ph != h2)) {
    bm->uncache();
  }
  if (!*id(bm)) {
    if (bm->data_w() != w2 || bm->data_h() != h2) { // build a scaled id_ for bm
      Fl_Bitmap *bm2 = (Fl_Bitmap*)bm->copy(w2, h2);
      cache(bm2);
      *id(bm) = *id(bm2);
      *id(bm2) = 0;
      *pw = w2; *ph = h2; // memorize size of cached form of bitmap
      delete bm2;
    } else cache(bm);
  }
  // draw bm using its scaled id_
  draw_fixed(bm, X, Y, W, H, cx, cy);
}


/** Draws an Fl_RGB_Image object using this graphics driver.
 Specifies a bounding box for the image, with the origin (upper left-hand corner) of
 the image offset by the cx and cy arguments.
 */
void Fl_Graphics_Driver::draw_rgb(Fl_RGB_Image *img, int XP, int YP, int WP, int HP, int cx, int cy) {
  // Don't draw an empty image...
  if (!img->d() || !img->array) {
    Fl_Graphics_Driver::draw_empty(img, XP, YP);
    return;
  }
  if (start_image(img, XP, YP, WP, HP, cx, cy, XP, YP, WP, HP)) {
    return;
  }
  int need_scaled_drawing = ( fabs(img->w() - img->data_w()/scale())/img->w() > 0.05 ||
                            fabs(img->h() - img->data_h()/scale())/img->h() > 0.05 );
  // to allow rescale at runtime
  int w2, h2, *pw, *ph;
  if (need_scaled_drawing) {
    w2 = img->w(); h2 = img->h();
    cache_size(img, w2, h2);
  } else {
    w2 = img->data_w(); h2 = img->data_h();
  } // after this, w2 x h2 is desired cached image size
  cache_w_h(img, pw, ph); // after this, *pw x *ph is current size of cached image
  if (*id(img) && (w2 != *pw || h2 != *ph )) {
    img->uncache();
  }
  if (!*id(img) && need_scaled_drawing) { // build and draw a scaled id_ for img
    Fl_RGB_Scaling keep = Fl_Image::RGB_scaling();
    Fl_Image::RGB_scaling(Fl_Image::scaling_algorithm());
    Fl_RGB_Image *img2 = (Fl_RGB_Image*)img->copy(w2, h2);
    Fl_Image::RGB_scaling(keep);
    cache(img2);
    draw_fixed(img2, XP, YP, WP, HP, cx, cy);
    *id(img) = *id(img2);
    *mask(img) = *mask(img2);
    *id(img2) = 0;
    *mask(img2) = 0;
    *pw = w2;
    *ph = h2;
    delete img2;
  }
  else { // draw img using its scaled id_
    if (!*id(img)) cache(img);
    draw_fixed(img, XP, YP, WP, HP, cx, cy);
  }
}

/** Accessor to private member function of Fl_Image_Surface */
Fl_Offscreen Fl_Graphics_Driver::get_offscreen_and_delete_image_surface(Fl_Image_Surface* surface) {
  Fl_Offscreen off = surface->get_offscreen_before_delete_();
  delete surface;
  return off;
}

void Fl_Graphics_Driver::xyline(int x, int y, int x1) {
  line(x, y, x1, y);
}

void Fl_Graphics_Driver::xyline(int x, int y, int x1, int y2) {
  xyline(x, y, x1);
  yxline(x1, y, y2);
}

void Fl_Graphics_Driver::xyline(int x, int y, int x1, int y2, int x3) {
  xyline(x, y, x1);
  yxline(x1, y, y2);
  xyline(x1, y2, x3);
}

void Fl_Graphics_Driver::yxline(int x, int y, int y1) {
  line(x, y, x, y1);
}

void Fl_Graphics_Driver::yxline(int x, int y, int y1, int x2) {
  yxline(x, y, y1);
  xyline(x, y1, x2);
}

void Fl_Graphics_Driver::yxline(int x, int y, int y1, int x2, int y3) {
  yxline(x, y, y1);
  xyline(x, y1, x2);
  yxline(x2, y1, y3);
}

void Fl_Graphics_Driver::line(int x, int y, int x1, int y1, int x2, int y2) {
  line(x, y, x1, y1);
  line(x1, y1, x2, y2);
}

void Fl_Graphics_Driver::loop(int x0, int y0, int x1, int y1, int x2, int y2) {
  line(x0, y0, x1, y1);
  line(x1, y1, x2, y2);
  line(x2, y2, x0, y0);
}

void Fl_Graphics_Driver::loop(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) {
  line(x0, y0, x1, y1);
  line(x1, y1, x2, y2);
  line(x2, y2, x3, y3);
  line(x3, y3, x0, y0);
}

void Fl_Graphics_Driver::polygon(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) {
  polygon(x0, y0, x1, y1, x3, y3);
  polygon(x1, y1, x2, y2, x3, y3);
}

void Fl_Graphics_Driver::push_no_clip() {
  if (rstackptr < region_stack_max) rstack[++rstackptr] = 0;
  else Fl::warning("Fl_Graphics_Driver::push_no_clip: clip stack overflow!\n");
  restore_clip();
}

void Fl_Graphics_Driver::pop_clip() {
  if (rstackptr > 0) {
    Fl_Region oldr = rstack[rstackptr--];
    if (oldr) XDestroyRegion(oldr);
  } else Fl::warning("Fl_Graphics_Driver::pop_clip: clip stack underflow!\n");
  restore_clip();
}

/** Sets the current value of the scaling factor */
void Fl_Graphics_Driver::scale(float f) { scale_ = f; }

/** Return whether the graphics driver can do alpha blending */
char Fl_Graphics_Driver::can_do_alpha_blending() { return 0; }

void Fl_Graphics_Driver::draw_fixed(Fl_Pixmap *pxm,int XP, int YP, int WP, int HP, int cx, int cy) {}

void Fl_Graphics_Driver::draw_fixed(Fl_Bitmap *bm,int XP, int YP, int WP, int HP, int cx, int cy) {}

void Fl_Graphics_Driver::draw_fixed(Fl_RGB_Image *rgb,int XP, int YP, int WP, int HP, int cx, int cy) {}

void Fl_Graphics_Driver::make_unused_color_(unsigned char &r, unsigned char &g, unsigned char &b, int color_count, void **data) {}

/** Support function for Fl_Pixmap drawing */
void Fl_Graphics_Driver::cache(Fl_Pixmap *img) { }

/** Support function for Fl_Bitmap drawing */
void Fl_Graphics_Driver::cache(Fl_Bitmap *img) { }

/** Support function for Fl_RGB_Image drawing */
void Fl_Graphics_Driver::cache(Fl_RGB_Image *img) { }

/** Support function for Fl_RGB_Image drawing */
void Fl_Graphics_Driver::uncache(Fl_RGB_Image *img, fl_uintptr_t &id_, fl_uintptr_t &mask_) { }

/** see fl_draw_image(const uchar* buf, int X,int Y,int W,int H, int D, int L) */
void Fl_Graphics_Driver::draw_image(const uchar* buf, int X,int Y,int W,int H, int D, int L) {}

/** see fl_draw_image_mono(const uchar* buf, int X,int Y,int W,int H, int D, int L) */
void Fl_Graphics_Driver::draw_image_mono(const uchar* buf, int X,int Y,int W,int H, int D, int L) {}

/** see fl_draw_image(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D) */
void Fl_Graphics_Driver::draw_image(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D) {}

/** see fl_draw_image_mono(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D) */
void Fl_Graphics_Driver::draw_image_mono(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D) {}

/** Support function for image drawing */
void Fl_Graphics_Driver::delete_bitmask(fl_uintptr_t /*bm*/) {}

/** see fl_point() */
void Fl_Graphics_Driver::point(int x, int y) {}

/** see fl_rect() */
void Fl_Graphics_Driver::rect(int x, int y, int w, int h) {}

/** see fl_rectf() */
void Fl_Graphics_Driver::rectf(int x, int y, int w, int h) {}

void Fl_Graphics_Driver::_rbox(int fill, int x, int y, int w, int h, int r) {
  static double lut[] = { 0.0, 0.07612, 0.29289, 0.61732, 1.0};
  if (r == 5) r = 4;  // use only even sizes for small corners (STR #2943)
  if (r == 7) r = 8;  // note: 8 is better than 6 (really)
  double xd = x, yd = y, rd = (x+w-1), bd = (y+h-1);
  double rr = r;
  if (fill) begin_polygon(); else begin_loop();
  // top left
  transformed_vertex(xd+lut[0]*rr, yd+lut[4]*rr);
  transformed_vertex(xd+lut[1]*rr, yd+lut[3]*rr);
  transformed_vertex(xd+lut[2]*rr, yd+lut[2]*rr);
  transformed_vertex(xd+lut[3]*rr, yd+lut[1]*rr);
  transformed_vertex(xd+lut[4]*rr, yd+lut[0]*rr);
  // top right
  transformed_vertex(rd-lut[4]*rr, yd+lut[0]*rr);
  transformed_vertex(rd-lut[3]*rr, yd+lut[1]*rr);
  transformed_vertex(rd-lut[2]*rr, yd+lut[2]*rr);
  transformed_vertex(rd-lut[1]*rr, yd+lut[3]*rr);
  transformed_vertex(rd-lut[0]*rr, yd+lut[4]*rr);
  // bottom right
  transformed_vertex(rd-lut[0]*rr, bd-lut[4]*rr);
  transformed_vertex(rd-lut[1]*rr, bd-lut[3]*rr);
  transformed_vertex(rd-lut[2]*rr, bd-lut[2]*rr);
  transformed_vertex(rd-lut[3]*rr, bd-lut[1]*rr);
  transformed_vertex(rd-lut[4]*rr, bd-lut[0]*rr);
  // bottom left
  transformed_vertex(xd+lut[4]*rr, bd-lut[0]*rr);
  transformed_vertex(xd+lut[3]*rr, bd-lut[1]*rr);
  transformed_vertex(xd+lut[2]*rr, bd-lut[2]*rr);
  transformed_vertex(xd+lut[1]*rr, bd-lut[3]*rr);
  transformed_vertex(xd+lut[0]*rr, bd-lut[4]*rr);
  if (fill) fl_end_polygon(); else fl_end_loop();
}

/** see fl_rounded_rect() */
void Fl_Graphics_Driver::rounded_rect(int x, int y, int w, int h, int r) {
  _rbox(0, x, y, w, h, r);
}

/** see fl_rounded_rectf() */
void Fl_Graphics_Driver::rounded_rectf(int x, int y, int w, int h, int r) {
  _rbox(1, x, y, w, h, r);
}

void Fl_Graphics_Driver::colored_rectf(int x, int y, int w, int h, uchar r, uchar g, uchar b) {
  color(r, g, b);
  rectf(x, y, w, h);
}

/** see fl_line(int, int, int, int) */
void Fl_Graphics_Driver::line(int x, int y, int x1, int y1) {}

/** see fl_polygon(int, int, int, int, int, int) */
void Fl_Graphics_Driver::polygon(int x0, int y0, int x1, int y1, int x2, int y2) {}

/** see fl_push_clip() */
void Fl_Graphics_Driver::push_clip(int x, int y, int w, int h) {}

/**
  Default graphics driver implementation of fl_clip_box().

  This default implementation is sufficient for a graphics driver that does not
  support clipping. Drivers that support clipping must override this virtual method.

  It returns
  - in (X, Y, W, H) the same values as given in (x, y, w, h) respectively
  - 0 (zero) as the function return value
  which means that \b nothing was clipped.

  \returns 0 (zero) - nothing was clipped

  \see fl_clip_box()
*/
int Fl_Graphics_Driver::clip_box(int x, int y, int w, int h, int &X, int &Y, int &W, int &H) {
  X = x;
  Y = y;
  W = w;
  H = h;
  return 0;
}

/** see fl_not_clipped() */
int Fl_Graphics_Driver::not_clipped(int x, int y, int w, int h) {return 1;}

/** see fl_begin_complex_polygon() */
void Fl_Graphics_Driver::begin_complex_polygon() {
  begin_polygon();
  gap_ = 0;
}

/** see fl_transformed_vertex() */
void Fl_Graphics_Driver::transformed_vertex(double xf, double yf) {
  transformed_vertex0(float(xf), float(yf));
}

/** see fl_vertex() */
void Fl_Graphics_Driver::vertex(double x, double y) {
  transformed_vertex(x*m.a + y*m.c + m.x, x*m.b + y*m.d + m.y);
}

/** see fl_end_points() */
void Fl_Graphics_Driver::end_points() {}

/** see fl_end_line() */
void Fl_Graphics_Driver::end_line() {}

void Fl_Graphics_Driver::fixloop() {  // remove equal points from closed path
  while (n>2 && xpoint[n-1].x == xpoint[0].x && xpoint[n-1].y == xpoint[0].y) n--;
}

/** see fl_end_loop() */
void Fl_Graphics_Driver::end_loop() {
  fixloop();
  if (n>2) transformed_vertex(xpoint[0].x, xpoint[0].y);
  end_line();
}

/** see fl_end_polygon() */
void Fl_Graphics_Driver::end_polygon() {}

/** see fl_end_complex_polygon() */
void Fl_Graphics_Driver::end_complex_polygon() {}

/** see fl_gap() */
void Fl_Graphics_Driver::gap() {
  while (n>gap_+2 && xpoint[n-1].x == xpoint[gap_].x && xpoint[n-1].y == xpoint[gap_].y) n--;
  if (n > gap_+2) {
    transformed_vertex(xpoint[gap_].x, xpoint[gap_].y);
    gap_ = n;
  } else {
    n = gap_;
  }
}

/** see fl_circle() */
void Fl_Graphics_Driver::circle(double x, double y, double r) {}

/** see fl_arc(int x, int y, int w, int h, double a1, double a2) */
void Fl_Graphics_Driver::arc(int x, int y, int w, int h, double a1, double a2) {}

/** see fl_pie() */
void Fl_Graphics_Driver::pie(int x, int y, int w, int h, double a1, double a2) {}

/** see fl_draw_circle() */
void Fl_Graphics_Driver::draw_circle(int x, int y, int d, Fl_Color c) {
  Fl_Color current_c = color();
  if (c != current_c) color(c);
  pie(x, y, d, d, 0., 360.);
  if (c != current_c) color(current_c);
}

/** see fl_line_style() */
void Fl_Graphics_Driver::line_style(int style, int width, char* dashes) {}

/** see fl_color(Fl_Color) */
void Fl_Graphics_Driver::color(Fl_Color c) { color_ = c; }

/** see fl_color(void) */
Fl_Color Fl_Graphics_Driver::color() { return color_; }

/** see fl_color(uchar, uchar, uchar) */
void Fl_Graphics_Driver::color(uchar r, uchar g, uchar b) {}

/** see fl_draw(const char *str, int n, int x, int y) */
void Fl_Graphics_Driver::draw(const char *str, int nChars, int x, int y) {}

/** Draw the first \p n bytes of the string \p str starting at position \p x , \p y */
void Fl_Graphics_Driver::draw(const char *str, int nChars, float x, float y) {
  draw(str, nChars, (int)(x+0.5), (int)(y+0.5));
}

/** see fl_draw(int angle, const char *str, int n, int x, int y) */
void Fl_Graphics_Driver::draw(int angle, const char *str, int nChars, int x, int y) {
  draw(str, nChars, x, y);
}

/** see fl_rtl_draw(const char *str, int n, int x, int y) */
void Fl_Graphics_Driver::rtl_draw(const char *str, int nChars, int x, int y) {
  draw(str, nChars, x, y);
}

/** Returns non-zero if the graphics driver possesses the \p feature */
int Fl_Graphics_Driver::has_feature(driver_feature feature) { return 0; }

/** see fl_font(Fl_Font, Fl_Fontsize) */
void Fl_Graphics_Driver::font(Fl_Font face, Fl_Fontsize fsize) {font_ = face; size_ = fsize;}

/** see fl_font(void) */
Fl_Font Fl_Graphics_Driver::font() {return font_; }

/** Return the current font size */
Fl_Fontsize Fl_Graphics_Driver::size() {return size_; }

/** Compute the width of the first \p n bytes of the string \p str if drawn with current font */
double Fl_Graphics_Driver::width(const char *str, int nChars) { return 0; }

/** Compute the width of Unicode character \p c if drawn with current font */
double Fl_Graphics_Driver::width(unsigned int c) {
  char buf[4];
  return width(buf, fl_utf8encode (c, buf));
}

/** Return the current line height */
int Fl_Graphics_Driver::height() { return size(); }

/** Return the current line descent */
int Fl_Graphics_Driver::descent() { return 0; }

/** Sets the value of the driver-specific graphics context. */
void Fl_Graphics_Driver::gc(void*) {}

/** Returns the driver-specific graphics context, of NULL if there's none. */
void *Fl_Graphics_Driver::gc(void) {return NULL;}

/** Support for pixmap drawing */
uchar **Fl_Graphics_Driver::mask_bitmap() { return 0; }

/** Support for PostScript drawing */
float Fl_Graphics_Driver::scale_font_for_PostScript(Fl_Font_Descriptor *desc, int s) {
  return float(s);
}

/** Support for PostScript drawing */
float Fl_Graphics_Driver::scale_bitmap_for_PostScript() { return 2; }

/** Support for Fl::get_font_name() */
const char* Fl_Graphics_Driver::get_font_name(Fl_Font fnum, int* ap) {return NULL;}

/** Support for Fl::get_font_sizes() */
int Fl_Graphics_Driver::get_font_sizes(Fl_Font fnum, int*& sizep) {return 0;}

/** Support for Fl::set_fonts() */
Fl_Font Fl_Graphics_Driver::set_fonts(const char *name) {return 0;}

/** Some platforms may need to implement this to support fonts */
Fl_Fontdesc* Fl_Graphics_Driver::calc_fl_fonts(void) {return NULL;}

/** Support for Fl::get_font() */
const char *Fl_Graphics_Driver::font_name(int num) {return NULL;}

/** Support for Fl::set_font() */
void Fl_Graphics_Driver::font_name(int num, const char *name) {}

/** Support function for fl_overlay_rect() and scaled GUI.*/
void Fl_Graphics_Driver::overlay_rect(int x, int y, int w , int h) {
  loop(x, y, x+w-1, y, x+w-1, y+h-1, x, y+h-1);
}

float Fl_Graphics_Driver::override_scale() { return scale();}

void Fl_Graphics_Driver::restore_scale(float) { }

void Fl_Graphics_Driver::transformed_vertex0(float x, float y) {
  if (!n || x != xpoint[n-1].x || y != xpoint[n-1].y) {
    if (n >= p_size) {
      p_size = xpoint ? 2*p_size : 16;
      xpoint = (XPOINT*)realloc((void*)xpoint, p_size*sizeof(*xpoint));
    }
    xpoint[n].x = x;
    xpoint[n].y = y;
    n++;
  }
}

void Fl_Graphics_Driver::antialias(int state) {}

int Fl_Graphics_Driver::antialias() {
  return 0;
}

/**
 \}
 \endcond
 */

#ifndef FL_DOXYGEN

Fl_Font_Descriptor::Fl_Font_Descriptor(const char* name, Fl_Fontsize Size) {
  next = 0;
#  if HAVE_GL
  listbase = 0;
#  endif
  // OpenGL needs those for its font handling
  size = Size;
}

Fl_Scalable_Graphics_Driver::Fl_Scalable_Graphics_Driver() : Fl_Graphics_Driver() {
  line_width_ = 0;
  fontsize_ = -1;
}

void Fl_Scalable_Graphics_Driver::rect(int x, int y, int w, int h)
{
  if (w > 0 && h > 0) {
    int s = (int)scale();
    int d = s / 2;
    rect_unscaled(this->floor(x) + d, this->floor(y) + d,
                  this->floor(x + w) - this->floor(x) - s,
                  this->floor(y + h) - this->floor(y) - s);
  }
}

// This function aims to compute accurately int(x * s) in
// presence of rounding errors existing with floating point numbers
// and that sometimes differ between 32 and 64 bits.
int Fl_Scalable_Graphics_Driver::floor(int x, float s) {
  if (s == 1) return x;
  int retval = int(abs(x) * s + 0.001f);
  return (x >= 0 ? retval : -retval);
}

void Fl_Scalable_Graphics_Driver::rectf(int x, int y, int w, int h)
{
  if (w <= 0 || h <= 0) return;
  rectf_unscaled(this->floor(x), this->floor(y),
                 this->floor(x + w) - this->floor(x), this->floor(y + h) - this->floor(y));
}

void Fl_Scalable_Graphics_Driver::point(int x, int y) {
  rectf(x, y, 1, 1);
}

void Fl_Scalable_Graphics_Driver::line(int x, int y, int x1, int y1) {
  if (y == y1) xyline(x, y, x1);
  else if (x == x1) yxline(x, y, y1);
  else line_unscaled(this->floor(x), this->floor(y), this->floor(x1), this->floor(y1));
}

void Fl_Scalable_Graphics_Driver::line(int x, int y, int x1, int y1, int x2, int y2) {
  line_unscaled(this->floor(x), this->floor(y), this->floor(x1), this->floor(y1), this->floor(x2), this->floor(y2));
}

void Fl_Scalable_Graphics_Driver::xyline(int x, int y, int x1) {
  if (y < 0) return;
  float s = scale(); int s_int = int(s);
  int xx = (x < x1 ? x : x1);
  int xx1 = (x < x1 ? x1 : x);
  if (s != s_int && line_width_ <= s_int) {
    int lwidth = this->floor((y+1)) - this->floor(y);
    bool need_change_width = (lwidth != s_int);
    void *data = NULL;
    if (need_change_width) data = change_pen_width(lwidth);
    xyline_unscaled(this->floor(xx), this->floor(y) + int(lwidth/2.f), this->floor(xx1+1)-1);
    if (need_change_width) reset_pen_width(data);
  } else {
    y = this->floor(y);
    if (line_width_ <= s_int) y += int(s/2.f);
    else y += s_int/2;
    xyline_unscaled(this->floor(xx), y, this->floor(xx1+1) - 1);
  }
}

void Fl_Scalable_Graphics_Driver::yxline(int x, int y, int y1) {
  if (x < 0) return;
  float s = scale();  int s_int = int(s);
  int yy = (y < y1 ? y : y1);
  int yy1 = (y < y1 ? y1 : y);
  if (s != s_int && line_width_ <= s_int) {
    int lwidth = (this->floor((x+1)) - this->floor(x));
    bool need_change_width = (lwidth != s_int);
    void *data = NULL;
    if (need_change_width) data = change_pen_width(lwidth);
    yxline_unscaled(this->floor(x) + int(lwidth/2.f), this->floor(yy), this->floor(yy1+1) - 1);
    if (need_change_width) reset_pen_width(data);
  } else {
    x = this->floor(x);
    if (line_width_ <= s_int) x += int(s/2.f);
    else x += s_int/2;
    yxline_unscaled(x, this->floor(yy), this->floor(yy1+1) - 1);
  }
}

void *Fl_Scalable_Graphics_Driver::change_pen_width(int lwidth) {return NULL;}

void Fl_Scalable_Graphics_Driver::reset_pen_width(void *data){}

void Fl_Scalable_Graphics_Driver::loop(int x0, int y0, int x1, int y1, int x2, int y2) {
  loop_unscaled(floor(x0), floor(y0), floor(x1), floor(y1), floor(x2), floor(y2));
}

void Fl_Scalable_Graphics_Driver::loop(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) {
  int X, Y, W, H;
  if (x0 == x3 && x1 == x2 && y0 == y1 && y3 == y2) { // rectangular loop
    X = x0 > x1 ? x1 : x0;
    Y = y0 > y3 ? y3 : y0;
    W = abs(x0 - x1);
    H = abs(y0 - y3);
    rect(X, Y, W + 1, H + 1);
  } else if (x0 == x1 && y1 == y2 && x2 == x3 && y3 == y0) { // rectangular loop also
    X = x0 > x3 ? x3 : x0;
    Y = y0 > y1 ? y1 : y0;
    W = abs(x0 - x3);
    H = abs(y0 - y1);
    rect(X, Y, W + 1, H + 1);
  } else {
    loop_unscaled(floor(x0), floor(y0), floor(x1), floor(y1), floor(x2), floor(y2), floor(x3), floor(y3));
  }
}

void Fl_Scalable_Graphics_Driver::polygon(int x0, int y0, int x1, int y1, int x2, int y2) {
  polygon_unscaled(floor(x0), floor(y0), floor(x1), floor(y1), floor(x2), floor(y2));
}

void Fl_Scalable_Graphics_Driver::polygon(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) {
  polygon_unscaled(floor(x0), floor(y0), floor(x1), floor(y1), floor(x2), floor(y2), floor(x3), floor(y3));
}

void Fl_Scalable_Graphics_Driver::circle(double x, double y, double r) {
  double xt = transform_x(x,y);
  double yt = transform_y(x,y);
  double rx = r * (m.c ? sqrt(m.a*m.a+m.c*m.c) : fabs(m.a));
  double ry = r * (m.b ? sqrt(m.b*m.b+m.d*m.d) : fabs(m.d));
  ellipse_unscaled(xt*scale(), yt*scale(), rx*scale(), ry*scale());
}

void Fl_Scalable_Graphics_Driver::font(Fl_Font face, Fl_Fontsize size) {
  if (!font_descriptor()) fl_open_display(); // to catch the correct initial value of scale_
  font_unscaled(face, Fl_Fontsize(size * scale()));
  fontsize_ = size;
}

Fl_Font Fl_Scalable_Graphics_Driver::font() {
  return Fl_Graphics_Driver::font();
}

double Fl_Scalable_Graphics_Driver::width(const char *str, int n) {
  return width_unscaled(str, n)/scale();
}

double Fl_Scalable_Graphics_Driver::width(unsigned int c) {
  return width_unscaled(c)/scale();
}

Fl_Fontsize Fl_Scalable_Graphics_Driver::size() {
  if (!font_descriptor() ) return -1;
  return fontsize_;
}

void Fl_Scalable_Graphics_Driver::text_extents(const char *str, int n, int &dx, int &dy, int &w, int &h) {
  text_extents_unscaled(str, n, dx, dy, w, h);
  dx = int(dx / scale());
  dy = int(dy / scale());
  w = int(w / scale());
  h = int(h / scale());
}

int Fl_Scalable_Graphics_Driver::height() {
  return int(height_unscaled()/scale());
}

int Fl_Scalable_Graphics_Driver::descent() {
  return int(descent_unscaled()/scale());
}

void Fl_Scalable_Graphics_Driver::draw(const char *str, int n, int x, int y) {
  if (!size_ || !font_descriptor()) font(FL_HELVETICA, FL_NORMAL_SIZE);
  Fl_Region r2 = scale_clip(scale());
  draw_unscaled(str, n, floor(x), floor(y));
  unscale_clip(r2);
}

void Fl_Scalable_Graphics_Driver::draw(const char *str, int n, float x, float y) {
  Fl_Graphics_Driver::draw(str, n, x, y);
}

void Fl_Scalable_Graphics_Driver::draw(int angle, const char *str, int n, int x, int y) {
  if (!size_ || !font_descriptor()) font(FL_HELVETICA, FL_NORMAL_SIZE);
  Fl_Region r2 = scale_clip(scale());
  draw_unscaled(angle, str, n, floor(x), floor(y));
  unscale_clip(r2);
}

void Fl_Scalable_Graphics_Driver::rtl_draw(const char* str, int n, int x, int y) {
  rtl_draw_unscaled(str, n, int(x * scale()), int(y * scale()));
}

void Fl_Scalable_Graphics_Driver::arc(int x, int y, int w, int h, double a1, double a2) {
  float s = scale();
  int xx = floor(x) + int((s-1)/2);
  int yy = floor(y) + int((s-1)/2);
  w = floor(x+w) - xx - 1 + line_width_/2 - int(s-1);
  h = floor(y+h) - yy - 1 + line_width_/2 - int(s-1);
  arc_unscaled(xx, yy, w, h, a1, a2);
}

void Fl_Scalable_Graphics_Driver::arc(double x, double y, double r, double start, double end) {
  Fl_Graphics_Driver::arc(x, y, r, start, end);
}

void Fl_Scalable_Graphics_Driver::pie(int x,int y,int w,int h,double a1,double a2) {
  int xx = floor(x) - 1;
  int yy = floor(y) - 1;
  w = floor(x+w) - xx;
  h = floor(y+h) - yy;
  pie_unscaled(xx, yy, w, h, a1, a2);
}


void Fl_Scalable_Graphics_Driver::draw_circle(int x0, int y0, int d, Fl_Color c) {
  Fl_Color saved = color();
  color(c);

  // make circles nice on scaled display
  float s = scale();
  int scaled_d = (s > 1.0) ? (int)(d * s) : d;

  // draw the circle
  switch (scaled_d) {
      // Larger circles draw fine...
    default:
      pie(x0, y0, d, d, 0.0, 360.0);
      break;

      // Small circles don't draw well on many systems...
    case 6:
      rectf(x0 + 2, y0, d - 4, d);
      rectf(x0 + 1, y0 + 1, d - 2, d - 2);
      rectf(x0, y0 + 2, d, d - 4);
      break;

    case 5:
    case 4:
    case 3:
      rectf(x0 + 1, y0, d - 2, d);
      rectf(x0, y0 + 1, d, d - 2);
      break;

    case 2:
    case 1:
      rectf(x0, y0, d, d);
      break;
  }
  color(saved);
}


void Fl_Scalable_Graphics_Driver::line_style(int style, int width, char* dashes) {
  if (width == 0) line_width_ = int(scale() < 2 ? 0 : scale());
  else line_width_ = int(width>0 ? width*scale() : -width*scale());
  line_style_unscaled(style, line_width_, dashes);
}

/* read the image data from a pointer or with a callback, scale it, and draw it */
void Fl_Scalable_Graphics_Driver::draw_image_rescale(void *buf, Fl_Draw_Image_Cb cb,
                                                     int X, int Y, int W, int H, int D, int L, bool mono) {
  int aD = abs(D);
  if (L == 0) L = W*aD;
  int depth = mono ? (aD%2==0?2:1) : aD;
  uchar *tmp_buf = new uchar[W*H*depth];
  if (cb) {
    for (int i = 0; i < H; i++) {
      cb(buf, 0, i, W, tmp_buf + i * W * depth);
    }
  } else {
    uchar *q, *p = tmp_buf;
    for (int i = 0; i < H; i++) {
      q = (uchar*)buf + i * L;
      for (int j = 0; j < W; j++) {
        memcpy(p, q, depth);
        p += depth; q += D;
      }
    }
  }
  Fl_RGB_Image *rgb = new Fl_RGB_Image(tmp_buf, W, H, depth);
  rgb->alloc_array = 1;
  Fl_RGB_Scaling keep = Fl_Image::RGB_scaling();
  Fl_Image::RGB_scaling(Fl_Image::scaling_algorithm());
  Fl_RGB_Image *scaled_rgb = (Fl_RGB_Image*)rgb->copy(floor(X+W)-floor(X), floor(Y+H)-floor(Y));
  Fl_Image::RGB_scaling(keep);
  delete rgb;
  if (scaled_rgb) {
    Fl_Region r2 = scale_clip(scale());
    draw_image_unscaled(scaled_rgb->array, floor(X), floor(Y), scaled_rgb->w(), scaled_rgb->h(), depth);
    unscale_clip(r2);
    delete scaled_rgb;
  }
}

void Fl_Scalable_Graphics_Driver::draw_image(const uchar* buf, int X,int Y,int W,int H, int D, int L) {
  if (scale() == 1) {
    draw_image_unscaled(buf, X,Y,W,H,D,L);
  } else {
    draw_image_rescale((void*)buf, NULL, X, Y, W, H, D, L, false);
  }
}

void Fl_Scalable_Graphics_Driver::draw_image(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D) {
  if (scale() == 1) {
    draw_image_unscaled(cb, data, X,Y,W,H,D);
  } else {
    draw_image_rescale(data, cb, X, Y, W, H, D, 0, false);
  }
}

void Fl_Scalable_Graphics_Driver::draw_image_mono(const uchar* buf, int X,int Y,int W,int H, int D, int L) {
  if (scale() == 1) {
    draw_image_mono_unscaled(buf, X,Y,W,H,D,L);
  } else {
    draw_image_rescale((void*)buf, NULL, X, Y, W, H, D, L, true);
  }
}

void Fl_Scalable_Graphics_Driver::draw_image_mono(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D) {
  if (scale() == 1) {
    draw_image_mono_unscaled(cb, data, X,Y,W,H,D);
  } else {
    draw_image_rescale(data, cb, X, Y, W, H, D, 0, true);
  }
}

void Fl_Scalable_Graphics_Driver::transformed_vertex(double xf, double yf) {
  transformed_vertex0(float(xf * scale()), float(yf * scale()));
}

void Fl_Scalable_Graphics_Driver::vertex(double x,double y) {
  transformed_vertex0(float((x*m.a + y*m.c + m.x) * scale()), float((x*m.b + y*m.d + m.y) * scale()));
}

void Fl_Scalable_Graphics_Driver::unscale_clip(Fl_Region r) {
  if (r) {
    if (rstack[rstackptr]) XDestroyRegion(rstack[rstackptr]);
    rstack[rstackptr] = r;
  }
}

Fl_Region Fl_Scalable_Graphics_Driver::scale_clip(float f) { return 0; }

void Fl_Scalable_Graphics_Driver::point_unscaled(float x, float y) {}

void Fl_Scalable_Graphics_Driver::rect_unscaled(int x, int y, int w, int h) {}

void Fl_Scalable_Graphics_Driver::rectf_unscaled(int x, int y, int w, int h) {}

void Fl_Scalable_Graphics_Driver::line_unscaled(int x, int y, int x1, int y1) {}

void Fl_Scalable_Graphics_Driver::line_unscaled(int x, int y, int x1, int y1, int x2, int y2) {}

void Fl_Scalable_Graphics_Driver::xyline_unscaled(int x, int y, int x1) {}

void Fl_Scalable_Graphics_Driver::yxline_unscaled(int x, int y, int y1) {}

void Fl_Scalable_Graphics_Driver::loop_unscaled(int x0, int y0, int x1, int y1, int x2, int y2) {}

void Fl_Scalable_Graphics_Driver::loop_unscaled(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) {}

void Fl_Scalable_Graphics_Driver::polygon_unscaled(int x0, int y0, int x1, int y1, int x2, int y2) {}

void Fl_Scalable_Graphics_Driver::polygon_unscaled(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) {}

void Fl_Scalable_Graphics_Driver::ellipse_unscaled(double xt, double yt, double rx, double ry) {}

void Fl_Scalable_Graphics_Driver::font_unscaled(Fl_Font face, Fl_Fontsize size) {}

double Fl_Scalable_Graphics_Driver::width_unscaled(const char *str, int n) { return 0.0; }

double Fl_Scalable_Graphics_Driver::width_unscaled(unsigned int c) { return 0.0; }

Fl_Fontsize Fl_Scalable_Graphics_Driver::size_unscaled() { return 0; }

void Fl_Scalable_Graphics_Driver::text_extents_unscaled(const char *str, int n, int &dx, int &dy, int &w, int &h) {}

int Fl_Scalable_Graphics_Driver::height_unscaled() { return 0; }

int Fl_Scalable_Graphics_Driver::descent_unscaled() { return 0; }

void Fl_Scalable_Graphics_Driver::draw_unscaled(const char *str, int n, int x, int y) {}

void Fl_Scalable_Graphics_Driver::draw_unscaled(int angle, const char *str, int n, int x, int y) {}

void Fl_Scalable_Graphics_Driver::rtl_draw_unscaled(const char* str, int n, int x, int y) {}

void Fl_Scalable_Graphics_Driver::arc_unscaled(int x, int y, int w, int h, double a1, double a2) {}

void Fl_Scalable_Graphics_Driver::pie_unscaled(int x, int y, int w, int h, double a1, double a2) {}

void Fl_Scalable_Graphics_Driver::line_style_unscaled(int style, int width, char* dashes) {}

void Fl_Scalable_Graphics_Driver::draw_image_unscaled(const uchar* buf, int X,int Y,int W,int H, int D, int L) {}

void Fl_Scalable_Graphics_Driver::draw_image_unscaled(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D) {}

void Fl_Scalable_Graphics_Driver::draw_image_mono_unscaled(const uchar* buf, int x, int y, int w, int h, int d, int l) {}

void Fl_Scalable_Graphics_Driver::draw_image_mono_unscaled(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D) {}

float Fl_Scalable_Graphics_Driver::override_scale() {
  float s = scale();
  if (s != 1.f) {
    push_no_clip();
    scale(1.f);
  }
  return s;
}

void Fl_Scalable_Graphics_Driver::restore_scale(float s) {
  if (s != 1.f) {
    scale(s);
    pop_clip();
  }
}

#endif
