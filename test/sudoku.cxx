//
// "$Id$"
//
// Sudoku game using the Fast Light Tool Kit (FLTK).
//
// Copyright 2005 by Michael Sweet.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

#include <FL/Fl.H>
#include <FL/Enumerations.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/fl_ask.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Help_Dialog.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Sys_Menu_Bar.H>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


//
// Default sizes...
//

#define GROUP_SIZE	160
#define CELL_SIZE	50
#define CELL_OFFSET	5
#ifdef __APPLE__
#  define MENU_OFFSET	0
#else
#  define MENU_OFFSET	25
#endif // __APPLE__


// Sudoku cell class...
class SudokuCell : public Fl_Widget {
  bool		readonly_;
  int		value_;
  int		test_value_[8];

  public:

		SudokuCell(int X, int Y, int W, int H);
  void		draw();
  int		handle(int event);
  void		readonly(bool r) { readonly_ = r; redraw(); }
  bool		readonly() const { return readonly_; }
  void		test_value(int v, int n) { test_value_[n] = v; redraw(); }
  int		test_value(int n) const { return test_value_[n]; }
  void		value(int v) {
		  value_ = v;
		  for (int i = 0; i < 8; i ++) test_value_[i] = 0;
		  redraw();
		}
  int		value() const { return value_; }
};

// Create a cell widget
SudokuCell::SudokuCell(int X, int Y, int W, int H)
  : Fl_Widget(X, Y, W, H, 0) {
  value(0);
}


// Draw cell
void
SudokuCell::draw() {
  static Fl_Align align[8] = {
    FL_ALIGN_TOP_LEFT,
    FL_ALIGN_TOP,
    FL_ALIGN_TOP_RIGHT,
    FL_ALIGN_RIGHT,
    FL_ALIGN_BOTTOM_RIGHT,
    FL_ALIGN_BOTTOM,
    FL_ALIGN_BOTTOM_LEFT,
    FL_ALIGN_LEFT
  };


  // Draw the cell box...
  if (readonly()) fl_draw_box(FL_UP_BOX, x(), y(), w(), h(), color());
  else fl_draw_box(FL_DOWN_BOX, x(), y(), w(), h(), color());

  // Draw the cell background...
  if (Fl::focus() == this && !readonly()) {
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

    fl_font(FL_HELVETICA_BOLD, h() - 10);
    fl_draw(s, x(), y(), w(), h(), FL_ALIGN_CENTER);
  }

  fl_font(FL_HELVETICA_BOLD, h() / 5);

  for (int i = 0; i < 8; i ++) {
    if (test_value_[i]) {
      s[0] = test_value_[i] + '0';
      fl_draw(s, x() + 5, y() + 5, w() - 10, h() - 10, align[i]);
    }
  }
}


// Handle events in cell
int
SudokuCell::handle(int event) {
  switch (event) {
    case FL_FOCUS :
      if (!readonly()) {
        Fl::focus(this);
	redraw();
	return 1;
      }
      break;

    case FL_UNFOCUS :
      redraw();
      return 1;
      break;

    case FL_PUSH :
      if (!readonly() && Fl::event_inside(this)) {
        Fl::focus(this);
	redraw();
	return 1;
      }
      break;

    case FL_KEYDOWN :
      int key = Fl::event_key() - '0';
      if (key > 0 && key <= 9) {
        if (Fl::event_state() & FL_SHIFT) {
	  int i;

	  for (i = 0; i < 8; i ++)
	    if (test_value_[i] == key) {
	      test_value_[i] = 0;
	      break;
	    }

          if (i >= 8) {
	    for (i = 0; i < 8; i ++)
	      if (!test_value_[i]) {
		test_value_[i] = key;
		break;
	      }
	  }

	  if (i >= 8) {
	    for (i = 0; i < 7; i ++) test_value_[i] = test_value_[i + 1];
	    test_value_[i] = key;
	  }

	  redraw();
	} else {
	  value(key);
	  do_callback();
	}
	return 1;
      } else if (key == 0 || Fl::event_key() == FL_BackSpace ||
                 Fl::event_key() == FL_Delete) {
        value(0);
	do_callback();
	return 1;
      }
      break;
  }

  return Fl_Widget::handle(event);
}


// Sudoku window class...
class Sudoku : public Fl_Window {
  Fl_Sys_Menu_Bar *menubar_;
  Fl_Group	*grid_;
  char		grid_values_[9][9];
  SudokuCell	*grid_cells_[9][9];
  Fl_Group	*grid_groups_[3][3];
  int		difficulty_;

