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
#include <FL/Fl_Text_Display.H>
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

static const char *phonetics[] = {
  "Alpha", "Bravo", "Charlie", "Delta", "Echo", "Foxtrot",
  "Golf", "Hotel", "India", "Juliet", "Kilo", "Lima", "Mike",
  "November", "Oscar", "Papa", "Quebec", "Romeo", "Sierra", "Tango",
  "Uniform", "Victor", "Whiskey", "X-ray", "Yankee", "Zulu", NULL
};

class ScrollBarSizeTest : public Fl_Group {
    Fl_Browser      *brow_a, *brow_b, *brow_c;
    Fl_Tree         *tree_a, *tree_b, *tree_c;
    MyTable         *table_a,*table_b,*table_c;
    Fl_Text_Display *text_a, *text_b, *text_c;

    Fl_Browser *makebrowser(int X,int Y,int W,int H,const char*L=0) {
	Fl_Browser *b = new Fl_Browser(X,Y,W,H,L);
	b->type(FL_MULTI_BROWSER);
	b->align(FL_ALIGN_TOP);
	for (int t=0; phonetics[t]; t++ ) {
	  b->add(phonetics[t]);
	  if ( phonetics[t][0] == 'C' ) b->add("Long entry will show h-bar");
	}
	return(b);
    }
    Fl_Tree *maketree(int X,int Y,int W,int H,const char*L=0) {
	Fl_Tree *b = new Fl_Tree(X,Y,W,H,L);
	b->type(FL_TREE_SELECT_MULTI);
	b->align(FL_ALIGN_TOP);
	for (int t=0; phonetics[t]; t++ ) {
	  b->add(phonetics[t]);
	  if ( phonetics[t][0] == 'C' ) b->add("Long entry will show h-bar");
	}
	return(b);
    }
    MyTable *maketable(int X,int Y,int W,int H,const char*L=0) {
	MyTable *mta = new MyTable(X,Y,W,H,L);
	mta->align(FL_ALIGN_TOP);
	mta->end();
	return(mta);
    }
    Fl_Text_Display *maketextdisplay(int X,int Y,int W,int H,const char*L=0) {
	Fl_Text_Display *dpy = new Fl_Text_Display(X,Y,W,H,L);
	Fl_Text_Buffer  *buf = new Fl_Text_Buffer();
	dpy->buffer(buf);
	for (int t=0; phonetics[t]; t++ ) {
	  buf->printf("%s\n", phonetics[t]);
	  if ( phonetics[t][0] == 'C' ) buf->printf("Long entry will show h-bar\n");
	}
	return(dpy);
    }
    void slide_cb2(Fl_Value_Slider *in) {
	const char *label = in->label();
	int val = int(in->value());
	//fprintf(stderr, "VAL='%d'\n",val);
	if ( strcmp(label,"A: Scroll Size") == 0 ) {
	    brow_a->scrollbar_size(val);
	    tree_a->scrollbar_size(val);
	    table_a->scrollbar_size(val);
	    text_a->scrollbar_size(val);
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
        int texty = tabley + tableh + 20;
        int texth = browh;
        brow_a = makebrowser(X+ 10,browy,100,browh,"Browser A");
        brow_b = makebrowser(X+120,browy,100,browh,"Browser B");
        brow_c = makebrowser(X+230,browy,100,browh,"Browser C");
        tree_a = maketree(X+ 10,treey,100,treeh,"Tree A");
        tree_b = maketree(X+120,treey,100,treeh,"Tree B");
        tree_c = maketree(X+230,treey,100,treeh,"Tree C");
        table_a = maketable(X+ 10,tabley,100,tableh,"Table A");
        table_b = maketable(X+120,tabley,100,tableh,"Table B");
        table_c = maketable(X+230,tabley,100,tableh,"Table C");
        text_a = maketextdisplay(X+ 10,texty,100,texth,"Text Display A");
        text_b = maketextdisplay(X+120,texty,100,texth,"Text Display B");
        text_c = maketextdisplay(X+230,texty,100,texth,"Text Display C");
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
        int msgbox_x = brow_c->x() + brow_c->w() + 20;
	int msgbox_w = W-(msgbox_x-X);
        Fl_Box *msgbox = new Fl_Box(msgbox_x,browy,msgbox_w,H-Y-48);
        msgbox->label("\nVerify global scrollbar sizing and per-widget scrollbar sizing. "
                      "Scrollbar's size should change interactively as size sliders are changed. "
                      "Changing 'Global Scroll Size' should affect all scrollbars AS LONG AS the "
                      "'A: Scroll Size' slider is 0. Otherwise its value takes precedence "
                      "for all the 'A' group widgets.");
        msgbox->labelsize(12);
        msgbox->align(FL_ALIGN_INSIDE|FL_ALIGN_CENTER|FL_ALIGN_LEFT|FL_ALIGN_WRAP);
	msgbox->box(FL_FLAT_BOX);
	msgbox->color(53); // 90% gray
      end();
    }
};

UnitTest scrollbarsize("scrollbar size", ScrollBarSizeTest::create);

//
// End of "$Id$"
//
