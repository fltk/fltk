//
// Support for Cairo graphics for the Fast Light Tool Kit (FLTK).
//
// Copyright 2021-2024 by Bill Spitzak and others.
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

/* \file
    Implementation of class Fl_Cairo_Graphics_Driver .
*/

#include <config.h>
#if USE_PANGO

#include "Fl_Cairo_Graphics_Driver.H"
#include "../../Fl_Screen_Driver.H"
#include <FL/platform.H>
#include <FL/fl_draw.H>
#include <FL/fl_utf8.h>
#include <cairo/cairo.h>
#include <pango/pangocairo.h>
#if ! PANGO_VERSION_CHECK(1,16,0)
#  error "Requires Pango 1.16 or higher"
#endif
#if CAIRO_VERSION < CAIRO_VERSION_ENCODE(1,10,0)
#  error "Requires Cairo 1.10 or higher"
#endif
#include <math.h>
#include <stdlib.h>  // abs(int)
#include <string.h>  // memcpy()
#include <stdint.h>  // uint32_t

extern unsigned fl_cmap[256]; // defined in fl_color.cxx

// The predefined fonts that FLTK has with Pango:
static Fl_Fontdesc built_in_table[] = {
  {"Sans"},
  {"Sans Bold"},
  {"Sans Italic"},
  {"Sans Bold Italic"},
  {"Monospace"},
  {"Monospace Bold"},
  {"Monospace Italic"},
  {"Monospace Bold Italic"},
  {"Serif"},
  {"Serif Bold"},
  {"Serif Italic"},
  {"Serif Bold Italic"},
  {"Standard Symbols PS"}, // FL_SYMBOL
  {"Monospace"},
  {"Monospace Bold"},
  {"D050000L"},            // FL_ZAPF_DINGBATS
};


FL_EXPORT Fl_Fontdesc *fl_fonts = built_in_table;

// Number of fonts found by Fl::set_fonts(char*) beyond FL_FREE_FONT
// -1 denotes "not yet initialised"
Fl_Font Fl_Cairo_Graphics_Driver::font_count_ = -1;

// duplicated from Fl_PostScript.cxx
struct callback_data {
  const uchar *data;
  int D, LD;
};
static const int dashes_flat[5][7]={
{-1,0,0,0,0,0,0},
{3,1,-1,0,0,0,0},
{1,1,-1,0,0,0,0},
{3,1,1,1,-1,0,0},
{3,1,1,1,1,1,-1}
};
static const double dashes_cap[5][7]={
{-1,0,0,0,0,0,0},
{2,2,-1,0,0,0,0},
{0.01,1.99,-1,0,0,0,0},
{2,2,0.01,1.99,-1,0,0},
{2,2,0.01,1.99,0.01,1.99,-1}
};
static void draw_image_cb(void *data, int x, int y, int w, uchar *buf) {
  struct callback_data *cb_data;
  const uchar *curdata;

  cb_data = (struct callback_data*)data;
  int last = x+w;
  const size_t aD = abs(cb_data->D);
  curdata = cb_data->data + x*cb_data->D + y*cb_data->LD;
  for (; x<last; x++) {
    memcpy(buf, curdata, aD);
    buf += aD;
    curdata += cb_data->D;
  }
}
// end of duplicated part


Fl_Cairo_Graphics_Driver::Fl_Cairo_Graphics_Driver() : Fl_Graphics_Driver() {
  cairo_ = NULL;
  pango_layout_ = NULL;
  pango_context_ = NULL;
  dummy_cairo_ = NULL;
  linestyle_ = FL_SOLID;
  clip_ = NULL;
  scale_x = scale_y = 1;
  wld_scale = 1;
  angle = 0;
  left_margin = top_margin = 0;
  needs_commit_tag_ = NULL;
  what = NONE;
}

Fl_Cairo_Graphics_Driver::~Fl_Cairo_Graphics_Driver() {
  if (pango_layout_) g_object_unref(pango_layout_);
  if (pango_context_) g_object_unref(pango_context_);
}

const cairo_format_t Fl_Cairo_Graphics_Driver::cairo_format = CAIRO_FORMAT_ARGB32;


void Fl_Cairo_Graphics_Driver::set_cairo(cairo_t *cr, float s) {
  if (dummy_cairo_) {
    cairo_destroy(dummy_cairo_);
    dummy_cairo_ = NULL;
 }
  cairo_ = cr;
  cairo_restore(cairo_);
  line_style(0);
  cairo_save(cairo_);
  if (s == 0) s = scale();
  cairo_scale(cairo_, s, s);
  cairo_translate(cairo_, 0.5, 0.5);
}


void Fl_Cairo_Graphics_Driver::rectf(int x, int y, int w, int h) {
  cairo_rectangle(cairo_, x-0.5, y-0.5, w, h);
  cairo_set_antialias(cairo_, CAIRO_ANTIALIAS_NONE);
  cairo_fill(cairo_);
  cairo_set_antialias(cairo_, CAIRO_ANTIALIAS_DEFAULT);
  check_status();
  surface_needs_commit();
}

void Fl_Cairo_Graphics_Driver::rect(int x, int y, int w, int h) {
  cairo_rectangle(cairo_, x, y, w-1, h-1);
  if (linestyle_ == FL_SOLID) cairo_set_antialias(cairo_, CAIRO_ANTIALIAS_NONE);
  cairo_stroke(cairo_);
  if (linestyle_ == FL_SOLID) cairo_set_antialias(cairo_, CAIRO_ANTIALIAS_DEFAULT);
  check_status();
  surface_needs_commit();
}

static bool need_antialias_none(cairo_t *cairo_, int style) {
  cairo_matrix_t matrix;
  cairo_get_matrix(cairo_, &matrix);
  double width = cairo_get_line_width(cairo_) * matrix.xx;
  bool needit = (style == FL_SOLID && width < 1.5);
  if (needit) cairo_set_antialias(cairo_, CAIRO_ANTIALIAS_NONE);
  return needit;
}

void Fl_Cairo_Graphics_Driver::line(int x1, int y1, int x2, int y2) {
  cairo_new_path(cairo_);
  cairo_move_to(cairo_, x1, y1);
  cairo_line_to(cairo_, x2, y2);
  bool needit = need_antialias_none(cairo_, linestyle_);
  cairo_stroke(cairo_);
  if (needit) cairo_set_antialias(cairo_, CAIRO_ANTIALIAS_DEFAULT);
  surface_needs_commit();
}

void Fl_Cairo_Graphics_Driver::line(int x0, int y0, int x1, int y1, int x2, int y2) {
  cairo_new_path(cairo_);
  cairo_move_to(cairo_, x0, y0);
  cairo_line_to(cairo_, x1, y1);
  cairo_line_to(cairo_, x2, y2);
  bool needit = need_antialias_none(cairo_, linestyle_);
  cairo_stroke(cairo_);
  if (needit) cairo_set_antialias(cairo_, CAIRO_ANTIALIAS_DEFAULT);
  surface_needs_commit();
}

void Fl_Cairo_Graphics_Driver::xyline(int x, int y, int x1) {
  cairo_move_to(cairo_, x, y);
  cairo_line_to(cairo_, x1, y);
  cairo_set_antialias(cairo_, CAIRO_ANTIALIAS_NONE);
  cairo_stroke(cairo_);
  cairo_set_antialias(cairo_, CAIRO_ANTIALIAS_DEFAULT);
  check_status();
  surface_needs_commit();
}

void Fl_Cairo_Graphics_Driver::xyline(int x, int y, int x1, int y2) {
  cairo_move_to(cairo_, x, y);
  cairo_line_to(cairo_, x1, y);
  cairo_line_to(cairo_, x1, y2);
  cairo_set_antialias(cairo_, CAIRO_ANTIALIAS_NONE);
  cairo_stroke(cairo_);
  cairo_set_antialias(cairo_, CAIRO_ANTIALIAS_DEFAULT);
  check_status();
  surface_needs_commit();
}

