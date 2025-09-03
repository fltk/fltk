//
// Line style code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2024 by Bill Spitzak and others.
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
  \file Fl_Xlib_Graphics_Driver_line_style.cxx
  \brief Line style drawing utility hiding different platforms.
*/

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/platform.H>
#include "../../flstring.h"
#include "Fl_Xlib_Graphics_Driver.H"
#include <stdlib.h>

void Fl_Xlib_Graphics_Driver::line_style_unscaled(int style, int width, char* dashes) {

  int ndashes = dashes ? strlen(dashes) : 0;
  // emulate the Windows dash patterns on X
  char buf[7] = {0};
  if (!ndashes && (style&0xff)) {
    int w = width ? width : 1;
    char dash, dot, gap;
    // adjust lengths to account for cap:
    if (style & 0x200) {
      dash = char(2*w);
      dot = 1; // unfortunately 0 does not work
      gap = char(2*w-1);
    } else {
      dash = char(3*w);
      dot = gap = char(w);
    }
    char* p = dashes = buf;
    switch (style & 0xff) {
      case FL_DASH:       *p++ = dash; *p++ = gap; break;
      case FL_DOT:        *p++ = dot; *p++ = gap; break;
      case FL_DASHDOT:    *p++ = dash; *p++ = gap; *p++ = dot; *p++ = gap; break;
      case FL_DASHDOTDOT: *p++ = dash; *p++ = gap; *p++ = dot; *p++ = gap; *p++ = dot; *p++ = gap; break;
    }
    ndashes = p-buf;
    if (*dashes == 0) ndashes = 0; // against error with very small scaling
  }
  static int Cap[4] = {CapButt, CapButt, CapRound, CapProjecting};
  static int Join[4] = {JoinMiter, JoinMiter, JoinRound, JoinBevel};
  XSetLineAttributes(fl_display, gc_,
                     line_width_,
                     ndashes ? LineOnOffDash : LineSolid,
                     Cap[(style>>8)&3], Join[(style>>12)&3]);
  if (ndashes) XSetDashes(fl_display, gc_, 0, dashes, ndashes);
}

void *Fl_Xlib_Graphics_Driver::change_pen_width(int lwidth) {
  XGCValues *gc_values = (XGCValues*)malloc(sizeof(XGCValues));
  gc_values->line_width = lwidth;
  XChangeGC(fl_display, gc_, GCLineWidth, gc_values);
  gc_values->line_width = line_width_;
  line_width_ = lwidth;
  return gc_values;
}

void Fl_Xlib_Graphics_Driver::reset_pen_width(void *data) {
  XGCValues *gc_values = (XGCValues*)data;
  line_width_ = gc_values->line_width;
  XChangeGC(fl_display, gc_, GCLineWidth, gc_values);
  free(data);
}
