//
// "$Id$"
//
// "Gleam" scheme box drawing routines for the Fast Light Tool Kit (FLTK).
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
// These box types provide a sort of Clearlooks Glossy scheme
// for FLTK.
//
// Copyright 2001-2005 by Colin Jones.
//
// Modified 2012-2013 by Edmanuel Torres
// This is a new version of the fl_gleam. The gradients are on the top
// and the bottom, the text area looks like in the classic FLTK way.
//

#include <FL/Fl.H>
#include <FL/fl_draw.H>

/*
  Implementation notes:

  All box types have 2 pixel wide borders, there is no distinction
  of "thin" box types, hence Fl::box_dx() = Fl::box_dy() = 2,
  Fl::box_dw() = Fl::box_dh() = 4 for all box types.

  All coordinates of function calls (x, y, w, h) describe the full
  box size with borders, i.e. the original box coordinates.
  The interior (background) of the box is drawn with reduced size.
  This is adjusted only in function shade_rect_top_bottom().

  Note: There is one pixel "leaking" out of the box border at each
  corner which is currently not drawn in a full box redraw.
*/

// Standard box drawing code

static void gleam_color(Fl_Color c) {
  Fl::set_box_color(c);
}

/* Draw the shaded background of the box.

  This is called before the box frame (border) is drawn.
  The interior of the box (shaded background) should be inset by two
  pixels, hence width and height should be reduced by 4 (border size).
  Since not all pixels of the border are drawn (border lines are shorter
  by one or two pixels on each side) setting other offsets (+1, -2 instead
  of +2, -4) would leak the box background color instead of the parent
  widget's background color.
  Note: the original implementation was maybe equivalent to using +1 and -2
  for offset and width, respectively instead of +2 and -4. It is not clear
  whether this was intended or by accident.
*/

static void shade_rect_top_bottom(int x, int y, int w, int h, Fl_Color fg1, Fl_Color fg2, float th) {
  // calculate background size w/o borders
  x += 2; y += 2; w -= 4; h -= 4;
  // draw the shiny background using maximum limits
  int h_top    = ((h/2) < (20) ? (h/2) : (20)); // min(h/2, 20);
  int h_bottom = ((h/6) < (15) ? (h/6) : (15)); // min(h/6, 15);
  int h_flat = h - h_top - h_bottom;
  float step_size_top = h_top > 1 ? (0.999f/float(h_top)) : 1;
  float step_size_bottom = h_bottom > 1 ? (0.999f/float(h_bottom)) : 1;
  // draw the gradient at the top of the widget
  float k = 1;
  for (int i = 0; i < h_top; i++, k -= step_size_top) {
    gleam_color(fl_color_average(fl_color_average(fg1, fg2, th), fg1, k));
    fl_xyline(x, y+i, x+w-1);
  }

  // draw a "flat" rectangle in the middle area of the box
  gleam_color(fg1);
  fl_rectf(x, y + h_top, w, h_flat);

  // draw the gradient at the bottom of the widget
  k = 1;
  for (int i = 0; i < h_bottom; i++, k -= step_size_bottom) {
    gleam_color(fl_color_average(fg1, fl_color_average(fg1, fg2, th), k));
    fl_xyline(x, y+h_top+h_flat+i, x+w-1);
  }
}

// See shade_rect_top_bottom()
static void shade_rect_top_bottom_up(int x, int y, int w, int h, Fl_Color bc, float th) {
  shade_rect_top_bottom(x, y, w, h, bc, FL_WHITE, th);
}

// See shade_rect_top_bottom()
static void shade_rect_top_bottom_down(int x, int y, int w, int h, Fl_Color bc, float th) {
  shade_rect_top_bottom(x, y, w, h, bc, FL_BLACK, th);
}

// Draw box borders. Color arguments:
// - fg1: outer border line (left, right, top, bottom)
// - fg2: inner border line (left, right)
// - lc : inner border line (top, bottom)