void Fl_Cairo_Graphics_Driver::xyline(int x, int y, int x1, int y2, int x3) {
  cairo_move_to(cairo_, x, y);
  cairo_line_to(cairo_, x1, y);
  cairo_line_to(cairo_, x1, y2);
  cairo_line_to(cairo_, x3, y2);
  cairo_set_antialias(cairo_, CAIRO_ANTIALIAS_NONE);
  cairo_stroke(cairo_);
  cairo_set_antialias(cairo_, CAIRO_ANTIALIAS_DEFAULT);
  check_status();
  surface_needs_commit();
}

void Fl_Cairo_Graphics_Driver::yxline(int x, int y, int y1) {
  cairo_move_to(cairo_, x, y);
  cairo_line_to(cairo_, x, y1);
  cairo_set_antialias(cairo_, CAIRO_ANTIALIAS_NONE);
  cairo_stroke(cairo_);
  cairo_set_antialias(cairo_, CAIRO_ANTIALIAS_DEFAULT);
  check_status();
  surface_needs_commit();
}

void Fl_Cairo_Graphics_Driver::yxline(int x, int y, int y1, int x2) {
  cairo_move_to(cairo_, x, y);
  cairo_line_to(cairo_, x, y1);
  cairo_line_to(cairo_, x2, y1);
  cairo_set_antialias(cairo_, CAIRO_ANTIALIAS_NONE);
  cairo_stroke(cairo_);
  cairo_set_antialias(cairo_, CAIRO_ANTIALIAS_DEFAULT);
  check_status();
  surface_needs_commit();
}

void Fl_Cairo_Graphics_Driver::yxline(int x, int y, int y1, int x2, int y3) {
  cairo_move_to(cairo_, x, y);
  cairo_line_to(cairo_, x, y1);
  cairo_line_to(cairo_, x2, y1);
  cairo_line_to(cairo_, x2, y3);
  cairo_set_antialias(cairo_, CAIRO_ANTIALIAS_NONE);
  cairo_stroke(cairo_);
  cairo_set_antialias(cairo_, CAIRO_ANTIALIAS_DEFAULT);
  check_status();
  surface_needs_commit();
}

void Fl_Cairo_Graphics_Driver::loop(int x0, int y0, int x1, int y1, int x2, int y2) {
  cairo_save(cairo_);
  cairo_new_path(cairo_);
  cairo_move_to(cairo_, x0, y0);
  cairo_line_to(cairo_, x1, y1);
  cairo_line_to(cairo_, x2, y2);
  cairo_close_path(cairo_);
  cairo_stroke(cairo_);
  cairo_restore(cairo_);
  surface_needs_commit();
}

void Fl_Cairo_Graphics_Driver::loop(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) {
  cairo_save(cairo_);
  cairo_new_path(cairo_);
  cairo_move_to(cairo_, x0, y0);
  cairo_line_to(cairo_, x1, y1);
  cairo_line_to(cairo_, x2, y2);
  cairo_line_to(cairo_, x3, y3);
  cairo_close_path(cairo_);
  if ((y0==y1 && x1==x2 && y2==y3 && x3==x0) || (x0==x1 && y1==y2 && x2==x3 && y3==y0)) {
    cairo_set_antialias(cairo_, CAIRO_ANTIALIAS_NONE);
  }
  cairo_stroke(cairo_);
  cairo_restore(cairo_);
  surface_needs_commit();
}

void Fl_Cairo_Graphics_Driver::polygon(int x0, int y0, int x1, int y1, int x2, int y2) {
  cairo_save(cairo_);
  cairo_new_path(cairo_);
  cairo_move_to(cairo_, x0, y0);
  cairo_line_to(cairo_, x1, y1);
  cairo_line_to(cairo_, x2, y2);
  cairo_close_path(cairo_);
  cairo_fill(cairo_);
  cairo_restore(cairo_);
  surface_needs_commit();
}

void Fl_Cairo_Graphics_Driver::polygon(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) {
  cairo_save(cairo_);
  cairo_new_path(cairo_);
  cairo_move_to(cairo_, x0, y0);
  cairo_line_to(cairo_, x1, y1);
  cairo_line_to(cairo_, x2, y2);
  cairo_line_to(cairo_, x3, y3);
  cairo_close_path(cairo_);
  if ((y0==y1 && x1==x2 && y2==y3 && x3==x0) || (x0==x1 && y1==y2 && x2==x3 && y3==y0)) {
    cairo_set_antialias(cairo_, CAIRO_ANTIALIAS_NONE);
  }
  cairo_fill(cairo_);
  cairo_restore(cairo_);
  surface_needs_commit();
}

void Fl_Cairo_Graphics_Driver::line_style(int style, int width, char* dashes) {
  linestyle_ = style;
  if(dashes){
    if(dashes != linedash_)
      strcpy(linedash_,dashes);

  } else
    linedash_[0]=0;
  char width0 = 0;
  if (!width){
    width=1; //for screen drawing compatibility
    width0=1;
  }
  cairo_set_line_width(cairo_, width);

  if(!style && (!dashes || !(*dashes)) && width0) //system lines
    style = FL_CAP_SQUARE;

  int cap = (style &0xf00);
  cairo_line_cap_t c_cap;
  if (cap == FL_CAP_SQUARE) c_cap = CAIRO_LINE_CAP_SQUARE;
  else if (cap == FL_CAP_FLAT) c_cap = CAIRO_LINE_CAP_BUTT;
  else if (cap == FL_CAP_ROUND) c_cap = CAIRO_LINE_CAP_ROUND;
  else c_cap = CAIRO_LINE_CAP_BUTT;
  cairo_set_line_cap(cairo_, c_cap);

  int join = (style & 0xf000);
  cairo_line_join_t c_join;
  if (join == FL_JOIN_MITER) c_join = CAIRO_LINE_JOIN_MITER;
  else if (join == FL_JOIN_ROUND)c_join = CAIRO_LINE_JOIN_ROUND;
  else if (join == FL_JOIN_BEVEL) c_join = CAIRO_LINE_JOIN_BEVEL;
  else c_join = CAIRO_LINE_JOIN_MITER;
  cairo_set_line_join(cairo_, c_join);

  double *ddashes = NULL;
  int l = 0;
  if (dashes && *dashes){
    ddashes = new double[strlen(dashes)];
    while (dashes[l]) {ddashes[l] = dashes[l]; l++; }
  } else if (style & 0xff) {
    ddashes = new double[6];
    if (style & 0x200){ // round and square caps, dash length need to be adjusted
      const double *dt = dashes_cap[style & 0xff];
      while (*dt >= 0){
        ddashes[l++] = width * (*dt);
        dt++;
      }
    } else {
      const int *ds = dashes_flat[style & 0xff];
      while (*ds >= 0){
        ddashes[l++] = width * (*ds);
        ds++;
      }
    }
  }
  cairo_set_dash(cairo_, ddashes, l, 0);
  cairo_set_antialias(cairo_, l ? CAIRO_ANTIALIAS_NONE : CAIRO_ANTIALIAS_DEFAULT);
  delete[] ddashes;
  check_status();
}

void Fl_Cairo_Graphics_Driver::color(unsigned char r, unsigned char g, unsigned char b) {
  Fl_Graphics_Driver::color( fl_rgb_color(r, g, b) );
  cr_ = r; cg_ = g; cb_ = b;
  double fr, fg, fb;
  fr = r/255.0;
  fg = g/255.0;
  fb = b/255.0;
  cairo_set_source_rgb(cairo_, fr, fg, fb);
  check_status();
}

void Fl_Cairo_Graphics_Driver::color(Fl_Color i) {
  Fl_Graphics_Driver::color(i);
  if (!cairo_) return; // no context yet? We will assign the color later.
  uchar r, g, b;
  double fa = 1.0;
  if (i & 0xFFFFFF00) {
    // translate rgb colors into color index
    r = i>>24;
    g = i>>16;
    b = i>> 8;
  } else {
    // translate index into rgb:
    unsigned c = fl_cmap[i];
    c = c ^ 0x000000ff; // trick to restore the color's correct alpha value
    r = c>>24;
    g = c>>16;
    b = c>> 8;
    fa = (c & 0xff)/255.0;
  }
  double fr = r/255.0;
  double fg = g/255.0;
  double fb = b/255.0;
  cairo_set_source_rgba(cairo_, fr, fg, fb, fa);
}

