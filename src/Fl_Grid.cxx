//
// Fl_Grid widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 2021-2022 by Albrecht Schlosser.
// Copyright 2022-2025 by Bill Spitzak and others.
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

/** \file src/Fl_Grid.cxx

  Implements the Fl_Grid container widget.

  \since 1.4.0
*/

#include <FL/Fl_Grid.H>
#include <FL/fl_draw.H>

// private class Col for column management

class Fl_Grid::Col {
  friend class Fl_Grid;
  int minw_;            // minimal size (width)
  int w_;               // calculated size (width)
  short weight_;        // weight used to allocate extra space
  short gap_;           // gap to the right of the column
  Col() {
    minw_   =  0;
    w_      =  0;
    weight_ = 50;
    gap_    = -1;
  }
  ~Col() {};
};

// private class Row for row management

class Fl_Grid::Row {
  friend class Fl_Grid;

  Cell *cells_;         // cells of this row
  int minh_;            // minimal size (height)
  int h_;               // calculated size (height)
  short weight_;        // weight used to allocate extra space
  short gap_;           // gap below the row (-1 = use default)

  Row() {
    cells_  = NULL;
    minh_   =  0;
    h_      =  0;
    weight_ = 50;
    gap_    = -1;
  }

  ~Row() {
    free_cells();
  };

  // Fl_Grid::Row::free_cells() - free all cells of a row

  void free_cells() {
    Cell *cel = cells_;
    while (cel) {
      Cell *next = cel->next();
      delete cel;
      cel = next;
    } // free_cells()
    cells_ = 0;
  }

  // Fl_Grid::Row::remove_cell() - remove all cells of column col from the list of cells

  void remove_cell(int col) { //
    Cell *cel = cells_;
    Cell *prev = 0;
    while (cel) {
      Cell *next = cel->next();
      if (cel->col() == col) {
        if (prev) {
          prev->next(next);
        } else {
          cells_ = next;
        }
        delete cel;
        return;
      } else {
        prev = cel;
        cel = next;
      }
    } // while (cel)
  } // Row::remove_cell(col)

}; // class Row

/**
  Create a new Fl_Grid widget.

  \todo More documentation of Fl_Grid constructor?
*/
Fl_Grid::Fl_Grid(int X, int Y, int W, int H, const char *L)
  : Fl_Group(X, Y, W, H, L) {

  init();
  box(FL_FLAT_BOX);
}

// private: init vars

void Fl_Grid::init() {
  rows_ = 0;
  cols_ = 0;
  margin_left_ = 0;
  margin_top_ = 0;
  margin_right_ = 0;
  margin_bottom_ = 0;
  gap_row_ = 0;
  gap_col_ = 0;
  Cols_ = 0;
  Rows_ = 0;
  old_size = Fl_Rect(0, 0, 0, 0);
  need_layout_ = false;               // no need to calculate layout
  grid_color = (Fl_Color)0xbbeebb00;  // light green
  draw_grid_ = false;                 // don't draw grid helper lines
  if (fl_getenv("FLTK_GRID_DEBUG"))
    draw_grid_ = true;
}

Fl_Grid::~Fl_Grid() {
  delete[] Cols_;
  delete[] Rows_;
}

