//
// "$Id$"
//
//     Test Jean-Marc's mods for keyboard nav and mouse selection
//     using a modified version of the Fl_Table 'singleinput' program.
//
//      Fl_Table[1.00/LGPL] 04/18/03 Mister Satan      -- Initial implementation, submitted to erco for Fl_Table
//      Fl_Table[1.10/LGPL] 05/17/03 Greg Ercolano     -- Small mods to follow changes to Fl_Table
//      Fl_Table[1.20/LGPL] 02/22/04 Jean-Marc Lienher -- Keyboard nav and mouse selection
//      Fl_Table[1.21/LGPL] 02/22/04 Greg Ercolano     -- Small reformatting mods, comments
//         FLTK[1.3.0/LGPL] 10/26/10 Greg Ercolano     -- Moved from Fl_Table to FLTK 1.3.x, CMP compliance
//
// Copyright 1998-2010 by Bill Spitzak and others.
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
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Table.H>
#include <stdio.h>
#include <stdlib.h>

const int MAX_COLS = 26;
const int MAX_ROWS = 500;

class SingleInput : public Fl_Table {
  Fl_Int_Input* input;
  int values[MAX_ROWS][MAX_COLS];
  int row_edit, col_edit;
  int s_left, s_top, s_right, s_bottom;	// kb nav + mouse selection

protected:
  void draw_cell(TableContext context, int=0, int=0, int=0, int=0, int=0, int=0);
  static void event_callback(Fl_Widget*, void*);
  void event_callback2();
  static void input_cb(Fl_Widget*, void* v);

public:
  SingleInput(int x, int y, int w, int h, const char* l=0) : Fl_Table(x,y,w,h,l) {
    int i, j;
    callback(&event_callback, (void*)this);
    when(FL_WHEN_NOT_CHANGED|when());
    input = new Fl_Int_Input(w/2,h/2,0,0);
    input->hide();
    input->callback(input_cb, (void*)this);
    input->when(FL_WHEN_ENTER_KEY_ALWAYS);
    input->maximum_size(5);
    for (i = 0; i < MAX_ROWS; i++) {
      for (j = 0; j < MAX_COLS; j++) {
	values[i][j] = (i + 2) * (j + 3);
      }
    }
    (new Fl_Box(9999,9999,0,0))->hide();  // HACK: prevent flickering in Fl_Scroll
    end();
  }
  ~SingleInput() { }

  // Change number of rows
  void rows(int val) {
    if (input->visible()) {
      input->do_callback(); 
    }
    Fl_Table::rows(val);
  }
  // Change number of columns
  void cols(int val) {
    if (input->visible()) {
      input->do_callback();
    }
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
  // Apply value from input widget to values[row][col] array
  void set_value() {
    values[row_edit][col_edit] = atoi(input->value());
    input->hide();
  }
};

void SingleInput::input_cb(Fl_Widget*, void* v) {
  ((SingleInput*)v)->set_value();
}

// Handle drawing all cells in table
void SingleInput::draw_cell(TableContext context, 
			    int R, int C, int X, int Y, int W, int H) {
  static char s[30]; 
  switch ( context ) {
    case CONTEXT_STARTPAGE:			// table about to redraw
      // Get kb nav + mouse 'selection region' for use below
      get_selection(s_top, s_left, s_bottom, s_right);
      break;

    case CONTEXT_COL_HEADER:			// table wants us to draw a column heading (C is column)
      fl_font(FL_HELVETICA | FL_BOLD, 14);
      fl_push_clip(X, Y, W, H);
      {
	fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, col_header_color());
	fl_color(FL_BLACK);
	if (C != cols()-1) {
	  // Not last column? Show column letter
	  s[0] = 'A' + C;
	  s[1] = '\0';
	  fl_draw(s, X, Y, W, H, FL_ALIGN_CENTER);
	} else {
	  // Last column? show 'TOTAL'
	  fl_draw("TOTAL", X, Y, W, H, FL_ALIGN_CENTER);
	}
      }
      fl_pop_clip();
      return;

    case CONTEXT_ROW_HEADER:			// table wants us to draw a row heading (R is row)
      fl_font(FL_HELVETICA | FL_BOLD, 14);
      fl_push_clip(X, Y, W, H);
      {
	fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, row_header_color());
	fl_color(FL_BLACK);
	if (R != rows()-1) {
	  // Not last row? Show row number
	  sprintf(s, "%d", R+1);
	  fl_draw(s, X, Y, W, H, FL_ALIGN_CENTER);
	} else {
	  // Last row? show 'TOTAL'
	  fl_draw("TOTAL", X, Y, W, H, FL_ALIGN_CENTER);
	}
      }
      fl_pop_clip();
      return;

