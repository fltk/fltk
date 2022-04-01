//
// Support for Cairo graphics for the Fast Light Tool Kit (FLTK).
//
// Copyright 2021-2022 by Bill Spitzak and others.
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
#include <FL/platform.H>
#include <FL/fl_draw.H>
#include <cairo/cairo.h>
#include <pango/pangocairo.h>
#include <math.h>
#include <stdlib.h>  // abs(int)
#include <string.h>  // memcpy()


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


Fl_Cairo_Graphics_Driver::Fl_Cairo_Graphics_Driver() : Fl_Graphics_Driver() {
  cairo_ = NULL;
  pango_layout_ = NULL;
  dummy_pango_layout_ = NULL;
  linestyle_ = FL_SOLID;
  clip_ = NULL;
  scale_x = scale_y = 1;
  angle = 0;
  left_margin = top_margin = 0;
  needs_commit_tag_ = NULL;
}

Fl_Cairo_Graphics_Driver::~Fl_Cairo_Graphics_Driver() {
  if (pango_layout_) g_object_unref(pango_layout_);
}

const cairo_format_t Fl_Cairo_Graphics_Driver::cairo_format = CAIRO_FORMAT_ARGB32;


void Fl_Cairo_Graphics_Driver::rectf(int x, int y, int w, int h) {
  cairo_rectangle(cairo_, x-0.5, y-0.5, w, h);
  cairo_fill(cairo_);
  check_status();
  surface_needs_commit();
}

void Fl_Cairo_Graphics_Driver::rect(int x, int y, int w, int h) {
  cairo_rectangle(cairo_, x, y, w-1, h-1);
  cairo_stroke(cairo_);
  check_status();
  surface_needs_commit();
}

void Fl_Cairo_Graphics_Driver::line(int x1, int y1, int x2, int y2) {
  cairo_new_path(cairo_);
  cairo_move_to(cairo_, x1, y1);
  cairo_line_to(cairo_, x2, y2);
  cairo_stroke(cairo_);
  surface_needs_commit();
}

void Fl_Cairo_Graphics_Driver::line(int x0, int y0, int x1, int y1, int x2, int y2) {
  cairo_new_path(cairo_);
  cairo_move_to(cairo_, x0, y0);
  cairo_line_to(cairo_, x1, y1);
  cairo_line_to(cairo_, x2, y2);
  cairo_stroke(cairo_);
  surface_needs_commit();
}

void Fl_Cairo_Graphics_Driver::xyline(int x, int y, int x1) {
  cairo_move_to(cairo_, x, y);
  cairo_line_to(cairo_, x1, y);
  cairo_stroke(cairo_);
  check_status();
  surface_needs_commit();
}

void Fl_Cairo_Graphics_Driver::xyline(int x, int y, int x1, int y2) {
  cairo_move_to(cairo_, x, y);
  cairo_line_to(cairo_, x1, y);
  cairo_line_to(cairo_, x1, y2);
  cairo_stroke(cairo_);
  check_status();
  surface_needs_commit();
}

void Fl_Cairo_Graphics_Driver::xyline(int x, int y, int x1, int y2, int x3) {
  cairo_move_to(cairo_, x, y);
  cairo_line_to(cairo_, x1, y);
  cairo_line_to(cairo_, x1, y2);
  cairo_line_to(cairo_, x3, y2);
  cairo_stroke(cairo_);
  check_status();
  surface_needs_commit();
}

void Fl_Cairo_Graphics_Driver::yxline(int x, int y, int y1) {
  cairo_move_to(cairo_, x, y);
  cairo_line_to(cairo_, x, y1);
  cairo_stroke(cairo_);
  check_status();
  surface_needs_commit();
}

void Fl_Cairo_Graphics_Driver::yxline(int x, int y, int y1, int x2) {
  cairo_move_to(cairo_, x, y);
  cairo_line_to(cairo_, x, y1);
  cairo_line_to(cairo_, x2, y1);
  cairo_stroke(cairo_);
  check_status();
  surface_needs_commit();
}

