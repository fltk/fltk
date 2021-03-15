//
// Support for Cairo graphics for the Fast Light Tool Kit (FLTK).
//
// Copyright 2021 by Bill Spitzak and others.
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
#include <FL/fl_draw.H>
#include <cairo/cairo.h>
#include <pango/pangocairo.h>
#include <math.h>

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
  curdata = cb_data->data + x*cb_data->D + y*cb_data->LD;
  for (; x<last; x++) {
    memcpy(buf, curdata, abs(cb_data->D));
    buf += abs(cb_data->D);
    curdata += cb_data->D;
  }
}


Fl_Cairo_Graphics_Driver::Fl_Cairo_Graphics_Driver() : Fl_Graphics_Driver() {}

Fl_Cairo_Graphics_Driver::~Fl_Cairo_Graphics_Driver() {}


void Fl_Cairo_Graphics_Driver::rectf(int x, int y, int w, int h) {
  cairo_rectangle(cairo_, x, y, w, h);
  cairo_fill(cairo_);
  check_status();
}

void Fl_Cairo_Graphics_Driver::rect(int x, int y, int w, int h) {
  cairo_rectangle(cairo_, x, y, w, h);
  cairo_stroke(cairo_);
  check_status();
}

void Fl_Cairo_Graphics_Driver::line(int x1, int y1, int x2, int y2) {
  cairo_new_path(cairo_);
  cairo_move_to(cairo_, x1, y1);
  cairo_line_to(cairo_, x2, y2);
  cairo_stroke(cairo_);
}

void Fl_Cairo_Graphics_Driver::line(int x0, int y0, int x1, int y1, int x2, int y2) {
  cairo_new_path(cairo_);
  cairo_move_to(cairo_, x0, y0);
  cairo_line_to(cairo_, x1, y1);
  cairo_line_to(cairo_, x2, y2);
  cairo_stroke(cairo_);
}

void Fl_Cairo_Graphics_Driver::xyline(int x, int y, int x1) {
  cairo_move_to(cairo_, x, y);
  cairo_line_to(cairo_, x1, y);
  cairo_stroke(cairo_);
  check_status();
}

void Fl_Cairo_Graphics_Driver::xyline(int x, int y, int x1, int y2) {
  cairo_move_to(cairo_, x, y);
  cairo_line_to(cairo_, x1, y);
  cairo_line_to(cairo_, x1, y2);
  cairo_stroke(cairo_);
  check_status();
}

void Fl_Cairo_Graphics_Driver::xyline(int x, int y, int x1, int y2, int x3) {
  cairo_move_to(cairo_, x, y);
  cairo_line_to(cairo_, x1, y);
  cairo_line_to(cairo_, x1, y2);
  cairo_line_to(cairo_, x3, y2);
  cairo_stroke(cairo_);
  check_status();
}

void Fl_Cairo_Graphics_Driver::yxline(int x, int y, int y1) {
  cairo_move_to(cairo_, x, y);
  cairo_line_to(cairo_, x, y1);
  cairo_stroke(cairo_);
  check_status();
}

void Fl_Cairo_Graphics_Driver::yxline(int x, int y, int y1, int x2) {
  cairo_move_to(cairo_, x, y);
  cairo_line_to(cairo_, x, y1);
  cairo_line_to(cairo_, x2, y1);
  cairo_stroke(cairo_);
  check_status();
}

void Fl_Cairo_Graphics_Driver::yxline(int x, int y, int y1, int x2, int y3) {
  cairo_move_to(cairo_, x, y);
  cairo_line_to(cairo_, x, y1);
  cairo_line_to(cairo_, x2, y1);
  cairo_line_to(cairo_, x2, y3);
  cairo_stroke(cairo_);
  check_status();
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
}

