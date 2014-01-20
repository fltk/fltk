//
// "$Id$"
//
//	Demonstrate Fl_Tree custom item draw callback. - erco 11/09/2013
//
// Copyright 2013 Greg Ercolano.
// Copyright 1998-2013 by Bill Spitzak and others.
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
#include <time.h>		/* ctime.. */
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Tree.H>

#ifndef MAX
#define MAX(a,b) ((a)>(b))?(a):(b)
#endif

#if FLTK_ABI_VERSION >= 10303
// DERIVE CUSTOM CLASS FROM Fl_Tree_Item TO IMPLEMENT SHOWING THE TIME OF DAY
//     This demonstrates that item content can be dynamic and highly customized.
//
class MyTimeItem : public Fl_Tree_Item {
  const char *time_format;
protected:
  // Remove trailing crlf
  const char* StripCrlf(char *s)
    { char *ss = strchr(s, '\n'); if (ss) *ss = 0; return s; }
  const struct tm* GetTimeStruct() {
    time_t t = time(NULL);
    if ( strcmp(time_format, "Local") == 0 ) return localtime(&t);
    if ( strcmp(time_format, "GMT"  ) == 0 ) return gmtime(&t);
    return 0;
  }
public:
  MyTimeItem(Fl_Tree *tree, const char *time_format) : Fl_Tree_Item(tree) {
    label(time_format);
    this->time_format = time_format;
  }
  // Handle custom drawing of the item
  //    Fl_Tree has already handled drawing everything to the left
  //    of the label area, including any 'user icon', collapse buttons,
  //    connector lines, etc.
  //
  //    All we're responsible for is drawing the 'label' area of the item
  //    and it's background. Fl_Tree gives us a hint as to what the
  //    foreground and background colors should be via the fg/bg parameters,
  //    and whether we're supposed to render anything or not.
  //
  //    The only other thing we must do is return the maximum X position
  //    of scrollable content, i.e. the right most X position of content
  //    that we want the user to be able to use the horizontal scrollbar
  //    to reach.
  //
  int draw_item_content(int render) {
    Fl_Color fg = drawfgcolor();
    Fl_Color bg = drawbgcolor();
    //    Show the date and time as two small strings
    //    one on top of the other in a single item.
    //
    // Our item's label dimensions
    int X = label_x(), Y = label_y(),
        W = label_w(), H = label_h(); 
    // Render background
    if ( render ) {
      if ( is_selected() ) {			// Selected? Use selectbox() style
        fl_draw_box(prefs().selectbox(),X,Y,W,H,bg);
      } else {					// Not Selected? use plain filled rectangle
        fl_color(bg); fl_rectf(X,Y,W,H);
      }
    }
    // Render the label
    if ( render ) {
      fl_color(fg);
      if ( label() ) fl_draw(label(), X,Y,W,H, FL_ALIGN_LEFT);
    }
    int lw=0, lh=0;
    if ( label() ) {
      lw=0; lh=0; fl_measure(label(), lw, lh);
    }
    X += lw + 8;
    // Draw some red/grn/blu boxes
    if ( render ) {
      fl_color(FL_RED);   fl_rectf(X+0,  Y+2, 10, H-4);
      fl_color(FL_GREEN); fl_rectf(X+10, Y+2, 10, H-4);
      fl_color(FL_BLUE);  fl_rectf(X+20, Y+2, 10, H-4);
    }
    X += 35;
    // Render the date and time, one over the other
    fl_font(labelfont(), 8);			// small font
    const struct tm *tm = GetTimeStruct();
    char s[80];
    sprintf(s, "Date: %02d/%02d/%02d", tm->tm_mon+1, tm->tm_mday, tm->tm_year % 100);
    lw=0, lh=0; fl_measure(s, lw, lh);		// get box around text (including white space)
    if ( render ) fl_draw(s, X,Y+4,W,H, FL_ALIGN_LEFT|FL_ALIGN_TOP);
    sprintf(s, "Time: %02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);
    if ( render ) fl_draw(s, X,Y+H/2,W,H/2, FL_ALIGN_LEFT|FL_ALIGN_TOP);
    int lw2=0, lh2=0; fl_measure(s, lw2, lh2);
    X += MAX(lw, lw2);
    return X;			// return right most edge of what we've rendered
  }
};

// TIMER TO HANDLE DYNAMIC CONTENT IN THE TREE
void Timer_CB(void *data) {
  Fl_Tree *tree = (Fl_Tree*)data;
  tree->redraw();	// keeps time updated
  Fl::repeat_timeout(0.2, Timer_CB, data);
}

int main(int argc, char *argv[]) {
  Fl::scheme("gtk+");
  Fl_Double_Window *win = new Fl_Double_Window(350, 400, "Simple Tree");
  win->begin();
  {
    // Create the tree
    Fl_Tree *tree = new Fl_Tree(0, 0, win->w(), win->h());
    tree->showroot(0);				// don't show root of tree
    tree->selectmode(FL_TREE_SELECT_MULTI);	// multiselect

    // Add some items
    tree->add("Flintstones/Fred");
    tree->add("Flintstones/Wilma");
    tree->add("Flintstones/Pebbles");
    {
      MyTimeItem *myitem;
      myitem = new MyTimeItem(tree, "Local");	// create custom item
      myitem->labelsize(20);
      tree->add("Time Add Item/Local", myitem);

      myitem = new MyTimeItem(tree, "GMT");	// create custom item
      myitem->labelsize(20);
      tree->add("Time Add Item/GMT", myitem);
    }
    // 'Replace' approach
    {
      Fl_Tree_Item *item;
      MyTimeItem *myitem;
      item = tree->add("Time Replace Item/Local Time");
      // Replace the 'Local' item with our own
      myitem = new MyTimeItem(tree, "Local");	// create custom item
      myitem->labelsize(20);
      item->replace(myitem);			// replace normal item with custom

      item = tree->add("Time Replace Item/GMT Time");
      // Replace the 'GMT' item with our own
      myitem = new MyTimeItem(tree, "GMT");	// create custom item
      myitem->labelsize(20);
      item->replace(myitem);			// replace normal item with custom
    }
    tree->add("Superjail/Warden");
    tree->add("Superjail/Jared");
    tree->add("Superjail/Alice");
    tree->add("Superjail/Jailbot");

    tree->show_self();

    // Start with some items closed
    tree->close("Superjail");

    // Set up a timer to keep time in tree updated
    Fl::add_timeout(0.2, Timer_CB, (void*)tree);
  }
  win->end();
  win->resizable(win);
  win->show(argc, argv);
  return(Fl::run());
}
#else
#include <FL/Fl.H>
#include <FL/fl_message.H>
int main(int, char**) {
  fl_alert("This demo is dependent on an ABI feature.\n"
           "FLTK_ABI_VERSION must be set to 10303 (or higher) in Enumerations.H");
  return 1;
}
#endif

//
// End of "$Id$".
//