/**
  Set the basic layout parameters of the Fl_Grid widget.

  You need to specify at least \p rows and \p cols to define a layout
  before you can add widgets to the grid.

  Parameters \p margin and \p gap are optional.

  You can call layout(int rows, int cols, int margin, int gap) again
  to change the layout but this is inefficient since all cells are
  reallocated if the layout changed.

  Calling this with the same values of \p rows and \p cols is fast and
  can be used to change \p margin and \p gap w/o reallocating the cells.

  \p margin sets all margins (left, top, right, bottom) to the same value.
  Negative values (e.g. -1) don't change the established margins.
  The default value set by the constructor is 0.

  \p gap sets row and column gaps to the same value. Negative values (e.g. -1)
  do not affect the established gaps. The default value set by the constructor
  is 0.

  After you added all widgets you must call layout() once without arguments
  to calculate the actual layout and to position and resize all widgets.

  \todo Document when and why to call layout() w/o args. See Fl_Flex::layout()

  \param[in]  rows    number of rows
  \param[in]  cols    number of columns
  \param[in]  margin  margin size inside the Fl_Grid's border
  \param[in]  gap     gap size between cells

  \see Fl_Grid::layout()
*/
void Fl_Grid::layout(int rows, int cols, int margin, int gap) {

  if (margin >= 0)
    margin_left_ = margin_top_ = margin_right_ = margin_bottom_ = margin;

  if (gap >= 0)
    gap_row_ = gap_col_ = gap;

  if (cols == cols_ && rows == rows_) // same size, nothing to do
    return;

  // release allocated memory if either rows <= 0 or cols <= 0

  if (rows <= 0 || cols <= 0) {
    clear_layout();
    return;
  }

  // allocate new cells, rows, and columns

  // Cell *new_cells = new Cell[rows * cols];

  // reallocate and copy old columns

  if (cols != cols_) {
    Col *new_cols = new Col[cols];
    for (int c = 0; c < cols; c++) {
      if (c < cols_)
        new_cols[c] = Cols_[c];
      else
        break;
    }
    delete[] Cols_;
    Cols_ = new_cols;
  }

  // reallocate and copy old rows

  if (rows != rows_) {
    Row *new_rows = new Row[rows];
    Row *row = Rows_;
    for (int r = 0; r < rows; r++, row++) {
      if (r < rows_) {
        new_rows[r] = *row;
        row->cells_ = 0;
      } else {
        break;
      }
    }
    delete[] Rows_;
    Rows_ = new_rows;
  }

  // store new layout and cells

  cols_ = cols;
  rows_ = rows;
  need_layout(1);

} // layout(int, int, int, int)


/**
  Draws the grid helper lines for design and debugging purposes.

  This method is protected so it can be modified in subclasses.
*/
void Fl_Grid::draw_grid() {

  int x0 = x() + Fl::box_dx(box()) + margin_left_;
  int y0 = y() + Fl::box_dy(box()) + margin_top_;
  int x1 = x() + w() - Fl::box_dx(box()) - margin_right_;
  int y1 = y() + h() - Fl::box_dy(box()) - margin_bottom_;

  fl_line_style(FL_SOLID, 1);
  fl_color(grid_color);

  // draw total layout frame

  fl_rect(x0, y0, x1 - x0, y1 - y0);

  // draw horizontal lines (gap = 0) or rectangles (gap > 0)

  for (int r = 0; r < rows_ - 1; r++) {

    int gap = Rows_[r].gap_ >= 0 ? Rows_[r].gap_ : gap_row_;
    y0 += Rows_[r].h_;
    if (gap == 0) {
      fl_xyline(x0, y0, x1);
    } else {
      fl_rectf(x0, y0, x1 - x0, gap);
    }
    y0 += gap;
  }

  // draw vertical lines (gap = 0) or rectangles (gap > 0)

  x0 = x() + Fl::box_dx(box()) + margin_left_;
  y0 = y() + Fl::box_dy(box()) + margin_top_;

  for (int c = 0; c < cols_ - 1; c++) {

    int gap = Cols_[c].gap_ >= 0 ? Cols_[c].gap_ : gap_col_;
    x0 += Cols_[c].w_;
    if (gap == 0) {
      fl_yxline(x0, y0, y1);
    } else {
      fl_rectf(x0, y0, gap, y1 - y0);
    }
    x0 += gap;
  }

  fl_line_style(FL_SOLID, 0);
  fl_color(FL_BLACK);

} // Fl_Grid::draw_grid()


/**
  Draws the Fl_Grid widget and all children.

  If the layout has been changed layout() is called before the widget
  is drawn so all children are arranged as designed.

  \see layout()
  \see need_layout()
*/
void Fl_Grid::draw() {

  if (need_layout())
    layout();

  if (damage() & ~FL_DAMAGE_CHILD) { // draw the entire widget
    draw_box();
    if (draw_grid_)
      draw_grid();
    draw_label();
  }
  draw_children();

} // Fl_Grid::draw()

