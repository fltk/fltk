//
// "Oxy" Scheme drawing routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 2011 by Dmitrij K. aka "kdiman"
// Copyright 2012-2024 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//   https://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//   https://www.fltk.org/str.php
//

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Rect.H>
#include "fl_oxy.h"

// Note:
//
// Drawing on the X11 platform w/o Cairo can be asymmetric for some "arrows"

#define GROFF 0.45f // gradients offset

#ifndef DEBUG_OXY_ARROW
#define DEBUG_OXY_ARROW 0   // 0 = off, 1 = cross at center position of arrows
#endif

// Draw a single arrow

static void single_arrow(Fl_Rect bb, Fl_Orientation o, Fl_Color col) {

  int x1 = bb.x();
  int y1 = bb.y();
  int w1 = bb.w();
  int h1 = bb.h();

  float angle = int(o) * 45.0f;

  // calculate arrow size
  int dx = (w1 - 3) / 2;
  if (h1 < w1) dx = (h1 - 3) / 2;
  if (dx > 4) dx = 4;
  else if (dx < 2) dx = 2;
  // dx = (dx + 1) & (-2);  // should be even ? not required

  int tx = x1 + w1/2;
  int ty = y1 + h1/2;

  const int lw = 2;         // arrow line width: n+1 pixels (must be even)
  const int dw = 1;         // half the line width

  fl_color(col);
  fl_line_style(FL_SOLID, 1);
  fl_push_matrix();

  fl_translate(tx, ty);     // move to center
  fl_rotate(angle);         // rotate by given angle

  // DEBUG: Draw a two-colored cross at the center of the arrow box
  // This can be used to debug or adjust array alignment (centering)

#if (DEBUG_OXY_ARROW)                   // draw a cross at the center
  int ll;                               // line length
  fl_color(FL_BLUE);                    // "horizontal" line
  ll = (o&2) ? h1/2 - 1 : w1/2 - 1;     // line length
  fl_begin_line();
  fl_vertex(-ll, 0); fl_vertex(ll, 0);
  fl_end_line();
  fl_color(0x22882200);                 // "vertical" line
  ll = (o&2) ? w1/2 - 1 : h1/2 - 1;     // line length
  fl_begin_line();
  fl_vertex(0, -ll); fl_vertex(0, ll);
  fl_end_line();
  fl_color(col);                        // back to original color
#endif

  // Draw the "arrow", similar to '>' at the center of the box

  fl_begin_complex_polygon();
  fl_vertex(-dx + dw,      -dx);
  fl_vertex(  0 + dw,        0);
  fl_vertex(-dx + dw,       dx);
  fl_vertex(-dx + dw + lw,  dx);
  fl_vertex(  0 + dw + lw,   0);
  fl_vertex(-dx + dw + lw, -dx);
  fl_end_complex_polygon();

  fl_pop_matrix();
  fl_line_style(0);
}

/**
  Draw an "arrow" GUI element for the 'oxy' scheme.

  This draws one or two "arrows" depending on the arrow type \p t.

  \param[in]  bb       bounding box
  \param[in]  t        arrow type
  \param[in]  o        orientation
  \param[in]  col      arrow color
*/
void oxy_arrow(Fl_Rect bb, Fl_Arrow_Type t, Fl_Orientation o, Fl_Color col) {

  switch(t) {

    case FL_ARROW_DOUBLE:

      switch (int(o)) {
        case FL_ORIENT_DOWN:
        case FL_ORIENT_UP:
          bb.h(bb.h() - 4);           // reduce size
          single_arrow(bb, o, col);
          bb.y(bb.y() + 4);           // shift down
          single_arrow(bb, o, col);
          break;
        default:
          bb.w(bb.w() - 4);           // reduce size
          single_arrow(bb, o, col);
          bb.x(bb.x() + 4);           // shift right
          single_arrow(bb, o, col);
          break;
      }
      break;

    case FL_ARROW_CHOICE:

      bb.y(bb.y() - 1);               // shift upwards
      bb.h(bb.h() - 4);               // reduce height
      single_arrow(bb, FL_ORIENT_UP, col);
      bb.y(bb.y() + 6);               // shift down
      single_arrow(bb, FL_ORIENT_DOWN, col);
      break;

    default:

      single_arrow(bb, o, col);
      break;
  }
}


