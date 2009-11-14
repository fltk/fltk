//
// Fl_Table -- A table widget
//
// Copyright 2002 by Greg Ercolano.
// Copyright (c) 2004 O'ksi'D
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
// Please report all bugs and problems to "erco at seriss dot com".
//
// TODO:
//    o Auto scroll during dragged selection
//    o Keyboard navigation (up/down/left/right arrow)
//

#ifndef _FL_TABLE_H
#define _FL_TABLE_H

#include <sys/types.h>
#include <string.h>		// memcpy
#ifdef _WIN32
#include <malloc.h>		// WINDOWS: malloc/realloc
#else /*_WIN32*/
#include <stdlib.h>		// UNIX: malloc/realloc
#endif /*_WIN32*/

#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Scrollbar.H>

class Fl_Table : public Fl_Group {
public:
  enum TableContext {
    CONTEXT_NONE       = 0,
    CONTEXT_STARTPAGE  = 0x01,	// before a page is redrawn
    CONTEXT_ENDPAGE    = 0x02,	// after a page is redrawn
    CONTEXT_ROW_HEADER = 0x04,	// in the row header
    CONTEXT_COL_HEADER = 0x08,	// in the col header
    CONTEXT_CELL       = 0x10,	// in one of the cells
    CONTEXT_TABLE      = 0x20,	// in the table
    CONTEXT_RC_RESIZE  = 0x40 	// column or row being resized
  };

private:
  int _rows, _cols;	// total rows/cols
  int _row_header_w;	// width of row header
  int _col_header_h;	// height of column header
  int _row_position;	// last row_position set (not necessarily == toprow!)
  int _col_position;	// last col_position set (not necessarily == leftcol!)

  char _row_header;	// row header enabled?
  char _col_header;	// col header enabled?
  char _row_resize;	// row resizing enabled?
  char _col_resize;	// col resizing enabled?
  int _row_resize_min;	// row minimum resizing height (default=1)
  int _col_resize_min;	// col minimum resizing width (default=1)

  // OPTIMIZATION: partial row/column redraw variables
  int _redraw_toprow;
  int _redraw_botrow;
  int _redraw_leftcol;
  int _redraw_rightcol;
  Fl_Color _row_header_color;
  Fl_Color _col_header_color;

  int _auto_drag;
  int _selecting;

  // An STL-ish vector without templates
  class IntVector {
    int *arr;
    unsigned int _size;
    void init() {
      arr = NULL;
      _size = 0;
    }
    void copy(int *newarr, unsigned int newsize) {
      size(newsize);
      memcpy(arr, newarr, newsize * sizeof(int));
    }
  public:
    IntVector() { init(); }					// CTOR
    ~IntVector() { if ( arr ) free(arr); arr = NULL; }		// DTOR
    IntVector(IntVector&o) { init(); copy(o.arr, o._size); }	// COPY CTOR
    IntVector& operator=(IntVector&o) {				// ASSIGN
      init();
      copy(o.arr, o._size);
      return(*this);
    }
    int operator[](int x) const { return(arr[x]); }
    int& operator[](int x) { return(arr[x]); }
    unsigned int size() { return(_size); }
    void size(unsigned int count) {
      if ( count != _size ) {
        arr = (int*)realloc(arr, count * sizeof(int));
	_size = count;
      }
    }
    int pop_back() { int tmp = arr[_size-1]; _size--; return(tmp); }
    void push_back(int val) { unsigned int x = _size; size(_size+1); arr[x] = val; }
    int back() { return(arr[_size-1]); }
  };

  IntVector _colwidths;			// column widths in pixels
  IntVector _rowheights;		// row heights in pixels

  Fl_Cursor _last_cursor;		// last mouse cursor before changed to 'resize' cursor

  // EVENT CALLBACK DATA
  TableContext _callback_context;	// event context
  int _callback_row, _callback_col;	// event row/col

  // handle() state variables.
  //    Put here instead of local statics in handle(), so more
  //    than one Fl_Table can exist without crosstalk between them.
  //
  int _resizing_col;			// column being dragged
  int _resizing_row;			// row being dragged
  int _dragging_x;			// starting x position for horiz drag
  int _dragging_y;			// starting y position for vert drag
  int _last_row;			// last row we FL_PUSH'ed

  // Redraw single cell
  void _redraw_cell(TableContext context, int R, int C);

  void _start_auto_drag();
  void _stop_auto_drag();
  void _auto_drag_cb();
  static void _auto_drag_cb2(void *d);

protected:
  enum ResizeFlag {
    RESIZE_NONE      = 0,
    RESIZE_COL_LEFT  = 1,
    RESIZE_COL_RIGHT = 2,
    RESIZE_ROW_ABOVE = 3,
    RESIZE_ROW_BELOW = 4
  };