/**
  Calculate the grid layout and resize and position all widgets.

  This is called automatically when the Fl_Grid is resized. You need to
  call it once after you added widgets or moved widgets between cells.

  Calling it once after all modifications are completed is enough.

  \todo Document when and why to call layout() w/o args. See Fl_Flex::layout()

  \see Fl_Grid::layout(int rows, int cols, int margin, int gap)
*/
void Fl_Grid::layout() {

  if (rows_ == 0 || cols_ == 0) // empty grid
    return;

  Row *row;
  Col *col;
  Cell *cel;

  // calculate the total available space w/o borders and margins

  int tw = w() - Fl::box_dw(box()) - margin_left_ - margin_right_;
  int th = h() - Fl::box_dh(box()) - margin_top_ - margin_bottom_;

  // initialize column widths and row heights

  col = Cols_;
  for (int c = 0; c < cols_; c++, col++) {
    col->w_ = col->minw_;
  }

  row = Rows_;
  for (int r = 0; r < rows_; r++, row++) {
    row->h_ = row->minh_;
  }

  // calculate minimal column widths and row heights (in one loop)

  row = Rows_;
  for (int r = 0; r < rows_; r++, row++) {
    col = Cols_;
    for (int c = 0; c < cols_; c++, col++) {
      cel = cell(r, c);
      if (cel) {
        Fl_Widget *wi = cel->widget_;
        if (wi && wi->visible()) {
          if (cel->colspan_ == 1 && cel->w_ > col->w_) col->w_ = cel->w_;
          if (cel->rowspan_ == 1 && cel->h_ > row->h_) row->h_ = cel->h_;
        } // widget
      } // colspan && rowspan
    } // cols
  } // rows

  // calculate total space occupied by rows and columns including gaps

  int tcwi = 0;       // total column width incl. gaps
  int tcwe = 0;       // total column weight
  int hcwe = 0;       // highest column weight
  int icwe = 0;       // index of column with highest weight

  int trhe = 0;       // total row height incl. gaps
  int trwe = 0;       // total row weight
  int hrwe = 0;       // highest row weight
  int irwe = 0;       // index of row with highest weight

  col = Cols_;
  for (int c = 0; c < cols_; c++, col++) {
    tcwi += col->w_;
    tcwe += col->weight_;
    if (c < cols_ - 1)
      tcwi += ((col->gap_ >= 0) ? col->gap_ : gap_col_);
    if (col->weight_ > hcwe) {
      hcwe = col->weight_;
      icwe = c;
    }
  }

  row = Rows_;
  for (int r = 0; r < rows_; r++, row++) {
    trhe += row->h_;
    trwe += row->weight_;
    if (r < rows_ - 1)
      trhe += ((row->gap_ >= 0) ? row->gap_ : gap_row_);
    if (row->weight_ > hrwe) {
      hrwe = row->weight_;
      irwe = r;
    }
  }

  // Add extra space to columns and rows to fill the entire grid, using relative weights.
  // Rounding differences are added to or subtracted from the col/row with the highest weight.

  int space = tw - tcwi;  // additional space for columns
  int add_space = 0;      // used for calculation
  int remaining = 0;      // remaining space

  if (space > 0 && tcwe > 0) {
    remaining = space;
    col = Cols_;
    for (int c = 0; c < cols_; c++, col++) {
      if (col->weight_ > 0) {
        add_space = int(float(space * col->weight_) / tcwe + 0.5);
        col->w_ += add_space;
        remaining -= add_space;
      }
    }
    if (remaining != 0)
      Cols_[icwe].w_ += remaining;
  }

  space = th - trhe;      // additional space for rows

  if (space > 0 && trwe > 0) {
    remaining = space;
    row = Rows_;
    for (int r = 0; r < rows_; r++, row++) {
      if (row->weight_ > 0) {
        add_space = int(float(space * row->weight_) / trwe + 0.5);
        row->h_ += add_space;
        remaining -= add_space;
      }
    }
    if (remaining != 0)
      Rows_[irwe].h_ += remaining;
  }

  // calculate and assign widget positions and sizes

  int x0, y0; // starting x/y positions

  y0 = y() + Fl::box_dy(box()) + margin_top_;

  row = Rows_;
  for (int r = 0; r < rows_; r++, row++) {
    x0 = x() + Fl::box_dx(box()) + margin_left_;
    col = Cols_;
    for (int c = 0; c < cols_; c++, col++) {
      int wx = x0;  // widget's x
      int wy = y0;  // widget's y
      cel = cell(r, c);
      if (cel) {
        Fl_Widget *wi = cel->widget_;
        if (wi && wi->visible()) {

          // calculate the cell's position and size, take cell spanning into account

          int ww = col->w_;
          int wh = row->h_;

          for (int i = 0; i < cel->colspan_ - 1; i++) {
            ww += (Cols_[c + i].gap_ >= 0) ? Cols_[c + i].gap_ : gap_col_;
            ww += Cols_[c + i + 1].w_;
          }

          for (int i = 0; i < cel->rowspan_ - 1; i++) {
            wh += (Rows_[r + i].gap_ >= 0) ? Rows_[r + i].gap_ : gap_row_;
            wh += Rows_[r + i + 1].h_;
          }

          // horizontal alignment: left + right => stretch

          Fl_Grid_Align ali = cel->align_;
          Fl_Grid_Align mask;

          mask = FL_GRID_LEFT | FL_GRID_RIGHT | FL_GRID_HORIZONTAL;
          if ((ali & mask) == 0) {
            wx += (ww - cel->w_) / 2;
            ww = cel->w_;
          } else if ((ali & mask) == FL_GRID_LEFT) {
            ww = cel->w_;
          } else if ((ali & mask) == FL_GRID_RIGHT) {
            wx += ww - cel->w_;
            ww = cel->w_;
          }

          // vertical alignment: top + bottom => stretch

          mask = FL_GRID_TOP | FL_GRID_BOTTOM | FL_GRID_VERTICAL;
          if ((ali & mask) == 0) {
            wy += (wh - cel->h_) / 2;
            wh = cel->h_;
          } else if ((ali & mask) == FL_GRID_TOP) {
            wh = cel->h_;
          } else if ((ali & mask) == FL_GRID_BOTTOM) {
            wy += wh - cel->h_;
            wh = cel->h_;
          }

          wi->resize(wx, wy, ww, wh);

        } // widget is visible

      } // cell

      x0 += (col->w_ + ((col->gap_ >= 0) ? col->gap_ : gap_col_));

    } // cols

    y0 += ( row->h_ + ((row->gap_ >= 0) ? row->gap_ : gap_row_) );

  } // rows

  need_layout(0);
  redraw();

} // layout()