    case CONTEXT_CELL: {			// table wants us to draw a cell
      if (R == row_edit && C == col_edit && input->visible()) {
	return;
      }

      // BACKGROUND
      fl_push_clip(X, Y, W, H);
      {
	// Keyboard nav and mouse selection highlighting
	if (R >= s_top && R <= s_bottom && C >= s_left && C <= s_right) {
	  fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, FL_YELLOW);
	} else {
	  fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, FL_WHITE);
	}
      }
      fl_pop_clip();

      // TEXT
      fl_push_clip(X+3, Y+3, W-6, H-6);
      {
	fl_color(FL_BLACK); 
	if (C != cols()-1 && R != rows()-1) {
	  fl_font(FL_HELVETICA, 14);
	  sprintf(s, "%d", values[R][C]);
	  fl_draw(s, X+3, Y+3, W-6, H-6, FL_ALIGN_RIGHT);
	} else {
	  int T = 0;
	  fl_font(FL_HELVETICA | FL_BOLD, 14);

	  if (C == cols()-1 && R == rows()-1) {	// All cells total
	    for (int c=0; c<cols()-1; ++c) {
	      for (int r=0; r<rows()-1; ++r) {
		T += values[r][c];
	      }
	    }
	  } else if (C == cols()-1) {		// Row subtotal
	    for (int c=0; c<cols()-1; ++c) {
	      T += values[R][c];
	    }
	  } else if (R == rows()-1) {		// Col subtotal
	    for (int r=0; r<rows()-1; ++r) {
	      T += values[r][C];
	    }
	  }

	  sprintf(s, "%d", T);
	  fl_draw(s, X+3, Y+3, W-6, H-6, FL_ALIGN_RIGHT);
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
void SingleInput::event_callback(Fl_Widget*, void* data) {
  SingleInput* o = (SingleInput*)data;
  o->event_callback2();
}

void SingleInput::event_callback2() {
  int R = callback_row();
  int C = callback_col();
  TableContext context = callback_context(); 

  switch ( context ) {
    case CONTEXT_CELL: {			// A table event occurred on a cell
      fprintf(stderr, "CALLBACK: CONTEXT_CELL: for R/C: %d / %d\n", R, C); 
      switch (Fl::event()) { 
	case FL_PUSH:
	  if (!Fl::event_clicks()) {
	    if (input->visible()) input->do_callback();
	    input->hide();
	    return;
	  }
	  Fl::event_clicks(0);
	  //FALLTHROUGH

	case FL_KEYBOARD:
	  if ( Fl::event_key() == FL_Escape )
	    exit(0);				// ESC closes app
	  if (Fl::event() == FL_KEYBOARD && 
	     ( Fl::e_length == 0 || Fl::event_key() == FL_Tab) ) {
	    return;				// ignore eg. keyboard nav keys
	  }
	  if (C == cols()-1 || R == rows()-1) return;
	  if (input->visible()) input->do_callback();
	  row_edit = R;
	  col_edit = C;
	  set_selection(R, C, R, C);
	  int XX,YY,WW,HH;
	  find_cell(CONTEXT_CELL, R, C, XX, YY, WW, HH);
	  input->resize(XX,YY,WW,HH);
	  char s[30];
	  sprintf(s, "%d", values[R][C]);
	  input->value(s);
	  input->position(0,strlen(s));		// pre-highlight (so typing replaces contents)
	  input->show();
	  input->take_focus();
	  if (Fl::event() == FL_KEYBOARD && Fl::e_text[0] != '\r') {
	    input->handle(Fl::event());
	  }
	  return;
      }
      return;
    }

    case CONTEXT_ROW_HEADER:			// A table event occurred on row/column header
    case CONTEXT_COL_HEADER:
      if (input->visible()) input->do_callback();
      input->hide();
      return;

    case CONTEXT_TABLE:				// A table event occurred on dead zone in table
      if (R < 0 && C < 0) {
	if (input->visible()) input->do_callback();
	input->hide();
      }
      return;
    
    default:
      return;
  }
}

// Change number of columns
void setcols_cb(Fl_Widget* w, void* v) {
  SingleInput* table = (SingleInput*)v;
  Fl_Valuator* in = (Fl_Valuator*)w;
  int cols = int(in->value()) + 1;
  table->cols(cols);
}

// Change number of rows
void setrows_cb(Fl_Widget* w, void* v) {
  SingleInput* table = (SingleInput*)v;
  Fl_Valuator* in = (Fl_Valuator*)w;
  int rows = int(in->value()) + 1;
  table->rows(rows);
}

int main() {
  Fl_Double_Window win(600, 400, "table with keyboard nav");

  SingleInput* table = new SingleInput(20, 20, win.w()-80, win.h()-80);
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
  win.begin();

  // Row slider
  Fl_Value_Slider setrows(win.w()-40,20,20,win.h()-80, 0);
  setrows.type(FL_VERT_NICE_SLIDER);
  setrows.bounds(2,MAX_ROWS);
  setrows.step(1);
  setrows.value(table->rows()-1);
  setrows.callback(setrows_cb, (void*)table);
  setrows.when(FL_WHEN_CHANGED);
  setrows.clear_visible_focus();

  // Column slider
  Fl_Value_Slider setcols(20,win.h()-40,win.w()-80,20, 0);
  setcols.type(FL_HOR_NICE_SLIDER);
  setcols.bounds(2,MAX_COLS);
  setcols.step(1);
  setcols.value(table->cols()-1);
  setcols.callback(setcols_cb, (void*)table);
  setcols.when(FL_WHEN_CHANGED);
  setcols.clear_visible_focus();

  win.end();
  win.resizable(table);
  win.show();

  return Fl::run();
}

//
// End of "$Id$".
//
