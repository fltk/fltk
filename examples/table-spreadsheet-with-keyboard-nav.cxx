//
// "$Id$"
//
//	Simple example of an interactive spreadsheet using Fl_Table.
//	Uses Mr. Satan's technique of instancing an Fl_Input around.
//	Modified to test Jean-Marc's mods for keyboard nav and mouse selection.
//
//      Fl_Table[1.00/LGPL] 04/18/03 Mister Satan      -- Initial implementation, submitted to erco for Fl_Table
//      Fl_Table[1.10/LGPL] 05/17/03 Greg Ercolano     -- Small mods to follow changes to Fl_Table
//      Fl_Table[1.20/LGPL] 02/22/04 Jean-Marc Lienher -- Keyboard nav and mouse selection
//      Fl_Table[1.21/LGPL] 02/22/04 Greg Ercolano     -- Small reformatting mods, comments
//         FLTK[1.3.0/LGPL] 10/26/10 Greg Ercolano     -- Moved from Fl_Table to FLTK 1.3.x, CMP compliance
//
// Copyright 1998-2010 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//
#include <stdio.h>
#include <stdlib.h>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Table.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/fl_draw.H>

const int MAX_COLS = 26;
const int MAX_ROWS = 500;

class Spreadsheet : public Fl_Table {
  Fl_Int_Input *input;					// single instance of Fl_Int_Input widget
  int values[MAX_ROWS][MAX_COLS];			// array of data for cells
  int row_edit, col_edit;				// row/col being modified
  int s_left, s_top, s_right, s_bottom;			// kb nav + mouse selection

protected:
  void draw_cell(TableContext context,int=0,int=0,int=0,int=0,int=0,int=0);
  void event_callback2();				// table's event callback (instance)
  static void event_callback(Fl_Widget*, void *v) {	// table's event callback (static)
    ((Spreadsheet*)v)->event_callback2();
  }
  static void input_cb(Fl_Widget*, void* v) {		// input widget's callback
    ((Spreadsheet*)v)->set_value_hide();
  }

public:
  Spreadsheet(int X,int Y,int W,int H,const char* L=0) : Fl_Table(X,Y,W,H,L) {
    callback(&event_callback, (void*)this);
    when(FL_WHEN_NOT_CHANGED|when());
    // Create input widget that we'll use whenever user clicks on a cell
    input = new Fl_Int_Input(W/2,H/2,0,0);
    input->hide();
    input->callback(input_cb, (void*)this);
    input->when(FL_WHEN_ENTER_KEY_ALWAYS);		// callback triggered when user hits Enter
    input->maximum_size(5);
    row_edit = col_edit = 0;
    s_left = s_top = s_right = s_bottom = 0;
    for (int c = 0; c < MAX_COLS; c++)
      for (int r = 0; r < MAX_ROWS; r++)
	values[r][c] = (r + 2) * (c + 3);		// initialize cells
    end();
  }
  ~Spreadsheet() { }

  // Apply value from input widget to values[row][col] array and hide (done editing)
  void set_value_hide() {
    values[row_edit][col_edit] = atoi(input->value());
    input->hide();
    window()->cursor(FL_CURSOR_DEFAULT);		// XXX: if we don't do this, cursor can disappear!
  }
  // Change number of rows
  void rows(int val) {
    Fl_Table::rows(val);
  }
  // Change number of columns
  void cols(int val) {
    Fl_Table::cols(val);
  }
  // Get number of rows
  inline int rows() {
    return Fl_Table::rows();
  }
  // Get number of columns
  inline int cols() {
    return Fl_Table::cols();
  }
  // Start editing a new cell: move the Fl_Int_Input widget to specified row/column
  //    Preload the widget with the cell's current value,
  //    and make the widget 'appear' at the cell's location.
  //
  void start_editing(int R, int C) {
    row_edit = R;					// Now editing this row/col
    col_edit = C;
    int X,Y,W,H;
    find_cell(CONTEXT_CELL, R,C, X,Y,W,H);		// Find X/Y/W/H of cell
    input->resize(X,Y,W,H);				// Move Fl_Input widget there
    char s[30]; sprintf(s, "%d", values[R][C]);		// Load input widget with cell's current value
    input->value(s);
    input->position(0,strlen(s));			// Select entire input field
    input->show();					// Show the input widget, now that we've positioned it
    input->take_focus();
  }
  // Tell the input widget it's done editing, and to 'hide'
  void done_editing() {
    if (input->visible()) {				// input widget visible, ie. edit in progress?
      set_value_hide();					// Transfer its current contents to cell and hide
    }
  }
  // Return the sum of all rows in this column
  int sum_rows(int C) {
    int sum = 0;
    for (int r=0; r<rows()-1; ++r)			// -1: don't include cell data in 'totals' column
      sum += values[r][C];
    return(sum);
  }
  // Return the sum of all cols in this row
  int sum_cols(int R) {
    int sum = 0;
    for (int c=0; c<cols()-1; ++c)			// -1: don't include cell data in 'totals' column
      sum += values[R][c];
    return(sum);
  }
  // Return the sum of all cells in table
  int sum_all() {
    int sum = 0;
    for (int c=0; c<cols()-1; ++c)			// -1: don't include cell data in 'totals' column
      for (int r=0; r<rows()-1; ++r)			// -1: ""
	sum += values[r][c];
    return(sum);
  }
};