/**
  Fl_Group calls this method when a child widget is about to be removed.

  Make sure that the widget is also removed from our internal list of children.
*/
void Fl_Grid::on_remove(int index) {

  Fl_Widget *wi = child(index);
  Cell *c = cell(wi); // find the cell of this child
  if (c) {
    remove_cell(c->row_, c->col_);
  }
}

// private: add a new cell to the grid

Fl_Grid::Cell *Fl_Grid::add_cell(int row, int col) {
  Cell *c = new Cell(row, col);
  Row *r = &Rows_[row];
  Cell* cel = r->cells_;  // "current" cell
  Cell* prev = 0;         // "previous" cell
  while (cel) {           // existing cells
    if (cel->col_ > col) { // found spot ...
      break;
    }
    prev = cel;
    cel = cel->next_;
  }

  // insertion point: prev => last cell or NULL, cel == next cell or NULL

  if (prev)
    prev->next_ = c;
  else
    r->cells_ = c;
  c->next_ = cel;

  need_layout(1);
  return c;
}

// private: remove a cell from the grid

void Fl_Grid::remove_cell(int row, int col) {
  Row *r = &Rows_[row];
  r->remove_cell(col);
  need_layout(1);
}

/**
  Recalculate the layout and position and resize all widgets.

  This method overrides Fl_Group::resize() and calculates all positions and
  sizes of its children according to its own rules.

  \param[in]  X,Y   new widget position
  \param[in]  W,H   new widget size
*/
void Fl_Grid::resize(int X, int Y, int W, int H) {
  old_size = Fl_Rect(x(), y(), w(), h());
  Fl_Widget::resize(X, Y, W, H);
  layout();
}

/**
  Reset the layout w/o removing widgets.

  Removes all cells and sets rows and cols to zero. Existing widgets are
  kept as children of the Fl_Group (base class) but are hidden.

  This method should be rarely used. You may want to call Fl_Grid::clear()
  to remove all widgets and reset the layout to zero rows and columns.

  You must call layout(int rows, int cols, ...) to set a new layout,
  allocate new cells, and assign widgets to new cells.

  \todo Fl_Grid::clear() needs to be implemented as documented above!
*/
void Fl_Grid::clear_layout() {

  delete[] Cols_;
  delete[] Rows_;
  init();
  for (int i = 0; i < children(); i++) {
    child(i)->hide();
  }
  need_layout(1);
  return;
}