// draw gradient from South to North
static void _oxy_up_box_(int x, int y, int w, int h, Fl_Color bg) {
  float groff = GROFF;
  if (groff < 0.0) {
    groff = 0.0f;
  }
  float gradoffset = groff;
  float stepoffset = (1.0f / (float)h);
  int xw = x + w - 1;
  // from bottom to top
  for (int _y = y; _y < y + h; _y++) {
    fl_color(fl_color_average(bg, FL_WHITE, (gradoffset < 1.0f) ? gradoffset : 1.0f));
    fl_xyline(x, _y, xw);
    gradoffset += stepoffset;
  }
}


// draw gradient from North to South
static void _oxy_down_box_(int x, int y, int w, int h, Fl_Color bg) {
  float groff = GROFF;
  if (groff < 0.0) {
    groff = 0.0f;
  }
  float gradoffset = groff;
  float stepoffset = (1.0f / (float)h);
  int xw = x + w - 1;
  // from top to bottom
  for (int _y = y + h - 1; _y >= y; _y--) {
    fl_color(fl_color_average(bg, FL_WHITE, (gradoffset < 1.0f) ? gradoffset : 1.0f));
    fl_xyline(x, _y, xw);
    gradoffset += stepoffset;
  }
}

// draw gradient for button up box
static void _oxy_button_up_box_(int x, int y, int w, int h, Fl_Color bg) {
  int half_h = h / 2;
  float gradoffset = 0.15f;
  float stepoffset = (1.0f / (float)half_h);
  Fl_Color col = fl_color_average(bg, FL_WHITE, 0.5);
  int xw = x + w - 1;
  for (int _y = y; _y <= y + half_h; _y++) {
    fl_color(fl_color_average(col, FL_WHITE, (gradoffset < 1.0f) ? gradoffset : 1.0f));
    fl_xyline(x, _y, xw);
    gradoffset += stepoffset;
  }
  gradoffset = 0.0f;
  col = bg;
  for (int _y = y + h - 1; _y >= y + half_h - 1; _y--) {
    fl_color(fl_color_average(col, FL_WHITE, (gradoffset < 1.0f) ? gradoffset : 1.0f));
    fl_xyline(x, _y, xw);
    gradoffset += stepoffset;
  }
}


// draw gradient for button down box
static void _oxy_button_down_box_(int x, int y, int w, int h, Fl_Color bg) {
  bg = fl_color_average(bg, FL_BLACK, 0.88f);
  int half_h = h / 2, xw = x + w - 1;
  float gradoffset = 0.15f, stepoffset = (1.0f / (float)half_h);
  Fl_Color col = fl_color_average(bg, FL_WHITE, 0.5);
  for (int _y = y; _y <= y + half_h; _y++) {
    fl_color(fl_color_average(col, FL_WHITE, (gradoffset < 1.0f) ? gradoffset : 1.0f));
    fl_xyline(x, _y, xw);
    gradoffset += stepoffset;
  }
  gradoffset = 0.0f;
  col = bg;
  for (int _y = y + h - 1; _y >= y + half_h - 1; _y--) {
    fl_color(fl_color_average(col, FL_WHITE, (gradoffset < 1.0f) ? gradoffset : 1.0f));
    fl_xyline(x, _y, xw);
    gradoffset += stepoffset;
  }
}


// draw rounded box
static void _oxy_rounded_box_(int x, int y, int w, int h, Fl_Color bg) {
  fl_color(bg);
  if (w > h) {
    fl_pie(x, y, h, h, 90.0, 270.0);          // right half of circle
    fl_rectf(x + h / 2, y, w - h + 1, h);     // rectangle between left and right half-circle
    fl_pie(x + w - h, y, h, h, 0.0, 90.0);    // top-left quarter of circle
    fl_pie(x + w - h, y, h, h, 270.0, 360.0); // bottom-left quarter of circle
  } else if (w == h) {
    fl_pie(x, y, w, w, 0.0, 360.0);
  } else {
    fl_pie(x, y, w, w, 0.0, 180.0);           // top half of circle
    fl_rectf(x, y + w / 2, w, h - w + 1);     // rectangle between top and bottom half-circle
    fl_pie(x, y + h - w, w, w, 180.0, 360.0); // bottom half of circle
  }
}


static Fl_Color oxy_color(Fl_Color col) {
  if (Fl::draw_box_active()) {
    return col;
  } else {
    return fl_inactive(col);
  }
}


