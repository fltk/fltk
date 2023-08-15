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

#include "sudoku_puzzle.h"
#include "sudoku.h"

#include <FL/fl_draw.H>

SudokuPuzzle::SudokuPuzzle(int x, int y, int w, int h, const char *label)
: Fl_Group(x, y, w, h, label)
{
  box(FL_BORDER_BOX);
}

#if 0
// A Sudoku (i.e. the puzzle) is a partially completed grid. A grid has 9 rows,
// 9 columns and 9 boxes, each having 9 cells (81 total). Boxes can also be
// called blocks or regions.[1] Three horizontally adjacent blocks are a band,
// and three vertically adjacent blocks are a stack.[2] The initially defined
// values are clues or givens. An ordinary Sudoku (i.e. a proper Sudoku) has
// one solution. Rows, columns and regions can be collectively referred to as
// groups, of which the grid has 27. The One Rule encapsulates the three prime
// rules, i.e. each digit (or number) can occur only once in each row, column,
// and box; and can be compactly stated as: "Each digit appears once in each
// group."
//  - wikipedia.org

// Wishlist:
// - [ ] new easy to new hard, etc.
// - [ ] store current puzzle in preferences
// - [ ] undo, redo
// - [ ] update hints now
// - [ ] always update hints
// - [ ] highlight row, column, and box
// - [ ] highlight other cells with same value
// - [ ] verify current solution
// - [ ] hint/flag/note vs. solve mode and button 1..9, erase
// - [ ] gift one field
// - [ ] bg is white and bright blue
// - [ ] selected field bg is green, boxed
// - [ ] same number is yellow
// - [ ] conflicts can use arrows and crosses
// - [ ] fixed numbers are bold, user values are not
// - [ ] timer
// - [ ] game hamburge menu

#include "sudoku.h"
#include "sudoku_cell.h"
#include "sudoku_sound.h"
#include "sudoku_generator.h"

#include <FL/Fl.H>
#include <FL/Enumerations.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/fl_ask.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Help_Dialog.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Sys_Menu_Bar.H>
#include <FL/platform.H>
#include <FL/Fl_Image_Surface.H>
#include <FL/Fl_Bitmap.H>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <FL/math.h>

#include "pixmaps/sudoku.xbm"

//
// Default sizes...
//

#define GROUP_SIZE      160
#define CELL_SIZE       50
#define CELL_OFFSET     5
#ifdef __APPLE__
#  define MENU_OFFSET   0
#else
#  define MENU_OFFSET   25
#endif // __APPLE__


// Sudoku class globals...
Fl_Help_Dialog  *Sudoku::help_dialog_ = (Fl_Help_Dialog *)0;
Fl_Preferences  Sudoku::prefs_(Fl_Preferences::USER_L, "fltk.org", "sudoku");


