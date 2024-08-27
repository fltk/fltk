//
// Line style code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2018 by Bill Spitzak and others.
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
  \file Fl_GDI_Graphics_Driver_line_style.cxx

  \brief Line style drawing utility for Windows (GDI) platform.
*/

#include <FL/Fl.H>
#include <FL/platform.H>
#include <FL/fl_draw.H>

#include "Fl_GDI_Graphics_Driver.H"


void Fl_GDI_Graphics_Driver::line_style_unscaled(int style, int width, char* dashes) {

  // According to Bill, the "default" cap and join should be the
  // "fastest" mode supported for the platform.  I don't know why
  // they should be different (same graphics cards, etc., right?) MRS

  static const DWORD Cap[4]  = {PS_ENDCAP_FLAT, PS_ENDCAP_FLAT, PS_ENDCAP_ROUND, PS_ENDCAP_SQUARE};
  static const DWORD Join[4] = {PS_JOIN_ROUND, PS_JOIN_MITER, PS_JOIN_ROUND, PS_JOIN_BEVEL};

  int s1 = PS_GEOMETRIC | Cap[(style>>8)&3] | Join[(style>>12)&3];
  DWORD a[16];
  int n = 0;
  if (dashes && dashes[0]) {
    s1 |= PS_USERSTYLE;
    for (n = 0; n < 16 && *dashes; n++) a[n] = *dashes++;
  } else {
    s1 |= style & 0xff; // allow them to pass any low 8 bits for style
  }
  if ((style || n) && !width) width = int(scale()); // fix cards that do nothing for 0?
  if (!width) width = 1;
  if (!fl_current_xmap) color(FL_BLACK);
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
  is_solid_ = ((style & 0xff) == FL_SOLID && (!dashes || !*dashes));
  style_ = style;
}

#if USE_GDIPLUS

void Fl_GDIplus_Graphics_Driver::line_style(int style, int width, char* dashes) {
  if (!active) return Fl_Scalable_Graphics_Driver::line_style(style, width, dashes);
  int gdi_width = (width ? width : 1);
  pen_->SetWidth(Gdiplus::REAL(gdi_width));
  int standard_dash = style & 0x7;
  if (standard_dash == FL_DASH )
    pen_->SetDashStyle(Gdiplus::DashStyleDash);
  else if (standard_dash == FL_DOT )
    pen_->SetDashStyle(Gdiplus::DashStyleDot);
  else if (standard_dash == FL_DASHDOT )
    pen_->SetDashStyle(Gdiplus::DashStyleDashDot);
  else if (standard_dash == FL_DASHDOTDOT )
    pen_->SetDashStyle(Gdiplus::DashStyleDashDotDot);
  else if(!dashes || !*dashes)
    pen_->SetDashStyle(Gdiplus::DashStyleSolid);

  if (style & FL_CAP_ROUND ) {
    pen_->SetStartCap(Gdiplus::LineCapRound);
    pen_->SetEndCap(Gdiplus::LineCapRound);
  } else if (style & FL_CAP_SQUARE ) {
    pen_->SetStartCap(Gdiplus::LineCapSquare);
    pen_->SetEndCap(Gdiplus::LineCapSquare);
  } else {
    pen_->SetStartCap(Gdiplus::LineCapFlat);
    pen_->SetEndCap(Gdiplus::LineCapFlat);
  }

  if (style & FL_JOIN_MITER ) {
    pen_->SetLineJoin(Gdiplus::LineJoinMiter);
  } else if (style & FL_JOIN_BEVEL ) {
    pen_->SetLineJoin(Gdiplus::LineJoinBevel);
  } else {
    pen_->SetLineJoin(Gdiplus::LineJoinRound);
  }

  if (dashes && *dashes) {
    int n = 0; while (dashes[n]) n++;
    Gdiplus::REAL *gdi_dashes = new Gdiplus::REAL[n];
    for (int i = 0; i < n; i++) gdi_dashes[i] = dashes[i]/float(gdi_width);
    pen_->SetDashPattern(gdi_dashes, n);
    delete[] gdi_dashes;
  }
  Fl_Scalable_Graphics_Driver::line_style(style, width, dashes);
}

#endif
