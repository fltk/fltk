//
// Lighted button widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2023 by Bill Spitzak and others.
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

// Subclass of Fl_Button where the "box" indicates whether it is
// pushed or not, and the "down box" is drawn small and square on
// the left to indicate the current state.

// The default down_box of zero draws a rectangle designed to look
// just like Flame's buttons.

#include <FL/Fl.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Radio_Light_Button.H>
#include <FL/fl_draw.H>
#include "flstring.h"

void Fl_Light_Button::draw() {
  if (box()) draw_box(this==Fl::pushed() ? fl_down(box()) : box(), color());
  Fl_Color col = value() ? (active_r() ? selection_color() :
                            fl_inactive(selection_color())) : color();

  // determine the color of the check mark or radio button (circle)

  Fl_Color check_color = selection_color(); // default = selection color
  if (Fl::is_scheme("gtk+"))
    check_color = FL_SELECTION_COLOR;       // exception for gtk+
  if (!active_r())
    check_color = fl_inactive(check_color);
  // select a color with enough contrast
  check_color = fl_contrast(check_color, FL_BACKGROUND2_COLOR);

  int W  = labelsize();         // check mark box size
  if (W > 25) W = 25;           // limit box size
  int bx = Fl::box_dx(box());   // box frame width
  int dx = bx + 2;              // relative position of check mark box
  int dy = (h() - W) / 2;       // neg. offset o.k. for vertical centering
  int lx = 0;                   // relative label position (STR #3237)
  int cx = x() + dx;            // check mark box x-position
  int cy = y() + dy;            // check mark box y-position
  int cw = 0;                   // check mark box width and height

  // FIXME: the *box type* of the widget determines the drawing style:
  // (a) down_box() ==  0: Fl_Light_Button style
  // (b) down_box() !=  0: other button styles

  if (down_box()) {             // draw "other" button styles (b):
    switch (down_box()) {
      case FL_DOWN_BOX :
      case FL_UP_BOX :
      case _FL_PLASTIC_DOWN_BOX :
      case _FL_PLASTIC_UP_BOX :
        // Check box...
        draw_box(down_box(), cx, cy, W, W, FL_BACKGROUND2_COLOR);
        if (value()) {
          // Check mark...
          // Calculate box position and size
          cx += Fl::box_dx(down_box());
          cy += Fl::box_dy(down_box());
          cw = W - Fl::box_dw(down_box());
          fl_draw_check(Fl_Rect(cx, cy, cw, cw), check_color);
        }
        break;
      case _FL_ROUND_DOWN_BOX :
      case _FL_ROUND_UP_BOX :
        // Radio button...
        draw_box(down_box(), x()+dx, y()+dy, W, W, FL_BACKGROUND2_COLOR);
        if (value()) {

          // Draw round check mark of radio button

          int tW = (W - Fl::box_dw(down_box())) / 2 + 1;
          if ((W - tW) & 1) tW++; // Make sure difference is even to center
          int tdx = dx + (W - tW) / 2;
          int tdy = dy + (W - tW) / 2;
          fl_draw_radio(x() + tdx - 1, y() + tdy - 1, tW + 2, check_color);

        } // Radio button: if (value())
        break;
      default :
        draw_box(down_box(), x()+dx, y()+dy, W, W, col);
        break;
    }
    lx = dx + W + 2;
  } else {
    // if down_box() is zero, draw light button style (a):
    int hh = h()-2*dy - 2;
    int ww = W/2+1;
    int xx = dx;
    if (w()<ww+2*xx) xx = (w()-ww)/2;
    if (Fl::is_scheme("plastic")) {
      col = active_r() ? selection_color() : fl_inactive(selection_color());
      fl_color(value() ? col : fl_color_average(col, FL_BLACK, 0.5f));
      fl_pie(x()+xx, y()+dy+1, ww, hh, 0, 360);
    } else {
      draw_box(FL_THIN_DOWN_BOX, x()+xx, y()+dy+1, ww, hh, col);
    }
    lx = dx + ww + 2;
  }
  draw_label(x()+lx, y(), w()-lx-bx, h());
  if (Fl::focus() == this) draw_focus();
}

int Fl_Light_Button::handle(int event) {
  switch (event) {
  case FL_RELEASE:
    if (box()) redraw();
  default:
    return Fl_Button::handle(event);
  }
}

/**
  Creates a new Fl_Light_Button widget using the given
  position, size, and label string.

  The default box type is \p FL_UP_BOX and the default down box
  type down_box() is \p FL_NO_BOX (0).

  The selection_color() sets the color of the "light".
  Default is FL_YELLOW.

  The default label alignment is \p 'FL_ALIGN_LEFT|FL_ALIGN_INSIDE' so
  the label is drawn inside the button area right of the "light".

  \note Do not change the default box types of Fl_Light_Button. The
    box types determine how the button is drawn. If you change the
    down_box() type the drawing behavior is undefined.
*/
Fl_Light_Button::Fl_Light_Button(int X, int Y, int W, int H, const char* l)
: Fl_Button(X, Y, W, H, l) {
  type(FL_TOGGLE_BUTTON);
  selection_color(FL_YELLOW);
  align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
}


Fl_Radio_Light_Button::Fl_Radio_Light_Button(int X,int Y,int W,int H,const char *l)
: Fl_Light_Button(X,Y,W,H,l)
{
  type(FL_RADIO_BUTTON);
}