// Create a Sudoku game window...
Sudoku::Sudoku()
  : Fl_Double_Window(GROUP_SIZE * 3, GROUP_SIZE * 3 + MENU_OFFSET, "Sudoku")
{
  int j, k;
  Fl_Group *g;
  SudokuCell *cell;
  static Fl_Menu_Item   items[] = {
    { "&Game", 0, 0, 0, FL_SUBMENU },
    { "&New Game", FL_COMMAND | 'n', new_cb, 0, FL_MENU_DIVIDER },
    { "&Check Game", FL_COMMAND | 'c', check_cb, 0, 0 },
    { "&Restart Game", FL_COMMAND | 'r', restart_cb, 0, 0 },
    { "&Solve Game", FL_COMMAND | 's', solve_cb, 0, FL_MENU_DIVIDER },
    { "&Update Helpers", FL_COMMAND | 'u', update_helpers_cb, 0, 0 },
    { "&Mute Sound", FL_COMMAND | 'm', mute_cb, 0, FL_MENU_TOGGLE | FL_MENU_DIVIDER },
    { "&Quit", FL_COMMAND | 'q', close_cb, 0, 0 },
    { 0 },
    { "&Difficulty", 0, 0, 0, FL_SUBMENU },
    { "&Easy", 0, diff_cb, (void *)"0", FL_MENU_RADIO },
    { "&Medium", 0, diff_cb, (void *)"1", FL_MENU_RADIO },
    { "&Hard", 0, diff_cb, (void *)"2", FL_MENU_RADIO },
    { "&Impossible", 0, diff_cb, (void *)"3", FL_MENU_RADIO },
    { 0 },
    { "&Help", 0, 0, 0, FL_SUBMENU },
    { "&About Sudoku", FL_F + 1, help_cb, 0, 0 },
    { 0 },
    { 0 }
  };


  // Setup sound output...
  prefs_.get("mute_sound", j, 0);
  if (j) {
    // Mute sound?
    sound_ = NULL;
    items[6].flags |= FL_MENU_VALUE;
  } else sound_ = new SudokuSound();

  // Menubar...
  prefs_.get("difficulty", difficulty_, 0);
  if (difficulty_ < 0 || difficulty_ > 3) difficulty_ = 0;

  items[10 + difficulty_].flags |= FL_MENU_VALUE;

  menubar_ = new Fl_Sys_Menu_Bar(0, 0, 3 * GROUP_SIZE, 25);
  menubar_->menu(items);

  // Create the grids...
  grid_ = new Fl_Group(0, MENU_OFFSET, 3 * GROUP_SIZE, 3 * GROUP_SIZE);

  for (j = 0; j < 3; j ++)
    for (k = 0; k < 3; k ++) {
      g = new Fl_Group(k * GROUP_SIZE, j * GROUP_SIZE + MENU_OFFSET,
                       GROUP_SIZE, GROUP_SIZE);
      g->box(FL_BORDER_BOX);
      if ((int)(j == 1) ^ (int)(k == 1)) g->color(FL_DARK3);
      else g->color(FL_DARK2);
      g->end();

      grid_groups_[j][k] = g;
    }

  for (j = 0; j < 9; j ++)
    for (k = 0; k < 9; k ++) {
      cell = new SudokuCell(k * CELL_SIZE + CELL_OFFSET +
                                (k / 3) * (GROUP_SIZE - 3 * CELL_SIZE),
                            j * CELL_SIZE + CELL_OFFSET + MENU_OFFSET +
                                (j / 3) * (GROUP_SIZE - 3 * CELL_SIZE),
                            CELL_SIZE, CELL_SIZE);
      cell->callback(reset_cb);
      grid_cells_[j][k] = cell;
    }

  // Set icon for window
  Fl_Bitmap bm(sudoku_bits, sudoku_width, sudoku_height);
  Fl_Image_Surface surf(sudoku_width, sudoku_height, 1);
  Fl_Surface_Device::push_current(&surf);
  fl_color(FL_WHITE);
  fl_rectf(0, 0, sudoku_width, sudoku_height);
  fl_color(FL_BLACK);
  bm.draw(0, 0);
  Fl_Surface_Device::pop_current();
  icon(surf.image());

  // Catch window close events...
  callback(close_cb);

  // Make the window resizable...
  resizable(grid_);
  size_range(3 * GROUP_SIZE, 3 * GROUP_SIZE + MENU_OFFSET, 0, 0, 5, 5, 1);

  // Restore the previous window dimensions...
  int X, Y, W, H;

  if (prefs_.get("x", X, -1)) {
    prefs_.get("y", Y, -1);
    prefs_.get("width", W, 3 * GROUP_SIZE);
    prefs_.get("height", H, 3 * GROUP_SIZE + MENU_OFFSET);

    resize(X, Y, W, H);
  }

  set_title();
}


// Destroy the sudoku window...
Sudoku::~Sudoku() {
  if (sound_) delete sound_;
}


// Check for a solution to the game...
void
Sudoku::check_cb(Fl_Widget *widget, void *) {
  ((Sudoku *)(widget->window()))->check_game();
}