/**
  Set all margins (left, top, right, bottom).

  All margins are measured in pixels inside the box borders. You need
  to specify at least one argument, all other arguments are optional.
  If you don't specify an argument or use a negative value (e.g. -1)
  then that particular margin is not affected.

  \param[in]  left    left margin
  \param[in]  top     top margin
  \param[in]  right   right margin
  \param[in]  bottom  bottom margin
*/
void Fl_Grid::margin(int left, int top, int right, int bottom) {
  if (left >= 0)
    margin_left_ = left;
  if (top >= 0)
    margin_top_ = top;
  if (right >= 0)
    margin_right_ = right;
  if (bottom >= 0)
    margin_bottom_ = bottom;
  need_layout(1);
}

/**
 Returns all outside margin sizes of the grid.

 All margin sizes are returned in the given arguments. If any argument
 is \p NULL the respective value is not returned.

 \param[out]  left    returns left margin if not \p NULL
 \param[out]  top     returns top margin if not \p NULL
 \param[out]  right   returns right margin if not \p NULL
 \param[out]  bottom  returns bottom margin if not \p NULL

 \return     whether all margins are equal
 \retval  1  all margins have the same size
 \retval  0  at least one margin has a different size
 */
int Fl_Grid::margin(int *left, int *top, int *right, int *bottom) const {
  if (left) *left = margin_left_;
  if (top) *top = margin_top_;
  if (right) *right = margin_right_;
  if (bottom) *bottom = margin_bottom_;
  if (margin_left_ == margin_top_ && margin_top_ == margin_right_ && margin_right_ == margin_bottom_)
    return 1;
  return 0;
}

/**
  Set default gaps for rows and columns.

  All gaps are positioned below the rows and right of their columns.

  The bottom row and the right-most column don't have a gap, i.e. the gap
  sizes of these columns and rows are ignored. You can use a right or
  bottom margin instead.

  You have to specify at least one argument, \p col_gap is optional.
  If you don't specify an argument or use a negative value (e.g. -1)
  then that margin is not affected.

  You can also initialize the default gaps with layout(int, int, int, int).

  \param[in]  row_gap  default gap for all rows
  \param[in]  col_gap  default gap for all columns

  \see Fl_Grid::layout(int rows, int cols, int margin, int gap)
*/
void Fl_Grid::gap(int row_gap, int col_gap) {
  if (row_gap >= 0)
    gap_row_ = row_gap;
  if (col_gap >= 0)
    gap_col_ = col_gap;
  need_layout(1);
}

/**
 Get the default gaps for rows and columns.

 \param[out]  row_gap  pointer to int to receive column gap, may be NULL
 \param[out]  col_gap  pointer to int to receive column gap, may be NULL
 */
void Fl_Grid::gap(int *row_gap, int *col_gap) const {
  if (row_gap)
    *row_gap = gap_row_;
  if (col_gap)
    *col_gap = gap_col_;
}

/**
  Get the grid cell of row \p row and column \p col.

  Widgets and other attributes are organized in cells (Fl_Grid::Cell).

  This cell is an opaque structure (class) with some public methods.
  \b Don't assume anything about grid cell sizes and ordering in memory.
  These are implementation details that can be changed without notice.

  The validity of an Fl_Grid::Cell pointer is limited. It will definitely be
  invalidated when the overall grid layout is changed, for instance by calling
  layout(int, int).

  Adding new cells beyond the current layout limits will also invalidate
  cell pointers but this is not (yet) implemented. Attempts to assign widgets
  to out-of-bounds cells are currently ignored.

  The only well-defined usage of cell pointers is to set one or more properties
  like widget alignment of a cell after retrieving the cell pointer. Don't
  store cell pointers in your program for later reference.

  \param[in]  row   row index
  \param[in]  col   column index

  \returns          pointer to cell
  \retval     NULL  if \p row or \p col is out of bounds or no widget was assigned
*/
Fl_Grid::Cell* Fl_Grid::cell(int row, int col) const {
  if (row < 0 || row >= rows_ || col < 0 || col >= cols_)
    return 0;
  Row *r = &Rows_[row];
  Cell *cel = r->cells_;
  while (cel) {
    if (cel->col_ > col)
      return 0;
    if (cel->col_ == col)
      return cel;
    cel = cel->next_;
  }
  return 0;
}