static void oxy_draw(int x, int y, int w, int h, Fl_Color col, int typebox, bool is_shadow) {

  if (w < 1 || h < 1)
    return;
  int X, Y, W, H, X1, Y1;

  // draw bg
  if (typebox != _FL_OXY_UP_FRAME && typebox != _FL_OXY_DOWN_FRAME) {

    X = x + 1;
    Y = y + 1;
    W = w - 2;
    H = h - 2;

    switch (typebox) {
      case _FL_OXY_UP_BOX: {
        _oxy_up_box_(X, Y, W, H, oxy_color(col));
        break;
      }
      case _FL_OXY_DOWN_BOX: {
        _oxy_down_box_(X, Y, W, H, oxy_color(col));
        break;
      }
      case _FL_OXY_BUTTON_UP_BOX: {
        _oxy_button_up_box_(X, Y, W, H, oxy_color(col));
        break;
      }
      case _FL_OXY_BUTTON_DOWN_BOX: {
        _oxy_button_down_box_(X, Y, W, H, oxy_color(col));
        break;
      }
      case _FL_OXY_ROUND_UP_BOX:
      case _FL_OXY_ROUND_DOWN_BOX:
        _oxy_rounded_box_(x, y, w, h, oxy_color(fl_color_average(col, FL_WHITE, 0.82f)));
        break;
      default: { break; }
    }
  }

  Fl_Color leftline = col, topline = col, rightline = col, bottomline = col;

  if (typebox == _FL_OXY_ROUND_UP_BOX || typebox == _FL_OXY_ROUND_DOWN_BOX) {
    leftline = fl_color_average(col, FL_WHITE, 0.88f);
    leftline = topline = rightline = bottomline = fl_color_average(leftline, FL_BLACK, 0.97f);
  }

  else if (typebox == _FL_OXY_UP_BOX || typebox == _FL_OXY_UP_FRAME) {
    topline = fl_color_average(col, FL_BLACK, 0.95f);
    leftline = fl_color_average(col, FL_BLACK, 0.85f);
    rightline = leftline;
    bottomline = fl_color_average(col, FL_BLACK, 0.88f);
  }

  else if (typebox == _FL_OXY_DOWN_BOX || typebox == _FL_OXY_DOWN_FRAME) {
    topline = fl_color_average(col, FL_BLACK, 0.88f);
    leftline = fl_color_average(col, FL_BLACK, 0.85f);
    rightline = leftline;
    bottomline = fl_color_average(col, FL_BLACK, 0.95f);
  }

  else if (typebox == _FL_OXY_BUTTON_UP_BOX || typebox == _FL_OXY_BUTTON_DOWN_BOX) {
    topline = leftline = rightline = bottomline = fl_color_average(col, FL_BLACK, 0.85f);
  }

  // draw border
  if (typebox != _FL_OXY_ROUND_UP_BOX && typebox != _FL_OXY_ROUND_DOWN_BOX) {
    // bottom side
    fl_color(oxy_color(bottomline));
    fl_line(x + 1, y + h - 1, x + w - 2, y + h - 1);
    // right side
    fl_color(oxy_color(rightline));
    fl_line(x + w - 1, y + 1, x + w - 1, y + h - 2);
    // top side
    fl_color(oxy_color(topline));
    fl_line(x + 1, y, x + w - 2, y);
    // left side
    fl_color(oxy_color(leftline));
    fl_line(x, y + 1, x, y + h - 2);
  }

  // draw shadow
  if (is_shadow) {

    if (typebox == _FL_OXY_ROUND_UP_BOX) {
      topline = fl_color_average(col, FL_WHITE, 0.35f);
      bottomline = fl_color_average(col, FL_BLACK, 0.94f);
    }

    else if (typebox == _FL_OXY_ROUND_DOWN_BOX) {
      topline = fl_color_average(col, FL_BLACK, 0.94f);
      bottomline = fl_color_average(col, FL_WHITE, 0.35f);
    }

    else if (typebox == _FL_OXY_UP_BOX || typebox == _FL_OXY_UP_FRAME) {
      topline = fl_color_average(col, FL_WHITE, 0.35f);
      leftline = fl_color_average(col, FL_WHITE, 0.4f);
      rightline = leftline;
      bottomline = fl_color_average(col, FL_BLACK, 0.8f);
    }

    else if (typebox == _FL_OXY_DOWN_BOX || typebox == _FL_OXY_DOWN_FRAME) {
      topline = fl_color_average(col, FL_BLACK, 0.8f);
      leftline = fl_color_average(col, FL_BLACK, 0.94f);
      rightline = leftline;
      bottomline = fl_color_average(col, FL_WHITE, 0.35f);
    }

    int xw1 = x + w - 1;
    int xw2 = x + w - 2;
    int xw3 = x + w - 3;
    int yh2 = y + h - 2;
    int yh1 = y + h - 1;

    if (typebox == _FL_OXY_UP_BOX || typebox == _FL_OXY_UP_FRAME) {
      fl_color(oxy_color(topline));
      X = x + 1;
      Y = y + 1;
      X1 = xw2;
      Y1 = y + 1;
      fl_line(X, Y, X1, Y1); // top line

      fl_color(oxy_color(leftline));
      X = x + 1;
      Y = yh2;
      X1 = x + 1;
      Y1 = y + 2;
      fl_line(X, Y, X1, Y1); // left line

      fl_color(oxy_color(rightline));
      X = xw2;
      Y = y + 2;
      X1 = xw2;
      Y1 = yh2;
      fl_line(X, Y, X1, Y1); // right line

      fl_color(oxy_color(bottomline));
      X = xw2;
      Y = yh2;
      X1 = x + 1;
      Y1 = yh2;
      fl_line(X, Y, X1, Y1); // bottom line
    }

    else if (typebox == _FL_OXY_DOWN_BOX || typebox == _FL_OXY_DOWN_FRAME) {
      fl_color(oxy_color(topline));
      X = x + 1;
      Y = y + 1;
      X1 = xw2;
      Y1 = y + 1;
      fl_line(X, Y, X1, Y1); // top line

      fl_color(oxy_color(leftline));
      X = x + 1;
      Y = yh2;
      X1 = x + 1;
      Y1 = y + 2;
      fl_line(X, Y, X1, Y1); // left line

      fl_color(oxy_color(rightline));
      X = xw2;
      Y = y + 2;
      X1 = xw2;
      Y1 = yh2;
      fl_line(X, Y, X1, Y1); // right line

      fl_color(oxy_color(bottomline));
      X = xw3;
      Y = yh2;
      X1 = x + 2;
      Y1 = yh2;
      fl_line(X, Y, X1, Y1); // bottom line
    }

    else if (typebox == _FL_OXY_ROUND_UP_BOX || typebox == _FL_OXY_ROUND_DOWN_BOX) {

      int Radius, smooth;
      int r_offset2; // quarter of smooth and half of smooth

      if (w > h) {
        smooth = w;
      } else {
        smooth = h;
      }

      // correcting `smooth'
      if (smooth > 0 && (smooth * 3 > w || smooth * 3 > h)) {
        if (h < w) {
          smooth = h / 3;
        } else {
          smooth = w / 3;
        }
      }

      r_offset2 = smooth / 2;

      Radius = smooth * 3;
      if (Radius == 3) {
        Radius = 4;
      }

      fl_color(oxy_color(topline));
      fl_line(x + 1, yh1 - smooth - r_offset2, x + 1, y + r_offset2 + smooth);  // left side
      fl_arc(x + 1, y + 1, Radius, Radius, 90.0, 180.0);                        // left-top corner
      if (typebox == _FL_OXY_ROUND_DOWN_BOX) {
        fl_arc(x + 1, y + 1, Radius + 1, Radius + 1, 90.0, 180.0);
      }                                                                         // left-top corner (DOWN_BOX)
      fl_line(x + smooth + r_offset2, y + 1, xw1 - smooth - r_offset2, y + 1);  // top side
      fl_arc(xw1 - Radius, y + 1, Radius, Radius, 00.0, 90.0);                  // right-top corner
      if (typebox == _FL_OXY_ROUND_DOWN_BOX) {
        fl_arc(xw1 - Radius, y + 1, Radius + 1, Radius + 1, 00.0, 90.0);
      }                                                                         // right-top corner (DOWN_BOX)
      fl_line(xw2, y + smooth + r_offset2, xw2, yh1 - smooth - r_offset2);      // right side
      fl_arc(x + 1, yh1 - Radius, Radius, Radius, 180.0, 200.0);                // left-bottom corner
      fl_arc(xw1 - Radius, yh1 - Radius, Radius, Radius, 340.0, 360.0);         // right-bottom
      fl_color(oxy_color(bottomline));
      fl_arc(x + 1, yh1 - Radius, Radius, Radius, 200.0, 270.0);                // left-bottom corner
      if (typebox == _FL_OXY_ROUND_UP_BOX) {
        fl_arc(x + 1, yh1 - Radius, Radius + 1, Radius + 1, 200.0, 270.0);
      }                                                                         // left-bottom corner (UP_BOX)
      fl_line(xw1 - smooth - r_offset2, yh2, x + smooth + r_offset2, yh2);      // bottom side
      fl_arc(xw1 - Radius, yh1 - Radius, Radius, Radius, 270.0, 340.0);         // right-bottom corner
      if (typebox == _FL_OXY_ROUND_UP_BOX) {
        fl_arc(xw1 - Radius, yh1 - Radius, Radius + 1, Radius + 1, 270.0, 340.0);
      } // right-bottom corner
    }

  } // end `if (is_shadow)'

} // end `static void oxy_draw(...)'