  int table_w, table_h;				// table's virtual size (in pixels)
  int toprow, botrow, leftcol, rightcol;	// four corners of viewable table

  // selection
  int current_row, current_col;
  int select_row, select_col;

  // OPTIMIZATION: Precomputed scroll positions for the toprow/leftcol
  int toprow_scrollpos;
  int leftcol_scrollpos;

  // Dimensions
  int tix, tiy, tiw, tih;			// data table inner dimension xywh
  int tox, toy, tow, toh;			// data table outer dimension xywh
  int wix, wiy, wiw, wih;			// widget inner dimension xywh

  Fl_Scroll *table;				// container for child fltk widgets (if any)
  Fl_Scrollbar *vscrollbar; 			// vertical scrollbar
  Fl_Scrollbar *hscrollbar;			// horizontal scrollbar

  // Fltk
  int handle(int e);				// fltk handle() override

  // Class maintenance
  void recalc_dimensions();
  void table_resized();				// table resized; recalc
  void table_scrolled();			// table scrolled; recalc
  void get_bounds(TableContext context,		// return x/y/w/h bounds for context
		  int &X, int &Y, int &W, int &H);
  void change_cursor(Fl_Cursor newcursor);	// change mouse cursor to some other shape
  TableContext cursor2rowcol(int &R, int &C, ResizeFlag &resizeflag);
					        // find r/c given current x/y event
  int find_cell(TableContext context,		// find cell's x/y/w/h given r/c
	       int R, int C, int &X, int &Y, int &W, int &H);
  int row_col_clamp(TableContext context, int &R, int &C);
					        // clamp r/c to known universe

  // Called to draw cells
  virtual void draw_cell(TableContext context, int R=0, int C=0, 
			 int X=0, int Y=0, int W=0, int H=0)
      { }					// overridden by deriving class

  long row_scroll_position(int row);		// find scroll position of row (in pixels)
  long col_scroll_position(int col);		// find scroll position of col (in pixels)

  int is_fltk_container() { 			// does table contain fltk widgets?
    return( Fl_Group::children() > 3 );		// (ie. more than box and 2 scrollbars?)
  }

  static void scroll_cb(Fl_Widget*,void*);	// h/v scrollbar callback

  void damage_zone(int r1, int c1, int r2, int c2, int r3 = 0, int c3 = 0);

  void redraw_range(int toprow, int botrow, int leftcol, int rightcol) {
    if ( _redraw_toprow == -1 ) {
      // Initialize redraw range
      _redraw_toprow = toprow;
      _redraw_botrow = botrow;
      _redraw_leftcol = leftcol;
      _redraw_rightcol = rightcol;
    } else {
      // Extend redraw range
      if ( toprow < _redraw_toprow ) _redraw_toprow = toprow;
      if ( botrow > _redraw_botrow ) _redraw_botrow = botrow;
      if ( leftcol < _redraw_leftcol ) _redraw_leftcol = leftcol;
      if ( rightcol > _redraw_rightcol ) _redraw_rightcol = rightcol;
    }

    // Indicate partial redraw needed of some cells
    damage(FL_DAMAGE_CHILD);
  }

public:
  Fl_Table(int X, int Y, int W, int H, const char *l=0);
  ~Fl_Table();

  virtual void clear() { rows(0); cols(0); }

  // topline()
  // middleline()
  // bottomline()