void Fl_Cairo_Graphics_Driver::line_style(int style, int width, char* dashes) {
  linewidth_=width;
  linestyle_=style;
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

void Fl_Cairo_Graphics_Driver::draw(int rotation, const char *str, int n, int x, int y)
{
  cairo_save(cairo_);
  cairo_translate(cairo_, x, y);
  cairo_rotate(cairo_, -rotation * M_PI / 180);
  this->transformed_draw(str, n, 0, 0);
  cairo_restore(cairo_);
}

void Fl_Cairo_Graphics_Driver::transformed_draw(const char* str, int n, double x, double y) {
  if (!n) return;
  pango_layout_set_font_description(pango_layout_, pango_font_description(Fl_Graphics_Driver::font()));
  int pwidth, pheight;
  cairo_save(cairo_);
  pango_layout_set_text(pango_layout_, str, n);
  pango_layout_get_size(pango_layout_, &pwidth, &pheight);
  if (pwidth > 0) {
    double s = width(str, n);
    cairo_translate(cairo_, x, y - height() + descent());
    s = (s/pwidth) * PANGO_SCALE;
    cairo_scale(cairo_, s, s);
    pango_cairo_show_layout(cairo_, pango_layout_);
  }
  cairo_restore(cairo_);
  check_status();
}

void Fl_Cairo_Graphics_Driver::rtl_draw(const char* str, int n, int x, int y) {
  int w = (int)width(str, n);
  transformed_draw(str, n, x - w, y);
}

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
  if(shape_==POINTS){
    cairo_move_to(cairo_, x, y);
    gap_=1;
    return;
  }
  if(gap_){
    cairo_move_to(cairo_, x, y);
    gap_=0;
  }else
    cairo_line_to(cairo_, x, y);
}

void Fl_Cairo_Graphics_Driver::curve(double x, double y, double x1, double y1, double x2, double y2, double x3, double y3)
{
  if(shape_==NONE) return;
  if(gap_)
    cairo_move_to(cairo_, x, y);
  else
    cairo_line_to(cairo_, x, y);
  gap_=0;
  cairo_curve_to(cairo_, x1 , y1 , x2 , y2 , x3 , y3);
}

void Fl_Cairo_Graphics_Driver::circle(double x, double y, double r){
  if (shape_==NONE){
    cairo_save(cairo_);
    concat();
    cairo_arc(cairo_, x, y, r, 0, 2*M_PI);
    reconcat();
    cairo_restore(cairo_);
  } else
    cairo_arc(cairo_, x, y, r, 0, 2*M_PI);
}

void Fl_Cairo_Graphics_Driver::arc(double x, double y, double r, double start, double a){
  if (shape_==NONE) return;
  gap_ = 0;
  if(start > a)
    cairo_arc(cairo_, x, y, r, -start*M_PI/180, -a*M_PI/180);
  else
    cairo_arc_negative(cairo_, x, y, r, -start*M_PI/180, -a*M_PI/180);
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
}

void Fl_Cairo_Graphics_Driver::end_points() {
  end_line();
}

void Fl_Cairo_Graphics_Driver::end_line() {
  gap_=1;
  reconcat();
  cairo_stroke(cairo_);
  cairo_restore(cairo_);
  shape_=NONE;
}

void Fl_Cairo_Graphics_Driver::end_loop(){
  gap_=1;
  reconcat();
  cairo_close_path(cairo_);
  cairo_stroke(cairo_);
  cairo_restore(cairo_);
  shape_=NONE;
}

void Fl_Cairo_Graphics_Driver::end_polygon() {
  gap_=1;
  reconcat();
  cairo_close_path(cairo_);
  cairo_fill(cairo_);
  cairo_restore(cairo_);
  shape_=NONE;
}

void Fl_Cairo_Graphics_Driver::transformed_vertex(double x, double y) {
  reconcat();
  if(gap_){
    cairo_move_to(cairo_, x, y);
    gap_=0;
  }else
    cairo_line_to(cairo_, x, y);
  concat();
}

void Fl_Cairo_Graphics_Driver::push_clip(int x, int y, int w, int h) {
  Clip * c=new Clip();
  clip_box(x,y,w,h,c->x,c->y,c->w,c->h);
  c->prev=clip_;
  clip_=c;
  cairo_save(cairo_);
  cairo_rectangle(cairo_, clip_->x-0.5 , clip_->y-0.5 , clip_->w  , clip_->h);
  cairo_clip(cairo_);
  check_status();
}

