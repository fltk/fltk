//
// "$Id$"
//
// Line style code for the Fast Light Tool Kit (FLTK).
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

#ifndef FL_CFG_GFX_XLIB_LINE_STYLE_CXX
#define FL_CFG_GFX_XLIB_LINE_STYLE_CXX

/**
  \file xlib_line_style.cxx
  \brief Line style drawing utility hiding different platforms.
*/

#include "../../config_lib.h"
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/x.H>
#include <FL/Fl_Printer.H>
#include "../../flstring.h"
#include <stdio.h>

#include "Fl_Xlib_Graphics_Driver.h"

// We save the current line width (absolute value) here.
// This is currently used only for X11 clipping, see src/fl_rect.cxx.
// FIXME: this would probably better be in class Fl::
extern int fl_line_width_;

void Fl_Xlib_Graphics_Driver::line_style(int style, int width, char* dashes) {

  // save line width in global variable for X11 clipping
  if (width == 0) fl_line_width_ = 1;
  else fl_line_width_ = width>0 ? width : -width;

  int ndashes = dashes ? strlen(dashes) : 0;
  // emulate the WIN32 dash patterns on X
  char buf[7];
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
    case FL_DASH:	*p++ = dash; *p++ = gap; break;
    case FL_DOT:	*p++ = dot; *p++ = gap; break;
    case FL_DASHDOT:	*p++ = dash; *p++ = gap; *p++ = dot; *p++ = gap; break;
    case FL_DASHDOTDOT: *p++ = dash; *p++ = gap; *p++ = dot; *p++ = gap; *p++ = dot; *p++ = gap; break;
    }
    ndashes = p-buf;
  }
  static int Cap[4] = {CapButt, CapButt, CapRound, CapProjecting};
  static int Join[4] = {JoinMiter, JoinMiter, JoinRound, JoinBevel};
  XSetLineAttributes(fl_display, gc, width,
		     ndashes ? LineOnOffDash : LineSolid,
		     Cap[(style>>8)&3], Join[(style>>12)&3]);
  if (ndashes) XSetDashes(fl_display, gc, 0, dashes, ndashes);
}

#endif // FL_CFG_GFX_XLIB_LINE_STYLE_CXX

//
// End of "$Id$".
//
