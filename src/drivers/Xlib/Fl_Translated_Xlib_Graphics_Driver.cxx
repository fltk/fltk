//
// "$Id: Fl_Translated_Xlib_Graphics_Driver.cxx 11217 2016-02-25 17:56:48Z manolo $"
//
// Draw-to-image code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2016 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

#include <config.h>
#include "Fl_Translated_Xlib_Graphics_Driver.H"
#include <FL/Fl.H>

#ifndef FL_DOXYGEN

Fl_Translated_Xlib_Graphics_Driver::Fl_Translated_Xlib_Graphics_Driver() {
  offset_x = 0; offset_y = 0;
  depth = 0;
}

Fl_Translated_Xlib_Graphics_Driver::~Fl_Translated_Xlib_Graphics_Driver() {}

void Fl_Translated_Xlib_Graphics_Driver::translate_all(int dx, int dy) { // reversibly adds dx,dy to the offset between user and graphical coordinates
  stack_x[depth] = offset_x;
  stack_y[depth] = offset_y;
  offset_x = stack_x[depth] + dx;
  offset_y = stack_y[depth] + dy;
  push_matrix();
  translate(dx, dy);
  if (depth < sizeof(stack_x)/sizeof(int)) depth++;
  else Fl::warning("%s: translate stack overflow!", "Fl_Translated_Xlib_Graphics_Driver");
}

void Fl_Translated_Xlib_Graphics_Driver::untranslate_all() { // undoes previous translate_all()
  if (depth > 0) depth--;
  offset_x = stack_x[depth];
  offset_y = stack_y[depth];
  pop_matrix();
}

void Fl_Translated_Xlib_Graphics_Driver::rect(int x, int y, int w, int h) {
  Fl_Xlib_Graphics_Driver::rect(x+offset_x, y+offset_y, w, h);
}

void Fl_Translated_Xlib_Graphics_Driver::rectf(int x, int y, int w, int h) {
  Fl_Xlib_Graphics_Driver::rectf(x+offset_x, y+offset_y, w, h);
}

void Fl_Translated_Xlib_Graphics_Driver::xyline(int x, int y, int x1) {
  Fl_Xlib_Graphics_Driver::xyline(x+offset_x, y+offset_y, x1+offset_x);
}

void Fl_Translated_Xlib_Graphics_Driver::xyline(int x, int y, int x1, int y2) {
  Fl_Xlib_Graphics_Driver::xyline(x+offset_x, y+offset_y, x1+offset_x, y2+offset_y);
}

void Fl_Translated_Xlib_Graphics_Driver::xyline(int x, int y, int x1, int y2, int x3) {
  Fl_Xlib_Graphics_Driver::xyline(x+offset_x, y+offset_y, x1+offset_x, y2+offset_y, x3+offset_x);
}

void Fl_Translated_Xlib_Graphics_Driver::yxline(int x, int y, int y1) {
  Fl_Xlib_Graphics_Driver::yxline(x+offset_x, y+offset_y, y1+offset_y);
}

void Fl_Translated_Xlib_Graphics_Driver::yxline(int x, int y, int y1, int x2) {
  Fl_Xlib_Graphics_Driver::yxline(x+offset_x, y+offset_y, y1+offset_y, x2+offset_x);
}

void Fl_Translated_Xlib_Graphics_Driver::yxline(int x, int y, int y1, int x2, int y3) {
  Fl_Xlib_Graphics_Driver::yxline(x+offset_x, y+offset_y, y1+offset_y, x2+offset_x, y3+offset_y);
}

void Fl_Translated_Xlib_Graphics_Driver::line(int x, int y, int x1, int y1) {
  Fl_Xlib_Graphics_Driver::line(x+offset_x, y+offset_y, x1+offset_x, y1+offset_y);
}

void Fl_Translated_Xlib_Graphics_Driver::line(int x, int y, int x1, int y1, int x2, int y2) {
  Fl_Xlib_Graphics_Driver::line(x+offset_x, y+offset_y, x1+offset_x, y1+offset_y, x2+offset_x, y2+offset_y);
}

void Fl_Translated_Xlib_Graphics_Driver::draw(const char* str, int n, int x, int y) {
  Fl_Xlib_Graphics_Driver::draw(str, n, x+offset_x, y+offset_y);
}

void Fl_Translated_Xlib_Graphics_Driver::draw(int angle, const char *str, int n, int x, int y) {
  Fl_Xlib_Graphics_Driver::draw(angle, str, n, x+offset_x, y+offset_y);
}

void Fl_Translated_Xlib_Graphics_Driver::rtl_draw(const char* str, int n, int x, int y) {
  Fl_Xlib_Graphics_Driver::rtl_draw(str, n, x+offset_x, y+offset_y);
}

void Fl_Translated_Xlib_Graphics_Driver::draw(Fl_Pixmap *pxm, int XP, int YP, int WP, int HP, int cx, int cy) {
  XP += offset_x; YP += offset_y;
  translate_all(-offset_x, -offset_y);
  Fl_Xlib_Graphics_Driver::draw(pxm, XP, YP, WP,HP,cx,cy);
  untranslate_all();
}