void Fl_Cairo_Graphics_Driver::push_no_clip() {
  Clip * c = new Clip();
  c->prev=clip_;
  clip_=c;
  clip_->x = clip_->y = clip_->w = clip_->h = -1;
  cairo_save(cairo_);
  cairo_reset_clip(cairo_);
  check_status();
}

void Fl_Cairo_Graphics_Driver::pop_clip() {
  if(!clip_)return;
  Clip * c=clip_;
  clip_=clip_->prev;
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
}

void Fl_Cairo_Graphics_Driver::draw_image_mono(const uchar *data, int ix, int iy, int iw, int ih, int D, int LD)
{
  struct callback_data cb_data;
  if (!LD) LD = iw*abs(D);
  if (D<0) data += iw*abs(D);
  cb_data.data = data;
  cb_data.D = D;
  cb_data.LD = LD;
  draw_image(draw_image_cb, &cb_data, ix, iy, iw, ih, abs(D));
}

void Fl_Cairo_Graphics_Driver::draw_image_mono(Fl_Draw_Image_Cb call, void *data, int ix, int iy, int iw, int ih, int D)
{
  draw_image(call, data, ix, iy, iw, ih, D);
}

static void destroy_BGRA(void *data) {
  delete[] (uchar*)data;
}

void Fl_Cairo_Graphics_Driver::draw_pixmap(Fl_Pixmap *pxm,int XP, int YP, int WP, int HP, int cx, int cy) {
  Fl_RGB_Image *rgb =  new Fl_RGB_Image(pxm);
  draw_rgb_bitmap_(rgb, XP, YP, WP, HP, cx, cy);
  delete rgb;
}

void Fl_Cairo_Graphics_Driver::draw_rgb(Fl_RGB_Image *rgb,int XP, int YP, int WP, int HP, int cx, int cy) {
  draw_rgb_bitmap_(rgb, XP, YP, WP, HP, cx, cy);
}

void Fl_Cairo_Graphics_Driver::draw_bitmap(Fl_Bitmap *bitmap,int XP, int YP, int WP, int HP, int cx, int cy) {
  draw_rgb_bitmap_(bitmap, XP, YP, WP, HP, cx, cy);
}

void Fl_Cairo_Graphics_Driver::draw_rgb_bitmap_(Fl_Image *img,int XP, int YP, int WP, int HP, int cx, int cy)
{
  cairo_surface_t *surf;
  cairo_format_t format = (img->d() >= 1 ? CAIRO_FORMAT_ARGB32 : CAIRO_FORMAT_A1);
  int stride = cairo_format_stride_for_width(format, img->data_w());
  uchar *BGRA = new uchar[stride * img->data_h()];
  memset(BGRA, 0, stride * img->data_h());
  if (img->d() >= 1) { // process Fl_RGB_Image of all depths
    Fl_RGB_Image *rgb = (Fl_RGB_Image*)img;
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
  } else {
    Fl_Bitmap *bm = (Fl_Bitmap*)img;
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
  }
  surf = cairo_image_surface_create_for_data(BGRA, format, img->data_w(), img->data_h(), stride);
  if (cairo_surface_status(surf) == CAIRO_STATUS_SUCCESS) {
    static cairo_user_data_key_t key = {};
    (void)cairo_surface_set_user_data(surf, &key, BGRA, destroy_BGRA);
    cairo_pattern_t *pat = cairo_pattern_create_for_surface(surf);
    cairo_save(cairo_);
    cairo_rectangle(cairo_, XP-0.5, YP-0.5, WP+1, HP+1);
    cairo_clip(cairo_);
    if (img->d() >= 1) cairo_set_source(cairo_, pat);
    cairo_matrix_t matrix;
    cairo_matrix_init_scale(&matrix, double(img->data_w())/(img->w()+1), double(img->data_h())/(img->h()+1));
    cairo_matrix_translate(&matrix, -XP+0.5+cx, -YP+0.5+cy);
    cairo_pattern_set_matrix(pat, &matrix);
    cairo_mask(cairo_, pat);
    cairo_pattern_destroy(pat);
    cairo_surface_destroy(surf);
    cairo_restore(cairo_);
    check_status();
  }
}

#endif // USE_PANGO