// Check if the user has correctly solved the game...
void
Sudoku::check_game(bool highlight) {
  bool empty = false;
  bool correct = true;
  int j, k, m;

  // Check the game for right/wrong answers...
  for (j = 0; j < 9; j ++)
    for (k = 0; k < 9; k ++) {
      SudokuCell *cell = grid_cells_[j][k];
      int val = cell->value();

      if (cell->readonly()) continue;

      if (!val) empty = true;
      else {
        for (m = 0; m < 9; m ++)
          if ((j != m && grid_cells_[m][k]->value() == val) ||
              (k != m && grid_cells_[j][m]->value() == val)) break;

        if (m < 9) {
          if (highlight) {
            cell->color(FL_YELLOW);
            cell->redraw();
          }

          correct = false;
        } else if (highlight) {
          cell->color(FL_LIGHT3);
          cell->redraw();
        }
      }
    }

  // Check subgrids for duplicate numbers...
  for (j = 0; j < 9; j += 3)
    for (k = 0; k < 9; k += 3)
      for (int jj = 0; jj < 3; jj ++)
        for (int kk = 0; kk < 3; kk ++) {
          SudokuCell *cell = grid_cells_[j + jj][k + kk];
          int val = cell->value();

          if (cell->readonly() || !val) continue;

          int jjj;

          for (jjj = 0; jjj < 3; jjj ++) {
            int kkk;

            for (kkk = 0; kkk < 3; kkk ++)
              if (jj != jjj && kk != kkk &&
                  grid_cells_[j + jjj][k + kkk]->value() == val) break;

            if (kkk < 3) break;
          }

          if (jjj < 3) {
            if (highlight) {
              cell->color(FL_YELLOW);
              cell->redraw();
            }

            correct = false;
          }
        }

  if (!empty && correct) {
    // Success!
    for (j = 0; j < 9; j ++) {
      for (k = 0; k < 9; k ++) {
        SudokuCell *cell = grid_cells_[j][k];
        cell->color(FL_GREEN);
        cell->readonly(1);
      }

      if (sound_) sound_->play('A' + grid_cells_[j][8]->value() - 1);
    }
  }
}


// Close the window, saving the game first...
void
Sudoku::close_cb(Fl_Widget *widget, void *) {
  Sudoku *s = (Sudoku *)(widget->window() ? widget->window() : widget);

  s->save_game();
  s->hide();

  if (help_dialog_) help_dialog_->hide();
}


// Set the level of difficulty...
void
Sudoku::diff_cb(Fl_Widget *widget, void *d) {
  Sudoku *s = (Sudoku *)(widget->window() ? widget->window() : widget);
  int diff = atoi((char *)d);

  if (diff != s->difficulty_) {
    s->difficulty_ = diff;
    s->new_game(s->seed_);
    s->set_title();

    if (diff > 1)
    {
      // Display a message about the higher difficulty levels for the
      // Sudoku zealots of the world...
      int val;

      prefs_.get("difficulty_warning", val, 0);

      if (!val)
      {
        prefs_.set("difficulty_warning", 1);
        fl_alert("Note: 'Hard' and 'Impossible' puzzles may have more than "
                 "one possible solution.\n"
                 "This is not an error or bug.");
      }
    }

    prefs_.set("difficulty", s->difficulty_);
  }
}

// Update the little marker numbers in all cells
void
Sudoku::update_helpers_cb(Fl_Widget *widget, void *) {
  Sudoku *s = (Sudoku *)(widget->window() ? widget->window() : widget);
  s->update_helpers();
}

void
Sudoku::update_helpers() {
  int j, k, m;

  // First we delete any entries that the user may have made
  // TODO: set all marks if none were set
  // TODO: clear marks if value is used, don't set individual marks
  for (j = 0; j < 9; j ++) {
    for (k = 0; k < 9; k ++) {
      SudokuCell *cell = grid_cells_[j][k];
      cell->clear_marks();
    }
  }

  // Now go through all cells and find out, what we can not be
  for (j = 0; j < 81; j ++) {
    char taken[10] = { 0 };
    // Find our destination cell
    int row = j / 9;
    int col = j % 9;
    SudokuCell *dst_cell = grid_cells_[row][col];
    if (dst_cell->value()) continue;
    // Find all values already taken in this row
    for (k = 0; k < 9; k ++) {
      SudokuCell *cell = grid_cells_[row][k];
      int v = cell->value();
      if (v) taken[v] = 1;
    }
    // Find all values already taken in this column
    for (k = 0; k < 9; k ++) {
      SudokuCell *cell = grid_cells_[k][col];
      int v = cell->value();
      if (v) taken[v] = 1;
    }
    // Now find all values already taken in this square
    int ro = (row / 3) * 3;
    int co = (col / 3) * 3;
    for (k = 0; k < 3; k ++) {
      for (m = 0; m < 3; m ++) {
        SudokuCell *cell = grid_cells_[ro + k][co + m];
        int v = cell->value();
        if (v) taken[v] = 1;
      }
    }
    // transfer our findings to the markers
    for (m = 1; m <= 9; m ++) {
      if (!taken[m])
        dst_cell->mark(m, true);
    }
    dst_cell->redraw();
  }
}


