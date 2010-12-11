//
// "$Id$"
//
//	table-sort -- An example application using a sortable Fl_Table
//                
//	Originally the 'sortapp.cxx' example program that came with 
//	erco's Fl_Table widget. Added to FLTK in 2010.
//
//      Example of a non-trivial application that uses Fl_Table 
//      with sortable columns. This example is not trying to be simple,
//      but to demonstrate the complexities of an actual app.
//
// Copyright 2010 Greg Ercolano.
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
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Table_Row.H>

#include <stdio.h>
#include <string.h>
#include <vector>
#include <algorithm>            // STL sort

#define MARGIN 20

#ifdef _WIN32
// WINDOWS
#define DIRCMD          "dir"
#define DIRHEADER       { "Date", "Time", "Size", "Filename", "", "", "", "", "" }
#define popen           _popen
#else /*_WIN32*/
// UNIX
#include <ctype.h>
#define DIRCMD          "ls -l"
#define DIRHEADER       { "Perms", "#L", "Own", "Group", "Size", "Date", "", "", "Filename" }
#endif /*_WIN32*/

// A single row of columns
class Row {
public:
    std::vector<char*> cols;
};

// Sort class to handle sorting column using std::sort
class SortColumn {
    int _col, _reverse;
public:
    SortColumn(int col, int reverse) {
        _col = col;
        _reverse = reverse;
    }
    bool operator()(const Row &a, const Row &b) {
        const char *ap = ( _col < (int)a.cols.size() ) ? a.cols[_col] : "",
                   *bp = ( _col < (int)b.cols.size() ) ? b.cols[_col] : "";
        if ( isdigit(*ap) && isdigit(*bp) ) {           // cheezy detection of numeric data
            // Numeric sort
            int av=0; sscanf(ap, "%d", &av);
            int bv=0; sscanf(bp, "%d", &bv);
            return( _reverse ? av < bv : bv < av );
        } else {
            // Alphabetic sort
            return( _reverse ? strcmp(ap, bp) > 0 : strcmp(ap, bp) < 0 );
        }
    }
};

// Derive a custom class from Fl_Table_Row
class MyTable : public Fl_Table_Row {
private:
    std::vector<Row> _rowdata;                                  // data in each row
    int _sort_reverse;
    int _sort_lastcol;

    static void event_callback(Fl_Widget*, void*);
    void event_callback2();                                     // callback for table events

protected:
    void draw_cell(TableContext context, int R=0, int C=0,      // table cell drawing
                   int X=0, int Y=0, int W=0, int H=0);
    void sort_column(int col, int reverse=0);                   // sort table by a column
    void draw_sort_arrow(int X,int Y,int W,int H,int sort);

public:
    // Ctor
    MyTable(int x, int y, int w, int h, const char *l=0) : Fl_Table_Row(x,y,w,h,l) {
        _sort_reverse = 0;
        _sort_lastcol = -1;
        end();
        callback(event_callback, (void*)this);
    }
    ~MyTable() { }                              // Dtor
    void load_command(const char *cmd);         // Load the output of a command into table
    void autowidth(int pad);                    // Automatically set column widths to data
    void resize_window();                       // Resize parent window to size of table
};

// Sort a column up or down
void MyTable::sort_column(int col, int reverse) {
    std::sort(_rowdata.begin(), _rowdata.end(), SortColumn(col, reverse));
    redraw();
}

// Draw sort arrow
void MyTable::draw_sort_arrow(int X,int Y,int W,int H,int sort) {
    int xlft = X+(W-6)-8;
    int xctr = X+(W-6)-4;
    int xrit = X+(W-6)-0;
    int ytop = Y+(H/2)-4;
    int ybot = Y+(H/2)+4;
    if ( _sort_reverse ) {
        // Engraved down arrow
        fl_color(FL_WHITE);
        fl_line(xrit, ytop, xctr, ybot);
        fl_color(41);                   // dark gray
        fl_line(xlft, ytop, xrit, ytop);
        fl_line(xlft, ytop, xctr, ybot);
    } else {
        // Engraved up arrow
        fl_color(FL_WHITE);
        fl_line(xrit, ybot, xctr, ytop);
        fl_line(xrit, ybot, xlft, ybot);
        fl_color(41);                   // dark gray
        fl_line(xlft, ybot, xctr, ytop);
    }
}

