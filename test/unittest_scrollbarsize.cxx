//
// "$Id$"
//
// Unit tests for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2009 by Bill Spitzak and others.
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

#include <FL/Fl_Group.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Tree.H>
#include <FL/Fl_Value_Slider.H>

//
// Test new 1.3.x global vs. local scrollbar sizing
//
class ScrollBarSizeTest : public Fl_Group {
    Fl_Browser *brow_a, *brow_b, *brow_c;
    Fl_Tree    *tree_a, *tree_b, *tree_c;

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
    void slide_cb2(Fl_Value_Slider *in) {
	const char *label = in->label();
	int val = in->value();
	//fprintf(stderr, "VAL='%d'\n",val);
	if ( strcmp(label,"A: Scroll Size") == 0 ) {
	    brow_a->scrollbar_size(val);
	    tree_a->scrollbar_size(val);
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
        //     |        |  |        |  |        |          |      |
        //     |        |  |        |  |        |          |browh |
        //     |        |  |        |  |        |          |      |
        //     ----------  ----------  ----------         ---   tgrph 
        //                                                 |      |
        //       tree_a      tree_b      tree_c            | 20   | 
        //     ----------  ----------  ----------         ---     |    <-- treey
        //     |        |  |        |  |        |          |      |
        //     |        |  |        |  |        |          |treeh |
        //     |        |  |        |  |        |          |      |
        //     ----------  ----------  ----------         ---  ------
        //                                     
        int tgrpy = Y+30;
        int tgrph = H-130;
        int browy = tgrpy+14;
        int browh = tgrph/2 - 20;
        int treey = browy + browh + 20;
        int treeh = browh;
        brow_a = makebrowser(X+ 10,browy,100,browh,"Browser A");
        brow_b = makebrowser(X+120,browy,100,browh,"Browser B");
        brow_c = makebrowser(X+240,browy,100,browh,"Browser C");
        tree_a = maketree(X+ 10,treey,100,treeh,"Tree A");
        tree_b = maketree(X+120,treey,100,treeh,"Tree B");
        tree_c = maketree(X+240,treey,100,treeh,"Tree C");
        Fl_Value_Slider *slide_glob = new Fl_Value_Slider(X+100,Y,100,18,"Global Scroll Size");
        slide_glob->value(16);
        slide_glob->type(FL_HORIZONTAL);
        slide_glob->align(FL_ALIGN_LEFT);
        slide_glob->range(0.0, 30.0);
        slide_glob->step(1.0);
        slide_glob->callback(slide_cb, (void*)this);
        slide_glob->labelsize(12);
        Fl_Value_Slider *slide_browa = new Fl_Value_Slider(X+350,Y,100,18,"A: Scroll Size");
        slide_browa->value(16);
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
	    "for 'Browser A', and the global size will only affect Browser B and C.");
      labelsize(12);
      align(FL_ALIGN_INSIDE|FL_ALIGN_BOTTOM|FL_ALIGN_LEFT|FL_ALIGN_WRAP);
    }
};

UnitTest scrollbarsize("scrollbar size", ScrollBarSizeTest::create);

//
// End of "$Id$"
//
