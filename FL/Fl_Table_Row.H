//
// Fl_Table_Row -- A row oriented table widget for the Fast Light Tool Kit (FLTK).
//
//    A class specializing in a table of rows.
//    Handles row-specific selection behavior.
//
// Copyright 2002 by Greg Ercolano.
// Copyright 2009-2025 by Bill Spitzak and others.
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

#ifndef _FL_TABLE_ROW_H
#define _FL_TABLE_ROW_H

#include <FL/Fl_Table.H>

#include <stdint.h>
#include <vector>

/**
 A table with row selection capabilities.

 This class implements a simple table with the ability to select
 rows.  This widget is similar to an Fl_Browser with columns.  Most
 methods of importance will be found in the Fl_Table widget, such
 as Fl_Table::rows() and Fl_Table::cols().

 To be useful it must be subclassed and at minimum the draw_cell()
 method must be overridden to provide the content of the cells. This widget
 does \em not manage the cell's data content; it is up to the parent
 class's draw_cell() method override to provide this.

 Events on the cells and/or headings generate callbacks when they are
 clicked by the user.  You control when events are generated based on
 the values you supply for Fl_Table::when().
 */
class FL_EXPORT Fl_Table_Row : public Fl_Table {
public:
  enum TableRowSelectMode {
    SELECT_NONE,                // no selection allowed
    SELECT_SINGLE,              // single row selection
    SELECT_MULTI                // multiple row selection (default)
  };
private:

  std::vector<uint8_t> _rowselect; // selection flag for each row

  // handle() state variables.
  //    Put here instead of local statics in handle(), so more
  //    than one instance can exist without crosstalk between.
  //
  int _dragging_select;         // dragging out a selection?
  int _last_row;
  int _last_y;                  // last event's Y position
  int _last_push_x;             // last PUSH event's X position
  int _last_push_y;             // last PUSH event's Y position

  TableRowSelectMode _selectmode;

protected:
  int handle(int event) override;
  int find_cell(TableContext context,           // find cell's x/y/w/h given r/c
                int R, int C, int &X, int &Y, int &W, int &H) {
    return(Fl_Table::find_cell(context, R, C, X, Y, W, H));
  }

public:
  /**
   The constructor for the Fl_Table_Row.
   This creates an empty table with no rows or columns,
   with headers and row/column resize behavior disabled.
   */
  Fl_Table_Row(int X, int Y, int W, int H, const char *l=0) : Fl_Table(X,Y,W,H,l) {
    _dragging_select = 0;
    _last_row        = -1;
    _last_y          = -1;
    _last_push_x     = -1;
    _last_push_y     = -1;
    _selectmode      = SELECT_MULTI;
  }

  /**
   The destructor for the Fl_Table_Row.
   Destroys the table and its associated widgets.
   */
  ~Fl_Table_Row() { }

  void rows(int val) override; // set number of rows
  int rows() {                          // get number of rows
    return(Fl_Table::rows());
  }

  /**
   Sets the table selection mode.

   - \p Fl_Table_Row::SELECT_NONE - No selection allowed
   - \p Fl_Table_Row::SELECT_SINGLE - Only single rows can be selected
   - \p Fl_Table_Row::SELECT_MULTI - Multiple rows can be selected
   */
  void type(TableRowSelectMode val);    // set selection mode

  TableRowSelectMode type() const {     // get selection mode
    return(_selectmode);
  }

  // Checks to see if 'row' is selected. Returns 1 if selected, 0 if not.
  int row_selected(int row);

  // Changes the selection state for 'row', depending on the value of 'flag'.
  int select_row(int row, int flag = 1);

  /**
   This convenience function changes the selection state
   for \em all rows based on 'flag'. 0=deselect, 1=select, 2=toggle existing state.
   */
  void select_all_rows(int flag=1);     // all rows to a known state

  void clear() override {
    rows(0);            // implies clearing selection
    cols(0);
    Fl_Table::clear();  // clear the table
  }
};

#endif /*_FL_TABLE_ROW_H*/