// Show the on-line help...
void
Sudoku::help_cb(Fl_Widget *, void *) {
  if (!help_dialog_) {
    help_dialog_ = new Fl_Help_Dialog();

    help_dialog_->value(
        "<HTML>\n"
        "<HEAD>\n"
        "<TITLE>Sudoku Help</TITLE>\n"
        "</HEAD>\n"
        "<BODY BGCOLOR='#ffffff'>\n"

        "<H2>About the Game</H2>\n"

        "<P>Sudoku (pronounced soo-dough-coo with the emphasis on the\n"
        "first syllable) is a simple number-based puzzle/game played on a\n"
        "9x9 grid that is divided into 3x3 subgrids. The goal is to enter\n"
        "a number from 1 to 9 in each cell so that each number appears\n"
        "only once in each column and row. In addition, each 3x3 subgrid\n"
        "may only contain one of each number.</P>\n"

        "<P>This version of the puzzle is copyright 2005-2010 by Michael R\n"
        "Sweet.</P>\n"

        "<P><B>Note:</B> The 'Hard' and 'Impossible' difficulty\n"
        "levels generate Sudoku puzzles with multiple possible solutions.\n"
        "While some purists insist that these cannot be called 'Sudoku'\n"
        "puzzles, the author (me) has personally solved many such puzzles\n"
        "in published/printed Sudoku books and finds them far more\n"
        "interesting than the simple single solution variety. If you don't\n"
        "like it, don't play with the difficulty set to 'High' or\n"
        "'Impossible'.</P>\n"

        "<H2>How to Play the Game</H2>\n"

        "<P>At the start of a new game, Sudoku fills in a random selection\n"
        "of cells for you - the number of cells depends on the difficulty\n"
        "level you use. Click in any of the empty cells or use the arrow\n"
        "keys to highlight individual cells and press a number from 1 to 9\n"
        "to fill in the cell. To clear a cell, press 0, Delete, or\n"
        "Backspace. When you have successfully completed all subgrids, the\n"
        "entire puzzle is highlighted in green until you start a new\n"
        "game.</P>\n"

        "<P>As you work to complete the puzzle, you can display possible\n"
        "solutions inside each cell by holding the Shift key and pressing\n"
        "each number in turn. Repeat the process to remove individual\n"
        "numbers, or press a number without the Shift key to replace them\n"
        "with the actual number to use.</P>\n"
        "</BODY>\n"
    );
  }

  help_dialog_->show();
}


// Load the game from saved preferences...
void
Sudoku::load_game() {

  // Load the current values and state of each grid...
  memset(grid_values_, 0, sizeof(grid_values_));

  bool solved = true;
  bool empty = false;

  for (int j = 0; j < 9; j ++) {
    Fl_Preferences row(prefs_, Fl_Preferences::Name("Row%d", j));
    for (int k = 0; k < 9; k ++) {
      Fl_Preferences p(row, Fl_Preferences::Name("Col%d", k));
      int v;

      SudokuCell *cell = grid_cells_[j][k];

      p.get("value", v, 0);
      grid_values_[j][k] = v;
      if (v) empty = false;

      p.get("state", v, 0);
      cell->value(v);

      p.set("readonly", cell->readonly());
      cell->readonly(v != 0);
      if (v) {
        cell->color(FL_GRAY);
      } else {
        cell->color(FL_LIGHT3);
        solved = false;
      }

      for (int m = 1; m <= 9; m ++) {
        p.get(Fl_Preferences::Name("m%d", m), v, 0);
        cell->mark(m, v);
      }
      cell->redraw();
    }
  }

  // If we didn't load any values or the last game was solved, then
  // create a new game automatically...
  if (solved || !grid_values_[0][0]) new_game(time(NULL));
  else check_game(false);
}