Fl_Color Fl_Cairo_Graphics_Driver::color() { return Fl_Graphics_Driver::color(); }


void Fl_Cairo_Graphics_Driver::concat(){
  cairo_matrix_t mat = {m.a , m.b , m.c , m.d , m.x , m.y};
  cairo_transform(cairo_, &mat);
}

void Fl_Cairo_Graphics_Driver::reconcat(){
  cairo_matrix_t mat = {m.a , m.b , m.c , m.d , m.x , m.y};
  cairo_status_t stat = cairo_matrix_invert(&mat);
  if (stat != CAIRO_STATUS_SUCCESS) {
    fputs("error in cairo_matrix_invert\n", stderr);
  }
  cairo_transform(cairo_, &mat);
}


void Fl_Cairo_Graphics_Driver::begin_line() {
  cairo_save(cairo_);
  concat();
  cairo_new_path(cairo_);
  gap_=1;
  what=LINE;
}

void Fl_Cairo_Graphics_Driver::begin_loop() {
  cairo_save(cairo_);
  concat();
  cairo_new_path(cairo_);
  gap_=1;
  what=LOOP;
}

void Fl_Cairo_Graphics_Driver::begin_polygon() {
  cairo_save(cairo_);
  concat();
  cairo_new_path(cairo_);
  gap_=1;
  what=POLYGON;
}

void Fl_Cairo_Graphics_Driver::vertex(double x, double y) {
  if (what==POINTS){
    return Fl_Graphics_Driver::vertex(x, y);
  }
  if (gap_){
    cairo_move_to(cairo_, x, y);
    gap_=0;
  } else {
    cairo_line_to(cairo_, x, y);
    surface_needs_commit();
  }
}

void Fl_Cairo_Graphics_Driver::curve(double x, double y, double x1, double y1, double x2, double y2, double x3, double y3)
{
  if(what==NONE) return;
  if (what == POINTS) Fl_Graphics_Driver::curve(x, y, x1, y1, x2, y2, x3, y3);
  else {
  if(gap_)
    cairo_move_to(cairo_, x, y);
  else
    cairo_line_to(cairo_, x, y);
  gap_=0;
  cairo_curve_to(cairo_, x1 , y1 , x2 , y2 , x3 , y3);
  }
  surface_needs_commit();
}

void Fl_Cairo_Graphics_Driver::circle(double x, double y, double r){
  if (what == NONE) {
    cairo_save(cairo_);
    concat();
    cairo_arc(cairo_, x, y, r, 0, 2*M_PI);
    cairo_stroke(cairo_);
    reconcat();
    cairo_restore(cairo_);
  } else {
    cairo_arc(cairo_, x, y, r, 0, 2*M_PI);
  }
  surface_needs_commit();
}

void Fl_Cairo_Graphics_Driver::arc(double x, double y, double r, double start, double a){
  if (what == NONE) return;
  if (gap_ == 1) cairo_new_sub_path(cairo_); // 1.2
  gap_ = 0;
  if (start > a)
    cairo_arc(cairo_, x, y, r, -start*M_PI/180, -a*M_PI/180);
  else
    cairo_arc_negative(cairo_, x, y, r, -start*M_PI/180, -a*M_PI/180);
  surface_needs_commit();
}

void Fl_Cairo_Graphics_Driver::arc(int x, int y, int w, int h, double a1, double a2) {
  if (w <= 1 || h <= 1) return;
  cairo_save(cairo_);
  begin_line();
  cairo_translate(cairo_, x + w/2.0 -0.5 , y + h/2.0 - 0.5);
  cairo_scale(cairo_, (w-1)/2.0 , (h-1)/2.0);
  arc(0,0,1,a2,a1);
  cairo_scale(cairo_, 2.0/(w-1) , 2.0/(h-1));
  cairo_translate(cairo_, -x - w/2.0 +0.5 , -y - h/2.0 +0.5);
  end_line();
  cairo_restore(cairo_);
  surface_needs_commit();
}

void Fl_Cairo_Graphics_Driver::pie(int x, int y, int w, int h, double a1, double a2) {
  cairo_save(cairo_);
  begin_polygon();
  cairo_translate(cairo_, x + w/2.0 -0.5 , y + h/2.0 - 0.5);
  cairo_scale(cairo_, w/2.0 , h/2.0);
  vertex(0,0);
  arc(0.0,0.0, 1, a2, a1);
  end_polygon();
  cairo_restore(cairo_);
}

void Fl_Cairo_Graphics_Driver::end_points() {
 for (int i = 0; i < n; i++) {
   point(xpoint[i].x, xpoint[i].y);
 }
}

void Fl_Cairo_Graphics_Driver::end_line() {
  gap_ = 1;
  reconcat();
  cairo_stroke(cairo_);
  cairo_restore(cairo_);
  what = NONE;
  surface_needs_commit();
}

void Fl_Cairo_Graphics_Driver::end_loop(){
  gap_ = 1;
  reconcat();
  cairo_close_path(cairo_);
  cairo_stroke(cairo_);
  cairo_restore(cairo_);
  what = NONE;
  surface_needs_commit();
}

void Fl_Cairo_Graphics_Driver::end_polygon() {
  gap_ = 1;
  reconcat();
  cairo_close_path(cairo_);
  cairo_fill(cairo_);
  cairo_restore(cairo_);
  what = NONE;
  surface_needs_commit();
}

void Fl_Cairo_Graphics_Driver::transformed_vertex(double x, double y) {
  if (what == POINTS) {
    Fl_Graphics_Driver::transformed_vertex(x, y);
  } else {
    reconcat();
    if (gap_) {
      cairo_move_to(cairo_, x, y);
      gap_ = 0;
    } else {
      cairo_line_to(cairo_, x, y);
      surface_needs_commit();
    }
    concat();
  }
}

void Fl_Cairo_Graphics_Driver::push_clip(int x, int y, int w, int h) {
  Clip *c = new Clip();
  clip_box(x,y,w,h,c->x,c->y,c->w,c->h);
  c->prev = clip_;
  clip_ = c;
  cairo_save(cairo_);
  cairo_rectangle(cairo_, clip_->x - 0.5 , clip_->y - 0.5 , clip_->w, clip_->h);
  cairo_set_antialias(cairo_, CAIRO_ANTIALIAS_NONE);
  cairo_clip(cairo_);
  cairo_set_antialias(cairo_, CAIRO_ANTIALIAS_DEFAULT);
  check_status();
}

void Fl_Cairo_Graphics_Driver::push_no_clip() {
  Clip *c = new Clip();
  c->prev = clip_;
  clip_ = c;
  clip_->x = clip_->y = clip_->w = clip_->h = -1;
  cairo_save(cairo_);
  cairo_reset_clip(cairo_);
  check_status();
}

void Fl_Cairo_Graphics_Driver::pop_clip() {
  if(!clip_)return;
  Clip *c = clip_;
  clip_ = clip_->prev;
  delete c;
  cairo_restore(cairo_);
  check_status();
}

void Fl_Cairo_Graphics_Driver::ps_origin(int x, int y) {
  cairo_restore(cairo_);
  cairo_restore(cairo_);
  cairo_save(cairo_);
  cairo_scale(cairo_, scale_x, scale_y);
  cairo_translate(cairo_, x, y);
  cairo_rotate(cairo_, angle * M_PI / 180);
  cairo_save(cairo_);
  check_status();
}

void Fl_Cairo_Graphics_Driver::ps_translate(int x, int y)
{
  cairo_save(cairo_);
  cairo_translate(cairo_, x, y);
  cairo_save(cairo_);
  check_status();
}

void Fl_Cairo_Graphics_Driver::ps_untranslate(void)
{
  cairo_restore(cairo_);
  cairo_restore(cairo_);
  check_status();
}