/**
  Get the grid cell of widget \p widget.

  The pointer to the cell can be used for further assignment of properties
  like alignment etc.

  Hint: If you know the row and column index of the cell you should use
  Fl_Grid::cell(int row, int col) instead because it is \b much faster.

  Please see Fl_Grid::cell(int row, int col) for details and the
    validity of cell pointers.

  \param[in]  widget  widget whose cell is requested
  \retval     NULL    if \p widget is not assigned to a cell
*/
Fl_Grid::Cell* Fl_Grid::cell(Fl_Widget *widget) const {
  Row *row = Rows_;
  for (int r = 0; r < rows_; r++, row++) {
    Cell *cel = row->cells_;
    while (cel) {
      if (cel->widget_ == widget)
        return cel;
      cel = cel->next_;
    }
  }
  return 0;
}

/**
  Assign a widget to a grid cell and set its alignment.

  This short form sets row and column spanning attributes to (1, 1).

  For more information see
  Fl_Grid::widget(Fl_Widget *wi, int row, int col, int rowspan, int colspan, Fl_Grid_Align align)

  \param[in]  wi      widget to be assigned to the cell
  \param[in]  row     row
  \param[in]  col     column
  \param[in]  align   widget alignment inside the cell

  \return     assigned cell
  \retval     NULL      if \p row or \p col is out of bounds

  \see Fl_Grid::widget(Fl_Widget *wi, int row, int col, int rowspan, int colspan, Fl_Grid_Align align)
*/
Fl_Grid::Cell *Fl_Grid::widget(Fl_Widget *wi, int row, int col, Fl_Grid_Align align) {
  return widget(wi, row, col, 1, 1, align);
}

/**
  Assign a widget to a grid cell and set cell spanning and alignment.

  Default alignment is \c FL_GRID_FILL which stretches the widget in
  horizontal and vertical directions to fill the whole cell(s) given
  by \p colspan and \p rowspan.

  You can use this method to move a widget from one cell to another; it
  is automatically removed from its old cell. If the new cell is already
  assigned to another widget that widget is deassigned but kept as a
  child of the group.

  Before you can assign a widget to a cell it must have been created as
  a child of the Fl_Grid widget (i.e. its Fl_Group).

  \param[in]  wi        widget to be assigned to the cell
  \param[in]  row       row
  \param[in]  col       column
  \param[in]  rowspan   vertical span in cells, default 1
  \param[in]  colspan   horizontal span in cells, default 1
  \param[in]  align     widget alignment inside the cell

  \return     assigned cell
  \retval     NULL      if \p row or \p col is out of bounds or \p wi is not a child
*/
Fl_Grid::Cell *Fl_Grid::widget(Fl_Widget *wi, int row, int col, int rowspan, int colspan, Fl_Grid_Align align) {

  int child = Fl_Group::find(wi); // is this widget one of our children?
  if (child >= children()) {
    // fprintf(stderr, "Fl_Grid::widget(): can't assign widget %p to cell (%d, %d): not a child!\n", wi, row, col);
    return 0;
  }
  if (row < 0 || row > rows_)
    return 0;
  if (col < 0 || col > cols_)
    return 0;

  Cell *c = cell(row, col);
  if (!c) {
    c = add_cell(row, col);
  }

  // check if the widget is already assigned to another cell and deassign it

  if (c->widget_ != wi) {
    Cell *oc = cell(wi);  // search cells for the same widget
    if (oc) {             // if found: deassign and remove cell
      remove_cell(oc->row_, oc->col_);
    }
  }

  // assign the widget to this cell

  c->widget_ = wi;
  c->align_ = align;

  c->w_ = wi->w();
  c->h_ = wi->h();

  if (rowspan > 0)
    c->rowspan_ = rowspan;
  if (colspan > 0)
    c->colspan_ = colspan;

  need_layout(1);
  return c;
}

/**
  Set the minimal width of a column.

  Column widths are calculated by using the maximum of all widget widths
  in that column and the given column width. After calculating the width
  additional space is added when resizing according to the \c weight of
  the column.

  You can set one or more column widths in one call by using
    Fl_Grid::col_width(const int *value, size_t size).

  \param[in]  col     column number (counting from 0)
  \param[in]  value   minimal column width, must be \>= 0

  \see Fl_Grid::col_width(const int *value, size_t size)
*/

void Fl_Grid::col_width(int col, int value) {
  if (col >= 0 && col < cols_) {
    if (Cols_[col].minw_ != value) {
      Cols_[col].minw_ = value;
      need_layout(1);
    }
  }
}