// Mute/unmute sound...
void
Sudoku::mute_cb(Fl_Widget *widget, void *) {
  Sudoku *s = (Sudoku *)(widget->window() ? widget->window() : widget);

  if (s->sound_) {
    delete s->sound_;
    s->sound_ = NULL;
    prefs_.set("mute_sound", 1);
  } else {
    s->sound_ = new SudokuSound();
    prefs_.set("mute_sound", 0);
  }
}


// Create a new game...
void
Sudoku::new_cb(Fl_Widget *widget, void *) {
  Sudoku *s = (Sudoku *)(widget->window() ? widget->window() : widget);

//  if (s->grid_cells_[0][0]->color() != FL_GREEN) {
//    if (!fl_choice("Are you sure you want to change the difficulty level and "
//                   "discard the current game?", "Keep Current Game", "Start New Game",
//                   NULL)) return;
//  }

  s->new_game(time(NULL));
}

// Create a new game...
void
Sudoku::new_game(time_t seed) {

  {
    int grid_data[81];
    int *g = grid_data;
    generate_sudoku(grid_data, 22, 31);
    SudokuCell *cell;
    for (int j = 0; j < 9; j ++) {
      for (int k = 0; k < 9; k ++) {
        int v = *g++;
        int vv = v; if (vv<0) vv = -vv;
        grid_values_[j][k] = vv;
        cell = grid_cells_[j][k];
        if (v<0) {
          cell->value(0);
          cell->readonly(0);
          cell->color(FL_LIGHT3);
        } else {
          cell->value(vv);
          cell->readonly(1);
          cell->color(FL_GRAY);
        }
      }
    }
    return;
  }




  int j, k, m, n, t, count;


  // Generate a new (valid) Sudoku grid...
  seed_ = seed;
  srand((unsigned int)seed);

  memset(grid_values_, 0, sizeof(grid_values_));

  for (j = 0; j < 9; j += 3) {
    for (k = 0; k < 9; k += 3) {
      for (t = 1; t <= 9; t ++) {
        for (count = 0; count < 20; count ++) {
          m = j + (rand() % 3);
          n = k + (rand() % 3);
          if (!grid_values_[m][n]) {
            int mm;

            for (mm = 0; mm < m; mm ++)
              if (grid_values_[mm][n] == t) break;

            if (mm < m) continue;

            int nn;

            for (nn = 0; nn < n; nn ++)
              if (grid_values_[m][nn] == t) break;

            if (nn < n) continue;

            grid_values_[m][n] = t;
            break;
          }
        }

        if (count == 20) {
          // Unable to find a valid puzzle so far, so start over...
          k = 9;
          j = -3;
          memset(grid_values_, 0, sizeof(grid_values_));
        }
      }
    }
  }

  // Start by making all cells editable
  SudokuCell *cell;

  for (j = 0; j < 9; j ++)
    for (k = 0; k < 9; k ++) {
      cell = grid_cells_[j][k];

      cell->value(0);
      cell->readonly(0);
      cell->color(FL_LIGHT3);
    }

  // Show N cells...
  count = 11 * (5 - difficulty_);

  int numbers[9];

  for (j = 0; j < 9; j ++) numbers[j] = j + 1;

  while (count > 0) {
    for (j = 0; j < 20; j ++) {
      k          = rand() % 9;
      m          = rand() % 9;
      t          = numbers[k];
      numbers[k] = numbers[m];
      numbers[m] = t;
    }

    for (j = 0; count > 0 && j < 9; j ++) {
      t = numbers[j];

      for (k = 0; count > 0 && k < 9; k ++) {
        cell = grid_cells_[j][k];

        if (grid_values_[j][k] == t && !cell->readonly()) {
          cell->value(grid_values_[j][k]);
          cell->readonly(1);
          cell->color(FL_GRAY);

          count --;
          break;
        }
      }
    }
  }
}


