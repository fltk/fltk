//
// Rectangle drawing routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2025 by Bill Spitzak and others.
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


#include <config.h>
#include "Fl_Xlib_Graphics_Driver.H"
#include "Fl_Font.H"
#include <FL/fl_draw.H>
#include <FL/platform.H>

#include <string.h>
#include <stdlib.h>


GC Fl_Xlib_Graphics_Driver::gc_ = NULL;
int Fl_Xlib_Graphics_Driver::fl_overlay = 0;

/* Reference to the current graphics context
 For back-compatibility only. The preferred procedure to get this pointer is
 Fl_Surface_Device::surface()->driver()->gc().
 */
GC fl_gc = 0;

GC fl_x11_gc() { return fl_gc; }

Fl_Xlib_Graphics_Driver::Fl_Xlib_Graphics_Driver(void) {
  mask_bitmap_ = NULL;
  short_point = NULL;
#if USE_PANGO
  Fl_Graphics_Driver::font(0, 0);
#endif
  offset_x_ = 0; offset_y_ = 0;
  depth_ = 0;
  clip_max_ = 32760; // clipping limit (2**15 - 8)
}

Fl_Xlib_Graphics_Driver::~Fl_Xlib_Graphics_Driver() {
  if (short_point) free(short_point);
}


void Fl_Xlib_Graphics_Driver::gc(void *value) {
  gc_ = (GC)value;
  fl_gc = gc_;
}

void Fl_Xlib_Graphics_Driver::scale(float f) {
#if USE_XFT || FLTK_USE_CAIRO
  if (f != scale()) {
    size_ = 0;
    Fl_Graphics_Driver::scale(f);
    //fprintf(stderr, "scale=%.2f\n", scale_);
    line_style(FL_SOLID); // scale also default line width
  }
#endif
}

void Fl_Xlib_Graphics_Driver::copy_offscreen(int x, int y, int w, int h, Fl_Offscreen pixmap, int srcx, int srcy) {
  XCopyArea(fl_display, (Pixmap)pixmap, fl_window, gc_, srcx*scale(), srcy*scale(), w*scale(), h*scale(), (x+offset_x_)*scale(), (y+offset_y_)*scale());

}

void Fl_Xlib_Graphics_Driver::add_rectangle_to_region(Fl_Region r, int X, int Y, int W, int H) {
  XRectangle R;
  R.x = X; R.y = Y; R.width = W; R.height = H;
  XUnionRectWithRegion(&R, (Region)r, (Region)r);
}

void Fl_Xlib_Graphics_Driver::transformed_vertex0(float fx, float fy) {
  short x = short(fx), y = short(fy);
  if (!n || x != short_point[n-1].x || y != short_point[n-1].y) {
    if (n >= p_size) {
      p_size = short_point ? 2*p_size : 16;
      short_point = (XPoint*)realloc((void*)short_point, p_size*sizeof(*short_point));
    }
    short_point[n].x = x ;
    short_point[n].y = y ;
    n++;
  }
}

void Fl_Xlib_Graphics_Driver::fixloop() {  // remove equal points from closed path
  while (n>2 && short_point[n-1].x == short_point[0].x && short_point[n-1].y == short_point[0].y) n--;
}

#if !USE_XFT
unsigned Fl_Xlib_Graphics_Driver::font_desc_size() {
  return (unsigned)sizeof(Fl_Xlib_Fontdesc);
}
#endif

const char *Fl_Xlib_Graphics_Driver::font_name(int num) {
#if USE_XFT
  return fl_fonts[num].name;
#else
  return ((Fl_Xlib_Fontdesc*)fl_fonts)[num].name;
#endif
}

void Fl_Xlib_Graphics_Driver::font_name(int num, const char *name) {
#if USE_XFT
#  if USE_PANGO
  init_built_in_fonts();
  if (pfd_array_length > num && pfd_array[num]) {
    pango_font_description_free(pfd_array[num]);
    pfd_array[num] = NULL;
  }
#  endif
  Fl_Fontdesc *s = fl_fonts + num;
#else
    Fl_Xlib_Fontdesc *s = ((Fl_Xlib_Fontdesc*)fl_fonts) + num;
#endif
  if (s->name) {
    if (!strcmp(s->name, name)) {s->name = name; return;}
#if !USE_XFT
    if (s->xlist && s->n >= 0) XFreeFontNames(s->xlist);
#endif
    for (Fl_Font_Descriptor* f = s->first; f;) {
      Fl_Font_Descriptor* n = f->next; delete f; f = n;
    }
    s->first = 0;
  }
  s->name = name;
  s->fontname[0] = 0;
#if !USE_XFT
  s->xlist = 0;
#endif
  s->first = 0;
}


Fl_Region Fl_Xlib_Graphics_Driver::scale_clip(float f) {
  Region r = (Region)rstack[rstackptr];
  if (r == 0 || (f == 1 && offset_x_ == 0 && offset_y_ == 0) ) return 0;
  Region r2 = XCreateRegion();
  for (int i = 0; i < r->numRects; i++) {
    int x = floor(r->rects[i].x1 + offset_x_, f);
    int y = floor(r->rects[i].y1 + offset_y_, f);
    int w = floor((r->rects[i].x2 + offset_x_) , f) - x;
    int h = floor((r->rects[i].y2 + offset_y_) , f) - y;
    Region R = (Region)XRectangleRegion(x, y, w, h);
    XUnionRegion(R, r2, r2);
    ::XDestroyRegion(R);
  }
  rstack[rstackptr] = r2;
  return r;
}


void Fl_Xlib_Graphics_Driver::translate_all(int dx, int dy) { // reversibly adds dx,dy to the offset between user and graphical coordinates
  if (depth_ < FL_XLIB_GRAPHICS_TRANSLATION_STACK_SIZE) {
    stack_x_[depth_] = offset_x_;
    stack_y_[depth_] = offset_y_;
    depth_++;
  } else {
    Fl::warning("%s: translate stack overflow!", "Fl_Xlib_Graphics_Driver");
  }
  offset_x_ += dx;
  offset_y_ += dy;
  push_matrix();
  translate(dx, dy);
}

void Fl_Xlib_Graphics_Driver::untranslate_all() { // undoes previous translate_all()
  if (depth_ > 0) depth_--;
  offset_x_ = stack_x_[depth_];
  offset_y_ = stack_y_[depth_];
  pop_matrix();
}

void Fl_Xlib_Graphics_Driver::set_current_() {
  restore_clip();
}

#if USE_PANGO
int Fl_Xlib_Graphics_Driver::pfd_array_length = FL_FREE_FONT;

PangoFontDescription **Fl_Xlib_Graphics_Driver::pfd_array = (PangoFontDescription**)calloc(pfd_array_length, sizeof(PangoFontDescription*));
#endif
