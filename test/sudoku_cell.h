//
// Sudoku game cell using the Fast Light Tool Kit (FLTK).
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

#ifndef _SUDOKU_CELL_H_
#define _SUDOKU_CELL_H_

#include <FL/Fl_Button.H>

// Sudoku cell class...
class SudokuCell : public Fl_Widget {
  bool          readonly_;
  int           value_;
  int           marks_;

  public:

                SudokuCell(int X, int Y, int W, int H);
  void  draw() FL_OVERRIDE;
  int   handle(int event) FL_OVERRIDE;
  void          readonly(bool r) { readonly_ = r; redraw(); }
  bool          readonly() const { return readonly_; }
  void mark(int n, bool set);
  void toggle_mark(int n);
  bool mark(int n);
  void clear_marks();
  void value(int v) {
    value_ = v;
    clear_marks();
    redraw();
  }
  int           value() const { return value_; }
};


#endif // _SUDOKU_CELL_H_
