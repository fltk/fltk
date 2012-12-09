//
// "$Id$"
//
// Unit tests for the Fast Light Tool Kit (FLTK).
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

#include <FL/Fl_Group.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Tree.H>
#include <FL/Fl_Table.H>
#include <FL/Fl_Value_Slider.H>

//
// Test new 1.3.x global vs. local scrollbar sizing
//
class MyTable : public Fl_Table {
  // Handle drawing table's cells
  //     Fl_Table calls this function to draw each visible cell in the table.
  //     It's up to us to use FLTK's drawing functions to draw the cells the way we want.
  //
  void draw_cell(TableContext context, int ROW=0, int COL=0, int X=0, int Y=0, int W=0, int H=0) {
    static char s[10];
    switch ( context ) {
      case CONTEXT_STARTPAGE:                   // before page is drawn..
	fl_font(FL_HELVETICA, 8);               // set font for drawing operations
	return; 
      case CONTEXT_CELL:                        // Draw data in cells
        sprintf(s, "%c", 'A'+ROW+COL);
	fl_push_clip(X,Y,W,H);
	  // Draw cell bg
	  fl_color(FL_WHITE); fl_rectf(X,Y,W,H);
	  // Draw cell data
	  fl_color(FL_GRAY0); fl_draw(s, X,Y,W,H, FL_ALIGN_CENTER);
	  // Draw box border
	  fl_color(color()); fl_rect(X,Y,W,H);
	fl_pop_clip();
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
    rows(13);                   // how many rows
    row_height_all(10);         // default height of rows
    // Cols
    cols(13);                   // how many columns
    col_width_all(10);          // default width of columns
    end();			// end the Fl_Table group
  }
  ~MyTable() { }
};

class ScrollBarSizeTest : public Fl_Group {
    Fl_Browser *brow_a, *brow_b, *brow_c;
    Fl_Tree    *tree_a, *tree_b, *tree_c;
    MyTable    *table_a,*table_b,*table_c;

    Fl_Browser *makebrowser(int X,int Y,int W,int H,const char*L=0) {
	Fl_Browser *b = new Fl_Browser(X,Y,W,H,L);
	b->type(FL_MULTI_BROWSER);
	b->align(FL_ALIGN_TOP);
	b->add("Papa");     b->add("Delta"); b->add("Hotel");
        b->add("Long entry will show h-bar");
	b->add("Charlie");  b->add("Echo");  b->add("Foxtrot");
	b->add("Golf");     b->add("Lima");  b->add("Victor");
	b->add("Alpha");    b->add("Xray");  b->add("Yankee");
	b->add("Oscar");    b->add("India"); b->add("Juliet");
	b->add("Kilo");     b->add("Mike");  b->add("Sierra");
	b->add("November"); b->add("Tango"); b->add("Quebec");
	b->add("Bravo");    b->add("Romeo"); b->add("Uniform");
	b->add("Whisky");   b->add("Zulu");
	b->add("Papa");     b->add("Delta"); b->add("Hotel");
	b->add("Charlie");  b->add("Echo");  b->add("Foxtrot");
	b->add("Golf");     b->add("Lima");  b->add("Victor");
	b->add("Alpha");    b->add("Xray");  b->add("Yankee");
	b->add("Oscar");    b->add("India"); b->add("Juliet");
	b->add("Kilo");     b->add("Mike");  b->add("Sierra");
	b->add("November"); b->add("Tango"); b->add("Quebec");
	b->add("Bravo");    b->add("Romeo"); b->add("Uniform");
	b->add("Whisky");   b->add("Zulu");
	return(b);
    }
    Fl_Tree *maketree(int X,int Y,int W,int H,const char*L=0) {
	Fl_Tree *b = new Fl_Tree(X,Y,W,H,L);
	b->type(FL_TREE_SELECT_MULTI);
	b->align(FL_ALIGN_TOP);
	b->add("Papa");     b->add("Delta"); b->add("Hotel");
        b->add("Long entry will show h-bar");
	b->add("Charlie");  b->add("Echo");  b->add("Foxtrot");
	b->add("Golf");     b->add("Lima");  b->add("Victor");
	b->add("Alpha");    b->add("Xray");  b->add("Yankee");
	b->add("Oscar");    b->add("India"); b->add("Juliet");
	b->add("Kilo");     b->add("Mike");  b->add("Sierra");
	b->add("November"); b->add("Tango"); b->add("Quebec");
	b->add("Bravo");    b->add("Romeo"); b->add("Uniform");
	b->add("Whisky");   b->add("Zulu");
	return(b);
    }
    MyTable *maketable(int X,int Y,int W,int H,const char*L=0) {
	MyTable *mta = new MyTable(X,Y,W,H,L);
	mta->align(FL_ALIGN_TOP);
	mta->end();
	return(mta);
    }
    void slide_cb2(Fl_Value_Slider *in) {
	const char *label = in->label();
	int val = int(in->value());
	//fprintf(stderr, "VAL='%d'\n",val);
	if ( strcmp(label,"A: Scroll Size") == 0 ) {
	    brow_a->scrollbar_size(val);
	    tree_a->scrollbar_size(val);
#if FLTK_ABI_VERSION >= 10301
	    // NEW
	    table_a->scrollbar_size(val);
#endif
	} else {
	    Fl::scrollbar_size(val);
	}
	in->window()->redraw();
    }
    static void slide_cb(Fl_Widget *w, void *data) {
        ScrollBarSizeTest *o = (ScrollBarSizeTest*)data;
	o->slide_cb2((Fl_Value_Slider*)w);
    }
public: 
    static Fl_Widget *create() {
      return(new ScrollBarSizeTest(TESTAREA_X, TESTAREA_Y, TESTAREA_W, TESTAREA_H));
    }