void Fl_Cairo_Graphics_Driver::check_status(void) {
#ifdef DEBUG
  if (cairo_status(cairo_) !=  CAIRO_STATUS_SUCCESS) {
    fprintf(stderr,"we have a problem");
  }
#endif
}

void Fl_Cairo_Graphics_Driver::draw_image(Fl_Draw_Image_Cb call, void *data, int ix, int iy, int iw, int ih, int D)
{
  uchar *array = new uchar[iw * D * ih];
  for (int l = 0; l < ih; l++) {
    call(data, 0, l, iw, array + l*D*iw);
    if (D%2 == 0) for (int i = 0; i < iw; i++) {
      *(array + l*D*iw + i*D + D-1) = 0xff;
    }
  }
  Fl_RGB_Image *rgb = new Fl_RGB_Image(array, iw, ih, D);
  rgb->alloc_array  = 1;
  draw_rgb(rgb, ix, iy, iw, ih, 0, 0);
  delete rgb;
  surface_needs_commit();
}

void Fl_Cairo_Graphics_Driver::draw_image_mono(const uchar *data, int ix, int iy, int iw, int ih, int D, int LD)
{
  struct callback_data cb_data;
  const size_t aD = abs(D);
  if (!LD) LD = iw * aD;
  cb_data.data = data;
  cb_data.D = D;
  cb_data.LD = LD;
  draw_image(draw_image_cb, &cb_data, ix, iy, iw, ih, abs(D));
  surface_needs_commit();
}

void Fl_Cairo_Graphics_Driver::draw_image_mono(Fl_Draw_Image_Cb call, void *data, int ix, int iy, int iw, int ih, int D)
{
  draw_image(call, data, ix, iy, iw, ih, D);
}


int Fl_Cairo_Graphics_Driver::not_clipped(int x, int y, int w, int h) {
  if (!clip_) return 1;
  if (clip_->w < 0) return 1;
  int X = 0, Y = 0, W = 0, H = 0;
  clip_box(x, y, w, h, X, Y, W, H);
  if (W) return 1;
  return 0;
}


int Fl_Cairo_Graphics_Driver::clip_box(int x, int y, int w, int h, int &X, int &Y, int &W, int &H) {
  if (!clip_) {
    X = x; Y = y; W = w; H = h;
    return 0;
  }
  if (clip_->w < 0) {
    X = x; Y = y; W = w; H = h;
    return 1;
  }
  int ret = 0;
  if (x > (X=clip_->x)) {X=x; ret=1;}
  if (y > (Y=clip_->y)) {Y=y; ret=1;}
  if ((x+w) < (clip_->x+clip_->w)) {
    W=x+w-X;

    ret=1;

  }else
    W = clip_->x + clip_->w - X;
  if(W<0){
    W=0;
    return 1;
  }
  if ((y+h) < (clip_->y+clip_->h)) {
    H=y+h-Y;
    ret=1;
  }else
    H = clip_->y + clip_->h - Y;
  if(H<0){
    W=0;
    H=0;
    return 1;
  }
  return ret;
}


void Fl_Cairo_Graphics_Driver::point(int x, int y) {
  rectf(x, y, 1, 1);
}


void Fl_Cairo_Graphics_Driver::draw_image(const uchar *data, int ix, int iy, int iw, int ih, int D, int LD) {
  if (abs(D)<3){ //mono
    draw_image_mono(data, ix, iy, iw, ih, D, LD);
    return;
  }
  struct callback_data cb_data;
  if (!LD) LD = iw*abs(D);
  cb_data.data = data;
  cb_data.D = D;
  cb_data.LD = LD;
  Fl_Cairo_Graphics_Driver::draw_image(draw_image_cb, &cb_data, ix, iy, iw, ih, abs(D));
}


void Fl_Cairo_Graphics_Driver::overlay_rect(int x, int y, int w , int h) {
  cairo_save(cairo_);
  cairo_matrix_t mat;
  cairo_get_matrix(cairo_, &mat);
  float s = (float)mat.xx;
  cairo_matrix_init_identity(&mat);
  cairo_set_matrix(cairo_, &mat); // use drawing units
  int lwidth = s < 1 ? 1 : int(s);
  cairo_set_line_width(cairo_, lwidth);
  cairo_translate(cairo_, lwidth/2., lwidth/2.); // translate by half of line width
  double ddash = (lwidth > 2 ? lwidth : 2);
  if (linestyle_ == FL_DOT) {
    cairo_set_dash(cairo_, &ddash, 1, 0); // dash size = line width
  }
  // rectangle in drawing units
  int Xs = Fl_Scalable_Graphics_Driver::floor(x, s);
  int Ws = Fl_Scalable_Graphics_Driver::floor(x+w-1, s) - Xs;
  int Ys = Fl_Scalable_Graphics_Driver::floor(y, s);
  int Hs = Fl_Scalable_Graphics_Driver::floor(y+h-1, s) - Ys;
  cairo_move_to(cairo_, Xs, Ys);
  cairo_line_to(cairo_, Xs+Ws, Ys);
  cairo_line_to(cairo_, Xs+Ws, Ys+Hs);
  cairo_line_to(cairo_, Xs, Ys+Hs);
  cairo_close_path(cairo_);
  cairo_stroke(cairo_);
  cairo_restore(cairo_);
  surface_needs_commit();
}


void Fl_Cairo_Graphics_Driver::draw_cached_pattern_(Fl_Image *img, cairo_pattern_t *pat, int X, int Y, int W, int H, int cx, int cy, int cache_w, int cache_h) {
  // compute size of output image in drawing units
  cairo_matrix_t matrix;
  cairo_get_matrix(cairo_, &matrix);
  float s = (float)matrix.xx;
  int Xs = Fl_Scalable_Graphics_Driver::floor(X - cx, s);
  int Ws = Fl_Scalable_Graphics_Driver::floor(X - cx + img->w(), s) - Xs  ;
  int Ys = Fl_Scalable_Graphics_Driver::floor(Y - cy, s);
  int Hs = Fl_Scalable_Graphics_Driver::floor(Y - cy + img->h(), s) - Ys;
  if (Ws == 0 || Hs == 0) return;
  cairo_save(cairo_);
  bool need_extend = (cache_w != Ws || cache_h != Hs || (W >= 2 && H >= 2));
  if (need_extend || cx || cy || W < img->w() || H < img->h()) { // clip when necessary
    cairo_set_antialias(cairo_, CAIRO_ANTIALIAS_NONE);
    cairo_rectangle(cairo_, X - 0.5, Y - 0.5, W, H);
    cairo_clip(cairo_);
    cairo_set_antialias(cairo_, CAIRO_ANTIALIAS_DEFAULT);
  }
  // remove any scaling and the current "0.5" translation useful for lines but bad for images
  matrix.xx = matrix.yy = 1;
  matrix.x0 -= 0.5 * s; matrix.y0 -= 0.5 * s;
  cairo_set_matrix(cairo_, &matrix);
  if (img->d() >= 1) cairo_set_source(cairo_, pat);
  if (need_extend) {
    bool condition = Fl_RGB_Image::scaling_algorithm() == FL_RGB_SCALING_BILINEAR &&
      (fabs(Ws/float(cache_w) - 1) > 0.02 || fabs(Hs/float(cache_h) - 1) > 0.02);
    cairo_pattern_set_filter(pat, condition ? CAIRO_FILTER_GOOD : CAIRO_FILTER_FAST);
    cairo_pattern_set_extend(pat, CAIRO_EXTEND_PAD);
  }
  cairo_matrix_init_scale(&matrix, double(cache_w)/Ws, double(cache_h)/Hs);
  cairo_matrix_translate(&matrix, -Xs , -Ys );
  cairo_pattern_set_matrix(pat, &matrix);
  if (img->d() > 1) cairo_paint(cairo_);
  else cairo_mask(cairo_, pat);
  cairo_restore(cairo_);
  surface_needs_commit();
}


