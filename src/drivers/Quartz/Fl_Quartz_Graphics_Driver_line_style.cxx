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

#include <FL/fl_draw.H>
#include <FL/platform.H>


/**
  \file quartz_line_style.cxx
  \brief Line style drawing utility hiding different platforms.
*/

#include "Fl_Quartz_Graphics_Driver.H"

void Fl_Quartz_Graphics_Driver::quartz_restore_line_style() {
  CGContextSetLineWidth(gc_, quartz_line_width_);
  CGContextSetLineCap(gc_, quartz_line_cap_);
  CGContextSetLineJoin(gc_, quartz_line_join_);
  CGContextSetLineDash(gc_, 0, quartz_line_pattern, quartz_line_pattern_size);
}

void Fl_Quartz_Graphics_Driver::line_style(int style, int width, char* dashes) {

  static CGLineCap Cap[4] = { kCGLineCapButt, kCGLineCapButt,
                                   kCGLineCapRound, kCGLineCapSquare };
  static CGLineJoin Join[4] = { kCGLineJoinMiter, kCGLineJoinMiter,
                                    kCGLineJoinRound, kCGLineJoinBevel };
  if (width<1) width = 1;
  quartz_line_width_ = (float)width;
  quartz_line_cap_ = Cap[(style>>8)&3];
  // when printing kCGLineCapSquare seems better for solid lines
  if ( Fl_Surface_Device::surface() != Fl_Display_Device::display_device()
      && style == FL_SOLID && dashes == NULL )
  {
    quartz_line_cap_ = kCGLineCapSquare;
  }
  quartz_line_join_ = Join[(style>>12)&3];
  char *d = dashes;
  static CGFloat pattern[16];
  if (d && *d) {
    CGFloat *pDst = pattern;
    while (*d) { *pDst++ = (float)*d++; }
    quartz_line_pattern = pattern;
    quartz_line_pattern_size = (int)(d-dashes);
  } else if (style & 0xff) {
    char dash, dot, gap;
    // adjust lengths to account for cap:
    if (style & 0x200) {
      dash = char(2*width);
      dot = 1;
      gap = char(2*width-1);
    } else {
      dash = char(3*width);
      dot = gap = char(width);
    }
    CGFloat *pDst = pattern;
    switch (style & 0xff) {
    case FL_DASH:       *pDst++ = dash; *pDst++ = gap; break;
    case FL_DOT:        *pDst++ = dot; *pDst++ = gap; break;
    case FL_DASHDOT:    *pDst++ = dash; *pDst++ = gap; *pDst++ = dot; *pDst++ = gap; break;
    case FL_DASHDOTDOT: *pDst++ = dash; *pDst++ = gap; *pDst++ = dot; *pDst++ = gap; *pDst++ = dot; *pDst++ = gap; break;
    }
    quartz_line_pattern_size = (int)(pDst-pattern);
    quartz_line_pattern = pattern;
  } else {
    quartz_line_pattern = 0;
    quartz_line_pattern_size = 0;
  }
  quartz_restore_line_style();
}
