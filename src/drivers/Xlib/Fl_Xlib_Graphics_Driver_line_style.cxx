//
// "$Id$"
//
// Line style code for the Fast Light Tool Kit (FLTK).
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

/**
  \file Fl_Xlib_Graphics_Driver_line_style.cxx
  \brief Line style drawing utility hiding different platforms.
*/

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/platform.H>
#include <FL/Fl_Printer.H>
#include "../../flstring.h"

#include "Fl_Xlib_Graphics_Driver.H"

void Fl_Xlib_Graphics_Driver::line_style_unscaled(int style, float width, char* dashes) {

  int ndashes = dashes ? strlen(dashes) : 0;
  // emulate the Windows dash patterns on X
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
if (*dashes == 0) ndashes = 0;//against error with very small scaling
  }
  static int Cap[4] = {CapButt, CapButt, CapRound, CapProjecting};
  static int Join[4] = {JoinMiter, JoinMiter, JoinRound, JoinBevel};
  XSetLineAttributes(fl_display, gc_,
                     line_width_,
		     ndashes ? LineOnOffDash : LineSolid,
		     Cap[(style>>8)&3], Join[(style>>12)&3]);
  if (ndashes) XSetDashes(fl_display, gc_, 0, dashes, ndashes);
}

//
// End of "$Id$".
//