void Fl_Cairo_Graphics_Driver::draw_rgb(Fl_RGB_Image *rgb,int XP, int YP, int WP, int HP, int cx, int cy) {
  int X, Y, W, H;
  // Don't draw an empty image...
  if (!rgb->d() || !rgb->array) {
    Fl_Graphics_Driver::draw_empty(rgb, XP, YP);
    return;
  }
  if (start_image(rgb, XP, YP, WP, HP, cx, cy, X, Y, W, H)) {
    return;
  }
  cairo_pattern_t *pat = (cairo_pattern_t*)*Fl_Graphics_Driver::id(rgb);
  if (!pat) {
    cache(rgb);
    pat = (cairo_pattern_t*)*Fl_Graphics_Driver::id(rgb);
  }
  draw_cached_pattern_(rgb, pat, X, Y, W, H, cx, cy, rgb->cache_w(), rgb->cache_h());
}


static cairo_user_data_key_t data_key_for_surface = {};

static void dealloc_surface_data(void *data) {
  delete[] (uchar*)data;
}


void Fl_Cairo_Graphics_Driver::cache(Fl_RGB_Image *rgb) {
  int stride = cairo_format_stride_for_width(Fl_Cairo_Graphics_Driver::cairo_format,
                                             rgb->data_w()); // 1.6
  uchar *BGRA = new uchar[stride * rgb->data_h()];
  memset(BGRA, 0, stride * rgb->data_h());
  int lrgb = rgb->ld() ? rgb->ld() : rgb->data_w() * rgb->d();
  uchar A = 0xff, R,G,B, *q;
  const uchar *r;
  float f = 1;
  if (rgb->d() >= 3) { // color images
    for (int j = 0; j < rgb->data_h(); j++) {
      r = rgb->array + j * lrgb;
      q = BGRA + j * stride;
      for (int i = 0; i < rgb->data_w(); i++) {
        R = *r;
        G = *(r+1);
        B = *(r+2);
        if (rgb->d() == 4) {
          A = *(r+3);
          f = float(A)/0xff;
        }
        // this produces ARGB data in native endian
        *(uint32_t *)q = A << 24 | (uchar)(R*f) << 16 | (uchar)(G*f) << 8 | (uchar)(B*f);
        r += rgb->d(); q += 4;
      }
    }
  } else if (rgb->d() == 1 || rgb->d() == 2) { // B&W
    for (int j = 0; j < rgb->data_h(); j++) {
      r = rgb->array + j * lrgb;
      q = BGRA + j * stride;
      for (int i = 0; i < rgb->data_w(); i++) {
        G = *r;
        if (rgb->d() == 2) {
          A = *(r+1);
          f = float(A)/0xff;
        }
        G = (uchar)(G * f);
        // this produces ARGB data in native endian
        *(uint32_t *)q = A << 24 | G << 16 | G << 8 | G;
        r += rgb->d(); q += 4;
      }
    }
  }
  cairo_surface_t *surf = cairo_image_surface_create_for_data(BGRA, Fl_Cairo_Graphics_Driver::cairo_format, rgb->data_w(), rgb->data_h(), stride);
  if (cairo_surface_status(surf) != CAIRO_STATUS_SUCCESS) return;
  (void)cairo_surface_set_user_data(surf, &data_key_for_surface, BGRA, dealloc_surface_data);
  cairo_pattern_t *pat = cairo_pattern_create_for_surface(surf);
  cairo_surface_destroy(surf);
  *Fl_Graphics_Driver::id(rgb) = (fl_uintptr_t)pat;
  int *pw, *ph;
  cache_w_h(rgb, pw, ph);
  *pw = rgb->data_w();
  *ph = rgb->data_h();
}


void Fl_Cairo_Graphics_Driver::uncache(Fl_RGB_Image *img, fl_uintptr_t &id_, fl_uintptr_t &mask_) {
  cairo_pattern_t *pat = (cairo_pattern_t*)id_;
  if (pat) {
    cairo_pattern_destroy(pat);
    id_ = 0;
  }
}


void Fl_Cairo_Graphics_Driver::draw_fixed(Fl_Bitmap *bm,int XP, int YP, int WP, int HP,
                                          int cx, int cy) {
  cairo_pattern_t *pat = NULL;
  float s = wld_scale * scale();
  XP = Fl_Scalable_Graphics_Driver::floor(XP, s);
  YP = Fl_Scalable_Graphics_Driver::floor(YP, s);
  cache_size(bm, WP, HP);
  cx *= s; cy *= s;
  cairo_matrix_t matrix; // temporarily remove scaling
  cairo_get_matrix(cairo_, &matrix);
  cairo_translate(cairo_, -0.5, -0.5);
  cairo_scale(cairo_, 1./s, 1/s);
  cairo_translate(cairo_, 0.5, 0.5);
  if (!bm->array) {
    draw_empty(bm, XP, YP);
  } else {
    pat = (cairo_pattern_t*)*Fl_Graphics_Driver::id(bm);
    color(color());
    int old_w = bm->w(), old_h = bm->h();
    bm->scale(bm->cache_w(), bm->cache_h(), 0, 1); // transiently
    draw_cached_pattern_(bm, pat, XP, YP, WP, HP, cx, cy, bm->cache_w(), bm->cache_h());
    bm->scale(old_w, old_h, 0, 1); // back
  }
  cairo_set_matrix(cairo_, &matrix);
}


cairo_pattern_t *Fl_Cairo_Graphics_Driver::bitmap_to_pattern(Fl_Bitmap *bm,
                                    bool complement, cairo_surface_t **p_surface) {
  int stride = cairo_format_stride_for_width(CAIRO_FORMAT_A1, bm->data_w()); // 1.6
  int w_bitmap = ((bm->data_w() + 7) / 8);
  uchar *BGRA = new uchar[stride * bm->data_h()];
  memset(BGRA, 0, stride * bm->data_h());
  for (int j = 0; j < bm->data_h(); j++) {
    uchar *r = (uchar *)bm->array + j * w_bitmap;
    uint32_t *q = (uint32_t *)(BGRA + j * stride);
    int k = 0;
    uint32_t mask32;
    uchar p;
    for (int i = 0; i < bm->data_w(); i++) {
      if (k == 0) {
#if WORDS_BIGENDIAN
        mask32 = (uint32_t)(1 << 31);
#else
        mask32 = 1;
#endif
      }
      if ((k & 7) == 0) { // (k & 7) == (k % 8)
        p = *r++;
        if (complement)
          p = ~p;
      }
      if (p & 1)
        (*q) |= mask32;
      k++;
      p >>= 1;
#if WORDS_BIGENDIAN
      mask32 >>= 1;
#else
      mask32 <<= 1;
#endif
      if (k == 32) {
        k = 0;
        q++;
      }
    }
  }
  cairo_surface_t *surf = cairo_image_surface_create_for_data(BGRA, CAIRO_FORMAT_A1, bm->data_w(), bm->data_h(), stride);
  cairo_pattern_t *pattern = cairo_pattern_create_for_surface(surf);
  if (p_surface) *p_surface = surf;
  else cairo_surface_destroy(surf);
  return pattern;
}


void Fl_Cairo_Graphics_Driver::cache(Fl_Bitmap *bm) {
  cairo_surface_t *surf;
  cairo_pattern_t *pattern = Fl_Cairo_Graphics_Driver::bitmap_to_pattern(bm, false, &surf);
  uchar *BGRA = cairo_image_surface_get_data(surf); // 1.2
  (void)cairo_surface_set_user_data(surf, &data_key_for_surface, BGRA, dealloc_surface_data);
  cairo_surface_destroy(surf);
  *Fl_Graphics_Driver::id(bm) = (fl_uintptr_t)pattern;
  int *pw, *ph;
  cache_w_h(bm, pw, ph);
  *pw = bm->data_w();
  *ph = bm->data_h();
}