  inline void table_box(Fl_Boxtype val) {
    table->box(val);
    table_resized();
  }
  inline Fl_Boxtype table_box( void ) {
    return(table->box());
  }
  virtual void rows(int val);			// set/get number of rows
  inline int rows() {
    return(_rows);
  }
  virtual void cols(int val);			// set/get number of columns
  inline int cols() {
    return(_cols);
  }
  inline void visible_cells(int& r1, int& r2, int& c1, int& c2) {
    r1 = toprow;
    r2 = botrow;
    c1 = leftcol;
    c2 = rightcol;
  } 
  // Interactive resizing happening?
  int is_interactive_resize() {
    return(_resizing_row != -1 || _resizing_col != -1);
  } 
  inline int row_resize() {
    return(_row_resize);
  }
  void row_resize(int flag) {			// enable row resizing
    _row_resize = flag;
  }
  inline int col_resize() {
    return(_col_resize);
  }
  void col_resize(int flag) {			// enable col resizing
    _col_resize = flag;
  }
  inline int col_resize_min() {			// column minimum resizing width
    return(_col_resize_min);
  }
  void col_resize_min(int val) {
    _col_resize_min = ( val < 1 ) ? 1 : val;
  } 
  inline int row_resize_min() {			// column minimum resizing width
    return(_row_resize_min);
  }
  void row_resize_min(int val) {
    _row_resize_min = ( val < 1 ) ? 1 : val;
  }
  inline int row_header() {			// set/get row header enable flag
    return(_row_header);
  }
  void row_header(int flag) {
    _row_header = flag;
    table_resized();
    redraw();
  }
  inline int col_header() {			// set/get col header enable flag
    return(_col_header);
  }
  void col_header(int flag) {
    _col_header = flag;
    table_resized();
    redraw();
  }
  inline void col_header_height(int height) {	// set/get col header height
    _col_header_h = height;
    table_resized();
    redraw();
  }
  inline int col_header_height() {
    return(_col_header_h);
  }
  inline void row_header_width(int width) {	// set/get row header width
    _row_header_w = width;
    table_resized();
    redraw();
  }
  inline int row_header_width() {
    return(_row_header_w);
  }
  inline void row_header_color(Fl_Color val) {	// set/get row header color
    _row_header_color = val;
    redraw();
  }
  inline Fl_Color row_header_color() {
    return(_row_header_color);
  } 
  inline void col_header_color(Fl_Color val) {	// set/get col header color
    _col_header_color = val;
    redraw();
  }
  inline Fl_Color col_header_color() {
    return(_col_header_color);
  }
  void row_height(int row, int height);		// set/get row height
  inline int row_height(int row) {
    return((row<0 || row>=(int)_rowheights.size()) ? 0 : _rowheights[row]);
  }
  void col_width(int col, int width);		// set/get a column's width
  inline int col_width(int col) {
    return((col<0 || col>=(int)_colwidths.size()) ? 0 : _colwidths[col]);
  }
  void row_height_all(int height) {		// set all row/col heights
    for ( int r=0; r<rows(); r++ ) {
      row_height(r, height);
    }
  }
  void col_width_all(int width) {
    for ( int c=0; c<cols(); c++ ) {
      col_width(c, width);
    }
  }
  void row_position(int row);			// set/get table's current scroll position
  void col_position(int col);
  int row_position() {				// current row position
    return(_row_position);
  }
  int col_position() {				// current col position
    return(_col_position);
  }
  inline void top_row(int row) {		// set/get top row (deprecated)
    row_position(row);
  }
  inline int top_row() {
    return(row_position());
  }
  int is_selected(int r, int c);		// selected cell
  void get_selection(int& s_top, int& s_left, int& s_bottom, int& s_right);
  void set_selection(int s_top, int s_left, int s_bottom, int s_right);
  int move_cursor(int R, int C);

  void resize(int X, int Y, int W, int H);	// fltk resize() override
  void draw(void);				// fltk draw() override

// This crashes sortapp() during init.
//  void box(Fl_Boxtype val) {
//    Fl_Group::box(val);
//    if ( table ) {
//      resize(x(), y(), w(), h());
//    }
//  }
//  Fl_Boxtype box(void) const {
//    return(Fl_Group::box());
//  }

  // Child group
  void init_sizes() {
    table->init_sizes();
    table->redraw();
  }
  void add(Fl_Widget& w) {
    table->add(w);
  }
  void add(Fl_Widget* w) {
    table->add(w);
  }
  void insert(Fl_Widget& w, int n) {
    table->insert(w,n);
  }
  void insert(Fl_Widget& w, Fl_Widget* w2) {
    table->insert(w,w2);
  }
  void remove(Fl_Widget& w) {
    table->remove(w);
  }
  void begin() {
    table->begin();
  }
  void end() {
    table->end();
    // HACK: Avoid showing Fl_Scroll; seems to erase screen
    //       causing unnecessary flicker, even if its box() is FL_NO_BOX.
    //
    if ( table->children() > 2 ) {
      table->show();
    } else {
      table->hide();
    } 
    Fl_Group::current((Fl_Group*)(Fl_Group::parent()));
  }
  Fl_Widget * const *array() {
    return(table->array());
  }
  Fl_Widget *child(int n) const {
    return(table->child(n));
  }
  int children() const {
    return(table->children()-2);    // -2: skip Fl_Scroll's h/v scrollbar widgets
  }
  int find(const Fl_Widget *w) const {
    return(table->find(w));
  }
  int find(const Fl_Widget &w) const {
    return(table->find(w));
  } 
  // CALLBACKS
  int callback_row() {
    return(_callback_row);
  }
  int callback_col() {
    return(_callback_col);
  }
  TableContext callback_context() {
    return(_callback_context);
  }
  void do_callback(TableContext context, int row, int col) {
    _callback_context = context;
    _callback_row = row;
    _callback_col = col;
    Fl_Widget::do_callback();
  }
};

#endif /*_FL_TABLE_H*/