// Handle drawing all cells in table
void MyTable::draw_cell(TableContext context, int R, int C, int X, int Y, int W, int H) {
    const char *s = ""; 
    if ( R < (int)_rowdata.size() && C < (int)_rowdata[R].cols.size() )
        s = _rowdata[R].cols[C]; 
    switch ( context ) {
        case CONTEXT_COL_HEADER:
            fl_push_clip(X,Y,W,H); {
                static const char *head[] = DIRHEADER; 
                fl_draw_box(FL_THIN_UP_BOX, X,Y,W,H, FL_BACKGROUND_COLOR);
                if ( C < 9 ) {
		    fl_font(FL_HELVETICA_BOLD, 16);
                    fl_color(FL_BLACK);
                    fl_draw(head[C], X+2,Y,W,H, FL_ALIGN_LEFT, 0, 0);         // +2=pad left 
                    // Draw sort arrow
                    if ( C == _sort_lastcol ) {
                        draw_sort_arrow(X,Y,W,H, _sort_reverse);
                    }
                }
            }
            fl_pop_clip();
            return; 
        case CONTEXT_CELL: {
            fl_push_clip(X,Y,W,H); {
                // Bg color
                Fl_Color bgcolor = row_selected(R) ? selection_color() : FL_WHITE;
                fl_color(bgcolor); fl_rectf(X,Y,W,H); 
		fl_font(FL_HELVETICA, 16);
                fl_color(FL_BLACK); fl_draw(s, X+2,Y,W,H, FL_ALIGN_LEFT);     // +2=pad left 
                // Border
                fl_color(FL_LIGHT2); fl_rect(X,Y,W,H);
            }
            fl_pop_clip();
            return;
        }
        default:
            return;
    }
}

// Automatically set column widths to widest data in each column
void MyTable::autowidth(int pad) {
    fl_font(FL_COURIER, 16); 
    // Initialize all column widths to lowest value
    for ( int c=0; c<cols(); c++ ) col_width(c, pad);
    for ( int r=0; r<(int)_rowdata.size(); r++ ) {
        int w, h;
        for ( int c=0; c<(int)_rowdata[r].cols.size(); c++ ) {
            fl_measure(_rowdata[r].cols[c], w, h, 0);           // get pixel width of text
            if ( (w + pad) > col_width(c)) col_width(c, w + pad);
        }
    }
    table_resized();
    redraw();
}

// Resize parent window to size of table
void MyTable::resize_window() {
    // Determine exact outer width of table with all columns visible
    int width = 4;                                          // width of table borders
    for ( int t=0; t<cols(); t++ ) width += col_width(t);   // total width of all columns
    width += MARGIN*2;
    if ( width < 200 || width > Fl::w() ) return;
    window()->resize(window()->x(), window()->y(), width, window()->h());  // resize window to fit
}

// Load table with output of 'cmd'
void MyTable::load_command(const char *cmd) {
    char s[512];
    FILE *fp = popen(cmd, "r");
    cols(0);
    for ( int r=0; fgets(s, sizeof(s)-1, fp); r++ ) {
        // Add a new row
        Row newrow; _rowdata.push_back(newrow);
        std::vector<char*> &rc = _rowdata[r].cols;
        // Break line into separate word 'columns'
        char *ss;
        const char *delim = " \t\n";
        for(int t=0; (t==0)?(ss=strtok(s,delim)):(ss=strtok(NULL,delim)); t++) {
            rc.push_back(strdup(ss));
        }
        // Keep track of max # columns
        if ( (int)rc.size() > cols() ) {
            cols((int)rc.size());
        }
    } 
    // How many rows we loaded
    rows((int)_rowdata.size()); 
    // Auto-calculate widths, with 20 pixel padding
    autowidth(20);
}

// Callback whenever someone clicks on different parts of the table
void MyTable::event_callback(Fl_Widget*, void *data) {
    MyTable *o = (MyTable*)data;
    o->event_callback2();
}

void MyTable::event_callback2() {
    //int ROW = callback_row();                 // unused
    int COL = callback_col();
    TableContext context = callback_context();
    switch ( context ) {
        case CONTEXT_COL_HEADER: {              // someone clicked on column header
            if ( Fl::event() == FL_RELEASE && Fl::event_button() == 1 ) {
                if ( _sort_lastcol == COL ) {   // Click same column? Toggle sort
                    _sort_reverse ^= 1;
                } else {                        // Click diff column? Up sort 
                    _sort_reverse = 0;
                }
                sort_column(COL, _sort_reverse);
                _sort_lastcol = COL;
            }
            break;
        }
        default:
            return;
    }
}

int main() {
    Fl_Window win(900,500,"Table Sorting");
        MyTable table(MARGIN, MARGIN, win.w()-MARGIN*2, win.h()-MARGIN*2);
        table.selection_color(FL_YELLOW);
        table.col_header(1);
        table.col_resize(1);
        table.when(FL_WHEN_RELEASE);            // handle table events on release
        table.load_command(DIRCMD);             // load table with a directory listing
        table.row_height_all(18);               // height of all rows
	table.tooltip("Click on column headings to toggle column sorting");
	table.color(FL_WHITE);
    win.end();
    win.resizable(table);
    table.resize_window();
    win.show();
    return(Fl::run());
}

//
// End of "$Id$".
//
