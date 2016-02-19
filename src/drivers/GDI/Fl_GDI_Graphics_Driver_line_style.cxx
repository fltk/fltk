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

#ifndef FL_CFG_GFX_GDI_LINE_STYLE_CXX
#define FL_CFG_GFX_GDI_LINE_STYLE_CXX

#include "../../config_lib.h"
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/x.H>
#include <FL/Fl_Printer.H>
#include "../../flstring.h"
#include <stdio.h>

/**
  \file gdi_line_style.cxx
  \brief Line style drawing utility hiding different platforms.
*/

#include "Fl_GDI_Graphics_Driver.h"

extern int fl_line_width_;  // defined in src/fl_line_style.cxx


void Fl_GDI_Graphics_Driver::line_style(int style, int width, char* dashes) {

  // save line width in global variable for X11 clipping
  if (width == 0) fl_line_width_ = 1;
  else fl_line_width_ = width>0 ? width : -width;

  // According to Bill, the "default" cap and join should be the
  // "fastest" mode supported for the platform.  I don't know why
  // they should be different (same graphics cards, etc., right?) MRS
  static DWORD Cap[4]= {PS_ENDCAP_FLAT, PS_ENDCAP_FLAT, PS_ENDCAP_ROUND, PS_ENDCAP_SQUARE};
  static DWORD Join[4]={PS_JOIN_ROUND, PS_JOIN_MITER, PS_JOIN_ROUND, PS_JOIN_BEVEL};
  int s1 = PS_GEOMETRIC | Cap[(style>>8)&3] | Join[(style>>12)&3];
  DWORD a[16]; int n = 0;
  if (dashes && dashes[0]) {
    s1 |= PS_USERSTYLE;
    for (n = 0; n < 16 && *dashes; n++) a[n] = *dashes++;
  } else {
    s1 |= style & 0xff; // allow them to pass any low 8 bits for style
  }
  if ((style || n) && !width) width = 1; // fix cards that do nothing for 0?
  LOGBRUSH penbrush = {BS_SOLID,fl_RGB(),0}; // can this be fl_brush()?
  HPEN newpen = ExtCreatePen(s1, width, &penbrush, n, n ? a : 0);
  if (!newpen) {
    Fl::error("fl_line_style(): Could not create GDI pen object.");
    return;
  }
  HPEN oldpen = (HPEN)SelectObject(gc_, newpen);
  DeleteObject(oldpen);
  DeleteObject(fl_current_xmap->pen);
  fl_current_xmap->pen = newpen;
}

#endif // FL_CFG_GFX_GDI_LINE_STYLE_CXX

//
// End of "$Id$".
//
