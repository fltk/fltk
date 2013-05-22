//
// "$Id$"
//
//	Fl_Tree as a container of FLTK widgets. - erco 04/15/2012
//
// Copyright 2010,2012 Greg Ercolano.
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
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Tree.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Input.H>

#define MAX_ROWS     20000
#define MAX_FIELDS   5
#define FIELD_WIDTH  70
#define FIELD_HEIGHT 30

class MyData : public Fl_Group {
    Fl_Input *fields[MAX_FIELDS];
public:
    MyData(int X,int Y,int W,int H) : Fl_Group(X,Y,W,H) {
	static unsigned int colors[MAX_FIELDS] = {
	    0xffffdd00, 0xffdddd00, 0xddffff00, 0xddffdd00, 0xddddff00
	};
        for ( int t=0; t<MAX_FIELDS; t++ ) {
	    fields[t] = new Fl_Input(X+t*FIELD_WIDTH,Y,FIELD_WIDTH,H);
	    fields[t]->color(Fl_Color(colors[t]));
	}
	end();
    }
    void SetData(int col, const char *val) {
        if ( col >= 0 && col < MAX_FIELDS )
	    fields[col]->value(val);
    }
};

int main(int argc, char *argv[]) {
  Fl_Double_Window *win = new Fl_Double_Window(450, 400, "Tree As FLTK Widget Container");
  win->begin();
  {
    // Create the tree
    Fl_Tree *tree = new Fl_Tree(10, 10, win->w()-20, win->h()-20);
    tree->showroot(0);				// don't show root of tree
    // Add some regular text nodes
    tree->add("Foo/Bar/001");
    tree->add("Foo/Bar/002");
    tree->add("Foo/Bla/Aaa");
    tree->add("Foo/Bla/Bbb");
    // Add items to the 'Data' node
    for ( int t=0; t<MAX_ROWS; t++ ) {
        // Add item to tree
        static char s[80];
	sprintf(s, "FLTK Widgets/%d", t);
	Fl_Tree_Item *item = tree->add(s);
	// Reconfigure item to be an FLTK widget (MyData)
	tree->begin();
	{
	    MyData *data = new MyData(0,0,FIELD_WIDTH*MAX_FIELDS, FIELD_HEIGHT);
	    item->widget(data);
	    // Initialize widget data
	    for ( int c=0; c<MAX_FIELDS; c++ ) {
		sprintf(s, "%d-%d", t,c);
	        data->SetData(c,s);
	    }
	}
	tree->end();
    }
  }
  win->end();
  win->resizable(win);
  win->show(argc, argv);
  return(Fl::run());
}

//
// End of "$Id$".
//
