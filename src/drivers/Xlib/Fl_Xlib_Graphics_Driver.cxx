//
// "$Id$"
//
// Rectangle drawing routines for the Fast Light Tool Kit (FLTK).
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


#include "../../config_lib.h"
#include "Fl_Xlib_Graphics_Driver.H"
#include <FL/fl_draw.H>
#include <FL/x.H>

#include <string.h>

#if HAVE_XRENDER
#include <X11/extensions/Xrender.h>
#endif

/* Reference to the current graphics context
 For back-compatibility only. The preferred procedure to get this pointer is
 Fl_Surface_Device::surface()->driver()->gc().
 */
GC fl_gc = 0;

void Fl_Graphics_Driver::global_gc()
{
}


/*
 * By linking this module, the following static method will instatiate the
 * X11 Xlib Graphics driver as the main display driver.
 */
Fl_Graphics_Driver *Fl_Graphics_Driver::newMainGraphicsDriver()
{
  return new Fl_Xlib_Graphics_Driver();
}

GC Fl_Xlib_Graphics_Driver::gc_ = NULL;

Fl_Xlib_Graphics_Driver::Fl_Xlib_Graphics_Driver(void) {
  mask_bitmap_ = NULL;
  p_size = 0;
  p = NULL;
}

void Fl_Xlib_Graphics_Driver::gc(void *value) {
  gc_ = (GC)value;
  fl_gc = gc_;
}


char Fl_Xlib_Graphics_Driver::can_do_alpha_blending() {
  return Fl_X::xrender_supported();
}


void Fl_Xlib_Graphics_Driver::copy_offscreen(int x, int y, int w, int h, Fl_Offscreen pixmap, int srcx, int srcy) {
  XCopyArea(fl_display, pixmap, fl_window, gc_, srcx, srcy, w, h, x, y);
}

#ifndef FL_DOXYGEN
void Fl_Xlib_Graphics_Driver::copy_offscreen_with_alpha(int x, int y, int w, int h,
                                                        Fl_Offscreen pixmap, int srcx, int srcy) {
#if HAVE_XRENDER
  XRenderPictureAttributes srcattr;
  memset(&srcattr, 0, sizeof(XRenderPictureAttributes));
  static XRenderPictFormat *srcfmt = XRenderFindStandardFormat(fl_display, PictStandardARGB32);
  static XRenderPictFormat *dstfmt = XRenderFindStandardFormat(fl_display, PictStandardRGB24);

  Picture src = XRenderCreatePicture(fl_display, pixmap, srcfmt, 0, &srcattr);
  Picture dst = XRenderCreatePicture(fl_display, fl_window, dstfmt, 0, &srcattr);

  if (!src || !dst) {
    fprintf(stderr, "Failed to create Render pictures (%lu %lu)\n", src, dst);
    return;
  }

  const Fl_Region clipr = fl_clip_region();
  if (clipr)
    XRenderSetPictureClipRegion(fl_display, dst, clipr);

  XRenderComposite(fl_display, PictOpOver, src, None, dst, srcx, srcy, 0, 0,
                   x, y, w, h);

  XRenderFreePicture(fl_display, src);
  XRenderFreePicture(fl_display, dst);
#endif
}
#endif


void Fl_Graphics_Driver::add_rectangle_to_region(Fl_Region r, int X, int Y, int W, int H) {
  XRectangle R;
  R.x = X; R.y = Y; R.width = W; R.height = H;
  XUnionRectWithRegion(&R, r, r);
}

void Fl_Xlib_Graphics_Driver::transformed_vertex0(short x, short y) {
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

//
// End of "$Id$".
//