void Fl_Cairo_Graphics_Driver::draw_fixed(Fl_Pixmap *pxm,int XP, int YP, int WP, int HP,
                                          int cx, int cy) {
  cairo_pattern_t *pat = NULL;
  float s = wld_scale * scale();
  XP = Fl_Scalable_Graphics_Driver::floor(XP, s);
  YP = Fl_Scalable_Graphics_Driver::floor(YP, s);
  cache_size(pxm, WP, HP);
  cx *= s; cy *= s;
  cairo_matrix_t matrix; // temporarily remove scaling
  cairo_get_matrix(cairo_, &matrix);
  cairo_translate(cairo_, -0.5, -0.5);
  cairo_scale(cairo_, 1/s, 1/s);
  cairo_translate(cairo_, 0.5, 0.5);
  // Don't draw an empty image...
  if (!pxm->data() || !pxm->w()) {
    Fl_Graphics_Driver::draw_empty(pxm, XP, YP);
  } else {
    pat = (cairo_pattern_t*)*Fl_Graphics_Driver::id(pxm);
    int old_w = pxm->w(), old_h = pxm->h();
    pxm->scale(pxm->cache_w(), pxm->cache_h(), 0, 1); // transiently
    draw_cached_pattern_(pxm, pat, XP, YP, WP, HP, cx, cy, pxm->cache_w(), pxm->cache_h());
    pxm->scale(old_w, old_h, 0, 1); // back
  }
  cairo_set_matrix(cairo_, &matrix);
}


void Fl_Cairo_Graphics_Driver::cache(Fl_Pixmap *pxm) {
  Fl_RGB_Image *rgb = new Fl_RGB_Image(pxm);
  cache(rgb);
  *Fl_Graphics_Driver::id(pxm) = *Fl_Graphics_Driver::id(rgb);
  *Fl_Graphics_Driver::id(rgb) = 0;
  delete rgb;
  int *pw, *ph;
  cache_w_h(pxm, pw, ph);
  *pw = pxm->data_w();
  *ph = pxm->data_h();
}


void Fl_Cairo_Graphics_Driver::uncache_pixmap(fl_uintptr_t p) {
  if (p) cairo_pattern_destroy((cairo_pattern_t*)p);
}


void Fl_Cairo_Graphics_Driver::delete_bitmask(fl_uintptr_t bm) {
  if (bm) cairo_pattern_destroy((cairo_pattern_t*)bm);
}


int Fl_Cairo_Graphics_Driver::height() {
  if (!font_descriptor()) font(0, 12);
  return ceil(((Fl_Cairo_Font_Descriptor*)font_descriptor())->line_height / float(PANGO_SCALE));
}


int Fl_Cairo_Graphics_Driver::descent() {
  return font_descriptor()->descent / float(PANGO_SCALE) + 0.5;
}


void Fl_Cairo_Graphics_Driver::init_built_in_fonts() {
  static int i = 0;
  if (!i) {
    while (i < FL_FREE_FONT) {
      i++;
      Fl::set_font((Fl_Font)i-1, fl_fonts[i-1].name);
    }
  }
}


// FIXME: this (static) function should likely be in Fl_Graphics_Driver.
// The code is the same as in src/drivers/Xlib/Fl_Xlib_Graphics_Driver_font_xft.cxx

static int font_name_process(const char *name, char &face) {
  int l = strlen(name);
  face = ' ';
  if      (l >  8 && !memcmp(name + l -  8, " Regular", 8)) l -= 8;
  else if (l >  6 && !memcmp(name + l -  6, " Plain", 6)) l -= 6;
  else if (l > 12 && !memcmp(name + l - 12, " Bold Italic", 12)) {l -= 12; face = 'P';}
  else if (l >  7 && !memcmp(name + l -  7, " Italic", 7)) {l -= 7; face = 'I';}
  else if (l >  5 && !memcmp(name + l -  5, " Bold", 5)) {l -= 5; face = 'B';}
  return l;
}

typedef int (*sort_f_type)(const void *aa, const void *bb);


static int font_sort(Fl_Fontdesc *fa, Fl_Fontdesc *fb) {
  char face_a, face_b;
  int la = font_name_process(fa->name, face_a);
  int lb = font_name_process(fb->name, face_b);
  int c = strncasecmp(fa->name, fb->name, la >= lb ? lb : la);
  return (c == 0 ? face_a - face_b : c);
}


Fl_Font Fl_Cairo_Graphics_Driver::set_fonts(const char* /*pattern_name*/)
{
  // Return immideatly if the fonts were already counted
  if (font_count_ != -1)
    return FL_FREE_FONT + font_count_;
  fl_open_display();
  int n_families, count = 0;
  PangoFontFamily **families;
  char *saved_lang = fl_getenv("LANG");
  const char *Clang = "LANG=C";
  if (saved_lang && strcmp(saved_lang, Clang)) {
    // Force LANG=C to prevent pango_font_face_get_face_name() below from returning
    // translated versions of Bold, Italic, etc… (see issue #732).
    // Unfortunately, using setlocale() doesn't do the job.
    char *p = saved_lang;
    saved_lang = (char*)malloc(strlen(p) + 6);
    memcpy(saved_lang, "LANG=", 5);
    strcpy(saved_lang + 5, p);
    fl_putenv(Clang);
  } else saved_lang = NULL;
  static PangoFontMap *pfmap_ = pango_cairo_font_map_get_default(); // 1.10
  Fl_Cairo_Graphics_Driver::init_built_in_fonts();
  pango_font_map_list_families(pfmap_, &families, &n_families);
  for (int fam = 0; fam < n_families; fam++) {
    PangoFontFace **faces;
    int n_faces;
    const char *fam_name = pango_font_family_get_name (families[fam]);
    int lfam = strlen(fam_name);
    pango_font_family_list_faces(families[fam], &faces, &n_faces);
    for (int j = 0; j < n_faces; j++) {
      const char *p = pango_font_face_get_face_name(faces[j]);
      // Remove " Regular" suffix from font names
      if (!strcasecmp(p, "regular")) p = NULL;
      // build the font's FLTK name
      int lfont = lfam + (p ? strlen(p) + 2 : 1);
      char *q = new char[lfont];
      if (p) snprintf(q, lfont, "%s %s", fam_name, p);
      else strcpy(q, fam_name);
      Fl::set_font((Fl_Font)(count++ + FL_FREE_FONT), q);
    }
    /*g_*/free(faces); // glib source code shows that g_free is equivalent to free
  }
  /*g_*/free(families);
  if (saved_lang) {
    fl_putenv(saved_lang);
    free(saved_lang);
  }
  // Sort the list into alphabetic order
  qsort(fl_fonts + FL_FREE_FONT, count, sizeof(Fl_Fontdesc), (sort_f_type)font_sort);
  font_count_ = count;
  return FL_FREE_FONT + count;
}


const char *Fl_Cairo_Graphics_Driver::font_name(int num) {
  return fl_fonts[num].name;
}


void Fl_Cairo_Graphics_Driver::font_name(int num, const char *name) {
  Fl_Fontdesc *s = fl_fonts + num;
  if (s->name) {
    if (!strcmp(s->name, name)) {s->name = name; return;}
    for (Fl_Font_Descriptor* f = s->first; f;) {
      Fl_Font_Descriptor* n = f->next; delete f; f = n;
    }
    s->first = 0;
  }
  s->name = name;
  s->fontname[0] = 0;
  s->first = 0;
}


// turn a stored font name into a pretty name:
#define ENDOFBUFFER  sizeof(fl_fonts->fontname)-1

const char* Fl_Cairo_Graphics_Driver::get_font_name(Fl_Font fnum, int* ap) {
  Fl_Fontdesc *f = fl_fonts + fnum;
  if (!f->fontname[0]) {
    strcpy(f->fontname, f->name); // to check
    const char* thisFont = f->name;
    if (!thisFont || !*thisFont) {if (ap) *ap = 0; return "";}
    int type = 0;
    if (strstr(f->name, "Bold")) type |= FL_BOLD;
    if (strstr(f->name, "Italic") || strstr(f->name, "Oblique")) type |= FL_ITALIC;
    f->fontname[ENDOFBUFFER] = (char)type;
  }
  if (ap) *ap = f->fontname[ENDOFBUFFER];
  return f->fontname;
}


