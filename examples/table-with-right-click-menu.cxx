//
//    Simple example of using Fl_Table with popup menus - Greg Ercolano 12/16/2023
//    Ref: https://www.seriss.com/people/erco/fltk/#GLDynamicPopup
//
// Copyright 2023 Greg Ercolano.
// Copyright 1998-2010 by Bill Spitzak and others.
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
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Table.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Menu_Button.H>

#define MAX_ROWS 30
#define MAX_COLS 26             // A-Z

// Derive a class from Fl_Table
class MyTable : public Fl_Table {
  // Post context menu at current event x,y
  void PostContextMenu() {
    int context = callback_context();
    switch (context) {
      case CONTEXT_COL_HEADER:
      case CONTEXT_CELL: {
        char s[80];
        // Create context sensitive menu label
        if ( context == CONTEXT_CELL ) {
          sprintf(s, "Cell %c%d", 'A'+callback_col(), callback_row());
        } else {
          sprintf(s, "Column %c", 'A'+callback_col());
        }
        // Post dynamically created context menu, get user's choice
        Fl_Menu_Button menu(Fl::event_x(), Fl::event_y(), 80, 1);
        menu.add(s, 0, 0, 0, FL_MENU_DIVIDER|FL_MENU_INACTIVE);
        menu.add("Item 1");
        menu.add("Item 2");
        const Fl_Menu_Item *item = menu.popup();
        if ( item ) printf("You chose '%s'\n", item->label());
        break;
      }
      default: break;
    }
  }

  // MyTable callback
  static void my_callback(Fl_Widget*, void*data) {
    MyTable *o = (MyTable*)data;
    if ( Fl::event_button() == FL_RIGHT_MOUSE ) o->PostContextMenu();
  }

  // Draw the row/col headings
  //    Make this a dark thin upbox with the text inside.
  //
  void DrawHeader(const char *s, int X, int Y, int W, int H) {
    fl_push_clip(X,Y,W,H);
      fl_draw_box(FL_THIN_UP_BOX, X,Y,W,H, row_header_color());
      fl_color(FL_BLACK);
      fl_draw(s, X,Y,W,H, FL_ALIGN_CENTER);
    fl_pop_clip();
  }
  // Draw the cells
  void DrawCell(const char *s, int X, int Y, int W, int H) {
    fl_push_clip(X,Y,W,H);
      fl_color(FL_WHITE); fl_rectf(X,Y,W,H);                    // Draw cell bg
      fl_color(FL_GRAY0); fl_draw(s, X,Y,W,H, FL_ALIGN_CENTER); // Draw cell text
      fl_color(color());  fl_rect(X,Y,W,H);                     // Draw box border
    fl_pop_clip();
  }
  // Handle drawing table's cells
  //     Fl_Table calls this function to draw each visible cell in the table.
  //     It's up to us to use FLTK's drawing functions to draw the cells the way we want.
  //
  void draw_cell(TableContext context, int ROW=0, int COL=0, int X=0, int Y=0, int W=0, int H=0) FL_OVERRIDE {
    static char s[40];
    switch ( context ) {
      case CONTEXT_STARTPAGE:                   // before page is drawn..
        fl_font(FL_HELVETICA, 16);              // set the font for our drawing operations
        return;
      case CONTEXT_COL_HEADER:                  // Draw column headers
        sprintf(s,"%c",'A'+COL);                // "A", "B", "C", etc.
        DrawHeader(s,X,Y,W,H);
        return;
      case CONTEXT_ROW_HEADER:                  // Draw row headers
        sprintf(s,"%03d:",ROW);                 // "001:", "002:", etc
        DrawHeader(s,X,Y,W,H);
        return;
      case CONTEXT_CELL:                        // Draw cells
        sprintf(s,"%c%d",'A'+COL,ROW);
        DrawCell(s,X,Y,W,H);
        return;
      default:
        return;
    }
  }
public:
  // Constructor
  MyTable(int X, int Y, int W, int H, const char *L=0) : Fl_Table(X,Y,W,H,L) {
    // Rows
    rows(MAX_ROWS);             // how many rows
    row_header(1);              // enable row headers (along left)
    row_height_all(20);         // default height of rows
    row_resize(0);              // disable row resizing
    // Cols
    cols(MAX_COLS);             // how many columns
    col_header(1);              // enable column headers (along top)
    col_width_all(80);          // default width of columns
    col_resize(1);              // enable column resizing
    end();                      // end the Fl_Table group
    callback(my_callback, (void*)this);  // set a callback for the table
  }
  ~MyTable() { }
};

int main(int argc, char **argv) {
  Fl_Double_Window win(900, 400, "Table Simple");
  MyTable table(10,10,880,380);
  win.end();
  win.resizable(table);
  win.show(argc,argv);
  return(Fl::run());
}
