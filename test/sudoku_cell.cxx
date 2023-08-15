//
// Sudoku game using the Fast Light Tool Kit (FLTK).
//
// Copyright 2005-2018 by Michael Sweet.
// Copyright 2019-2021 by Bill Spitzak and others.
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

#include "sudoku_cell.h"
#include "sudoku.h"

#include <FL/fl_draw.H>
#include <FL/fl_ask.H>

// Create a cell widget
SudokuCell::SudokuCell(int X, int Y, int W, int H)
: Fl_Widget(X, Y, W, H, 0),
  marks_(0)
{
  value(0);
}


// Draw cell
void
SudokuCell::draw() {
  static Fl_Align align[10] = {
    0,
    FL_ALIGN_TOP_LEFT,
    FL_ALIGN_TOP,
    FL_ALIGN_TOP_RIGHT,
    FL_ALIGN_LEFT,
    0,
    FL_ALIGN_RIGHT,
    FL_ALIGN_BOTTOM_LEFT,
    FL_ALIGN_BOTTOM,
    FL_ALIGN_BOTTOM_RIGHT,
  };


  // Draw the cell box...
  fl_draw_box(FL_BORDER_BOX, x(), y(), w(), h(), color());

  // Draw the cell background...
  if (Fl::focus() == this) {
    Fl_Color c = fl_color_average(FL_SELECTION_COLOR, color(), 0.5f);
    fl_color(c);
    fl_rectf(x() + 4, y() + 4, w() - 8, h() - 8);
    fl_color(fl_contrast(labelcolor(), c));
  } else fl_color(labelcolor());

  // Draw the cell value...
  char s[2];

  s[1] = '\0';

  if (value_) {
    s[0] = value_ + '0';

    if (readonly())
      fl_font(FL_HELVETICA_BOLD, h() - 10);
    else
      fl_font(FL_HELVETICA, h() - 10);
    fl_draw(s, x(), y(), w(), h(), FL_ALIGN_CENTER);
  }

  fl_font(FL_HELVETICA_BOLD, h()*2/9);

  for (int i = 1; i <= 9; i ++) {
    if (mark(i)) {
      s[0] = i + '0';
      fl_draw(s, x() + 5, y() + 5, w() - 10, h() - 10, align[i]);
    }
  }
}


// Handle events in cell
int
SudokuCell::handle(int event) {
  switch (event) {
    case FL_FOCUS :
      Fl::focus(this);
      redraw();
      return 1;

    case FL_UNFOCUS :
      redraw();
      return 1;

    case FL_PUSH :
      if (!readonly() && Fl::event_inside(this)) {
        if (Fl::event_clicks()) {
          // 2+ clicks increments/sets value
          if (value()) {
            if (value() < 9) value(value() + 1);
            else value(1);
          } else value(((Sudoku *)window())->next_value(this));
        }

        Fl::focus(this);
        redraw();
        return 1;
      }
      break;

    case FL_KEYDOWN :
      if (Fl::event_state() & FL_CTRL) break;
      int key = Fl::event_key() - '0';
      if (key < 0 || key > 9) key = Fl::event_key() - FL_KP - '0';
      if (key > 0 && key <= 9) {
        if (readonly()) {
          fl_beep(FL_BEEP_ERROR);
          return 1;
        }

        if (Fl::event_state() & (FL_SHIFT | FL_CAPS_LOCK)) {
          toggle_mark(key);
          value_ = 0;
          redraw();
        } else {
          value(key);
          do_callback();
        }
        return 1;
      } else if (key == 0 || Fl::event_key() == FL_BackSpace ||
                 Fl::event_key() == FL_Delete) {
        if (readonly()) {
          fl_beep(FL_BEEP_ERROR);
          return 1;
        }

        value(0);
        do_callback();
        return 1;
      }
      break;
  }

  return Fl_Widget::handle(event);
}

void SudokuCell::mark(int n, bool set) {
  if (n<1 || n>9) return;
  if (set) {
    marks_ |= (1<<n);
  } else {
    marks_ &= ~(1<<n);
  }
}

void SudokuCell::toggle_mark(int n) {
  if (n<1 || n>9) return;
  marks_ ^= (1<<n);
}

bool SudokuCell::mark(int n) {
  if (n<1 || n>9) return 0;
  return (marks_>>n) & 1;
}

void SudokuCell::clear_marks() {
  marks_ = 0;
}



