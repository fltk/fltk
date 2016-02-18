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


#include <config.h>
#include "../../config_lib.h"
#include "Fl_Xlib_Graphics_Driver.h"
#include <string.h>

#if HAVE_XRENDER
#include <X11/extensions/Xrender.h>
#endif


const char *Fl_Xlib_Graphics_Driver::class_id = "Fl_Xlib_Graphics_Driver";

/* Reference to the current graphics context
 For back-compatibility only. The preferred procedure to get this pointer is
 Fl_Surface_Device::surface()->driver()->get_gc().
 */
GC fl_gc = 0;

void Fl_Graphics_Driver::global_gc()
{
  fl_gc = (GC)get_gc();
}


/*
 * By linking this module, the following static method will instatiate the
 * X11 Xlib Graphics driver as the main display driver.
 */
Fl_Graphics_Driver *Fl_Graphics_Driver::newMainGraphicsDriver()
{
  return new Fl_Xlib_Graphics_Driver();
}

GC Fl_Xlib_Graphics_Driver::gc = NULL;

Fl_Xlib_Graphics_Driver::Fl_Xlib_Graphics_Driver(void) {
  if (!gc) {
    fl_open_display();
    // the unique GC used by all X windows
    gc = XCreateGC(fl_display, RootWindow(fl_display, fl_screen), 0, 0);
  }
}

char Fl_Xlib_Graphics_Driver::can_do_alpha_blending() {
  return Fl_X::xrender_supported();
}


void Fl_Xlib_Graphics_Driver::copy_offscreen(int x, int y, int w, int h, Fl_Offscreen pixmap, int srcx, int srcy) {
  XCopyArea(fl_display, pixmap, fl_window, gc, srcx, srcy, w, h, x, y);
}

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

//
// End of "$Id$".
//
