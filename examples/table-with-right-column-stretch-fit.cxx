//
// "$Id$"
//
//
// Demonstrate resizing Fl_Table_Row right column to fit window to avoid appearance of horiz scrollbar
//
//     Date      Author   Description
//     --------  ------   -----------
//     01/03/17  erco     Modified version of examples/simple-table.cxx for Duncan's question on fltk.general
//                        Subject: How to disable horizontal scrollbar in Fl_Table_Row?
//     01/05/17  duncan   Added rows() == 0 early exit to FixColumnSize()
//     08/20/19  erco     Allow "- Row" button to completely clear table
//
// Copyright 2010, 2019 Greg Ercolano.
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
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Table.H>
#include <FL/fl_draw.H>

// Derive a class from Fl_Table
class MyTable : public Fl_Table {

  // Draw the row/col headings
  //    Make this a dark thin upbox with the text inside.
  //
  void DrawHeader(const char *s, int X, int Y, int W, int H) {
    fl_push_clip(X,Y,W,H);
      fl_draw_box(FL_THIN_UP_BOX, X,Y,W,H, row_header_color());
      fl_color(FL_BLACK);
      fl_draw(s, X,Y,W,H, FL_ALIGN_LEFT);
    fl_pop_clip();
  }
  // Draw the cell data
  //    Dark gray text on white background with subtle border
  //
  void DrawData(const char *s, int X, int Y, int W, int H) {
    fl_push_clip(X,Y,W,H);
      // Draw cell bg
      fl_color(FL_WHITE); fl_rectf(X,Y,W,H);
      // Draw cell data
      fl_color(FL_GRAY0); fl_draw(s, X,Y,W,H, FL_ALIGN_LEFT);
      // Draw box border
      fl_color(color()); fl_rect(X,Y,W,H);
    fl_pop_clip();
  }
  // Handle drawing table's cells
  //     Fl_Table calls this function to draw each visible cell in the table.
  //     It's up to us to use FLTK's drawing functions to draw the cells the way we want.
  //
  void draw_cell(TableContext context, int ROW=0, int COL=0, int X=0, int Y=0, int W=0, int H=0) {
    static char s[40];
    switch ( context ) {
      case CONTEXT_STARTPAGE:          // before page is drawn..
        fl_font(FL_HELVETICA, 14);     // set the font for our drawing operations
        return;
      case CONTEXT_COL_HEADER:         // Draw column headers
        switch(COL) {
          case 0: DrawHeader(" #Id",X,Y,W,H); break;
          case 1: DrawHeader(" Date / Time",X,Y,W,H); break;
        }
        return;
      case CONTEXT_ROW_HEADER:         // Draw row headers
        return;
      case CONTEXT_CELL:               // Draw data in cells
        switch(COL) {
          case 0: sprintf(s, " #%d", ROW); DrawData(s,X,Y,W,H); break;
          case 1: sprintf(s, " 2017-01-02 / 09:20:25.%d", ROW); DrawData(s,X,Y,W,H); break;
        }
        return;
      default:
        return;
    }
  }
public:
  // Constructor
  //     Make our data array, and initialize the table options.
  //
  MyTable(int X, int Y, int W, int H, const char *L=0) : Fl_Table(X,Y,W,H,L) {
    // Rows
    rows(10);              // how many rows
    row_header(0);         // disable row headers (along left)
    row_height_all(20);    // default height of rows
    row_resize(0);         // disable interactive row resizing
    // Cols
    cols(2);               // how many columns
    col_header(1);         // enable column headers (along top)
    col_width(0, 50);      // fixed width for left column
    col_width(1, 300);     // fixed width for right column (changed later by FixColumnSize()..)
    col_resize(0);         // disable interactive column resizing
    end();                 // end the Fl_Table group
    FixColumnSize();       // apply our auto-column-sizing behavior
  }
  ~MyTable() { }

  // Fix the right column's size to precisely match width of window
  void FixColumnSize() {
    if (rows() == 0) return;              // early exit if no rows to work with
    int X,Y,W,H;
    find_cell(CONTEXT_CELL,0,1,X,Y,W,H);  // get xywh of right column cell in first row
    int off = (X+W) - (tox+tow);          // we just need X pos and width. Compute offset from table's outer size
    int oldw = col_width(1);              // save old col width
    col_width(1, oldw - off);             // set new column width based on offset for perfect fit
  }

  // Handle window resizing
  void resize(int X,int Y,int W,int H) {
    Fl_Table::resize(X,Y,W,H);
    FixColumnSize();                      // after letting window resize, fix our right most column
  }
};

// Add more rows
void More_CB(Fl_Widget *w, void *d) {
  MyTable *table = (MyTable*)d;
  table->rows(table->rows()+1);
  table->FixColumnSize();
}

// Remove rows
void Less_CB(Fl_Widget *w, void *d) {
  MyTable *table = (MyTable*)d;
  if ( table->rows() > 0 ) table->rows(table->rows()-1);
  table->FixColumnSize();
}

int main(int argc, char **argv) {
  Fl_Double_Window *win = new Fl_Double_Window(500, 400, "Simple Table");
  MyTable *table = new MyTable(10,10,win->w()-20,340);
  Fl_Button more(10, 360,100,25,"+ Row"); more.callback(More_CB, (void*)table);
  Fl_Button less(220,360,100,25,"- Row"); less.callback(Less_CB, (void*)table);
  win->end();
  win->resizable(table);
  win->show(argc,argv);
  return(Fl::run());
}