// Handle drawing all cells in table
void Spreadsheet::draw_cell(TableContext context, int R,int C, int X,int Y,int W,int H) {
  static char s[30]; 
  switch ( context ) {
    case CONTEXT_STARTPAGE:			// table about to redraw
      // Get kb nav + mouse 'selection region' for use below
      get_selection(s_top, s_left, s_bottom, s_right);
      break;

    case CONTEXT_COL_HEADER:			// table wants us to draw a column heading (C is column)
      fl_font(FL_HELVETICA | FL_BOLD, 14);	// set font for heading to bold
      fl_push_clip(X,Y,W,H);			// clip region for text
      {
	fl_draw_box(FL_THIN_UP_BOX, X,Y,W,H, col_header_color());
	fl_color(FL_BLACK);
	if (C == cols()-1) {			// Last column? show 'TOTAL'
	  fl_draw("TOTAL", X,Y,W,H, FL_ALIGN_CENTER);
	} else {				// Not last column? show column letter
	  sprintf(s, "%c", 'A' + C);
	  fl_draw(s, X,Y,W,H, FL_ALIGN_CENTER);
	}
      }
      fl_pop_clip();
      return;

    case CONTEXT_ROW_HEADER:			// table wants us to draw a row heading (R is row)
      fl_font(FL_HELVETICA | FL_BOLD, 14);	// set font for row heading to bold
      fl_push_clip(X,Y,W,H);
      {
	fl_draw_box(FL_THIN_UP_BOX, X,Y,W,H, row_header_color());
	fl_color(FL_BLACK);
	if (R == rows()-1) {			// Last row? Show 'Total'
	  fl_draw("TOTAL", X,Y,W,H, FL_ALIGN_CENTER);
	} else {				// Not last row? show row#
	  sprintf(s, "%d", R+1);
	  fl_draw(s, X,Y,W,H, FL_ALIGN_CENTER);
	}
      }
      fl_pop_clip();
      return;

    case CONTEXT_CELL: {			// table wants us to draw a cell
      if (R == row_edit && C == col_edit && input->visible()) {
	return;					// dont draw for cell with input widget over it
      }
      // Background
      // Keyboard nav and mouse selection highlighting
      if (R >= s_top && R <= s_bottom && C >= s_left && C <= s_right) {
	fl_draw_box(FL_THIN_UP_BOX, X,Y,W,H, FL_YELLOW);
      } else if ( C < cols()-1 && R < rows()-1 ) {
	fl_draw_box(FL_THIN_UP_BOX, X,Y,W,H, FL_WHITE);
      } else {
	fl_draw_box(FL_THIN_UP_BOX, X,Y,W,H, 0xbbddbb00);	// money green
      }
      // Text
      fl_push_clip(X+3, Y+3, W-6, H-6);
      {
	fl_color(FL_BLACK); 
	if (C == cols()-1 || R == rows()-1) {	// Last row or col? Show total
	  fl_font(FL_HELVETICA | FL_BOLD, 14);	// ..in bold font
	  if (C == cols()-1 && R == rows()-1) {	// Last row+col? Total all cells
	    sprintf(s, "%d", sum_all());
	  } else if (C == cols()-1) {		// Row subtotal
	    sprintf(s, "%d", sum_cols(R));
	  } else if (R == rows()-1) {		// Col subtotal
	    sprintf(s, "%d", sum_rows(C));
	  }
	  fl_draw(s, X+3,Y+3,W-6,H-6, FL_ALIGN_RIGHT);
	} else {				// Not last row or col? Show cell contents
	  fl_font(FL_HELVETICA, 14);		// ..in regular font
	  sprintf(s, "%d", values[R][C]);
	  fl_draw(s, X+3,Y+3,W-6,H-6, FL_ALIGN_RIGHT);
	}
      }
      fl_pop_clip();
      return;
    }

    case CONTEXT_RC_RESIZE: {			// table resizing rows or columns
      if (!input->visible()) return;
      find_cell(CONTEXT_TABLE, row_edit, col_edit, X, Y, W, H);
      if (X==input->x() && Y==input->y() && W==input->w() && H==input->h()) {
	return;					// no change? ignore
      }
      input->resize(X,Y,W,H);
      return;
    }

    default:
      return;
  }
}

