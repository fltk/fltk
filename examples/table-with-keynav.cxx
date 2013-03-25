//
// "$Id$"
//
//    Example of Fl_Table with keyboard selection navigation - Greg Ercolano 04/14/2012
//
//    Display a 10x10 multiplication table, and allow the user to
//    make cell or row selections (with mouse or keyboard navigation)
//    to select areas of the table, and show the sum of the cell's values.
//
//    Started with the "testkeyboardnav.cxx" example from the original
//    Fl_Table project, using Jean-Marc Lienher's additions for keyboard nav.
//
// Copyright 2003, 2012 Greg Ercolano.
// Copyright 2004 Jean-Marc Lienher
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
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Toggle_Button.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Table_Row.H>
#include <FL/names.h>
#include <stdio.h>
#include <stdlib.h>

// GLOBALS
class MyTable;
Fl_Toggle_Button *G_rowselect = 0;		// toggle to enable row selection
MyTable          *G_table = 0;			// table widget
Fl_Output        *G_sum = 0;			// displays sum of user's selection

class MyTable : public Fl_Table_Row {
protected:
    // Handle drawing all cells in table
    void draw_cell(TableContext context, int R=0,int C=0, int X=0,int Y=0,int W=0,int H=0) {
	static char s[30]; 
	switch ( context ) {
	    case CONTEXT_COL_HEADER:
	    case CONTEXT_ROW_HEADER:
		fl_font(FL_HELVETICA | FL_BOLD, 14);
		fl_push_clip(X, Y, W, H);
		{
		    Fl_Color c = (context==CONTEXT_COL_HEADER) ? col_header_color() : row_header_color();
		    fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, c);
		    fl_color(FL_BLACK);
		    // Draw text for headers
		    sprintf(s, "%d", (context == CONTEXT_COL_HEADER) ? C : R);
		    fl_draw(s, X, Y, W, H, FL_ALIGN_CENTER);
		}
		fl_pop_clip();
		return;
	    case CONTEXT_CELL: {
		// Keyboard nav and mouse selection highlighting
		int selected = G_rowselect->value() ? row_selected(R) : is_selected(R,C);
		fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, selected ? FL_YELLOW : FL_WHITE);
		// Draw text for the cell
		fl_push_clip(X+3, Y+3, W-6, H-6);
		{
		    fl_font(FL_HELVETICA, 14);
		    fl_color(FL_BLACK); 
		    sprintf(s, "%d", R*C);		// factor row + col for data cells
		    fl_draw(s, X+3, Y+3, W-6, H-6, FL_ALIGN_RIGHT);
		}
		fl_pop_clip();
		return;
	    }
	    default:
		return;
	}
    }
public:
    // CTOR
    MyTable(int x, int y, int w, int h, const char *l=0) : Fl_Table_Row(x,y,w,h,l) {
	// Row init
	row_header(1);
	row_header_width(70);
	row_resize(1);
	rows(11);
	row_height_all(20);
	// Col init
	col_header(1);
	col_header_height(20);
	col_resize(1);
	cols(11);
	col_width_all(70);
	end();			// Fl_Table derives from Fl_Group, so end() it
    }
    ~MyTable() { }
    // Update the displayed sum value
    int GetSelectionSum() {
        int sum = -1;
        for ( int R=0; R<rows(); R++ ) {
	    for ( int C=0; C<cols(); C++ ) {
	        if ( G_rowselect->value() ? row_selected(R) : is_selected(R,C) ) {
		    if ( sum == -1 ) sum = 0;
		    sum += R*C;
		}
	    }
	}
	return(sum);
    }
    // Update the "Selection sum:" display
    void UpdateSum() {
	static char s[80];
	int sum = GetSelectionSum();
	if ( sum == -1 ) { sprintf(s, "(nothing selected)"); G_sum->color(48); }
	else             { sprintf(s, "%d", sum); G_sum->color(FL_WHITE); }
	// Update only if different (lets one copy/paste from sum)
	if ( strcmp(s,G_sum->value()))
	    { G_sum->value(s); G_sum->redraw(); }
    }
    // Keyboard and mouse events
    int handle(int e) {
        int ret = Fl_Table_Row::handle(e);
	if ( e == FL_KEYBOARD && Fl::event_key() == FL_Escape ) exit(0);
        switch (e) {
	    case FL_PUSH:
	    case FL_RELEASE:
	    case FL_KEYUP:
	    case FL_KEYDOWN:
	    case FL_DRAG: {
		//ret = 1;		// *don't* indicate we 'handled' these, just update ('handling' prevents e.g. tab nav)
	        UpdateSum();
		redraw();
		break;
	    }
	    case FL_FOCUS:		// tells FLTK we're interested in keyboard events
	    case FL_UNFOCUS:
	        ret = 1;
		break;
	}
	return(ret);
    }
};

// User changed the 'row select' toggle button
void RowSelect_CB(Fl_Widget *w, void*) {
    w->window()->redraw();		// redraw with changes applied
    G_table->UpdateSum();
}
int main() {
    Fl::option(Fl::OPTION_ARROW_FOCUS, 0);		// disable arrow focus nav (we want arrows to control cells)
    Fl_Double_Window win(862, 312, "table-with-keynav");
    win.begin();
	// Create table
	G_table = new MyTable(10, 30, win.w()-20, win.h()-70, "Times Table");
	G_table->tooltip("Use mouse or Shift + Arrow Keys to make selections.\n"
		         "Sum of selected values is shown.");
	// Row select toggle button
	G_rowselect = new Fl_Toggle_Button(140,10,12,12,"Row selection");
	G_rowselect->align(FL_ALIGN_LEFT);
	G_rowselect->value(0);
	G_rowselect->selection_color(FL_YELLOW);
	G_rowselect->callback(RowSelect_CB);
	G_rowselect->tooltip("Click to toggle row vs. row/col selection");
	// Selection sum display
	win.end();
	win.begin();
	G_sum = new Fl_Output(140,G_table->y()+G_table->h()+10,160,25,"Selection Sum:");
	G_sum->value("(nothing selected)");
	G_sum->color(48);
	G_sum->tooltip("This field shows the sum of the selected cells in the table");
    win.end();
    win.resizable(G_table);
    win.show(); 
    return Fl::run();
}

//
// End of "$Id$".
//