int Fl_Cairo_Graphics_Driver::get_font_sizes(Fl_Font fnum, int*& sizep) {
  static int array[128];
  if (!fl_fonts) fl_fonts = calc_fl_fonts();
  Fl_Fontdesc *s = fl_fonts+fnum;
  if (!s->name) s = fl_fonts; // empty slot in table, use entry 0
  int cnt = 0;

  array[0] = 0;
  sizep = array;
  cnt = 1;

  return cnt;
}


Fl_Cairo_Font_Descriptor::Fl_Cairo_Font_Descriptor(const char* name, Fl_Fontsize size,
                                                   PangoContext *context) :
                                                      Fl_Font_Descriptor(name, size) {
  char *string = new char[strlen(name) + 10];
  strcpy(string, name);
  snprintf(string + strlen(string), 10, " %dpx", size);
  //A PangoFontDescription describes a font in an implementation-independent manner.
  fontref = pango_font_description_from_string(string);
  delete[] string;
  width = NULL;
  //A PangoFontset represents a set of PangoFont to use when rendering text.
  PangoFontset *fontset = pango_font_map_load_fontset(
                                      pango_cairo_font_map_get_default(), // 1.10
                                      context, fontref,
                                      pango_language_get_default() // 1.16
                                                      );
  PangoFontMetrics *metrics = pango_fontset_get_metrics(fontset);
  ascent = pango_font_metrics_get_ascent(metrics);
  descent = pango_font_metrics_get_descent(metrics);
/* Function pango_font_metrics_get_height() giving the line height of a pango font
 appears with pango version 1.44. However, with pango version 1.48.10 and below,
 this function gives values that make the underscore invisible on lowdpi display
 and at 100% scaling (the underscore becomes visible starting at 120% scaling).
 With pango version 1.50.6 (Ubuntu 22.04) this problem disappears.
 Consequently, function pango_font_metrics_get_height() is not used until version 1.50.6
*/
//#if PANGO_VERSION_CHECK(1,44,0)
#if PANGO_VERSION_CHECK(1,50,6)
  line_height = pango_font_metrics_get_height(metrics); // 1.44
#else
  line_height = (pango_font_metrics_get_ascent(metrics) + pango_font_metrics_get_descent(metrics)) * 1.025 + 0.5;
#endif
  pango_font_metrics_unref(metrics);
  g_object_unref(fontset);
//fprintf(stderr, "[%s](%d) ascent=%d descent=%d line_height=%d\n", name, size, ascent, descent, line_height);
}


Fl_Cairo_Font_Descriptor::~Fl_Cairo_Font_Descriptor() {
  pango_font_description_free(fontref);
  if (width) {
    for (int i = 0; i < 64; i++) delete[] width[i];
  }
  delete[] width;
}


static Fl_Font_Descriptor* find(Fl_Font fnum, Fl_Fontsize size, PangoContext *context) {
  Fl_Fontdesc* s = fl_fonts+fnum;
  if (!s->name) s = fl_fonts; // use 0 if fnum undefined
  Fl_Font_Descriptor* f;
  for (f = s->first; f; f = f->next)
    if (f->size == size) return f;
  f = new Fl_Cairo_Font_Descriptor(s->name, size, context);
  f->next = s->first;
  s->first = f;
  return f;
}


/* Implementation note :
 * The pixel width of a drawn string equals the sum of the widths of its
 * characters, except when kerning occurs.
 */
void Fl_Cairo_Graphics_Driver::font(Fl_Font fnum, Fl_Fontsize s) {
  if (!font_descriptor()) fl_open_display();
  if (!cairo_) {
    cairo_surface_t *surf = cairo_image_surface_create(Fl_Cairo_Graphics_Driver::cairo_format, 100, 100);
    cairo_ = cairo_create(surf);
    cairo_surface_destroy(surf);
    dummy_cairo_ = cairo_;
  }
  if (s == 0) return;
  if (fnum == -1) {
    Fl_Graphics_Driver::font(0, 0);
    return;
  }
  Fl_Graphics_Driver::font(fnum, s);
  if (!pango_context_) {
    //A PangoFontMap represents the set of fonts available for a particular rendering system.
    PangoFontMap *def_font_map = pango_cairo_font_map_get_default(); // 1.10
    //A PangoContext stores global information used to control the itemization process.
#if PANGO_VERSION_CHECK(1,22,0)
    pango_context_ = pango_font_map_create_context(def_font_map); // 1.22
#else
    pango_context_ = pango_context_new();
    pango_context_set_font_map(pango_context_, def_font_map);
#endif
    pango_layout_ = pango_layout_new(pango_context_);
  }
  font_descriptor( find(fnum, s, pango_context_) );
  //If no font description is set on the layout, the font description from the layout’s context is used.
  pango_context_set_font_description(pango_context_,
                                  ((Fl_Cairo_Font_Descriptor*)font_descriptor())->fontref);
}


// Scans the input string str with fl_utf8decode() that, by default, accepts
// also non-UTF-8 and processes it as if encoded in CP1252.
// Returns a true UTF-8 string and its length, possibly transformed from CP1252.
// If the input string is true UTF-8, the returned string is the same pointer as input.
// Otherwise, the returned string is in private memory allocated inside clean_utf8()
// and extended when necessary.
const char *Fl_Cairo_Graphics_Driver::clean_utf8(const char* str, int &n) {
  static char *utf8_buffer = NULL;
  static int utf8_buffer_len = 0;
  char *q = utf8_buffer;
  const char *p = str;
  const char *retval = str;
  int len, len2;
  const char *end = str + n;
  char buf4[4];
  while (p < end) {
    unsigned codepoint = fl_utf8decode(p, end, &len);
    if (retval != str || (len == 1 &&  *(uchar*)p >= 0x80)) { // switch to using utf8_buffer
      len2 = fl_utf8encode(codepoint, buf4);
      if (!utf8_buffer_len || utf8_buffer_len < (q - utf8_buffer) + len2) {
        utf8_buffer_len += (q - utf8_buffer) + len2 + 1000;
        utf8_buffer = (char *)realloc(utf8_buffer, utf8_buffer_len);
      }
      if (retval == str) {
        retval = utf8_buffer;
        q = utf8_buffer;
        if (p > str) { memcpy(q, str, p - str); q += (p - str); }
      }
      memcpy(q, buf4, len2);
      q += len2;
    }
    p += len;
  }
  if (retval != str) n = q - retval;
  return retval;
}


void Fl_Cairo_Graphics_Driver::draw(const char* str, int n, float x, float y) {
  if (!n) return;
  cairo_save(cairo_);
  Fl_Cairo_Font_Descriptor *fd = (Fl_Cairo_Font_Descriptor*)font_descriptor();
  cairo_translate(cairo_, x - 1, y - (fd->line_height - fd->descent) / float(PANGO_SCALE) - 1);
  str = clean_utf8(str, n);
  pango_layout_set_text(pango_layout_, str, n);
  pango_cairo_show_layout(cairo_, pango_layout_); // 1.1O
  cairo_restore(cairo_);
  surface_needs_commit();
}


void Fl_Cairo_Graphics_Driver::draw(int rotation, const char *str, int n, int x, int y)
{
  cairo_save(cairo_);
  cairo_translate(cairo_, x, y);
  cairo_rotate(cairo_, -rotation * M_PI / 180);
  this->draw(str, n, 0, 0);
  cairo_restore(cairo_);
}


void Fl_Cairo_Graphics_Driver::rtl_draw(const char* str, int n, int x, int y) {
  int w = (int)width(str, n);
  draw(str, n, x - w, y);
}


