//
// "$Id$"
//
// Rectangle drawing routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2018 by Bill Spitzak and others.
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


#include "../../config_lib.h"
#include "Fl_Xlib_Graphics_Driver.H"
#include "Fl_Font.H"
#include <FL/fl_draw.H>
#include <FL/platform.H>

#include <string.h>

#if HAVE_XRENDER
#include <X11/extensions/Xrender.h>
#endif

extern XIC fl_xim_ic;
extern char fl_is_over_the_spot;
#if !USE_XFT
extern char *fl_get_font_xfld(int fnum, int size);
#endif

/*
 * By linking this module, the following static method will instantiate the
 * X11 Xlib Graphics driver as the main display driver.
 */
Fl_Graphics_Driver *Fl_Graphics_Driver::newMainGraphicsDriver()
{
  return new Fl_Xlib_Graphics_Driver();
}

GC Fl_Xlib_Graphics_Driver::gc_ = NULL;

/* Reference to the current graphics context
 For back-compatibility only. The preferred procedure to get this pointer is
 Fl_Surface_Device::surface()->driver()->gc().
 */
GC fl_gc = 0;

Fl_Xlib_Graphics_Driver::Fl_Xlib_Graphics_Driver(void) {
  mask_bitmap_ = NULL;
  p_size = 0;
  p = NULL;
  line_delta_ = 0;
#if USE_PANGO
  pfd_ = pango_font_description_new();
  Fl_Graphics_Driver::font(0, 0);
#endif
  offset_x_ = 0; offset_y_ = 0;
  depth_ = 0;
}

Fl_Xlib_Graphics_Driver::~Fl_Xlib_Graphics_Driver() {
  if (p) free(p);
#if USE_PANGO
  pango_font_description_free(pfd_);
#endif
}


void Fl_Xlib_Graphics_Driver::gc(void *value) {
  gc_ = (GC)value;
  fl_gc = gc_;
}

void Fl_Xlib_Graphics_Driver::scale(float f) {
#if USE_XFT
  if (f != scale_) {
    size_ = 0;
    scale_ = f;
    //fprintf(stderr, "scale=%.2f\n", scale_);
    line_style(FL_SOLID); // scale also default line width
    /* Scaling >= 2 transforms 1-pixel wide lines into wider lines.
     X11 draws 2-pixel wide lines so that half of the line width is above or at left
     of a 1-pixel wide line that would be drawn with the same coordinates.
     Thus, if the line starts at coordinates (0,0) half of the line width is invisible.
     Similarly, if the line ends at w()-1 the last pixel of the window is not drawn.
     What is wanted when scale_ == 2 is a visible 2-pixel wide line in the first case,
     and a line at the window's edge in the 2nd case.
     Setting line_delta_ to 1 and offsetting all line, rectangle, text and clip
     coordinates by line_delta_ achieves what is wanted until scale_ <= 3.5.
     */
    line_delta_ =  (scale_ > 1.75 ? 1 : 0);
  }
#endif
}

void Fl_Xlib_Graphics_Driver::copy_offscreen(int x, int y, int w, int h, Fl_Offscreen pixmap, int srcx, int srcy) {
  XCopyArea(fl_display, pixmap, fl_window, gc_, srcx*scale_, srcy*scale_, w*scale_, h*scale_, (x+offset_x_)*scale_, (y+offset_y_)*scale_);

}

void Fl_Xlib_Graphics_Driver::add_rectangle_to_region(Fl_Region r, int X, int Y, int W, int H) {
  XRectangle R;
  R.x = X; R.y = Y; R.width = W; R.height = H;
  XUnionRectWithRegion(&R, r, r);
}

void Fl_Xlib_Graphics_Driver::transformed_vertex0(float fx, float fy) {
  short x = short(fx), y = short(fy);
  if (!n || x != p[n-1].x || y != p[n-1].y) {
    if (n >= p_size) {
      p_size = p ? 2*p_size : 16;
      p = (XPOINT*)realloc((void*)p, p_size*sizeof(*p));
    }
    p[n].x = x;
    p[n].y = y;
    n++;
  }
}

