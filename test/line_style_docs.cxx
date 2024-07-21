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

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Grid.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>

// constants

static const int len = 35;          // length of line segments
static const int sep = 15;          // separation between items
static const int width[] = {0, 4};  // line widths (thin + thick)

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
    int X = x() + sep / 2;
    int Y = y() + (h() - len) / 2;
    draw_box();
    fl_font(FL_HELVETICA, 12);
    fl_color(FL_BLACK);
    // draw the text
    fl_draw(style_str(style), X, y() + h()/2 + fl_height()/2 - 2);
    X += 110;
    for (int i = 0; i < 2; i++, X += len + sep) { // thin + thick lines
      fl_line_style(style, width[i]);
      // ___
      //    |
      //    |
      fl_line(X, Y, X + len, Y, X + len, Y + len);
      X += len + sep;
      // ___
      //   /
      //  /
      fl_line(X, Y, X + len, Y, X, Y + len);
    }
    fl_line_style(FL_SOLID, 0); // restore to default
  }
};

int main(int argc, char **argv) {
  Fl_Double_Window win(660, 340, "fl_line_style()");
  win.color(FL_WHITE);
  Fl_Grid grid(4, 4, win.w() - 8, win.h() - 8);
  grid.box(FL_FLAT_BOX);
  grid.color(0xd0d0d000);   // margins and gaps
  grid.layout(6, 2, 4, 4);  // 6 rows, 2 columns

  // first column
  StyleBox sb00(FL_SOLID,      0, 0);
  StyleBox sb01(FL_DASH,       1, 0);
  StyleBox sb02(FL_DOT,        2, 0);
  StyleBox sb03(FL_DASHDOT,    3, 0);
  StyleBox sb04(FL_DASHDOTDOT, 4, 0);

  // empty box in row 5
  Fl_Box empty(0, 0, 0, 0);
  empty.box(FL_FLAT_BOX);
  empty.color(FL_WHITE);
  grid.widget(&empty, 5, 0, FL_GRID_FILL);

  // second column
  StyleBox sb05(FL_CAP_FLAT,   0, 1);
  StyleBox sb06(FL_CAP_ROUND,  1, 1);
  StyleBox sb07(FL_CAP_SQUARE, 2, 1);
  StyleBox sb08(FL_JOIN_MITER, 3, 1);
  StyleBox sb09(FL_JOIN_ROUND, 4, 1);
  StyleBox sb10(FL_JOIN_BEVEL, 5, 1);

  grid.end();
  win.end();
  // win.resizable(win);
  // win.size_range(win.w(), win.h()); // don't allow to shrink
  win.show(argc, argv);
  return Fl::run();
}