void Fl_Cairo_Graphics_Driver::yxline(int x, int y, int y1, int x2, int y3) {
  cairo_move_to(cairo_, x, y);
  cairo_line_to(cairo_, x, y1);
  cairo_line_to(cairo_, x2, y1);
  cairo_line_to(cairo_, x2, y3);
  cairo_stroke(cairo_);
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

void Fl_Cairo_Graphics_Driver::color(Fl_Color c) {
  Fl::get_color(c, cr_, cg_, cb_);
  Fl_Cairo_Graphics_Driver::color(cr_, cg_, cb_);
}

Fl_Color Fl_Cairo_Graphics_Driver::color() { return Fl_Graphics_Driver::color(); }


void Fl_Cairo_Graphics_Driver::concat(){
  cairo_matrix_t mat = {fl_matrix->a , fl_matrix->b , fl_matrix->c , fl_matrix->d , fl_matrix->x , fl_matrix->y};
  cairo_transform(cairo_, &mat);
}

void Fl_Cairo_Graphics_Driver::reconcat(){
  cairo_matrix_t mat = {fl_matrix->a , fl_matrix->b , fl_matrix->c , fl_matrix->d , fl_matrix->x , fl_matrix->y};
  cairo_status_t stat = cairo_matrix_invert(&mat);
  if (stat != CAIRO_STATUS_SUCCESS) {
    fputs("error in cairo_matrix_invert\n", stderr);
  }
  cairo_transform(cairo_, &mat);
}

void Fl_Cairo_Graphics_Driver::begin_points() {
  cairo_save(cairo_);
  concat();
  cairo_new_path(cairo_);
  gap_=1;
  shape_=POINTS;
}

void Fl_Cairo_Graphics_Driver::begin_line() {
  cairo_save(cairo_);
  concat();
  cairo_new_path(cairo_);
  gap_=1;
  shape_=LINE;
}

void Fl_Cairo_Graphics_Driver::begin_loop() {
  cairo_save(cairo_);
  concat();
  cairo_new_path(cairo_);
  gap_=1;
  shape_=LOOP;
}

void Fl_Cairo_Graphics_Driver::begin_polygon() {
  cairo_save(cairo_);
  concat();
  cairo_new_path(cairo_);
  gap_=1;
  shape_=POLYGON;
}

void Fl_Cairo_Graphics_Driver::vertex(double x, double y) {
  if (shape_==POINTS){
    cairo_move_to(cairo_, x, y);
    gap_=1;
    return;
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
  if(shape_==NONE) return;
  if (shape_ == POINTS) Fl_Graphics_Driver::curve(x, y, x1, y1, x2, y2, x3, y3);
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
  if (shape_ == NONE) {
    cairo_save(cairo_);
    concat();
    cairo_arc(cairo_, x, y, r, 0, 2*M_PI);
    reconcat();
    cairo_restore(cairo_);
  } else {
    cairo_arc(cairo_, x, y, r, 0, 2*M_PI);
  }
  surface_needs_commit();
}

void Fl_Cairo_Graphics_Driver::arc(double x, double y, double r, double start, double a){
  if (shape_ == NONE) return;
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
  cairo_scale(cairo_, (w-1)/2.0 , (h-1)/2.0);
  vertex(0,0);
  arc(0.0,0.0, 1, a2, a1);
  end_polygon();
  cairo_restore(cairo_);
  surface_needs_commit();
}

void Fl_Cairo_Graphics_Driver::end_points() {
  end_line();
}

void Fl_Cairo_Graphics_Driver::end_line() {
  gap_ = 1;
  reconcat();
  cairo_stroke(cairo_);
  cairo_restore(cairo_);
  shape_ = NONE;
  surface_needs_commit();
}

void Fl_Cairo_Graphics_Driver::end_loop(){
  gap_ = 1;
  reconcat();
  cairo_close_path(cairo_);
  cairo_stroke(cairo_);
  cairo_restore(cairo_);
  shape_ = NONE;
  surface_needs_commit();
}

void Fl_Cairo_Graphics_Driver::end_polygon() {
  gap_ = 1;
  reconcat();
  cairo_close_path(cairo_);
  cairo_fill(cairo_);
  cairo_restore(cairo_);
  shape_ = NONE;
  surface_needs_commit();
}

void Fl_Cairo_Graphics_Driver::transformed_vertex(double x, double y) {
  if (shape_ == POINTS) {
    cairo_move_to(cairo_, x, y);
    point(x, y);
    gap_ = 1;
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
  cairo_rectangle(cairo_, clip_->x-0.5 , clip_->y-0.5 , clip_->w  , clip_->h);
  cairo_clip(cairo_);
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
  if (D<0) data += iw * aD;
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
  if (D<0) data += iw*abs(D);
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


void Fl_Cairo_Graphics_Driver::draw_cached_pattern_(Fl_Image *img, cairo_pattern_t *pat, int X, int Y, int W, int H, int cx, int cy) {
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
  if (cx || cy || W < img->w() || H < img->h()) { // clip when necessary
    cairo_rectangle(cairo_, X-0.5, Y-0.5, W+1, H+1);
    cairo_clip(cairo_);
  }
  // remove any scaling and the current "0.5" translation useful for lines but bad for images
  matrix.xx = matrix.yy = 1;
  matrix.x0 -= 0.5 * s; matrix.y0 -= 0.5 * s;
  cairo_set_matrix(cairo_, &matrix);
  if (img->d() >= 1) cairo_set_source(cairo_, pat);
  int offset = 0;
  if (Ws >= img->data_w()*1.09 || Hs >= img->data_h()*1.09) {
    // When enlarging while drawing, 1 pixel around target area seems unpainted,
    // so we increase a bit the target area and move it int(s) pixels to left and top.
    Ws = (img->w()+2)*s, Hs = (img->h()+2)*s;
    offset = int(s);
  }

//fprintf(stderr,"WHs=%dx%d dataWH=%dx%d s=%.1f offset=%d\n",Ws,Hs,img->data_w(),img->data_h(),s,offset);
  cairo_matrix_init_scale(&matrix, double(img->data_w())/Ws, double(img->data_h())/Hs);
  cairo_matrix_translate(&matrix, -Xs + offset, -Ys + offset);
  cairo_pattern_set_matrix(pat, &matrix);
  cairo_mask(cairo_, pat);
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
  draw_cached_pattern_(rgb, pat, X, Y, W, H, cx, cy);
}


static cairo_user_data_key_t data_key_for_surface = {};

static void dealloc_surface_data(void *data) {
  delete[] (uchar*)data;
}


void Fl_Cairo_Graphics_Driver::cache(Fl_RGB_Image *rgb) {
  int stride = cairo_format_stride_for_width(Fl_Cairo_Graphics_Driver::cairo_format, rgb->data_w());
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
        *q =  B * f;
        *(q+1) =  G * f;
        *(q+2) =  R * f;
        *(q+3) =  A;
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
        *(q) =  G * f;
        *(q+1) =  G * f;
        *(q+2) =  G * f;
        *(q+3) =  A;
        r += rgb->d(); q += 4;
      }
    }
  }
  cairo_surface_t *surf = cairo_image_surface_create_for_data(BGRA, Fl_Cairo_Graphics_Driver::cairo_format, rgb->data_w(), rgb->data_h(), stride);
  if (cairo_surface_status(surf) != CAIRO_STATUS_SUCCESS) return;
  (void)cairo_surface_set_user_data(surf, &data_key_for_surface, BGRA, dealloc_surface_data);
  cairo_pattern_t *pat = cairo_pattern_create_for_surface(surf);
  *Fl_Graphics_Driver::id(rgb) = (fl_uintptr_t)pat;
}


void Fl_Cairo_Graphics_Driver::uncache(Fl_RGB_Image *img, fl_uintptr_t &id_, fl_uintptr_t &mask_) {
  cairo_pattern_t *pat = (cairo_pattern_t*)id_;
  if (pat) {
    cairo_surface_t *surf;
    cairo_pattern_get_surface(pat, &surf);
    cairo_pattern_destroy(pat);
    cairo_surface_destroy(surf);
    id_ = 0;
  }
}


void Fl_Cairo_Graphics_Driver::draw_bitmap(Fl_Bitmap *bm,int XP, int YP, int WP, int HP, int cx, int cy) {
  int X, Y, W, H;
  if (!bm->array) {
    draw_empty(bm, XP, YP);
    return;
  }
  if (start_image(bm, XP,YP,WP,HP,cx,cy,X,Y,W,H)) return;
  cairo_pattern_t *pat = (cairo_pattern_t*)*Fl_Graphics_Driver::id(bm);
  if (!pat) {
    cache(bm);
    pat = (cairo_pattern_t*)*Fl_Graphics_Driver::id(bm);
  }
  if (pat) {
    draw_cached_pattern_(bm, pat, X, Y, W, H, cx, cy);
  }
}


void Fl_Cairo_Graphics_Driver::cache(Fl_Bitmap *bm) {
  int stride = cairo_format_stride_for_width(CAIRO_FORMAT_A1, bm->data_w());
  uchar *BGRA = new uchar[stride * bm->data_h()];
  memset(BGRA, 0, stride * bm->data_h());
    uchar  *r, p;
    unsigned *q;
    for (int j = 0; j < bm->data_h(); j++) {
      r = (uchar*)bm->array + j * ((bm->data_w() + 7)/8);
      q = (unsigned*)(BGRA + j * stride);
      unsigned k = 0, mask32 = 1;
      p = *r;
      for (int i = 0; i < bm->data_w(); i++) {
        if (p&1) (*q) |= mask32;
        k++;
        if (k % 8 != 0) p >>= 1; else p = *(++r);
        if (k % 32 != 0) mask32 <<= 1; else {q++; mask32 = 1;}
      }
    }
  cairo_surface_t *surf = cairo_image_surface_create_for_data(BGRA, CAIRO_FORMAT_A1, bm->data_w(), bm->data_h(), stride);
  if (cairo_surface_status(surf) == CAIRO_STATUS_SUCCESS) {
    (void)cairo_surface_set_user_data(surf, &data_key_for_surface, BGRA, dealloc_surface_data);
    cairo_pattern_t *pat = cairo_pattern_create_for_surface(surf);
    *Fl_Graphics_Driver::id(bm) = (fl_uintptr_t)pat;
  }
}


void Fl_Cairo_Graphics_Driver::draw_pixmap(Fl_Pixmap *pxm,int XP, int YP, int WP, int HP, int cx, int cy) {
  int X, Y, W, H;
  // Don't draw an empty image...
  if (!pxm->data() || !pxm->w()) {
    Fl_Graphics_Driver::draw_empty(pxm, XP, YP);
    return;
  }
  if (start_image(pxm, XP, YP, WP, HP, cx, cy, X, Y, W, H)) {
    return;
  }
  cairo_pattern_t *pat = (cairo_pattern_t*)*Fl_Graphics_Driver::id(pxm);
  if (!pat) {
    cache(pxm);
    pat = (cairo_pattern_t*)*Fl_Graphics_Driver::id(pxm);
  }
  draw_cached_pattern_(pxm, pat, X, Y, W, H, cx, cy);
}


void Fl_Cairo_Graphics_Driver::cache(Fl_Pixmap *pxm) {
  Fl_RGB_Image *rgb = new Fl_RGB_Image(pxm);
  cache(rgb);
  *Fl_Graphics_Driver::id(pxm) = *Fl_Graphics_Driver::id(rgb);
  *Fl_Graphics_Driver::id(rgb) = 0;
  delete rgb;
}


void Fl_Cairo_Graphics_Driver::uncache_pixmap(fl_uintptr_t p) {
  cairo_pattern_t *pat = (cairo_pattern_t*)p;
  if (pat) {
    cairo_surface_t *surf;
    cairo_pattern_get_surface(pat, &surf);
    cairo_pattern_destroy(pat);
    cairo_surface_destroy(surf);
  }
}


void Fl_Cairo_Graphics_Driver::delete_bitmask(fl_uintptr_t bm) {
  cairo_pattern_t *pat = (cairo_pattern_t*)bm;
  if (pat) {
    cairo_surface_t *surf;
    cairo_pattern_get_surface(pat, &surf);
    cairo_pattern_destroy(pat);
    cairo_surface_destroy(surf);
  }
}


int Fl_Cairo_Graphics_Driver::height() {
  if (!font_descriptor()) font(0, 12);
  return (font_descriptor()->ascent + font_descriptor()->descent)*1.1  /*1.15 scale=1*/;
}


int Fl_Cairo_Graphics_Driver::descent() {
  return font_descriptor()->descent;
}


extern Fl_Fontdesc *fl_fonts;


void Fl_Cairo_Graphics_Driver::init_built_in_fonts() {
  static int i = 0;
  if (!i) {
    while (i < FL_FREE_FONT) {
      i++;
      Fl::set_font((Fl_Font)i-1, fl_fonts[i-1].name);
    }
  }
}


static int font_name_process(const char *name, char &face) {
  int l = strlen(name);
  face = ' ';
  if (!memcmp(name + l - 8, " Regular", 8)) l -= 8;
  else if (!memcmp(name + l - 6, " Plain", 6)) l -= 6;
  else if (!memcmp(name + l - 12, " Bold Italic", 12)) {l -= 12; face='P';}
  else if (!memcmp(name + l - 7, " Italic", 7)) {l -= 7; face='I';}
  else if (!memcmp(name + l - 5, " Bold", 5)) {l -= 5; face='B';}
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
  fl_open_display();
  int n_families, count = 0;
  PangoFontFamily **families;
  static PangoFontMap *pfmap_ = pango_cairo_font_map_get_default(); // 1.10
  Fl_Cairo_Graphics_Driver::init_built_in_fonts();
  pango_font_map_list_families(pfmap_, &families, &n_families);
  for (int fam = 0; fam < n_families; fam++) {
    PangoFontFace **faces;
    int n_faces;
    const char *fam_name = pango_font_family_get_name (families[fam]);
    int l = strlen(fam_name);
    pango_font_family_list_faces(families[fam], &faces, &n_faces);
    for (int j = 0; j < n_faces; j++) {
      const char *p = pango_font_face_get_face_name(faces[j]);
      // build the font's FLTK name
      l += strlen(p) + 2;
      char *q = new char[l];
      sprintf(q, "%s %s", fam_name, p);
      Fl::set_font((Fl_Font)(count++ + FL_FREE_FONT), q);
    }
    /*g_*/free(faces); // glib source code shows that g_free is equivalent to free
  }
  /*g_*/free(families);
  // Sort the list into alphabetic order
  qsort(fl_fonts + FL_FREE_FONT, count, sizeof(Fl_Fontdesc), (sort_f_type)font_sort);
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


Fl_Cairo_Font_Descriptor::Fl_Cairo_Font_Descriptor(const char* name, Fl_Fontsize size) : Fl_Font_Descriptor(name, size) {
  char string[70];
  strcpy(string, name);
  sprintf(string + strlen(string), " %d", int(size * 0.7 + 0.5) ); // why reduce size?
  fontref = pango_font_description_from_string(string);
  width = NULL;
  static PangoFontMap *def_font_map = pango_cairo_font_map_get_default(); // 1.10
  static PangoContext *pango_context = pango_font_map_create_context(def_font_map); // 1.22
  static PangoLanguage *language = pango_language_get_default(); // 1.16
  PangoFontset *fontset = pango_font_map_load_fontset(def_font_map, pango_context, fontref, language);
  PangoFontMetrics *metrics = pango_fontset_get_metrics(fontset);
  ascent = pango_font_metrics_get_ascent(metrics)/PANGO_SCALE;
  descent = pango_font_metrics_get_descent(metrics)/PANGO_SCALE;
  q_width = pango_font_metrics_get_approximate_char_width(metrics)/PANGO_SCALE;
  pango_font_metrics_unref(metrics);
  g_object_unref(fontset);
//fprintf(stderr, "[%s](%d) ascent=%d descent=%d q_width=%d\n", name, size, ascent, descent, q_width);
}


Fl_Cairo_Font_Descriptor::~Fl_Cairo_Font_Descriptor() {
  pango_font_description_free(fontref);
  if (width) {
    for (int i = 0; i < 64; i++) delete[] width[i];
  }
  delete[] width;
}


static Fl_Font_Descriptor* find(Fl_Font fnum, Fl_Fontsize size) {
  Fl_Fontdesc* s = fl_fonts+fnum;
  if (!s->name) s = fl_fonts; // use 0 if fnum undefined
  Fl_Font_Descriptor* f;
  for (f = s->first; f; f = f->next)
    if (f->size == size) return f;
  f = new Fl_Cairo_Font_Descriptor(s->name, size);
  f->next = s->first;
  s->first = f;
  return f;
}


void Fl_Cairo_Graphics_Driver::font(Fl_Font fnum, Fl_Fontsize s) {
  if (!font_descriptor()) fl_open_display();
  if (!pango_layout_) {
    bool needs_dummy = (cairo_ == NULL);
    if (!cairo_) {
      cairo_surface_t *surf = cairo_image_surface_create(Fl_Cairo_Graphics_Driver::cairo_format, 100, 100);
      cairo_ = cairo_create(surf);
    }
    pango_layout_ = pango_cairo_create_layout(cairo_);
    if (needs_dummy) dummy_pango_layout_ = pango_layout_;
  }
  if (s == 0) return;
  if (font() == fnum && size() == s) return;
  if (fnum == -1) {
    Fl_Graphics_Driver::font(0, 0);
    return;
  }
  Fl_Graphics_Driver::font(fnum, s);
  font_descriptor( find(fnum, s) );
  pango_layout_set_font_description(pango_layout_, ((Fl_Cairo_Font_Descriptor*)font_descriptor())->fontref);
}


void Fl_Cairo_Graphics_Driver::draw(const char* str, int n, float x, float y) {
  if (!n) return;
  cairo_save(cairo_);
  cairo_translate(cairo_, x, y - height() + descent() -1);
  pango_layout_set_text(pango_layout_, str, n);
  pango_cairo_show_layout(cairo_, pango_layout_);
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


double Fl_Cairo_Graphics_Driver::width(const char* c, int n) {
  if (!font_descriptor()) return -1.0;
  int i = 0, w = 0, l;
  const char *end = c + n;
  unsigned int ucs;
  while (i < n) {
    ucs = fl_utf8decode(c + i, end, &l);
    i += l;
    w += width(ucs);
  }
  return (double)w;
}


double Fl_Cairo_Graphics_Driver::width(unsigned int c) {
  unsigned int r = 0;
  Fl_Cairo_Font_Descriptor *desc = NULL;
  if (c <= 0xFFFF) { // when inside basic multilingual plane
    desc = (Fl_Cairo_Font_Descriptor*)font_descriptor();
    r = (c & 0xFC00) >> 10;
    if (!desc->width) {
      desc->width = (int**)new int*[64];
      memset(desc->width, 0, 64*sizeof(int*));
    }
    if (!desc->width[r]) {
      desc->width[r] = (int*)new int[0x0400];
      for (int i = 0; i < 0x0400; i++) desc->width[r][i] = -1;
    } else {
      if ( desc->width[r][c & 0x03FF] >= 0 ) { // already cached
        return (double) desc->width[r][c & 0x03FF];
      }
    }
  }
  char buf[4];
  int n = fl_utf8encode(c, buf);
  pango_layout_set_text(pango_layout_, buf, n);
  int  W = 0, H;
  pango_layout_get_pixel_size(pango_layout_, &W, &H);
  if (c <= 0xFFFF) desc->width[r][c & 0x03FF] = W;
  return (double)W;
}


void Fl_Cairo_Graphics_Driver::text_extents(const char* txt, int n, int& dx, int& dy, int& w, int& h) {
  pango_layout_set_text(pango_layout_, txt, n);
  PangoRectangle ink_rect;
  pango_layout_get_pixel_extents(pango_layout_, &ink_rect, NULL);
  dx = ink_rect.x;
  dy = ink_rect.y - height() + descent();
  w = ink_rect.width;
  h = ink_rect.height;
}

//
// Region-handling member functions.
// They are used ONLY if the cairo graphics driver is the display graphics driver.
// They are not used if the cairo graphics driver is used to draw PostScript.
//

Fl_Region Fl_Cairo_Graphics_Driver::XRectangleRegion(int x, int y, int w, int h) {
  struct flCairoRegion *R = (struct flCairoRegion*)malloc(sizeof(*R));
  R->count = 1;
  R->rects = (cairo_rectangle_t *)malloc(sizeof(cairo_rectangle_t));
  R->rects->x=x, R->rects->y=y, R->rects->width=w; R->rects->height=h;
  return (Fl_Region)R;
}


// r1 ⊂ r2
static bool CairoRectContainsRect(cairo_rectangle_t *r1, cairo_rectangle_t *r2) {
  return r1->x >= r2->x && r1->y >= r2->y && r1->x+r1->width <= r2->x+r2->width &&
    r1->y+r1->height <= r2->y+r2->height;
}


void Fl_Cairo_Graphics_Driver::add_rectangle_to_region(Fl_Region r_, int X, int Y, int W, int H) {
  struct flCairoRegion *r = (struct flCairoRegion*)r_;
  cairo_rectangle_t arg = {double(X), double(Y), double(W), double(H)};
  int j; // don't add a rectangle totally inside the Fl_Region
  for (j = 0; j < r->count; j++) {
    if (CairoRectContainsRect(&arg, &(r->rects[j]))) break;
  }
  if (j >= r->count) {
    r->rects = (cairo_rectangle_t*)realloc(r->rects, (++(r->count)) * sizeof(cairo_rectangle_t));
    r->rects[r->count - 1] = arg;
  }
}


void Fl_Cairo_Graphics_Driver::XDestroyRegion(Fl_Region r_) {
  if (r_) {
    struct flCairoRegion *r = (struct flCairoRegion*)r_;
    free(r->rects);
    free(r);
  }
}


void Fl_Cairo_Graphics_Driver::restore_clip() {
  if (cairo_) {
    cairo_reset_clip(cairo_);
    // apply what's in rstack
    struct flCairoRegion *r = (struct flCairoRegion*)rstack[rstackptr];
    if (r) {
      for (int i = 0; i < r->count; i++) {
        cairo_rectangle(cairo_, r->rects[i].x - 0.5, r->rects[i].y - 0.5, r->rects[i].width, r->rects[i].height);
      }
      cairo_clip(cairo_);
    }
  }
}

#endif // USE_PANGO