void Fl_Xlib_Graphics_Driver::fixloop() {  // remove equal points from closed path
  while (n>2 && p[n-1].x == p[0].x && p[n-1].y == p[0].y) n--;
}

// FIXME: should be members of Fl_Xlib_Graphics_Driver
XRectangle fl_spot;
int fl_spotf = -1;
int fl_spots = -1;

void Fl_Xlib_Graphics_Driver::reset_spot(void)
{
  fl_spot.x = -1;
  fl_spot.y = -1;
  //if (fl_xim_ic) XUnsetICFocus(fl_xim_ic);
}

void Fl_Xlib_Graphics_Driver::set_spot(int font, int size, int X, int Y, int W, int H, Fl_Window *win)
{
  int change = 0;
  XVaNestedList preedit_attr;
  static XFontSet fs = NULL;
  char **missing_list;
  int missing_count;
  char *def_string;
  char *fnt = NULL;
  bool must_free_fnt =true;

  static XIC ic = NULL;

  if (!fl_xim_ic || !fl_is_over_the_spot) return;
  //XSetICFocus(fl_xim_ic);
  if (X != fl_spot.x || Y != fl_spot.y) {
    fl_spot.x = X;
    fl_spot.y = Y;
    fl_spot.height = H;
    fl_spot.width = W;
    change = 1;
  }
  if (font != fl_spotf || size != fl_spots) {
    fl_spotf = font;
    fl_spots = size;
    change = 1;
    if (fs) {
      XFreeFontSet(fl_display, fs);
    }
#if USE_XFT

#if defined(__GNUC__)
    // FIXME: warning XFT support here
#endif /*__GNUC__*/

    fnt = NULL; // fl_get_font_xfld(font, size);
    if (!fnt) {fnt = (char*)"-misc-fixed-*";must_free_fnt=false;}
    fs = XCreateFontSet(fl_display, fnt, &missing_list,
                        &missing_count, &def_string);
#else
    fnt = fl_get_font_xfld(font, size);
    if (!fnt) {fnt = (char*)"-misc-fixed-*";must_free_fnt=false;}
    fs = XCreateFontSet(fl_display, fnt, &missing_list,
                        &missing_count, &def_string);
#endif
  }
  if (fl_xim_ic != ic) {
    ic = fl_xim_ic;
    change = 1;
  }

  if (fnt && must_free_fnt) free(fnt);
  if (!change) return;


  preedit_attr = XVaCreateNestedList(0,
                                     XNSpotLocation, &fl_spot,
                                     XNFontSet, fs, NULL);
  XSetICValues(fl_xim_ic, XNPreeditAttributes, preedit_attr, NULL);
  XFree(preedit_attr);
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


Region Fl_Xlib_Graphics_Driver::scale_clip(float f) {
  Region r = rstack[rstackptr];
  if (r == 0 || (f == 1 && offset_x_ == 0 && offset_y_ == 0) ) return 0;
  int deltaf = f/2;
  Region r2 = XCreateRegion();
  for (int i = 0; i < r->numRects; i++) {
    int x = (r->rects[i].x1 + offset_x_)*f;
    int y = (r->rects[i].y1 + offset_y_)*f;
    int w = int((r->rects[i].x2 + offset_x_) * f) - x;
    int h = int((r->rects[i].y2 + offset_y_) * f) - y;
    x += line_delta_ - deltaf;
    y += line_delta_ - deltaf;
    Region R = XRectangleRegion(x, y, w, h);
    XUnionRegion(R, r2, r2);
    ::XDestroyRegion(R);
  }
  rstack[rstackptr] = r2;
  return r;
}


void Fl_Xlib_Graphics_Driver::translate_all(int dx, int dy) { // reversibly adds dx,dy to the offset between user and graphical coordinates
  stack_x_[depth_] = offset_x_;
  stack_y_[depth_] = offset_y_;
  offset_x_ = stack_x_[depth_] + dx;
  offset_y_ = stack_y_[depth_] + dy;
  push_matrix();
  translate(dx, dy);
  if (depth_ < sizeof(stack_x_)/sizeof(int)) depth_++;
  else Fl::warning("%s: translate stack overflow!", "Fl_Xlib_Graphics_Driver");
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

//
// End of "$Id$".
//
