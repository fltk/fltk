//
// Fl_Graphics_Driver class for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2020 by Bill Spitzak and others.
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
 \cond DriverDev
 \addtogroup DriverDeveloper
 \{
 */

#include <FL/Fl.H>
#include "config_lib.h"
#include <FL/Fl_Graphics_Driver.H>
#include "Fl_Screen_Driver.H"
#include <FL/Fl_Image.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Image_Surface.H>
#include <FL/math.h>
#include <FL/platform.H>

FL_EXPORT Fl_Graphics_Driver *fl_graphics_driver; // the current driver of graphics operations

const Fl_Graphics_Driver::matrix Fl_Graphics_Driver::m0 = {1, 0, 0, 1, 0, 0};

unsigned Fl_Graphics_Driver::need_pixmap_bg_color = 0;

/** Constructor */
Fl_Graphics_Driver::Fl_Graphics_Driver()
{
  font_ = 0;
  size_ = 0;
  sptr=0; rstackptr=0;
  rstack[0] = NULL;
  fl_clip_state_number=0;
  m = m0;
  fl_matrix = &m;
  font_descriptor_ = NULL;
  scale_ = 1;
};

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
  if (px + pw > px_width/s) {pw = px_width/s - px;}
  if (py + ph > px_height/s) {ph = px_height/s - py;}
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

/** see fl_set_spot() */
void Fl_Graphics_Driver::set_spot(int font, int size, int X, int Y, int W, int H, Fl_Window *win)
{
  // nothing to do, reimplement in driver if needed
}


/** see fl_reset_spot() */
void Fl_Graphics_Driver::reset_spot()
{
  // nothing to do, reimplement in driver if needed
}


/** Sets the value of the fl_gc global variable when it changes */
void Fl_Graphics_Driver::global_gc()
{
  // nothing to do, reimplement in driver if needed
}


/** see Fl::set_color(Fl_Color, unsigned) */
void Fl_Graphics_Driver::set_color(Fl_Color i, unsigned c)
{
  // nothing to do, reimplement in driver if needed
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
void Fl_Graphics_Driver::cache_size(int &width, int &height)
{
  if ( int(scale()) == scale() ) {
    width  = width * scale();
    height = height * scale();
  } else {
    width  = (width+1) * scale();
    height = (height+1) * scale();
  }
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
  cache_size(w2, h2); // after this, w2 x h2 is size of desired cached image
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
  cache_size(w2, h2); // after this, w2 x h2 is size of desired cached image
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
    cache_size(w2, h2);
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
  line(x, y, x1, y);
  line(x1, y, x1, y2);
}

void Fl_Graphics_Driver::xyline(int x, int y, int x1, int y2, int x3) {
  line(x, y, x1, y);
  line(x1, y, x1, y2);
  line(x1, y2, x3, y2);
}

void Fl_Graphics_Driver::yxline(int x, int y, int y1) {
  line(x, y, x, y1);
}

void Fl_Graphics_Driver::yxline(int x, int y, int y1, int x2) {
  line(x, y, x, y1);
  line(x, y1, x2, y1);
}

void Fl_Graphics_Driver::yxline(int x, int y, int y1, int x2, int y3) {
  line(x, y, x, y1);
  line(x, y1, x2, y1);
  line(x2, y1, x2, y3);
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

void Fl_Graphics_Driver::make_unused_color_(unsigned char &r, unsigned char &g, unsigned char &b) {}

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
Fl_Bitmask Fl_Graphics_Driver::create_bitmask(int w, int h, const uchar *array) {return 0; }

/** Support function for image drawing */
void Fl_Graphics_Driver::delete_bitmask(Fl_Bitmask bm) {}

/** see fl_point() */
void Fl_Graphics_Driver::point(int x, int y) {}

/** see fl_rect() */
void Fl_Graphics_Driver::rect(int x, int y, int w, int h) {}

/** see fl_rectf() */
void Fl_Graphics_Driver::rectf(int x, int y, int w, int h) {}

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
void Fl_Graphics_Driver::begin_complex_polygon() {}

/** see fl_transformed_vertex() */
void Fl_Graphics_Driver::transformed_vertex(double xf, double yf) {}

/** see fl_vertex() */
void Fl_Graphics_Driver::vertex(double x, double y) {}

/** see fl_end_points() */
void Fl_Graphics_Driver::end_points() {}

/** see fl_end_line() */
void Fl_Graphics_Driver::end_line() {}

/** see fl_end_loop() */
void Fl_Graphics_Driver::end_loop() {}

/** see fl_end_polygon() */
void Fl_Graphics_Driver::end_polygon() {}

/** see fl_end_complex_polygon() */
void Fl_Graphics_Driver::end_complex_polygon() {}

/** see fl_gap() */
void Fl_Graphics_Driver::gap() {}

/** see fl_circle() */
void Fl_Graphics_Driver::circle(double x, double y, double r) {}

/** see fl_arc(int x, int y, int w, int h, double a1, double a2) */
void Fl_Graphics_Driver::arc(int x, int y, int w, int h, double a1, double a2) {}

/** see fl_pie() */
void Fl_Graphics_Driver::pie(int x, int y, int w, int h, double a1, double a2) {}

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
double Fl_Graphics_Driver::width(unsigned int c) { char ch = (char)c; return width(&ch, 1); }

/** Return the current line height */
int Fl_Graphics_Driver::height() { return size(); }

/** Return the current line descent */
int Fl_Graphics_Driver::descent() { return 0; }

/** Set the current Fl_Font_Descriptor */
void Fl_Graphics_Driver::font_descriptor(Fl_Font_Descriptor *d) { font_descriptor_ = d;}

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
}

void Fl_Scalable_Graphics_Driver::rect(int x, int y, int w, int h)
{
  if (int(scale()) == scale()) {
    rect_unscaled(x * scale(), y * scale(), w * scale(), h * scale());
  } else {
    if (w > 0 && h > 0) {
      xyline(x, y, x+w-1);
      yxline(x, y, y+h-1);
      yxline(x+w-1, y, y+h-1);
      xyline(x, y+h-1, x+w-1);
    }
  }
}

void Fl_Scalable_Graphics_Driver::rectf(int x, int y, int w, int h)
{
  rectf_unscaled(x * scale(), y * scale(), w * scale(), h * scale());
}

void Fl_Scalable_Graphics_Driver::point(int x, int y) {
  point_unscaled(x * scale(), y * scale());
}

void Fl_Scalable_Graphics_Driver::line(int x, int y, int x1, int y1) {
  if (y == y1) xyline(x, y, x1);
  else if (x == x1) yxline(x, y, y1);
  else line_unscaled( x*scale(), y*scale(), x1*scale(), y1*scale());
}

void Fl_Scalable_Graphics_Driver::line(int x, int y, int x1, int y1, int x2, int y2) {
  if ( (y == y1 || x == x1) && (y2 == y1 || x2 == x1) ) { // only horizontal or vertical lines
    line(x, y, x1, y1);
    line(x1, y1, x2, y2);
  } else line_unscaled( x*scale(), y*scale(), x1*scale(), y1*scale(), x2*scale(), y2*scale());
}

void Fl_Scalable_Graphics_Driver::xyline(int x, int y, int x1) {
  xyline_unscaled(x*scale(), y*scale(), x1*scale());
}

void Fl_Scalable_Graphics_Driver::xyline(int x, int y, int x1, int y2) {
  xyline(x, y, x1);
  yxline(x1, y, y2);
}

void Fl_Scalable_Graphics_Driver::xyline(int x, int y, int x1, int y2, int x3) {
  xyline(x, y, x1);
  yxline(x1, y, y2);
  xyline(x1, y2, x3);
}

void Fl_Scalable_Graphics_Driver::yxline(int x, int y, int y1) {
  yxline_unscaled(x*scale(), y*scale(), y1*scale());
}

void Fl_Scalable_Graphics_Driver::yxline(int x, int y, int y1, int x2) {
  yxline(x, y, y1);
  xyline(x, y1, x2);
}

void Fl_Scalable_Graphics_Driver::yxline(int x, int y, int y1, int x2, int y3) {
  yxline(x, y, y1);
  xyline(x, y1, x2);
  yxline(x2, y1, y3);
}

void Fl_Scalable_Graphics_Driver::loop(int x0, int y0, int x1, int y1, int x2, int y2) {
  loop_unscaled(x0*scale(), y0*scale(), x1*scale(), y1*scale(), x2*scale(), y2*scale());
}

void Fl_Scalable_Graphics_Driver::loop(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) {
    loop_unscaled(x0*scale(), y0*scale(), x1*scale(), y1*scale(), x2*scale(), y2*scale(), x3*scale(), y3*scale());
}

void Fl_Scalable_Graphics_Driver::polygon(int x0, int y0, int x1, int y1, int x2, int y2) {
  polygon_unscaled(x0*scale(), y0*scale(), x1*scale(), y1*scale(), x2*scale(), y2*scale());
}

void Fl_Scalable_Graphics_Driver::polygon(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) {
  polygon_unscaled(x0*scale(), y0*scale(), x1*scale(), y1*scale(), x2*scale(), y2*scale(), x3*scale(), y3*scale());
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
  font_unscaled(face, size * scale());
}

double Fl_Scalable_Graphics_Driver::width(const char *str, int n) {
  return width_unscaled(str, n)/scale();
}

double Fl_Scalable_Graphics_Driver::width(unsigned int c) {
  return width_unscaled(c)/scale();
}

Fl_Fontsize Fl_Scalable_Graphics_Driver::size() {
  if (!font_descriptor() ) return -1;
  return size_unscaled()/scale();
}

void Fl_Scalable_Graphics_Driver::text_extents(const char *str, int n, int &dx, int &dy, int &w, int &h) {
  text_extents_unscaled(str, n, dx, dy, w, h);
  dx /= scale();
  dy /= scale();
  w /= scale();
  h /= scale();
}

int Fl_Scalable_Graphics_Driver::height() {
  return int(height_unscaled()/scale());
}

int Fl_Scalable_Graphics_Driver::descent() {
  return descent_unscaled()/scale();
}

void Fl_Scalable_Graphics_Driver::draw(const char *str, int n, int x, int y) {
  if (!size_ || !font_descriptor()) font(FL_HELVETICA, FL_NORMAL_SIZE);
  Fl_Region r2 = scale_clip(scale());
  draw_unscaled(str, n, x*scale(), y*scale());
  unscale_clip(r2);
}

void Fl_Scalable_Graphics_Driver::draw(int angle, const char *str, int n, int x, int y) {
  if (!size_ || !font_descriptor()) font(FL_HELVETICA, FL_NORMAL_SIZE);
  Fl_Region r2 = scale_clip(scale());
  draw_unscaled(angle, str, n, x*scale(), y*scale());
  unscale_clip(r2);
}

void Fl_Scalable_Graphics_Driver::rtl_draw(const char* str, int n, int x, int y) {
  rtl_draw_unscaled(str, n, x * scale(), y * scale());
}

void Fl_Scalable_Graphics_Driver::arc(int x,int y,int w,int h,double a1,double a2) {
  arc_unscaled(x * scale(), y * scale(), w * scale(), h * scale(), a1, a2);
}

void Fl_Scalable_Graphics_Driver::pie(int x,int y,int w,int h,double a1,double a2) {
  pie_unscaled(x * scale(), y * scale(), w * scale(), h * scale(), a1, a2);
}

void Fl_Scalable_Graphics_Driver::line_style(int style, int width, char* dashes) {
  if (width == 0) line_width_ = scale() < 2 ? 0 : scale();
  else line_width_ = width>0 ? width*scale() : -width*scale();
  line_style_unscaled(style, line_width_, dashes);
}

/* read the image data from a pointer or with a callback, scale it, and draw it */
void Fl_Scalable_Graphics_Driver::draw_image_rescale(void *buf, Fl_Draw_Image_Cb cb,
                                                     int X, int Y, int W, int H, int D, int L, bool mono, float s) {
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
  Fl_RGB_Image *scaled_rgb = (Fl_RGB_Image*)rgb->copy(ceil(W * s), ceil(H * s));
  Fl_Image::RGB_scaling(keep);
  delete rgb;
  if (scaled_rgb) {
    Fl_Region r2 = scale_clip(s);
    draw_image_unscaled(scaled_rgb->array, X * s, Y * s, scaled_rgb->w(), scaled_rgb->h(), depth);
    unscale_clip(r2);
    delete scaled_rgb;
  }
}

void Fl_Scalable_Graphics_Driver::draw_image(const uchar* buf, int X,int Y,int W,int H, int D, int L) {
  if (scale() == 1) {
    draw_image_unscaled(buf, X,Y,W,H,D,L);
  } else {
    draw_image_rescale((void*)buf, NULL, X, Y, W, H, D, L, false, scale());
  }
}

void Fl_Scalable_Graphics_Driver::draw_image(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D) {
  if (scale() == 1) {
    draw_image_unscaled(cb, data, X,Y,W,H,D);
  } else {
    draw_image_rescale(data, cb, X, Y, W, H, D, 0, false, scale());
  }
}

void Fl_Scalable_Graphics_Driver::draw_image_mono(const uchar* buf, int X,int Y,int W,int H, int D, int L) {
  if (scale() == 1) {
    draw_image_mono_unscaled(buf, X,Y,W,H,D,L);
  } else {
    draw_image_rescale((void*)buf, NULL, X, Y, W, H, D, L, true, scale());
  }
}

void Fl_Scalable_Graphics_Driver::draw_image_mono(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D) {
  if (scale() == 1) {
    draw_image_mono_unscaled(cb, data, X,Y,W,H,D);
  } else {
    draw_image_rescale(data, cb, X, Y, W, H, D, 0, true, scale());
  }
}

void Fl_Scalable_Graphics_Driver::transformed_vertex(double xf, double yf) {
  transformed_vertex0(xf * scale(), yf * scale());
}

void Fl_Scalable_Graphics_Driver::vertex(double x,double y) {
  transformed_vertex0((x*m.a + y*m.c + m.x) * scale(), (x*m.b + y*m.d + m.y) * scale());
}

void Fl_Scalable_Graphics_Driver::unscale_clip(Fl_Region r) {
  if (r) {
    if (rstack[rstackptr]) XDestroyRegion(rstack[rstackptr]);
    rstack[rstackptr] = r;
  }
}

Fl_Region Fl_Scalable_Graphics_Driver::scale_clip(float f) { return 0; }

void Fl_Scalable_Graphics_Driver::point_unscaled(float x, float y) {}

void Fl_Scalable_Graphics_Driver::rect_unscaled(float x, float y, float w, float h) {}

void Fl_Scalable_Graphics_Driver::rectf_unscaled(float x, float y, float w, float h) {}

void Fl_Scalable_Graphics_Driver::line_unscaled(float x, float y, float x1, float y1) {}

void Fl_Scalable_Graphics_Driver::line_unscaled(float x, float y, float x1, float y1, float x2, float y2) {}

void Fl_Scalable_Graphics_Driver::xyline_unscaled(float x, float y, float x1) {}

void Fl_Scalable_Graphics_Driver::yxline_unscaled(float x, float y, float y1) {}

void Fl_Scalable_Graphics_Driver::loop_unscaled(float x0, float y0, float x1, float y1, float x2, float y2) {}

void Fl_Scalable_Graphics_Driver::loop_unscaled(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3) {}

void Fl_Scalable_Graphics_Driver::polygon_unscaled(float x0, float y0, float x1, float y1, float x2, float y2) {}

void Fl_Scalable_Graphics_Driver::polygon_unscaled(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3) {}

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

void Fl_Scalable_Graphics_Driver::arc_unscaled(float x, float y, float w, float h, double a1, double a2) {}

void Fl_Scalable_Graphics_Driver::pie_unscaled(float x, float y, float w, float h, double a1, double a2) {}

void Fl_Scalable_Graphics_Driver::line_style_unscaled(int style, float width, char* dashes) {}

void Fl_Scalable_Graphics_Driver::draw_image_unscaled(const uchar* buf, int X,int Y,int W,int H, int D, int L) {}

void Fl_Scalable_Graphics_Driver::draw_image_unscaled(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D) {}

void Fl_Scalable_Graphics_Driver::draw_image_mono_unscaled(const uchar* buf, int x, int y, int w, int h, int d, int l) {}

void Fl_Scalable_Graphics_Driver::draw_image_mono_unscaled(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D) {}

void Fl_Scalable_Graphics_Driver::transformed_vertex0(float x, float y) {}

#endif