    // CTOR
    ScrollBarSizeTest(int X, int Y, int W, int H) : Fl_Group(X,Y,W,H) {
      begin();
        //      _____________    _______________
        //     |_____________|  |_______________|
        //                                                ---   -----  <-- tgrpy
        //       brow_a      brow_b      brow_c            | 14   | 
        //     ----------  ----------  ----------         ---     |    <-- browy
        //     |        |  |        |  |        |          |browh |
        //     |        |  |        |  |        |          |      |
        //     ----------  ----------  ----------         ---   tgrph 
        //                                                 |      |
        //       tree_a      tree_b      tree_c            | 20   | 
        //     ----------  ----------  ----------         ---     |    <-- treey
        //     |        |  |        |  |        |          |treeh |
        //     |        |  |        |  |        |          |      |
        //     ----------  ----------  ----------         ---     |
        //                                                 |      |
        //      table_a     table_b     table_c            | 20   | 
        //     ----------  ----------  ----------         ---     |    <-- tabley
        //     |        |  |        |  |        |          |tableh|
        //     |        |  |        |  |        |          |      |
        //     ----------  ----------  ----------         ---  ------
        //  etc..
        int tgrpy = Y+30;
        int tgrph = H-130;
        int browy = tgrpy+14;
        int browh = tgrph/3 - 20;
        int treey = browy + browh + 20;
        int treeh = browh;
        int tabley = treey + treeh + 20;
        int tableh = browh;
        brow_a = makebrowser(X+ 10,browy,100,browh,"Browser A");
        brow_b = makebrowser(X+120,browy,100,browh,"Browser B");
        brow_c = makebrowser(X+230,browy,100,browh,"Browser C");
        tree_a = maketree(X+ 10,treey,100,treeh,"Tree A");
        tree_b = maketree(X+120,treey,100,treeh,"Tree B");
        tree_c = maketree(X+230,treey,100,treeh,"Tree C");
        table_a = maketable(X+ 10,tabley,100,tableh,"Table A");
        table_b = maketable(X+120,tabley,100,tableh,"Table B");
        table_c = maketable(X+230,tabley,100,tableh,"Table C");
        Fl_Value_Slider *slide_glob = new Fl_Value_Slider(X+100,Y,100,18,"Global Scroll Size");
        slide_glob->value(16);
        slide_glob->type(FL_HORIZONTAL);
        slide_glob->align(FL_ALIGN_LEFT);
        slide_glob->range(0.0, 30.0);
        slide_glob->step(1.0);
        slide_glob->callback(slide_cb, (void*)this);
        slide_glob->labelsize(12);
        Fl_Value_Slider *slide_browa = new Fl_Value_Slider(X+350,Y,100,18,"A: Scroll Size");
        slide_browa->value(0);
        slide_browa->type(FL_HORIZONTAL);
        slide_browa->align(FL_ALIGN_LEFT);
        slide_browa->range(0.0, 30.0);
        slide_browa->step(1.0);
        slide_browa->callback(slide_cb, (void*)this);
        slide_browa->labelsize(12);
      end();
      label("Verify global scroll sizing and per-widget scroll sizing.\n"
	    "Scrollbar's size should change interactively as size sliders are changed.\n"
            "Changing 'Global Scroll Size' should affect all three browser's scrollbars UNLESS\n"
	    "the 'A: Scroll Size' slider is changed, in which case its value will take precedence\n"
#if FLTK_ABI_VERSION >= 10301
	    "for the 'A' group of widgets.");
#else
	    "for the 'A' group of widgets. (NOTE: 'table_a' does not currently support this)");
#endif
      labelsize(10);
      align(FL_ALIGN_INSIDE|FL_ALIGN_BOTTOM|FL_ALIGN_LEFT|FL_ALIGN_WRAP);
    }
};

UnitTest scrollbarsize("scrollbar size", ScrollBarSizeTest::create);

//
// End of "$Id$"
//
