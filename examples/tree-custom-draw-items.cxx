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
#include <math.h>	// sin(3)
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Tree.H>

#if FLTK_ABI_VERSION >= 10303
static void draw_item(Fl_Tree_Item *item, void *data) {
  Fl_Tree *tree = (Fl_Tree*)data;
  int X=item->label_x(), Y=item->label_y(),
      W=item->label_w(), H=item->label_h();
  // Draw the background
  fl_color(item->is_selected() ? tree->selection_color() : item->labelbgcolor());
  fl_rectf(X,Y,W,H);
  // Draw some red/grn/blu boxes
  int x = X + 5;
  fl_color(FL_RED);   fl_rectf(x, Y+2, 10, H-4); x += 10;
  fl_color(FL_GREEN); fl_rectf(x, Y+2, 10, H-4); x += 10;
  fl_color(FL_BLUE);  fl_rectf(x, Y+2, 10, H-4); x += 10;
  x += 5;
  // Draw text
  fl_font(item->labelfont(), item->labelsize());
  fl_color(item->labelfgcolor());
  char s[80];
  sprintf(s, "Custom: '%s'", item->label()?item->label():"---");
  fl_draw(s, x+tree->labelmarginleft(),Y,W,H, FL_ALIGN_LEFT);
  int fw=0,fh=0;
  fl_measure(s,fw,fh);
  x += fw + 10;
  // Draw a red sine wave past the text to end of xywh area
  fl_color(FL_RED);
  for ( float a=0.0; x<(X+W); x++,a+=.1) {
    int y = Y + sin(a) * ((H-2)/2) + (H/2);
    fl_point(x,y);
  }
}

int main(int argc, char *argv[]) {
  Fl::scheme("gtk+");
  Fl_Double_Window *win = new Fl_Double_Window(250, 400, "Simple Tree");
  win->begin();
  {
    // Create the tree
    Fl_Tree *tree = new Fl_Tree(0, 0, win->w(), win->h());
    tree->showroot(0);					// don't show root of tree
    tree->item_draw_callback(draw_item, (void*)tree);	// setup a callback for the tree

    // Add some items
    tree->add("Flintstones/Fred");
    tree->add("Flintstones/Wilma");
    tree->add("Flintstones/Pebbles");
    tree->add("Simpsons/Homer");
    tree->add("Simpsons/Marge");
    tree->add("Simpsons/Bart");
    tree->add("Simpsons/Lisa");
    tree->add("Superjail/Warden");
    tree->add("Superjail/Jared");
    tree->add("Superjail/Alice");
    tree->add("Superjail/Jailbot");

    // Start with some items closed
    tree->close("Simpsons");
    tree->close("Superjail");
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
