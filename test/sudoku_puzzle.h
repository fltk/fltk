//
// Sudoku game puzzle using the Fast Light Tool Kit (FLTK).
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

#ifndef _SUDOKU_PUZZLE_H_
#define _SUDOKU_PUZZLE_H_

#include <FL/Fl_Group.H>

class SudokuPuzzle : public Fl_Group {
public:
  SudokuPuzzle(int x, int y, int w, int h, const char *label=NULL);
};

#if 0
class SudokuCell;
class SudokuSound;
class Fl_Sys_Menu_Bar;
class Fl_Help_Dialog;

// Sudoku window class...
class Sudoku : public Fl_Double_Window {
  Fl_Sys_Menu_Bar *menubar_;
  Fl_Group      *grid_;
  time_t        seed_;
  char          grid_values_[9][9];
  SudokuCell    *grid_cells_[9][9];
  Fl_Group      *grid_groups_[3][3];
  int           difficulty_;
  SudokuSound   *sound_;

  static void   check_cb(Fl_Widget *widget, void *);
  static void   close_cb(Fl_Widget *widget, void *);
  static void   diff_cb(Fl_Widget *widget, void *d);
  static void   update_helpers_cb(Fl_Widget *, void *);
  static void   help_cb(Fl_Widget *, void *);
  static void   mute_cb(Fl_Widget *widget, void *);
  static void   new_cb(Fl_Widget *widget, void *);
  static void   reset_cb(Fl_Widget *widget, void *);
  static void   restart_cb(Fl_Widget *widget, void *);
  void          set_title();
  static void   solve_cb(Fl_Widget *widget, void *);

  static Fl_Help_Dialog *help_dialog_;
  static Fl_Preferences prefs_;
  public:

                Sudoku();
                ~Sudoku();

  void          check_game(bool highlight = true);
  void          load_game();
  void          new_game(time_t seed);
  int           next_value(SudokuCell *c);
  void  resize(int X, int Y, int W, int H) FL_OVERRIDE;
  void          save_game();
  void          solve_game();
  void          update_helpers();
};

#endif

#endif // _SUDOKU_PUZZLE_H_