/**
  Set minimal widths of more than one column.

  The values are taken from the array \p value and assigned sequentially
  to columns, starting from column 0. If the array \p size is too large
  extraneous values are ignored.

  Negative values in the \p array are not assigned to their columns,
  i.e. the existing value for the corresponding column is not changed.

  Example:
  \code
    int widths[] = { 0, 0, 50, -1, -1, 50, 0 };
    grid->col_width(widths, sizeof(width)/sizeof(width[0]));
  \endcode

  \param[in]  value an array of column widths
  \param[in]  size  the size of the array (number of values)
*/
void Fl_Grid::col_width(const int *value, size_t size) {
  Col *c = Cols_;
  for (int i = 0; i < cols_; i++, value++, c++) {
    if (i >= (int)size) break;
    if (*value >= 0)
      c->minw_ = *value;
  }
  need_layout(1);
}

int Fl_Grid::col_width(int col) const {
  if (col >= 0 && col < cols_) return Cols_[col].minw_;
  return 0;
}

/**
  Set the weight of a column.

  Column and row weights are used to distribute additional space when the
  grid is resized beyond its defined (minimal) size. All weight values are
  relative and can be chosen freely. Suggested weights are in the range
  {0 .. 100}, 0 (zero) disables resizing of the column.

  How does it work?

  Whenever additional space (say: \c SPACE in pixels) is to be distributed
  to a set of columns the weights of all columns are added to a value \c SUM,
  then every single column width is increased by the value (in pseudo code):
  \code
    col.width += SPACE * col.weight / SUM
  \endcode
  Resulting pixel values are rounded to the next integer and rounding
  differences are added to or subtracted from the column with the highest
  weight. If more columns have the same weight one of them is chosen.

  \note If none of the columns considered for resizing have weights \> 0
    then Fl_Grid assigns the remaining space to an arbitrary column or to
    all considered columns evenly. This is implementation defined and can
    be changed without notice. You can avoid this situation by designing
    your grid with sensible sizes and weights.

  \param[in]  col     column number (counting from 0)
  \param[in]  value   weight, must be \>= 0
*/
void Fl_Grid::col_weight(int col, int value) {
  if (col >= 0 && col < cols_)
    Cols_[col].weight_ = value;
  need_layout(1);
}


/**
  Set the weight of more than one column.

  The values are taken from the array \p value and assigned sequentially
  to columns, starting from column 0. If the array \p size is too large
  extraneous values are ignored.

  Negative values in the \p array are not assigned to their columns,
  i.e. the existing value for the corresponding column is not changed.

  Example:
  \code
    int val[] = { 0, 0, 50, -1, -1, 50, 0 };
    grid->col_weight(val, sizeof(val)/sizeof(val[0]));
  \endcode

  \param[in]  value an array of column weights
  \param[in]  size  the size of the array (number of values)
*/
void Fl_Grid::col_weight(const int *value, size_t size) {
  Col *c = Cols_;
  for (int i = 0; i < cols_; i++, value++, c++) {
    if (i >= (int)size) break;
    if (*value >= 0)
      c->weight_ = *value;
  }
  need_layout(1);
}

int Fl_Grid::col_weight(int col) const {
  if (col >= 0 && col < cols_) return Cols_[col].weight_;
  return 0;
}

/**
  Set the gap of column \c col.

  Note that the gap is right of each column except the last one
  which is ignored. Use margin() for the right most column.

  \param[in]    col     column
  \param[in]    value   gap size after the column
*/
void Fl_Grid::col_gap(int col, int value) {
  if (col >= 0 && col < cols_)
    Cols_[col].gap_ = value;
  need_layout(1);
}

/**
  Set more than one column gaps at once.

  \see Fl_Grid::col_weight(const int *value, size_t size) for
    handling of the value array and \p size.
*/
void Fl_Grid::col_gap(const int *value, size_t size) {
  Col *c = Cols_;
  for (int i = 0; i < cols_; i++, value++, c++) {
    if (i >= (int)size) break;
    if (*value >= 0)
      c->gap_ = *value;
  }
  need_layout(1);
}

int Fl_Grid::col_gap(int col) const {
  if (col >= 0 && col < cols_) return Cols_[col].gap_;
  return 0;
}

/**
  Set the minimal row height of row \c row.

  \param[in]    row     row
  \param[in]    value   minimal height of the row
*/
void Fl_Grid::row_height(int row, int value) {
  if (row >= 0 && row < rows_)
    Rows_[row].minh_ = value;
  need_layout(1);
}