// Return the next available value for a cell...
int
Sudoku::next_value(SudokuCell *c) {
  int   j = 0, k = 0, m = 0, n = 0;


  for (j = 0; j < 9; j ++) {
    for (k = 0; k < 9; k ++)
      if (grid_cells_[j][k] == c) break;

    if (k < 9) break;
  }

  if (j == 9) return 1;

  j -= j % 3;
  k -= k % 3;

  int numbers[9];

  memset(numbers, 0, sizeof(numbers));

  for (m = 0; m < 3; m ++)
    for (n = 0; n < 3; n ++) {
      c = grid_cells_[j + m][k + n];
      if (c->value()) numbers[c->value() - 1] = 1;
    }

  for (j = 0; j < 9; j ++)
    if (!numbers[j]) return j + 1;

  return 1;
}


// Reset widget color to gray...
void
Sudoku::reset_cb(Fl_Widget *widget, void *) {
  widget->color(FL_LIGHT3);
  widget->redraw();

  ((Sudoku *)(widget->window()))->check_game(false);
}


// Resize the window...
void
Sudoku::resize(int X, int Y, int W, int H) {
  // Resize the window...
  Fl_Double_Window::resize(X, Y, W, H);

  // Save the new window geometry...
  prefs_.set("x", X);
  prefs_.set("y", Y);
  prefs_.set("width", W);
  prefs_.set("height", H);
}


// Restart game from beginning...
void
Sudoku::restart_cb(Fl_Widget *widget, void *) {
  Sudoku *s = (Sudoku *)(widget->window());
  bool solved = true;

  for (int j = 0; j < 9; j ++)
    for (int k = 0; k < 9; k ++) {
      SudokuCell *cell = s->grid_cells_[j][k];

      if (!cell->readonly()) {
        solved = false;
        int v = cell->value();
        cell->value(0);
        cell->color(FL_LIGHT3);
        if (v && s->sound_) s->sound_->play('A' + v - 1);
      }
    }

  if (solved) s->new_game(s->seed_);
}


// Save the current game state...
void
Sudoku::save_game() {
  // Save the current values and state of each grid...
  for (int j = 0; j < 9; j ++) {
    Fl_Preferences row(prefs_, Fl_Preferences::Name("Row%d", j));
    for (int k = 0; k < 9; k ++) {
      Fl_Preferences p(row, Fl_Preferences::Name("Col%d", k));
      char name[255];
      SudokuCell *cell = grid_cells_[j][k];
      p.set("value", grid_values_[j][k]);
      p.set("state", cell->value());
      p.set("readonly", cell->readonly());
      for (int m = 1; m <= 9; m ++) {
        if (cell->mark(m))
          p.set(Fl_Preferences::Name("m%d", m), 1);
        else
          p.deleteEntry(Fl_Preferences::Name("m%d", m));
      }
    }
  }
}


// Set title of window...
void
Sudoku::set_title() {
  static const char * const titles[] = {
    "Sudoku - Easy",
    "Sudoku - Medium",
    "Sudoku - Hard",
    "Sudoku - Impossible"
  };

  label(titles[difficulty_]);
}


// Solve the puzzle...
void
Sudoku::solve_cb(Fl_Widget *widget, void *) {
  ((Sudoku *)(widget->window()))->solve_game();
}


// Solve the puzzle...
void
Sudoku::solve_game() {
  int j, k;

  for (j = 0; j < 9; j ++) {
    for (k = 0; k < 9; k ++) {
      SudokuCell *cell = grid_cells_[j][k];

      cell->value(grid_values_[j][k]);
      cell->readonly(1);
      cell->color(FL_GRAY);
    }

    if (sound_) sound_->play('A' + grid_cells_[j][8]->value() - 1);
  }
}


// Main entry for game...
// Note 21-17 (proven minimum) clues can be set
// easy: 30-36
// expert: 25-30
// algo: 22 (rare) to 25

// extremely easy: 46+
// easy: 36-46
// medium: 32-35
// difficult: 28-31
// evil: 17-27

int
main(int argc, char *argv[]) {
  Sudoku s;

  // Show the game...
  s.show(argc, argv);

  // Load the previous game...
  s.load_game();

  // Run until the user quits...
  return (Fl::run());
}

#endif