void button_up_box(int x, int y, int w, int h, Fl_Color col) {
  oxy_draw(x, y, w, h, col, _FL_OXY_BUTTON_UP_BOX, true);
}
void button_down_box(int x, int y, int w, int h, Fl_Color col) {
  oxy_draw(x, y, w, h, col, _FL_OXY_BUTTON_DOWN_BOX, true);
}
void up_box(int x, int y, int w, int h, Fl_Color col) {
  oxy_draw(x, y, w, h, col, _FL_OXY_UP_BOX, true);
}
void down_box(int x, int y, int w, int h, Fl_Color col) {
  oxy_draw(x, y, w, h, col, _FL_OXY_DOWN_BOX, true);
}
void thin_up_box(int x, int y, int w, int h, Fl_Color col) {
  oxy_draw(x, y, w, h, col, _FL_OXY_UP_BOX, false);
}
void thin_down_box(int x, int y, int w, int h, Fl_Color col) {
  oxy_draw(x, y, w, h, col, _FL_OXY_DOWN_BOX, false);
}
void up_frame(int x, int y, int w, int h, Fl_Color col) {
  oxy_draw(x, y, w, h, col, _FL_OXY_UP_FRAME, true);
}
void down_frame(int x, int y, int w, int h, Fl_Color col) {
  oxy_draw(x, y, w, h, col, _FL_OXY_DOWN_FRAME, true);
}
void thin_up_frame(int x, int y, int w, int h, Fl_Color col) {
  oxy_draw(x, y, w, h, col, _FL_OXY_UP_FRAME, false);
}
void thin_down_frame(int x, int y, int w, int h, Fl_Color col) {
  oxy_draw(x, y, w, h, col, _FL_OXY_DOWN_FRAME, false);
}
void round_up_box(int x, int y, int w, int h, Fl_Color col) {
  oxy_draw(x, y, w, h, col, _FL_OXY_ROUND_UP_BOX, true);
}
void round_down_box(int x, int y, int w, int h, Fl_Color col) {
  oxy_draw(x, y, w, h, col, _FL_OXY_ROUND_DOWN_BOX, true);
}


