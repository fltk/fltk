//
// "$Id$"
//
// Show how FLTK widgets can be parented by Fl_Table.
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
// Please report all bugs and problems to:
//
//     http://www.fltk.org/str.php
//

#include <stdio.h>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Input.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Table.H>

void button_cb(Fl_Widget *w, void*);

//
// Simple demonstration class deriving from Fl_Table
//
class WidgetTable : public Fl_Table {
protected:
  void draw_cell(TableContext context,  		// table cell drawing
		 int R=0, int C=0, int X=0, int Y=0, int W=0, int H=0);

public:
  WidgetTable(int x, int y, int w, int h, const char *l=0) : Fl_Table(x,y,w,h,l) {
    col_header(1);
    col_resize(1);
    col_header_height(25);
    row_header(1);
    row_resize(1);
    row_header_width(80);
    end();
  }
  ~WidgetTable() { }

  void SetSize(int newrows, int newcols) {
    rows(newrows);
    cols(newcols);

    begin();		// start adding widgets to group
    {
      for ( int r = 0; r<newrows; r++ ) {
	for ( int c = 0; c<newcols; c++ ) {
	  int X,Y,W,H;
	  find_cell(CONTEXT_TABLE, r, c, X, Y, W, H);

	  char s[40];
	  if ( c & 1 ) {
	    // Create the input widgets
	    sprintf(s, "%d.%d", r, c);
	    Fl_Input *in = new Fl_Input(X,Y,W,H);
	    in->value(s);
	  } else {
	    // Create the light buttons
	    sprintf(s, "%d/%d ", r, c);
	    Fl_Light_Button *butt = new Fl_Light_Button(X,Y,W,H,strdup(s));
	    butt->align(FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
	    butt->callback(button_cb, (void*)0);
	    butt->value( ((r+c*2) & 4 ) ? 1 : 0);
	  }
	}
      }
    }
    end();
  }
};

// Handle drawing all cells in table
void WidgetTable::draw_cell(TableContext context, 
			  int R, int C, int X, int Y, int W, int H) {
  switch ( context ) {
    case CONTEXT_STARTPAGE:
      fl_font(FL_HELVETICA, 12);		// font used by all headers
      break;

    case CONTEXT_RC_RESIZE: {
      int X, Y, W, H;
      int index = 0;
      for ( int r = 0; r<rows(); r++ ) {
	for ( int c = 0; c<cols(); c++ ) {
	  if ( index >= children() ) break;
	  find_cell(CONTEXT_TABLE, r, c, X, Y, W, H);
	  child(index++)->resize(X,Y,W,H);
	}
      }
      init_sizes();			// tell group children resized
      return;
    }

    case CONTEXT_ROW_HEADER:
      fl_push_clip(X, Y, W, H);
      {
	static char s[40];
	sprintf(s, "Row %d", R);
	fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, row_header_color());
	fl_color(FL_BLACK);
	fl_draw(s, X, Y, W, H, FL_ALIGN_CENTER);
      }
      fl_pop_clip();
      return;

    case CONTEXT_COL_HEADER:
      fl_push_clip(X, Y, W, H);
      {
	static char s[40];
	sprintf(s, "Column %d", C);
	fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, col_header_color());
	fl_color(FL_BLACK);
	fl_draw(s, X, Y, W, H, FL_ALIGN_CENTER);
      }
      fl_pop_clip();
      return;

    case CONTEXT_CELL:
      return;		// fltk handles drawing the widgets

    default:
      return;
  }
}

void button_cb(Fl_Widget *w, void*) {
  fprintf(stderr, "BUTTON: %s\n", (const char*)w->label());
}

int main() {
  Fl_Double_Window win(940, 500, "table as container");
  WidgetTable table(20, 20, win.w()-40, win.h()-40, "FLTK widget table");
  table.SetSize(50, 50);
  win.end();
  win.resizable(table);
  win.show();
  return(Fl::run());
}

//
// End of "$Id$".
//