// cache the widths of single Unicode characters
double Fl_Cairo_Graphics_Driver::width(unsigned int utf32) {
  unsigned r=0;
  Fl_Cairo_Font_Descriptor *desc = NULL;
  if (utf32 <= 0xFFFF) {
    desc = (Fl_Cairo_Font_Descriptor*)font_descriptor();
    r = (utf32 & 0xFC00) >> 10;
    if (!desc->width) {
      desc->width = (int**)new int*[64];
      memset(desc->width, 0, 64*sizeof(int*));
    }
    if (!desc->width[r]) {
      desc->width[r] = (int*)new int[0x0400];
      for (int i = 0; i < 0x0400; i++) desc->width[r][i] = -1;
    } else {
      if ( desc->width[r][utf32&0x03FF] >= 0 ) { // already cached
        return desc->width[r][utf32 & 0x03FF] / double(PANGO_SCALE);
      }
    }
  }
  char buf4[4];
  int n = fl_utf8encode(utf32, buf4);
  int width = do_width_unscaled_(buf4, n);
  if (utf32 <= 0xFFFF) {
    desc->width[r][utf32 & 0x03FF] = width;
  }
  return width / double(PANGO_SCALE);
}


double Fl_Cairo_Graphics_Driver::width(const char* str, int n) {
  if (!font_descriptor()) return -1;
  if ((str == NULL) || (n == 0)) return 0.;
  if (n == fl_utf8len(*str)) { // str contains a single unicode character
    int l;
    unsigned c = fl_utf8decode(str, str+n, &l);
    return width(c); // that character's width may have been cached
  }
  return do_width_unscaled_(str, n) / double(PANGO_SCALE); // full width computation for multi-char strings
}


int Fl_Cairo_Graphics_Driver::do_width_unscaled_(const char* str, int n) {
  if (!n) return 0;
  str = clean_utf8(str, n);
  pango_layout_set_text(pango_layout_, str, n);
  PangoRectangle p_rect;
  pango_layout_get_extents(pango_layout_, NULL, &p_rect);
  return p_rect.width;
}


void Fl_Cairo_Graphics_Driver::text_extents(const char* txt, int n, int& dx, int& dy, int& w, int& h) {
  txt = clean_utf8(txt, n);
  pango_layout_set_text(pango_layout_, txt, n);
  PangoRectangle ink_rect;
  pango_layout_get_extents(pango_layout_, &ink_rect, NULL);
  double f = PANGO_SCALE;
  Fl_Cairo_Font_Descriptor *fd = (Fl_Cairo_Font_Descriptor*)font_descriptor();
  dx = ink_rect.x / f - 1;
  dy = (ink_rect.y - fd->line_height + fd->descent) / f - 1;
  w = ceil(ink_rect.width / f);
  h = ceil(ink_rect.height / f);
}

//
// Region-handling member functions.
// They are used ONLY if the cairo graphics driver is the display graphics driver.
// They are not used if the cairo graphics driver is used to draw PostScript.
// Type cairo_region_t and associated functions require cairo ≥ 1.10

Fl_Region Fl_Cairo_Graphics_Driver::XRectangleRegion(int x, int y, int w, int h) {
  cairo_rectangle_int_t rect = {x, y, w, h};
  return cairo_region_create_rectangle(&rect); // 1.10
}


void Fl_Cairo_Graphics_Driver::add_rectangle_to_region(Fl_Region r_, int X, int Y, int W, int H) {
  cairo_rectangle_int_t rect = {X, Y, W, H};
  cairo_region_union_rectangle((cairo_region_t*)r_, &rect); // 1.10
}


void Fl_Cairo_Graphics_Driver::XDestroyRegion(Fl_Region r_) {
  cairo_region_destroy((cairo_region_t*)r_); // 1.10
}


void Fl_Cairo_Graphics_Driver::restore_clip() {
  if (cairo_) {
    cairo_reset_clip(cairo_);
    // apply what's in rstack
    cairo_region_t *r = (cairo_region_t*)rstack[rstackptr];
    if (r) {
      if (!clip_) {
        clip_ = new Clip();
        clip_->prev = NULL;
      }
      int count = cairo_region_num_rectangles(r); // 1.10
      cairo_rectangle_int_t rect;
      for (int i = 0; i < count; i++) {
        cairo_region_get_rectangle(r, i, &rect); // 1.10
        cairo_rectangle(cairo_, rect.x - 0.5, rect.y - 0.5, rect.width, rect.height);
      }
      // put in clip_ the bounding rect of region r
      cairo_region_get_extents(r, &rect); // 1.10
      clip_->x = rect.x;
      clip_->y = rect.y;
      clip_->w = rect.width;
      clip_->h = rect.height;
      cairo_set_antialias(cairo_, CAIRO_ANTIALIAS_NONE);
      cairo_clip(cairo_);
      cairo_set_antialias(cairo_, CAIRO_ANTIALIAS_DEFAULT );
    } else if (clip_) {
      clip_->w = -1;
    }
  }
}


char Fl_Cairo_Graphics_Driver::can_do_alpha_blending() {
  return 1;
}


float Fl_Cairo_Graphics_Driver::override_scale() {
  float s = scale();
  if (s != 1.f && Fl_Display_Device::display_device()->is_current()) {
    Fl::screen_driver()->scale(0, 1.f);
    cairo_scale(cairo_, 1/s, 1/s);
  }
  return s;
}


void Fl_Cairo_Graphics_Driver::restore_scale(float s) {
  if (s != 1.f && Fl_Display_Device::display_device()->is_current()) {
    Fl::screen_driver()->scale(0, s);
    cairo_scale(cairo_, s, s);
  }
}


void Fl_Cairo_Graphics_Driver::antialias(int state) {
  cairo_set_antialias(cairo_, state ? CAIRO_ANTIALIAS_DEFAULT : CAIRO_ANTIALIAS_NONE);
}


int Fl_Cairo_Graphics_Driver::antialias() {
  return (cairo_get_antialias(cairo_) != CAIRO_ANTIALIAS_NONE);
}


void Fl_Cairo_Graphics_Driver::focus_rect(int x, int y, int w, int h)
{
  cairo_save(cairo_);
  cairo_set_line_width(cairo_, 1);
  cairo_set_line_cap(cairo_, CAIRO_LINE_CAP_BUTT);
  cairo_set_line_join(cairo_, CAIRO_LINE_JOIN_MITER);
  static double dots[2] = {1., 1.};
  cairo_set_dash(cairo_, dots, 2, 1);
  cairo_set_antialias(cairo_,  CAIRO_ANTIALIAS_NONE);
  cairo_rectangle(cairo_, x, y, w-1, h-1);
  cairo_stroke(cairo_);
  cairo_restore(cairo_);
  surface_needs_commit();
}


cairo_pattern_t *Fl_Cairo_Graphics_Driver::calc_cairo_mask(const Fl_RGB_Image *rgb) {
  int i, j, d = rgb->d(), w = rgb->data_w(), h = rgb->data_h(), ld = rgb->ld();
  int bytesperrow = cairo_format_stride_for_width(CAIRO_FORMAT_A1, w); // 1.6
  if (!ld) ld = d * w;
  unsigned u;
  uchar byte, onebit;
  // build a CAIRO_FORMAT_A1 surface covering the non-black part of the image
  uchar* bits = new uchar[h*bytesperrow]; // to store the surface data
  for (i = 0; i < h; i++) {
    const uchar* alpha = (const uchar*)*rgb->data() + i * ld;
    uchar *p = (uchar*)bits + i * bytesperrow;
    byte = 0;
    onebit = 1;
    for (j = 0; j < w; j++) {
      u = *alpha;
      u += *(alpha+1);
      u += *(alpha+2);
      if (u > 0) { // if the pixel is not black
        byte |= onebit; // turn on the corresponding bit of the bitmap
      }
      onebit = onebit << 1; // move the single set bit one position to the left
      if (onebit == 0 || j == w-1) {
        onebit = 1;
        *p++ = byte; // store in bitmap one pack of bits
        byte = 0;
      }
      alpha += d; // point to next rgb pixel
    }
  }
  cairo_surface_t *mask_surf = cairo_image_surface_create_for_data(bits,
    CAIRO_FORMAT_A1, w, h, bytesperrow);
  cairo_pattern_t *mask_pattern = cairo_pattern_create_for_surface(mask_surf);
  cairo_surface_destroy(mask_surf);
  return mask_pattern;
}

#endif // USE_PANGO