extern void fl_round_focus(Fl_Boxtype bt, int x, int y, int w, int h, Fl_Color fg, Fl_Color bg);
extern void fl_internal_boxtype(Fl_Boxtype, Fl_Box_Draw_F*, Fl_Box_Draw_Focus_F* =NULL);

Fl_Boxtype fl_define_FL_OXY_UP_BOX() {

  fl_internal_boxtype(_FL_OXY_UP_BOX, up_box);
  fl_internal_boxtype(_FL_OXY_DOWN_BOX, down_box);
  fl_internal_boxtype(_FL_OXY_UP_FRAME, up_frame);
  fl_internal_boxtype(_FL_OXY_DOWN_FRAME, down_frame);
  fl_internal_boxtype(_FL_OXY_THIN_UP_BOX, thin_up_box);
  fl_internal_boxtype(_FL_OXY_THIN_DOWN_BOX, thin_down_box);
  fl_internal_boxtype(_FL_OXY_THIN_UP_FRAME, thin_up_frame);
  fl_internal_boxtype(_FL_OXY_THIN_DOWN_FRAME, thin_down_frame);
  fl_internal_boxtype(_FL_OXY_ROUND_UP_BOX, round_up_box, fl_round_focus);
  fl_internal_boxtype(_FL_OXY_ROUND_DOWN_BOX, round_down_box, fl_round_focus);
  fl_internal_boxtype(_FL_OXY_BUTTON_UP_BOX, button_up_box);
  fl_internal_boxtype(_FL_OXY_BUTTON_DOWN_BOX, button_down_box);

  return _FL_OXY_UP_BOX;
}