static void frame_rect(int x, int y, int w, int h, Fl_Color fg1, Fl_Color fg2, Fl_Color lc) {
  // outer border line:
  gleam_color(fg1);
  fl_xyline(x+1,   y,     x+w-2);   // top
  fl_yxline(x+w-1, y+1,   y+h-2);   // right
  fl_xyline(x+1,   y+h-1, x+w-2);   // bottom
  fl_yxline(x,     y+1,   y+h-2);   // left

  // inner border line (left, right):
  gleam_color(fg2);
  fl_yxline(x+1,   y+2,   y+h-3);   // left
  fl_yxline(x+w-2, y+2,   y+h-3);   // right

  // inner border line (top, bottom):
  gleam_color(lc);
  fl_xyline(x+2,   y+1,   x+w-3);   // top
  fl_xyline(x+2,   y+h-2, x+w-3);   // bottom
}

// Draw box borders with different colors (up/down effect).

static void frame_rect_up(int x, int y, int w, int h, Fl_Color bc, Fl_Color lc, float th1, float th2) {
  frame_rect(x, y, w, h, fl_color_average(fl_darker(bc), FL_BLACK, th1), fl_color_average(bc, FL_WHITE, th2), lc);
}

static void frame_rect_down(int x, int y, int w, int h, Fl_Color bc, Fl_Color lc, float th1, float th2) {
  frame_rect(x,y,w,h,fl_color_average(bc, FL_WHITE, th1), fl_color_average(FL_BLACK, bc, th2), lc);
}

// Draw the different box types. These are the actual box drawing functions.

static void up_frame(int x, int y, int w, int h, Fl_Color c) {
  frame_rect_up(x, y, w, h, c, fl_color_average(c, FL_WHITE, .25f), .55f, .05f);
}

static void up_box(int x, int y, int w, int h, Fl_Color c) {
  shade_rect_top_bottom_up(x, y, w, h, c, .15f);
  frame_rect_up(x, y, w, h, c, fl_color_average(c, FL_WHITE, .05f), .15f, .05f);
}

static void thin_up_box(int x, int y, int w, int h, Fl_Color c) {
  shade_rect_top_bottom_up(x, y, w, h, c, .25f);
  frame_rect_up(x, y, w, h, c, fl_color_average(c, FL_WHITE, .45f), .25f, .15f);
}

static void down_frame(int x, int y, int w, int h, Fl_Color c) {
  frame_rect_down(x, y, w, h, fl_darker(c), fl_darker(c), .25f, .95f);
}

static void down_box(int x, int y, int w, int h, Fl_Color c) {
  shade_rect_top_bottom_down(x, y, w, h, c, .65f);
  frame_rect_down(x, y, w, h, c, fl_color_average(c, FL_BLACK, .05f), .05f, .95f);
}

static void thin_down_box(int x, int y, int w, int h, Fl_Color c) {
  shade_rect_top_bottom_down(x, y, w, h, c, .85f);
  frame_rect_down(x, y, w, h, c, fl_color_average(c, FL_BLACK, .45f), .35f, 0.85f);
}

extern void fl_internal_boxtype(Fl_Boxtype, Fl_Box_Draw_F*);

Fl_Boxtype fl_define_FL_GLEAM_UP_BOX() {
  fl_internal_boxtype(_FL_GLEAM_UP_BOX, up_box);
  fl_internal_boxtype(_FL_GLEAM_DOWN_BOX, down_box);
  fl_internal_boxtype(_FL_GLEAM_UP_FRAME, up_frame);
  fl_internal_boxtype(_FL_GLEAM_DOWN_FRAME, down_frame);
  fl_internal_boxtype(_FL_GLEAM_THIN_UP_BOX, thin_up_box);
  fl_internal_boxtype(_FL_GLEAM_THIN_DOWN_BOX, thin_down_box);
  fl_internal_boxtype(_FL_GLEAM_ROUND_UP_BOX, up_box);
  fl_internal_boxtype(_FL_GLEAM_ROUND_DOWN_BOX, down_box);
  return _FL_GLEAM_UP_BOX;
}


//
// End of "$Id$".
//