void Fl_Translated_Xlib_Graphics_Driver::draw(Fl_Bitmap *bm, int XP, int YP, int WP, int HP, int cx, int cy) {
  XP += offset_x; YP += offset_y;
  translate_all(-offset_x, -offset_y);
  Fl_Xlib_Graphics_Driver::draw(bm, XP, YP, WP,HP,cx,cy);
  untranslate_all();
}

void Fl_Translated_Xlib_Graphics_Driver::draw(Fl_RGB_Image *img, int XP, int YP, int WP, int HP, int cx, int cy) {
  XP += offset_x; YP += offset_y;
  translate_all(-offset_x, -offset_y);
  Fl_Xlib_Graphics_Driver::draw(img, XP, YP, WP,HP,cx,cy);
  untranslate_all();
}

void Fl_Translated_Xlib_Graphics_Driver::draw_image(const uchar* buf, int X,int Y,int W,int H, int D, int L) {
  X += offset_x; Y += offset_y;
  translate_all(-offset_x, -offset_y);
  Fl_Xlib_Graphics_Driver::draw_image(buf, X, Y, W,H,D,L);
  untranslate_all();
}

void Fl_Translated_Xlib_Graphics_Driver::draw_image(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D) {
  X += offset_x; Y += offset_y;
  translate_all(-offset_x, -offset_y);
  Fl_Xlib_Graphics_Driver::draw_image(cb, data, X, Y, W,H,D);
  untranslate_all();
}

void Fl_Translated_Xlib_Graphics_Driver::draw_image_mono(const uchar* buf, int X,int Y,int W,int H, int D, int L) {
  X += offset_x; Y += offset_y;
  translate_all(-offset_x, -offset_y);
  Fl_Xlib_Graphics_Driver::draw_image_mono(buf, X, Y, W,H,D,L);
  untranslate_all();
}

void Fl_Translated_Xlib_Graphics_Driver::draw_image_mono(Fl_Draw_Image_Cb cb, void* data, int X,int Y,int W,int H, int D) {
  X += offset_x; Y += offset_y;
  translate_all(-offset_x, -offset_y);
  Fl_Xlib_Graphics_Driver::draw_image_mono(cb, data, X, Y, W,H,D);
  untranslate_all();
}

void Fl_Translated_Xlib_Graphics_Driver::copy_offscreen(int x, int y, int w, int h, Fl_Offscreen pixmap, int srcx, int srcy) {
  Fl_Xlib_Graphics_Driver::copy_offscreen(x+offset_x, y+offset_y, w, h,pixmap,srcx,srcy);
}

void Fl_Translated_Xlib_Graphics_Driver::push_clip(int x, int y, int w, int h) {
  Fl_Xlib_Graphics_Driver::push_clip(x+offset_x, y+offset_y, w, h);
}

int Fl_Translated_Xlib_Graphics_Driver::not_clipped(int x, int y, int w, int h) {
  return Fl_Xlib_Graphics_Driver::not_clipped(x + offset_x, y + offset_y, w, h);
}

int Fl_Translated_Xlib_Graphics_Driver::clip_box(int x, int y, int w, int h, int& X, int& Y, int& W, int& H) {
  int retval = Fl_Xlib_Graphics_Driver::clip_box(x + offset_x, y + offset_y, w,h,X,Y,W,H);
  X -= offset_x;
  Y -= offset_y;
  return retval;
}

void Fl_Translated_Xlib_Graphics_Driver::pie(int x, int y, int w, int h, double a1, double a2) {
  Fl_Xlib_Graphics_Driver::pie(x+offset_x,y+offset_y,w,h,a1,a2);
}

void Fl_Translated_Xlib_Graphics_Driver::arc(int x, int y, int w, int h, double a1, double a2) {
  Fl_Xlib_Graphics_Driver::arc(x+offset_x,y+offset_y,w,h,a1,a2);
}

void Fl_Translated_Xlib_Graphics_Driver::polygon(int x0, int y0, int x1, int y1, int x2, int y2) {
  Fl_Xlib_Graphics_Driver::polygon(x0+offset_x,y0+offset_y,x1+offset_x,y1+offset_y,x2+offset_x,y2+offset_y);
}

void Fl_Translated_Xlib_Graphics_Driver::polygon(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) {
  Fl_Xlib_Graphics_Driver::polygon(x0+offset_x,y0+offset_y,x1+offset_x,y1+offset_y,x2+offset_x,y2+offset_y,x3+offset_x,y3+offset_y);
}

void Fl_Translated_Xlib_Graphics_Driver::loop(int x0, int y0, int x1, int y1, int x2, int y2) {
  Fl_Xlib_Graphics_Driver::loop(x0+offset_x,y0+offset_y,x1+offset_x,y1+offset_y,x2+offset_x,y2+offset_y);
}

void Fl_Translated_Xlib_Graphics_Driver::loop(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) {
  Fl_Xlib_Graphics_Driver::loop(x0+offset_x,y0+offset_y,x1+offset_x,y1+offset_y,x2+offset_x,y2+offset_y,x3+offset_x,y3+offset_y);
}

void Fl_Translated_Xlib_Graphics_Driver::point(int x, int y) {
  Fl_Xlib_Graphics_Driver::point(x+offset_x, y+offset_y);
}

#endif // FL_DOXYGEN

//
// End of "$Id: Fl_Translated_Xlib_Graphics_Driver.cxx 11217 2016-02-25 17:56:48Z manolo $".
//