  static void	check_cb(Fl_Widget *widget, void *);
  static void	close_cb(Fl_Widget *widget, void *);
  static void	diff_cb(Fl_Widget *widget, void *d);
  static void	help_cb(Fl_Widget *, void *);
  static void	new_cb(Fl_Widget *widget, void *);
  static void	reset_cb(Fl_Widget *widget, void *);
  void		set_title();
  static void	solve_cb(Fl_Widget *widget, void *);

  static Fl_Help_Dialog *help_dialog_;
  static Fl_Preferences	prefs_;
  public:

	      	Sudoku();

  void		check_game(bool highlight = true);
  void		load_game();
  void		new_game();
  void		resize(int X, int Y, int W, int H);
  void		save_game();
  void		solve_game();
};


// Sudoku class globals...
Fl_Help_Dialog	*Sudoku::help_dialog_ = (Fl_Help_Dialog *)0;
Fl_Preferences	Sudoku::prefs_(Fl_Preferences::USER, "fltk.org", "sudoku");


// Create a Sudoku game window...
Sudoku::Sudoku()
  : Fl_Window(GROUP_SIZE * 3, GROUP_SIZE * 3 + MENU_OFFSET, "Sudoku")
{
  int i, j;
  Fl_Group *g;
  SudokuCell *cell;
  static Fl_Menu_Item	items[] = {
    { "&Game", 0, 0, 0, FL_SUBMENU },
    { "&New Game", FL_COMMAND | 'n', new_cb, 0, FL_MENU_DIVIDER },
    { "&Check Game", FL_COMMAND | 'c', check_cb, 0, 0 },
    { "&Solve Game", FL_COMMAND | 's', solve_cb, 0, FL_MENU_DIVIDER },
    { "&Quit", FL_COMMAND | 'q', close_cb, 0, 0 },
    { 0 },
    { "&Difficulty", 0, 0, 0, FL_SUBMENU },
    { "&Easy", FL_COMMAND | '1', diff_cb, (void *)"0", FL_MENU_RADIO },
    { "&Medium", FL_COMMAND | '2', diff_cb, (void *)"1", FL_MENU_RADIO },
    { "&Hard", FL_COMMAND | '3', diff_cb, (void *)"2", FL_MENU_RADIO },
    { "&Impossible", FL_COMMAND | '4', diff_cb, (void *)"3", FL_MENU_RADIO },
    { 0 },
    { "&Help", 0, 0, 0, FL_SUBMENU },
    { "&About Sudoku", FL_F + 1, help_cb, 0, 0 },
    { 0 },
    { 0 }
  };


  // Menubar...
  prefs_.get("difficulty", difficulty_, 0);
  if (difficulty_ < 0 || difficulty_ > 3) difficulty_ = 0;

  items[7 + difficulty_].flags |= FL_MENU_VALUE;

  menubar_ = new Fl_Sys_Menu_Bar(0, 0, 3 * GROUP_SIZE, 25);
  menubar_->menu(items);

  // Create the grids...
  grid_ = new Fl_Group(0, MENU_OFFSET, 3 * GROUP_SIZE, 3 * GROUP_SIZE);

  for (i = 0; i < 3; i ++)
    for (j = 0; j < 3; j ++) {
      g = new Fl_Group(j * GROUP_SIZE, i * GROUP_SIZE + MENU_OFFSET,
		       GROUP_SIZE, GROUP_SIZE);
      g->box(FL_BORDER_BOX);
      g->color(FL_DARK3);
      g->end();

      grid_groups_[i][j] = g;
    }

  for (i = 0; i < 9; i ++)
    for (j = 0; j < 9; j ++) {
      cell = new SudokuCell(j * CELL_SIZE + CELL_OFFSET +
                                (j / 3) * (GROUP_SIZE - 3 * CELL_SIZE),
	                    i * CELL_SIZE + CELL_OFFSET + MENU_OFFSET +
			        (i / 3) * (GROUP_SIZE - 3 * CELL_SIZE),
			    CELL_SIZE, CELL_SIZE);
      cell->callback(reset_cb);
      grid_cells_[i][j] = cell;
    }

  callback(close_cb);
  resizable(grid_);
  size_range(3 * GROUP_SIZE, 3 * GROUP_SIZE + MENU_OFFSET, 0, 0, 1, 1, 1);

  // Restore the previous window dimensions...
  int X, Y, W, H;

  if (prefs_.get("x", X, -1)) {
    prefs_.get("y", Y, -1);
    prefs_.get("width", W, 3 * GROUP_SIZE);
    prefs_.get("height", H, 3 * GROUP_SIZE + MENU_OFFSET);

    resize(X, X, W, H);
  }

  // Load the previous game...
  load_game();
  set_title();
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
  int i, j;

  // Check the game for right/wrong answers...
  for (i = 0; i < 9; i ++)
    for (j = 0; j < 9; j ++) {
      SudokuCell *cell = grid_cells_[i][j];
      int val = cell->value();

      if (!val) empty = true;

      if (val && grid_values_[i][j] != val) {
        if (highlight) {
	  cell->color(FL_YELLOW);
	  cell->redraw();
	}

	correct = false;
      }
    }

  if (!empty && correct) {
    // Success!
    solve_game();
    fl_message("Congratulations, you solved the game!");
  } else {
    int k, m;

    for (i = 0; i < 9; i += 3)
      for (j = 0; j < 9; j += 3) {
        correct = true;

        for (k = 0; correct && k < 3; k ++)
	  for (m = 0; m < 3; m ++)
	    if (grid_cells_[i + k][j + m]->value() !=
	            grid_values_[i + k][j + m]) {
	      correct = false;
	      break;
	    }

        if (correct) {
	  for (k = 0; k < 3; k ++)
	    for (m = 0; m < 3; m ++) {
	      SudokuCell *cell = grid_cells_[i + k][j + m];

              cell->readonly(1);
	      cell->color(fl_color_average(FL_GRAY, FL_GREEN, 0.5f));
	    }
	}
        
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

  s->difficulty_ = atoi((char *)d);
  s->new_game();
  s->set_title();

  prefs_.set("difficulty", s->difficulty_);
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
	"<BODY>\n"

	"<H2>About the Game</H2>\n"

	"<P>Sudoku (pronounced soo-dough-coo with the emphasis on the\n"
        "first syllable) is a simple number-based puzzle/game played on a\n"
	"9x9 grid that is divided into 3x3 subgrids. The goal is to enter\n"
	"a number from 1 to 9 in each cell so that each number appears\n"
	"only once in each column and row.</P>\n"

	"<P>This version of the puzzle is Copyright 2005 by Michael R Sweet</P>\n"

	"<H2>How to Play the Game</H2>\n"

	"<P>At the start of a new game, Sudoku fills in a random selection\n"
	"of cells for you - the number of cells depends on the difficulty\n"
	"level you use. Click in any of the empty cells or use the arrow\n"
	"keys to highlight individual cells and press a number from 1 to 9\n"
	"to fill in the cell. To clear a cell, press 0, Delete, or\n"
	"Backspace. As you complete each subgrid, correct subgrids are\n"
	"highlighted in green. When you have successfully completed all\n"
	"subgrids, the entire puzzle is highlighted until you start a new\n"
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

  for (int i = 0; i < 9; i ++)
    for (int j = 0; j < 9; j ++) {
      char name[255];
      int val;

      SudokuCell *cell = grid_cells_[i][j];

      sprintf(name, "value%d.%d", i, j);
      if (!prefs_.get(name, val, 0)) {
        i = 9;
	grid_values_[0][0] = 0;
	break;
      }

      grid_values_[i][j] = val;

      sprintf(name, "state%d.%d", i, j);
      prefs_.get(name, val, 0);
      cell->value(val);
 
      sprintf(name, "readonly%d.%d", i, j);
      prefs_.get(name, val, 0);
      cell->readonly(val);

      if (val) cell->color(FL_GRAY);
      else {
        cell->color(FL_LIGHT3);
	solved = false;
      }

      for (int k = 0; k < 8; k ++) {
	sprintf(name, "test%d%d.%d", k, i, j);
	prefs_.get(name, val, 0);
	cell->test_value(val, k);
      }
    }

  // If we didn't load any values or the last game was solved, then
  // create a new game automatically...
  if (solved || !grid_values_[0][0]) new_game();
  else check_game(false);
}


// Create a new game...
void
Sudoku::new_cb(Fl_Widget *widget, void *) {
  ((Sudoku *)(widget->window()))->new_game();
}


// Create a new game...
void
Sudoku::new_game() {
  int i, j, k, m, t, count;


  // Generate a new (valid) Sudoku grid...
  srand(time(NULL));

  memset(grid_values_, 0, sizeof(grid_values_));

  for (i = 0; i < 9; i += 3) {
    for (j = 0; j < 9; j += 3) {
      for (t = 1; t <= 9; t ++) {
	for (count = 0; count < 20; count ++) {
	  k = i + (rand() % 3);
	  m = j + (rand() % 3);
	  if (!grid_values_[k][m]) {
	    int kk;

	    for (kk = 0; kk < k; kk ++)
	      if (grid_values_[kk][m] == t) break;

	    if (kk < k) continue;

	    int mm;

	    for (mm = 0; mm < m; mm ++)
	      if (grid_values_[k][mm] == t) break;

	    if (mm < m) continue;

	    grid_values_[k][m] = t;
	    break;
	  }
	}

	if (count == 20) {
	  // Unable to find a valid puzzle so far, so start over...
	  j = 9;
	  i = -3;
	  memset(grid_values_, 0, sizeof(grid_values_));
	}
      }
    }
  }

  // Start by making all cells editable
  SudokuCell *cell;

  for (i = 0; i < 9; i ++)
    for (j = 0; j < 9; j ++) {
      cell = grid_cells_[i][j];

      cell->value(0);
      cell->readonly(0);
      cell->color(FL_LIGHT3);
    }

  // Show N cells...
  count = 5 * (5 - difficulty_);

  int numbers[9];

  for (i = 0; i < 9; i ++) numbers[i] = i + 1;

  while (count > 0) {
    for (i = 0; i < 20; i ++) {
      k          = rand() % 9;
      m          = rand() % 9;
      t          = numbers[k];
      numbers[k] = numbers[m];
      numbers[m] = t;
    }

    for (i = 0; count > 0 && i < 9; i ++) {
      t = numbers[i];

      for (j = 0; count > 0 && j < 9; j ++) {
        cell = grid_cells_[i][j];

        if (grid_values_[i][j] == t && !cell->readonly()) {
	  cell->value(grid_values_[i][j]);
	  cell->readonly(1);
	  cell->color(FL_GRAY);

	  count --;
	  break;
	}
      }
    }
  }

  // Show additional cells as needed to avoid ambiguous solutions.
  // The basic premise is to find all possible numbers for each hidden
  // cell and show the cell if we have more than two possible solutions.
  int possible;

  count = 5 * (5 - difficulty_);

  while (count > 0) {
    i    = rand() % 9;
    j    = rand() % 9;
    cell = grid_cells_[i][j];

    if (cell->readonly()) continue;

    possible = 9;
    memset(numbers, 0, sizeof(numbers));

    // Check vertical cells
    for (k = 0; k < 9; k ++) {
      cell = grid_cells_[k][j];
      t    = grid_values_[k][j] - 1;

      if (i != k && !numbers[t] && cell->readonly()) {
	possible --;
	numbers[t] = 1;
      }
    }

    // Check horizontal cells
    for (m = 0; m < 9; m ++) {
      cell = grid_cells_[i][m];
      t    = grid_values_[i][m] - 1;

      if (j != m && !numbers[t] && cell->readonly()) {
	possible --;
	numbers[t] = 1;
      }
    }

    // Now, if the count > 2, show this cell...
    if (possible > 2) {
      cell = grid_cells_[i][j];
      cell->value(grid_values_[i][j]);
      cell->readonly(1);
      cell->color(FL_GRAY);
      count --;
    }
  }
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
  Fl_Window::resize(X, Y, W, H);

  // Save the new window geometry...
  prefs_.set("x", X);
  prefs_.set("y", Y);
  prefs_.set("width", W);
  prefs_.set("height", H);
}


// Save the current game state...
void
Sudoku::save_game() {
  // Save the current values and state of each grid...
  for (int i = 0; i < 9; i ++)
    for (int j = 0; j < 9; j ++) {
      char name[255];
      SudokuCell *cell = grid_cells_[i][j];

      sprintf(name, "value%d.%d", i, j);
      prefs_.set(name, grid_values_[i][j]);

      sprintf(name, "state%d.%d", i, j);
      prefs_.set(name, cell->value());

      sprintf(name, "readonly%d.%d", i, j);
      prefs_.set(name, cell->readonly());

      for (int k = 0; k < 8; k ++) {
	sprintf(name, "test%d%d.%d", k, i, j);
	prefs_.set(name, cell->test_value(k));
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
  int i, j;

  for (i = 0; i < 9; i ++)
    for (j = 0; j < 9; j ++) {
      SudokuCell *cell = grid_cells_[i][j];

      cell->value(grid_values_[i][j]);
      cell->readonly(1);
      cell->color(fl_color_average(FL_GRAY, FL_GREEN, 0.5f));
    }
}


// Main entry for game...
int
main(int argc, char *argv[]) {
  Sudoku s;

  s.show(argc, argv);
  return (Fl::run());
}


//
// End of "$Id$".
//