// Callback whenever someone clicks on different parts of the table
void Spreadsheet::event_callback2() {
  int R = callback_row();
  int C = callback_col();
  TableContext context = callback_context(); 

  switch ( context ) {
    case CONTEXT_CELL: {				// A table event occurred on a cell
      switch (Fl::event()) { 				// see what FLTK event caused it
	case FL_PUSH:					// mouse click?
	  done_editing();				// finish editing previous
	  if (R != rows()-1 && C != cols()-1 )		// only edit cells not in total's columns
	    start_editing(R,C);				// start new edit
	  return;

	case FL_KEYBOARD:				// key press in table?
	  if ( Fl::event_key() == FL_Escape ) exit(0);	// ESC closes app
	  if (C == cols()-1 || R == rows()-1) return;	// no editing of totals column
	  done_editing();				// finish any previous editing
	  set_selection(R, C, R, C);			// select the current cell
	  start_editing(R,C);				// start new edit
	  if (Fl::event() == FL_KEYBOARD && Fl::e_text[0] != '\r') {
	    input->handle(Fl::event());			// pass keypress to input widget
	  }
	  return;
      }
      return;
    }

    case CONTEXT_TABLE:					// A table event occurred on dead zone in table
    case CONTEXT_ROW_HEADER:				// A table event occurred on row/column header
    case CONTEXT_COL_HEADER:
      done_editing();					// done editing, hide
      return;
    
    default:
      return;
  }
}

// Change number of columns
void setcols_cb(Fl_Widget* w, void* v) {
  Spreadsheet* table = (Spreadsheet*)v;
  Fl_Valuator* in = (Fl_Valuator*)w;
  int cols = int(in->value()) + 1;
  table->cols(cols);
  table->redraw();
}

// Change number of rows
void setrows_cb(Fl_Widget* w, void* v) {
  Spreadsheet* table = (Spreadsheet*)v;
  Fl_Valuator* in = (Fl_Valuator*)w;
  int rows = int(in->value()) + 1;
  table->rows(rows);
  table->redraw();
}

int main() {
  Fl::option(Fl::OPTION_ARROW_FOCUS, 1);		// we want arrow keys to navigate table's widgets
  Fl_Double_Window *win = new Fl_Double_Window(922, 382, "Fl_Table Spreadsheet with Keyboard Navigation");
  Spreadsheet* table = new Spreadsheet(20, 20, win->w()-80, win->h()-80);
  // Table rows
  table->row_header(1);
  table->row_header_width(70);
  table->row_resize(1);
  table->rows(11);
  table->row_height_all(25);
  // Table cols
  table->col_header(1);
  table->col_header_height(25);
  table->col_resize(1);
  table->cols(11);
  table->col_width_all(70);
  table->set_selection(0,0,0,0);	// select top/left cell

  // Add children to window
  win->begin();

  // Row slider
  Fl_Value_Slider setrows(win->w()-40,20,20,win->h()-80, 0);
  setrows.type(FL_VERT_NICE_SLIDER);
  setrows.bounds(2,MAX_ROWS);
  setrows.step(1);
  setrows.value(table->rows()-1);
  setrows.callback(setrows_cb, (void*)table);
  setrows.when(FL_WHEN_CHANGED);
  setrows.clear_visible_focus();

  // Column slider
  Fl_Value_Slider setcols(20,win->h()-40,win->w()-80,20, 0);
  setcols.type(FL_HOR_NICE_SLIDER);
  setcols.bounds(2,MAX_COLS);
  setcols.step(1);
  setcols.value(table->cols()-1);
  setcols.callback(setcols_cb, (void*)table);
  setcols.when(FL_WHEN_CHANGED);
  setcols.clear_visible_focus();

  win->end();
  win->resizable(table);
  win->show();

  return Fl::run();
}

//
// End of "$Id$".
//
