//
// Line style image (docs) for the Fast Light Tool Kit (FLTK).
//
// Copyright 2024 by Bill Spitzak and others.
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

// Notes to devs (and users):
//
// 1. Run this program to create the screenshot for the fl_line_style() docs.
//    Save a screenshot of its original size to documentation/src/fl_line_style.png
// 2. For further tests it's possible to resize the window. Line sizes and widths
//    are adjusted (resized) as well, depending on the window size.
// 3. Some lines may draw outside their boxes in unusual window width/height ratios.
//    These effects are intentionally ignored.

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Grid.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>

// constants

static const int sep = 22;    // separation between items
static int width[] = {1, 4};  // line widths (thin + thick/dyn.)

// This class draws a box with one line style inside an Fl_Grid widget.
// Row and column parameters are used to position the box inside the grid.

class StyleBox : public Fl_Box {
  int style;              // line style
public:
  StyleBox(int S, int row, int col) // style, row, column
    : Fl_Box(0, 0, 0, 0) {
    box(FL_FLAT_BOX);
    color(FL_WHITE);
    style = S;
    Fl_Grid *grid = (Fl_Grid *)parent();
    grid->widget(this, row, col, FL_GRID_FILL);
  }
  const char *style_str(int style) {
    switch(style) {
      case FL_SOLID       : return "FL_SOLID"      ;
      case FL_DASH        : return "FL_DASH"       ;
      case FL_DOT         : return "FL_DOT"        ;
      case FL_DASHDOT     : return "FL_DASHDOT"    ;
      case FL_DASHDOTDOT  : return "FL_DASHDOTDOT" ;
      case FL_CAP_FLAT    : return "FL_CAP_FLAT"   ;
      case FL_CAP_ROUND   : return "FL_CAP_ROUND"  ;
      case FL_CAP_SQUARE  : return "FL_CAP_SQUARE" ;
      case FL_JOIN_MITER  : return "FL_JOIN_MITER" ;
      case FL_JOIN_ROUND  : return "FL_JOIN_ROUND" ;
      case FL_JOIN_BEVEL  : return "FL_JOIN_BEVEL" ;
      default             : return "(?)";
    }
  }
  void draw() FL_OVERRIDE {
    draw_box();
    if (style < 0) // draw an empty box
      return;

    // set font and measure widest text
    fl_font(FL_HELVETICA, 12);
    fl_color(FL_BLACK);
    static int text_width = 0;
    if (!text_width) {
      int h = 0; // dummy
      fl_measure("FL_DASHDOTDOT", text_width, h);
    }

    // draw the text
    int X = x() + sep / 2;
    fl_draw(style_str(style), X, y() + h()/2 + fl_height()/2 - 2);

    // calculate dynamic line sizes and widths
    X += text_width + sep / 2;
    int dx = (w() - text_width - 5 * sep) / 4; // horizontal distance
    int dy = h() - sep;
    int Y = y() + sep / 2;
    if (dx >= 80 || dy >= 80)
      width[1] = 9;
    else if (dx >= 60 || dy >= 60)
      width[1] = 8;
    else if (dx >= 40 || dy >= 40)
      width[1] = 7;
    else
      width[1] = 5;

    // draw the lines
    for (int i = 0; i < 2; i++, X += dx + sep) { // thin + thick lines
      fl_line_style(style, width[i]);
      // ___
      //    |
      //    |
      fl_line(X, Y, X + dx, Y, X + dx, Y + dy);
      X += dx + sep;
      // ___
      //   /
      //  /
      fl_line(X, Y, X + dx, Y, X, Y + dy);
    }

    // restore line settings to default
    fl_line_style(FL_SOLID, 0);
  }
};

int main(int argc, char **argv) {
  Fl_Double_Window win(740, 400, "fl_line_style()");
  win.color(FL_WHITE);

  // create grid with a nice white 4px border and a
  // light gray background (color) so margins and gaps show thru
  Fl_Grid grid(4, 4, win.w() - 8, win.h() - 8);
  grid.box(FL_FLAT_BOX);
  grid.color(0xd0d0d000);
  grid.layout(6, 2, 4, 4);  // 6 rows, 2 columns, ...

  // first column
  StyleBox sb00(FL_SOLID,      0, 0);
  StyleBox sb01(FL_DASH,       1, 0);
  StyleBox sb02(FL_DOT,        2, 0);
  StyleBox sb03(FL_DASHDOT,    3, 0);
  StyleBox sb04(FL_DASHDOTDOT, 4, 0);
  StyleBox sb05(-1,            5, 0); // empty box

  // second column
  StyleBox sb10(FL_CAP_FLAT,   0, 1);
  StyleBox sb11(FL_CAP_ROUND,  1, 1);
  StyleBox sb12(FL_CAP_SQUARE, 2, 1);
  StyleBox sb13(FL_JOIN_MITER, 3, 1);
  StyleBox sb14(FL_JOIN_ROUND, 4, 1);
  StyleBox sb15(FL_JOIN_BEVEL, 5, 1);

  grid.end();
  win.end();
  win.resizable(win);
  win.size_range(660, 340); // don't allow to shrink too much
  win.show(argc, argv);
  return Fl::run();
}