/**
  Set the minimal row height of more than one row.

  \param[in]    value   array of height values
  \param[in]    size    size of array \p value

  \see Fl_Grid::col_weight(const int *value, size_t size) for
    handling of the value array and \p size.
*/
void Fl_Grid::row_height(const int *value, size_t size) {
  Row *r = Rows_;
  for (int i = 0; i < rows_; i++, value++, r++) {
    if (i >= (int)size) break;
    if (*value >= 0)
      r->minh_ = *value;
  }
  need_layout(1);
}

int Fl_Grid::row_height(int row) const {
  if (row >= 0 && row < rows_) return Rows_[row].minh_;
  return 0;
}

/**
  Set the row weight of row \c row.

  \param[in]    row     row
  \param[in]    value   weight of the row
*/
void Fl_Grid::row_weight(int row, int value) {
  if (row >= 0 && row < rows_)
    Rows_[row].weight_ = value;
  need_layout(1);
}

/**
  Set the weight of more than one row.

  \param[in]    value   array of height values
  \param[in]    size    size of array \p value

  \see Fl_Grid::col_weight(const int *value, size_t size) for
    handling of the \p value array and \p size.
*/
void Fl_Grid::row_weight(const int *value, size_t size) {
  Row *r = Rows_;
  for (int i = 0; i < rows_; i++, value++, r++) {
    if (i >= (int)size) break;
    if (*value >= 0)
      r->weight_ = *value;
  }
  need_layout(1);
}

int Fl_Grid::row_weight(int row) const {
  if (row >= 0 && row < rows_) return Rows_[row].weight_;
  return 0;
}

/**
  Set the gap of row \c row.

  Note that the gap is below each row except the last one
  which is ignored. Use margin() for the bottom row.

  \param[in]    row     row
  \param[in]    value   gap size below the row
*/
void Fl_Grid::row_gap(int row, int value) {
  if (row >= 0 && row < rows_)
    Rows_[row].gap_ = value;
  need_layout(1);
}


/**
  Set more than one row gaps at once.

  \see Fl_Grid::col_weight(const int *value, size_t size) for
    handling of the value array and \p size.
*/
void Fl_Grid::row_gap(const int *value, size_t size) {
  Row *r = Rows_;
  for (int i = 0; i < rows_; i++, value++, r++) {
    if (i >= (int)size) break;
    if (*value >= 0)
      r->gap_ = *value;
  }
  need_layout(1);
}

int Fl_Grid::row_gap(int row) const {
  if (row >= 0 && row < rows_) return Rows_[row].gap_;
  return 0;
}

int Fl_Grid::computed_col_width(int col) const {
  return Cols_[col].w_;
}

int Fl_Grid::computed_row_height(int row) const {
  return Rows_[row].h_;
}

/**
  Output layout information of this Fl_Grid to stderr.

  Parameter \p level will be used to define the amount of output.
  - 0 = nothing
  - 127 = everything
  - other values not yet defined

  \note It is not yet defined which kind of values \p level will have,
    either a numerical value (127 = maximum, 0 = nothing) or a bit mask
    that determines what to output.

  \todo Add more information about cells and children.
  \todo Control output by using \p level.

  \param[in]  level  not yet used (0-127, default = 127)
*/
void Fl_Grid::debug(int level) {
  if (level <= 0)
    return;
  fprintf(stderr, "Fl_Grid::layout(%d, %d) at (%d, %d, %d, %d)\n",
          rows_, cols_, x(), y(), w(), h());
  fprintf(stderr, "    margins:   (%2d, %2d, %2d, %2d)\n",
          margin_left_, margin_top_, margin_right_, margin_bottom_);
  fprintf(stderr, "       gaps:   (%2d, %2d)\n",
          gap_row_, gap_col_);
  Row *row = Rows_;
  for (int r = 0; r < rows_; r++, row++) {
    fprintf(stderr, "Row %2d: minh = %d, weight = %d, gap = %d, h = %d\n",
            r, row->minh_, row->weight_, row->gap_, row->h_);
    Cell *cel = row->cells_;
    while (cel) {
      fprintf(stderr, "        Cell(%2d, %2d)\n", cel->row_, cel->col_);
      cel = cel->next_;
    }
  }
  fflush(stderr); // necessary for Windows
}
